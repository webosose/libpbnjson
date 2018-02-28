// Copyright (c) 2015-2018 LG Electronics, Inc.
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

#include <jerror.h>

namespace {

class TestNewSchemaArraySanity : public ::testing::Test
{
protected:
	static jschema_ref schema;
	jvalue_ref parsed;

	static void SetUpTestCase()
	{
		schema = jschema_parse(j_cstr_to_buffer(
		   "{"
		      "\"type\": \"array\","
		      "\"items\": { \"type\": \"number\" },"
		      "\"minItems\": 1,"
		      "\"maxItems\": 1"
		    "}"
		), 0, nullptr);

		ASSERT_TRUE(schema != nullptr);
	}

	static void TearDownTestCase()
	{
		jschema_release(&schema);
	}

	virtual void SetUp()
	{
		parsed = nullptr;
	}

	virtual void TearDown()
	{
		j_release(&parsed);
	}
};

	jschema_ref TestNewSchemaArraySanity::schema = nullptr;

} // namespace

static inline void jerror_free_ptr(jerror** err_ptr)
{
	if (err_ptr)
	{
		jerror_free(*err_ptr);
		*err_ptr = nullptr;
	}
}

TEST_F(TestNewSchemaArraySanity, Invalid0)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer(" ");

	parsed = jdom_create(input, schema, &err);
	EXPECT_FALSE(jis_valid(parsed));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);

	EXPECT_FALSE(jvalue_validate(parsed, schema, &err));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaArraySanity, Invalid1)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer("[\"abc\"]");

	parsed = jdom_create(input, schema, &err);
	EXPECT_FALSE(jis_valid(parsed));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);

	EXPECT_FALSE(jvalue_validate(parsed, schema, &err));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaArraySanity, Invalid2)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer("[{}]");

	parsed = jdom_create(input, schema, &err);
	EXPECT_FALSE(jis_valid(parsed));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);

	EXPECT_FALSE(jvalue_validate(parsed, schema, &err));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaArraySanity, Invalid3)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer("[]");

	parsed = jdom_create(input, schema, &err);
	EXPECT_FALSE(jis_valid(parsed));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);

	EXPECT_FALSE(jvalue_validate(parsed, schema, &err));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaArraySanity, Invalid4)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer("[true]");

	parsed = jdom_create(input, schema, &err);
	EXPECT_FALSE(jis_valid(parsed));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);

	EXPECT_FALSE(jvalue_validate(parsed, schema, &err));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaArraySanity, Invalid5)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer("[null]");

	parsed = jdom_create(input, schema, &err);
	EXPECT_FALSE(jis_valid(parsed));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);

	EXPECT_FALSE(jvalue_validate(parsed, schema, &err));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaArraySanity, Invalid6)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer("{}");

	parsed = jdom_create(input, schema, &err);
	EXPECT_FALSE(jis_valid(parsed));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);

	EXPECT_FALSE(jvalue_validate(parsed, schema, &err));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaArraySanity, Valid1)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer("[1]");

	parsed = jdom_create(input, schema, &err);
	EXPECT_TRUE(jis_valid(parsed));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);

	EXPECT_TRUE(jvalue_validate(parsed, schema, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaArraySanity, Valid2)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer("[1.0]");

	parsed = jdom_create(input, schema, &err);
	EXPECT_TRUE(jis_valid(parsed));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);

	EXPECT_TRUE(jvalue_validate(parsed, schema, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaArraySanity, Valid3)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer(
	      "[2394309382309842309234825.62345235323253253220398443213241234"
	      "123431413e90234098320982340924382340982349023423498234908234]"
	      );

	parsed = jdom_create(input, schema, &err);
	EXPECT_TRUE(jis_valid(parsed));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);

	EXPECT_TRUE(jvalue_validate(parsed, schema, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaArraySanity, Valid4)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer("[-50]");

	parsed = jdom_create(input, schema, &err);
	EXPECT_TRUE(jis_valid(parsed));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);

	EXPECT_TRUE(jvalue_validate(parsed, schema, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);
}

// vim: set noet ts=4 sw=4 tw=80:
