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

#ifndef _TUPL_LARGEKEYEXCEPTION_HPP
#define _TUPL_LARGEKEYEXCEPTION_HPP

#include "DatabaseError.hpp"

namespace tupl {

/**
 * Thrown when a key is too large to fit into a page. Maximum key size is
 * defined as: {@code min(16383, (pageSize / 2) - 22)}. When using the default
 * page size of 4096 bytes, the maximum key size is 2026 bytes.
 *
 * @author Brian S O'Neill
 * @author Vishal Parakh
 */
class LargeKeyError: public DatabaseError {
public:
    LargeKeyError(const size_t length) {
        // FIXME: bubble up
        // super("Key is too large: " + length);
    }
};

}

#endif
