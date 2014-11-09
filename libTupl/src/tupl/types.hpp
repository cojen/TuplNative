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

#ifndef _TUPL_TYPES_HPP
#define _TUPL_TYPES_HPP

#include <cassert>
#include <cstdint>
#include <utility>

namespace tupl {

typedef std::uint_least8_t byte;

class MutableBytes;

/**
   Represents an overlay over an unmanaged buffer

   @author Vishal Parakh
 */
class Bytes {
public:
    Bytes() : Bytes(nullptr, 0) {}
    Bytes(const void* data, std::size_t size);
    
    const byte* data() const { return mData; }
    std::size_t size() const { return mSize; }
    
    // MUST NOT HAVE A DESTRUCTOR, Overlay type    
private:
    const byte* mData; // NOT const to allow move/assign/swap
    std::size_t mSize; // NOT const to allow move/assign/swap
    
    friend class MutableBytes;
};

class MutableBytes final: public Bytes {
public:
    MutableBytes() : MutableBytes(nullptr, 0) {}
    MutableBytes(byte* data, std::size_t size) : Bytes(data, size) {}
    byte* data() { return const_cast<byte*>(static_cast<Bytes*>(this)->mData); }
};

inline
Bytes::Bytes(const void* data, const std::size_t size)
    : mData(static_cast<const byte*>(data)), mSize(size)
{
    assert(size > 0 ? data != nullptr : true);
    assert(data != nullptr ? size > 0 : true);
}

}

#endif 
