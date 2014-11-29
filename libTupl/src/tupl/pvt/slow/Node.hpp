#ifndef _TUPL_PVT_SLOW_NODE_HPP
#define _TUPL_PVT_SLOW_NODE_HPP

/*
  Copyright (C) 2012-2014 Brian S O'Neill
  Copyright (C) 2014      Vishal Parakh
  
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
 
      http://www.apache.org/licenses/LICENSE-2.0
 
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

/**
   Minimal implementation of the Node "interface" that is needed for a Tree
   to work

   This version does not do it's own memory management, nor does it care
   about memory layout.
   
   These Nodes are solely designed to make it possible to glue a working
   Tree implemenation together and flesh out the relationship between
   the different pieces.
 */

#include "../../types.hpp"
#include "../Latch.hpp"
#include "../CursorFrame.hpp"
#include "../Buffer.hpp"
#include "../ptrCast.hpp"

#include <cassert>
#include <vector>

#include <boost/operators.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/container/string.hpp>
#include <boost/iterator/transform_iterator.hpp>

namespace tupl { namespace pvt { namespace slow {

enum class NodeType {
    LEAF,
    INTERNAL,
    FRAGMENT,
    ROOT,
    UNDO_LOG
};

enum class SiblingDirection : bool {
    LEFT  = 0x0,
    RIGHT = 0x1,    
};

enum class InsertResult {
    FAILED_NO_SPACE,
    INSERTED,
};

class Node;

template<typename NodeT>
struct Split final {
    NodeT* sibling;
    SiblingDirection direction;
    Buffer key;
};

class Node: public pvt::Latch {
public:    
    // void bindCursorFrame(Iterator, pvt::CursorFrame);
    NodeType type() const { return mNodeType; }
    
    void recordSplit(
        Node& sibling, const SiblingDirection direction, Buffer&& splitKey)
    {
        assert(mSplit.sibling == nullptr);
        mSplit.sibling = &sibling;
        mSplit.direction = direction;
        mSplit.key = splitKey;
    }
    
    std::size_t bytes() { return mBytes; }
    
    size_t capacity() const { return mCapacity; }
    
    
    bool hasSibling() const { return mSplit.sibling != nullptr; }
    
protected:
    class Ops;
    
    Node(NodeType nodeType) :
        mNodeType(nodeType), mCapacity(4096), mBytes(0), mSplit() {}
    
private:
    typedef boost::intrusive::list_member_hook<
        boost::intrusive::link_mode<
            boost::intrusive::safe_link>> ListMemberHook;

    const NodeType mNodeType;
    const std::uint_fast16_t mCapacity;
    
protected:
    std::uint_fast16_t mBytes;    
    Split<Node> mSplit;
    
public:
    // CursorFrame's bound to this Node
    boost::intrusive::list<
        CursorFrame,
        boost::intrusive::constant_time_size<false>,
        boost::intrusive::member_hook<
            CursorFrame,
            CursorFrame::ListMemberHook,
            &CursorFrame::visitors_>
        > visitorFrames;
    
    // Do nothing right now with the next two hooks.
    // Needed for when we support eviction
    // 
    // Do not use directly, for manipluation by boost::intrusive container
    ListMemberHook mDirtyListHook_; // guarded by the DB's usage latch
    
    // Do not use directly, for manipluation by boost::intrusive container
    ListMemberHook mUsageListHook_; // guarded by the page allocator
    
    friend class ::tupl::pvt::slow::Node::Ops;
};

enum class RemoveResult {
    REMOVED,
    NOT_FOUND,    
};

class InternalNode;
class LeafNode;

class LeafNode final: public Node {
    typedef pvt::Buffer Buffer;
    typedef std::vector<std::pair<Buffer, Buffer>> ValuesMap;
    
