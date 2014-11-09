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

#ifndef _TUPL_PVT_NONPAGEDB_HPP
#define _TUPL_PVT_NONPAGEDB_HPP

#include "PageDb.hpp"

namespace tupl { namespace pvt {

class NonPageDb: public PageDb {
    const size_t mPageSize;
    
public:
    NonPageDb(size_t pageSize): mPageSize (pageSize) {}
    size_t pageSize() const override { return mPageSize; }
    long allocPage()  override { return 2; }
};

} }

#endif 
