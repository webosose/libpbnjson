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

#ifndef J_CALLBACKS_H_
#define J_CALLBACKS_H_

#include "japi.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/// internal structure maintaining context about the parser
struct PJSON_LOCAL  __JSAXContext;

/// @brief opaque reference to the parser context
///
/// opaque reference to the parser context
typedef struct __JSAXContext* JSAXContextRef;

/// @brief callback function which will be invoked when there is an error while parsing the input. The callback should return true if the error is fixed and parsing can be continued
///
/// callback function that is called when there is an error while parsing the input. The callback should return true if the error is fixed and parsing can be continued
typedef bool (*jerror_parsing)(void *ctxt, JSAXContextRef parseCtxt);

/// @brief callback function which will be invoked when JSON does not match a schema
///
/// callback function which will be invoked when JSON does not match a schema
typedef bool (*jerror_schema)(void *ctxt, JSAXContextRef parseCtxt);

/// @brief callback function which will be invoked when general error occurs
///
/// callback function which will be invoked when general error occurs
typedef bool (*jerror_misc)(void *ctxt, JSAXContextRef parseCtxt);

/**
 * @brief Structure contains set of callbacks which will be invoked if errors occur during JSON processing.
 * All fields are optional to initialize. If a callback is not specified(is NULL) it will not be called.
 *
 * Structure contains set of callbacks which will be invoked if errors occur during JSON processing.
 * All fields are optional to initialize. If a callback is not specified(is NULL) it will not be called.
 */
typedef struct JErrorCallbacks
{
	/// there was an error parsing the input
	jerror_parsing m_parser;
	/// there was an error validating the input against the schema
	jerror_schema m_schema;
	/// some other error occured while parsing
	jerror_misc m_unknown;
	/// user-specified data. Data that will be passed to callback function
	void *m_ctxt;
} *JErrorCallbacksRef;

#ifdef __cplusplus
}
#endif

#endif /* J_CALLBACKS_H_ */
