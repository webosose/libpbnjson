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

#ifndef JERROR_HANDLER_H_
#define JERROR_HANDLER_H_

#include "japi.h"

#include <string>

namespace pbnjson {

class JParser;

/**
 * The class represents an interface to the user error handler.
 *
 * @deprecated Use JResult to get error information.
 */
class PJSONCXX_API JErrorHandler
{
public:

	/**
	 * @brief The SyntaxError enum error code of JSON syntax parsing
	 */
	enum SyntaxError {
		ERR_SYNTAX_GENERIC = 20,
	};

	// TODO: Expand error codes to cover all C-level validation error codes

	/**
	 * @brief The SchemaError enum - error code of parsing JSON Schema
	 */
	enum SchemaError {
		ERR_SCHEMA_GENERIC = 40,
		ERR_SCHEMA_MISSING_REQUIRED_KEY,
		ERR_SCHEMA_UNEXPECTED_TYPE,
	};

	/**
	 * error code of parsing JSON Object
	 * @deprecated will be removed in 3.0
	 *
	 * @see SchemaError
	 */
	enum BadObject {
		ERR_BAD_OBJECT_OPEN = 60,
		ERR_BAD_OBJECT_KEY,
		ERR_BAD_OBJECT_CLOSE,
	};

	/**
	 * @brief error code of parsing JSON array
	 * @deprecated will be removed in 3.0
	 *
	 * @see SchemaError
	 */
	enum BadArray {
		ERR_BAD_ARRAY_OPEN = 80,
		ERR_BAD_ARRAY_CLOSE,
	};

	/**
	 * @brief JErrorHandler copy constructor
	 * @param handler other instance
	 */
	JErrorHandler(const JErrorHandler& handler);

	/**
	 * @brief ~JErrorHandler destruct a error handler
	 */
	virtual ~JErrorHandler();

	/**
	 * @brief Syntax error notification
	 * @param ctxt Parser object
	 * @param code Error code. See #SyntaxError
	 * @param reason Detailed description
	 */
	virtual void syntax(JParser *ctxt, SyntaxError code, const std::string& reason) = 0;

	/**
	 * @brief Notification on schema violation
	 * @param ctxt Parser object
	 * @param code Error code. See #SchemaError
	 * @param reason Detailed description
	 */
	virtual void schema(JParser *ctxt, SchemaError code, const std::string& reason) = 0;

	/**
	 * @brief Notification on other error
	 * @param ctxt Parser object
	 * @param reason Detailed description
	 */
	virtual void misc(JParser *ctxt, const std::string& reason) = 0;

	/**
	 * @brief Error notification from parser
	 * @param ctxt Parser object
	 * @param reason Detailed description
	 */
	virtual void parseFailed(JParser *ctxt, const std::string& reason) = 0;

	/**
	 * @brief Error notification from parser
	 * @deprecated badObject will be removed in 3.0
	 */
	virtual void badObject(JParser *ctxt, BadObject code) = 0;

	/**
	 * @brief Error notification from parser
	 * @deprecated badArray will be removed in 3.0
	 */
	virtual void badArray(JParser *ctxt, BadArray code) = 0;

	/**
	 * @brief Error notification from parser
	 * @deprecated badString will be removed in 3.0
	 */
	virtual void badString(JParser *ctxt, const std::string& str) = 0;

	/**
	 * @brief Error notification from parser
	 * @deprecated badNumber will be removed in 3.0
	 */
	virtual void badNumber(JParser *ctxt, const std::string& number) = 0;

	/**
	 * @brief Error notification from parser
	 * @deprecated badBoolean will be removed in 3.0
	 */
	virtual void badBoolean(JParser *ctxt) = 0;

	/**
	 * @brief Error notification from parser
	 * @deprecated badNull will be removed in 3.0
	 */
	virtual void badNull(JParser *ctxt) = 0;

protected:
	/**
	 * @brief JErrorHandler default constructor
	 */
	JErrorHandler();
};

}

#endif /* JERROR_HANDLER_H_ */
