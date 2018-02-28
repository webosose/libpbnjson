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
#include <pbnjson.h>
#include <pbnjson.hpp>
#ifdef HAVE_JSON_C
#include <json-c/json.h>
#endif
#include <yajl.h>
#include "PerformanceUtils.hpp"
#include "TestUtils.hpp"

using namespace std;

namespace {

class JSchemaC : public pbnjson::JSchema
{
public:
	JSchemaC(jschema_ref schema) :
		JSchema(schema ? jschema_copy(schema) : NULL)
	{}
};

#if HAVE_CJSON
void ParseCjson(raw_buffer const &input)
{
	unique_ptr<json_object, void(*)(json_object*)>
		o{json_tokener_parse(input.m_str), &json_object_put};
	ASSERT_TRUE(o.get());
	ASSERT_FALSE(is_error(o.get()));
}
#endif // HAVE_CJSON

void ParsePbnjson(raw_buffer const &input, JDOMOptimizationFlags opt, jschema_ref schema)
{
	JSchemaInfo schemaInfo;
	jschema_info_init(&schemaInfo, schema, NULL, NULL);
	unique_ptr<jvalue, function<void(jvalue_ref &)>>
		jv{jdom_parse(input, opt, &schemaInfo), [](jvalue_ref &v) { j_release(&v); }};

	ASSERT_TRUE(jis_valid(jv.get()));
}

void ParsePbnjsonPp(raw_buffer const &input, JDOMOptimizationFlags opt, jschema_ref schema)
{
	pbnjson::JDomParser parser;
	parser.changeOptimization((JDOMOptimization)opt);

	bool answer = parser.parse(std::string(input.m_str, input.m_len), JSchemaC(schema));

	ASSERT_TRUE( answer );
}

#if HAVE_YAJL
void ParseYajl(raw_buffer const &input)
{
	yajl_callbacks nocb = { 0 };
	unique_ptr<remove_pointer<yajl_handle>::type, void(*)(yajl_handle)>
		handle{
#if YAJL_VERSION < 20000
			yajl_alloc(&nocb, NULL, NULL, NULL),
#else
			yajl_alloc(&nocb, NULL, NULL),
#endif
			&yajl_free
		};

	ASSERT_EQ(yajl_status_ok,
		yajl_parse(handle.get(), (const unsigned char *)input.m_str, input.m_len));
	ASSERT_EQ(yajl_status_ok,
#if YAJL_VERSION < 20000
	          yajl_parse_complete(handle.get())
#else
	          yajl_complete_parse(handle.get())
#endif
	          );
}
#endif // HAVE_YAJL

void ParseSax(raw_buffer const &input, jschema_ref schema)
{
	JSchemaInfo schemaInfo;
	jschema_info_init(&schemaInfo, schema, NULL, NULL);

	ASSERT_TRUE(jsax_parse(NULL, input, &schemaInfo));
}

void ParsePpSax(raw_buffer const &input, jschema_ref schema)
{
	struct NoopParser final : pbnjson::JParser
	{
		bool jsonObjectOpen() override { return true; }
		bool jsonObjectKey(const std::string &) { return true; }
		bool jsonObjectClose() override { return true; }
		bool jsonArrayOpen() override { return true; }
		bool jsonArrayClose() override { return true; }
		bool jsonString(const std::string &) override { return true; }
		bool jsonNumber(const std::string &) override { return true; }
		bool jsonBoolean(bool) override { return true; }
		bool jsonNull() override { return true; }
		NumberType conversionToUse() const override { return JNUM_CONV_RAW; }
	};

	struct JSchemaRef final : pbnjson::JSchema
	{
		JSchemaRef(jschema_ref schema) : JSchema(schema) {}
		~JSchemaRef() override { set(nullptr); } // prevent additional schema unref
	};

	ASSERT_TRUE(NoopParser().parse(input, JSchemaRef{schema}));
}


#define ITERATION_STEP 5

void ParseSaxIterate(raw_buffer const &input, jschema_ref schema)
{
	JSchemaInfo schemaInfo;
	jschema_info_init(&schemaInfo, schema, NULL, NULL);

	jsaxparser_ref parser = jsaxparser_create(&schemaInfo, NULL, NULL);

	ASSERT_TRUE(parser);

	const char* start = input.m_str;
	const char* end = input.m_str + input.m_len;
	int modulo = input.m_len % ITERATION_STEP;
	const char* i = start;
	for (; i < end - modulo; i += ITERATION_STEP) {
		if (!jsaxparser_feed(parser, i, ITERATION_STEP)) {
			ASSERT_TRUE(0);
			break;
		}
	}

	ASSERT_TRUE(jsaxparser_feed(parser, i, modulo));
	ASSERT_TRUE(jsaxparser_end(parser));
	jsaxparser_release(&parser);
}

void ParseDomIterate(raw_buffer const &input, jschema_ref schema)
{
	JSchemaInfo schemaInfo;
	jschema_info_init(&schemaInfo, schema, NULL, NULL);

	jdomparser_ref parser = jdomparser_create(&schemaInfo, 0);

	ASSERT_TRUE(parser);

	const char* start = input.m_str;
	const char* end = input.m_str + input.m_len;
	int modulo = input.m_len % ITERATION_STEP;
	const char* i = start;
	for (; i < end - modulo; i += ITERATION_STEP) {
		if (!jdomparser_feed(parser, i, ITERATION_STEP)) {
			ASSERT_TRUE(0);
			break;
		}
	}

	ASSERT_TRUE(jdomparser_feed(parser, i, modulo));
	ASSERT_TRUE(jdomparser_end(parser));
	jvalue_ref jval = jdomparser_get_result(parser);
	ASSERT_TRUE(jis_valid(jval));

	jdomparser_release(&parser);
	j_release(&jval);
}

/**
 * Walk through whole hierarchy of jvalue 's while accessing values they are storing
 */
void PerformAccess(jvalue_ref val)
{
	switch (jget_type(val))
	{
	case JV_NULL:
		break;
	case JV_BOOL:
		{
			bool bval;
			(void) jboolean_get(val, &bval);
		}
		break;
	case JV_NUM:
		{
			int32_t i32val;
			int64_t i64val;
			double f64val;
			raw_buffer rawval;
			(void) jnumber_get_i32(val, &i32val);
			(void) jnumber_get_i64(val, &i64val);
			(void) jnumber_get_f64(val, &f64val);
			(void) jnumber_get_raw(val, &rawval);
		}
		break;
	case JV_STR:
		(void) jstring_get_fast(val);
		break;
	case JV_ARRAY:
		{
			ssize_t n = jarray_size(val);
			for (ssize_t i = 0; i < n; ++i)
			{
				PerformAccess(jarray_get(val, i));
			}
		}
		break;
	case JV_OBJECT:
		{
			jobject_iter iter;
			jobject_key_value keyval;
			(void) jobject_iter_init(&iter, val);
			while (jobject_iter_next(&iter, &keyval))
			{
				PerformAccess(keyval.key);
				PerformAccess(keyval.value);
			}
		}
		break;
	}
}

void BenchmarkMBps(const std::string &label, size_t elem_size, std::function<void(size_t)> f)
{
	double seconds = BenchmarkPerform(f);

	size_t width = cout.width();
	size_t precision = cout.precision(1);
	auto fflags = cout.flags();

	cout << left << setw(24) << label << " "
	     << right << setw(8) << fixed << ConvertToMBps(elem_size, seconds)
	     << " MB/s (size: " << elem_size << " bytes)" << endl;

	(void) cout.flags(fflags);
	(void) cout.precision(precision);
	(void) cout.width(width);
}

const int OPT_NONE = DOMOPT_NOOPT;

const int OPT_ALL = DOMOPT_INPUT_NOCHANGE
                  | DOMOPT_INPUT_OUTLIVES_DOM
                  | DOMOPT_INPUT_NULL_TERMINATED;

const raw_buffer small_inputs[] =
{
	J_CSTR_TO_BUF("{}"),
	J_CSTR_TO_BUF("[]"),
	J_CSTR_TO_BUF("[\"e1\", \"e2\", \"e3\"]"),
	J_CSTR_TO_BUF("{ \"returnValue\" : true }"),
	J_CSTR_TO_BUF("{ \"returnValue\" : true, \"results\" : [ { \"property\" : \"someName\", \"value\" : 40.5 } ] }")
};

const size_t small_inputs_size = []() {
	size_t size = 0;
	for (const raw_buffer &rb : small_inputs)
		size += rb.m_len;
	return size;
}();

const raw_buffer big_input = J_CSTR_TO_BUF(
	"{ "
	"\"o1\" : null, "
	"\"o2\" : {}, "
	"\"a1\" : null, "
	"\"a2\" : [], "
	"\"o3\" : {"
		"\"x\" : true, "
		"\"y\" : false, "
		"\"z\" : \"\\\"es'ca'pes'\\\"\""
	"}, "
	"\"n1\" : 0"
	"                              "
	",\"n2\" : 232452312412, "
	"\"n3\" : -233243.653345e-2342 "
	"                              "
	",\"s1\" : \"adfa\","
	"\"s2\" : \"asdflkmsadfl jasdf jasdhf ashdf hasdkf badskjbf a,msdnf ;whqoehnasd kjfbnakjd "
	"bfkjads fkjasdbasdf jbasdfjk basdkjb fjkndsab fjk\","
	"\"a3\" : [ true, false, null, true, false, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}],"
	"\"a4\" : [[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[]],"
	"\"n4\" : 928437987349237893742897234987243987234297982347987249387,"
	"\"b1\" : true"
	"}");
const size_t big_input_size = big_input.m_len;
} //namespace;

