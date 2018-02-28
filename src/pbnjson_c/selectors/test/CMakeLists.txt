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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
include_directories(${WEBOS_GTEST_INCLUDES})

set(TEST_LIBRARIES pbnjson_cpp)

SET(UnitTests
	TestSyntaxParser
	TestChildrenGenerators
	TestSimpleSelectors
	TestTypeSelectors
	TestKeySelectors
	TestContainsSelector
	TestHas
	TestExpression
	TestParentCombinator
	TestAncestorCombinator
	TestSiblingCombinator
	TestArrayElements
	TestValueSelector
	TestOrSelector
	)

FOREACH(TEST ${UnitTests})
	add_executable(${TEST} ${TEST}.cpp Utils.cpp)
	target_link_libraries(${TEST} ${TEST_LIBRARIES} ${WEBOS_GTEST_LIBRARIES} ${GLIB2_LDFLAGS})
	add_test(Selectors.${TEST} ${TEST})
ENDFOREACH()
