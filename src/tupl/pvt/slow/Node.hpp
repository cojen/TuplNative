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

#ifndef _TUPL_PVT_SLOW_NODE_HPP
#define _TUPL_PVT_SLOW_NODE_HPP

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

#include <cassert>

#include <boost/operators.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/string.hpp>
#include <boost/iterator/transform_iterator.hpp>

namespace tupl { namespace pvt { namespace slow {

typedef Bytes RawBytes;

class Split {  
};

enum class NodeType {
    LEAF,
    FRAGMENT,
    INTERNAL,
    ROOT,
    UNDO_LOG
};

enum class SiblingDirection {
    LEFT  = 0x0,
    RIGHT = 0x1,    
};

class Node: public pvt::Latch {
public:    
    // void bindCursorFrame(Iterator, pvt::CursorFrame);
    NodeType type() const { return mNodeType; }
    
    void recordSplit(Node& sibling, const SiblingDirection direction) {
        assert(!mSibling && (&sibling));
        mSibling = &sibling;
        mSiblingDirection = direction;
    }
    
    void clearSplit() { mSibling = nullptr; }
    
protected:
    Node(NodeType nodeType) :
        mNodeType(nodeType), mMaxBytes(4096), mSibling() {}
    
    template<typename T>
    std::pair<T*, SiblingDirection> split() {
        return std::make_pair(static_cast<T*>(mSibling), mSiblingDirection);
    }
    
    size_t maxBytes() const { return mMaxBytes; }
    
private:
    typedef boost::intrusive::list_member_hook<
        boost::intrusive::link_mode<
            boost::intrusive::safe_link>> ListMemberHook;

    const NodeType mNodeType;
    const std::uint_fast16_t mMaxBytes;
    
protected:
    std::uint_fast16_t mBytes;
    
private:
    Node* mSibling;
    SiblingDirection mSiblingDirection;
    
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
    
};

enum class InsertResult {
    FAILED_NO_SPACE,
    INSERTED,
};

class RemoveResult {
};

class InternalNode;
class LeafNode;

class InternalNode final: public Node {
    typedef pvt::Buffer Buffer;
    typedef boost::container::flat_map<Buffer, Node*> ChildMap;
    
public:
    class Iterator {
    public:
        Node* node() { return mIt->second; }    
    private:        
        Iterator(ChildMap::iterator it) : mIt(it) {}
        const ChildMap::iterator mIt;
        friend class InternalNode;  
    };
    
    InternalNode(LeafNode& leafChild);
    
    InternalNode(InternalNode& internalChild)
        : Node(NodeType::INTERNAL), mLastChild(&internalChild), mBytes(0) {}
    
    Iterator lowerBound(RawBytes key);
    
    Iterator begin() { return mChildren.begin(); }
    Iterator end()   { return mChildren.end(); }
    
    InsertResult insert(Iterator position, RawBytes key, Node& node);
    RemoveResult remove(RawBytes key);

    std::size_t size() const { return mChildren.size(); }
private:
    Node* mLastChild;
    std::uint_fast16_t mBytes;
    InternalNode* mSibling;
    
    ChildMap mChildren;
};

class LeafNode final: public Node {
    typedef pvt::Buffer Buffer;
    typedef std::vector<std::pair<Buffer, Buffer>> ValuesMap;
    
    struct BufferPairToBytesPair {
        std::pair<Bytes, Bytes> operator()(const ValuesMap::value_type& t) {
            return std::make_pair(Bytes{t.first.data(),  t.first.size()},
                                  Bytes{t.second.data(), t.second.size()});
        }
    };
    
public:
    typedef boost::transform_iterator<BufferPairToBytesPair,
                                      ValuesMap::iterator,
                                      std::pair<Bytes, Bytes>&
                                      > Iterator;
    
    LeafNode() : Node(NodeType::LEAF) {}
    
    Iterator find();

    Iterator begin() { return { mValues.begin(), BufferPairToBytesPair() }; }
    Iterator end()   { return { mValues.end(), BufferPairToBytesPair() }; }
    
    InsertResult insert(Iterator position, RawBytes key, RawBytes value);
    
    RemoveResult remove(RawBytes key);
    
    std::size_t size() const { return mValues.size(); }
    
    std::pair<LeafNode*, SiblingDirection> split() {
        return Node::split<LeafNode>();
    }
    
    static Iterator moveEntries(
        LeafNode& src, Iterator srcBegin, Iterator srcEnd,
        LeafNode& dst, Iterator tgtBegin);    
private:
    ValuesMap mValues;
};

inline
InternalNode::InternalNode(LeafNode& leafChild)
    : Node(NodeType::INTERNAL), mLastChild(&leafChild), mBytes(0)
{
}

} } } // namespace tupl::pvt::slow

#endif
