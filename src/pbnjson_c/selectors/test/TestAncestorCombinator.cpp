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

#define PBNJSON_USE_DEPRECATED_API
#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <iostream>

#include "Utils.hpp"

namespace {

using namespace std;

static jvalue_ref json = []()
{
	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"({"root_object":)"
	                                               R"({"enum": 4, "ebool": true, "estr": "str", "enull": null,)"
	                                               R"("eobj": [{"ch1": null, "ch2": [42, 7]}, 7, 8],)"
	                                               R"("earray": [6, "brdm"]}})"),
	                             jschema_all(),
	                             &err);

	return json;
}();

TEST(Selectors, TestArrayAncestor)
{
	jerror *err = NULL;

	jvalue_ref json_arr = jdom_create(j_cstr_to_buffer(R"([42, 7])"),
	                                  jschema_all(),
	                                  NULL);
	ASSERT_TRUE(jis_valid(json_arr));

	// Array in an array
	jquery_ptr query = jquery_create("array array", &err);
	ASSERT_TRUE(query);

	ASSERT_TRUE(jquery_init(query, json, &err));
	jvalue_ref result = jquery_next(query);

	ASSERT_TRUE(jis_valid(result) && jis_array(result));
	ASSERT_TRUE(jvalue_equal(json_arr, result));

	ASSERT_FALSE(jis_valid(jquery_next(query)));
	j_release(&json_arr);
	jquery_free(query);

	// String in element with key
	query = jquery_create(".earray string", &err);
	ASSERT_TRUE(query);

	ASSERT_TRUE(jquery_init(query, json, &err));
	result = jquery_next(query);

	ASSERT_TRUE(jis_valid(result) && jis_string(result));
	const char *resstr = jstring_get_fast(result).m_str;
	ASSERT_STREQ(resstr, "brdm");

	ASSERT_FALSE(jis_valid(jquery_next(query)));
	jquery_free(query);
}

TEST(Selectors, TestNullAncestor)
{
	jerror *err = NULL;

	// Null in an array
	jquery_ptr query = jquery_create("array null", &err);
	ASSERT_TRUE(query);
	jquery_init(query, json, &err);

	jvalue_ref result = jquery_next(query);
	ASSERT_TRUE(jis_valid(result) && jis_null(result));

	ASSERT_FALSE(jis_valid(jquery_next(query)));
	jquery_free(query);

	// Null in element with key
	query = jquery_create(".eobj null", &err);
	ASSERT_TRUE(query);
	jquery_init(query, json, &err);

	result = jquery_next(query);
	ASSERT_TRUE(jis_valid(result) && jis_null(result));

	ASSERT_FALSE(jis_valid(jquery_next(query)));
	jquery_free(query);
}

TEST(Selectors, TestNoAncestor)
{
	jerror *err = NULL;

	// Boolean in an array
	jquery_ptr query = jquery_create("array boolean", &err);
	ASSERT_TRUE(query);
	jquery_init(query, json, &err);

	ASSERT_FALSE(jis_valid(jquery_next(query)));
	jquery_free(query);

	// String in an object with key
	query = jquery_create(".eobj string", &err);
	ASSERT_TRUE(query);
	jquery_init(query, json, &err);

	ASSERT_FALSE(jis_valid(jquery_next(query)));
	jquery_free(query);
}

TEST(Selectors, TestDeepAncestor)
{
	jerror *err = NULL;

	// Boolean in an array
	jquery_ptr query = jquery_create(".root_object array string", &err);
	ASSERT_TRUE(query);

	ASSERT_TRUE(jquery_init(query, json, &err));
	jvalue_ref result = jquery_next(query);

	ASSERT_TRUE(jis_valid(result) && jis_string(result));
	const char *resstr = jstring_get_fast(result).m_str;
	ASSERT_STREQ(resstr, "brdm");

	ASSERT_FALSE(jis_valid(jquery_next(query)));
	jquery_free(query);
}

} //namespace
