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
#include <fstream>
#include <vector>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>

using namespace pbnjson;

void ReadFileToString(const std::string& fileName, std::string& dst)
{
	std::ifstream file(fileName);
	if (!file.is_open())
		throw std::runtime_error("Failed to open file: " + fileName);

	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	dst.resize(size);
	file.seekg(0);
	file.read(&dst[0], size);
}

struct SAXCallbacks : public JParser
{
	SAXCallbacks()
		: JParser(NULL)
		, jsonObjectOpenCount(0)
		, jsonObjectCloseCount(0)
		, jsonArrayOpenCount(0)
		, jsonArrayCloseCount(0)
		, jsonNullCount(0)
	{}

	int jsonObjectOpenCount;
	int jsonObjectCloseCount;
	int jsonArrayOpenCount;
	int jsonArrayCloseCount;
	int jsonNullCount;
	std::vector<std::string> jsonObjectKeyStorage;
	std::vector<std::string> jsonStringStorage;
	std::vector<std::string> jsonNumberStringStorage;
	std::vector<int64_t> jsonNumberInt64Storage;
	std::vector<double> jsonNumberDoubleStorage;
	std::vector<bool> jsonBooleanStorage;

	bool jsonObjectOpen() {
		jsonObjectOpenCount++;
		return true;
	}
	bool jsonObjectKey(const std::string& key) {
		jsonObjectKeyStorage.push_back(key);
		return true;
	}
	bool jsonObjectClose() {
		jsonObjectCloseCount++;
		return true;
	}
	bool jsonArrayOpen() {
		jsonArrayOpenCount++;
		return true;
	}
	bool jsonArrayClose() {
		jsonArrayCloseCount++;
		return true;
	}
	bool jsonString(const std::string& s) {
		jsonStringStorage.push_back(s);
		return true;
	}
	bool jsonNumber(const std::string& n) {
		jsonNumberStringStorage.push_back(n);
		return true;
	}
	bool jsonNumber(int64_t number) {
		jsonNumberInt64Storage.push_back(number);
		return true;
	}
	bool jsonNumber(double &number, ConversionResultFlags asFloat) {
		jsonNumberDoubleStorage.push_back(number);
		return true;
	}
	bool jsonBoolean(bool truth) {
		jsonBooleanStorage.push_back(truth);
		return true;
	}
	bool jsonNull() {
		jsonNullCount++;
		return true;
	}

	NumberType conversionToUse() const {return JNUM_CONV_NATIVE;}
};

template<class T>
std::vector<T> MultVector(const std::vector<T>& vec, int times) {
	std::vector<T> newVec;
	for(int i = 0 ; i < times ; i++) {
		std::copy(vec.begin(),vec.end(),back_inserter(newVec));
	}
	return newVec;
}

TEST(TestParse, saxparser_simple)
{
    struct DummyParser : JParser
    {
        NumberType conversionToUse() const override { return JNUM_CONV_RAW; }
    };
    // test all 3 overloads (including deprecated one)
    EXPECT_FALSE( DummyParser().parse("{\"sample\": \"hi\"}") );
    EXPECT_FALSE( DummyParser().parse(JInput{"{\"sample\": \"hi\"}"}, JSchema::AllSchema()) );
    EXPECT_FALSE( DummyParser().parse(std::string{"{\"sample\": \"hi\"}"}, JSchema::AllSchema()) );
}

