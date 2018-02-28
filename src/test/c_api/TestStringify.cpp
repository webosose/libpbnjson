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

#include <pbnjson.h>
#include <gtest/gtest.h>
#include <jvalue_stringify.h>

TEST(JStringify, jvalue_prettify)
{
	const raw_buffer INPUT = j_cstr_to_buffer( R"json(
		{
			"name": "Alisha"
		}
	)json");

	JSchemaInfo schemainfo;
	jschema_info_init(&schemainfo, jschema_all(), NULL, NULL);

	jvalue_ref json = jdom_parse(INPUT, DOMOPT_NOOPT, &schemainfo);
	EXPECT_TRUE(json);
	const char* json_str = jvalue_stringify(json);
	EXPECT_TRUE(strcmp(json_str, "{\"name\":\"Alisha\"}") == 0);

	json_str = jvalue_prettify(json, "  ");
	const char* res2 = "{\n  \"name\": \"Alisha\"\n}\n";
	EXPECT_TRUE(strcmp(json_str, res2) == 0);

	json_str = jvalue_prettify(json, "\t");
	const char* res3 = "{\n\t\"name\": \"Alisha\"\n}\n";
	EXPECT_TRUE(strcmp(json_str, res3) == 0);

	json_str = jvalue_prettify(json, ".."); // not acceptable, using "  " instead
	EXPECT_TRUE(strcmp(json_str, res2) == 0);

	json_str = jvalue_prettify(NULL, "  ");
	EXPECT_TRUE(json_str == NULL);

	j_release(&json);
}
