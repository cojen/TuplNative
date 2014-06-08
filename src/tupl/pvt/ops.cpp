#include "ops.hpp"

#include "Cursor.hpp"
#include "CursorFrame.hpp"
#include "Latch.hpp"
#include "Tree.hpp"

#include "slow/Node.hpp"

using namespace tupl::pvt::slow;

#include "ArrayStackGeneric.ii"

namespace tupl { namespace pvt {

namespace {

/*
  Returns a iterator to the child node where key should be 
 */ 
Node* findChildNode(InternalNode& node, Bytes key) {
    return node.lowerBound(key).node();
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

}

Latch::scoped_exclusive_lock ops::unwindAndLockRoot(Tree& tree, Cursor& visitor)
{
    auto& stackFrames = visitor.stackFrames;
    
    const size_t numFrames = stackFrames.size();
    
    for (size_t i = 1; i < numFrames; ++i) {
        auto& frame = stackFrames.top();
        // FIXME: abstract this better
        const auto node = frame.node.load(std::memory_order_acquire);
        auto toEraseIt = node->cursorFrames.s_iterator_to(frame); // s => static
        {
            Latch::scoped_exclusive_lock nodeLock(*node);
            node->cursorFrames.erase(toEraseIt);
        }
        stackFrames.pop();
    }
    
    auto& frame = stackFrames.top();
    const auto stackRoot = frame.node.load(std::memory_order_acquire);
    auto toEraseIt = stackRoot->cursorFrames.s_iterator_to(frame);
    
    Latch::scoped_exclusive_lock rootLock(*stackRoot);
    stackRoot->cursorFrames.erase(toEraseIt);
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
        visitor.stackFrames.top().node.load(std::memory_order_acquire));
    
    Latch::scoped_exclusive_lock exclusiveLeafLock(*leafNode);
    
    // TODO: Used saved cursor position
    leafNode->insert(
        leafNode->end(), Bytes{visitor.key.data(), visitor.key.size()}, value);
}

} }
