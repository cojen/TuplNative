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

#ifndef _TUPL_PVT_PAGEDB_H
#define _TUPL_PVT_PAGEDB_H

#include <mutex>

namespace tupl { namespace pvt {

/**
 * @author Brian S O'Neill
 * @author Vishal Parakh
 * 
 * @see DurablePageDb
 * @see NonPageDb
 */
class PageDb {
public:
    class SharedMutex {
    public:
        SharedMutex();
        SharedMutex(const SharedMutex&) = delete;
        SharedMutex& operator=(const SharedMutex&) = delete;        
    private:
        std::recursive_mutex mMutex;
    };
    
private:
    // Need to use a reentrant lock instead of a latch to simplify the
    // logic for persisting in-flight undo logs during a checkpoint. Pages
    // might need to be allocated during this time, and so reentrancy is
    // required to avoid deadlock. Ideally, lock should be fair in order
    // for exclusive lock request to de-prioritize itself by timing out and
    // retrying. See Database.checkpoint. Being fair slows down overall
    // performance, because it increases the cost of acquiring the shared
    // lock. For this reason, it isn't fair.    
    SharedMutex mMutex;
    
public:
    /**
     * Returns the fixed size of all pages in the store, in bytes.
     */
    virtual size_t pageSize() const = 0;
    
    /**
     * Allocates a page to be written to.
     *
     * @throws system_error on failure 
     * @return page id; never zero or one
     */
    virtual long allocPage() = 0;
    
    /**
     * Commit lock. Holding the shared lock prevents commits.
     */
    SharedMutex& commitLock() { return mMutex; }
    PageDb() {}
    
    PageDb(const PageDb&) = delete; /// Noncopyable
    PageDb& operator=(const PageDb&) = delete; /// Noncopyable
};
    
} } // namepsace tupl::pvt

#endif
