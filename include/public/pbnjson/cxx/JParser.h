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

#ifndef JPARSER_H
#define JPARSER_H

#include "japi.h"

#include <string>
#include <memory>

#include "JSchema.h"
#include "../c/jconversion.h"
#include "../c/compiler/deprecated_attribute.h"
#include "JInput.h"

namespace pbnjson {

class JErrorHandler;
class JSchema;
class JResolver;
class JSchemaResolverWrapper;

/**
 * The JParser class represents SAX JSON parser.
 *
 * @note This section requires some understanding of XML terminology. The DOM
 * represents JSON values stored as a tree in memory. SAX represents JSON parsing
 * that doesn’t actually create any intermediary representation but instead tokenizes
 * into JSON primitives that the caller is responsible for handling.
 *
 * @see JDomParser
 */
class PJSONCXX_API JParser
{
public:
	enum NumberType {
		/**
		 * The numeric string is converted to a 64-bit integer or a floating point
		 * number.
		 *
		 * @note  There are no assumptions about limits.
		 */
		JNUM_CONV_RAW,

		/**
		 * The numeric string is passed untouched.
		 *
		 * @note Use this only for handling some specific numeric types.
		 */
		JNUM_CONV_NATIVE,
	};

	/**
	 * Structure, which represents current parser position
	 */
	struct ParserPosition {
		/// current line
		int m_line;
		/// current column
		int m_column;
	};

	/**
	 * Initialize new parser.
	 */
	JParser();

	/**
	 * Initialize a parser with the concrete schema.
	 */
	JParser(const JSchema &schema);

	/**
	 * Initialize a JSON. Resolver is used if schema contains external links.
	 *
	 * @param schemaResolver The object to use when resolving external references within a schema.
	 *
	 * @deprecated will be removed in 3.0. Resolve schema with JSchema::resolve()
	 *
	 * @see JSchemaFile
	 */
	JParser(JResolver *schemaResolver) DEPRECATED_API_MSG("Do JSchema::resolve before using of the schema");
	/**
	 * @brief JParser Copy constructor
	 * @param other other instance
	 */
	JParser(const JParser& other);
	virtual ~JParser();

	/**
	 * Parse the input using the given schema.
	 *
	 * @param input The JSON string to parse. Must be a JSON object or an array. Behaviour is undefined
	 * if it isn't. This is part of the JSON spec.
	 * @param schema The JSON schema to use when parsing.
	 * @param errors The error handler to use if you want more detailed information if parsing failed.
	 *
	 * @retval true if the JSON input was valid and accepted by the schema.
	 * @retval false if the JSON input was mailformed or wasn't accepted by the schema.
	 *
	 * @deprecated
	 * @see JSchema
	 * @see JSchemaFile
	 * @see JErrorHandler
	 */
	virtual bool parse(const std::string& input, const JSchema &schema, JErrorHandler *errors = NULL) DEPRECATED_API_MSG("Use parse w/o JErrorHandler");

	/**
	 * @brief Parse the input
	 *
	 * @param input The input string to parse
	 *
	 * @retval true if the JSON input was valid and accepted by the schema.
	 * @retval false if the JSON input was mailformed or wasn't accepted by the schema.
	 */
	bool parse(const JInput &input);

	/**
	 * @brief Parse the input using the given schema.
	 *
	 * @param input The input string to parse
	 * @param schema The schema to use for validation of the input
	 *
	 * @retval true if the JSON input was valid and accepted by the schema.
	 * @retval false if the JSON input was mailformed or wasn't accepted by the schema.
	 */
	bool parse(const JInput &input, const JSchema &schema);

	/**
	 * @brief Get current error handler.
	 */
	JErrorHandler* getErrorHandler() const;
	/**
	 * @brief Get current parser position.
	 */
	ParserPosition getPosition() const;

	/**
	 * @brief Prepare parser to parse JSON from a stream with given schema and error handlers
	 *
	 * @param schema Schema to validate
	 * @param errors Custom error callbacks
	 *
	 * @retval true on success
	 * @retval false on error
	 *
	 * @deprecated use JParser::reset(const JSchema &schema)
	 */
	bool begin(const JSchema &schema, JErrorHandler *errors = NULL) DEPRECATED_API_MSG("Use reset");

	/**
	 * @brief Prepare class to parse JSON from stream
	 */
	void reset();

	/**
	 * @brief Reset the parser to use a concrete schema for parsing from a stream.
	 *
	 * @param schema The schema to use for validation of the input
	 */
	void reset(const JSchema &schema);

	/**
	 * @brief Feed next chunk of the JSON from the stream. Use char * and int's length as input buffer.
	 *
	 * @param buf input buffer
	 * @param length input buffer size
	 *
	 * @retval true on success
	 * @retval false on error
	 */
	bool feed(const char *buf, int length) DEPRECATED_API;

	/**
	 * @brief Feed next chunk of the JSON from the stream.  Use JInput as input buffer.
	 *
	 * @param input data input buffer
	 *
	 * @retval true on success
	 * @retval false on error
	 */
	bool feed(const JInput& input);

	/**
	 * @brief Feed next chunk of the JSON from the stream. Use std::string as input buffer.
	 *
	 * @param data input buffer
	 *
	 * @retval true on success
	 * @retval false on error
	 */
	bool feed(const std::string &data) DEPRECATED_API;