    struct BufferPairToBytesPair {
        std::pair<Bytes, Bytes> operator()(const ValuesMap::value_type& t) const
        {
            return std::make_pair(Bytes{t.first.data(),  t.first.size()},
                                  Bytes{t.second.data(), t.second.size()});
        }
    };
    
public:
    typedef boost::transform_iterator<BufferPairToBytesPair,
                                      ValuesMap::iterator
                                      > Iterator;
    
    LeafNode() : Node(NodeType::LEAF) {}
    
    Iterator find(Bytes key);

    Iterator begin() { return { mChildren.begin(), BufferPairToBytesPair() }; }
    Iterator end()   { return { mChildren.end(), BufferPairToBytesPair() }; }
    
    InsertResult insert(Bytes key, Bytes value);
    InsertResult insert(Iterator position, Bytes key, Bytes value);
    
    std::size_t size() const { return mChildren.size(); }

    bool empty() const { return mChildren.empty(); }
    
    void splitAndInsert(Bytes key, Bytes value, LeafNode& sibling);

    const Split<LeafNode>& split() { return *ptrCast<Split<LeafNode>>(&mSplit); }
    
    static Iterator moveEntries(
        LeafNode& src, Iterator srcBegin, Iterator srcEnd,
        LeafNode& dst, Iterator tgtBegin);    
private:
    ValuesMap mChildren;
    
    friend class ::tupl::pvt::slow::Node::Ops;
};

class InternalNode final: public Node {
    typedef std::vector<std::pair<Buffer, Node*>> ChildMap;

    struct BufferKeyToBytesKeyPair {
        std::pair<Bytes, Node*> operator()(const ChildMap::value_type& t) const
        {
            return std::make_pair(Bytes{t.first.data(),  t.first.size()},
                                  t.second);
        }
    };
    
public:
    typedef boost::transform_iterator<BufferKeyToBytesKeyPair,
                                      ChildMap::iterator
                                      > Iterator;
    
    InternalNode(Node& leftestChild);
    
    // InternalNode(LeafNode& leafChild);
    
    // InternalNode(InternalNode& internalChild)
    //     : Node(NodeType::INTERNAL), mLastChild(&internalChild), mBytes(0) {}

    Iterator find(Bytes key) { throw std::logic_error("unimplemeted"); }
    
    Iterator lowerBound(Bytes key) { assert(0); }
    
    Iterator begin() { return { mChildren.begin(), BufferKeyToBytesKeyPair() };}
    Iterator end()   { return { mChildren.end(), BufferKeyToBytesKeyPair() }; }
    
    const Split<InternalNode>& split() {
        return *ptrCast<Split<InternalNode>>(&mSplit);
    }    
    
    // InsertResult insert(Iterator position, Bytes key, LeafNode& value) {
    //     return insertGeneric(position, key, value);
    // }
    
    // InsertResult insert(Iterator position, Bytes key, InternalNode& value) {
    //     return insertGeneric(position, key, value);
    // }
    
    // InsertResult insert(Bytes key, InternalNode& value) {
    //     return insertGeneric(key, value);
    // }
    
    // InsertResult insert(Bytes key, LeafNode& value) {
    //     return insertGeneric(key, value);
    // }
    
    // void splitAndInsert(Bytes key, InternalNode& value, InternalNode& sibling) {
    //     return splitAndInsertGeneric(key, value, sibling);
    // }
    
    // void splitAndInsert(Bytes key, LeafNode& value, InternalNode& sibling) {
    //     return splitAndInsertGeneric(key, value, sibling);
    // }
    
    std::size_t size() const { return mChildren.size(); }
    
    bool empty() const { return mChildren.empty(); }
    
    InsertResult insert(Iterator position, Bytes key, Node& value);
    
    InsertResult insert(Bytes key, Node& value);
    
    void splitAndInsert(Bytes key, Node& value, InternalNode& sibling);
private:
    
    ChildMap mChildren;
    
    friend class ::tupl::pvt::slow::Node::Ops;
};

} } } // namespace tupl::pvt::slow

#endif
