/*
  Copyright (C) 2012-2014 Brian S O'Neill
  Copyright (C) 2014 Vishal Parakh
  
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
 
  http://www.apache.org/licenses/LICENSE-2.0
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */
#ifndef _TUPL_PVT_LATCH_HPP
#define _TUPL_PVT_LATCH_HPP

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace tupl { namespace pvt {

/**
   TODO Requirements:
   
   Non-reentrant read/write lock, using unfair acquisition. Implementation
   need not track thread ownership or check for illegal usage.
 */
class Latch: public boost::shared_mutex {
public:
    typedef boost::unique_lock<boost::shared_mutex> scoped_exclusive_lock;
    typedef boost::shared_lock<boost::shared_mutex> scoped_shared_lock;
};

} }

#endif
