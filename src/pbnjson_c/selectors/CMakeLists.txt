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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

find_package(FLEX 2.5.7 REQUIRED)

set(FLEX_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/selectors_grammar_l.c)
set(LEMON_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/selectors_grammar_y.c)

# ---------- FLEX LEXER GENERATOR ------------
add_custom_command(
	OUTPUT ${FLEX_OUTPUT}
	COMMAND ${FLEX_EXECUTABLE} -o ${FLEX_OUTPUT}
	        ${CMAKE_CURRENT_SOURCE_DIR}/selectors_grammar_l.l
	DEPENDS ${LEMON_OUTPUT}
	        ${CMAKE_CURRENT_SOURCE_DIR}/selectors_grammar_l.l
	COMMENT "Generating selectors lexer: ${FLEX_OUTPUT}"
	)

# ---------- LEMON PARSER GENERATOR -----------
add_custom_command(
	OUTPUT ${LEMON_OUTPUT}
	COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/selectors_grammar_y.y .
	        && ${LEMON} selectors_grammar_y.y
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/selectors_grammar_y.y
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating selectors parser: ${LEMON_OUTPUT}"
	)

# Suppress warnings for generated files
SET_SOURCE_FILES_PROPERTIES(${FLEX_OUTPUT} PROPERTIES COMPILE_FLAGS "-Wno-unused-function")
SET_SOURCE_FILES_PROPERTIES(${LEMON_OUTPUT} PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")

add_library(
	json_selectors
	OBJECT
	expression.c
	jquery.c
	jquery_selectors.c
	jquery_generators.c
	${LEMON_OUTPUT}
	${FLEX_OUTPUT}
	)

include_directories(
	${API_HEADERS}
	${API_HEADERS}/pbnjson
	${API_HEADERS}/pbnjson/c
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}
	)

if(WEBOS_CONFIG_BUILD_TESTS)
	add_subdirectory(test)
	enable_testing()
endif()