TEST(TestParse, saxparser)
{
	std::string json_str;
	ReadFileToString("../schemas/parse/test_stream_parser.json", json_str);

	JSchemaFile schema("../schemas/parse/test_stream_parser.schema");

	for (int j = 0 ; j < 2 ; j++) {
		SAXCallbacks parser;
		for (int k = 1 ; k < 3 ; k++) {
			ASSERT_TRUE(parser.begin(schema));
			for (std::string::const_iterator i = json_str.begin() ; i != json_str.end() ; ++i) {
				ASSERT_TRUE(parser.feed(&(*i), 1));
			}
			ASSERT_TRUE(parser.end());

			EXPECT_EQ(k*1, parser.jsonObjectOpenCount);
			EXPECT_EQ(k*1, parser.jsonObjectCloseCount);
			EXPECT_EQ(k*1, parser.jsonArrayOpenCount);
			EXPECT_EQ(k*1, parser.jsonArrayCloseCount);
			EXPECT_EQ(k*1, parser.jsonNullCount);
			EXPECT_EQ(MultVector(std::vector<std::string>({"null", "bool", "number", "string", "array"}), k), parser.jsonObjectKeyStorage);
			EXPECT_EQ(MultVector(std::vector<std::string>({"asd", "qwerty"}), k), parser.jsonStringStorage);
			EXPECT_EQ(MultVector(std::vector<std::string>(), k), parser.jsonNumberStringStorage);
			EXPECT_EQ(MultVector(std::vector<int64_t>({2}), k), parser.jsonNumberInt64Storage);
			EXPECT_EQ(MultVector(std::vector<double>({1.1}), k), parser.jsonNumberDoubleStorage);
			EXPECT_EQ(MultVector(std::vector<bool>({true}), k), parser.jsonBooleanStorage);
		}
	}
}

TEST(TestParse, domparser)
{
	std::string json_str;
	ReadFileToString("../schemas/parse/test_stream_parser.json", json_str);

	JSchemaFile schema("../schemas/parse/test_stream_parser.schema");

	JValue jval;
	for (int  j = 0 ; j < 2 ; j++) {
		JDomParser parser;
		for (int k = 0 ; k < 2 ; k++) {
			ASSERT_TRUE(parser.begin(schema));
			for (std::string::const_iterator i = json_str.begin() ; i != json_str.end() ; ++i) {
				ASSERT_TRUE(parser.feed(&(*i), 1));
			}
			ASSERT_TRUE(parser.end());
		}
		jval = parser.getDom();
	}

	EXPECT_TRUE(jval["null"].isNull());
	EXPECT_EQ(true, jval["bool"].asBool());
	EXPECT_EQ(1.1, jval["number"].asNumber<double>());
	EXPECT_EQ("asd", jval["string"].asString());
	EXPECT_EQ(2, jval["array"][0].asNumber<int64_t>());
	EXPECT_EQ("qwerty", jval["array"][1].asString());

}

TEST(TestParse, invalid_json) {
	std::string input("{\"number\"\":1, \"str\":\"asd\"}");

	// Create a new parser, use default schema
	pbnjson::JDomParser parser;

	// Start stream parsing
	if (!parser.begin(JSchema::AllSchema())) {
		std::string error = parser.getError();
		return;
	}

	// parse input data part by part. Parts can be of any size, in this example it will be one byte.
	// Actually all data, that is available for the moment of call Parse, should be passed. It
	// will increase performance.
	for (std::string::const_iterator i = input.begin(); i != input.end() ; ++i) {
		if (!parser.feed(&(*i), 1)) {
			std::string error = parser.getError();
			return;
		}
	}

	if (!parser.end()) {
		std::string error = parser.getError();
		return;
	}

	// Get root JValue
	pbnjson::JValue json = parser.getDom();
}

