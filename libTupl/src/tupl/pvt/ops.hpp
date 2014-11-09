/*
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

#ifndef _TUPL_PVT_OPS_HPP
#define _TUPL_PVT_OPS_HPP

#include "Latch.hpp"
#include "slow/Node.hpp"

namespace tupl {

class Bytes;

}

namespace tupl { namespace pvt {

class Cursor;
class CursorFrame;
class Tree;

/**
   Class that encapsulates all the operations that need to work in conjunction
   with instances of the Tree, Nodes, Cursor, CursorFrames classes
   
   This class name is lower case to capture the intent that this is effectively
   an access controlled namespace.
 */
class ops final {
public:    
    static void find(Tree& t, Cursor& visitor, Bytes key);
    
private:
    ops() = delete;
    
    typedef slow::Node Node;
    typedef slow::LeafNode LeafNode;
    
    static Latch::scoped_exclusive_lock unwindAndLockRoot(
        Tree& tree, Cursor& visitor);
    
    static void bubbleSplitUpOneLevel(
        Tree& t, Node& splitNodeParent, Node& splitNode);
    
    static void store(Tree& t, Cursor& visitor, Bytes value);
    
    static LeafNode* splitLeaf(LeafNode& source, LeafNode::Iterator insertPos,
                               Bytes key, Bytes value);
    
    static void moveFrames(Node& source, Node& sibling,
                           std::size_t insertPos, std::size_t splitPos);
    
    template<typename Node>
    static void split(Node& source, Node& sibling);
};

} }

#endif
