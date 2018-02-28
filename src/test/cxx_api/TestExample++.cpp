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
#include <iostream>

TEST(TestExample, Generate)
{
//! [generate]
	pbnjson::JValue myresponse = pbnjson::JObject{
		{"errorCode", 5},
		{"errorText", "This is an example of a pbnjson object"}
	};

	std::cout << myresponse.stringify() << std::endl;  // write out {"errorCode":5,"errorText"...} to stdout
//! [generate]
}

TEST(TestExample, ParseSchema)
//! [parse schema]
{
	// since the schema doesn't specify additionalProperties, all other
	// properties are accepted by default
	const char *input = "{\"guess\" : 5.3, \"cheat\" : true}";

	pbnjson::JSchema inputSchema = pbnjson::JSchema::fromFile(SCHEMA_DIR "input.schema");

	pbnjson::JValue parsed = pbnjson::JDomParser::fromString(input, inputSchema);
	if (parsed.isError()) {
		std::cerr << parsed.errorString() << std::endl;
		return;
	}

	std::cout << parsed["guess"].asNumber<double>() << std::endl; // this is always guaranteed to print a number between 1 & 10.  no additional validation necessary within the code.
	std::cout << parsed["cheat"].asBool() << std::endl; // this will print the value of cheat (if cheat isn't present or isn't a boolean, this will print false)
}
//! [parse schema]

TEST(TestExample, ParseAll)
//! [parse all]
{
	const char *input = "{\"guess\" : 5.3, \"cheat\" : true}";

	// in this example, I don't care about validation against JSON schema.
	pbnjson::JValue parsed = pbnjson::JDomParser::fromString(input);
	if (parsed.isError()) {
		std::cerr << parsed.errorString() << std::endl;
	}

	std::cout << parsed["guess"].asNumber<double>() << std::endl; // this is always guaranteed to print a number between 1 & 10.  no additional validation necessary within the code.
	std::cout << parsed["cheat"].asBool() << std::endl; // this will print the value of cheat (if cheat isn't present or isn't a boolean, this will print false)
}
//! [parse all]

TEST(TestExample, ParseStreamDom)
//! [parse stream dom]
{
	std::string input = R"({"number": 1, "str": "asd"})";

	// Create a new parser, use default schema
	pbnjson::JDomParser parser;

	// Start stream parsing
	parser.reset();

	// parse input data part by part. Parts can be of any size, in this example it will be one byte.
	// Actually all data, that is available for the moment of call Parse, should be passed. It
	// will increase performance.
	for (auto ch : input) {
		if (!parser.feed(pbnjson::JInput{&ch, 1})) {
			std::cerr << "Parse error: " << parser.getError() << std::endl;
			return;
		}
	}

	if (!parser.end()) {
		std::cerr << "Parse error: " << parser.getError() << std::endl;
		return;
	}

	// Get root JValue
	pbnjson::JValue json = parser.getDom();
}
//! [parse stream dom]
