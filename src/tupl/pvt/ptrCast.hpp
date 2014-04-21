/*
  Copyright (C) 2014      Vishal Parakh
  
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

#ifndef _TUPL_PVT_PTRCAST_HPP
#define _TUPL_PVT_PTRCAST_HPP

namespace tupl { namespace pvt {

/*
  Reintpret pointer conversion that does not trigger GCC's
  aliasing warnings
 */ 
template<typename D>
D* ptrCast(void* src) {
    return static_cast<D*>(src);
}

/*
  Reintpret pointer conversion that does not trigger GCC's
  aliasing warnings
 */ 
template<typename D>
const D* ptrCast(const void* src) {
    return static_cast<D*>(src);
}

} }

#endif
