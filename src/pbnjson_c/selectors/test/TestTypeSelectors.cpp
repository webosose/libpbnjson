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
#include <unordered_set>
#include <vector>

#include "Utils.hpp"

namespace {

using namespace std;

static jvalue_ref json = []()
{
	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"({"enum": 3, "ebool": true, "estr": "str", "enull": null,)"
	                                               R"("eobj": {"ch1": 5, "ch2": false},)"
	                                               R"("earray": [6, "brdm"]})"),
	                             jschema_all(),
	                             &err);

	return json;
}();

TEST(Selectors, TestNumbSelector)
{
	jerror *err = NULL;
	std::unordered_set<int> numbers {3, 5, 6};
	jquery_ptr query = jquery_create("number", &err);

	ASSERT_NE(nullptr, query);
	ASSERT_TRUE(jquery_init(query, json, &err));

	jvalue_ref result = jquery_next(query);
	while(jis_valid(result))
	{
		ASSERT_TRUE(jis_valid(result) && jis_number(result));

		int32_t resnum;
		jnumber_get_i32(result, &resnum);

		ASSERT_TRUE(numbers.count(resnum) != 0);
		numbers.erase(resnum);
		result = jquery_next(query);
	};
	ASSERT_TRUE(numbers.empty());

	jquery_free(query);
}

TEST(Selectors, TestBoolSelector)
{
	jerror *err = NULL;
	std::unordered_set<bool> bools { true, false };
	jquery_ptr query = jquery_create("boolean", &err);

	ASSERT_TRUE(jquery_init(query, json, &err));

	jvalue_ref result = jquery_next(query);
	while(jis_valid(result))
	{
		ASSERT_TRUE(jis_valid(result) && jis_boolean(result));

		bool bval;
		jboolean_get(result, &bval);

		ASSERT_TRUE(bools.count(bval) != 0);
		bools.erase(bval);
		result = jquery_next(query);
	};
	ASSERT_TRUE(bools.empty());

	jquery_free(query);
}

TEST(Selectors, TestStringSelector)
{
	jerror *err = NULL;
	std::unordered_set<std::string> strings {"str", "brdm"};
	jquery_ptr query = jquery_create("string", &err);

	ASSERT_TRUE(jquery_init(query, json, &err));

	jvalue_ref result = jquery_next(query);
	while(jis_valid(result))
	{
		ASSERT_TRUE(jis_valid(result) && jis_string(result));

		const char *resstr = jstring_get_fast(result).m_str;

		ASSERT_TRUE(strings.count(resstr) != 0);
		strings.erase(resstr);
		result = jquery_next(query);
	};
	ASSERT_TRUE(strings.empty());

	jquery_free(query);
}

TEST(Selectors, TestNullSelector)
{
	jerror *err = NULL;
	size_t null_cnt = 1;
	jquery_ptr query = jquery_create("null", &err);

	ASSERT_TRUE(jquery_init(query, json, &err));

	jvalue_ref result = jquery_next(query);
	while(jis_valid(result))
	{
		ASSERT_TRUE(jis_valid(result) && jis_null(result));
		--null_cnt;
		result = jquery_next(query);
	}
	ASSERT_EQ(0u, null_cnt);

	jquery_free(query);
}

TEST(Selectors, TestObjSelector)
{
	jerror *err = NULL;
	size_t obj_cnt = 2;
	jquery_ptr query = jquery_create("object", &err);

	ASSERT_TRUE(jquery_init(query, json, &err));

	jvalue_ref result = jquery_next(query);
	ASSERT_TRUE(jvalue_equal(result, json));
	while(jis_valid(result))
	{
		ASSERT_TRUE(jis_valid(result) && jis_object(result));
		--obj_cnt;
		result = jquery_next(query);
	}
	ASSERT_EQ(0u, obj_cnt);

	jquery_free(query);
}

TEST(Selectors, TestArraySelector)
{
	jerror *err = NULL;
	jvalue_ref json_arr = jdom_create(j_cstr_to_buffer(R"([6, "brdm"])"),
	                                  jschema_all(),
	                                  NULL);
	ASSERT_TRUE(jis_valid(json_arr));

	size_t arr_cnt = 1;
	jquery_ptr query = jquery_create("array", &err);

	ASSERT_TRUE(jquery_init(query, json, &err));

	jvalue_ref result = jquery_next(query);
	ASSERT_TRUE(jvalue_equal(result, json_arr));
	while(jis_valid(result))
	{
		ASSERT_TRUE(jis_valid(result) && jis_array(result));
		--arr_cnt;
		result = jquery_next(query);
	}
	ASSERT_EQ(0u, arr_cnt);

	j_release(&json_arr);
	jquery_free(query);
}

TEST(Selectors, ArrayQueryOrderTest)
{
	std::vector<pbnjson::JValue> num_list { 1, 2, 7, 12 };

	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"([[1, 2], [7, 12]])"),
	                              jschema_all(),
	                              &err);
	ASSERT_TRUE(jis_valid(json));

	jquery_ptr query = jquery_create("number", &err);
	ASSERT_TRUE(query);

	ASSERT_TRUE(jquery_init(query, json, &err));

	int32_t resnum;
	for (const auto &cjson: num_list)
	{
		jvalue_ref num = jquery_next(query);
		ASSERT_TRUE(jis_valid(num));

		jnumber_get_i32(num, &resnum);

		ASSERT_EQ(cjson, resnum);
	}

	jquery_free(query);
	j_release(&json);
}

} // namespace
