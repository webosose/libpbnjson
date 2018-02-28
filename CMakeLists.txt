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

cmake_minimum_required(VERSION 2.8.8)
project(pbnjson)

include(webOS/webOS)
webos_modules_init(1 0 0 QUALIFIER RC9)
webos_component(2 15 0)
add_definitions(-DWEBOS_COMPONENT_VERSION="${WEBOS_COMPONENT_VERSION}")

include(FindPkgConfig)

find_library(GMP_LIBRARY gmp ${LIB_INSTALL_DIR})
find_package(Threads)

# prefer local headers
include_directories(include/public)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL -Wall -D__STRICT_ANSI__ ${GLIB2_CFLAGS_OTHER} -pthread)

set(WEBOS_USE_WERROR FALSE CACHE BOOL "Set to TRUE to enable -Werror")
if(WEBOS_USE_WERROR)
	message("Build with treating most warnings as errors")
	webos_add_compiler_flags(ALL
	                        -Werror -Wextra
	                        -Wno-error=missing-field-initializers
	                        -Wno-error=unused-parameter
	                        )
endif()

# YAJL-1 doesn't ship any pkg-check module
pkg_check_modules(YAJL yajl)
if("${YAJL_VERSION}" STREQUAL "")
	message(STATUS "Looking for YAJL in library paths")
	find_library(YAJL_LDFLAGS yajl ${LIB_INSTALL_DIR})
	if(${YAJL_LDFLAGS} STREQUAL "YAJL_LDFLAGS-NOTFOUND")
		message(FATAL_ERROR "Cannot find YAJL")
	else()
		message(STATUS "Found ${YAJL_LDFLAGS}")
	endif()
else()
	include_directories(${YAJL_INCLUDE_DIRS})
endif()

pkg_check_modules(URIPARSER REQUIRED liburiparser)
include_directories(${URIPARSER_INCLUDE_DIRS})

find_program(GPERF NAMES gperf DOC "GNU gperf perfect hash function generator")
if(${GPERF} STREQUAL "GPERF-NOTFOUND")
	message(FATAL_ERROR "Cannot find GNU gperf executable")
endif()

find_program(LEMON NAMES lemon DOC "The LEMON LALR(1) parser generator")
if(${LEMON} STREQUAL "LEMON-NOTFOUND")
	message(FATAL_ERROR "Cannot find lemon LALR(1) parser generator")
endif()

set(API_HEADERS ${CMAKE_SOURCE_DIR}/include/public)
set(WEBOS_WITH_TESTS FALSE CACHE BOOL "Control building of unit tests")
set(PBNJSON_LOG TRUE CACHE BOOL "Enable logging in libpbnjson")
set(PBNJSON_LOG_WARN TRUE CACHE BOOL "Log not only errors but warnings also")
set(PBNJSON_INSTALL_TOOLS FALSE CACHE BOOL "Install pbnjson tools like pbnjson_validate")

if(PBNJSON_LOG)
	if(NOT PBNJSON_LOG_WARN)
		webos_add_compiler_flags(ALL -DPJSON_NO_WARNINGS=1)
	endif()
else()
	webos_add_compiler_flags(ALL -DPJSON_NO_LOGGING=1)
endif()

if(WEBOS_CONFIG_BUILD_DOCS)
	add_subdirectory(doc)
else()
	message(STATUS "Skipping document generation")
endif()

if(WEBOS_CONFIG_BUILD_TESTS)
	include(CTest)

	webos_use_gtest()

	add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)
else()
	message(STATUS "Skipping automatic tests")
endif()

add_subdirectory(src)
