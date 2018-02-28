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

#include "JSchema.h"
#include "JValue.h"
#include "JSchemaFragment.h"
#include "JSchemaResolverWrapper.h"

#include <pbnjson.h>

using namespace std;

namespace pbnjson {

#ifndef SK_DISALLOWED
#define SK_DISALLOWED "disallowed"
#endif

const JSchema& JSchema::NullSchema()
{
	static const JSchemaFragment NO_VALID_INPUT_SCHEMA(
		"{\"" SK_DISALLOWED "\":\"any\"}"
	);
	return NO_VALID_INPUT_SCHEMA;
}

const JSchema& JSchema::AllSchema()
{
	static const JSchema all_schema(jschema_all());
	return all_schema;
}

JSchema::JSchema(const JSchema& other)
	: JResult(other)
	, schema(other.schema ? jschema_copy(other.schema) : NULL)
{
}

JSchema::~JSchema()
{
	jschema_release(&schema);
}

JSchema& JSchema::operator=(const JSchema& other)
{
	if (this != &other)
	{
		JSchema(other).swap(*this);
	}
	return *this;
}

void JSchema::swap(JSchema &other)
{
	JResult::swap(other);
	std::swap(schema, other.schema);
}

JSchema::JSchema()
	: schema(NULL)
{
}

JSchema::JSchema(jschema_ref aSchema)
	: schema(aSchema)
{
}

bool JSchema::isInitialized() const
{
	return schema != NULL;
}

jschema_ref JSchema::peek() const
{
	return schema;
}

void JSchema::set(jschema_ref aSchema)
{
	schema = aSchema;
}

JSchema JSchema::fromString(const JInput &input)
{
	JSchema res;
	res.set(jschema_create(input, &res.error));
	return res;
}

JSchema JSchema::fromFile(const char *file)
{
	JSchema res;
	res.set(jschema_fcreate(file, &res.error));
	return res;
}

JSchema JSchema::fromJValue(const JValue &value)
{
	JSchema res;
	res.set(jschema_jcreate(value.m_jval, &res.error));
	return res;
}

bool JSchema::resolve(JResolver &resolver)
{
	JSchemaResolverWrapper resolverWrapper(&resolver);
	JSchemaResolver schemaResolver = {};
	schemaResolver.m_resolve = &(resolverWrapper.sax_schema_resolver);
	schemaResolver.m_userCtxt = &resolverWrapper;

	return jschema_resolve(schema, &schemaResolver);
}

JResult JSchema::validate(const JValue &value) const
{
	JResult res;
	jvalue_validate(value.peekRaw(), schema, &res.error);
	return res;
}

JResult JSchema::apply(JValue &value) const
{
	JResult res;
	jvalue_validate_apply(value.peekRaw(), schema, &res.error);
	return res;
}

}
