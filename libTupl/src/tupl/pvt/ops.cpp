#include "ops.hpp"

#include "Cursor.hpp"
#include "CursorFrame.hpp"
#include "Latch.hpp"
#include "Tree.hpp"

#include "slow/Node.hpp"

#include <algorithm>
#include <iterator>

using namespace tupl::pvt::slow;
using std::size_t;

#include "ArrayStackGeneric.ii"

namespace tupl { namespace pvt {

namespace {

/*
  Returns a iterator to the child node where key should be 
 */ 
Node* findChildNode(InternalNode& node, Bytes key) {
    return node.lowerBound(key)->second;
}

struct FindValueResult {
    Bytes value() {
        assert(false);
        return {};
    }

    bool found() { return false; }
};

FindValueResult findValue(Tree& t, LeafNode& node, Bytes key) {
    
}

bool isLeaf(const Node* const node) {
    return node->type() == NodeType::LEAF;
}

bool isSplit(const Node* const node) {
    return false;
}

template<typename T>
typename std::make_unsigned<T>::type makeUnsigned(const T& src) {
    return src;
}

}

Latch::scoped_exclusive_lock ops::unwindAndLockRoot(Tree& tree, Cursor& visitor)
{
    auto& stackFrames = visitor.stackFrames;
    
    const size_t numFrames = stackFrames.size();
    
    for (size_t i = 1; i < numFrames; ++i) {
        auto& frame = stackFrames.top();
        // FIXME: abstract this better
        const auto node = frame.node();
        auto toEraseIt = node->visitorFrames.s_iterator_to(frame); // s => static
        {
            Latch::scoped_exclusive_lock nodeLock(*node);
            node->visitorFrames.erase(toEraseIt);
        }
        stackFrames.pop();
    }
    
    auto& frame = stackFrames.top();
    const auto stackRoot = frame.node();
    auto toEraseIt = stackRoot->visitorFrames.s_iterator_to(frame);
    
    Latch::scoped_exclusive_lock rootLock(*stackRoot);
    stackRoot->visitorFrames.erase(toEraseIt);
    stackFrames.pop();
    
    //FIXME: Deal with changing height by making this a "circular" stack
    const auto root = tree.root.load(std::memory_order_acquire);
    
    if (stackRoot != root) {
        rootLock = std::move(Latch::scoped_exclusive_lock(*root));
    }
    
    return rootLock;
}

void ops::bubbleSplitUpOneLevel(Tree& t, Node& splitNodeParent, Node& splitNode)
{
    throw std::logic_error("unimplemented");
}

void ops::find(Tree& tree, Cursor& visitor, Bytes key) {
    // TODO: pre-conditions and post-conditions on these guys in
    //       the face of failure would be nice to have
    visitor.key = Buffer{key.data(), key.size()};
    // visitor.value = Bytes{};
    
    // Restore the cursor to the root
    Latch::scoped_exclusive_lock parentLock;
    
    if (visitor.stackFrames.empty()) {
        parentLock = std::move(Latch::scoped_exclusive_lock(
                                   *tree.root.load(std::memory_order_acquire)));
    } else {
        parentLock = unwindAndLockRoot(tree, visitor);
    }        
    
    Node* node = tree.root;
    assert(node);
    
    while (!isLeaf(node)) {
        const auto in = static_cast<InternalNode*>(node);        
        auto const childNode = findChildNode(*in, key);
        
        {
            Latch::scoped_exclusive_lock childLock{*childNode};
            
            if (isSplit(childNode)) {
                bubbleSplitUpOneLevel(tree, *childNode, *node);
                // TODO: pick one of the children without re-searching
                continue; // Parent lock is still held
            }
            
            // Destructor releases Parent's Latch
            std::swap(parentLock, childLock); 
        }
        
        CursorFrame frame;// TODO: bind with {*in, childIt};        
        node = childNode;
    }
    
    assert(isLeaf(node));
    assert(parentLock.owns_lock()); // leaf is safely locked
    
    const auto leaf = static_cast<LeafNode*>(node);
    const auto findResult = findValue(tree, *leaf, key);
    
    CursorFrame frame;// TODO: bind with {leaf, findResult};
}

void ops::store(Tree& t, Cursor& visitor, Bytes value) {
    // TODO: pre-check size
    if (visitor.stackFrames.empty()) {
        throw std::runtime_error("unpositioned");
    }
    
    LeafNode* const leafNode = static_cast<LeafNode*>(
        visitor.stackFrames.top().node());
    
    Latch::scoped_exclusive_lock exclusiveLeafLock(*leafNode);

    const auto key = Bytes{visitor.key.data(), visitor.key.size()};
    
    // TODO: Used saved cursor position
    const auto insertResult = leafNode->insert(leafNode->end(), key, value);
    
    if (insertResult == InsertResult::FAILED_NO_SPACE) {
        assert(insertResult == InsertResult::INSERTED);
        splitLeaf(*leafNode, leafNode->end(), key, value);
    
        // TODO: Start bubbling split upwards
    }
}

/**
  pre-conditions:  Source is latched
  post-conditions: Source is latched
*/
LeafNode* ops::splitLeaf(
    LeafNode& source, LeafNode::Iterator insertIt, Bytes key, Bytes value)
{
    // TODO: Allocate a new Node
    auto& sibling = *static_cast<LeafNode*>(nullptr);
    const auto sourceSize = source.size() + 1;
    
    assert(sourceSize >= 4);
    
    const auto medianPos = sourceSize / 2;
    
    const auto insertPos =
        makeUnsigned(std::distance(source.begin(), insertIt));
    
    const auto srcBegin   = source.begin();
    const auto srcEnd     = source.end();
    const auto siblingEnd = sibling.end();
    
    if (insertPos > medianPos) { // cleave the "right" side
        source.recordSplit(sibling, SiblingDirection::RIGHT);
        
        LeafNode::moveEntries(
            source, srcBegin + medianPos, srcBegin + insertPos,
            sibling, siblingEnd);
        
        sibling.insert(siblingEnd, key, value);
        
        LeafNode::moveEntries(source, srcBegin + insertPos, srcEnd,
                              sibling, siblingEnd);
        
    } else { // cleave the "left" side
        source.recordSplit(sibling, SiblingDirection::LEFT);
        
        LeafNode::moveEntries(source, srcBegin, srcBegin + insertPos,
                              sibling, siblingEnd);
        
        sibling.insert(siblingEnd, key, value);
        
        LeafNode::moveEntries(
            source, srcBegin + insertPos, srcBegin + medianPos,
            sibling, siblingEnd);
    }
    
    moveFrames(source, sibling, insertPos, medianPos);
    return &sibling;
}

/**
  pre-conditions:  Both source and sibling are latched
  post-conditions: Both source and sibling are latched
*/
void ops::moveFrames(Node& source, Node& sibling,
                     const size_t insertPos, const size_t splitPos)
{
    const auto srcEndIt = source.visitorFrames.end();
    const auto dstEndIt = source.visitorFrames.end();
    
    for (auto srcIt = source.visitorFrames.begin(); srcIt != srcEndIt;) {
        auto thisIt = srcIt;
        ++srcIt;
        
        const auto originalPos = thisIt->position;
        
        if (originalPos >= splitPos) {
            if (originalPos < insertPos) {
                thisIt->position = originalPos - splitPos;
            } else if (originalPos > insertPos)  {
                thisIt->position = originalPos - splitPos + 1;                
            } else { // originalPos == insertPos
                assert(false); // TODO: compare and decide which way to go
            }

            if (insertPos > splitPos) {
                sibling.visitorFrames.splice(
                    dstEndIt, source.visitorFrames, thisIt);
            }
        } else {
            if (originalPos < insertPos) {
                // nothing to do
            } else if (originalPos > insertPos)  {
                thisIt->position = originalPos + 1;                
            } else { // originalPos == insertPos
                assert(false); // TODO: compare and decide which way to go
            }
            
            if (insertPos <= splitPos) {
                sibling.visitorFrames.splice(
                    dstEndIt, source.visitorFrames, thisIt);
            }
        }
    }
}

} }

