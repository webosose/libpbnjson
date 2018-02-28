# Copyright (c) 2009-2018 LG Electronics, Inc.
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
	)

set(TEST_LIBRARIES pbnjson_c pbnjson_cpp ${JSON_C_LDFLAGS} ${YAJL_LDFLAGS})

######################### TEST CONFIGURATION ########################
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/yajl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/yajl.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${YAJL_INCLUDE_DIRS} ${CJSON_INCLUDE_DIRS} ${WEBOS_GTEST_INCLUDES})

list(APPEND TEST_PARSE_FILES file_parse_test)

foreach(i ${TEST_PARSE_FILES})
	configure_file(${i}.json ${i}.json COPYONLY)
	configure_file(${i}.schema ${i}.schema COPYONLY)
endforeach()

######################### THE UNIT TESTS ############################

SET(UnitTest
	SmokeTestMemLeakBadInput
	TestParse
	TestParserMemPool
	TestDOM
	TestJError
	TestJvalue
	TestJobject
	TestSchemaContact
	TestSchemaUniqueItems
	TestSchemaSanity
	TestSchemaParsingErrorReporting
	TestSchemaValidationErrorReporting
	TestSchemaFromJvalue
	TestStringify
	TestNewSchemaContact
	TestNewSchemaArraySanity
	TestExample
	TestThreading
	)

FOREACH(TEST ${UnitTest})
	add_executable(${TEST} ${TEST}.cpp)
	target_link_libraries(${TEST} ${TEST_LIBRARIES} ${WEBOS_GTEST_LIBRARIES} pthread)
	add_test(C.${TEST} ${TEST})
ENDFOREACH()

######################### THE PERFORMANCE TESTS ############################

SET(PerformanceTests
	TestPerformance
	TestSchemaPerformance
	TestJobjectPerformance
	)

FOREACH(TEST ${PerformanceTests})
	add_executable(${TEST} ${TEST}.cpp)
	target_link_libraries(${TEST} ${TEST_LIBRARIES} ${WEBOS_GTEST_LIBRARIES} pthread)
ENDFOREACH()

add_definitions(-DSCHEMA_DIR="${CMAKE_CURRENT_BINARY_DIR}/../schemas/")