	/**
	 * @brief Finalize stream parsing. This indicates the end of
	 * the JSON. Perform final schema check.
	 *
	 * @retval true on success
	 * @retval false on error
	 */
	bool end();

	/**
	 * @brief Return error description if any of begin, feed or end functions have returned false.
	 *
	 * @return error description
	 */
	char const *getError();

protected:
	/*
	 * SAX parser callbacks. By default, this parser will not parse any input. You must override
	 * all the functions that might be called if you want. Schema validation occurs
	 * just before these virtual functions are called.
	 */

	/**
	 * Called when a valid object open token { is encountered
	 * @return Returns true if parsing should continue and false if parsing should be discontinued.
	 */
	virtual bool jsonObjectOpen() { return false; }
	/**
	 * Called when a valid property instance name is encountered
	 * @return Returns true if parsing should continue and false if parsing should be discontinued.
	 */
	virtual bool jsonObjectKey(const std::string& key) { return false; }
	/**
	 * Called when a valid object close token } is encountered
	 * @return Returns true if parsing should continue and false if parsing should be discontinued.
	 */
	virtual bool jsonObjectClose() { return false; }

	/**
	 * Called when a valid array open token [ is encountered
	 * @return Returns true if parsing should continue and false if parsing should be discontinued.
	 */
	virtual bool jsonArrayOpen() { return false; }
	/**
	 * Called when a valid array close token ] is encountered
	 * @return Returns true if parsing should continue and false if parsing should be discontinued.
	 */
	virtual bool jsonArrayClose() { return false; }

	/**
	 * Called when a valid non-object-key string is encountered
	 * @return Returns true if parsing should continue and false if parsing should be discontinued.
	 */
	virtual bool jsonString(const std::string& s) { return false; }

	/**
	 * Called when a valid number is encountered.  This method is invoked
	 * only if conversionToUse returns JNUM_CONV_RAW.
	 * @return Returns true if parsing should continue and false if parsing should be discontinued.
	 */
	virtual bool jsonNumber(const std::string& n) { return false; }
	/**
	 * Called when a valid number is encountered.  This method is invoked
	 * only if conversionToUse returns JNUM_CONV_NATIVE and the JSON number can be
	 * converted perfectly to a 64-bit integer.  Careful because even if you expect a floating point
	 * number, you may receive an integer.
	 * @return Returns true if parsing should continue and false if parsing should be discontinued.
	 */
	virtual bool jsonNumber(int64_t number) { return false; }
	/**
	 * Called when a valid number is encountered.  This method is invoked
	 * only if the number cannot be converted to a 64-bit integer.  Any errors converting this
	 * to a number are passed along (e.g. potential loss of precision, overflows, etc).
	 * @return Returns true if parsing should continue and false if parsing should be discontinued.
	 */
	virtual bool jsonNumber(double &number, ConversionResultFlags asFloat) { return false; }
	/**
	 * Called when a valid boolean is encountered.
	 * @return Returns true if parsing should continue and false if parsing should be discontinued.
	 */
	virtual bool jsonBoolean(bool truth) { return false; }
	/**
	 * Called when a valid null is encountered.
	 * @return Returns true if parsing should continue and false if parsing should be discontinued.
	 */
	virtual bool jsonNull() { return false; }

	/**
	 * The conversionToUse member specifies how the parser expects
	 * numbers to be handled @see NumberType
	 *
	 * JNUM_CONV_NATIVE - The numeric string is converted to a 64-bit integer or a floating point
	 *                    number internally (using proper JSON number parsing)
	 * JNUM_CONV_RAW    - The numeric string is passed unchanged. Use this only if there is
	 *                    some well defined need that can't be served by using native types
	 *
	 * @return The number type this parser is expecting. This controls the sax routine
	 *         that is called for number notification.
	 */
	virtual NumberType conversionToUse() const = 0;

	JErrorHandler* errorHandlers() const;

	/**
	 * Set error handlers for the parser.
	 *
	 * @param errors Error handler
	 */
	void setErrorHandlers(JErrorHandler* errors);

protected:
	/// Internal resolve wrapper
	JSchemaResolverWrapper *m_resolverWrapper;

	// Parser schema
	JSchema schema;
	/// Parser schema info
	JSchemaInfo schemaInfo;
	/// Parser error handler
	JErrorCallbacks errorHandler;
	/// Parser schema resolver
	JSchemaResolver externalRefResolver;

	/**
	 * Create new schema info object.
	 */
	JSchemaInfo prepare(const JSchema &schema, JSchemaResolver &resolver, JErrorCallbacks &cErrCbs, JErrorHandler *errors);

	/**
	 * Create new schema resolver.
	 */
	JSchemaResolver prepareResolver() const;
	//TODO remove in 3.0
	bool oldInterface;

private:
	/// Internal errors
	JErrorHandler* m_errors;
	friend class SaxBounce;
	/// internal C parser.
	// TODO why not to use jsaxparser here instead of reference?
	jsaxparser_ref parser;

	/**
	 * Create error handler object.
	 */
	JErrorCallbacks prepareCErrorCallbacks();
};

}

#endif /* JPARSER_H_ */
