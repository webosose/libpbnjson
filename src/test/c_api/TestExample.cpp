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
#include <pbnjson.h>

using namespace std;

TEST(TestExample, Generate)
//! [generate]
{
	//create jvalue with 2 members - errorCode - jnumber, errorText - jstring
	jvalue_ref my_response = jobject_create_var(
		jkeyval(J_CSTR_TO_JVAL("errorCode"), jnumber_create_i32(5)),
		jkeyval(J_CSTR_TO_JVAL("errorText"), jstring_create("This is an example of a pbnjson object")),
		J_END_OBJ_DECL
	);

	//serialized is: {"errorCode":5,"errorText":"This is an example of a pbnjson object"}
	const char *serialized = jvalue_stringify(my_response); // asStr lives as long as responseSchema

	fprintf(stdout, "%s", serialized);
	j_release(&my_response);
}
//! [generate]

TEST(TestExample, ParseAndSchema)
{
//! [parse and validate]
	jerror *error = NULL;
	const char *input = "{\"guess\": 7, \"returnValue\": true}";

	jschema_ref schema = jschema_fcreate(SCHEMA_DIR "input.schema", &error);
	if (error) {
		char str_buf[256];
		jerror_to_string(error, str_buf, sizeof(str_buf));
		fprintf(stderr, "Failed to parse schema: %s\n", str_buf);
		jerror_free(error);
		return;
	}

	jvalue_ref parsed = jdom_create(j_cstr_to_buffer(input), schema, &error);
	jschema_release(&schema);

	if (error) {
		// input failed to parse (this is OK since we only allow parsing of top level elements (an object or array)
		char str_buf[256];
		jerror_to_string(error, str_buf, sizeof(str_buf));
		fprintf(stderr, "Failed to parse input: %s\n", str_buf);
		jerror_free(error);
		return;
	}

    // this is always guaranteed to print a number between 1 & 10.  no additional validation necessary within the code.
	int guess;
	jnumber_get_i32(jobject_get(parsed, J_CSTR_TO_BUF("guess")), &guess);
	fprintf(stdout, "The guess was %d\n", guess);

	j_release(&parsed);
//! [parse and validate]
}

TEST(TestExample, ParseAll)
{
//! [parse all]
	jerror *error = NULL;
	const char *input = "{\"guess\": 7, \"returnValue\": true}";

	jvalue_ref parsed = jdom_create(j_cstr_to_buffer(input), jschema_all(), &error);

	if (error) {
		// input failed to parse (this is OK since we only allow parsing of top level elements (an object or array)
		char str_buf[256];
		jerror_to_string(error, str_buf, sizeof(str_buf));
		fprintf(stderr, "Failed to parse input: %s\n", str_buf);
		jerror_free(error);
		return;
	}

	// Be careful to check whether "guess" is number at all!
	int guess;
	jnumber_get_i32(jobject_get(parsed, J_CSTR_TO_BUF("guess")), &guess);
	fprintf(stdout, "The guess was %d\n", guess);

	j_release(&parsed);
//! [parse all]
}

TEST(TestExample, StreamParserDom)
{
//! [parse stream dom]
	const char input_json[] = "{\"number\":1, \"str\":\"asd\"}";

	//create parser
	jdomparser_ref parser = jdomparser_new(jschema_all());
	if (!parser)
		return;

	// Call jdomparser_feed for every part of incoming json string. It will be done byte by byte.
	for (const char *i = input_json, *i_end = i + strlen(input_json);
		 i != i_end;
		 ++i)
	{
		if (!jdomparser_feed(parser, i, 1)) {
			// Get error description
			fprintf(stderr, "Parse error: %s\n", jdomparser_get_error(parser));
			jdomparser_release(&parser);
			return;
		}
	}

	// Finalize parsing
	if (!jdomparser_end(parser)) {
		// Get error description
		fprintf(stderr, "Parse error: %s\n", jdomparser_get_error(parser));
		jdomparser_release(&parser);
		return;
	}

	// Get root of json tree
	jvalue_ref jval = jdomparser_get_result(parser);

	// Release parser
	jdomparser_release(&parser);

	// Release json object
	j_release(&jval);
//! [parse stream dom]
}

//! [parse stream sax]
int on_null(JSAXContextRef ctxt) {
	// get pointer that was passed to jsaxparser_init
	//void *orig_context = jsax_getContext(ctxt);

	// return 1 to continue parsing
	return 1;
}
int on_boolean(JSAXContextRef ctxt, bool value)                             {return 1;}
int on_number(JSAXContextRef ctxt, const char *number, size_t numberLen)    {return 1;}
int on_string(JSAXContextRef ctxt, const char *string, size_t stringLen)    {return 1;}
int on_object_start(JSAXContextRef ctxt)                                    {return 1;}
int on_object_key(JSAXContextRef ctxt, const char *key, size_t keyLen)      {return 1;}
int on_object_end(JSAXContextRef ctxt)                                      {return 1;}
int on_array_start(JSAXContextRef ctxt)                                     {return 1;}
int on_array_end(JSAXContextRef ctxt)                                       {return 1;}

bool parse_stream_sax()
{
	const char input_json[] = "{\"number\":1, \"str\":\"asd\"}";

	// initialize sax callback structure.
	PJSAXCallbacks callbacks;
	callbacks.m_objStart    = on_object_start;
	callbacks.m_objKey      = on_object_key;
	callbacks.m_objEnd      = on_object_end;
	callbacks.m_arrStart    = on_array_start;
	callbacks.m_arrEnd      = on_array_end;
	callbacks.m_string      = on_string;
	callbacks.m_number      = on_number;
	callbacks.m_boolean     = on_boolean;
	callbacks.m_null        = on_null;

	// Pointer that will be passed to callback. void * for simplicity
	void *callback_ctxt = NULL;

	//create parser
	jsaxparser_ref parser = jsaxparser_new(jschema_all(), &callbacks, callback_ctxt);
	if (!parser)
	{
		fprintf(stderr, "Failed to create parser\n");
		return false;
	}

	// Call jsaxparser_feed for every part of incoming json string. It will be done byte by byte.
	for (const char *i = input_json, *i_end = i + strlen(input_json);
		 i != i_end;
		 ++i)
	{
		if (!jsaxparser_feed(parser, i, 1)) {
			// Get error description
			fprintf(stderr, "Parse error: %s\n", jsaxparser_get_error(parser));
			jsaxparser_release(&parser);
			return false;
		}
	}

	if (!jsaxparser_end(parser)) {
		// Get error description
		fprintf(stderr, "Parse error: %s\n", jsaxparser_get_error(parser));
		jsaxparser_release(&parser);
		return false;
	}

	// Release parser
	jsaxparser_release(&parser);

	return true;
}
//! [parse stream sax]

TEST(TestExample, ParseStreamSax)
{
	EXPECT_TRUE(parse_stream_sax());
}

// vim: set noet ts=4 sw=4:
