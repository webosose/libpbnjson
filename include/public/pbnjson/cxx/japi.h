// Copyright (c) 2009-2018 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef J_CXX_API_H_
#define J_CXX_API_H_

//! @cond Doxygen_Suppress
#define API_EXPORT __attribute__((visibility("default")))
#define API_IMPORT __attribute__((visibility("default")))
#define API_LOCAL __attribute__((visibility("hidden")))

#ifdef PJSONCXX_SHARED
	#ifdef PJSONCXX_EXPORT
		#define PJSONCXX_API API_EXPORT
		#define PJSONCXX_LOCAL API_LOCAL
	#else
		#define PJSONCXX_API API_IMPORT
		#define PJSONCXX_LOCAL
	#endif
#else
	#define PJSONCXX_API
	#define PJSONCXX_LOCAL
#endif
//! @endcond

namespace pbnjson {

/**
 * Set the name of the component utilizing this library (per-process). This should be as descriptive as possible
 * to help narrow down bugs.
 *
 * Note: String must have program lifetime.
 * Note: If this is never called, then the component's name will be printed as base name of /proc/$pid/cmdline or
 * if detecting fails - 'unknown process name'.
 *
 * @param name The name of the component using this API.
 */
PJSONCXX_API void setConsumerName(const char *name);

/**
 * Get the consumer name. See #setConsumerName
 *
 * @return The name passed to setConsumerName previously or NULL if it was never called.
 */
PJSONCXX_API const char *getConsumerName();

}

#endif /* J_CXX_API_H_ */
