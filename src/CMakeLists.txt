# Copyright (C) 2014 Vishal Parakh
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required (VERSION 2.8)

project (Tupl)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
  set(CMAKE_CXX_FLAGS "-std=gnu++11 -Wextra -pedantic")
endif()

FIND_PACKAGE(Boost)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ENDIF()

aux_source_directory(tupl     PubSources)
aux_source_directory(tupl/pvt PvtSources)

add_library(tupl ${PubSources} ${PvtSources})

