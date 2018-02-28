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

#ifndef JSCHEMA_FRAGMENT_H_
#define JSCHEMA_FRAGMENT_H_

#include "JSchema.h"
#include <string>
#include "../c/compiler/deprecated_attribute.h"

namespace pbnjson {

/**
 * Javascript-style comments are allowed within schemas.
 *
 * Initialize this schema object to represent the schema string.  Behaviour is
 * undefined if this is an invalid schema.
 *
 * <palm-internal-comment>
 * This is a temporary convenience class only and is likely to go away in the future.
 * Also, use JSchemaFile instead, since it is more likely what you want.
 *
 * DO NOT INLINE SCHEMA INTO CODE UNLESS YOU HAVE A VERY GOOD REASON.  IT IS HIGHLY
 * UNLIKELY YOU HAVE A VERY GOOD REASON.
 * </palm-internal-comment>
 *
 * @deprecated Use JSchema::fromString.
 */
class PJSONCXX_API JSchemaFragment : public JSchema
{
private:
	/**
	 * Create new C schema from the fragment
	 *
	 * \param fragment string fragment representation
	 */
	jschema_ref createSchema(const std::string &fragment);

public:
	/**
	 * Create new schema fragment from the string
	 *
	 * \param fragment string fragment representation
	 */
	JSchemaFragment(const std::string& fragment);

	/**
	 * Create a copy of the schema fragment
	 *
	 * \param other Source schema fragment
	 */
	JSchemaFragment(const JSchemaFragment& other);
	~JSchemaFragment();
} /* TODO DEPRECATED_API_MSG("Use JSchema::fromString") //smartkey-hun failed */;

}

#endif /* JSCHEMA_FRAGMENT_H_ */
