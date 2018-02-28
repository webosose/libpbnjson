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

static const vector<string> queries =
{
	".languagesSpoken .lang",
	".drinkPreference :first-child",
	".seatingPreference :nth-child(15)",
	".\"weight\"",
	".lang",
	".favoriteColor",
	"string.favoriteColor",
	"string:last-child",
	"string:nth-child(2)",
	"string:nth-last-child(1)",
	":root",
	"number",
	":has(:root > .preferred)",
	".preferred ~ .lang",
	":has(.lang:val(\"Spanish\")) > .level",
	".lang:val(\"Bulgarian\") ~ .level",
	".weight:expr(x<160) ~ .name .first"
};

TEST(Selectors, SelectorExamples)
{
	for (const string &query_str: queries)
	{
		jerror *err = NULL;
		jquery_ptr query = jquery_create(query_str.c_str(), &err);
		if (!query)
		{
			fprintf(stderr, "Parser error. Query: '%s'. Error: '%s'\n",
			        query_str.c_str(),
			        getErrorString(err).c_str());
		}
		ASSERT_TRUE(query);

		jquery_free(query);
	}
}

TEST(Selectors, TestBaseSelection)
{
	jerror *err = NULL;
	jvalue_ref json = jdom_create(j_cstr_to_buffer(R"({"test": 3, "test2": true, "test3": "str"})"),
	                              jschema_all(),
	                              &err);
	ASSERT_TRUE(jis_valid(json));

	jquery_ptr query = jquery_create("*", &err);
	ASSERT_TRUE(query);

	ASSERT_TRUE(jquery_init(query, json, &err));
	ASSERT_TRUE(jvalue_equal(json, jquery_next(query)));

	j_release(&json);

	jquery_free(query);
}

TEST(Selectors, TestInvalidSymbols)
{
	jerror *err = NULL;

	jquery_ptr query = jquery_create("#", &err);
	ASSERT_FALSE(query);

	ASSERT_STREQ("Syntax error. Unexpected symbol '#' in the query string", getErrorString(err).c_str());

	jerror_free(err);
	err = NULL;
	jquery_free(query);

	query = jquery_create("	", &err);
	ASSERT_FALSE(query);

	ASSERT_STREQ("Syntax error. Unexpected symbol '\\t' in the query string", getErrorString(err).c_str());

	jerror_free(err);
	jquery_free(query);
}

TEST(Selectors, TestInvalidTokens)
{
	jerror *err = NULL;

	jquery_ptr query = jquery_create("fuzz.bazz", &err);
	ASSERT_FALSE(query);

	ASSERT_STREQ("Syntax error. Unexpected token 'fuzz' in the query string", getErrorString(err).c_str());

	jerror_free(err);
	err = NULL;
	jquery_free(query);

	query = jquery_create(".key ", &err);
	ASSERT_FALSE(query);

	ASSERT_STREQ("Syntax error. Unexpected end of the query string", getErrorString(err).c_str());

	jerror_free(err);
	jquery_free(query);
}

TEST(Selectors, TestSelectorNullErr)
{
	jquery_ptr query = jquery_create("fuzz.bazz", NULL);
	ASSERT_FALSE(query);

	query = jquery_create("*", NULL);
	ASSERT_TRUE(query);

	ASSERT_FALSE(jquery_init(query, NULL, NULL));
	ASSERT_TRUE(jquery_init(query, jnull(), NULL));

	jquery_free(query);
}

} // namespace