TEST(TestParse, saxparser3)
{
	std::string json_str;
	ReadFileToString("../schemas/parse/test_stream_parser.json", json_str);

	JSchema schema = JSchema::fromFile("../schemas/parse/test_stream_parser.schema");

	for (int j = 0 ; j < 2 ; j++) {
		SAXCallbacks parser;
		for (int k = 1 ; k < 3 ; k++) {
			parser.reset(schema);
			for (std::string::const_iterator i = json_str.begin() ; i != json_str.end() ; ++i) {
				ASSERT_TRUE(parser.feed((const JInput&){&(*i), 1}));
			}
			ASSERT_TRUE(parser.end());

			EXPECT_EQ(k*1, parser.jsonObjectOpenCount);
			EXPECT_EQ(k*1, parser.jsonObjectCloseCount);
			EXPECT_EQ(k*1, parser.jsonArrayOpenCount);
			EXPECT_EQ(k*1, parser.jsonArrayCloseCount);
			EXPECT_EQ(k*1, parser.jsonNullCount);
			EXPECT_EQ(MultVector(std::vector<std::string>({"null", "bool", "number", "string", "array"}), k), parser.jsonObjectKeyStorage);
			EXPECT_EQ(MultVector(std::vector<std::string>({"asd", "qwerty"}), k), parser.jsonStringStorage);
			EXPECT_EQ(MultVector(std::vector<std::string>(), k), parser.jsonNumberStringStorage);
			EXPECT_EQ(MultVector(std::vector<int64_t>({2}), k), parser.jsonNumberInt64Storage);
			EXPECT_EQ(MultVector(std::vector<double>({1.1}), k), parser.jsonNumberDoubleStorage);
			EXPECT_EQ(MultVector(std::vector<bool>({true}), k), parser.jsonBooleanStorage);
		}
	}
}

TEST(TestParse, domparser3)
{
	std::string json_str;
	ReadFileToString("../schemas/parse/test_stream_parser.json", json_str);

	JSchema schema = JSchema::fromFile("../schemas/parse/test_stream_parser.schema");

	JValue jval;
	for (int  j = 0 ; j < 2 ; j++) {
		JDomParser parser;
		for (int k = 0 ; k < 2 ; k++) {
			parser.reset(schema);
			for (std::string::const_iterator i = json_str.begin() ; i != json_str.end() ; ++i) {
				ASSERT_TRUE(parser.feed((const JInput&){&(*i), 1}));
			}
			ASSERT_TRUE(parser.end());
		}
		jval = parser.getDom();
	}

	EXPECT_TRUE(jval["null"].isNull());
	EXPECT_EQ(true, jval["bool"].asBool());
	EXPECT_EQ(1.1, jval["number"].asNumber<double>());
	EXPECT_EQ("asd", jval["string"].asString());
	EXPECT_EQ(2, jval["array"][0].asNumber<int64_t>());
	EXPECT_EQ("qwerty", jval["array"][1].asString());
}

namespace {
	#define TYPEMAPITEM(TYPE) {std::type_index(typeid(TYPE)), (#TYPE)}
	template <typename T>
	std::string report(T expected, T result, const std::string & _string) {
		using std::string;
		using std::type_index;
		const static std::unordered_map<type_index, string> t_map = {
				TYPEMAPITEM(int32_t),
				TYPEMAPITEM(int64_t),
				TYPEMAPITEM(double)
		};
		std::stringstream ss;
		try {
			const std::string name_of_type = t_map.at(type_index(typeid(T)));
			ss << "The string value '"
			   << _string << "' was read as "
			   << name_of_type << "(" << result
			   << ") and it's not equal to expected "
			   << name_of_type << "(" << expected
			   << ") value.";
		} catch (const std::out_of_range & e) {
			ss << "Unexpected type '" << typeid(T).name() << "' in "
			   << e.what();
			ADD_FAILURE() << ss.str();
		}
		return ss.str();
	}

	template <typename T>
	inline void NumberCompare(T expected, const char * _string) {
		T result = JDomParser::fromString(_string).asNumber<T>();
		EXPECT_EQ(expected, JDomParser::fromString(_string).asNumber<T>())
			<< report(expected, result, _string);
	}

	template <>
	inline void NumberCompare<double>(double expected, const char * _string) {
		double result = JDomParser::fromString(_string).asNumber<double>();
		EXPECT_DOUBLE_EQ(expected, JDomParser::fromString(_string).asNumber<double>())
			<< report(expected, result, _string);
	}
}

