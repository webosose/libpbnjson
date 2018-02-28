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

#include <gtest/gtest.h>
#include <pbnjson.h>
#include <string>

#include "../../pbnjson_c/jschema_types_internal.h"
#include "../../pbnjson_c/validation/error_code.h"

#include "TestUtils.hpp"

using namespace std;

class TestSchemaValidationErrorReportingOld : public ::testing::Test
{
protected:
	JErrorCallbacks errors;
	int errorCounter;
	int errorCode;

	TestSchemaValidationErrorReportingOld()
	{
		errors.m_parser = &OnError;
		errors.m_schema = &OnValidationError;
		errors.m_unknown = &OnError;
		errors.m_ctxt = this;
	}

	virtual void SetUp()
	{
		errorCounter = 0;
		errorCode = VEC_OK;
	}

	static bool OnError(void *ctxt, JSAXContextRef parseCtxt)
	{
		return false;
	}

	static bool OnValidationError(void *ctxt, JSAXContextRef parseCtxt)
	{
		TestSchemaValidationErrorReportingOld *t = reinterpret_cast<TestSchemaValidationErrorReportingOld *>(ctxt);
		assert(t);
		t->errorCounter++;
		t->errorCode = parseCtxt->m_error_code;
		return false;
	}

	bool TestError(const char *schemaStr, const char *json, ValidationErrorCode error)
	{
		SetUp();
		auto schema = mk_ptr(jschema_parse(j_cstr_to_buffer(schemaStr), JSCHEMA_DOM_NOOPT, NULL));
		if (!schema.get())
			return false;

		JSchemaInfo schemaInfo;
		jschema_info_init(&schemaInfo, schema.get(), NULL, &errors);

		EXPECT_FALSE(jis_valid(mk_ptr(jdom_parse(j_cstr_to_buffer(json), DOMOPT_NOOPT, &schemaInfo)).get()));
		EXPECT_EQ(error, errorCode);
		EXPECT_EQ(1, errorCounter);
		return errorCounter == 1;
	}
};

class TestSchemaValidationErrorReporting : public ::testing::Test
{
protected:
	bool TestError(const char *schemaStr, const char *json, ValidationErrorCode errorCode)
	{
		int len = 0;
		char buf[128];
		jerror* error = NULL;

		auto schema = mk_ptr(jschema_create(j_cstr_to_buffer(schemaStr), NULL));
		if (!schema.get())
			return false;

		EXPECT_FALSE(jis_valid(mk_ptr(jdom_create(j_cstr_to_buffer(json), schema.get(), &error)).get()));
		EXPECT_NE(error, nullptr);

		len = jerror_to_string(error, buf, 128);
		jerror_free(error);

		return len != 0;
	}
};

template <typename T>
/* using SchemaTestDispatcher = T; */
class SchemaTestDispatcher : public T
{ };

typedef ::testing::Types<TestSchemaValidationErrorReportingOld, TestSchemaValidationErrorReporting> Implementations;

TYPED_TEST_CASE(SchemaTestDispatcher, Implementations);

TYPED_TEST(SchemaTestDispatcher, Null)
{
	EXPECT_TRUE(this->TestError("{\"type\": \"null\"}", "1", VEC_NOT_NULL));
}

TYPED_TEST(SchemaTestDispatcher, Number)
{
	const char *schema = "{\"type\": \"number\", \"minimum\": 1, \"maximum\": 10 }";
	EXPECT_TRUE(this->TestError(schema, "null", VEC_NOT_NUMBER));
	EXPECT_TRUE(this->TestError(schema, "0", VEC_NUMBER_TOO_SMALL));
	EXPECT_TRUE(this->TestError(schema, "100", VEC_NUMBER_TOO_BIG));
	EXPECT_TRUE(this->TestError("{\"type\": \"integer\"}", "1.2", VEC_NOT_INTEGER_NUMBER));
}

TYPED_TEST(SchemaTestDispatcher, Boolean)
{
	EXPECT_TRUE(this->TestError("{\"type\": \"boolean\"}", "null", VEC_NOT_BOOLEAN));
}

