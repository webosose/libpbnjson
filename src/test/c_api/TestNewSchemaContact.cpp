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
#include <string>
#include <uriparser/Uri.h>

namespace {

	static const std::string resolution_dir = std::string{SCHEMA_DIR} + "contact/";
	static const std::string localref_dir = std::string{SCHEMA_DIR} + "localref/";
	static const std::string xref_dir = std::string{SCHEMA_DIR} + "xref/";

	class TestNewSchemaContact : public ::testing::Test
	{
		protected:
			static jschema_ref schema;
			static JSchemaResolver resolver;
			static JSchemaResolver xResolver;
			jvalue_ref parsed;

			static void SetUpTestCase()
			{
				resolver.m_resolve = &SimpleResolver;
				schema = jschema_parse_file_resolve((resolution_dir + "Contact.schema").c_str(),
						(resolution_dir + "Contact.schema").c_str(), nullptr, &resolver);
				ASSERT_TRUE(schema != nullptr);
				ASSERT_TRUE(jschema_resolve_ex(schema, &resolver));
			}

			static void TearDownTestCase()
			{
				jschema_release(&schema);
			}

			virtual void SetUp()
			{
				parsed = NULL;
			}

			virtual void TearDown()
			{
				j_release(&parsed);
			}

			static JSchemaResolutionResult rslv(JSchemaResolverRef resolver,
                                                jschema_ref *resolved, const char *base)
			{
				std::string resource(resolver->m_resourceToResolve.m_str,
				                resolver->m_resourceToResolve.m_len);

				// user gets file name here
				std::string absolute = base + resource;

				if (::access(absolute.c_str(), F_OK) == -1) {
					std::cerr << "SCHEMA_NOT_FOUND " << absolute.c_str() << std::endl;
					return SCHEMA_NOT_FOUND;
				}

				*resolved = jschema_fcreate(absolute.c_str(), NULL);

				if (!jschema_resolve_ex(*resolved, resolver)) {
					std::cerr << "SCHEMA_INVALID " << absolute.c_str() << std::endl;
					return SCHEMA_INVALID;
				}

				return SCHEMA_RESOLVED;
			}
			static JSchemaResolutionResult XResolver(JSchemaResolverRef resolver,
                                                     jschema_ref *resolved)
			{
				return rslv(resolver, resolved, xref_dir.c_str());
			}

			static JSchemaResolutionResult LResolver(JSchemaResolverRef resolver,
                                                     jschema_ref *resolved)
			{
				return rslv(resolver, resolved, localref_dir.c_str());
			}

			static JSchemaResolutionResult SimpleResolver(JSchemaResolverRef resolver,
					jschema_ref *resolved)
			{
				std::string resource(resolver->m_resourceToResolve.m_str, resolver->m_resourceToResolve.m_len);
				std::string lookup_path = resolution_dir + "/" + resource + ".schema";

				if (::access(lookup_path.c_str(), F_OK) == -1)
				{
					return SCHEMA_NOT_FOUND;
				}

				*resolved = jschema_parse_file(lookup_path.c_str(), NULL);
				return *resolved == NULL ? SCHEMA_INVALID : SCHEMA_RESOLVED;
			}

	};

	jschema_ref TestNewSchemaContact::schema = nullptr;
	JSchemaResolver TestNewSchemaContact::resolver;
	JSchemaResolver TestNewSchemaContact::xResolver;

} // namespace

static inline void jerror_free_ptr(jerror** err_ptr)
{
	if (err_ptr)
	{
		jerror_free(*err_ptr);
		*err_ptr = nullptr;
	}
}

static inline bool check_error_for_message(jerror* err, const std::string& message)
{
	std::string errorText(256, 0);
	jerror_to_string(err, &errorText[0], errorText.size());
	return errorText.find(message) != std::string::npos;
}

