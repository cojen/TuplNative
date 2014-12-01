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

#ifndef _TUPL_PVT_CURSOR_HPP
#define _TUPL_PVT_CURSOR_HPP

#include "ArrayDeque.hpp"
#include "CursorFrame.hpp"
#include "Buffer.hpp"

namespace tupl {

class Tree;

}

namespace tupl { namespace pvt {

class ops;

/**
   @author Vishal Parakh
 */
class Cursor final {
    // TODO: This class might not be able to be private because we need to leak
    // knowledge of the CursorFrame stack to the caller
    //
    // Consider making Cursor an inner-class of Tree
public:    
    /**
      Not copyable.
      
      TODO: This may need to be revised. A situation where a second cursor lives
            at the same spot in the tree might be desriable.
     */ 
    Cursor(const Cursor& c) = delete;
    
    /**
       @see reset()
     */
    ~Cursor();
    
private:    
    // Lower case because these two are part of the interface exposed to ops;
    Buffer key;    
    ArrayDeque<CursorFrame, 64> stackFrames;
    
    friend class ops;
};

} }

#endif
