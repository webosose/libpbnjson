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
#include <unordered_set>

#include "Utils.hpp"

namespace {

using namespace std;

static jvalue_ref json = []()
{
	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"({"string1": "prefix string", "ebool": true, "estr": "str", "enull": null,)"
	                                               R"("twotype": "str", "eobj": {"string2": "substring", "twotype": true},)"
	                                               R"("earray": [6, "string suffix"], "string3": "full match"})"),
	                             jschema_all(),
	                             &err);

	return json;
}();

TEST(Selectors, TestContainsSelector)
{
	jerror *err = NULL;

	ASSERT_TRUE(jis_valid(json));

	// String is a prefix of the source str
	jvalue_ref result = getFirstQueryResult(":contains(\"prefix s\")", json, &err);
	ASSERT_TRUE(jis_valid(result) && jis_string(result));

	const char *resstr = jstring_get_fast(result).m_str;
	ASSERT_STREQ(resstr, "prefix string");

	// String is a substring in the middle of the source str
	result = getFirstQueryResult(":contains(\"ubstr\")", json, &err);
	ASSERT_TRUE(jis_valid(result) && jis_string(result));

	resstr = jstring_get_fast(result).m_str;
	ASSERT_STREQ(resstr, "substring");

	// String is a suffix of the source str
	result = getFirstQueryResult(":contains(\"g suffix\")", json, &err);
	ASSERT_TRUE(jis_valid(result) && jis_string(result));

	resstr = jstring_get_fast(result).m_str;
	ASSERT_STREQ(resstr, "string suffix");

	// String matches all the source str
	result = getFirstQueryResult(":contains(\"full match\")", json, &err);
	ASSERT_TRUE(jis_valid(result) && jis_string(result));

	resstr = jstring_get_fast(result).m_str;
	ASSERT_STREQ(resstr, "full match");
}

} // namespace
