# Copyright (c) 2015-2018 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

include_directories(
	${API_HEADERS}
	${API_HEADERS}/pbnjson
	${API_HEADERS}/pbnjson/c
	../../pbnjson_c
	)

######################### TEST CONFIGURATION ########################
set(TEST_LIBRARIES pbnjson_c pbnjson_cpp ${YAJL_LDFLAGS})

include(CheckSymbolExists)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${YAJL_INCLUDE_DIRS} ${WEBOS_GTEST_INCLUDES})
include_directories(${CMAKE_SOURCE_DIR})

######################### THE UNIT TESTS ############################

SET(UnitTest
	TestNumConversion
	TestKeyDictionary
	)

FOREACH(TEST ${UnitTest})
	add_executable(${TEST} ${TEST}.cpp)
	target_link_libraries(${TEST} ${TEST_LIBRARIES} ${WEBOS_GTEST_LIBRARIES} pthread)
	add_test(C.${TEST} ${TEST})
ENDFOREACH()
