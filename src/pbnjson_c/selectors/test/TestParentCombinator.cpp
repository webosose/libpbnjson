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

namespace {

using namespace std;

static jvalue_ref json = []()
{
	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"({"enum": 4, "ebool": true, "estr": "str", "enull": null,)"
	                                               R"("eobj": {"ch1": null, "ch2": false},)"
	                                               R"("earray": [6, "brdm"]})"),
	                             jschema_all(),
	                             &err);

	return json;
}();

TEST(Selectors, TestSimpleParent)
{
	jerror *err = NULL;

	// String in an array
	jquery_ptr query = jquery_create("array > string", &err);
	ASSERT_TRUE(query);

	ASSERT_TRUE(jquery_init(query, json, &err));
	jvalue_ref result = jquery_next(query);

	ASSERT_TRUE(jis_valid(result) && jis_string(result));
	const char *resstr = jstring_get_fast(result).m_str;
	ASSERT_STREQ(resstr, "brdm");

	ASSERT_FALSE(jis_valid(jquery_next(query)));
	jquery_free(query);

	// Number in an array
	query = jquery_create("array > number", &err);
	ASSERT_TRUE(query);

	jquery_init(query, json, &err);

	result = jquery_next(query);
	ASSERT_TRUE(jis_valid(result) && jis_number(result));
	int32_t resnum;
	jnumber_get_i32(result, &resnum);
	ASSERT_EQ(6, resnum);

	ASSERT_FALSE(jis_valid(jquery_next(query)));
	jquery_free(query);
}

TEST(Selectors, TestComplexParent)
{
	jerror *err = NULL;

	// Boolean in an object
	jquery_ptr query = jquery_create("object > boolean.ch2", &err);
	ASSERT_TRUE(query);
	jquery_init(query, json, &err);

	jvalue_ref result = jquery_next(query);
	ASSERT_TRUE(jis_valid(result) && jis_boolean(result));

	ASSERT_FALSE(jis_valid(jquery_next(query)));
	jquery_free(query);

	// Boolean within parent with key
	query = jquery_create(".eobj > boolean.ch2", &err);
	ASSERT_TRUE(query);
	jquery_init(query, json, &err);

	result = jquery_next(query);
	ASSERT_TRUE(jis_valid(result) && jis_boolean(result));

	ASSERT_FALSE(jis_valid(jquery_next(query)));
	jquery_free(query);

	// Boolean within parent with key
	query = jquery_create("array.earray > string", &err);
	ASSERT_TRUE(query);
	jquery_init(query, json, &err);

	result = jquery_next(query);
	ASSERT_TRUE(jis_valid(result) && jis_string(result));
	const char *resstr = jstring_get_fast(result).m_str;
	ASSERT_STREQ(resstr, "brdm");

	ASSERT_FALSE(jis_valid(jquery_next(query)));
	jquery_free(query);
}

TEST(Selectors, TestPseudoParent)
{
	jerror *err = NULL;

	// All selector test
	jquery_ptr query = jquery_create("* > .estr", &err);
	ASSERT_TRUE(query);
	jquery_init(query, json, &err);

	jvalue_ref result = jquery_next(query);
	ASSERT_TRUE(jis_valid(result) && jis_string(result));
	const char *resstr = jstring_get_fast(result).m_str;
	ASSERT_STREQ(resstr, "str");

	ASSERT_FALSE(jis_valid(jquery_next(query)));
	jquery_free(query);

	// Root selector test
	query = jquery_create(":has(:root > .ch1)", &err);
	ASSERT_TRUE(query);
	jquery_init(query, json, &err);

	// .eobj Object
	result = jquery_next(query);
	ASSERT_TRUE(jis_valid(result) && jis_object(result));
	ASSERT_FALSE(jis_valid(jquery_next(query)));

	jquery_free(query);
}

TEST(Selectors, TestDescentParent)
{
	jerror *err = NULL;

	// All selector test
	jquery_ptr query = jquery_create("object > .eobj > .ch2", &err);
	ASSERT_TRUE(query);
	jquery_init(query, json, &err);

	jvalue_ref result = jquery_next(query);
	ASSERT_TRUE(jis_valid(result) && jis_boolean(result));
	bool bres = true;
	jboolean_get(result, &bres);
	ASSERT_FALSE(bres);

	jquery_free(query);
}

} //namespace