TEST(Performance, ParseSmallInput)
{
	cout << "Performance test result in megabytes per second, bigger is better." << endl;
	cout << "Parsing small JSON (size: " << small_inputs_size << " bytes), MBps:" << endl;
}

#if HAVE_YAJL
TEST(Performance, ParseSmallYajl)
{
	BenchmarkMBps("YAJL:", small_inputs_size, [&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParseYajl(rb);
			}
		});
}
#endif

#if HAVE_CJSON
TEST(Performance, ParseSmallCJson)
{
	BenchmarkMBps("CJSON:", small_inputs_size, [&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParseCjson(rb);
			}
		});
}
#endif

TEST(Performance, ParseSmallPbnjsonSax)
{
	BenchmarkMBps("pbnjson-sax:", small_inputs_size, [&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParseSax(rb, jschema_all());
			}
		});
}

TEST(Performance, ParseSmallPbnjsonPpSax)
{
	BenchmarkMBps("pbnjson++-sax:", small_inputs_size, [&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParsePpSax(rb, jschema_all());
			}
		});
}

TEST(Performance, ParseSmallPbnjsonDomNoOpt)
{
	BenchmarkMBps("pbnjson (-opts):", small_inputs_size, [&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParsePbnjson(rb, OPT_NONE, jschema_all());
			}
		});
}

