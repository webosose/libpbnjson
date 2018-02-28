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

#include "Utils.hpp"

namespace {

using namespace std;

static jvalue_ref json = []()
{
	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"({"ebool1": false, "ebool2": true, "estr": "str", "estr2": "str2",)"
	                                               R"("eobj": {"ch1": null, "ch2": false},)"
	                                               R"("earray": [6, "brdm"]})"),
	                             jschema_all(),
	                             &err);

	return json;
}();

TEST(Selectors, TestSimpleSibling)
{
	jerror *err = NULL;

	// Boolean and null
	jquery_ptr query = jquery_create("null ~ boolean", &err);
	ASSERT_TRUE(query);

	ASSERT_TRUE(jquery_init(query, json, &err));
	jvalue_ref result = jquery_next(query);

	ASSERT_TRUE(jis_valid(result) && jis_boolean(result));

	jquery_free(query);

	// String and number
	query = jquery_create("number ~ string", &err);
	ASSERT_TRUE(query);

	ASSERT_TRUE(jquery_init(query, json, &err));
	result = jquery_next(query);

	ASSERT_TRUE(jis_valid(result) && jis_string(result));
	const char *resstr = jstring_get_fast(result).m_str;
	ASSERT_STREQ(resstr, "brdm");

	jquery_free(query);

	// Number and string
	query = jquery_create("string ~ number", &err);
	ASSERT_TRUE(query);

	ASSERT_TRUE(jquery_init(query, json, &err));
	result = jquery_next(query);

	ASSERT_TRUE(jis_valid(result) && jis_number(result));
	int32_t resnum;
	jnumber_get_i32(result, &resnum);
	ASSERT_EQ(6, resnum);

	jquery_free(query);
}

TEST(Selectors, TestPseudoSiblings)
{
	jerror *err = NULL;

	std::unordered_set<std::string> strings {"str", "str2", "brdm"};
	// String with any sibling
	jquery_ptr query = jquery_create("* ~ string", &err);
	ASSERT_TRUE(query);

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

	// All with all
	query = jquery_create("* ~ *", &err);
	ASSERT_TRUE(query);

	ASSERT_TRUE(jquery_init(query, json, &err));
	result = jquery_next(query);
	int counter = 10;
	while(jis_valid(result))
	{
		--counter;
		result = jquery_next(query);
	};
	ASSERT_EQ(0, counter);

	jquery_free(query);
}

TEST(Selectors, TestKeySiblings)
{
	jerror *err = NULL;

	jquery_ptr query = jquery_create(".ch1 ~ .ch2", &err);
	ASSERT_TRUE(query);

	ASSERT_TRUE(jquery_init(query, json, &err));
	jvalue_ref result = jquery_next(query);
	ASSERT_TRUE(jis_valid(result));
	ASSERT_TRUE(jis_boolean(result));

	bool bres = true;
	jboolean_get(result, &bres);
	ASSERT_FALSE(bres);

	jquery_free(query);
}

} // namespace
