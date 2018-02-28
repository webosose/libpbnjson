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

#include <algorithm>

#include <gtest/gtest.h>

#include <pbnjson.hpp>

TEST(TestIterator, ObjectIterator)
{
	using namespace pbnjson;

	std::string object = "{ \"0\": 0, \"1\": 1, \"2\": 2, \"3\": 3, \"4\": 4}";

	JDomParser parser;
	EXPECT_TRUE(parser.parse(object, JSchema::AllSchema(), nullptr));

	JValue root = parser.getDom();
	EXPECT_TRUE(root.isValid() && root.isObject());

	for (auto it = root.children().begin(); it != root.children().end(); ++it)
	{
		EXPECT_EQ(std::stof((*it).first.asString()), (*it).second.asNumber<int>());
	}

	for (auto it : root.children())
	{
		EXPECT_EQ(std::stof(it.first.asString()), it.second.asNumber<int>());
	}
}

TEST(TestIterator, ArrayIterator)
{
	using namespace pbnjson;

	std::string object = "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]";

	JDomParser parser;
	EXPECT_TRUE(parser.parse(object, JSchema::AllSchema(), nullptr));

	JValue root = parser.getDom();
	EXPECT_TRUE(root.isValid() && root.isArray());

	int index = 0;
	for (auto it = root.items().begin(); it != root.items().end(); ++it, ++index)
	{
		EXPECT_EQ(index, (*it).asNumber<int>());
	}

	{
		auto it1 = root.items().begin();
		auto it2 = it1 + 5;

		EXPECT_EQ(it2 - 5, it1);
		EXPECT_EQ(5, (*it2).asNumber<int>());

		++it1;
		--it2;

		EXPECT_EQ(1, (*it1).asNumber<int>());
		EXPECT_EQ(4, (*it2).asNumber<int>());

		auto it3 = it1--;
		auto it4 = it2++;

		EXPECT_EQ(0, (*it1).asNumber<int>());
		EXPECT_EQ(5, (*it2).asNumber<int>());
		EXPECT_EQ(1, (*it3).asNumber<int>());
		EXPECT_EQ(4, (*it4).asNumber<int>());
	}

	index = 0;
	for (auto it : root.items())
	{
		EXPECT_EQ(index, it.asNumber<int>());
		++index;
	}
}

TEST(TestIterator, TempObjIterator)
{
	using namespace pbnjson;

	JValue source_obj = JObject {{"arr", JArray { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }}};
	JValue source_arr = JArray {JObject {{"val1", 1}, {"val2", 2}, {"val3", 3}}};

	auto sum = 0;
	for (auto i: source_obj["arr"].items())
	{
		sum += i.asNumber<int>();
	}
	EXPECT_EQ(45, sum);

	sum = 0;
	for (auto i: source_arr[0].children())
	{
		sum += i.second.asNumber<int>();
	}
	EXPECT_EQ(6, sum);
}
