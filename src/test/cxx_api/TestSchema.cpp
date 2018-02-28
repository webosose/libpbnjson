// Copyright (c) 2014-2018 LG Electronics, Inc.
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

TEST(TestSchema, OneOf)
{
	auto schema = JSchemaFragment("{\"oneOf\":["
		"{\"additionalProperties\":{\"type\":\"string\"}},"
		"{\"additionalProperties\":{\"type\":\"integer\"}}"
	"]}");
	ASSERT_TRUE(schema.isInitialized());

	JDomParser parser;
	EXPECT_FALSE(parser.parse("{}", schema));
	EXPECT_FALSE(parser.parse("{\"foo\":null}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":42}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":\"bar\"}", schema));
}

TEST(TestSchema, LocalRef)
{
	auto schema = JSchemaFragment("{"
		"\"definitions\":{"
			"\"foo\":{\"additionalProperties\":{\"type\":\"string\"}},"
			"\"bar\":{\"additionalProperties\":{\"type\":\"integer\"}}"
		"},"
		"\"oneOf\":["
			"{\"$ref\":\"#/definitions/foo\"},"
			"{\"$ref\":\"#/definitions/bar\"}"
		"]"
	"}");
	ASSERT_TRUE(schema.isInitialized());

	JDomParser parser;
	EXPECT_FALSE(parser.parse("{}", schema));
	EXPECT_FALSE(parser.parse("{\"foo\":null}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":42}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":\"bar\"}", schema));
}

TEST(TestSchema, LocalRefEscaped)
{
	auto schema = JSchemaFragment("{"
		"\"definitions\":{"
			"\"foo~1\":{\"additionalProperties\":{\"type\":\"string\"}},"
			"\"bar/schema\":{\"additionalProperties\":{\"type\":\"integer\"}}"
		"},"
		"\"oneOf\":["
			"{\"$ref\":\"#/definitions/foo~01\"},"
			"{\"$ref\":\"#/definitions/bar~1schema\"}"
		"]"
	"}");
	ASSERT_TRUE(schema.isInitialized());

	JDomParser parser;
	EXPECT_FALSE(parser.parse("{}", schema));
	EXPECT_FALSE(parser.parse("{\"foo\":null}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":42}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":\"bar\"}", schema));
}

TEST(TestSchema, LocalRefRef)
{
	auto schema = JSchemaFragment("{"
		"\"definitions\":{"
			"\"foo\":{\"additionalProperties\":{\"type\":\"string\"}},"
			"\"bar\":{\"additionalProperties\":{\"type\":\"integer\"}},"
			"\"foofoo\":{\"$ref\":\"#/definitions/foo\"}"
		"},"
		"\"oneOf\":["
			"{\"$ref\":\"#/definitions/foofoo\"},"
			"{\"$ref\":\"#/definitions/bar\"}"
		"]"
	"}");
	ASSERT_TRUE(schema.isInitialized());

	JDomParser parser;
	EXPECT_FALSE(parser.parse("{}", schema));
	EXPECT_FALSE(parser.parse("{\"foo\":null}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":42}", schema));
	EXPECT_TRUE(parser.parse("{\"foo\":\"bar\"}", schema));
}

////////////////////////////////////////////////////////////////////// NEW API

TEST(TestSchema, OneOf3)
{
	auto schema = JSchema::fromString("{\"oneOf\":["
		"{\"additionalProperties\":{\"type\":\"string\"}},"
		"{\"additionalProperties\":{\"type\":\"integer\"}}"
	"]}");
	ASSERT_TRUE(schema.isInitialized());

	JDomParser parser(schema);
	EXPECT_FALSE(parser.parse("{}"));
	EXPECT_FALSE(parser.parse("{\"foo\":null}"));
	EXPECT_TRUE(parser.parse("{\"foo\":42}"));
	EXPECT_TRUE(parser.parse("{\"foo\":\"bar\"}"));
}

TEST(TestSchema, LocalRef3)
{
	auto schema = JSchema::fromString(R"(
	{
		"definitions":{
			"foo":{"additionalProperties":{"type":"string"}},
			"bar":{"additionalProperties":{"type":"integer"}}
		},
		"oneOf":[
			{"$ref":"#/definitions/foo"},
			{"$ref":"#/definitions/bar"}
		]
	})");
	ASSERT_TRUE(schema.isInitialized());

	JDomParser parser(schema);
	EXPECT_FALSE(parser.parse("{}"));
	EXPECT_FALSE(parser.parse("{\"foo\":null}"));
	EXPECT_TRUE(parser.parse("{\"foo\":42}"));
	EXPECT_TRUE(parser.parse("{\"foo\":\"bar\"}"));
}

TEST(TestSchema, LocalRefEscaped3)
{
	auto schema = JSchema::fromString(R"({
		"definitions":{
			"foo~1":{"additionalProperties":{"type":"string"}},
			"bar/schema":{"additionalProperties":{"type":"integer"}}
		},
		"oneOf":[
			{"$ref":"#/definitions/foo~01"},
			{"$ref":"#/definitions/bar~1schema"}
		]
	})");
	ASSERT_TRUE(schema.isInitialized());

	JDomParser parser(schema);
	EXPECT_FALSE(parser.parse("{}"));
	EXPECT_FALSE(parser.parse("{\"foo\":null}"));
	EXPECT_TRUE(parser.parse("{\"foo\":42}"));
	EXPECT_TRUE(parser.parse("{\"foo\":\"bar\"}"));
}

TEST(TestSchema, LocalRefRef3)
{
	auto schema = JSchema::fromString(R"({
		"definitions":{
			"foo":{"additionalProperties":{"type":"string"}},
			"bar":{"additionalProperties":{"type":"integer"}},
			"foofoo":{"$ref":"#/definitions/foo"}
		},
		"oneOf":[
			{"$ref":"#/definitions/foofoo"},
			{"$ref":"#/definitions/bar"}
		]
	})");
	ASSERT_TRUE(schema.isInitialized());

	JDomParser parser(schema);
	EXPECT_FALSE(parser.parse("{}"));
	EXPECT_FALSE(parser.parse("{\"foo\":null}"));
	EXPECT_TRUE(parser.parse("{\"foo\":42}"));
	EXPECT_TRUE(parser.parse("{\"foo\":\"bar\"}"));
}

TEST(TestSchema, NonContainerValidation)
{
	auto invalid = JArray()[0]; // out-of-bounds results in an invalid value

	auto EXPECT_PASS = [](const JValue &value, const JSchema &schema)
	{
		JResult result;
		EXPECT_FALSE((result = schema.validate(value)).isError()) << result.errorString();
	};

	auto EXPECT_FAIL = [](const JValue &value, const JSchema &schema)
	{
		JResult result;
		EXPECT_TRUE((result = schema.validate(value)).isError()) << result.errorString();
	};

	// Object
	auto schema = JSchema::fromString(R"({"type":"object"})");
	EXPECT_PASS(JObject(), schema);
	EXPECT_FAIL(JArray(), schema);
	EXPECT_FAIL(JValue(42), schema);
	EXPECT_FAIL(JValue(true), schema);
	EXPECT_FAIL(JValue("Hello world"), schema);
	EXPECT_FAIL(JValue(), schema);
	EXPECT_FAIL(invalid, schema);

	// Array
	schema = JSchema::fromString(R"({"type":"array"})");
	EXPECT_PASS(JArray(), schema);
	EXPECT_FAIL(JObject(), schema);
	EXPECT_FAIL(JValue(42), schema);
	EXPECT_FAIL(JValue(true), schema);
	EXPECT_FAIL(JValue("Hello world"), schema);
	EXPECT_FAIL(JValue(), schema);
	EXPECT_FAIL(invalid, schema);
	EXPECT_FAIL(JArray { invalid }, schema);

	// Number
	schema = JSchema::fromString(R"({"type":"number"})");
	EXPECT_PASS(JValue(42), schema);
	EXPECT_FAIL(JArray(), schema);
	EXPECT_FAIL(JObject(), schema);
	EXPECT_FAIL(JValue(true), schema);
	EXPECT_FAIL(JValue("Hello world"), schema);
	EXPECT_FAIL(JValue(), schema);
	EXPECT_FAIL(invalid, schema);

	// Boolean
	schema = JSchema::fromString(R"({"type":"boolean"})");
	EXPECT_PASS(JValue(true), schema);
	EXPECT_FAIL(JArray(), schema);
	EXPECT_FAIL(JObject(), schema);
	EXPECT_FAIL(JValue(42), schema);
	EXPECT_FAIL(JValue("Hello world"), schema);
	EXPECT_FAIL(JValue(), schema);
	EXPECT_FAIL(invalid, schema);

	// String
	schema = JSchema::fromString(R"({"type":"string"})");
	EXPECT_PASS(JValue("Hello world"), schema);
	EXPECT_FAIL(JArray(), schema);
	EXPECT_FAIL(JObject(), schema);
	EXPECT_FAIL(JValue(42), schema);
	EXPECT_FAIL(JValue(true), schema);
	EXPECT_FAIL(JValue(), schema);
	EXPECT_FAIL(invalid, schema);

	// String
	schema = JSchema::fromString(R"({"type":"null"})");
	EXPECT_PASS(JValue(), schema);
	EXPECT_FAIL(JArray(), schema);
	EXPECT_FAIL(JObject(), schema);
	EXPECT_FAIL(JValue(42), schema);
	EXPECT_FAIL(JValue(true), schema);
	EXPECT_FAIL(JValue("Hello world"), schema);
	EXPECT_FAIL(invalid, schema);
}

TEST(TestSchema, CastToBool)
{
	auto schema = JSchema::fromString(R"({"type":"object"})");
	EXPECT_TRUE((bool)schema);

	schema = JSchema::fromFile("../schemas/parse/not_existed.schema");
	EXPECT_FALSE((bool)schema);
}

TEST(TestSchema, FromJValue)
{
	static auto schema = pbnjson::JSchema::fromJValue({
		{"type", "object"},
		{"properties", {
			{"shift", {{"type", "integer"}, {"default", 0}}},
			{"mask", {{"type", "integer"}, {"default", 1}}},
			{"toggleBits", {{"type", "number"}, {"default", 2.718}}},
			{"mapping", {{"type", "array"}}},
		}},
	});

	pbnjson::JValue value {
		{"shift", 2},
	};

	ASSERT_FALSE(schema.isError())
		<< "Schema error: " << schema.errorString();

	auto result = schema.apply(value);
	EXPECT_FALSE(result.isError()) << "Error: " << result.errorString();

	EXPECT_EQ(2, value["shift"].asNumber<int32_t>());
	EXPECT_EQ(1, value["mask"].asNumber<int32_t>());
	EXPECT_EQ(2.718, value["toggleBits"].asNumber<double>());
	EXPECT_EQ("2", value["shift"].stringify());
	EXPECT_EQ("1", value["mask"].stringify());
	EXPECT_EQ("2.718", value["toggleBits"].stringify());
}

// ex: set noet ts=4 sw=4 tw=80:
