// Copyright (c) 2013-2018 LG Electronics, Inc.
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
#include <pbnjson.h>
#include <string>
#include <algorithm>

#include <boost/scope_exit.hpp>

using namespace std;

class JobjRemove
	: public testing::Test
{
protected:
	virtual void SetUp()
	{
		obj = jobject_create();
		jobject_put(obj, J_CSTR_TO_JVAL("a"), jnumber_create_i32(5));
		jobject_put(obj, J_CSTR_TO_JVAL("b"), jstring_create("Hello, world"));
		jobject_put(obj, J_CSTR_TO_JVAL("c"), jnumber_create_i32(13));
		jobject_put(obj, J_CSTR_TO_JVAL("d"), jboolean_create(true));
		jobject_put(obj, J_CSTR_TO_JVAL("e"), jboolean_create(false));
	}

	virtual void TearDown()
	{
		j_release(&obj);
	}

	jvalue_ref obj;

	string GetIteration() const
	{
		string res;
		jobject_iter it;
		jobject_key_value keyval;

		jobject_iter_init(&it, obj);
		while (jobject_iter_next(&it, &keyval))
		{
			res.push_back(jstring_get_fast(keyval.key).m_str[0]);
		}
		std::sort(res.begin(), res.end());
		return res;
	}
};

TEST_F(JobjRemove, ObjectRemoveAndIterate)
{
	jobject_remove(obj, j_cstr_to_buffer("b"));
	ASSERT_EQ(GetIteration(), "acde");
	jobject_remove(obj, j_cstr_to_buffer("a"));
	ASSERT_EQ(GetIteration(), "cde");
	jobject_remove(obj, j_cstr_to_buffer("e"));
	ASSERT_EQ(GetIteration(), "cd");
}

TEST(JobjRemove2, ObjectRemoveHashCollision)
{
	// [GF-6968] libpbnjson: jobject_find() won't find a key after jobject_remove
	jvalue_ref obj = jobject_create();
	BOOST_SCOPE_EXIT((&obj)) {
		j_release(&obj);
	} BOOST_SCOPE_EXIT_END

	// The hashes of these keys must collide
	jobject_put(obj, J_CSTR_TO_JVAL("ab"), jnumber_create_i32(5));
	jobject_put(obj, J_CSTR_TO_JVAL("b"), jstring_create("Hello, world"));
	ASSERT_TRUE(jobject_containskey(obj, j_cstr_to_buffer("ab")));
	ASSERT_TRUE(jobject_containskey(obj, j_cstr_to_buffer("b")));
	jobject_remove(obj, j_cstr_to_buffer("ab"));
	ASSERT_FALSE(jobject_containskey(obj, j_cstr_to_buffer("ab")));
	ASSERT_TRUE(jobject_containskey(obj, j_cstr_to_buffer("b")));
}

TEST(JobjRemove2, GetNested)
{
	jvalue_ref root = jobject_create();
	jvalue_ref a = jobject_create();
	ASSERT_TRUE(jobject_put(root, J_CSTR_TO_JVAL("a"), a));
	jvalue_ref b = jobject_create();
	ASSERT_TRUE(jobject_put(a, J_CSTR_TO_JVAL("b"), b));
	jvalue_ref c = jarray_create(NULL);
	ASSERT_TRUE(jobject_put(b, J_CSTR_TO_JVAL("c"), c));
	jvalue_ref d = jboolean_create(true);
	ASSERT_TRUE(jobject_put(a, J_CSTR_TO_JVAL("d"), d));

	EXPECT_EQ(a, jobject_get_nested(root, "a", NULL));
	EXPECT_EQ(b, jobject_get_nested(root, "a", "b", NULL));
	EXPECT_EQ(c, jobject_get_nested(root, "a", "b", "c", NULL));
	EXPECT_EQ(d, jobject_get_nested(root, "a", "d", NULL));

	EXPECT_FALSE(jis_valid(jobject_get_nested(root, "x", NULL)));
	EXPECT_FALSE(jis_valid(jobject_get_nested(root, "a", "x", NULL)));
	EXPECT_FALSE(jis_valid(jobject_get_nested(root, "a", "d", "x", NULL)));

	j_release(&root);
}
