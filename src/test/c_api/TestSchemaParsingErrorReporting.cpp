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

#include "../../pbnjson_c/validation/error_code.h"
#include "../../pbnjson_c/jschema_types_internal.h"

#include "TestUtils.hpp"

using namespace std;

class TestSchemaParsingErrorReportingOld : public ::testing::Test
{
protected:
	JErrorCallbacks error;
	int errorCounter;
	int errorCode;
	string errorMsg;

	TestSchemaParsingErrorReportingOld()
	{
		error.m_parser = &OnError;
		error.m_ctxt = this;
	}

	virtual void SetUp()
	{
		errorCounter = 0;
		errorCode = VEC_OK;
		errorMsg = "";
	}

	static bool OnError(void *ctxt, JSAXContextRef parseCtxt)
	{
		TestSchemaParsingErrorReportingOld *t = reinterpret_cast<TestSchemaParsingErrorReportingOld *>(ctxt);
		assert(t);
		t->errorCounter++;
		t->errorCode = parseCtxt->m_error_code;
		t->errorMsg = parseCtxt->errorDescription;
		return false;
	}

	bool TestError(const char *schemaStr, int errorCode)
	{
		SetUp();
		auto schema = mk_ptr(jschema_parse(j_cstr_to_buffer(schemaStr), JSCHEMA_DOM_NOOPT, &error));
		EXPECT_EQ(1, this->errorCounter);
		bool right_error = this->errorCode == errorCode;
		EXPECT_TRUE(right_error);
		// Syntax error could provide YAJL error description message
		if (errorCode != SEC_SYNTAX)
			EXPECT_EQ(this->errorMsg, SchemaGetErrorMessage(errorCode));
		return this->errorCounter == 1 && right_error;
	}
};

class TestSchemaParsingErrorReporting : public ::testing::Test
{
protected:
	bool TestError(const char *schemaStr, int errorCode)
	{
		int len = 0;
		char buf[128];
		jerror *error = NULL;

		auto schema = mk_ptr(jschema_create(j_cstr_to_buffer(schemaStr), &error));
		EXPECT_NE(error, nullptr);

		len = jerror_to_string(error, buf, 128);
		jerror_free(error);

		return len != 0;
	}
};

/* Next declaration is C++11 compliant, but cause "internal compiler error" on gcc-4.9.2
template <typename T>
using SchemaTestDispatcher = T;
*/
template <typename T>
class SchemaTestDispatcher : public T
{ };

typedef ::testing::Types<TestSchemaParsingErrorReportingOld, TestSchemaParsingErrorReporting> Implementations;

TYPED_TEST_CASE(SchemaTestDispatcher, Implementations);

TYPED_TEST(SchemaTestDispatcher, InvalidJson)
{
	EXPECT_TRUE(this->TestError("]", SEC_SYNTAX));
	EXPECT_TRUE(this->TestError("{]", SEC_SYNTAX));
	EXPECT_TRUE(this->TestError("[qwe]", SEC_SYNTAX));
}

TYPED_TEST(SchemaTestDispatcher, InvalidSchemaType1)
{
	EXPECT_TRUE(this->TestError("{ \"type\" : null }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : 0 }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : false }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : {} }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : {\"a\":null} }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [] }", SEC_TYPE_ARRAY_EMPTY));
	EXPECT_TRUE(this->TestError("{ \"type\" : \"invalid_type\" }", SEC_TYPE_VALUE));
	EXPECT_TRUE(this->TestError("{ \"type\" : [null] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [0] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [false] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [{}] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [{\"a\":null}] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [[]] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [[1]] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [\"invalid_type\"] }", SEC_TYPE_VALUE));
	EXPECT_TRUE(this->TestError("{ \"type\" : [\"array\", null] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [\"array\", 0 ] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [\"array\", false ] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [\"array\", {} ] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [\"array\", {\"a\":null} ] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [\"array\", [] ] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [\"array\", [1] ] }", SEC_TYPE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"type\" : [\"array\", \"invalid_type\"] }", SEC_TYPE_VALUE));
	EXPECT_TRUE(this->TestError("{ \"type\" : [\"array\", \"array\"] }", SEC_TYPE_ARRAY_DUPLICATES));
	EXPECT_TRUE(this->TestError("{ \"type\" : [\"integer\", \"number\", \"integer\"] }", SEC_TYPE_ARRAY_DUPLICATES));
	EXPECT_TRUE(this->TestError("{ \"type\" : [\"number\", \"integer\", \"number\"] }", SEC_TYPE_ARRAY_DUPLICATES));
}

