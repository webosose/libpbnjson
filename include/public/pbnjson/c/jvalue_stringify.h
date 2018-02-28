// Copyright (c) 2015-2018 LG Electronics, Inc.
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

#ifndef INCLUDE_PUBLIC_PBNJSON_C_JVALUE_STRINGIFY_H_
#define INCLUDE_PUBLIC_PBNJSON_C_JVALUE_STRINGIFY_H_

#include "japi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Equivalent of JSON.stringify in JavaScript. Converts the JSON value
 *        to it's equivalent string representation that is ready to be transferred
 *        across the wire (with all appropriate escaping and quoting performed).
 *
 * @param val A reference to the JSON object to convert to a string
 * @return The string representation of the value with a life-time limited by life-time of jvalue_ref or moment of its modification
 */
PJSON_API const char* jvalue_stringify(jvalue_ref val);

/**
 * @brief Equivalent to JSON.stringify within Javascript. Converts the JSON value
 *        to it's equivalent string representation that is ready to be transferred
 *        across the wire (with all appropriate escaping and quoting performed).
 *        Like jvalue_stringify, but with pretty-print option.
 *
 * @param val     A reference to the JSON object to convert to a string
 * @param indent  An Indent for pretty-printed format. Allowed symbols: \\n, \\v, \\f, \\t, \\r and space.
 *                  Combinations of them are permitted as well.
 * @return The string representation of the value with a lifetime equivalent to the value reference
 */
PJSON_API const char* jvalue_prettify(jvalue_ref val, const char *indent);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_PUBLIC_PBNJSON_C_JVALUE_STRINGIFY_H_ */