TEST(Performance, ParseSmallPbnjsonDomPPNoOpt)
{
	BenchmarkMBps("pbnjson++ (-opts):", small_inputs_size, [&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParsePbnjsonPp(rb, OPT_NONE, jschema_all());
			}
		});
}

TEST(Performance, ParseSmallPbnjsonDomOpt)
{
	BenchmarkMBps("pbnjson (+opts):", small_inputs_size, [&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParsePbnjson(rb, OPT_ALL, jschema_all());
			}
		});
}

TEST(Performance, ParseSmallPbnjsonDomPPOpt)
{
	BenchmarkMBps("pbnjson++ (+opts):", small_inputs_size, [&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParsePbnjsonPp(rb, OPT_ALL, jschema_all());
			}
		});
}

TEST(Performance, ParseSmallPbnjsonSaxIter)
{
	BenchmarkMBps("pbnjson-sax iterate:", small_inputs_size, [&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParseSaxIterate(rb, jschema_all());
			}
		});
}

TEST(Performance, ParseSmallPbnjsonDomIter)
{
	BenchmarkMBps("pbnjson-dom iterate:", small_inputs_size, [&](size_t n)
		{
			for (; n > 0; --n)
			{
				for (auto const &rb : small_inputs)
					ParseDomIterate(rb, jschema_all());
			}
		});
}