TYPED_TEST(SchemaTestDispatcher, String)
{
	const char *schema = "{\"type\": \"string\", \"minLength\": 3, \"maxLength\": 10 }";
	EXPECT_TRUE(this->TestError(schema, "null", VEC_NOT_STRING));
	EXPECT_TRUE(this->TestError(schema, "\"h\"", VEC_STRING_TOO_SHORT));
	EXPECT_TRUE(this->TestError(schema, "\"hello world\"", VEC_STRING_TOO_LONG));
}

TYPED_TEST(SchemaTestDispatcher, Array)
{
	const char *schema = "{\"type\": \"array\", \"minItems\": 1, \"maxItems\": 3, \"uniqueItems\": true }";
	EXPECT_TRUE(this->TestError(schema, "null", VEC_NOT_ARRAY));
	EXPECT_TRUE(this->TestError(schema, "[]", VEC_ARRAY_TOO_SHORT));
	EXPECT_TRUE(this->TestError(schema, "[1, 2, 3, 4]", VEC_ARRAY_TOO_LONG));
	EXPECT_TRUE(this->TestError(schema, "[1, 1]", VEC_ARRAY_HAS_DUPLICATES));
	EXPECT_TRUE(this->TestError("{\"type\": \"array\", \"items\": [{}], \"additionalItems\": false }", "[1, 1]", VEC_ARRAY_TOO_LONG));
}

TYPED_TEST(SchemaTestDispatcher, Object)
{
	const char *schema = "{\"type\": \"object\", \"minProperties\": 1, \"maxProperties\": 2}";
	EXPECT_TRUE(this->TestError(schema, "null", VEC_NOT_OBJECT));
	EXPECT_TRUE(this->TestError(schema, "{}", VEC_NOT_ENOUGH_KEYS));
	EXPECT_TRUE(this->TestError(schema, "{\"a\": 1, \"b\": 2, \"c\": 3 }", VEC_TOO_MANY_KEYS));
	EXPECT_TRUE(this->TestError("{\"type\": \"object\", \"required\": [\"a\", \"b\"] }", "{\"a\": 1 }", VEC_MISSING_REQUIRED_KEY));
	EXPECT_TRUE(this->TestError("{\"type\": \"object\", \"properties\": {\"a\": {} }, \"additionalProperties\": false }", "{\"a\": 1, \"b\": 2 }", VEC_OBJECT_PROPERTY_NOT_ALLOWED));
}

TYPED_TEST(SchemaTestDispatcher, Types)
{
	EXPECT_TRUE(this->TestError("{\"type\": [\"object\", \"array\"] }", "null", VEC_TYPE_NOT_ALLOWED));
}

TYPED_TEST(SchemaTestDispatcher, Enum)
{
	EXPECT_TRUE(this->TestError("{\"enum\": [1, false] }", "0", VEC_UNEXPECTED_VALUE));
}

TYPED_TEST(SchemaTestDispatcher, AllOf)
{
	EXPECT_TRUE(this->TestError("{\"allOf\": [{}, {\"type\": \"string\"} ] }", "0", VEC_NOT_EVERY_ALL_OF));
}

TYPED_TEST(SchemaTestDispatcher, AnyOf)
{
	EXPECT_TRUE(this->TestError("{\"anyOf\": [{\"type\": \"array\"}, {\"type\": \"string\"} ] }", "0", VEC_NEITHER_OF_ANY));
}

TYPED_TEST(SchemaTestDispatcher, OneOf)
{
	const char *schema = "{\"oneOf\": [{\"enum\": [\"hello\"]}, {\"type\": \"string\"} ] }";
	EXPECT_TRUE(this->TestError(schema, "\"hello\"", VEC_MORE_THAN_ONE_OF));
	EXPECT_TRUE(this->TestError(schema, "null", VEC_NEITHER_OF_ANY));
}

TYPED_TEST(SchemaTestDispatcher, Complex)
{
	EXPECT_TRUE(this->TestError("{\"type\": \"string\", \"enum\": [\"hello\"], \"anyOf\": [{}] }", "\"h\"", VEC_UNEXPECTED_VALUE));
	EXPECT_TRUE(this->TestError("{\"type\": \"array\", \"anyOf\": [{}], \"uniqueItems\": true }", "[1, 1]", VEC_ARRAY_HAS_DUPLICATES));
}
