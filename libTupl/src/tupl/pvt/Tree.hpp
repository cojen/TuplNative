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

#ifndef _TUPL_PVT_TREE_HPP
#define _TUPL_PVT_TREE_HPP

#include <atomic>

// TODO: clean this up and make the Node a proper duck-type
#include "slow/Node.hpp"

namespace tupl { namespace pvt {

/**
  Represents a B+Tree.
  
  @author Vishal Parakh
 */
class Tree final {
    /*
      Represents a B+Tree. Based on and compatible with the original Tupl Java
      implementation.
      
      The present implementation treat splits differently than the Java
      implemenation. Specifically:
      
      1. The root is never split, if the root overflows it the height of the
         tree is immediately increased without releasing the root node
         exclusive latch.
         
      2. Split nodes are repaired on first contact, never followed.
      
      The preceding assumptions code follow splits unnecessary at the cost
      of reduced insert/find concurrency.
      
      The thread that created the split is still responsible for repairing the
      split or saving a reference to it for future cleanup.
    */
    
    std::atomic<slow::InternalNode*> root;
    friend class ops;
};

} }

#endif