#if HAVE_YAJL
TEST(Performance, ParseBigYajl)
{
	BenchmarkMBps("YAJL:", big_input_size, [&](size_t n)
		{
			for (; n > 0; --n)
				ParseYajl(big_input);
		});
}
#endif

#if HAVE_CJSON
TEST(Performance, ParseBigCJson)
{
	BenchmarkMBps("CJSON:", big_input_size, [&](size_t n)
		{
			for (; n > 0; --n)
				ParseCjson(big_input);
		});
}
#endif

TEST(Performance, ParseBigPbnjsonSax)
{
	BenchmarkMBps("pbnjson-sax:", big_input_size, [&](size_t n)
		{
			for (; n > 0; --n)
				ParseSax(big_input, jschema_all());
		});
}

TEST(Performance, ParseBigPbnjsonPpSax)
{
	BenchmarkMBps("pbnjson++-sax:", big_input_size, [&](size_t n)
		{
			for (; n > 0; --n)
				ParsePpSax(big_input, jschema_all());
		});
}

TEST(Performance, ParseBigPbnjsonDomNoOpts)
{
	BenchmarkMBps("pbnjson (-opts):", big_input_size, [&](size_t n)
		{
			for (; n > 0; --n)
				ParsePbnjson(big_input, OPT_NONE, jschema_all());
		});
}

TEST(Performance, ParseBigPbnjsonDomPPNoOpts)
{
	BenchmarkMBps("pbnjson++ (-opts):", big_input_size, [&](size_t n)
		{
			for (; n > 0; --n)
				ParsePbnjsonPp(big_input, OPT_NONE, jschema_all());
		});
}

TEST(Performance, ParseBigPbnjsonDomOpts)
{
	BenchmarkMBps("pbnjson (+opts):", big_input_size, [&](size_t n)
		{
			for (; n > 0; --n)
				ParsePbnjson(big_input, OPT_ALL, jschema_all());
		});
}

TEST(Performance, ParseBigPbnjsonDomPPOpts)
{
	BenchmarkMBps("pbnjson++ (+opts):", big_input_size, [&](size_t n)
		{
			for (; n > 0; --n)
				ParsePbnjsonPp(big_input, OPT_ALL, jschema_all());
		});
}

TEST(Performance, ParseBigPbnjsonSaxIter)
{
	BenchmarkMBps("pbnjson-sax iterate:", big_input_size, [&](size_t n)
		{
			for (; n > 0; --n)
				ParseSaxIterate(big_input, jschema_all());
		});
}

TEST(Performance, ParseBigPbnjsonDomIter)
{
	BenchmarkMBps("pbnjson-dom iterate:", big_input_size, [&](size_t n)
		{
			for (; n > 0; --n)
				ParseDomIterate(big_input, jschema_all());
		});
}

TEST(Performance, AccessBigPbnjsonDom)
{
	auto root = mk_ptr(jdom_create(big_input, jschema_all(), nullptr));

	BenchmarkMBps("pbnjson-dom access:", big_input_size, [&](size_t n)
		{
			for (; n > 0; --n)
			{
				PerformAccess(root.get());
			}
		});
}

TEST(Performance, AccessBoolsPbnjsonDom)
{
	constexpr size_t count = 1000;
	auto root = mk_ptr(jarray_create_hint(nullptr, count));
	for (size_t i = 0; i < count; ++i)
	{
		(void) jarray_put(root.get(), i, jboolean_create((i & 1) == 0 ? true : false));
	}
	size_t json_size = strlen(jvalue_stringify(root.get()));

	BenchmarkMBps("pbnjson-dom access (bool):", json_size, [&](size_t n)
		{
			for (; n > 0; --n)
			{
				PerformAccess(root.get());
			}
		});
}

// vim: set noet ts=4 sw=4:
