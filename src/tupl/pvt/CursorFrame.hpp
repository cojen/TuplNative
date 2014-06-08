/*
  Copyright (C) 2012-2014 Brian S O'Neill
  Copyright (C) 2014 Vishal Parakh
  
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

#ifndef _TUPL_PVT_CURSORFRAME_HPP
#define _TUPL_PVT_CURSORFRAME_HPP

#include <boost/intrusive/list.hpp>

#include <atomic>

#include "../types.hpp"
#include "types.hpp"

namespace tupl { namespace pvt {

namespace slow { class Node; }

class ops;

/**
   @author Vishal Parakh
 */
class CursorFrame final {
    typedef slow::Node Node;
    
    std::atomic<Node*>    node;
    
    // Used to keep track of which key we're visiting
    MaxAlignT position[128 / sizeof(MaxAlignT)];
    
    Bytes   notFoundKey; // TODO: Who owns me??
public:
    // Used to keep track of all the Cursor's visiting a Node
    // 
    // Use a safe-link during development. In the long-term,
    // this can be revised to use a normal link
    // 
    // Guarded by the Latch guarding the node that this Frame is visiting
    // 
    // Do not use directly, for manipluation by boost::intrusive container
    typedef boost::intrusive::list_member_hook<
        boost::intrusive::link_mode<
            boost::intrusive::safe_link>> ListMemberHook;

    ListMemberHook visitors_;

    friend class ops;
};

} }
#endif
