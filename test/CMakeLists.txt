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


include_directories (${TEST_SOURCE_DIR}/src)
# include(CMakePrintHelpers)

add_definitions(-DBOOST_TEST_DYN_LINK)

aux_source_directory(tupl TuplTestSource)
foreach(SrcFile ${TuplTestSource})
  
  add_executable(${SrcFile} ${SrcFile})
  
  target_link_libraries(${SrcFile}
    Tupl
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

  add_test(TuplTests ${SrcFile})
endforeach()

aux_source_directory(tupl/pvt TuplPvtTestSources)
foreach(SrcFile ${TuplPvtTestSources})
  add_executable(${SrcFile} ${SrcFile})
  
  target_link_libraries(${SrcFile}
    Tupl
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
  
  add_test(TuplPvtTestSources ${SrcFile})
endforeach()
