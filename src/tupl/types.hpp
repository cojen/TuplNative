/*
 *  Copyright (C) 2012-2014 Brian S O'Neill
 *  Copyright (C) 2014 Vishal Parakh
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef _TUPL_TYPES_HPP
#define _TUPL_TYPES_HPP

#include <cstdint>
#include <utility>

namespace tupl {

typedef std::uint8_t byte;

class Range {
public:
    Range(byte* data, std::size_t size);
    
    const byte* data() const;
    byte* mutableData();
    std::size_t size() const;
    
private:
    byte* const mData;
    const std::size_t mSize;
};

inline
Range::Range(byte* data, const std::size_t size) : mData(data), mSize(size) {
}

}

#endif 
