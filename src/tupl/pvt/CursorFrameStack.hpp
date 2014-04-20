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

#ifndef _TUPL_PVT_CURSORFRAMESTACK_HPP
#define _TUPL_PVT_CURSORFRAMESTACK_HPP

#include "../types.hpp"

#include <vector>

namespace tupl { namespace pvt {

class CursorFrame {
public:
    friend bool operator==(const CursorFrame& l, const CursorFrame& r) {
        // FIXME: Implement this
        return &l == &r;
    }
};

class CursorFrameStack {
    typedef std::vector<CursorFrame>::size_type size_type;    
public:    
    // TODO: Make into a stronger type to prevent accidental
    //       conversion. BOOST_STRONG_TYPEDEF maybe?
    typedef size_type FrameId;
    
    CursorFrameStack() = default;
    
    /*
      Disable copy construction
    */
    CursorFrameStack(const CursorFrameStack&) = delete;
    
    /*
      Disable copy assignment
    */
    CursorFrameStack& operator=(const CursorFrameStack&) = delete;
    
    void remove(FrameId id);
    
    FrameId emplaceBack(CursorFrame&& frame);
    
    const CursorFrame& get(FrameId id) const;
    
    bool empty() const;
    
private:
    // Invariant: The last (top) element is never a tombstone
    std::vector<CursorFrame> mCursorFrames;
};

} } // namespace tupl::pvt
#endif
