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


#ifndef JVALIDATOR_H_
#define JVALIDATOR_H_

#include "japi.h"
#include "JValue.h"
#include "JSchema.h"
#include "JResolver.h"
#include "JErrorHandler.h"
#include "../c/compiler/deprecated_attribute.h"

namespace pbnjson {

	/**
	 * JValidator is a class to check validity of JValue against JSchema. Performs checking and
	 * calls corresponding error handler in case of error.
	 *
	 * @deprecated This class is obsolete now! Will be removed in 3.0 version. Use JSchema::validate() and JSchema::apply().
	 */
class JValidator {
public:

	/**
	 * Check validity of JValue against JSchema. The function is able to solve external references. In order to check
	 * schema with externals, provide an appropriate resolver.
	 *
	 * @param jValue A reference to the JSON object to check
	 * @param jSchema A reference to schema
	 * @param jResolver A reference to resolver of externals
	 * @param errors The error handler to use if you want more detailed information if parsing failed.
	 * @return true if JSON value is valid against schema
	 *
	 * @deprecated use JSchema::validate
	 * @see JValue
	 * @see JSchema
	 * @see JErrorHandler
	 */
	static bool isValid(const JValue &jValue, const JSchema &jSchema, JResolver &jResolver, JErrorHandler *errors = NULL);

	/**
	 * Check validity of JValue against JSchema. The function is not able to resolve externals
	 *
	 * @param jValue A reference to the JSON object to check
	 * @param jSchema A reference to schema
	 * @param errors The error handler to use if you want more detailed information if parsing failed.
	 * @return true if JSON value is valid against schema
	 *
	 * @deprecated use JSchema::validate
	 * @see JValue
	 * @see JSchema
	 * @see JErrorHandler
	 */
	static bool isValid(const JValue &jValue, const JSchema &jSchema, JErrorHandler *errors = NULL);

	/**
	 * Check validity of JValue against JSchema, and modifies JValue by inserting default values(specified by schema).
	 * The function is able to resovle external references in the schema.
	 *
	 * @param jValue A reference to the JSON object to check
	 * @param jSchema A reference to schema
	 * @param jResolver A pointer to resolver of externals
	 * @param errors The error handler to use if you want more detailed information if parsing failed.
	 * @return true if JSON value is valid against schema
	 *
	 * @deprecated use JSchema::apply
	 * @see JValue
	 * @see JSchema
	 * @see JErrorHandler
	 */
	static bool apply(const JValue &jValue, const JSchema &jSchema, JResolver *jResolver = NULL, JErrorHandler *errors = NULL);
} DEPRECATED_API_MSG("Use JSchema::validate or JSchema::apply");

}


#endif // JVALIDATOR_H_
