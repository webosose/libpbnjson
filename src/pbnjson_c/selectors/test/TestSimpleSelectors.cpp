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
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"({"enum": 3, "ebool": true, "estr": "str", "enull": null,)"
	                                               R"("eobj": {"ch1": 5, "ch2": false},)"
	                                               R"("earray": [6, "brdm"]})"),
	                             jschema_all(),
	                             &err);

	return json;
}();

TEST(Selectors, TestAsteriskSelector)
{
	jerror *err = NULL;
	ASSERT_TRUE(jis_valid(json));

	jquery_ptr query = jquery_create("*", &err);
	ASSERT_TRUE(query);

	ASSERT_TRUE(jquery_init(query, json, &err));

	size_t json_size = 10;
	jvalue_ref root_jval = jquery_next(query);
	ASSERT_TRUE(jvalue_equal(json, root_jval));
	while (jis_valid(jquery_next(query)))
	{
		--json_size;
	}
	ASSERT_EQ(0u, json_size);

	jquery_free(query);
}

TEST(Selectors, TestRootSelector)
{
	jerror *err = NULL;
	ASSERT_TRUE(jis_valid(json));

	jquery_ptr query = jquery_create(":root", &err);
	ASSERT_TRUE(query);

	ASSERT_TRUE(jquery_init(query, json, &err));

	jvalue_ref root_jval = jquery_next(query);
	ASSERT_EQ(json, root_jval);
	ASSERT_FALSE(jis_valid(jquery_next(query)));

	jquery_free(query);
}

} //namespace