TEST_F(TestNewSchemaContact, Invalid1)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer("");

	EXPECT_FALSE(jsax_parse_with_callbacks(input, schema, nullptr, nullptr, &err));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);

	parsed = jdom_create(input, schema, &err);
	EXPECT_TRUE(jis_null(parsed));
	EXPECT_FALSE(jis_valid(parsed));
	EXPECT_FALSE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaContact, Valid1)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer(
			"{"
			"\"contactIds\": [ \"1\" ],"
			"\"displayIndex\": \"first name\""
			"}"
			);

	EXPECT_TRUE(jsax_parse_with_callbacks(input, schema, nullptr, nullptr, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);

	parsed = jdom_create(input, schema, &err);
	EXPECT_TRUE(jis_object(parsed));
	EXPECT_TRUE(jvalue_validate(parsed, schema, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaContact, Valid2)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer("{}");

	EXPECT_TRUE(jsax_parse_with_callbacks(input, schema, nullptr, nullptr, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);

	parsed = jdom_create(input, schema, &err);
	EXPECT_TRUE(jis_object(parsed));
	EXPECT_TRUE(jvalue_validate(parsed, schema, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaContact, Valid3)
{
	jerror *err = nullptr;
	raw_buffer input = j_cstr_to_buffer(
			"{"
			"\"displayName\": \"\","
			"\"name\": {},"
			"\"birthday\": \"\","
			"\"anniversary\": \"\","
			"\"gender\": \"undisclosed\""
			"}"
			);

	EXPECT_TRUE(jsax_parse_with_callbacks(input, schema, nullptr, nullptr, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);

	parsed = jdom_create(input, schema, &err);
	EXPECT_TRUE(jis_object(parsed));
	EXPECT_TRUE(jvalue_validate(parsed, schema, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);
}

TEST_F(TestNewSchemaContact, localReferences)
{
	jerror *err = nullptr;

	memset(&xResolver, 0, sizeof(xResolver));
	xResolver.m_resolve = &TestNewSchemaContact::LResolver;
	jschema_ref xschema = jschema_fcreate((localref_dir + "rA.schema").c_str(),
			nullptr);
	ASSERT_TRUE(xschema != nullptr);
	jschema_resolve_ex(xschema, &xResolver);

	raw_buffer input = j_cstr_to_buffer( R"schema(
	{
		"name": "Alisha",
		"flag": true,
		"field": {
				"familyName": "Lalala",
				"flagB": true
			}
	}
	)schema");

	EXPECT_TRUE(jsax_parse_with_callbacks(input, xschema, nullptr, nullptr, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);

	parsed = jdom_create(input, xschema, &err);
	EXPECT_TRUE(jis_object(parsed));
	EXPECT_TRUE(jvalue_validate(parsed, xschema, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);

	raw_buffer fail_input = j_cstr_to_buffer( R"schema(
	{
		"name": "Alisha",
		"flag": true,
		"field": {
				"familyName": false,
				"flagB": true
			}
	}
	)schema");

	EXPECT_FALSE(jsax_parse_with_callbacks(fail_input, xschema, nullptr, nullptr, &err));
	EXPECT_FALSE(err == nullptr);
	EXPECT_TRUE(check_error_for_message(err, "Not string"));
	jerror_free_ptr(&err);

	jschema_release(&xschema);
}

TEST_F(TestNewSchemaContact, crossReferences)
{
	jerror *err = nullptr;

	memset(&xResolver, 0, sizeof(xResolver));
	xResolver.m_resolve = &TestNewSchemaContact::XResolver;
	jschema_ref xschema = jschema_fcreate((xref_dir + "xA.schema").c_str(), nullptr);
	ASSERT_TRUE(xschema != nullptr);
	jschema_resolve_ex(xschema, &xResolver);

	raw_buffer input = j_cstr_to_buffer( R"schema(
	{
		"name": "Alisha",
		"flag": true,
		"field": {
			"familyName": "Simpson",
			"fieldA": {
				"name": "Andrii",
				"flag": false
			},
			"fieldC": {
				"stringC": "Hi",
				"fieldB": {
					"familyName": "Griffin"
				}
			}
		}
	}
	)schema");

	EXPECT_TRUE(jsax_parse_with_callbacks(input, xschema, nullptr, nullptr, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);

	parsed = jdom_create(input, xschema, &err);
	EXPECT_TRUE(jis_object(parsed));
	EXPECT_TRUE(jvalue_validate(parsed, xschema, &err));
	EXPECT_TRUE(err == nullptr);
	jerror_free_ptr(&err);

	raw_buffer fail_input = j_cstr_to_buffer( R"schema(
	{
		"name": "Alisha",
		"flag": true,
		"field": {
			"familyName": "Simpson",
			"fieldA": {
				"name": "Andrii",
				"flag": "O NO STRING!"
			}
		}
	}
	)schema");

	EXPECT_FALSE(jsax_parse_with_callbacks(fail_input, xschema, nullptr, nullptr, &err));
	EXPECT_FALSE(err == nullptr);
	EXPECT_TRUE(check_error_for_message(err, "Not boolean"));
	jerror_free_ptr(&err);

	jschema_release(&xschema);
}

// vim: set noet ts=4 sw=4 tw=80:
