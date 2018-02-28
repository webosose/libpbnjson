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

#ifndef JDOMPARSER_H_
#define JDOMPARSER_H_

#include "JParser.h"
#include "JValue.h"
#include "../c/jparse_types.h"
#include "JInput.h"
#include "../c/compiler/deprecated_attribute.h"

namespace pbnjson {

class JResolver;

/**
 * The JDomParser class represents DOM JSON parser.
 *
 * @note This section requires some understanding of XML terminology. The DOM
 * represents JSON values stored as a tree in memory. SAX represents JSON parsing
 * that doesn’t actually create any intermediary representation but instead tokenizes
 * into JSON primitives that the caller is responsible for handling.
 *
 * @see JParser
 */
class PJSONCXX_API JDomParser : public JParser
{
public:
	/**
	 * Initialize a JSON parser that will generate a DOM.
	 */
	JDomParser();

	/**
	 * Initialize a JSON parser with concrete schema.
	 *
	 * @param schema The schema to use for validation of the input
	 */
	JDomParser(const JSchema &schema);

	/**
	* Initialize a JSON parser that will generate a DOM. Resolver is used if schema contains external links.
	*
	* @param resolver The object to use when resolving external references within a schema.
	*
	* @deprecated will be removed in 3.0. External references resolving happens once during schema parsing with JSchema::resolve
	* @see JResolver
	* @see JSchemaFile
	*/
	JDomParser(JResolver *resolver) DEPRECATED_API_MSG("Do JSchema::resolve before using of the schema");

	virtual ~JDomParser();

	/**
	 * Allows the caller to change the optimization level which can vary depending on the
	 * type of stream to be parsed. By default, this is set to a conservative optimization level.
	 *
	 * @param optLevel The optimization level to use.
	 *
	 * @deprecated Client not able to change optimization level.
	 *
	 * @see JDOMOptimization
	 */
	void changeOptimization(JDOMOptimization optLevel) DEPRECATED_API { m_optimization = optLevel; }

	/**
	 * Parse the input using the given schema and given error handlers.
	 *
	 * @param input The JSON string to parse. Must be a JSON object or an array.
	 * Behaviour is undefined if it isn't. This is part of the JSON spec.
	 * @param schema The JSON schema to use when parsing.
	 * @param errors The error handler to use if you want more detailed information if parsing failed.
	 *
	 * @retval true if the JSON input was valid and accepted by the schema
	 * @retval flase if the JSON input was malformed or not accepted by the schema
	 *
	 * @deprecated Use parse w/o JErrorHandler.
	 *
	 * @see JSchema
	 * @see JSchemaFile
	 * @see JErrorHandler
	 */
	bool parse(const std::string& input, const JSchema& schema, JErrorHandler *errors) DEPRECATED_API_MSG("Use parse w/o JErrorHandler");

	/**
	 * @brief Parse the input and create corresponding JValue.
	 *
	 * @param input The input string to parse
	 *
	 * @retval true if the JSON input was valid
	 * @retval false if the JSON input was malformed
	 */
	bool parse(const JInput& input);

	/**
	 * @brief Parse the input using the schema from JDomParser and create corresponding JValue.
	 *
	 * @param input The input string to parse
	 * @param schema The schema to use for validation of the input
	 *
	 * @retval true if the JSON input was valid and accepted by the schema
	 * @retval flase if the JSON input was malformed or not accepted by the schema
	 */
	bool parse(const JInput &input, const JSchema &schema);

	/**
	 * Parse the input file using the given schema.
	 * @param file The JSON string to parse.  Must be a JSON object or an array.  Behaviour is undefined
	 *             if it isn't.  This is part of the JSON spec.
	 * @param schema The JSON schema to use when parsing
	 * @param optimization The optimization level to use for parsing the file.  The JDOMOptimization is
	 *                     not used - it is implicitely set to the most optimal level since the backing buffer
	 *                     will be owned by pbnjson.
	 * @param errors The error handler to use if you want more detailed information if parsing failed.
	 *
	 * @retval true if the JSON input was valid and accepted by the schema
	 * @retval flase if the JSON input was malformed or not accepted by the schema
	 *
	 * @deprecated Use JDomParser::fromFile.
	 *
	 * @see JSchema
	 * @see JSchemaFile
	 * @see JErrorHandler
	 */
	bool parseFile(const std::string& file, const JSchema& schema, ::JFileOptimizationFlags optimization = JFileOptNoOpt, JErrorHandler *errors = NULL)
	DEPRECATED_API_MSG("Use JDomParser::fromFile");

	/**
	 * @brief Prepare parser to parse JSON from a stream with given schema and error handlers.
	 *
	 * @param schema Schema to validate
	 * @param errors Custom error callbacks
	 *
	 * @retval true on success
	 * @retval false on error
	 *
	 * @deprecated Use JDomParser::reset.
	 */
	bool begin(const JSchema &schema, JErrorHandler *errors = NULL) DEPRECATED_API_MSG("Use reset");

	/**
	 * @brief Reset the JSON parser to initial state (ready to parse new JSON).
 	 */
	void reset();

	/**
	 * @brief Reset the JSON parser to initial state with a concrete schema(ready to parse new JSON).
 	 *
 	 * @param schema The schema to use for validation of the input
 	 */
	void reset(const JSchema &schema);

	/**
	 * @brief Parse input JSON chunk by chunk
	 *
	 * @param buf input buffer
	 * @param length input buffer size
	 *
	 * @retval true on success
	 * @retval false on error
	 */
	bool feed(const char *buf, int length) DEPRECATED_API;

	/**
	 * @brief Parse input JSON chunk by chunk
	 *
	 * @param input data input buffer
	 *
	 * @retval true on success
	 * @retval false on error
	 */
	bool feed(const JInput& input);

	/**
	 * @brief Parse input JSON chunk by chunk
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
	 * @brief Returns error description if any errors are encountered by the begin, feed or end methods.
	 *
	 * @return error description
	 */
	const char *getError();

	/**
	 * Retrieve the "DOM" representation of the input that was last parsed by this object.
	 *
	 * @return A JValue representation of the input.
	 * @see JValue
	 */
	JValue getDom();

	/**
	 * @brief Create DOM structure of the JSON document from the string.
	 *
	 * @param input  The input string to parse
	 * @param schema The schema to use for validation of the input
	 * @return A JValue representation of the input, JINVALID with error information on failure
	 */
	static JValue fromString(const JInput &input, const JSchema &schema = JSchema::AllSchema());

	/**
	 * @brief Create DOM structure of the JSON document from the file.
	 *
	 * @param file  The file path to the JSON
	 * @param schema The schema to use for validation of the input
	 * @return A JValue representation of the file, JINVALID with error information on failure
	 */
	static JValue fromFile(const char *file, const JSchema &schema = JSchema::AllSchema());

protected:
	JParser::NumberType conversionToUse() const { return JParser::JNUM_CONV_RAW; }

private:
	JErrorCallbacks prepareCErrorCallbacks();

	JValue m_dom;
	JDOMOptimization m_optimization;
	jdomparser_ref parser;
};

}

#endif /* JDOMPARSER_H_ */
