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
    Latch::scoped_exclusive_lock nodeLock; // Optimized for NVRO
    
    auto& stackFrames = visitor.stackFrames;
    
    Node* node = nullptr;

    while (!stackFrames.empty()) {
        auto& frame = stackFrames.top();
        // FIXME: abstract this better
        node = frame.node.load(std::memory_order_relaxed);
        nodeLock = std::move(Latch::scoped_exclusive_lock(*node));
        node->cursorFrames.erase(node->cursorFrames.iterator_to(frame));
        stackFrames.pop();
    }
    
    assert(nodeLock.owns_lock());

    Node* const root = tree.root.load(std::memory_order_relaxed);
    
    if (node != root) {
        nodeLock = std::move(Latch::scoped_exclusive_lock(*root));
    }
    
    return nodeLock;
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
                                   *tree.root.load(std::memory_order_relaxed)));
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
