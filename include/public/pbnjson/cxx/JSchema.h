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

#ifndef JSCHEMA_CXX_H_
#define JSCHEMA_CXX_H_

#include "japi.h"
#include "../c/jschema_types.h"
#include "JResult.h"
#include "JInput.h"

namespace pbnjson {

class JValue;
class JResolver;

/**
 * This is an abstract class representing a JSON schema as defined in http://JSON-schema.org.
 * Schemas are used to ensure that any conversion to/from serialized form generates data
 * that is semantically valid (IPC, for large part, is the biggest visible user targetted for
 * this feature).
 *
 * Javascript-style comments are allowed within schemas.
 *
 * @note Not thread safe. Do not share instances of this class between threads.
 */

class PJSONCXX_API JSchema : public JResult
{
protected:
	/**
	 * Construct a schema wrapper.
	 */
	JSchema();

	/**
	 * Construct a schema wrapper for jschema.
	 *
	 * @param schema C schema object
	 */
	JSchema(jschema_ref schema);

	/**
	 * Set original C schema
	 */
	void set(jschema_ref);

private:
	/// Underlying C schema
	jschema_ref schema;

public:
	/**
	 * @brief Return a schema that is guaranteed to never accept any
	 * input as valid.
	 */
	static const JSchema& NullSchema();

	/**
	 * @brief A schema that is guaranteed to accept any input
	 *        as valid.
	 */
	static const JSchema& AllSchema();

	/**
	 * Creates JSchema, that takes ownership of passed jschema_ref.
	 *
	 * @param[in] schema a reference to adopt
	 * @return Newly created JSchema that wraps original reference and manges its life-time
	 */

	static JSchema adopt(jschema_ref schema)
	{
		return schema;
	}

	/**
	 * @brief Creates DOM structure of the schema from a string.
	 *
	 * @param input The input to use for the schema
	 * @return Created schema
	 */
	static JSchema fromString(const JInput &input);

	/**
	 * @brief Creates DOM structure of the schema from a file.
	 *
	 * @param file The file path to the schema
	 * @return Created schema
	 */
	static JSchema fromFile(const char *file);

	/**
	 * @brief Creates DOM structure of the schema from a JSON object.
	 *
	 * @param value The input to generate a schema from - must be a valid JSON object schema
	 * @return Created schema
	 */
	static JSchema fromJValue(const JValue& value);

	/**
	 * @brief Prepare schema for URI resolving.
	 *
	 * @param resolver External (custom) URI resolver
	 * @return true, if resolve succeeded
	 */
	bool resolve(JResolver &resolver);

	/**
	 * @brief Check validity of JValue against the schema.
	 *
	 * @param value A reference to the JSON object to check
	 * @return Positive JResult if value is valid against the schema, otherwise
	 *         JResult with error
	 */
	JResult validate(const JValue &value) const;

	/**
	 * @brief Check validity of JValue against the schema.
	 *        Insert default values specified by the schema into the JSON object.
	 *
	 * @param value A reference to the JSON object to check
	 * @return Positive JResult if value is valid against the schema, otherwise
	 *         JResult with error
	 */
	JResult apply(JValue &value) const;

	/**
	 * Create a copy of the schema object.
	 *
	 * @param other Original schema
	 */
	JSchema(const JSchema& other);

#ifdef CPP11
	/**
	* @brief Move constructor. Move passed schema to newly created.
	*
	* @param other Original schema
	*/
	JSchema(JSchema&& other)
		: JResult(std::move(other))
	{
		schema = other.schema;
		other.schema = nullptr;
	}
#endif

	virtual ~JSchema();

	/**
	 * Assignment operator. Copy source schema.
	 *
	 * @param other Source schema
	 */
	virtual JSchema& operator=(const JSchema& other);

#ifdef CPP11
	/**
	* @brief Move operator. Move passed schema to existing one.
	*
	* @param other Source schema
	*/
	JSchema& operator=(JSchema&& other)
	{
		if (this != &other)
		{
			swap(other);
		}
		return *this;
	}
#endif

	/**
	* @brief Swap with passed JSchema
	*
	* @param other JSchema to swap
	*/
	void swap(JSchema& other);

	/**
	 * Returns true if schema was initialized.
	 *
	 * @retval true if initialized, false otherwise
	 */
	virtual bool isInitialized() const;

	/**
	 * Get underlying C schema.
	 *
	 * @note Ownership for life-time is kept by this JSchema and not transfered to caller .
	 * @return jschema_ref
	 */
	jschema_ref peek() const;

	/**
	 * Return true if schema is initialized.
	 *
	 * @return true if initialized, false otherwise
	*/
#ifdef CPP11
	explicit
#endif

	/**
	 * If schema is NULL, it means that error has occured.
	 */
	inline operator bool() const
	{
		return schema != NULL;
	}

	friend class JParser;
	friend class JGenerator;
	friend class JDomParser;
	friend class JSchemaResolverWrapper;
	friend class JValidator;
};

}

#endif /* JSCHEMA_CXX_H_ */
