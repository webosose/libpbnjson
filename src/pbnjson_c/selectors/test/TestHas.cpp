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

#include "Utils.hpp"

using namespace std;

namespace {

class MemManager
{
public:
	~MemManager()
	{
		for (auto &v : _jvalue_refs)
			j_release(&v);
	}

	jvalue_ref operator()(jvalue_ref v)
	{
		return _jvalue_refs.push_back(v), v;
	}

private:
	vector<jvalue_ref> _jvalue_refs;
} mem_manager;

jvalue_ref GetJson()
{
	static const char *const json =
		R"(
{
	"num": 13,
	"array": [17, {"id1": 45, "value1": "asdf"}],
	"object": {
		"string2": "qwer",
		"array2": [101],
		"object2": {"id2": 47}
	}
}
		)";
	static auto v = mem_manager(
		jdom_create(j_cstr_to_buffer(json),
		            jschema_all(),
		            nullptr)
		);
	return v;
}

} //namespace;

TEST(Has, Everything)
{
	jerror *err = nullptr;

	jquery_ptr query = jquery_create(":has(number.id1)", &err);
	ASSERT_TRUE(query);
	EXPECT_EQ(err, nullptr);

	ASSERT_TRUE(jquery_init(query, GetJson(), nullptr));

	set<jvalue_ref> match;
	for (jvalue_ref v = jquery_next(query);
	     jis_valid(v);
	     v = jquery_next(query))
	{
		match.insert(v);
	}

	EXPECT_EQ(3u, match.size());

	auto v1 = GetJson();
	EXPECT_TRUE(match.count(v1));
	auto v2 = jobject_get(v1, J_CSTR_TO_BUF("array"));
	EXPECT_TRUE(match.count(v2));
	auto v3 = jarray_get(v2, 1);
	EXPECT_TRUE(match.count(v3));

	jquery_free(query);
}

TEST(Has, HasRoot)
{
	jerror *err = nullptr;

	jquery_ptr query = jquery_create(":has(:root)", &err);
	ASSERT_TRUE(query);
	EXPECT_EQ(nullptr, err);

	ASSERT_TRUE(jquery_init(query, GetJson(), nullptr));

	int count = 0;
	for (jvalue_ref v = jquery_next(query);
	     jis_valid(v);
	     v = jquery_next(query))
	{
		++count;
	}
	ASSERT_EQ(13, count);

	jquery_free(query);
}

TEST(Has, HasTypeMismatch)
{
	jerror *err = nullptr;

	jquery_ptr query = jquery_create(":has(string.id1)", &err);
	ASSERT_TRUE(query);
	EXPECT_EQ(err, nullptr);

	ASSERT_TRUE(jquery_init(query, GetJson(), nullptr));

	ASSERT_FALSE(jis_valid(jquery_next(query)));

	jquery_free(query);
}

TEST(Has, HasKeyMismatch)
{
	jerror *err = nullptr;

	jquery_ptr query = jquery_create(":has(number.id3)", &err);
	ASSERT_TRUE(query);
	EXPECT_EQ(err, nullptr);

	ASSERT_TRUE(jquery_init(query, GetJson(), nullptr));

	ASSERT_FALSE(jis_valid(jquery_next(query)));

	jquery_free(query);
}
