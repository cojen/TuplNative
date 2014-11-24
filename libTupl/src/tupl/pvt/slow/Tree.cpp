#include "Node.hpp"

#include "../make_unique.hpp"
#include "../ptrCast.hpp"

namespace tupl { namespace pvt { namespace slow {

class Tree {
public:
    Tree();
    void insert(Bytes key, Bytes value);
    std::pair<Bytes, bool> find(Bytes key);
    
private:
    struct InsertContext {
        Tree& tree;
        const Bytes& key;
        const Bytes& value;
    };
    
    static void insertRecursive(Node& cur, InsertContext& ctx);

    InternalNode* allocateInternal();
    LeafNode* allocateLeaf();
    
    InternalNode* mRoot;
    std::vector<std::unique_ptr<LeafNode>>     mLeafNodes;
    std::vector<std::unique_ptr<InternalNode>> mInternalNodes;
};

Tree::Tree() {
    mLeafNodes.emplace_back(std::make_unique<LeafNode>());
    mInternalNodes.emplace_back(std::make_unique<InternalNode>(
                                    *mLeafNodes.back()));

    mRoot = mInternalNodes.back().get();
}

LeafNode* Tree::allocateLeaf() {
    auto newLeaf = make_unique<LeafNode>();
    const auto newLeafRaw = newLeaf.get();
    
    mLeafNodes.emplace_back(std::move(newLeaf));
    
    return newLeafRaw;
}

InternalNode* Tree::allocateInternal() {
    auto newInternal = std::unique_ptr<InternalNode>();
        // make_unique<InternalNode>();

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
        auto next = nextIt->second;
        
        insertRecursive(*next, ctx);
        
        // auto split = next.split();
        
        // if (split.siblingDirection() == SiblingDirection::RIGHT) {
        //     ++nextIt;
        // }
        
        // const auto insertResult = cur.insert(nextIt, key, sibling);
        
        // if (insertResult != InsertResult::INSERTED) {
        //     assert(insertResult == InsertResult::FAILED_NO_SPACE);
        //     cur.splitAndInsert(ctx.key, key, sibling);
        // }
    }
}


} } } // namespace tupl::pvt::slow
