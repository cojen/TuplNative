#include "Node.hpp"

#include <iterator>

namespace tupl { namespace pvt { namespace slow {

namespace {

struct KeyLesser {
    template<typename K, typename V>
    bool operator()(const std::pair<K, V>& k1, const std::pair<K, V>& k2) const
    {
        return k1.first < k2.first;
    }

    template<typename K, typename V>
    bool operator()(const std::pair<K, V>& k1, Bytes k2) const {
        return std::lexicographical_compare(k1.first.begin(), k1.first.end(),
                                            k2.data(), k2.data() + k2.size());
    }
    
    template<typename K, typename V>
    bool operator()(Bytes k1, const std::pair<K, V>& k2) const {
        return std::lexicographical_compare(k1.data(), k1.data() + k1.size(),
                                            k2.first.begin(), k2.first.end());
    }
};

struct KeyEquals {
    template<typename K, typename V>
    bool operator()(Bytes k1, const std::pair<K, V>& k2) const {
        return k1.size() == k2.first.size() &&
            std::equal(k2.first.begin(), k2.first.end(), k1.data());
    }
    
    template<typename K, typename V>
    bool operator()(const std::pair<K, V>& k1, Bytes k2) const {
        return operator()(k2, k1);
    }
};

size_t valueSize(Bytes value) {
    return value.size();
}

size_t valueSize(const Node& node) {
    return sizeof(&node);
}

size_t verifiedEntrySize(const Node& node,
                         const size_t keyBytes, const size_t valueBytes)
{
    
    const size_t capacity = node.capacity();
    const size_t kvBytes = keyBytes + valueBytes;
    
    // ensures that a split always produces siblings with at least two entries
    const size_t maxEntry = capacity / 4;
    
    if (kvBytes > maxEntry) { throw std::invalid_argument("entry too big"); }
    
    return kvBytes;
}

Buffer transformToNodeValue(Bytes value) {
    return Buffer{value.data(), value.size()};
}

Node* transformToNodeValue(Node& childNode) {
    return &childNode;
}

}

class Node::Ops final {
public:
    static InsertResult insert(Bytes key, Bytes value, LeafNode& node) {
        // TODO: fold back into caller
        auto& children = node.mChildren;
        
        const auto insertPos = std::lower_bound(
            children.begin(), children.end(), key, KeyLesser());
        
        if (insertPos != children.end() && KeyEquals()(key, *insertPos)) {
            throw std::invalid_argument("duplicate key not allowed");
        }
        
        return insert(insertPos, key, value, node);
    }
    
    static InsertResult insert(Bytes key, Node& value, InternalNode& node) {
        // TODO: fold back into caller
        auto& children = node.mChildren;
        
        auto insertPos = std::lower_bound(
            children.begin() + 1, children.end(), key, KeyLesser());
        
        if (insertPos != children.end() && KeyEquals()(key, *insertPos)) {
            throw std::invalid_argument("duplicate key not allowed");
        }
        
        // This position has a pointer to children who's value is greater
        // than or equal to key. Move backwards for lower keys.
        --insertPos;
        
        return insert(insertPos, key, value, node);
    }
    
    template<typename Iterator, typename Value, typename NodeType>
    static InsertResult insert(
        Iterator& position, Bytes key, Value& value, NodeType& node)
    {
        auto& children = node.mChildren;
        
        const auto entrySize =
            verifiedEntrySize(node, key.size(), valueSize(value));
    
        const auto usedBytes = node.bytes();
        const auto availableBytes = node.capacity() - usedBytes;

        if (entrySize > availableBytes) {
            return InsertResult::FAILED_NO_SPACE;
        }
    
        const auto oldSize = children.size();
    
        children.emplace(position,
                         std::make_pair(Buffer{key.data(), key.size()},
                                        transformToNodeValue(value)));
    
        if (oldSize == children.size()) {
            // FIXME: add an assertion here, this is a trusted method
            throw std::invalid_argument("key exists"); 
        }
    
        node.mBytes = usedBytes + entrySize;
        
        return InsertResult::INSERTED;
    }
    
