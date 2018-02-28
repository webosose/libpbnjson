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

using namespace std;

namespace {

class TestNumberSanityOld : public ::testing::Test
{
protected:
	jschema_ref schema;
	JSchemaInfo schema_info;
	jvalue_ref parsed;

	virtual void SetUp()
	{
		schema = jschema_parse(j_cstr_to_buffer(
			"{"
				"\"type\": \"array\","
				"\"items\": { \"type\": \"number\" },"
				"\"minItems\": 1,"
				"\"maxItems\": 1"
			"}"
			), 0, NULL);

		ASSERT_TRUE(schema != NULL);
		jschema_info_init(&schema_info, schema, NULL, NULL);
		parsed = NULL;
	}

	virtual void TearDown()
	{
		j_release(&parsed);
		jschema_release(&schema);
	}

	bool sax_parse(const raw_buffer& in)
	{
		return jsax_parse_ex(NULL, in, &schema_info, NULL);
	}

	void dom_parse(const raw_buffer& in)
	{
		parsed = jdom_parse(in, DOMOPT_NOOPT, &schema_info);
	}

	bool check()
	{
		return jvalue_check_schema(parsed, &schema_info);
	}

	void valid(const raw_buffer& in)
	{
		EXPECT_TRUE(sax_parse(in));
		dom_parse(in);
		EXPECT_TRUE(jis_array(parsed));
		EXPECT_TRUE(check());
	}

	void invalid(const raw_buffer& in)
	{
		EXPECT_FALSE(sax_parse(in));
		dom_parse(in);
		EXPECT_FALSE(jis_valid(parsed));
	}
};

class TestNumberSanity : public ::testing::Test
{
protected:
	jschema_ref schema;
	jvalue_ref parsed;

	virtual void SetUp()
	{
		schema = jschema_parse(j_cstr_to_buffer(
			"{"
				"\"type\": \"array\","
				"\"items\": { \"type\": \"number\" },"
				"\"minItems\": 1,"
				"\"maxItems\": 1"
			"}"
			), 0, NULL);

		ASSERT_TRUE(schema != NULL);
		parsed = NULL;
	}

	virtual void TearDown()
	{
		j_release(&parsed);
		jschema_release(&schema);
	}

	bool sax_parse(const raw_buffer& in)
	{
		return jsax_parse_with_callbacks(in, schema, NULL, NULL, NULL);
	}

	void dom_parse(const raw_buffer& in)
	{
		parsed = jdom_create(in, schema, NULL);
	}

	bool check()
	{
		return jvalue_validate(parsed, schema, NULL);
	}

	void valid(const raw_buffer& in)
	{
		EXPECT_TRUE(sax_parse(in));
		dom_parse(in);
		EXPECT_TRUE(jis_array(parsed));
		EXPECT_TRUE(check());
	}

	void invalid(const raw_buffer& in)
	{
		EXPECT_FALSE(sax_parse(in));
		dom_parse(in);
		EXPECT_FALSE(jis_valid(parsed));
	}
};

} // namespace

template <typename T>
/* using SchemaTestDispatcher = T; */
class SchemaTestDispatcher : public T
{ };

typedef ::testing::Types<TestNumberSanityOld, TestNumberSanity> Implementations;

TYPED_TEST_CASE(SchemaTestDispatcher, Implementations);

TYPED_TEST(SchemaTestDispatcher, Invalid0)
{
	const raw_buffer INPUT = j_cstr_to_buffer(" ");

	EXPECT_FALSE(this->sax_parse(INPUT));
}

TYPED_TEST(SchemaTestDispatcher, Invalid1)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[\"abc\"]");

	this->invalid(INPUT);
}

TYPED_TEST(SchemaTestDispatcher, Invalid2)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[{}]");

	this->invalid(INPUT);
}

TYPED_TEST(SchemaTestDispatcher, Invalid3)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[]");

	this->invalid(INPUT);
}

TYPED_TEST(SchemaTestDispatcher, Invalid4)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[true]");

	this->invalid(INPUT);
}

TYPED_TEST(SchemaTestDispatcher, Invalid5)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[null]");

	this->invalid(INPUT);
}

TYPED_TEST(SchemaTestDispatcher, Invalid6)
{
	const raw_buffer INPUT = j_cstr_to_buffer("{}");

	this->invalid(INPUT);
}

TYPED_TEST(SchemaTestDispatcher, Valid1)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[1]");

	this->valid(INPUT);
}

TYPED_TEST(SchemaTestDispatcher, Valid2)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[1.0]");

	this->valid(INPUT);
}

TYPED_TEST(SchemaTestDispatcher, Valid3)
{
	const raw_buffer INPUT = j_cstr_to_buffer(
		"[2394309382309842309234825.62345235323253253220398443213241234"
		"123431413e90234098320982340924382340982349023423498234908234]"
		);

	this->valid(INPUT);
}

TYPED_TEST(SchemaTestDispatcher, Valid4)
{
	const raw_buffer INPUT = j_cstr_to_buffer("[-50]");

	this->valid(INPUT);
}

// vim: set noet ts=4 sw=4 tw=80:
