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

include_directories(src)

file(GLOB_RECURSE Sources FOLLOW_SYMLINKS src/*.cpp)

add_library(Tupl ${Sources})

target_link_libraries(Tupl
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  )

file(GLOB_RECURSE TestSources FOLLOW_SYMLINKS test/*.cpp)
foreach(TestSource ${TestSources})
  get_filename_component(TEST_NAME ${TestSource} NAME_WE)
  add_executable(${TEST_NAME} ${TestSource})
  add_test(${TEST_NAME} ${TEST_NAME})  
  set_target_properties(${TEST_NAME} PROPERTIES COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)
  
  target_link_libraries(${TEST_NAME}
    Tupl
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
endforeach()
