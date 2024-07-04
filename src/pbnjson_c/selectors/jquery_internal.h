// Copyright (c) 2015-2024 LG Electronics, Inc.
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

#ifndef __JQUERY_INTERNAL_H_
#define __JQUERY_INTERNAL_H_


#include <glib.h>

#include "jquery.h"
#include "jobject.h"

#include "jquery_selectors.h"
#include "jquery_generators.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*query_context_destructor)(void *);

struct jquery
{
	// Check if current JSON satisfies the selector
	selector_filter_function sel_func;
	void *sel_ctxt;
	query_context_destructor ctxt_destructor;
	// Next query. Consumes valid JSONs, to continue filtering. May be NULL.
	jquery_ptr parent_query;
	// Object generator
	jquery_generator generator;
};

/* root_query points to the most general query, which takes
 * JSON as input. deepest_query contains its root query as
 * parent member. To optimize querying, user gets deepest_query,
 * which pulls its parent.
 *
 * ------ parent ------ parent ---------
 * |root|<-------|    |<-------|deepest|
 * ------        ------        ---------
 *                                 ^
 *                        returned to the user
 */
typedef struct
{
	jquery_ptr root_query;
	jquery_ptr deepest_query;
} jquery_pair;

typedef struct
{
	jerror **error;
	jquery_pair root_pair;
} jq_parser_context;

jquery_ptr jquery_new(selector_filter_function sfunc,
                      void *sfunc_ctxt,
                      query_context_destructor ctxt_destr,
                      jquery_generator_type generator_type);

/* Function preserves found object keys, array indexes and pointers to parents.
 * Should be used in the jquery combinators
 */
void jquery_internal_init(jquery_ptr query, jvalue_search_result JSON);

static inline void j_release_helper(jvalue_ref val)
{
    j_release(&val);
}

#ifdef __cplusplus
}
#endif

#endif // __JQUERY_INTERNAL_H_
