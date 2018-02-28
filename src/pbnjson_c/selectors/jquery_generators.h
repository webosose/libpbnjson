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

#ifndef __JQUERY_GENERATORS_H_
#define __JQUERY_GENERATORS_H_

#include <stdbool.h>

#include "jobject.h"
#include "jquery.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _jvalue_search_result jvalue_search_result;

struct _jvalue_search_result
{
	jvalue_ref value;
	jvalue_search_result *parent;
	ssize_t value_index;
	jvalue_ref value_key;
};

typedef jvalue_search_result(*jq_generator_function)(jvalue_search_result *, void *);

typedef enum
{
	JQG_TYPE_CHILDREN = 1,
	JQG_TYPE_DESCENDANTS = 2,
	JQG_TYPE_SELF = 4,
	JQG_TYPE_RECURSIVE = JQG_TYPE_DESCENDANTS | JQG_TYPE_SELF
} jquery_generator_type;

typedef struct _jquery_generator jquery_generator;
typedef jquery_generator *jquery_generator_ptr;

struct _jquery_generator
{
	jvalue_search_result json;
	// Contains array index if json is an array, or jobject iterator
	// position if json is an object.
	ssize_t array_iterator;
	// jobject iterator, if json is an object
	jobject_iter object_iterator;
	// Self element was return (if recursive generator, or self generator.
	bool self_returned;
	// Recursive generator
	jquery_generator_type type;
	jquery_generator_ptr next_gen;
};

jquery_generator_ptr jq_generator_new(jvalue_search_result json, int type);
void jq_generator_reset(jquery_generator_ptr generator, jvalue_search_result json);
jvalue_search_result jq_generator_next(jquery_generator_ptr generator);
void jq_generator_free(jquery_generator_ptr generator);

#ifdef __cplusplus
}
#endif

#endif // __JQUERY_GENERATORS_H_
