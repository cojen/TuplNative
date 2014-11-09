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

#ifndef _TUPL_MALLOCALLOCATOR_HPP
#define _TUPL_MALLOCALLOCATOR_HPP

#include "Allocator.hpp"

namespace tupl {

/*  
  Wrapper around std::malloc and std::free
 */ 
class MallocAllocator final: public Allocator {
public:
    void* alloc(std::size_t size) override;
    
    void* realloc(std::size_t newSize,
                  void* oldMem, std::size_t  /* oldSize */) override;
    
    void  free(void* mem, std::size_t /* size */) override;
};

}
#endif 
