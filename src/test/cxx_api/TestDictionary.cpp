// Copyright (c) 2016-2018 LG Electronics, Inc.
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
#include <random>
#include <thread>

using namespace pbnjson;
using namespace std;

void jsonParserThread(void) {
    static const char *const inputs[] =
    {
        R"({"subscribe": true})",
        R"({"returnValue": true, "subscribe": false})",
        R"({"subscribe": false})"
    };
    uniform_int_distribution<> uid {0, sizeof(inputs) / sizeof(inputs[0]) - 1};
    default_random_engine e;

    for (size_t i{0}, n{1<<16}; i != n; ++i)
    {
        auto result = pbnjson::JDomParser::fromString(inputs[uid(e)]);
        ASSERT_FALSE(result.isError());
    }
}

TEST(TestDictionary, ContentionWithKeyDestructor)
{
    thread t1(jsonParserThread);
    thread t2(jsonParserThread);

    t1.join();
    t2.join();
}
