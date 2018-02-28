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
#include <string>
#include <algorithm>
#include <iostream>

#include "Utils.hpp"

namespace {

using namespace std;
using namespace pbnjson;

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

TEST(Selectors, TestSimpleOrSelector)
{
	jerror *err = NULL;

	auto result = getAllQueryResults("boolean, null", json, &err);
	ASSERT_EQ(3u, result.size());

	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(true)));
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(false)));
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue::Null()));

	result = getAllQueryResults("number, string", json, &err);
	ASSERT_EQ(5u, result.size());

	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(3)));
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(5)));
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(6)));
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue("str")));
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue("brdm")));
}

TEST(Selectors, TestComplexOrSelector)
{
	jerror *err = NULL;

	auto result = getAllQueryResults(".eobj > number, .earray > number", json, &err);
	ASSERT_EQ(2u, result.size());

	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(5)));
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(6)));

	result = getAllQueryResults(":contains(\"t\"), :contains(\"r\")", json, &err);

	ASSERT_EQ(2u, result.size());

	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue("str")));
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue("brdm")));
}

TEST(Selectors, TestAdditionalOrSelector)
{
	jerror *err = NULL;

	auto result = getAllQueryResults(":root, array", json, &err);
	ASSERT_EQ(2u, result.size());

	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(jvalue_copy(json))));
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JArray({6, "brdm"})));

	result = getAllQueryResults(".ch1 ~ *, :root array", json, &err);
	ASSERT_EQ(2u, result.size());

	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JValue(false)));
	ASSERT_EQ(1u, std::count(result.begin(), result.end(), JArray({6, "brdm"})));
}

} // namespace
