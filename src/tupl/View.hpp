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

#ifndef _TUPL_VIEW_HPP
#define _TUPL_VIEW_HPP

#include "Cursor.hpp"

namespace tupl {

/**
 * Mapping of keys to values, in no particular order. Subclasses and
 * implementations may specify an explicit ordering.
 *
 * @author Brian S O'Neill
 * @authro Vishal Parakh
 * @see Database
 */
class View {
public:
    /**
     * @return a new unpositioned cursor
     *
     * FIXME: This interface does not work for C++, think and replace,
     * maybe Cursor (takes a CursorImpl).
     *
     * I do not want to expose a pointer at all, all objects should be
     * copyable, pointage is an implementation detail.
     */
    virtual Cursor* newCursor() = 0;
};

}
#endif
