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

#ifndef __JQUERY_SELECTORS_H_
#define __JQUERY_SELECTORS_H_

#include <stdbool.h>

#include "jobject.h"

#include "jquery_generators.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __query_pair {
	jquery_ptr first;
	jquery_ptr second;
} *jquery_pair_ptr;

/* Filter all subelements.
jvalue_ref current element
void * next element (jobject_key_value or jvalue_ref)
void * context
*/
typedef bool (*selector_filter_function)(jvalue_search_result *, void *);

/*
 * *
 * Any node
 */
bool selector_all(jvalue_search_result *json, void *ctxt);
/*
 * :root
 * A node which is the root of the JSON document
 */
bool selector_root(jvalue_search_result *json, void *ctxt);

/*
 * T
 * A node of type T, where T is one of: string, number, object, array, boolean, or null
 * Context JValueType
 */
bool selector_type(jvalue_search_result *json, void *ctxt);

/*
 * .key
 * A node that is a child of an object, and is a property with given key
 * Context char*
 */
bool selector_key(jvalue_search_result *json, void *ctxt);

/*
 * :contains(S)
 * A node with a string value contains the substring S
 */
bool selector_contains(jvalue_search_result *json, void *ctxt);

/*
 * :has(T)
 * Context is jquery_ptr
 */
bool selector_has(jvalue_search_result *json, void *ctxt);

/*
 * :expr(E)
 * Context is SelEx *
 */
bool selector_expr(jvalue_search_result *json, void *ctxt);

/*
 * :val(V)
 * Context is jvalue_ref
 */
bool selector_value(jvalue_search_result *json, void *ctxt);

/* T > U
 * A node U with a parent T
 * Context is jquery_ptr
 */
bool selector_parent(jvalue_search_result *json, void *ctxt);

/*
 * T U
 * A node U with an ancestor T
 * Context is jquery_ptr
 */
bool selector_ancestor(jvalue_search_result *json, void *ctxt);

/*
 * T ~ U
 * A node U with a sibling T
 * Context is jquery_ptr
 */
bool selector_sibling(jvalue_search_result *json, void *ctxt);

/*
 * :empty
 * An an array or object with no child
 */
bool selector_empty(jvalue_search_result *json, void *ctxt);

/*
 * :only-child
 * A node, which is the only child of an array
 */
bool selector_only_child(jvalue_search_result *json, void *ctxt);

/*
 * :nth-child(N) :nth-last-child(N) :first-child :last-child
 * A node, which is the nth child of an array parent
 * Context is int32_t - index
 */
bool selector_nth_child(jvalue_search_result *json, void *ctxt);

/*
 * S1, S2
 * Any node which matches either selector S1 or S2
 * Context is jquery_pair
 */
bool selector_or(jvalue_search_result *json, void *ctxt);
void jquery_pair_ptr_free(jquery_pair_ptr pair);

#ifdef __cplusplus
}
#endif

#endif //__JQUERY_SELECTORS_H_
