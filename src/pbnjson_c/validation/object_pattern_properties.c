// Copyright (c) 2016-2018 LG Electronics, Inc.
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

#include "object_pattern_properties.h"

#include <assert.h>

#include "validator.h"
#include "combined_validator.h"


typedef struct _Entry
{
	GRegex *regex;          // Regex of property name
	Validator *validator;   // Validator on the property value
} Entry;

static void entry_free(Entry *entry)
{
	g_regex_unref(entry->regex);
	validator_unref(entry->validator);
	g_free(entry);
}

static void release(Feature *f)
{
	ObjectPatternProperties *o = (ObjectPatternProperties *) f;
	g_slist_free_full(o->patterns, (GDestroyNotify) entry_free);
	g_free(o);
}

static Validator* apply(Feature *f, Validator *v)
{
	assert(f);
	return validator_set_object_pattern_properties(v, (ObjectPatternProperties *) f);
}

static FeatureVtable object_pattern_properties_vtable =
{
	.release = release,
	.apply = apply,
};

ObjectPatternProperties* object_pattern_properties_new(void)
{
	ObjectPatternProperties *o = g_new0(ObjectPatternProperties, 1);
	feature_init(&o->base, &object_pattern_properties_vtable);
	return o;
}

ObjectPatternProperties* object_pattern_properties_ref(ObjectPatternProperties *o)
{
	return (ObjectPatternProperties *) feature_ref(&o->base);
}

void object_pattern_properties_unref(ObjectPatternProperties *o)
{
	feature_unref(&o->base);
}

bool object_pattern_properties_add(ObjectPatternProperties *o, const char *pattern, size_t pattern_len, Validator *v)
{
	// Create a null-terminated copy of the pattern
	char buffer[pattern_len + 1];
	memcpy(buffer, pattern, pattern_len);
	buffer[pattern_len] = 0;

	Entry *entry = g_new0(Entry, 1);
	entry->validator = v;
	entry->regex = g_regex_new(buffer, G_REGEX_JAVASCRIPT_COMPAT, 0, NULL);
	if (!entry->regex)
	{
		validator_unref(v);
		g_free(entry);
		return false;
	}
	o->patterns = g_slist_prepend(o->patterns, entry);
	return true;
}

Validator* object_pattern_properties_find(ObjectPatternProperties *o, const char *key)
{
	if (!o)
		return NULL;

	// Look through the list of patterns and see what regex match the tested key,
	// collect their validators.
	GSList *validators = NULL;
	for (GSList *s = o->patterns; s != NULL; s = g_slist_next(s))
	{
		Entry *entry = (Entry *) s->data;
		if (g_regex_match(entry->regex, key, 0, NULL))
			validators = g_slist_prepend(validators, entry->validator);
	}

	// If no match, fine.
	if (!validators)
		return NULL;

	// If there's a single match, return it's validator for continuation.
	if (!g_slist_next(validators))
	{
		Validator *ret = validator_ref(validators->data);
		g_slist_free_1(validators);
		return ret;
	}

	// If multiple patterns match, we'll have to continue with "anyOf" matched validators
	// accept the property value.
	CombinedValidator *ret = any_of_validator_new();
	for (GSList *s = validators; s != NULL; s = g_slist_next(s))
		combined_validator_add_value(ret, s->data);
	g_slist_free(validators);
	return &ret->base;
}

void object_pattern_properties_visit(ObjectPatternProperties *o,
                                     VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                                     void *ctxt)
{
	if (!o)
		return;

	for (GSList *s = o->patterns; s != NULL; s = g_slist_next(s))
	{
		Entry *entry = (Entry *) s->data;
		Validator *v = entry->validator;

		enter_func(NULL, v, ctxt);
		validator_visit(v, enter_func, exit_func, ctxt);
		Validator *new_v = NULL;
		exit_func(NULL, v, ctxt, &new_v);
		if (new_v)
		{
			validator_unref(v);
			entry->validator = new_v;
		}
	}
}
