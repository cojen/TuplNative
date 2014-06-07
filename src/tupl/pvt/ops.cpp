#include "ops.hpp"

#include "Cursor.hpp"
#include "CursorFrame.hpp"
#include "Latch.hpp"
#include "Node.hpp"
#include "Tree.hpp"

#include "ArrayStackGeneric.ii"

namespace tupl { namespace pvt {

namespace {

// TODO: In reality the iterator is an interator to a pointer.
//       I don't want to expose too many pointers. 
std::vector<Node*> gEmpty;

/*
   Empty class, just a place-holder for a type so that
   overloads work correctly.
   
   Do not add any members or methods here. This class exists only
   for clarity.
 */
class InternalNode final : public Node {};

/*
   Empty class, just a place-holder for a type so that
   overloads work correctly
   
   Do not add any members or methods here. This class exists only
   for clarity.
 */
class LeafNode final : public Node {};

/*
  Returns a iterator to the child node where key should be 
 */ 
Node::ChildIterator findChildNode(InternalNode& node, const Range& key) {
    return gEmpty.end();
}

struct FindValueResult {
    Range value() {
        assert(false);
        return {};
    }

    bool found() { return false; }
};

FindValueResult findValue(Tree& t, LeafNode& node, const Range& key) {
    
}

}

Latch::scoped_exclusive_lock ops::unwindAndLockRoot(Tree& tree, Cursor& visitor)
{
    auto& stackFrames = visitor.stackFrames;
    
    const size_t numFrames = stackFrames.size();
    
    for (size_t i = 1; i < numFrames; ++i) {
        auto& frame = stackFrames.top();
        // FIXME: abstract this better
        Node* const node = frame.node.load(std::memory_order_acquire);
        auto toEraseIt = node->cursorFrames.s_iterator_to(frame); // s => static
        {
            Latch::scoped_exclusive_lock nodeLock(*node);
            node->cursorFrames.erase(toEraseIt);
        }
        stackFrames.pop();
    }
    
    auto& frame = stackFrames.top();
    Node* const stackRoot = frame.node.load(std::memory_order_acquire);
    auto toEraseIt = stackRoot->cursorFrames.s_iterator_to(frame);
    
    Latch::scoped_exclusive_lock rootLock(*stackRoot);
    stackRoot->cursorFrames.erase(toEraseIt);
    stackFrames.pop();
    
    //FIXME: Deal with changing height by making this a "circular" stack
    Node* const root = tree.root.load(std::memory_order_acquire);
    
    if (stackRoot != root) {
        rootLock = std::move(Latch::scoped_exclusive_lock(*root));
    }
    
    return rootLock;
}

void ops::bubbleSplitUpOneLevel(Tree& t, Node& splitNodeParent, Node& splitNode)
{
    
}

void ops::find(Tree& tree, Cursor& visitor, const Range& key) {
    // TODO: pre-conditions and post-conditions on these guys in
    //       the face of failure would be nice to have
    visitor.key = key;
    visitor.value = Range{};
    
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
    
    while (!node->isLeaf()) {
        const auto in = static_cast<InternalNode*>(node);        
        const auto childIt = findChildNode(*in, key);

        Node* const childNode = *childIt;
        
        {
            Latch::scoped_exclusive_lock childLock{**childIt};
            
            if (childNode->split()) {
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
    
    assert(node->isLeaf());
    assert(parentLock.owns_lock()); // leaf is safely locked
    
    const auto leaf = static_cast<LeafNode*>(node);    
    const auto findResult = findValue(tree, *leaf, key);
    
    CursorFrame frame;// TODO: bind with {leaf, findResult};
}

} }