#define CMP_F64(DNUMBER) NumberCompare(double(DNUMBER), #DNUMBER)
#define CMP_F64I64(DNUMBER) do {\
		NumberCompare(double(DNUMBER), #DNUMBER);\
		NumberCompare(int64_t(DNUMBER), #DNUMBER);\
	} while (false)
#define CMP_F64I64I32(DNUMBER) do {\
		NumberCompare(double(DNUMBER), #DNUMBER);\
		NumberCompare(int64_t(DNUMBER), #DNUMBER);\
		NumberCompare(int32_t(DNUMBER), #DNUMBER);\
	} while (false)

TEST(TestParse, number) {
	CMP_F64I64I32( 1.8e+6);
	CMP_F64I64I32(-1.8e+6);
	CMP_F64I64I32( 1.8e6);
	CMP_F64I64I32(-1.8e6);
	CMP_F64I64I32( 1.8e-6);
	CMP_F64I64I32(-1.8e-6);

	CMP_F64I64I32( 1.8E+6);
	CMP_F64I64I32(-1.8E+6);
	CMP_F64I64I32( 1.8E6);
	CMP_F64I64I32(-1.8E6);
	CMP_F64I64I32( 1.8E-6);
	CMP_F64I64I32(-1.8E-6);

	CMP_F64I64I32( 41e-3);
	CMP_F64I64I32(-41e-3);
	CMP_F64I64I32( 41e-3);
	CMP_F64I64I32(-41e-3);

	CMP_F64I64I32( 41E-3);
	CMP_F64I64I32(-41E-3);
	CMP_F64I64I32( 41E-3);
	CMP_F64I64I32(-41E-3);

	CMP_F64I64I32( 2.2250738585072014e-308);
	CMP_F64I64I32(-2.2250738585072014e-308);
	CMP_F64I64I32( 2.2250738585072014E-308);
	CMP_F64I64I32(-2.2250738585072014E-308);

	CMP_F64( 1.7976931348623157e+308);
	CMP_F64(-1.7976931348623157e+308);
	CMP_F64( 1.7976931348623157e308);
	CMP_F64(-1.7976931348623157e308);
	CMP_F64( 1.7976931348623157E+308);
	CMP_F64(-1.7976931348623157E+308);
	CMP_F64( 1.7976931348623157E308);
	CMP_F64(-1.7976931348623157E308);

	CMP_F64I64I32( 0e+308);
	CMP_F64I64I32(-0e+308);
	CMP_F64I64I32( 0e308);
	CMP_F64I64I32(-0e308);
	CMP_F64I64I32( 0e-308);
	CMP_F64I64I32(-0e-308);

	CMP_F64I64I32( 0E+308);
	CMP_F64I64I32(-0E+308);
	CMP_F64I64I32( 0E308);
	CMP_F64I64I32(-0E308);
	CMP_F64I64I32( 0E-308);
	CMP_F64I64I32(-0E-308);

	CMP_F64I64I32( 0.0e+308);
	CMP_F64I64I32(-0.0e+308);
	CMP_F64I64I32( 0.0e308);
	CMP_F64I64I32(-0.0e308);
	CMP_F64I64I32( 0.0e-308);
	CMP_F64I64I32(-0.0e-308);

	CMP_F64I64I32( 0.0E+308);
	CMP_F64I64I32(-0.0E+308);
	CMP_F64I64I32( 0.0E308);
	CMP_F64I64I32(-0.0E308);
	CMP_F64I64I32( 0.0E-308);
	CMP_F64I64I32(-0.0E-308);

	CMP_F64( 1e+308);
	CMP_F64(-1e+308);
	CMP_F64( 1e308);
	CMP_F64(-1e308);
	CMP_F64I64I32( 1e-308);
	CMP_F64I64I32(-1e-308);

	CMP_F64( 1E+308);
	CMP_F64(-1E+308);
	CMP_F64( 1E308);
	CMP_F64(-1E308);
	CMP_F64I64I32( 1E-308);
	CMP_F64I64I32(-1E-308);

	CMP_F64( 1.0e+308);
	CMP_F64(-1.0e+308);
	CMP_F64( 1.0e308);
	CMP_F64(-1.0e308);
	CMP_F64I64I32( 1.0e-308);
	CMP_F64I64I32(-1.0e-308);

	CMP_F64( 1.0E+308);
	CMP_F64(-1.0E+308);
	CMP_F64( 1.0E308);
	CMP_F64(-1.0E308);
	CMP_F64I64I32( 1.0E-308);
	CMP_F64I64I32(-1.0E-308);

	CMP_F64I64I32(1e-23);
	CMP_F64(8.533e+68);
	CMP_F64I64I32(4.1006e-184);
	CMP_F64(9.998e+307);
	CMP_F64I64I32(9.9538452227e-280);
	CMP_F64I64I32(6.47660115e-260);
	CMP_F64(7.4e+47);
	CMP_F64(5.92e+48);
	CMP_F64(7.35e+66);
	CMP_F64(8.32116e+55);
	CMP_F64I64(3.518437208883201171875E+013);
	CMP_F64I64(3.518437208883201172000E+013);

	CMP_F64I64I32(1.00000000059604644775);
	CMP_F64I64I32(1.00000000596046447755);
	CMP_F64I64I32(1.00000005960464477550);
	CMP_F64I64I32(1.00000059604644775500);
	CMP_F64I64I32(1.00000596046447755000);
	CMP_F64I64I32(1.00005960464477550000);
	CMP_F64I64I32(1.00059604644775500000);
	CMP_F64I64I32(1.00596046447755000000);
	CMP_F64I64I32(1.05960464477550000000);

	// INT64_MAX
	CMP_F64I64( 9223372036854775807);

	CMP_F64I64( 9223372036854775807E0);
	CMP_F64I64( 9223372036854775807E+0);
	CMP_F64I64( 9223372036854775807e0);
	CMP_F64I64( 9223372036854775807e+0);

	CMP_F64I64( 9.223372036854775807E18);
	CMP_F64I64( 9.223372036854775807E+18);
	CMP_F64I64( 9.223372036854775807e18);
	CMP_F64I64( 9.223372036854775807e+18);

	// INT64_MIN
	CMP_F64I64(-9.223372036854775808);

	CMP_F64I64(-9223372036854775808E0);
	CMP_F64I64(-9223372036854775808E+0);
	CMP_F64I64(-9223372036854775808e0);
	CMP_F64I64(-9223372036854775808e+0);

	CMP_F64I64(-9.223372036854775808E18);
	CMP_F64I64(-9.223372036854775808E+18);
	CMP_F64I64(-9.223372036854775808e18);
	CMP_F64I64(-9.223372036854775808e+18);

	// INT32_MAX
	CMP_F64I64I32( 2147483647);

	CMP_F64I64I32( 2147483647E0);
	CMP_F64I64I32( 2147483647E+0);
	CMP_F64I64I32( 2147483647e0);
	CMP_F64I64I32( 2147483647e+0);

	CMP_F64I64I32( 2.147483647E9);
	CMP_F64I64I32( 2.147483647E+9);
	CMP_F64I64I32( 2.147483647e9);
	CMP_F64I64I32( 2.147483647e+9);

	// INT32_MIN
	CMP_F64I64I32(-2147483648);

	CMP_F64I64I32(-2147483648E0);
	CMP_F64I64I32(-2147483648E+0);
	CMP_F64I64I32(-2147483648e0);
	CMP_F64I64I32(-2147483648e+0);

	CMP_F64I64I32(-2.147483648E9);
	CMP_F64I64I32(-2.147483648E+9);
	CMP_F64I64I32(-2.147483648e9);
	CMP_F64I64I32(-2.147483648e+9);
}
