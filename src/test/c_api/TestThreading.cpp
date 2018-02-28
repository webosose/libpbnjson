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

#include "TestUtils.hpp"

#include <pbnjson.h>

#include <gtest/gtest.h>

#include <memory>
#include <array>
#include <thread>

namespace {
	std::string toText(jerror *error)
	{
		if (!error) return "ok";
		char buf[512];
		int length = jerror_to_string(error, buf, sizeof(buf));
		return std::string(buf, std::min(size_t(length), sizeof(buf)));
	}
} // anonymous namespace

TEST(Threading, jvalue)
{
	const size_t nthreads = 8, nsteps = 1000;

	auto str = mk_ptr(jstring_create("hello"));
	ASSERT_STREQ("hello", jstring_get_fast(str.get()).m_str);

	const auto f = [&]() {
		for (size_t step = 0; step < nsteps; ++step)
		{
			auto copy = mk_ptr(jvalue_copy(str.get()));
			ASSERT_STREQ("hello", jstring_get_fast(copy.get()).m_str);
		}
	};
	f();
	EXPECT_STREQ("hello", jstring_get_fast(str.get()).m_str);

	// now lets check that rule in threads
	std::array<std::thread, nthreads> threads;
	for (auto &thread : threads) thread = std::thread(f);
	for (auto &thread : threads) thread.join();

	EXPECT_STREQ("hello", jstring_get_fast(str.get()).m_str);
}

TEST(Threading, schema)
{
	const size_t nthreads = 8, nsteps = 1000;
	auto schema_buf = J_CSTR_TO_BUF(R"({"type": "object", "properties": {"x": {"type": "integer"}}})");
	auto resolve_func = [](JSchemaResolverRef, jschema_ref *) -> JSchemaResolutionResult
	{
		return SCHEMA_NOT_FOUND;
	};

	auto value = mk_ptr(jobject_create_var(
		jkeyval(J_CSTR_TO_JVAL("x"), jnumber_create_i32(42)),
		J_END_OBJ_DECL
		));
	ASSERT_TRUE(jis_valid(value.get()));

	jerror *error = nullptr;

	auto schema = mk_ptr(jschema_create(schema_buf, &error));
	ASSERT_FALSE(error) << toText(error);

	JSchemaResolver resolver = {};
	resolver.m_resolve = resolve_func;
	ASSERT_TRUE(jschema_resolve(schema.get(), &resolver));

	ASSERT_TRUE(jvalue_validate(value.get(), schema.get(), &error)) << toText(error);

	const auto f = [&]() {
		for (size_t step = 0; step < nsteps; ++step)
		{
			auto copy = mk_ptr(jschema_copy(schema.get()));
			ASSERT_TRUE(jvalue_validate(value.get(), schema.get(), &error)) << toText(error);
		}
	};
	f();
	EXPECT_TRUE(jvalue_validate(value.get(), schema.get(), &error)) << toText(error);

	// now lets check that rule in threads
	std::array<std::thread, nthreads> threads;
	for (auto &thread : threads) thread = std::thread(f);
	for (auto &thread : threads) thread.join();

	EXPECT_TRUE(jvalue_validate(value.get(), schema.get(), &error)) << toText(error);
}
