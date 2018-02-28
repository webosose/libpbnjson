// Copyright (c) 2009-2018 LG Electronics, Inc.
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
#include <string>

using namespace std;
using namespace pbnjson;

namespace {

static const string resolution_dir = string{SCHEMA_DIR} + "contact/";

class TestResolver : public JResolver
{
	virtual JSchema resolve(const ResolutionRequest &request,
	                        JSchemaResolutionResult &result)
	{
		string lookup_path = resolution_dir + "/" + request.resource() + ".schema";
		if (-1 == ::access(lookup_path.c_str(), F_OK))
		{
			result = SCHEMA_NOT_FOUND;
			return JSchema::NullSchema();
		}

		JSchemaFile schema{lookup_path.c_str()};
		if (!schema.isInitialized())
		{
			result = SCHEMA_INVALID;
			return JSchema::NullSchema();
		}

		result = SCHEMA_RESOLVED;
		return schema;
	}
};

class TestResolver3 : public JResolver
{
	virtual JSchema resolve(const ResolutionRequest &request,
	                        JSchemaResolutionResult &result)
	{
		string lookup_path = resolution_dir + "/" + request.resource() + ".schema";

		JSchema schema = JSchema::fromFile(lookup_path.c_str());
		if (schema.isError())
		{
			result = SCHEMA_INVALID;
			return JSchema::NullSchema();
		}

		result = SCHEMA_RESOLVED;
		return schema;
	}
};

class TestSchemaContact
	: public ::testing::Test
{
protected:
	static unique_ptr<JSchema> schema;
	static JSchema schema3;
	static JDomParser dom_parser;
	static JSchemaInfo schema_info;
	jvalue_ref parsed;

	static void SetUpTestCase()
	{
		resolver = new TestResolver;
		schema.reset(new JSchemaFile(resolution_dir + "Contact.schema",
		                             resolution_dir + "Contact.schema", NULL, resolver));
		ASSERT_TRUE(schema->isInitialized());

		schema3.resolve(resolver3);
		dom_parser.reset(schema3);
		ASSERT_TRUE(bool(schema3));
	}

	static void TearDownTestCase()
	{
		delete resolver;
		resolver = NULL;
	}

	static TestResolver *resolver;
	static TestResolver3 resolver3;
};

TestResolver* TestSchemaContact::resolver = NULL;
TestResolver3 TestSchemaContact::resolver3;

unique_ptr<JSchema> TestSchemaContact::schema;
JSchema TestSchemaContact::schema3 = JSchema::fromFile((resolution_dir + "Contact.schema").c_str());
JDomParser TestSchemaContact::dom_parser;

} // namespace

TEST_F(TestSchemaContact, Invalid1)
{
	JDomParser parser;
	EXPECT_FALSE(parser.parse("", *schema.get()));
}

TEST_F(TestSchemaContact, Valid1)
{
	JDomParser parser;
	ASSERT_TRUE(parser.parse(
		"{"
			"\"contactIds\": [ \"1\" ],"
			"\"displayIndex\": \"first name\""
		"}",
		*schema.get()));
	auto parsed = parser.getDom();
	EXPECT_TRUE(parsed.isObject());
	EXPECT_TRUE(JValidator::isValid(parsed, *schema.get()));
}

TEST_F(TestSchemaContact, Valid2)
{
	JDomParser parser;
	ASSERT_TRUE(parser.parse("{}", *schema.get()));
	auto parsed = parser.getDom();
	EXPECT_TRUE(parsed.isObject());
	EXPECT_TRUE(JValidator::isValid(parsed, *schema.get()));
}

TEST_F(TestSchemaContact, Valid3)
{
	JDomParser parser;
	ASSERT_TRUE(parser.parse(
		"{"
			"\"displayName\": \"\","
			"\"name\": {},"
			"\"birthday\": \"\","
			"\"anniversary\": \"\","
			"\"gender\": \"undisclosed\""
		"}",
		*schema.get())
		);
	auto parsed = parser.getDom();
	EXPECT_TRUE(parsed.isObject());
	EXPECT_TRUE(JValidator::isValid(parsed, *schema.get()));
}


TEST_F(TestSchemaContact, Invalid3_1)
{
	EXPECT_FALSE(dom_parser.parse(""));
}

TEST_F(TestSchemaContact, Valid3_1)
{
	ASSERT_TRUE(dom_parser.parse(
		"{"
			"\"contactIds\": [ \"1\" ],"
			"\"displayIndex\": \"first name\""
		"}"));
	auto parsed = dom_parser.getDom();
	EXPECT_TRUE(parsed.isObject());
	EXPECT_TRUE(bool(schema3.validate(parsed)));
}

TEST_F(TestSchemaContact, Valid3_2)
{
	ASSERT_TRUE(dom_parser.parse("{}"));
	auto parsed = dom_parser.getDom();
	EXPECT_TRUE(parsed.isObject());
	EXPECT_TRUE(bool(schema3.validate(parsed)));
}

TEST_F(TestSchemaContact, Valid3_3)
{
	ASSERT_TRUE(dom_parser.parse(
		"{"
			"\"displayName\": \"\","
			"\"name\": {},"
			"\"birthday\": \"\","
			"\"anniversary\": \"\","
			"\"gender\": \"undisclosed\""
		"}")
		);
	auto parsed = dom_parser.getDom();
	EXPECT_TRUE(parsed.isObject());
	EXPECT_TRUE(bool(schema3.validate(parsed)));
}

// vim: set noet ts=4 sw=4 tw=80:
