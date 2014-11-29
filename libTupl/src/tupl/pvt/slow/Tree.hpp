#ifndef _TUPL_PVT_SLOW_TREE_HPP
#define _TUPL_PVT_SLOW_TREE_HPP
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

#include "Node.hpp"
#include <vector>

namespace tupl { namespace pvt { namespace slow {

class TreeTestBridge;

class Tree {
public:
    Tree();
    void insert(Bytes key, Bytes value);
    std::pair<Bytes, bool> find(Bytes key);
    
private:
    struct InsertContext;
    
    static void insertRecursive(Node& cur, InsertContext& ctx);

    InternalNode* allocateInternal();
    LeafNode* allocateLeaf();
    
    InternalNode* mRoot;
    std::vector<std::unique_ptr<LeafNode>>     mLeafNodes;
    std::vector<std::unique_ptr<InternalNode>> mInternalNodes;

    friend class ::tupl::pvt::slow::TreeTestBridge;
};

} } } // namespace tupl::pvt::slow

#endif