    template<typename Value, typename NodeTyp>
    static void splitAndInsert(Bytes key, Value& value,
                               NodeTyp& original, NodeTyp& sibling) 
    {
        assert(original.type() == sibling.type());
        
        const auto entryBytes =
            verifiedEntrySize(original, key.size(), valueSize(value));
        
        const auto usedBytes = original.bytes();
        const auto availableBytes = original.capacity() - usedBytes;
        const auto size = original.mChildren.size();
    
        if (entryBytes < availableBytes || size < 4) {
            throw std::domain_error("split is not required");
        }
    
        const auto beginIt = original.mChildren.begin();
        const auto endIt   = original.mChildren.end();
    
        const auto splitBeginIt = beginIt + size / 2;
    
        // TODO: Use custom lower_bound that checks for duplicates
        const auto origInsertIt = std::lower_bound(
            beginIt, endIt, key, KeyLesser());
        
        if (origInsertIt != endIt && KeyEquals()(key, *origInsertIt)) {
            throw std::invalid_argument("duplicate key not allowed");
        }
        
        auto siblingEndInserter = std::back_inserter(sibling.mChildren);
    
        if (origInsertIt < splitBeginIt) { // new element goes to the left half
            std::move(beginIt, origInsertIt, siblingEndInserter);
        
            siblingEndInserter = std::make_pair(Buffer{key.data(), key.size()},
                                                transformToNodeValue(value));
        
            std::move(origInsertIt, splitBeginIt, siblingEndInserter);
        
            original.mChildren.erase(beginIt, splitBeginIt);
            
            if (original.type() == NodeType::INTERNAL) {
                splitBeginIt->first = Buffer{};
                original.recordSplit(sibling, SiblingDirection::LEFT);
            } else {
                original.recordSplit(sibling, SiblingDirection::LEFT);
            }
        } else { // new element goes to right half
            if (original.type() == NodeType::INTERNAL) {
                siblingEndInserter =
                    std::make_pair(Buffer{}, std::move(splitBeginIt->second));
                
                std::move(splitBeginIt + 1, origInsertIt, siblingEndInserter);
            } else {
                std::move(splitBeginIt, origInsertIt, siblingEndInserter);
            }
            
            siblingEndInserter = std::make_pair(Buffer{key.data(), key.size()},
                                                transformToNodeValue(value));
        
            std::move(origInsertIt, endIt, siblingEndInserter);
        
            original.mChildren.erase(splitBeginIt, endIt);
        
            original.recordSplit(sibling, SiblingDirection::RIGHT);
        }
    }
};

/*---------------------------------------------------------------------------*/
// InternalNode implementation 
/*---------------------------------------------------------------------------*/
InternalNode::InternalNode(Node& leftestChild) :
    Node(NodeType::INTERNAL)
{
    mChildren.emplace_back(std::make_pair(Buffer{}, &leftestChild));
    // mChildren.emplace_back(std::make_pair(Buffer{key.data(), key.size()},
    //                                       &right));
}

InsertResult InternalNode::insertGeneric(Bytes key, Node& value)
{
    return Ops::insert(key, value, *this);
}

InsertResult InternalNode::insertGeneric(
    Iterator position, Bytes key, Node& value)
{
    return Ops::insert(position.base(), key, value, *this);
}

void InternalNode::splitAndInsertGeneric(
    Bytes key, Node& value, InternalNode& sibling)
{
    Ops::splitAndInsert(key, value, *this, sibling);
}

/*---------------------------------------------------------------------------*/
// LeafNode implementation
/*---------------------------------------------------------------------------*/
InsertResult LeafNode::insert(Bytes key, Bytes value) {
    return Ops::insert(key, value, *this);
}

InsertResult LeafNode::insert(Iterator position, Bytes key, Bytes value) {
    return Ops::insert(position.base(), key, value, *this);
}

void LeafNode::splitAndInsert(Bytes key, Bytes value, LeafNode& sibling) {
    Ops::splitAndInsert(key, value, *this, sibling);
}

LeafNode::Iterator LeafNode::moveEntries(
    LeafNode& src, Iterator srcBegin, Iterator srcEnd,
    LeafNode& /* dst */, Iterator tgtBegin)
{
    auto srcBeginIt = srcBegin.base();
    auto srcEndIt = srcEnd.base();
    
    auto retVal = std::move(srcBeginIt, srcEndIt, tgtBegin.base());
    
    src.mChildren.erase(srcBeginIt, srcEndIt);
    
    return {retVal, BufferPairToBytesPair()};
}

} } }

/*
  Split Logic for Internal Nodes:

  The "slow" node implementation has every internal node key maintain a pointer
  to a child whose keys are GE than itself. The extra pointer is therefore
  logically to the left
  
                    +--------------+
                    | SS | 50 | 80 | 
                    +-+------+--+--+
                      |      |  |
          +-----------+      |  +--------------+
          |                  |                 |
   +------+-------+   +------+-------+  +------+-------+ 
   | 30 | 35 | 40 |   | 50 | 55 | 60 |  | 80 | 85 | 90 | 
   +--------------+   +--------------+  +--------------+

  As result of this the split logic that follows is:

  The split key is defined to be the key in the node at which the split occurs:
  
  +--------------+
  | *5 | 55 | 60 |  (* => just inserted value)
  +--------------+

  The split key in this case is 55. Based on our description of the tree above,
  a child node associated with 55, will only have values GE than 55
  
  If the parent is an internal node and detects that its child has split
  (child can be a leaf), then the the parent needs to:

  If a LEFT split (i.e. the new child is to the left of the existing one):
    -Update the pointer associated with the existing key to the new node
    -Add the split key to the set with a reference to the old node
   
  If a RIGHT split (i.e. the new child is to the right of the existing one):
    -Add the split key to the set with a reference to the new node    
       
  Internal Node splits:
    -Split Key is set to sentinel
    -Split Key is bubbled to parent as the split key
*/ 
