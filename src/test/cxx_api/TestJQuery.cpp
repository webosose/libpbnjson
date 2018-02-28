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
#include <pbnjson.hpp>

using namespace std;
using namespace pbnjson;

auto json = JDomParser::fromString(R"([
	{ "k1": "qwe", "k2": "asd" },
	{ "k1": "zxc", "k2": 12    },
	{ "k1": 42,    "k2": "qaz" }
])");

TEST(TestJQuery, TestValid)
{
	JQuery q1 { ":has(.field)" };
	ASSERT_TRUE((bool)q1);

	std::string q { "object" };
	JQuery q2 { q };
	ASSERT_TRUE((bool)q2);
}

TEST(TestJQuery, TestInvalid)
{
	JQuery q { ":invalid()" };
	ASSERT_FALSE((bool)q);
}

TEST(TestJQuery, TestClassic)
{
	JQuery query { "string.k1" };
	ASSERT_TRUE((bool)query);

	int cnt = 0;
	query.apply(json);
	for (JQuery::iterator it = query.begin(); it != query.end(); ++it) {
		JValue e = *it;
		ASSERT_TRUE(e.isString());
		cnt++;
	}
	ASSERT_EQ(2, cnt);

	for (JQuery::iterator it = query.begin(); it != query.end(); ++it) {
		JValue e = *it;
		ASSERT_TRUE(e.isString());
		cnt--;
	}
	ASSERT_EQ(0, cnt);
}

TEST(TestJQuery, TestCPP11)
{
	JQuery q { "number.k1" };
	ASSERT_TRUE((bool)q);

	int cnt = 0;
	for (auto e : q(json)) {
		ASSERT_TRUE(e.isNumber());
		cnt++;
	}
	ASSERT_EQ(1, cnt);

	for (auto e : q(json)) {
		ASSERT_TRUE(e.isNumber());
		cnt--;
	}
	ASSERT_EQ(0, cnt);
}
