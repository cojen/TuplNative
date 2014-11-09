/*
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

#ifndef _TUPL_ALLOCATOR_HPP
#define _TUPL_ALLOCATOR_HPP

#include <cstddef>

namespace tupl {
/*
  Interface to a low-level allocator
  
  Unlike std::malloc and std::free, implementations are not
  required to keep track of the size of the allocation.
 */ 
class Allocator {
public:
    virtual void* alloc(std::size_t size) = 0;
    
    virtual void* realloc(size_t newSize,
                          void* oldMem, std::size_t oldSize) = 0;
    
    virtual void  free(void* mem, std::size_t size) = 0;
    
    virtual ~Allocator();
};

} // namespace tupl
#endif 
