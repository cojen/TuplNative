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

#ifndef _TUPL_CURSOR_HPP
#define _TUPL_CURSOR_HPP

#include "types.hpp"

#include <utility>

namespace tupl {

/**
 * Maintains a logical position in a {@link View}. Cursor instances can only be
 * safely used by one thread at a time, and they must be {@link #reset reset}
 * when no longer needed. Instances can be exchanged by threads, as long as a
 * happens-before relationship is established. Without proper exclusion,
 * multiple threads interacting with a Cursor instance may cause database
 * corruption.
 *
 * @author Brian S O'Neill
 * @see View#newCursor View.newCursor
 */
class Cursor {
public:
    typedef std::pair<const byte*, const std::size_t> Range;
    
    /**
     * Empty marker which indicates that value exists but has not been {@link
     * #load loaded}.
     */
    const static Range NOT_LOADED;

    /**
     * Returns an uncopied reference to the current key, or null if Cursor is
     * unpositioned. Array contents must not be modified.
     */
    virtual Range key() const = 0;
    
    /**
     * Returns an uncopied reference to the current value, which might be null
     * or {@link #NOT_LOADED}. Array contents can be safely modified.
     */
    virtual Range value() const = 0;
    
    /**
     * Moves the Cursor to find the given key.
     *
     * <p>Ownership of the key instance transfers to the Cursor, and it must
     * not be modified after calling this method.
     * @throws NullPointerException if key is null
     */
    virtual void find(Range key) = 0;
    
    /**
     * Stores a value into the current entry, leaving the position
     * unchanged. An entry may be inserted, updated or deleted by this
     * method. A null value deletes the entry. Unless an exception is thrown,
     * the object returned by the {@link #value value} method will be the same
     * instance as was provided to this method.
     *
     * @param value value to store; pass null to delete
     * @throws IllegalStateException if position is undefined at invocation time
     * @throws ViewConstraintException if value is not permitted
     */
    virtual void store(Range value) = 0;
    
    /**
     * Resets Cursor and moves it to an undefined position. The key and value references are
     * also cleared.
     */
    virtual void reset() = 0;
};

}

#endif
