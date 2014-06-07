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

#include <boost/intrusive/list.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/string.hpp>

namespace tupl { namespace pvt { namespace slow {

typedef Range RawBytes;

class Split {  
};

enum class NodeType {
    LEAF,
    FRAGMENT,
    INTERNAL,
    ROOT,
    UNDO_LOG
};

class BaseNode: public pvt::Latch {
public:    
    // void bindCursorFrame(Iterator, pvt::CursorFrame);
    
protected:
    BaseNode() : mCapacity(4096) {}
    
    size_t capacity() const { return mCapacity; }
    
private:
    typedef boost::intrusive::list_member_hook<
        boost::intrusive::link_mode<
            boost::intrusive::safe_link>> ListMemberHook;
    
    std::uint_fast16_t mCapacity;
    
public:    
    // CursorFrame's bound to this Node
    boost::intrusive::list<
        CursorFrame,
        boost::intrusive::constant_time_size<false>,
        boost::intrusive::member_hook<
            CursorFrame,
            CursorFrame::ListMemberHook,
            &CursorFrame::_fellowTravelers>
        > cursorFrames;
    
    // Do nothing right now with the next two hooks.
    // Needed for when we support eviction
    // 
    // Do not use directly, for manipluation by boost::intrusive container
    ListMemberHook mDirtyListHook_; // guarded by the DB's usage latch
    
    // Do not use directly, for manipluation by boost::intrusive container
    ListMemberHook mUsageListHook_; // guarded by the page allocator
};

template <NodeType nodeType>
class Node: public BaseNode {
public:
    NodeType type() const { return nodeType; }
};

struct InsertResult {
    bool inserted;
};

class RemoveResult {
};

class InternalNode;
class LeafNode;

class InternalNode: public Node<NodeType::INTERNAL> {
    typedef boost::container::basic_string<byte> Buffer;
    typedef std::map<const Buffer, Node*> ChildMap;
    
public:
    class Iterator {
        Iterator(ChildMap::iterator it) : mIt(it) {}
        const ChildMap::iterator mIt;
        friend class InternalNode;  
    };

    InternalNode() : mBytes(0) {}
    
    Iterator lowerBound(RawBytes key);
    
    Iterator begin() { return mChildren.begin(); }
    Iterator end()   { return mChildren.end(); }
    
    InsertResult insert(Iterator position, RawBytes key, Node& node);
    RemoveResult remove(RawBytes& key);

    std::size_t size() const { return mChildren.size(); }
    
private:
    std::uint_fast16_t mBytes;
    ChildMap mChildren;
};

class LeafNodeIterator;

class LeafNode: public Node<NodeType::LEAF> {
    typedef boost::container::basic_string<byte> Buffer;
    typedef std::map<const Buffer, Buffer> ValuesMap;
    
public:
    class Iterator {
        Iterator(ValuesMap::iterator it) : mIt(it) {}
        const ValuesMap::iterator mIt;
        friend class LeafNode;  
    };

    LeafNode() : mBytes(0) {}
    
    Iterator find();

    Iterator begin() { return mValues.begin(); }
    Iterator end()   { return mValues.end(); }
    
    InsertResult insert(Iterator position, RawBytes key, RawBytes value);
    RemoveResult remove(RawBytes& key);
    
    std::size_t size() const { return mValues.size(); }
    
private:
    std::uint_fast16_t mBytes;
    ValuesMap mValues;
};

} } } // namespace tupl::pvt::slow
