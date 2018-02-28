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

#ifndef JGEN_TYPES_H_
#define JGEN_TYPES_H_

#include "japi.h"
#include "jtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief JStreamRef should be used, instead of __JStream
 */
typedef struct __JStream* JStreamRef;

/**
 *  Defines error codes
 */
typedef enum {
	/// success. there is no error
	GEN_OK,
	/// key is not a string
	GEN_KEYS_MUST_BE_STRINGS,
	/// document is incomplete
	GEN_INCOMPLETE_DOCUMENT,
	/// jvalue violates a schema
	GEN_SCHEMA_VIOLATION,
	/// other error
	GEN_GENERIC_ERROR,
} StreamStatus;

/**
 * @brief definition of a callback which will be called when the parser finds the beginning of an object.
 * @param stream JStreamRef pointer. Which can be a custom user's structure where __JStream is the first member with additional fields after it.
 * Such fields can be used to store user's data.
 */
typedef JStreamRef (*jObjectBegin)(JStreamRef stream);

/**
 * @brief definition of a callback which will be called when the parser finds an ObjectKey
 * @param stream JStreamRef pointer. See #jObjectBegin for details
 * @param str name of a key
 */
typedef JStreamRef (*jObjectKey)(JStreamRef stream, raw_buffer str);

/**
 * @brief definition of a callback which will be called when the object ends.
 * @param stream JStreamRef pointer. See #jObjectBegin for details
 */
typedef JStreamRef (*jObjectEnd)(JStreamRef stream);

/**
 * @brief definition of a callback  which will be called when an array begins.
 * @param stream JStreamRef pointer. See #jObjectBegin for details
 */
typedef JStreamRef (*jArrayBegin)(JStreamRef stream);

/**
 * @brief definition of a callback which will be called when an array ends.
 * @param stream JStreamRef pointer. See #jObjectBegin for details
 */
typedef JStreamRef (*jArrayEnd)(JStreamRef stream);

/**
 * @brief definition of a callback which will be called when a numeric value is found.
 * @param stream JStreamRef pointer. See #jObjectBegin for details
 * @param num string representation of a number
 */
typedef JStreamRef (*jNumber)(JStreamRef stream, raw_buffer num);

/**
 * @brief definition of a callback which will be called when a integer value is found.
 * @param stream JStreamRef pointer. See #jObjectBegin for details
 * @param num integer number
 */
typedef JStreamRef (*jNumberI)(JStreamRef stream, int64_t num);

/**
 * @brief definition of a callback which will be called when a floating point value is found.
 * @param stream JStreamRef pointer. See #jObjectBegin for details
 * @param num number with floating point
 */
typedef JStreamRef (*jNumberF)(JStreamRef stream, double num);

/**
 * @brief definition of a callback which will be called when a string value is found.
 * @param stream JStreamRef pointer. See #jObjectBegin for details
 * @param str string value
 */
typedef JStreamRef (*jString)(JStreamRef stream, raw_buffer str);

/**
 * @brief definition of a callback which will be called when a boolean value is found.
 * @param stream JStreamRef pointer. See #jObjectBegin for details
 * @param value boolean value
 */
typedef JStreamRef (*jBoolean)(JStreamRef stream, bool value);

/**
 * @brief definition of a callback which will be called when a null value is found.
 * @param stream JStreamRef pointer. See #jObjectBegin for details
 */
typedef JStreamRef (*jNull)(JStreamRef stream);

/**
 * @brief definition of the latest callback with status
 * @param stream JStreamRef pointer. See #jObjectBegin for details
 * @param errorCode error code. See #StreamStatus
 */
typedef char* (*jFinish)(JStreamRef stream, StreamStatus *errorCode);

/**
 * @brief The _JStream structure contains a set of callbacks, that are invoked during traversing of jvalue
 * A callback is invoked when the proper event happens: start of an object, end of array, ...
 * Do not use __JStream directly. Use #JStreamRef structure.
 */
struct __JStream {
	/// begin of an object. Pointer to a function which will be called when the parser finds the beginning of an object.
    jObjectBegin o_begin;
	/// name of a key. Pointer to a function which will be called when the parser finds an ObjectKey.
    jObjectKey o_key;
	/// end of an object. Pointer to a function which will be called when the object ends.
    jObjectEnd o_end;
	/// begin of an array. Pointer to a function which will be called when an array begins.
    jArrayBegin a_begin;
	/// end of an array. Pointer to a function which will be called when an array ends.
    jArrayEnd a_end;
	/// value is a number. Pointer to a function which will be called when a numeric value is found.
    jNumber number;
	/// value is an integer. Pointer to a function which will be called when a integer value is found.
    jNumberI integer;
	/// value is a floating point. Pointer to a function which will be called when a floating point value is found.
    jNumberF floating;
	/// value is a string. Pointer to a function which will be called when a string value is found.
    jString string;
	/// boolean value is a boolean. Pointer to a function which will be called when a boolean value is found.
    jBoolean boolean;
	/// value is null. Pointer to a function which will be called when a null value is found.
    jNull null_value;
	/// finish the last callback with status. Pointer to a function which will be called when parsing is complete.
    jFinish finish;
};

#ifdef __cplusplus
}
#endif

#endif /* JGEN_TYPES_H_ */
