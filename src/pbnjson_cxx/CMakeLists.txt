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

file(GLOB SHARED_SOURCE "./*.cpp" "../pbnjson_c/debugging.c")

if(WITH_VERBOSE_DEBUG)
	add_definitions(-DPJSON_LOG_DBG=1)
endif()
if(WITH_VERBOSE_TRACE)
	add_definitions(-DPJSON_LOG_TRACE=1)
	add_definitions(-DPJSON_SCHEMA_TRACE=3)
endif()
add_definitions(-DPJSONCXX_SHARED)
add_definitions(-DPJSON_SHARED)
add_definitions(-DPJSONCXX_EXPORT)

option(WEBOS_CLANG_BUILD "Build with clang" OFF)
if(${WEBOS_CLANG_BUILD})
	set(CLANG_EXT "_clang")
else()
	set(CLANG_EXT "")
endif()

add_definitions(-DLIBRARY_NAME=pbnjson_cpp${CLANG_EXT})

add_library(pbnjson_cpp${CLANG_EXT} SHARED ${SHARED_SOURCE})
target_link_libraries(pbnjson_cpp${CLANG_EXT} pbnjson_c${CLANG_EXT})

include_directories(${API_HEADERS}
                    ${API_HEADERS}/pbnjson
                    ${API_HEADERS}/pbnjson/c
                    ${API_HEADERS}/pbnjson/cxx
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/../pbnjson_c
                    ${CMAKE_CURRENT_SOURCE_DIR})


# All of the headers are installed by pbnjson_c
webos_build_library(NAME pbnjson_cpp${CLANG_EXT} NOHEADERS)
webos_build_pkgconfig(files/pkgconfig/pbnjson_cpp)
