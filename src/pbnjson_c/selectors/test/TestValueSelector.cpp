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
using namespace pbnjson;

static jvalue_ref json = []()
{
	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(
	    "{"
	        R"("strings": [{"data": "yes"}, {"data": "no"}],)"
	        R"("numbers": [{"data": 24}, {"data": 42}],)"
	        R"("booleans": [{"data": true}, {"data": false}])"
	    "}"),
	    jschema_all(),
	    &err);

	return json;
}();

TEST(Selectors, TestValueString)
{
	jerror *err = NULL;
	auto results = getAllQueryResults(R"(.strings .data:val("yes"))", json, &err);

	ASSERT_TRUE(err == NULL);
	ASSERT_EQ(1u, results.size());

	for (const auto& value : results)
	{
		ASSERT_EQ(value, JValue("yes"));
	};
}

TEST(Selectors, TestValueBool)
{
	jerror *err = NULL;
	auto results = getAllQueryResults(R"(.booleans .data:val(false))", json, &err);

	ASSERT_TRUE(err == NULL);
	ASSERT_EQ(1u, results.size());

	for (const auto& value : results)
	{
		ASSERT_EQ(value, JValue(false));
	};
}

TEST(Selectors, TestValueNumber)
{
	jerror *err = NULL;
	auto results = getAllQueryResults(R"(.numbers .data:val(42))", json, &err);

	ASSERT_TRUE(err == NULL);
	ASSERT_EQ(1u, results.size());

	for (const auto& value : results)
	{
		ASSERT_EQ(value, JValue(42));
	};
}

} // namespace
