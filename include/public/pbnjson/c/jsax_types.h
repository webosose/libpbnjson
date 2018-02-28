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

#ifndef JSAX_TYPES_H_
#define JSAX_TYPES_H_

#include "jcallbacks.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*jsax_null)(JSAXContextRef ctxt);
typedef int (*jsax_boolean)(JSAXContextRef ctxt, bool value);
typedef int (*jsax_number)(JSAXContextRef ctxt, const char *number, size_t numberLen);
typedef int (*jsax_string)(JSAXContextRef ctxt, const char *string, size_t stringLen);
typedef int (*jsax_object_start)(JSAXContextRef ctxt);
typedef int (*jsax_object_key)(JSAXContextRef ctxt, const char *key, size_t keyLen);
typedef int (*jsax_object_end)(JSAXContextRef ctxt);
typedef int (*jsax_array_start)(JSAXContextRef ctxt);
typedef int (*jsax_array_end)(JSAXContextRef ctxt);

/**
  * @brief The structure contains set of callbacks, that will be called during parsing of JSON string. The structure is used in JSON SAX parser
  */
typedef struct {
    /// callback which will be called when the parser finds the beginning of an object.
	jsax_object_start m_objStart;
    /// callback which will be called when the parser finds an ObjectKey.
	jsax_object_key m_objKey;
    /// callback which will be called when the object ends.
	jsax_object_end m_objEnd;

    /// callback which will be called when an array begins.
	jsax_array_start m_arrStart;
    /// callback which will be called when an array ends.
	jsax_array_end m_arrEnd;

    /// callback which will be called when a string value is found.
	jsax_string m_string;
    /// callback which will be called when a numeric value is found.
	jsax_number m_number;
    /// callback which will be called when a boolean value is found.
	jsax_boolean m_boolean;
    /// callback which will be called when a null value is found.
	jsax_null m_null;
} PJSAXCallbacks;

#ifdef __cplusplus
}
#endif

#endif /* JSAX_TYPES_H_ */
