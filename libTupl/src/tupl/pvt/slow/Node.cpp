#include "Node.hpp"

#include <iterator>

namespace tupl { namespace pvt { namespace slow {

class Ops {
public:
    
};

namespace {

struct KeyCompare {
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

static size_t verifiedEntrySize(const Node& node,
                                const size_t keyBytes, const size_t valueBytes)
{
    
    const size_t capacity = node.capacity();
    const size_t kvBytes = keyBytes + valueBytes;
    
    // ensures that a split always produces siblings with at least two entries
    const size_t maxEntry = capacity / 4;
    
    if (kvBytes > maxEntry) { throw std::invalid_argument("entry too big"); }
    
    return kvBytes;
}

}

InternalNode::Iterator InternalNode::lowerBound(Bytes key) {
    const Buffer keyBuf(key.data(), key.size());
    return mChildren.lower_bound(keyBuf);
}

InsertResult InternalNode::insert(
    Iterator position, Bytes key, Node& node)
{
    const auto entrySize = verifiedEntrySize(*this, key.size(), sizeof(Node*));
    const auto usedBytes = bytes();
    const auto availableBytes = capacity() - usedBytes;

    if (entrySize > availableBytes) { return InsertResult::FAILED_NO_SPACE; }
    
    const auto oldSize = mChildren.size();
    
    mChildren.emplace_hint(
        position.mIt, Buffer{key.data(), key.size()}, &node);

    if (oldSize == mChildren.size()) {
        throw std::invalid_argument("key exists");
    }
    
    mBytes = usedBytes + entrySize;
    
    return InsertResult::INSERTED;
}

InsertResult LeafNode::insert(Bytes key, Bytes value) {
    // FIXME: Duplicate key detection, improve comparator
    const auto insertPos = std::lower_bound(mValues.begin(), mValues.end(), key,
                                            KeyCompare());
    
    return insert(Iterator{insertPos}, key, value);
}

InsertResult LeafNode::insert(Iterator position, Bytes key, Bytes value)
{
    const auto entrySize = verifiedEntrySize(*this, key.size(), value.size());
    const auto usedBytes = bytes();
    const auto availableBytes = capacity() - usedBytes;

    if (entrySize > availableBytes) { return InsertResult::FAILED_NO_SPACE; }
    
    const auto oldSize = mValues.size();
    
    mValues.emplace(position.base(),
                    std::make_pair(Buffer{key.data(), key.size()},
                                   Buffer{value.data(), value.size()}));
    
    if (oldSize == mValues.size()) {
        throw std::invalid_argument("key exists"); // FIXME: This is broken
    }
    
    mBytes = usedBytes + entrySize;
    
    return InsertResult::INSERTED;
}

void LeafNode::splitAndInsert(Bytes key, Bytes value, LeafNode& sibling)
{
    const auto entryBytes = verifiedEntrySize(*this, key.size(), value.size());
    const auto usedBytes = bytes();
    const auto availableBytes = capacity() - usedBytes;
    const auto size = mValues.size();
    
    if (entryBytes < availableBytes || size < 4) {
        throw std::domain_error("split is not required");
    }
    
    const auto beginIt = mValues.begin();
    const auto endIt   = mValues.end();
    
    // std::advance is borked with flat_map, this works fine
    // splitLeftEndIt
    const auto splitBeginIt = beginIt + size / 2;
    
    const auto origInsertIt = std::lower_bound(
        beginIt, endIt, key, KeyCompare());
    
    auto siblingEndInserter = std::back_inserter(sibling.mValues);
    
    if (origInsertIt < splitBeginIt) { // new element goes to the left half
        std::move(beginIt, origInsertIt, siblingEndInserter);
        
        siblingEndInserter = std::make_pair(Buffer{key.data(), key.size()},
                                            Buffer{value.data(), value.size()});
        
        std::move(origInsertIt, splitBeginIt, siblingEndInserter);
        
        mValues.erase(beginIt, splitBeginIt);
        
        recordSplit(sibling, SiblingDirection::LEFT);
    } else { // new element goes to right half        
        std::move(splitBeginIt, origInsertIt, siblingEndInserter);
        
        siblingEndInserter = std::make_pair(Buffer{key.data(), key.size()},
                                            Buffer{value.data(), value.size()});
        
        std::move(origInsertIt, endIt, siblingEndInserter);
        
        mValues.erase(splitBeginIt, endIt);
        
        recordSplit(sibling, SiblingDirection::RIGHT);
    }
}

LeafNode::Iterator LeafNode::moveEntries(
    LeafNode& src, Iterator srcBegin, Iterator srcEnd,
    LeafNode& /* dst */, Iterator tgtBegin)
{
    auto srcBeginIt = srcBegin.base();
    auto srcEndIt = srcEnd.base();
    
    auto retVal = std::move(srcBeginIt, srcEndIt, tgtBegin.base());
    
    src.mValues.erase(srcBeginIt, srcEndIt);
    
    return {retVal, BufferPairToBytesPair()};
}

} } }
