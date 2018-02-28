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

#include "jquery_selectors.h"

#include "../jobject_internal.h"
#include "jquery_internal.h"
#include "expression.h"

bool selector_all(jvalue_search_result *json, void *ctxt)
{
	return true;
}

bool selector_root(jvalue_search_result *json, void *ctxt)
{
	return json->parent == NULL;
}

bool selector_type(jvalue_search_result *json, void *ctxt)
{
	JValueType type = (JValueType)ctxt;

	return json->value->m_type == type;
}

bool selector_key(jvalue_search_result *json, void *ctxt)
{
	assert(ctxt);

	const char *key = (const char *) ctxt;

	if (!jis_valid(json->value_key))
		return false;

	raw_buffer test = jstring_get_fast(json->value_key);
	if (strlen(key) != test.m_len)
		return false;

	return memcmp(key, test.m_str, test.m_len) == 0;
}

bool selector_contains(jvalue_search_result *json, void *ctxt)
{
	assert(ctxt);

	if (!jis_string(json->value))
	{
		return false;
	}

	const char *substring = (const char *) ctxt;
	raw_buffer string = jstring_get_fast(json->value);

	return (NULL != g_strstr_len(string.m_str, string.m_len, substring));
}

bool selector_has(jvalue_search_result *json, void *ctxt)
{
	assert(ctxt);
	jquery_ptr q = (jquery_ptr) ctxt;

	jquery_init(q, json->value, NULL);
	return jis_valid(jquery_next(q));
}

bool selector_expr(jvalue_search_result *json, void *ctxt)
{
	assert(ctxt);
	SelEx *ex = (SelEx *) ctxt;
	return sel_ex_eval(ex, json->value);
}

bool selector_value(jvalue_search_result *json, void *ctxt)
{
	assert(ctxt);
	jvalue_ref value = (jvalue_ref)ctxt;

	if (!jis_valid(value))
		return false;

	return jvalue_equal(value, json->value);
}

bool selector_parent(jvalue_search_result *json, void *ctxt)
{
	if (json->parent)
	{
		assert(ctxt);
		jquery_ptr q = (jquery_ptr) ctxt;

		jquery_internal_init(q, *json->parent);
		return jis_valid(jquery_next(q));
	}

	return false;
}

bool selector_ancestor(jvalue_search_result *json, void *ctxt)
{
	jvalue_search_result *parent = json->parent;

	if (NULL == parent) return false;

	assert(ctxt);
	jquery_ptr q = (jquery_ptr) ctxt;

	do
	{
		jquery_internal_init(q, *parent);
		if (jis_valid(jquery_next(q))) return true;
	}
	while (NULL != (parent = parent->parent));

	return false;
}

bool selector_sibling(jvalue_search_result *json, void *ctxt)
{
	jvalue_search_result *parent = json->parent;

	if (NULL == parent) return false;

	assert(ctxt);
	jquery_ptr q = (jquery_ptr) ctxt;

	if (jis_object(parent->value))
	{
		jobject_iter it;
		jobject_iter_init(&it, parent->value);
		jobject_key_value kv;
		while (jobject_iter_next(&it, &kv))
		{
			if (kv.key != json->value_key)
			{
				jquery_internal_init(q, (jvalue_search_result){ kv.value, parent, -1, kv.key });
				if (jis_valid(jquery_next(q))) return true;
			}
		}
	}
	else if (jis_array(parent->value))
	{
		for (ssize_t i = 0; i < jarray_size(parent->value); ++i)
		{
			if (i != json->value_index)
			{
				jquery_internal_init(q, (jvalue_search_result){ jarray_get(parent->value, i), parent, i, NULL });
				if (jis_valid(jquery_next(q))) return true;
			}
		}
	}

	return false;
}

bool selector_empty(jvalue_search_result *json, void *ctxt)
{
	if (jis_object(json->value))
	{
		return jobject_size(json->value) == 0;
	}
	else if (jis_array(json->value))
	{
		return jarray_size(json->value) == 0;
	}

	return false;
}

bool selector_only_child(jvalue_search_result *json, void *ctxt)
{
	jvalue_search_result *parent = json->parent;

	return parent && jis_array(parent->value) && jarray_size(parent->value) == 1;
}

bool selector_nth_child(jvalue_search_result *json, void *ctxt)
{
	jvalue_search_result *parent = json->parent;

	if (parent && jis_array(parent->value))
	{
		assert(ctxt);

		int index = GPOINTER_TO_INT(ctxt);

		return index >= 1 ?
		       index - 1 == json->value_index :
		       jarray_size(parent->value) + index == json->value_index;
	}

	return false;
}

bool selector_or(jvalue_search_result *json, void *ctxt)
{
	assert(ctxt);

	jquery_pair_ptr pair = (jquery_pair_ptr) ctxt;

	jquery_internal_init(pair->first, *json);
	if (jis_valid(jquery_next(pair->first))) return true;

	jquery_internal_init(pair->second, *json);
	return jis_valid(jquery_next(pair->second));
}

void jquery_pair_ptr_free(jquery_pair_ptr pair)
{
	jquery_free(pair->first);
	jquery_free(pair->second);
	g_free(pair);
}
