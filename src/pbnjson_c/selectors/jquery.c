// Copyright (c) 2015-2023 LG Electronics, Inc.
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

#include "jquery_internal.h"

#include <stdio.h>

#include <glib.h>

#include "../jerror_internal.h"
#include "../liblog.h"

#include "jquery_generated_declarations.h"
#include "jquery_selectors.h"

jquery_ptr
jquery_new(selector_filter_function sfunc,
           void *sfunc_ctxt,
           query_context_destructor ctxt_destr,
           jquery_generator_type generator_type)
{
	jquery_ptr query = g_new0(struct jquery, 1);

	query->sel_func = sfunc;
	query->sel_ctxt = sfunc_ctxt;
	query->ctxt_destructor = ctxt_destr;
	query->generator.type = generator_type;

	return query;
}

void jquery_free(jquery_ptr query)
{
	if (NULL == query) return;

	jquery_free(query->parent_query);
	jq_generator_free(query->generator.next_gen);
	if (query->ctxt_destructor)
	{
		query->ctxt_destructor(query->sel_ctxt);
	}

	g_free(query);
}

jquery_ptr jquery_create(const char *str, jerror **err)
{
	CHECK_POINTER_SET_ERROR_RETURN_NULL(str, err);

	// If err is NULL, use internal error, to handle parser failure
	jerror *internal_error = NULL;
	if (NULL == err)
	{
		err = &internal_error;
	}

	yyscan_t scanner;
	JQueryScan_lex_init(&scanner);
	JQueryScan__scan_string(str, scanner);
	JQueryScan_set_extra(err, scanner);

	void *parser = JQueryParseAlloc(malloc);
	if (!parser) {
		JQueryScan_lex_destroy(scanner);
		jerror_set(err, JERROR_TYPE_INVALID_PARAMETERS, "'parser' parameter must be a non-null pointer");
		return NULL;
	}
	jq_parser_context context = { err, { NULL, NULL } };
	//JQueryParseTrace(stdout, ">> ");

	int token_id = JQueryScan_lex(scanner);
	for(; token_id > 0; token_id = JQueryScan_lex(scanner))
	{
#if false // Debugging function
		fprintf(stderr, "Got token of type %d, with value %s\n",
		        token_id,
		        JQueryScan_get_text(scanner));
#endif
		JQueryParse(parser, token_id,
		           (raw_buffer){ JQueryScan_get_text(scanner), JQueryScan_get_leng(scanner) },
		           &context);

		if (NULL != *err) break;
	}
	JQueryParse(parser, 0, (raw_buffer){ NULL, 0 }, &context);

	JQueryParseFree(parser, free);
	JQueryScan_lex_destroy(scanner);

	if (NULL != *err)
	{
		jquery_free(context.root_pair.deepest_query);
		jerror_free(internal_error);

		return NULL;
	}

	// Add recursive root generator, to iterate over the source user JSON
	if (context.root_pair.root_query)
	{
		context.root_pair.root_query->parent_query = jquery_new(selector_all, NULL, NULL, JQG_TYPE_RECURSIVE);
		return context.root_pair.deepest_query;
	}
	else
	{
		return jquery_new(selector_all, NULL, NULL, JQG_TYPE_RECURSIVE);
	}
}

static jvalue_search_result
jquery_internal_next(jquery_ptr query)
{
	while (true)
	{
		jvalue_search_result val = jq_generator_next(&query->generator);

		// If current generator still has values, proceed
		if (jis_valid(val.value))
		{
			if (query->sel_func(&val, query->sel_ctxt))
			{
				return val;
			}
			else
			{
				continue;
			}
		}
		// Generator is depleted. Ask our parent for the next element
		else if (NULL != query->parent_query)
		{
			jvalue_search_result val = jquery_internal_next(query->parent_query);
			if (jis_valid(val.value))
			{
				jq_generator_reset(&query->generator, val);
				continue;
			}
		}

		break;
	}

	// Current generator is depleted and parent query nas no elements
	return (jvalue_search_result) { jinvalid(), NULL };
}

jvalue_ref jquery_next(jquery_ptr query)
{
	jvalue_search_result result = jquery_internal_next(query);
	return result.value;
}

void
jquery_internal_init(jquery_ptr query, jvalue_search_result JSON)
{

	// Assign JSON to the root selector
	if (query->parent_query)
	{
		jq_generator_reset(&query->generator,
		                   (jvalue_search_result) { jinvalid(), NULL });
		jquery_internal_init(query->parent_query, JSON);
	}
	else
	{
		jq_generator_reset(&query->generator, JSON);
	}
}

bool jquery_init(jquery_ptr query, jvalue_ref JSON, jerror **err)
{
	CHECK_POINTER_SET_ERROR_RETURN(query, false, err, "'query' parameter must be a non-null pointer");
	CHECK_POINTER_SET_ERROR_RETURN(JSON, false, err, "'JSON' parameter must be a non-null pointer");

	jvalue_search_result val = { JSON, NULL };
	jquery_internal_init(query, val);

	return true;
}
