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
#include "pbnjson.h"

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_set>

#include "../jquery_generators.h"

#include "Utils.hpp"

namespace {

using namespace std;

TEST(Generators, RecursiveGenerator)
{
	std::unordered_set<jvalue_ref> set;

	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"({"test1": 3, "test2": true, "test3": "str"})"),
	                              jschema_all(),
	                              &err);
	ASSERT_TRUE(jis_valid(json));

	jquery_generator_ptr generator = jq_generator_new((jvalue_search_result){ json, NULL },
	                                                  JQG_TYPE_RECURSIVE | JQG_TYPE_SELF);
	ASSERT_TRUE(generator);

	jvalue_ref current_result = jq_generator_next(generator).value;
	while (jis_valid(current_result))
	{
		set.insert(current_result);
		current_result = jq_generator_next(generator).value;
	}

	ASSERT_EQ(4u, set.size());
	ASSERT_EQ(1u, set.count(json));
	ASSERT_EQ(1u, set.count(jobject_get(json, J_CSTR_TO_BUF("test1"))));
	ASSERT_EQ(1u, set.count(jobject_get(json, J_CSTR_TO_BUF("test2"))));
	ASSERT_EQ(1u, set.count(jobject_get(json, J_CSTR_TO_BUF("test3"))));

	jq_generator_free(generator);
	j_release(&json);
}

TEST(Generators, RecursiveGeneratorTree)
{
	std::unordered_multiset<jvalue_ref> set;
	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"({"test1": [3, 6, 7, 12], "test2": { "test4": true, "test5": true }, "test3": "str"})"),
	                              jschema_all(),
	                              &err);
	ASSERT_TRUE(jis_valid(json));

	jquery_generator_ptr generator = jq_generator_new((jvalue_search_result){ json, NULL }, JQG_TYPE_RECURSIVE);
	ASSERT_TRUE(generator);

	jvalue_ref current_result = jq_generator_next(generator).value;
	while (jis_valid(current_result))
	{
		set.insert(current_result);
		current_result = jq_generator_next(generator).value;
	}

	ASSERT_EQ(10u, set.size());
	ASSERT_EQ(1u, set.count(json));
	ASSERT_EQ(1u, set.count(jobject_get(json, J_CSTR_TO_BUF("test3"))));
	auto arr = jobject_get(json, J_CSTR_TO_BUF("test1"));
	ASSERT_EQ(1u, set.count(arr));
	ASSERT_EQ(1u, set.count(jarray_get(arr, 0)));
	ASSERT_EQ(1u, set.count(jarray_get(arr, 1)));
	ASSERT_EQ(1u, set.count(jarray_get(arr, 2)));
	ASSERT_EQ(1u, set.count(jarray_get(arr, 3)));
	auto obj = jobject_get(json, J_CSTR_TO_BUF("test2"));
	ASSERT_EQ(1u, set.count(obj));
	EXPECT_NE(0u, set.count(jobject_get(obj, J_CSTR_TO_BUF("test4"))));
	EXPECT_NE(0u, set.count(jobject_get(obj, J_CSTR_TO_BUF("test5"))));

	jq_generator_free(generator);
	j_release(&json);
}

TEST(Generators, NonRecursiveGenerator)
{
	std::unordered_set<jvalue_ref> set;
	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"({"test1": 3, "test2": true, "test3": "str"})"),
	                              jschema_all(),
	                              &err);
	ASSERT_TRUE(jis_valid(json));

	jquery_generator_ptr generator = jq_generator_new((jvalue_search_result){ json, NULL }, JQG_TYPE_CHILDREN);
	ASSERT_TRUE(generator);

	jvalue_ref current_result = jq_generator_next(generator).value;
	while (jis_valid(current_result))
	{
		set.insert(current_result);
		current_result = jq_generator_next(generator).value;
	}

	ASSERT_EQ(3u, set.size());
	ASSERT_EQ(0u, set.count(json));
	ASSERT_EQ(1u, set.count(jobject_get(json, J_CSTR_TO_BUF("test1"))));
	ASSERT_EQ(1u, set.count(jobject_get(json, J_CSTR_TO_BUF("test2"))));
	ASSERT_EQ(1u, set.count(jobject_get(json, J_CSTR_TO_BUF("test3"))));

	jq_generator_free(generator);
	j_release(&json);
}

TEST(Generators, NonRecursiveGeneratorTree)
{
	std::unordered_set<jvalue_ref> set;
	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"({"test1": [3, 6, 7, 12], "test2": { "test4": true, "test5": true }, "test3": "str"})"),
	                              jschema_all(),
	                              &err);
	ASSERT_TRUE(jis_valid(json));

	jquery_generator_ptr generator = jq_generator_new((jvalue_search_result){ json, NULL }, JQG_TYPE_CHILDREN);
	ASSERT_TRUE(generator);

	jvalue_ref current_result = jq_generator_next(generator).value;
	while (jis_valid(current_result))
	{
		set.insert(current_result);
		current_result = jq_generator_next(generator).value;
	}

	ASSERT_EQ(3u, set.size());
	ASSERT_EQ(0u, set.count(json));
	ASSERT_EQ(1u, set.count(jobject_get(json, J_CSTR_TO_BUF("test1"))));
	ASSERT_EQ(1u, set.count(jobject_get(json, J_CSTR_TO_BUF("test2"))));
	ASSERT_EQ(1u, set.count(jobject_get(json, J_CSTR_TO_BUF("test3"))));

	jq_generator_free(generator);
	j_release(&json);
}

} // namespace
