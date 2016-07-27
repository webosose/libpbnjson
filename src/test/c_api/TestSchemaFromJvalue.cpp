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
#include "pbnjson.hpp"
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

	auto jvalue_dup = [](jvalue_ref value) {
		return mk_ptr(
			jvalue_duplicate(value)
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

	auto native_jval_repr = [](pbnjson::JValue &repr) {
		return mk_ptr(jvalue_copy(repr.peekRaw()));
	};

	auto parse_jval_repr = [](pbnjson::JValue &repr) {
		return jvalue_str(jvalue_tostring(repr.peekRaw(), jschema_all()));
	};

	typedef decltype(jvalue_str(nullptr)) jvalue_t;
	typedef decltype(schema_str(nullptr)) schema_t;
	typedef function< schema_t(jvalue_ref) > schema_jvalue_t;
	typedef function< jvalue_t(pbnjson::JValue &) > repr_t;
} // anonymous namespace

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::Combine;

class SchemaFromJvalue : public TestWithParam<tr1::tuple<pbnjson::JValue, const char *, schema_jvalue_t, repr_t>>
{ };

TEST_P(SchemaFromJvalue, Basic)
{
	auto param = GetParam();
	static_assert( 4 == tr1::tuple_size<decltype(param)>::value, "param should be a tuple of 4 values" );
	auto schema_repr = get<0>(param);
	auto json = get<1>(param);
	auto schema_jvalue_f = get<2>(param);
	auto repr_f = get<3>(param);

	const char *schema = jvalue_tostring(schema_repr.peekRaw(), jschema_all());
	ASSERT_TRUE( !!schema );

	auto schema_s = schema_str(schema);
	ASSERT_TRUE( !!schema_s );

	auto parsed_schema = repr_f(schema_repr);
	ASSERT_TRUE( !!parsed_schema );

	auto schema_j = schema_jvalue_f(parsed_schema.get());
	ASSERT_TRUE( !!schema_j );

	auto value_s = jvalue_str(json, schema_s.get());
	auto value_j = jvalue_str(json, schema_j.get());
	auto value = jvalue_str(json);
	auto value_apply_s = jvalue_dup(value.get()); // need independent value
	auto value_apply_j = jvalue_dup(value.get());

	// note that jinvalid() have same representation with jnull(), so we'll
	// check it separately
	EXPECT_EQ( jis_valid(value_s.get()), jis_valid(value_j.get()) )
		<< "There should be no difference in parsing with schema built from string and from jvalue should";
	EXPECT_EQ( jis_valid(value_s.get()), jis_valid(value.get()) && jvalue_validate(value.get(), schema_s.get(), nullptr) )
		<< "Result of validation against schema during parsing from text and after should give same results";
	EXPECT_EQ( jis_valid(value_s.get()), jis_valid(value.get()) && jvalue_validate(value.get(), schema_j.get(), nullptr) )
		<< "Validation against schema built from text during parsing shouldn't be different"
		<< " from validation of jvalue against schema built from jvalue";
	EXPECT_EQ( jis_valid(value_s.get()), jis_valid(value_apply_s.get()) && jvalue_validate_apply(value_apply_s.get(), schema_s.get(), nullptr) )
		<< "Result of validation against schema during parsing from text and after should give same results";
	EXPECT_EQ( jis_valid(value_s.get()), jis_valid(value_apply_j.get()) && jvalue_validate_apply(value_apply_j.get(), schema_j.get(), nullptr) )
		<< "Validation against schema built from text during parsing shouldn't be different"
		<< " from validation of jvalue against schema built from jvalue";

	// Now let's compare values (if they are different from jinvalid())
	EXPECT_TRUE( jvalue_equal(value_s.get(), value_j.get()) )
		<< "Expecting " << jvalue_tostring(value_s.get(), jschema_all())
		<< ", but got " << jvalue_tostring(value_j.get(), jschema_all());

	if (jis_valid(value_s.get()))
	{
		// Note that value_apply_* will be a valid jvalue as long as we get a
		// valid json on input regardless of validation against schema.
		EXPECT_TRUE( jvalue_equal(value_s.get(), value_apply_s.get()) )
			<< "Expecting " << jvalue_tostring(value_s.get(), jschema_all())
			<< ", but got " << jvalue_tostring(value_apply_s.get(), jschema_all());

		EXPECT_STREQ( jvalue_tostring(value_s.get(), jschema_all()),
		              jvalue_tostring(value_apply_s.get(), jschema_all()) );

		EXPECT_TRUE( jvalue_equal(value_s.get(), value_apply_j.get()) )
			<< "Expecting " << jvalue_tostring(value_s.get(), jschema_all())
			<< ", but got " << jvalue_tostring(value_apply_j.get(), jschema_all());

		EXPECT_STREQ( jvalue_tostring(value_s.get(), jschema_all()),
		              jvalue_tostring(value_apply_j.get(), jschema_all()) );
	}
}

using jval = pbnjson::JValue;
using jobj = pbnjson::JObject;
using jarr = pbnjson::JArray;

// cartesian product of schemas vs json values
INSTANTIATE_TEST_CASE_P(Samples, SchemaFromJvalue,
	Combine(
		Values(
			jobj {},
			jobj {{"type", "integer"}},
			jobj {{"type", "array"}, {"uniqueItems", true}},
			jobj {{"properties", {{"foo", {{"default", false}}}}}},
			jobj {{"properties", {{"foo", {{"type", "integer"}}}}}},
			jobj {{"type", "object"}, {"properties", {{"foo", {{"type", "integer"}, {"default", 3}}}}}},
			jobj {{"type", "object"}, {"properties", {{"foo", {{"type", "number"}, {"default", 0.5}}}}}},
			jobj {{"oneOf", jarr {{{"type", "string"}}, {{"type", "integer"}}}}},
			jobj {
				{"definitions", {
					{"foo", {{ "$ref", "#/definitions/bar" }}},
					{"bar", {{ "type", "integer" }}}
				}},
				{"oneOf", jarr {{{"$ref", "#/definitions/foo" }}}}
			},
			jobj {{"additionalProperties", {{"type", "integer"}}}}
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
		),
		Values(
			repr_t(native_jval_repr), // use jvalue as-is
			repr_t(parse_jval_repr) // obtain jvalue through stringify->parse
		)
	)
);
