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

#include "Utils.hpp"

#include <memory>
#include <functional>

std::string getErrorString(jerror *error) {
	const unsigned int err_size = 170;

	char result[err_size];
	jerror_to_string(error, result, err_size);

	return result;
}

std::function<decltype(jquery_free)> q_destr(jquery_free);

jvalue_ref getFirstQueryResult(const char *query_str, jvalue_ref json, jerror **err)
{
	std::unique_ptr<struct jquery, decltype(q_destr)> query (jquery_create(query_str, err), q_destr);
	if (!query)
	{
		fprintf(stderr, "Error parsing query string: %s\n", getErrorString(*err).c_str());
		return jinvalid();
	}

	if (!jquery_init(query.get(), json, err))
	{
		fprintf(stderr, "Ivalid json passed: %s\n", getErrorString(*err).c_str());
		return jinvalid();
	}

	jvalue_ref query_result = jquery_next(query.get());

	return query_result;
}


std::vector<pbnjson::JValue> getAllQueryResults(const char *query_str, jvalue_ref json, jerror **err)
{
	std::unique_ptr<struct jquery, decltype(q_destr)> query (jquery_create(query_str, err), q_destr);

	if (!query)
	{
		fprintf(stderr, "Error parsing query string: %s\n", getErrorString(*err).c_str());
		return { jinvalid() };
	}

	if (!jquery_init(query.get(), json, err))
	{
		fprintf(stderr, "Ivalid json passed: %s\n", getErrorString(*err).c_str());
		return { jinvalid() };
	}

	std::vector<pbnjson::JValue> result;
	jvalue_ref query_result = jquery_next(query.get());
	while (jis_valid(query_result))
	{
		result.emplace_back(jvalue_copy(query_result));

		query_result = jquery_next(query.get());
	}

	return result;
}
