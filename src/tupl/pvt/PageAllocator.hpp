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

#ifndef _TUPL_PVT_PAGEALLOCATOR_HPP
#define _TUPL_PVT_PAGEALLOCATOR_HPP

#include "Latch.hpp"
#include "PageDb.hpp"

namespace tupl { namespace pvt { namespace disabled {

class Node;

/**
 * Tracks a list of pages which were allocated, allowing them to be iterated
 * over in the original order.
 *
 * @author Brian S O'Neill
 * @author Vishal Parakh
 */
class PageAllocator {
    Latch mLatch;
    
    // Linked list of dirty nodes.
    Node* mFirstDirty;
    Node* mLastDirty;
    
    // Iterator over dirty nodes.
    Node* mFlushNext;

    // TODO: Do we need to save this? Can't we just pass in a
    //       a free page?
    PageDb& mPageDbSavedForSanity;
    
public:    
    PageAllocator(PageDb& source)
        : mFirstDirty(nullptr),
          mLastDirty(nullptr),
          mFlushNext(nullptr),
          mPageDbSavedForSanity(source) {}
    
    long allocPage(Node& forNode, PageDb& source) {
        assert(&source == &mPageDbSavedForSanity);
        
        dirty(forNode);
        return source.allocPage();
    }
    
    void dirty(Node& node);
};

} } }

#endif
