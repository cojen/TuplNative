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

#include "Node.hpp"

#include "../types.hpp"

#include <algorithm>
#include <cassert>

using std::size_t;
using std::pair;
using std::make_pair;

namespace tupl { namespace pvt {

namespace {

template<typename V, typename T>
class OffsetPointerGeneric {
public:
    typedef T OffsetType;
    typedef V ValueType;
    
    explicit OffsetPointerGeneric(T offset) : mOffset(offset) {}
    
    const ValueType& withBase(const void* baseAddr) const {
        const byte* const addr = static_cast<const byte*>(baseAddr) + mOffset;
        return *static_cast<const ValueType*>(static_cast<const void*>(addr));
    }
    
    OffsetType offset() const { return mOffset; }
    
private:
    static T encode(T native) { return native;  /* todo: !BigEndian */ }
    static T decode(T encoded) { return encoded; /* todo: !BigEndian */ }
    
    T mOffset;
};

class EmbeddedValue {
public:
    const Range value() const { return Range{nullptr, 0}; }
};

typedef OffsetPointerGeneric<EmbeddedValue, uint16_t> KeyOffsetPtr;

class SearchVector {
public:
    const KeyOffsetPtr* begin() const { return nullptr; }
    const KeyOffsetPtr* end() const   { return nullptr; }
private:
    
};

class BottomInternalNode {
public:
    SearchVector searchVec() const { return SearchVector(); }
};

class KeyComparator {
public:
    class State {
    public:
        State(const BottomInternalNode* n) :
            node(n), hiPrefixSize(0), loPrefixSize(0), mEqual(false) {}

        bool equal() const { return mEqual; }
        
    private:
        Range extractRange(const KeyOffsetPtr& offsetPtr) const {
            return offsetPtr.withBase(node).value();
        }

        // FIXME: Specialize Node comparator and remove from KeyComparator
        const BottomInternalNode* const node;
        
        size_t hiPrefixSize;
        size_t loPrefixSize;
        
        bool   mEqual;
        
        friend class KeyComparator;
    };
    
    explicit KeyComparator(State& state) : mState(state) {}
   
    bool operator()(const Range& l, const KeyOffsetPtr& rPtr) {
        return compareLower(l, mState.extractRange(rPtr), mState);
    }
    
    bool operator()(const KeyOffsetPtr& lPtr, const Range& r) {
        return compareLower(mState.extractRange(lPtr), r, mState);
    }

    bool operator()(const Range& l, const Range& r) {
        State scratch(0);
        return compareLower(l, r, scratch);
    }
    
    static bool compareLower(const Range& l, const Range& r, State& st) {
        const byte* const lPtr = l.data();
        const byte* const rPtr = r.data();
        
        const size_t cmpSize  = std::min(l.size(), r.size());
        const size_t prefixSize = std::min(st.loPrefixSize, st.hiPrefixSize);

        assert(prefixSize < cmpSize);
        
        const auto mismatched = std::mismatch(
            lPtr + prefixSize, lPtr + cmpSize, rPtr + prefixSize);
        
        if (mismatched.first == (lPtr + cmpSize)) {
            // No mismatch, is lower if size is lower
            if (l.size() == r.size()) { st.mEqual = true; }
            return l.size() < r.size(); // => "ab" < "abc"
        } else if (*mismatched.first < *mismatched.second) {
            st.loPrefixSize = mismatched.first - lPtr;
            return true; // => "abb" < "abc"
        } else {
            st.hiPrefixSize = mismatched.first - lPtr;
            return false; // => !("abc" < "abb")
        }   
    }
    
private:
    State& mState;
};

/**
 * 
 */
pair<const KeyOffsetPtr*, bool> binarySearch(
    const BottomInternalNode* node,
    const KeyOffsetPtr* begin, const KeyOffsetPtr* end, const Range key)
{
    // TODO: These should be part of a standard verification utility
    assert(node && begin && end && key.data() && key.size());
    assert(begin < end);
    assert(begin >= node->searchVec().begin());
    assert(end   <= node->searchVec().end());
    
    KeyComparator::State comparatorState(node);
    KeyComparator comparator(comparatorState);
    
    return make_pair(std::lower_bound(begin, end, key, comparator),
                     comparatorState.equal());
}

} } } // namespace tupl::pvt::(anonymous)

namespace tupl { namespace pvt {

Node::Node():
    mMoreUsed(nullptr),
    mLessUsed(nullptr),
    mNextDirty(nullptr),
    mPrevDirty(nullptr),
    mLastCursorFrame(nullptr),
    mPage(nullptr)
{
}

} } 
