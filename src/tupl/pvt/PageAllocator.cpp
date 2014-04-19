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

#include "PageAllocator.hpp"

#include "Node.hpp"

namespace tupl { namespace pvt {

void PageAllocator::dirty(Node& nodeRef) {
    Node* const node = &nodeRef;
        
    std::lock_guard<Latch> exclusiveLock(mLatch);

    Node* const next = node->nextDirty;
    Node* const prev = node->prevDirty;
        
    if (next != nullptr) {
        if ((next->prevDirty = prev) == nullptr) {
            mFirstDirty = next;
        } else {
            prev->nextDirty = next;
        }
        node->nextDirty = nullptr;
        (node->prevDirty = mLastDirty)->nextDirty = node;
        
    } else if (prev == nullptr) {
        Node* last = mLastDirty;
        
        if (last == node) { return; }
        
        if (last == nullptr) {
            mFirstDirty = node;
        } else {
            node->prevDirty = last;
            last->nextDirty = node;
        }
    }
    
    mLastDirty = node;
    
    // See flushNextDirtyNode for explanation for node latch requirement.
    if (mFlushNext == node) { mFlushNext = next; }
}

} }
