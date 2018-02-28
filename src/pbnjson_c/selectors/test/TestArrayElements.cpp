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
#include <algorithm>

#include "Utils.hpp"

namespace {

using namespace std;
using namespace pbnjson;

static jvalue_ref json = []()
{
	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"({"enum": 4, "ebool": true, "estr": "str", "enull": null,)"
	                                               R"("earray2": [{"ch1": null, "ch2": [7]}, 7, 8],)"
	                                               R"("earray": []})"),
	                             jschema_all(),
	                             &err);

	return json;
}();

TEST(Selectors, TestEmptySelector)
{
	jerror *err = NULL;

	auto result = getAllQueryResults(":empty", json, &err);
	ASSERT_EQ(1u, result.size());

	jvalue_ref val = jobject_get(json, J_CSTR_TO_BUF("earray"));
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(jvalue_copy(val))));
}

TEST(Selectors, TestOnlyChildSelector)
{
	jerror *err = NULL;
	auto result = getAllQueryResults(":only-child", json, &err);

	ASSERT_EQ(1u, result.size());

	const JValue &val = *result.begin();
	ASSERT_EQ(JValue{7}, val);
}

TEST(Selectors, TestFirstChild)
{
	jerror *err = NULL;

	auto result = getAllQueryResults(":first-child", json, &err);

	ASSERT_EQ(2u, result.size());

	auto v1 = jarray_get(jobject_get(json, J_CSTR_TO_BUF("earray2")), 0);
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(jvalue_copy(v1))));
	auto v2 = jarray_get(jobject_get(v1, J_CSTR_TO_BUF("ch2")), 0);
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(jvalue_copy(v2))));
}


TEST(Selectors, TestLastChild)
{
	jerror *err = NULL;

	auto result = getAllQueryResults(":last-child", json, &err);

	ASSERT_EQ(2u, result.size());

	auto v1 = jarray_get(jobject_get(json, J_CSTR_TO_BUF("earray2")), 2);
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(jvalue_copy(v1))));
	auto v15 = jarray_get(jobject_get(json, J_CSTR_TO_BUF("earray2")), 0);
	auto v2 = jarray_get(jobject_get(v15, J_CSTR_TO_BUF("ch2")), 0);
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(jvalue_copy(v2))));
}

TEST(Selectors, TestNthChild)
{
	jerror *err = NULL;

	auto result_vector1 = getAllQueryResults(":nth-child(2)", json, &err);
	ASSERT_EQ(1u, result_vector1.size());

	auto result_vector2 = getAllQueryResults(":nth-child(-2)", json, &err);
	ASSERT_EQ(1u, result_vector2.size());

	auto v = jarray_get(jobject_get(json, J_CSTR_TO_BUF("earray2")), 1);
	ASSERT_EQ(1u, std::count(result_vector1.begin(), result_vector1.end(), JValue(jvalue_copy(v))));
	ASSERT_EQ(1u, std::count(result_vector2.begin(), result_vector2.end(), JValue(jvalue_copy(v))));
}

TEST(Selectors, TestArrayIndexError)
{
	jerror *err = NULL;
	jquery_ptr q  = jquery_create(":nth-child(0)", &err);
	ASSERT_TRUE(q == NULL);

	auto serr = getErrorString(err);
	ASSERT_STREQ("Syntax error. Invalid array index in array children selector: 0. "
	             "Must be a nonzero int32 value",
	             serr.c_str());

	jerror_free(err);
	err = NULL;
	q  = jquery_create(":nth-child(93672101203851021389651234)", &err);
	ASSERT_TRUE(q == NULL);

	serr = getErrorString(err);
	ASSERT_STREQ("Syntax error. Invalid array index in array children selector: "
	             "93672101203851021389651234. Must be a nonzero int32 value",
	             serr.c_str());
	jerror_free(err);
}

} // namespace
