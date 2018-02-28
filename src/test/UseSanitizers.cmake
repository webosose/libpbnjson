# Copyright (c) 2016-2018 LG Electronics, Inc.
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

include(CheckCCompilerFlag)

check_c_compiler_flag("-fsanitize=address" HAVE_FLAG_SANITIZE_ADDRESS_ALONE)

if(NOT HAVE_FLAG_SANITIZE_ADDRESS_ALONE)
	set(ASAN_LIBRARIES -lasan -ldl -lpthread -lm)
	set(CMAKE_REQUIRED_LIBRARIES ${ASAN_LIBRARIES})
	check_c_compiler_flag("-fsanitize=address" HAVE_FLAG_SANITIZE_ADDRESS_LIBS)
	unset(CMAKE_REQUIRED_LIBRARIES)
endif()

if(HAVE_FLAG_SANITIZE_ADDRESS_ALONE OR HAVE_FLAG_SANITIZE_ADDRESS_LIBS)
	webos_add_compiler_flags(DEBUG -fsanitize=address)
	webos_add_linker_options(DEBUG ${ASAN_LIBRARIES})
endif()
