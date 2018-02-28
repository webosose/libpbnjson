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

class TestUniqueItemsOld : public ::testing::Test
{
protected:
	jschema_ref schema;
	JSchemaInfo schema_info;
	JSchemaInfo schema_info_all;

	JErrorCallbacks errors;
	int errorCounter;
	int errorCode;

	TestUniqueItemsOld()
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

		schema = jschema_parse(j_cstr_to_buffer(
			"{"
				"\"type\": \"array\","
				"\"uniqueItems\": true"
			"}"
			), 0, NULL);

		ASSERT_TRUE(schema != NULL);
		jschema_info_init(&schema_info, schema, NULL, &errors);
		jschema_info_init(&schema_info_all, jschema_all(), NULL, NULL);
	}

	virtual void TearDown()
	{
		jschema_release(&schema);
	}

	static bool OnError(void *ctxt, JSAXContextRef parseCtxt)
	{
		return false;
	}

	static bool OnValidationError(void *ctxt, JSAXContextRef parseCtxt)
	{
		TestUniqueItemsOld *t = reinterpret_cast<TestUniqueItemsOld *>(ctxt);
		assert(t);
		t->errorCounter++;
		t->errorCode = parseCtxt->m_error_code;
		return false;
	}

	auto parse_json(raw_buffer in, bool strict = true) -> decltype(mk_ptr(static_cast<jvalue_ref>(nullptr)))
	{
		return mk_ptr(jdom_parse(in, DOMOPT_NOOPT, strict ? &schema_info : &schema_info_all));
	}

	bool check_jvalue(jvalue_ref v)
	{
		return jvalue_check_schema(v, &schema_info);
	}
};

class TestUniqueItems : public ::testing::Test
{
protected:
	jschema_ref schema;

	int errorCounter;
	int errorCode;

	TestUniqueItems()
	{ }

	virtual void SetUp()
	{
		errorCounter = 0;
		errorCode = VEC_OK;

		schema = jschema_create(j_cstr_to_buffer(
			"{"
				"\"type\": \"array\","
				"\"uniqueItems\": true"
			"}"
			), NULL);

		ASSERT_TRUE(schema != NULL);
	}

	virtual void TearDown()
	{
		jschema_release(&schema);
	}

	auto parse_json(raw_buffer in, bool strict = true) -> decltype(mk_ptr(static_cast<jvalue_ref>(nullptr)))
	{
		return mk_ptr(jdom_create(in, strict ? schema : jschema_all(), NULL));
	}

	bool check_jvalue(jvalue_ref v)
	{
		jerror* error = NULL;
		bool res = jvalue_validate(v, schema, &error);
		if (error != NULL) {
			// XXX new interface doesn't have error codes, so set up it
			// manually for test purpose
			errorCounter++;
			errorCode = VEC_ARRAY_HAS_DUPLICATES;
		}
		jerror_free(error);
		return res;
	}
};

template <typename T>
/* using SchemaTestDispatcher = T; */
class SchemaTestDispatcher : public T
{ };

typedef ::testing::Types<TestUniqueItemsOld, TestUniqueItems> Implementations;

TYPED_TEST_CASE(SchemaTestDispatcher, Implementations);

TYPED_TEST(SchemaTestDispatcher, Valid0)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[]");
	auto res = this->parse_json(INPUT);
	EXPECT_TRUE(jis_valid(res.get()));
	EXPECT_TRUE(this->check_jvalue(res.get()));
}

TYPED_TEST(SchemaTestDispatcher, Valid1)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[null]");
	auto res = this->parse_json(INPUT);
	EXPECT_TRUE(jis_valid(res.get()));
	EXPECT_TRUE(this->check_jvalue(res.get()));
}

TYPED_TEST(SchemaTestDispatcher, Valid2)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[null, true]");
	auto res = this->parse_json(INPUT);
	EXPECT_TRUE(jis_valid(res.get()));
	EXPECT_TRUE(this->check_jvalue(res.get()));
}

TYPED_TEST(SchemaTestDispatcher, Valid3)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[null, true, false]");
	auto res = this->parse_json(INPUT);
	EXPECT_TRUE(jis_valid(res.get()));
	EXPECT_TRUE(this->check_jvalue(res.get()));
}

TYPED_TEST(SchemaTestDispatcher, Invalid1)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[null, true, 1, false, 1]");
	auto res = this->parse_json(INPUT);
	EXPECT_FALSE(jis_valid(res.get()));
	res = this->parse_json(INPUT, false);
	ASSERT_TRUE(jis_array(res.get()));
	this->errorCounter = 0;
	EXPECT_FALSE(this->check_jvalue(res.get()));
	EXPECT_EQ(1, this->errorCounter);
	EXPECT_EQ(VEC_ARRAY_HAS_DUPLICATES, this->errorCode);
}

TYPED_TEST(SchemaTestDispatcher, Invalid2)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[null, true, 1, true, 0]");
	auto res = this->parse_json(INPUT);
	EXPECT_FALSE(jis_valid(res.get()));
	res = this->parse_json(INPUT, false);
	ASSERT_TRUE(jis_array(res.get()));
	this->errorCounter = 0;
	EXPECT_FALSE(this->check_jvalue(res.get()));
	EXPECT_EQ(1, this->errorCounter);
	EXPECT_EQ(VEC_ARRAY_HAS_DUPLICATES, this->errorCode);
}

TYPED_TEST(SchemaTestDispatcher, Invalid3)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[{\"a\":1, \"b\":\"hello\"}, true, {\"b\":\"hello\", \"a\":1}, 0]");
	auto res = this->parse_json(INPUT);
	EXPECT_FALSE(jis_valid(res.get()));
	res = this->parse_json(INPUT, false);
	ASSERT_TRUE(jis_array(res.get()));
	this->errorCounter = 0;
	EXPECT_FALSE(this->check_jvalue(res.get()));
	EXPECT_EQ(1, this->errorCounter);
	EXPECT_EQ(VEC_ARRAY_HAS_DUPLICATES, this->errorCode);
}
