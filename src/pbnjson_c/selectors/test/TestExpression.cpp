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
#include "pbnjson.h"
#include "../jquery_internal.h"
#include "../expression.h"

#define private public
#define protected public
#include "pbnjson.hpp"
#undef private
#undef protected

using namespace std;
using namespace pbnjson;


TEST(Expression, X)
{
	auto query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x)", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	SelEx *ex = (SelEx *) query->sel_ctxt;


	EXPECT_TRUE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{0}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Array().m_jval));
}

TEST(Expression, Equal)
{
	auto query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x = x)", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	SelEx *ex = (SelEx *) query->sel_ctxt;

	EXPECT_TRUE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{0}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Array().m_jval));
}

TEST(Expression, Null)
{
	auto query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x = null)", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	SelEx *ex = (SelEx *) query->sel_ctxt;

	EXPECT_TRUE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{0}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));
}

TEST(Expression, Number)
{
	auto query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x = 42)", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	SelEx *ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{42}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{42.0}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{42.42}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{-42}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{0}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));

	query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x = 42.42)", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{42.42}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{42}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{-42.42}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{0}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));
}

TEST(Expression, String)
{
	auto query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x = \"Hello world\")", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	SelEx *ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{42}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{42.0}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{42.42}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{-42}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{0}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"Hello"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"world"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"HELLO WORLD"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"Hello world"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));
}

TEST(Expression, NotEqual)
{
	auto query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x != 1)", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	SelEx *ex = (SelEx *) query->sel_ctxt;

	EXPECT_TRUE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(":expr(x != true)", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_TRUE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(R"(:expr(x != "true"))", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_TRUE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Array().m_jval));
}

TEST(Expression, Less)
{
	auto query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x < 13)", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	SelEx *ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(":expr(x < true)", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(R"(:expr(x < "2"))", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"100"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"2"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"3"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));
}

TEST(Expression, Greater)
{
	auto query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x > 1)", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	SelEx *ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(":expr(x > false)", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(R"(:expr(x > "2"))", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"100"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"2"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"20"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"3"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));
}

TEST(Expression, Lequal)
{
	auto query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x <= 13)", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	SelEx *ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{17}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(":expr(x <= true)", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(R"(:expr(x <= "2"))", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"100"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"2"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"3"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));
}

TEST(Expression, Gequal)
{
	auto query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x >= 1)", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	SelEx *ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{0}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(":expr(x >= false)", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(R"(:expr(x >= "2"))", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"100"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"2"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"20"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"3"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));
}

TEST(Expression, And)
{
	auto query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x && true)", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	SelEx *ex = (SelEx *) query->sel_ctxt;

	EXPECT_TRUE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{0}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(":expr(x && false)", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{0}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(R"(:expr(x < 10 && x >= 5))", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"13"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"1"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{100}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{5}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{7}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{10}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));
}

TEST(Expression, Or)
{
	auto query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x || true)", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	SelEx *ex = (SelEx *) query->sel_ctxt;

	EXPECT_TRUE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{0}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(":expr(x || false)", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_TRUE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{0}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{""}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{"true"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, Array().m_jval));

	query.reset(jquery_create(R"(:expr(x >= 10 || x < 5))", nullptr));
	ASSERT_TRUE(query.get());
	ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"13"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"1"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{1}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{5}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{7}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{10}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));
}

TEST(Expression, AndOr)
{
	auto query = unique_ptr<jquery, void(*)(jquery_ptr)>{
		jquery_create(":expr(x < 10 && x > 5 || x < 20 && x > 15)", nullptr),
		jquery_free
	};
	ASSERT_TRUE(query.get());
	SelEx *ex = (SelEx *) query->sel_ctxt;

	EXPECT_FALSE(sel_ex_eval(ex, JValue::JNULL.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{true}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{false}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"13"}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{"1"}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{6}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{5}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, JValue{13}.m_jval));
	EXPECT_TRUE(sel_ex_eval(ex, JValue{17}.m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Object().m_jval));
	EXPECT_FALSE(sel_ex_eval(ex, Array().m_jval));
}