TYPED_TEST(SchemaTestDispatcher, InvalidMultipleOf)
{
	EXPECT_TRUE(this->TestError("{ \"multipleOf\" : null }", SEC_MULTIPLE_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"multipleOf\" : false }", SEC_MULTIPLE_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"multipleOf\" : \"mul\" }", SEC_MULTIPLE_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"multipleOf\" : {} }", SEC_MULTIPLE_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"multipleOf\" : [] }", SEC_MULTIPLE_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"multipleOf\" : -1.2 }", SEC_MULTIPLE_OF_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"multipleOf\" : 0 }", SEC_MULTIPLE_OF_VALUE_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidMaximum)
{
	EXPECT_TRUE(this->TestError("{ \"maximum\" : null }", SEC_MAXIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maximum\" : false }", SEC_MAXIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maximum\" : \"max\" }", SEC_MAXIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maximum\" : {} }", SEC_MAXIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maximum\" : [] }", SEC_MAXIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"exclusiveMaximum\" : null }", SEC_EXCLUSIVE_MAXIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"exclusiveMaximum\" : 0 }", SEC_EXCLUSIVE_MAXIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"exclusiveMaximum\" : \"exclusive\" }", SEC_EXCLUSIVE_MAXIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"exclusiveMaximum\" : {} }", SEC_EXCLUSIVE_MAXIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"exclusiveMaximum\" : [] }", SEC_EXCLUSIVE_MAXIMUM_FORMAT));
	// FIXME: if "exclusiveMaximum" is present "maximum" should be present also
	//EXPECT_TRUE(this->TestError("{ \"exclusiveMaximum\" : false }", SEC_SYNTAX));
}

TYPED_TEST(SchemaTestDispatcher, InvalidMinimum)
{
	EXPECT_TRUE(this->TestError("{ \"minimum\" : null }", SEC_MINIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minimum\" : false }", SEC_MINIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minimum\" : \"min\" }", SEC_MINIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minimum\" : {} }", SEC_MINIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minimum\" : [] }", SEC_MINIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"exclusiveMinimum\" : null }", SEC_EXCLUSIVE_MINIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"exclusiveMinimum\" : 0 }", SEC_EXCLUSIVE_MINIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"exclusiveMinimum\" : \"exclusive\" }", SEC_EXCLUSIVE_MINIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"exclusiveMinimum\" : {} }", SEC_EXCLUSIVE_MINIMUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"exclusiveMinimum\" : [] }", SEC_EXCLUSIVE_MINIMUM_FORMAT));
	// FIXME: if "exclusiveMinimum" is present "minimum" should be present also
	//EXPECT_TRUE(this->TestError("{ \"exclusiveMinimum\" : false }", SEC_SYNTAX));
}

TYPED_TEST(SchemaTestDispatcher, InvalidMaxLength)
{
	EXPECT_TRUE(this->TestError("{ \"maxLength\" : null }", SEC_MAX_LENGTH_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxLength\" : false }", SEC_MAX_LENGTH_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxLength\" : \"length\" }", SEC_MAX_LENGTH_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxLength\" : {} }", SEC_MAX_LENGTH_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxLength\" : [] }", SEC_MAX_LENGTH_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxLength\" : 1.2 }", SEC_MAX_LENGTH_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxLength\" : 12e-2 }", SEC_MAX_LENGTH_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxLength\" : -1 }", SEC_MAX_LENGTH_VALUE_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidMinLength)
{
	EXPECT_TRUE(this->TestError("{ \"minLength\" : null }", SEC_MIN_LENGTH_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minLength\" : false }", SEC_MIN_LENGTH_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minLength\" : \"length\" }", SEC_MIN_LENGTH_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minLength\" : {} }", SEC_MIN_LENGTH_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minLength\" : [] }", SEC_MIN_LENGTH_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minLength\" : 1.2 }", SEC_MIN_LENGTH_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minLength\" : 12e-2 }", SEC_MIN_LENGTH_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minLength\" : -1 }", SEC_MIN_LENGTH_VALUE_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidPattern)
{
	EXPECT_TRUE(this->TestError("{ \"pattern\" : null }", SEC_PATTERN_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"pattern\" : false }", SEC_PATTERN_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"pattern\" : 0 }", SEC_PATTERN_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"pattern\" : {} }", SEC_PATTERN_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"pattern\" : [] }", SEC_PATTERN_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"pattern\" : \"*\" }", SEC_PATTERN_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"pattern\" : \"[\" }", SEC_PATTERN_VALUE_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidItems)
{
	EXPECT_TRUE(this->TestError("{ \"items\" : null }", SEC_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"items\" : 0 }", SEC_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"items\" : false }", SEC_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"items\" : \"item\" }", SEC_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"items\" : [null] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"items\" : [0] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"items\" : [false] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"items\" : [\"item\"] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"items\" : [{}, null] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"items\" : [{}, 0] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"items\" : [{}, false] }", SEC_ITEMS_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"items\" : [{}, \"item\"] }", SEC_ITEMS_ARRAY_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidAdditionalItems)
{
	EXPECT_TRUE(this->TestError("{ \"additionalItems\" : null }", SEC_ADDITIONAL_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"additionalItems\" : 0 }", SEC_ADDITIONAL_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"additionalItems\" : \"item\" }", SEC_ADDITIONAL_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"additionalItems\" : [] }", SEC_ADDITIONAL_ITEMS_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidMaxItems)
{
	EXPECT_TRUE(this->TestError("{ \"maxItems\" : null }", SEC_MAX_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxItems\" : false }", SEC_MAX_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxItems\" : \"max\" }", SEC_MAX_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxItems\" : {} }", SEC_MAX_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxItems\" : [] }", SEC_MAX_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxItems\" : 1.2 }", SEC_MAX_ITEMS_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxItems\" : 12e-2 }", SEC_MAX_ITEMS_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxItems\" : -1 }", SEC_MAX_ITEMS_VALUE_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidMinItems)
{
	EXPECT_TRUE(this->TestError("{ \"minItems\" : null }", SEC_MIN_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minItems\" : false }", SEC_MIN_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minItems\" : \"min\" }", SEC_MIN_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minItems\" : {} }", SEC_MIN_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minItems\" : [] }", SEC_MIN_ITEMS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minItems\" : 1.2 }", SEC_MIN_ITEMS_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minItems\" : 12e-2 }", SEC_MIN_ITEMS_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minItems\" : -1 }", SEC_MIN_ITEMS_VALUE_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidUniqueItems)
{
	EXPECT_TRUE(this->TestError("{ \"uniqueItems\" : null }", SEC_UNIQUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"uniqueItems\" : 0 }", SEC_UNIQUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"uniqueItems\" : \"unique\" }", SEC_UNIQUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"uniqueItems\" : {} }", SEC_UNIQUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"uniqueItems\" : [] }", SEC_UNIQUE_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidMaxProperties)
{
	EXPECT_TRUE(this->TestError("{ \"maxProperties\" : null }", SEC_MAX_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxProperties\" : false }", SEC_MAX_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxProperties\" : \"max\" }", SEC_MAX_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxProperties\" : {} }", SEC_MAX_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxProperties\" : [] }", SEC_MAX_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxProperties\" : 1.2 }", SEC_MAX_PROPERTIES_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxProperties\" : 12e-2 }", SEC_MAX_PROPERTIES_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"maxProperties\" : -1 }", SEC_MAX_PROPERTIES_VALUE_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidMinProperties)
{
	EXPECT_TRUE(this->TestError("{ \"minProperties\" : null }", SEC_MIN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minProperties\" : false }", SEC_MIN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minProperties\" : \"min\" }", SEC_MIN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minProperties\" : {} }", SEC_MIN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minProperties\" : [] }", SEC_MIN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minProperties\" : 1.2 }", SEC_MIN_PROPERTIES_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minProperties\" : 12e-2 }", SEC_MIN_PROPERTIES_VALUE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"minProperties\" : -1 }", SEC_MIN_PROPERTIES_VALUE_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidRequired)
{
	EXPECT_TRUE(this->TestError("{ \"required\" : null }", SEC_REQUIRED_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : false }", SEC_REQUIRED_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : 0 }", SEC_REQUIRED_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : \"req\" }", SEC_REQUIRED_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : {} }", SEC_REQUIRED_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : [] }", SEC_REQUIRED_ARRAY_EMPTY));
	EXPECT_TRUE(this->TestError("{ \"required\" : [null] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : [false] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : [0] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : [{}] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : [[]] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : [\"a\", null] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : [\"a\", false] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : [\"a\", 0] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : [\"a\", {}] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : [\"a\", []] }", SEC_REQUIRED_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"required\" : [\"a\", \"a\"] }", SEC_REQUIRED_ARRAY_DUPLICATES));
}

TYPED_TEST(SchemaTestDispatcher, InvalidAdditionalProperties)
{
	EXPECT_TRUE(this->TestError("{ \"additionalProperties\" : null }", SEC_ADDITIONAL_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"additionalProperties\" : 0 }", SEC_ADDITIONAL_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"additionalProperties\" : \"item\" }", SEC_ADDITIONAL_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"additionalProperties\" : [] }", SEC_ADDITIONAL_PROPERTIES_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidProperties)
{
	EXPECT_TRUE(this->TestError("{ \"properties\" : null }", SEC_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : false }", SEC_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : 0 }", SEC_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : \"item\" }", SEC_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : [] }", SEC_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : { \"a\" : null } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : { \"a\" : false } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : { \"a\" : 0 } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : { \"a\" : \"hello\" } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : { \"a\" : [] } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : { \"a\" : {}, \"b\" : null } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : { \"a\" : {}, \"b\" : false } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : { \"a\" : {}, \"b\" : 0 } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : { \"a\" : {}, \"b\" : \"hello\" } }", SEC_PROPERTIES_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"properties\" : { \"a\" : {}, \"b\" : [] } }", SEC_PROPERTIES_OBJECT_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidPatternProperties)
{
	EXPECT_TRUE(this->TestError("{ \"patternProperties\" : null }", SEC_PATTERN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"patternProperties\" : false }", SEC_PATTERN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"patternProperties\" : 0 }", SEC_PATTERN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"patternProperties\" : \"item\" }", SEC_PATTERN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"patternProperties\" : [] }", SEC_PATTERN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"patternProperties\" : { \"a\" : null } }", SEC_PATTERN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"patternProperties\" : { \"a\" : false } }", SEC_PATTERN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"patternProperties\" : { \"a\" : 0 } }", SEC_PATTERN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"patternProperties\" : { \"a\" : \"hello\" } }", SEC_PATTERN_PROPERTIES_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"patternProperties\" : { \"a\" : [] } }", SEC_PATTERN_PROPERTIES_FORMAT));

	jerror *error = nullptr;
	auto schema = mk_ptr(jschema_create(j_cstr_to_buffer(R"({"patternProperties": {"^(/[^/]+)+$": {}}})"), &error));
	EXPECT_EQ(error, nullptr);
}

// TODO: Implement "dependencies" property
//
//TYPED_TEST(SchemaTestDispatcher, InvalidDependencies)
//{
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : null }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : false }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : 0 }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : \"dep\" }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : [] }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : null } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : false } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : 0 } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : \"dep\" } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : [] } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : [null] } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : [false] } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : [0] } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : [{}] } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : [[]] } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : [\"dep\", null] } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : [\"dep\", false] } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : [\"dep\", 0] } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : [\"dep\", {}] } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : [\"dep\", []] } }", SEC_SYNTAX));
//	EXPECT_TRUE(this->TestError("{ \"dependencies\" : { \"a\" : [\"dep\", \"dep\"] } }", SEC_SYNTAX));
//}

TYPED_TEST(SchemaTestDispatcher, InvalidEnum)
{
	EXPECT_TRUE(this->TestError("{ \"enum\" : null }", SEC_ENUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"enum\" : false }", SEC_ENUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"enum\" : 0 }", SEC_ENUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"enum\" : \"e\" }", SEC_ENUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"enum\" : {} }", SEC_ENUM_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"enum\" : [] }", SEC_ENUM_ARRAY_EMPTY));
	EXPECT_TRUE(this->TestError("{ \"enum\" : [null, null] }", SEC_ENUM_ARRAY_DUPLICATES));
	EXPECT_TRUE(this->TestError("{ \"enum\" : [\"hello\", null, \"hello\"] }", SEC_ENUM_ARRAY_DUPLICATES));
	EXPECT_TRUE(this->TestError("{ \"enum\" : [[], []] }", SEC_ENUM_ARRAY_DUPLICATES));
	EXPECT_TRUE(this->TestError("{ \"enum\" : [[null, 12], 12, [null, 12]] }", SEC_ENUM_ARRAY_DUPLICATES));
	EXPECT_TRUE(this->TestError("{ \"enum\" : [{}, {}] }", SEC_ENUM_ARRAY_DUPLICATES));
	EXPECT_TRUE(this->TestError("{ \"enum\" : [ {\"a\":null, \"b\":\"str\"}, null, {\"b\":\"str\", \"a\":null } ] }", SEC_ENUM_ARRAY_DUPLICATES));
}

TYPED_TEST(SchemaTestDispatcher, InvalidAllOf)
{
	EXPECT_TRUE(this->TestError("{ \"allOf\" : null }", SEC_ALL_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : false }", SEC_ALL_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : 0 }", SEC_ALL_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : \"a\" }", SEC_ALL_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : {} }", SEC_ALL_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : [] }", SEC_ALL_OF_ARRAY_EMPTY));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : [null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : [false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : [0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : [\"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : [[]] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : [{}, null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : [{}, false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : [{}, 0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : [{}, \"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"allOf\" : [{}, []] }", SEC_COMBINATOR_ARRAY_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidAnyOf)
{
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : null }", SEC_ANY_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : false }", SEC_ANY_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : 0 }", SEC_ANY_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : \"a\" }", SEC_ANY_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : {} }", SEC_ANY_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : [] }", SEC_ANY_OF_ARRAY_EMPTY));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : [null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : [false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : [0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : [\"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : [[]] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : [{}, null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : [{}, false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : [{}, 0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : [{}, \"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"anyOf\" : [{}, []] }", SEC_COMBINATOR_ARRAY_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidOneOf)
{
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : null }", SEC_ONE_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : false }", SEC_ONE_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : 0 }", SEC_ONE_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : \"a\" }", SEC_ONE_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : {} }", SEC_ONE_OF_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : [] }", SEC_ONE_OF_ARRAY_EMPTY));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : [null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : [false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : [0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : [\"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : [[]] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : [{}, null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : [{}, false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : [{}, 0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : [{}, \"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"oneOf\" : [{}, []] }", SEC_COMBINATOR_ARRAY_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidNot)
{
	EXPECT_TRUE(this->TestError("{ \"not\" : null }", SEC_NOT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"not\" : false }", SEC_NOT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"not\" : 0 }", SEC_NOT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"not\" : \"a\" }", SEC_NOT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"not\" : [] }", SEC_NOT_ARRAY_EMPTY));
	EXPECT_TRUE(this->TestError("{ \"not\" : [null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"not\" : [false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"not\" : [0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"not\" : [\"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"not\" : [[]] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"not\" : [{}, null] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"not\" : [{}, false] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"not\" : [{}, 0] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"not\" : [{}, \"a\"] }", SEC_COMBINATOR_ARRAY_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"not\" : [{}, []] }", SEC_COMBINATOR_ARRAY_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidDefinitions)
{
	EXPECT_TRUE(this->TestError("{ \"definitions\" : null }", SEC_DEFINITIONS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : false }", SEC_DEFINITIONS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : 0 }", SEC_DEFINITIONS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : \"def\" }", SEC_DEFINITIONS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : [] }", SEC_DEFINITIONS_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : { \"a\" : null } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : { \"a\" : false } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : { \"a\" : 0 } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : { \"a\" : \"hello\" } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : { \"a\" : [] } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : null } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : false } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : 0 } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : \"hello\" } }", SEC_DEFINITIONS_OBJECT_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"definitions\" : { \"a\" : {}, \"b\" : [] } }", SEC_DEFINITIONS_OBJECT_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidTitle)
{
	EXPECT_TRUE(this->TestError("{ \"title\" : null }", SEC_TITLE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"title\" : false }", SEC_TITLE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"title\" : 0 }", SEC_TITLE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"title\" : {} }", SEC_TITLE_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"title\" : [] }", SEC_TITLE_FORMAT));
}

TYPED_TEST(SchemaTestDispatcher, InvalidDescription)
{
	EXPECT_TRUE(this->TestError("{ \"description\" : null }", SEC_DESCRIPTION_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"description\" : false }", SEC_DESCRIPTION_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"description\" : 0 }", SEC_DESCRIPTION_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"description\" : {} }", SEC_DESCRIPTION_FORMAT));
	EXPECT_TRUE(this->TestError("{ \"description\" : [] }", SEC_DESCRIPTION_FORMAT));
}
