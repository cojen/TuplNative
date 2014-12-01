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
  set(CMAKE_CXX_FLAGS "-std=gnu++11 -Wall -Wextra -pedantic")
endif()

# TODO: For compatability, we're going to have to build seperate 
#       static and shared libs with different names (for Windows).
#    
#       This works for now
set(BUILD_SHARED_LIBS true)

if(WIN32)
    set(BUILD_SHARED_LIBS false)
    add_definitions(-DBOOST_ALL_DYN_LINK=1)
endif(WIN32)

enable_testing()

FIND_PACKAGE(Boost COMPONENTS system thread unit_test_framework REQUIRED)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY_DIR})
ENDIF()

add_subdirectory (libTupl)
