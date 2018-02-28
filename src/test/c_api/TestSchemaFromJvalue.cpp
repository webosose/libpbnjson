// Copyright (c) 2014-2018 LG Electronics, Inc.
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

/**
 *  @file TestSchemaFromJvalue.cpp
 */

#include <gtest/gtest.h>

#include "pbnjson.h"
#include "../../pbnjson_c/validation/validator.h"

#include "TestUtils.hpp"

using namespace std;

namespace {

	auto schema_str = [](const char *schemaStr) {
		return mk_ptr(
			jschema_create(j_cstr_to_buffer(schemaStr), nullptr)
		);
	};

	auto jvalue_str = [](const char *jsonStr, jschema_ref schema = jschema_all()) {
		return mk_ptr(
			jdom_create(j_cstr_to_buffer(jsonStr), schema, nullptr)
		);
	};

	auto schema_jvalue_old = [](jvalue_ref value) {
		return mk_ptr(
			jschema_parse_jvalue(value, nullptr, "")
		);
	};

	auto schema_jvalue = [](jvalue_ref value) {
		return mk_ptr(
			jschema_jcreate(value, nullptr)
		);
	};

	typedef function< decltype(schema_jvalue(nullptr))(jvalue_ref) > schema_jvalue_t;
} // anonymous namespace

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::Combine;

class SchemaFromJvalue : public TestWithParam<tr1::tuple<const char *, const char *, schema_jvalue_t>>
{ };

TEST_P(SchemaFromJvalue, Basic)
{
	auto param = GetParam();
	static_assert( 3 == tr1::tuple_size<decltype(param)>::value, "param should be a tuple of 3 values" );
	auto schema = get<0>(param);
	auto json = get<1>(param);
	auto schema_jvalue_f = get<2>(param);

	auto schema_s = schema_str(schema);
	ASSERT_TRUE( !!schema_s );

	auto parsed_schema = jvalue_str(schema);
	ASSERT_TRUE( !!parsed_schema );

	auto schema_j = schema_jvalue_f(parsed_schema.get());
	ASSERT_TRUE( !!schema_j );

	auto value_s = jvalue_str(json, schema_s.get());
	auto value_j = jvalue_str(json, schema_j.get());

	// note that jerror() have same representation with null, so we'll check it
	// separately
	EXPECT_EQ( jis_valid(value_s.get()), jis_valid(value_j.get()) );

	// we do a special comparison for null values
	if (jis_null(value_s.get()) || jis_null(value_j.get()))
	{
		EXPECT_EQ( jis_null(value_s.get()), jis_null(value_j.get()) );
	}
	else // fill for non-null values only
	{
		// we relay on the fact that different results have different string
		// representations using strings gives nicier output
		const char *str_s = jvalue_tostring(value_s.get(), jschema_all());
		const char *str_j = jvalue_tostring(value_j.get(), jschema_all());

		EXPECT_STREQ( str_s, str_j );
	}
}

// cartesian product of schemas vs json values
INSTANTIATE_TEST_CASE_P(Samples, SchemaFromJvalue,
	Combine(
		Values(
			"{}",
			"{\"type\":\"integer\"}",
			"{\"type\":\"array\", \"uniqueItems\":true}",
			"{\"properties\":{\"foo\":{\"default\":false}}}",
			"{\"properties\":{\"foo\":{\"type\":\"integer\"}}}",
			"{\"oneOf\":[{\"type\":\"string\"},{\"type\":\"integer\"}]}",
			"{\"definitions\":{\"foo\":{\"$ref\":\"#/definitions/bar\"},\"bar\":{\"type\":\"integer\"}},\"oneOf\":[{\"$ref\":\"#/definitions/foo\"}]}",
			"{\"additionalProperties\":{\"type\":\"integer\"}}"
		),
		Values(
			"1",
			"3.14",
			"true",
			"null",
			"[1,2,3]",
			"[1,2,3,1]",
			"[null,true]",
			"{}",
			"{\"foo\":1}",
			"{\"foo\":true}",
			"{\"foo\":true, \"bar\":40}",
			"{\"bar\":40}"
		),
		Values(
			schema_jvalue_t(schema_jvalue_old),
			schema_jvalue_t(schema_jvalue)
		)
	)
);
