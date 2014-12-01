#include "Tree.hpp"
#include "Node.hpp"

#include "../make_unique.hpp"
#include "../ptrCast.hpp"

namespace tupl { namespace pvt { namespace slow {

class TreeTestBridge;

struct Tree::InsertContext {
    Tree& tree;
    const Bytes& key;
    const Bytes& value;
};

Tree::Tree() {
    mLeafNodes.emplace_back(std::make_unique<LeafNode>());
    mInternalNodes.emplace_back(std::make_unique<InternalNode>(
                                    *mLeafNodes.back()));

    mRoot = mInternalNodes.back().get();
}

LeafNode* Tree::allocateLeaf() {
    auto newLeaf = std::make_unique<LeafNode>();
    const auto newLeafRaw = newLeaf.get();
    
    mLeafNodes.emplace_back(std::move(newLeaf));
    
    return newLeafRaw;
}

InternalNode* Tree::allocateInternal() {
    auto newInternal = std::unique_ptr<InternalNode>();

    const auto newInternalRaw = newInternal.get();
    
    mInternalNodes.emplace_back(std::move(newInternal));

    return newInternalRaw;
}

void Tree::insert(Bytes key, Bytes value) {
    auto ctx = InsertContext{ *this, key, value };
    insertRecursive(*mRoot, ctx);
}

void Tree::insertRecursive(Node& node, InsertContext& ctx) {
    if (node.type() == NodeType::LEAF) {
        auto& cur = *ptrCast<LeafNode>(&node);
        const auto insertResult = cur.insert(ctx.key, ctx.value);
        
        if (insertResult != InsertResult::INSERTED) {
            assert(insertResult == InsertResult::FAILED_NO_SPACE);
            cur.splitAndInsert(ctx.key, ctx.value, *ctx.tree.allocateLeaf());
        }
    } else {
        assert(node.type() == NodeType::INTERNAL);
        
        auto& cur = *ptrCast<InternalNode>(&node);
        auto nextIt = cur.find(ctx.key);
        auto& next = *nextIt->second;
        
        insertRecursive(next, ctx);
        
        if (next.hasSibling()) {    
            const auto& split = ptrCast<InternalNode>(&next)->split();
            
            if (split.direction == SiblingDirection::RIGHT) { ++nextIt; }
            
            const auto insertResult =
                cur.insert(nextIt, split.key, *split.sibling);
            
            if (insertResult == InsertResult::FAILED_NO_SPACE) {
                cur.splitAndInsert(split.key, *split.sibling,
                                   *ctx.tree.allocateInternal());
            } else {
                assert(insertResult == InsertResult::INSERTED);
            }
        }
    }
}

} } } // namespace tupl::pvt::slow
