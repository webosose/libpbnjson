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
#include <vector>
#include <algorithm>

#include <boost/scope_exit.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

const size_t SIZE = 128 * 1024;

TEST(JobjPerformanceAtoms, CreateBools)
{
	jvalue_ref vals[SIZE];
	for (size_t i = 0; i < SIZE; ++i)
	{
		vals[i] = jboolean_create(false);
	}
	for (size_t i = 0; i < SIZE; ++i)
	{
		j_release(&vals[i]);
	}
}

TEST(JobjPerformanceAtoms, CreateNums)
{
	jvalue_ref vals[SIZE];
	for (size_t i = 0; i < SIZE; ++i)
	{
		vals[i] = jnumber_create_i32(0);
	}
	for (size_t i = 0; i < SIZE; ++i)
	{
		j_release(&vals[i]);
	}
}

TEST(JobjPerformanceAtoms, CreateStrings)
{
	jvalue_ref vals[SIZE];
	for (size_t i = 0; i < SIZE; ++i)
	{
		vals[i] = jstring_create("performance test string");
	}
	for (size_t i = 0; i < SIZE; ++i)
	{
		j_release(&vals[i]);
	}
}

TEST(JobjPerformanceAtoms, CreateArrays)
{
	jvalue_ref vals[SIZE];
	for (size_t i = 0; i < SIZE; ++i)
	{
		vals[i] = jarray_create(NULL);
	}
	for (size_t i = 0; i < SIZE; ++i)
	{
		j_release(&vals[i]);
	}
}

TEST(JobjPerformanceAtoms, CreateObjects)
{
	jvalue_ref vals[SIZE];
	for (size_t i = 0; i < SIZE; ++i)
	{
		vals[i] = jobject_create();
	}
	for (size_t i = 0; i < SIZE; ++i)
	{
		j_release(&vals[i]);
	}
}

class JobjPerformanceObject
	: public testing::Test
{
public:
	JobjPerformanceObject()
	{
		keys.reserve(SIZE);
		for (size_t i = 0; i < SIZE; ++i)
		{
			keys.emplace_back(boost::lexical_cast<string>(i));
		}
	}

protected:
	vector<string> keys;
};

TEST_F(JobjPerformanceObject, CreateObjectOfBools)
{
	jvalue_ref obj = jobject_create();
	for (auto const &key : keys)
	{
		jvalue_ref jkey = jstring_create_nocopy(j_str_to_buffer(key.c_str(), key.size()));
		jobject_put(obj, jkey, jboolean_create(false));
	}
	j_release(&obj);
}

TEST_F(JobjPerformanceObject, CreateObjectOfNums)
{
	jvalue_ref obj = jobject_create();
	for (auto const &key : keys)
	{
		jvalue_ref jkey = jstring_create_nocopy(j_str_to_buffer(key.c_str(), key.size()));
		jobject_put(obj, jkey, jnumber_create_i32(1));
	}
	j_release(&obj);
}

TEST_F(JobjPerformanceObject, CreateObjectOfStrings)
{
	jvalue_ref obj = jobject_create();
	for (auto const &key : keys)
	{
		jvalue_ref jkey = jstring_create_nocopy(j_str_to_buffer(key.c_str(), key.size()));
		jobject_put(obj, jkey, jstring_create("test string"));
	}
	j_release(&obj);
}

TEST_F(JobjPerformanceObject, CreateObjectOfArrays)
{
	jvalue_ref obj = jobject_create();
	for (auto const &key : keys)
	{
		jvalue_ref jkey = jstring_create_nocopy(j_str_to_buffer(key.c_str(), key.size()));
		jobject_put(obj, jkey, jarray_create(NULL));
	}
	j_release(&obj);
}

TEST_F(JobjPerformanceObject, CreateObjectOfObjects)
{
	jvalue_ref obj = jobject_create();
	for (auto const &key : keys)
	{
		jvalue_ref jkey = jstring_create_nocopy(j_str_to_buffer(key.c_str(), key.size()));
		jobject_put(obj, jkey, jobject_create());
	}
	j_release(&obj);
}

class JobjPerformanceRemove
	: public JobjPerformanceObject
{
protected:
	jvalue_ref obj;

	virtual void SetUp()
	{
		JobjPerformanceObject::SetUp();

		obj = jobject_create();
		for (auto const &key : keys)
		{
			jvalue_ref jkey = jstring_create_nocopy(j_str_to_buffer(key.c_str(), key.size()));
			jobject_put(obj, jkey, jstring_create("test string"));
		}
	}

	virtual void TearDown()
	{
		j_release(&obj);
	}
};

TEST_F(JobjPerformanceRemove, RemoveStringsFromObject)
{
	for (auto const &key : keys)
		jobject_remove(obj, j_cstr_to_buffer(key.c_str()));
}
