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
#include <memory>
#include <vector>
#include <string>
#include <iostream>

#include "Utils.hpp"

using namespace std;

static jvalue_ref json = []()
{
	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"({"enum": 3, "ebool": true, "estr": "str", "enull": null,)"
	                                               R"("twotype": "str", "eobj": {"ch1": 5, "twotype": true},)"
	                                               R"("earray": [6, "brdm"], "complex key": 42,)"
	                                               R"("complex.!@#key_2__,&*&@124": 422})"),
	                             jschema_all(),
	                             &err);

	return json;
}();

TEST(Selectors, TestKeySelector)
{
	jerror *err = NULL;

	ASSERT_TRUE(jis_valid(json));

	// Key with different size
	jvalue_ref result = getFirstQueryResult(".two", json, &err);
	ASSERT_FALSE(jis_valid(result));

	result = getFirstQueryResult(".twotype_ex", json, &err);
	ASSERT_FALSE(jis_valid(result));

	// A number
	result = getFirstQueryResult(".enum", json, &err);
	ASSERT_TRUE(jis_number(result));
	int32_t num;
	jnumber_get_i32(result, &num);
	ASSERT_EQ(3, num);

	// A string
	result = getFirstQueryResult(".estr", json, &err);
	ASSERT_TRUE(jis_string(result));
	const char *resstr = jstring_get_fast(result).m_str;
	ASSERT_STREQ(resstr, "str");

	// An object
	jvalue_ref json_obj = jdom_create(j_cstr_to_buffer(R"({"ch1": 5, "twotype": true})"),
	                                 jschema_all(),
	                                 &err);
	ASSERT_TRUE(jis_valid(json_obj));

	result = getFirstQueryResult(".eobj", json, &err);
	ASSERT_TRUE(jvalue_equal(result, json_obj));
	j_release(&json_obj);

	// Recursive number
	result = getFirstQueryResult(".ch1", json, &err);
	ASSERT_TRUE(jis_number(result));

	jnumber_get_i32(result, &num);
	ASSERT_EQ(5, num);

	// Complex keys
	result = getFirstQueryResult(".\"complex key\"", json, &err);
	ASSERT_TRUE(jis_number(result));

	jnumber_get_i32(result, &num);
	ASSERT_EQ(42, num);

	result = getFirstQueryResult(".\"complex.!@#key_2__,&*&@124\"", json, &err);
	ASSERT_TRUE(jis_number(result));

	jnumber_get_i32(result, &num);
	ASSERT_EQ(422, num);
}

TEST(Selectors, TestKeyWithTypeSelector)
{
	jerror *err = NULL;

	// A number
	jvalue_ref result = getFirstQueryResult("number.enum", json, &err);
	ASSERT_TRUE(jis_number(result));
	int32_t num;
	jnumber_get_i32(result, &num);
	ASSERT_EQ(3, num);

	result = getFirstQueryResult("string.enum", json, &err);
	ASSERT_FALSE(jis_valid(result));

	// We have this keys of two types: string and bool
	result = getFirstQueryResult("string.twotype", json, &err);
	ASSERT_TRUE(jis_string(result));
	const char *resstr = jstring_get_fast(result).m_str;
	ASSERT_STREQ(resstr, "str");

	result = getFirstQueryResult("boolean.twotype", json, &err);
	ASSERT_TRUE(jis_boolean(result));
	bool boolean;
	jboolean_get(result, &boolean);
	ASSERT_TRUE(boolean);

	// An object
	jvalue_ref json_obj = jdom_create(j_cstr_to_buffer(R"({"ch1": 5, "twotype": true})"),
	                                 jschema_all(),
	                                 &err);
	ASSERT_TRUE(jis_valid(json_obj));

	result = getFirstQueryResult("object.eobj", json, &err);
	ASSERT_TRUE(jvalue_equal(result, json_obj));

	result = getFirstQueryResult("array.eobj", json, &err);
	ASSERT_FALSE(jis_valid(result));

	result = getFirstQueryResult("string.eobj", json, &err);
	ASSERT_FALSE(jis_valid(result));
	j_release(&json_obj);

	// Recursive number
	result = getFirstQueryResult("number.ch1", json, &err);
	ASSERT_TRUE(jis_number(result));

	jnumber_get_i32(result, &num);
	ASSERT_EQ(5, num);
}
