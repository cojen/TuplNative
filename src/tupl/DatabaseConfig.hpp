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

#ifndef _TUPL_DATABASECONFIG_HPP
#define _TUPL_DATABASECONFIG_HPP

#include <cstdint>

namespace tupl {

/**
 * Configuration options used when {@link Database#open opening} a database.
 *
 * @author Brian S O'Neill
 * @author Vishal Parakh
 */
class DatabaseConfig {
    typedef std::size_t size_t;
    
    size_t mPageSize;
    size_t mMinCachedBytes;
    size_t mMaxCachedBytes;

public:
    DatabaseConfig() :
        mPageSize(4096),
        mMinCachedBytes( 1 * 1024 * 1024),
        mMaxCachedBytes(16 * 1024 * 1024) {}
    
    /**
     * Set the minimum cache size, overriding the default.
     *
     * @param minBytes cache size, in bytes
     */
    DatabaseConfig& minCacheSize(const size_t minBytes) {
        mMinCachedBytes = minBytes;
        return *this;
    }
    
    /**
     * Set the maximum cache size, overriding the default.
     *
     * @param maxBytes cache size, in bytes
     */
    DatabaseConfig& maxCacheSize(const size_t maxBytes) {
        mMaxCachedBytes = maxBytes;
        return *this;
    }

    /**
     * Set the page size, which is 4096 bytes by default.
     */
    DatabaseConfig& pageSize(const size_t size) {
        mPageSize = size;
        return *this;
    }
};

}
#endif
