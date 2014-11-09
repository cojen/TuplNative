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

#ifndef _TUPL_PVT_ARRAYSTACKGENERIC_HPP
#define _TUPL_PVT_ARRAYSTACKGENERIC_HPP

#include <type_traits>

namespace tupl { namespace pvt {

template<typename T, std::size_t N>
class ArrayStackGeneric {
public:
    ArrayStackGeneric();
    ~ArrayStackGeneric();
    
    void emplaceBack(T&& val);
    
    void pop();
    
    const T& top() const;
    
    T& top();
    
    bool empty() const;
    
    bool full() const;
    
    std::size_t size() const;
    
    void clear();

    /*
      Disable copy construction
    */
    ArrayStackGeneric(const ArrayStackGeneric&) = delete;
    
    /*
      Disable copy assignment
    */
    ArrayStackGeneric& operator=(const ArrayStackGeneric&) = delete;
    
    /*
      Disable move construction
    */
    ArrayStackGeneric(const ArrayStackGeneric&&) = delete;
    
    /*
      Disable move assignment
    */
    ArrayStackGeneric& operator=(const ArrayStackGeneric&&) = delete;
    
private:
    T* mTop;
    std::aligned_storage<sizeof(T), alignof(T)> mElems[N];
};

// typedef ArrayStackGeneric<CursorFrame, 64u> CursorFrameStack;

} } // namespace tupl::pvt

#endif
