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

#include "jquery_generators.h"

#include <stdio.h>

jquery_generator_ptr jq_generator_new(jvalue_search_result json, int type)
{
	jquery_generator_ptr result = g_new0(jquery_generator, 1);

	result->json = json;
	result->type = type;

	return result;
}

void jq_generator_reset(jquery_generator_ptr generator, jvalue_search_result json)
{
	if (NULL == generator) return;

	generator->json = json;
	generator->array_iterator = 0;
	generator->self_returned = false;
}

void jq_generator_free(jquery_generator_ptr generator)
{
	if (NULL == generator) return;

	if (NULL != generator->next_gen)
	{
		jq_generator_free(generator->next_gen);
	}
	g_free(generator);
}

static void
jq_reset_recursive_generator(jquery_generator_ptr parent_generator, jvalue_ref json,
                             ssize_t value_index, jvalue_ref value_key)
{
	jvalue_search_result next_element = { json, &parent_generator->json, value_index, value_key };

	if (NULL != parent_generator->next_gen)
	{
		jq_generator_reset(parent_generator->next_gen, next_element);
	}
	else
	{
		// If our generator is recursive, next generator should be recursive as well
		// otherwise it should be self-generator (just return our children)
		parent_generator->next_gen = jq_generator_new(next_element,
		     parent_generator->type & JQG_TYPE_DESCENDANTS ? JQG_TYPE_RECURSIVE : JQG_TYPE_SELF);
	}
}

jvalue_search_result jq_generator_next(jquery_generator_ptr generator)
{
	// If generator is self-return or recursive, we must return element itself
	if (!generator->self_returned
	    && (generator->type & JQG_TYPE_SELF))
	{
		generator->self_returned = true;

		return generator->json;
	}
	// We've returned element itself, and don't have neither RECURSIVE
	// nor DESCENDANTS flags. Just return invalid result for all following
	// requests.
	else if (!(~JQG_TYPE_SELF & generator->type))
	{
		return (jvalue_search_result){ jinvalid(), generator->json.parent };
	}

	// If the object is not new, we probably have valid recursive generator
	if (NULL != generator->next_gen)
	{
		jvalue_search_result element = jq_generator_next(generator->next_gen);

		if (jis_valid(element.value))
		{
			return element;
		}
	}

	// If we are here, our recursive generator is depleted and we've returned the
	// element itself. Let's iterate over our children.
	if (jis_array(generator->json.value)
		&& generator->array_iterator < jarray_size(generator->json.value))
	{
		jvalue_ref element = jarray_get(generator->json.value, generator->array_iterator);

		jq_reset_recursive_generator(generator, element, generator->array_iterator, jinvalid());

		++generator->array_iterator;
		return jq_generator_next(generator);
	}
	// Else if our value is an object, feed our next selector with the object values.
	else if (jis_object(generator->json.value))
	{
		// New object - init iterator
		if (generator->array_iterator == 0)
		{
			jobject_iter_init(&generator->object_iterator, generator->json.value);
		}

		jobject_key_value keyval;
		if (generator->array_iterator < g_hash_table_size(g_hash_table_iter_get_hash_table(&generator->object_iterator.m_iter))
		    && jobject_iter_next(&generator->object_iterator, &keyval))
		{
			++generator->array_iterator;
			jq_reset_recursive_generator(generator, keyval.value, -1, keyval.key);

			return jq_generator_next(generator);
		}
	}

	return (jvalue_search_result) { jinvalid(), NULL };
}
