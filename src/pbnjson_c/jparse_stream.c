// Copyright (c) 2009-2023 LG Electronics, Inc.
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

#include <jparse_stream.h>
#include <jobject.h>

#include <yajl/yajl_parse.h>
#include "yajl_compat.h"
#include "liblog.h"
#include "jobject_internal.h"
#include "jparse_stream_internal.h"
#include "jtraverse.h"
#include "key_dictionary.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <glib.h>

#include <inttypes.h>
#include <sys/mman.h>
#include "dom_string_memory_pool.h"
#include <assert.h>
#define DOM_POOL_SIZE 4

//Dummy PJSAXCallbacks for DOM parsing
static int dummy_dom_boolean(void *context, int value) { return 1; }
static int dummy_dom_string(void *context, const char *string, yajl_size_t len) { return 1; }
static int dummy_dom_context(void *context) { return 1; }

static yajl_callbacks no_callbacks =
{
	dummy_dom_context, // yajl_null
	dummy_dom_boolean, // yajl_boolean
	NULL, // yajl_integer
	NULL, // yajl_double
	dummy_dom_string, // yajl_number
	(pj_yajl_string)dummy_dom_string, // yajl_string
	dummy_dom_context, // yajl_start_map
	(pj_yajl_map_key)dummy_dom_string, // yajl_map_key
	dummy_dom_context, // yajl_end_map
	dummy_dom_context, // yajl_start_array
	dummy_dom_context, // yajl_end_array
};

static bool jsax_parse_internal(PJSAXCallbacks *parser, raw_buffer input, const jschema_ref schema, void **ctxt, jerror **err);
// TODO: deprecated
static bool jsax_parse_internal_old(PJSAXCallbacks *parser, raw_buffer input, JSchemaInfoRef schemaInfo, void **ctxt);

static inline jvalue_ref createOptimalString(dom_string_memory_pool* pool, JDOMOptimization opt, const char *str, size_t strLen)
{
	if (opt == DOMOPT_INPUT_OUTLIVES_WITH_NOCHANGE)
		return jstring_create_nocopy(j_str_to_buffer(str, strLen));
	if (pool)
		return jstring_create_from_pool_internal(pool, str, strLen);
	return jstring_create_copy(j_str_to_buffer(str, strLen));
}

static inline jvalue_ref createOptimalNumber(dom_string_memory_pool* pool, JDOMOptimization opt, const char *str, size_t strLen)
{
	if (opt == DOMOPT_INPUT_OUTLIVES_WITH_NOCHANGE)
		return jnumber_create_unsafe(j_str_to_buffer(str, strLen), NULL);
	if (pool)
		return jnumber_create_from_pool_internal(pool, str, strLen);
	return jnumber_create(j_str_to_buffer(str, strLen));
}

static inline DomInfo* getDOMInfo(JSAXContextRef ctxt)
{
	struct jdomcontext* dctxt = (struct jdomcontext*)jsax_getContext(ctxt);
	return dctxt->context;
}

static inline void changeDOMInfo(JSAXContextRef ctxt, DomInfo *domCtxt)
{
	struct jdomcontext* dctxt = (struct jdomcontext*)jsax_getContext(ctxt);
	dctxt->context = domCtxt;
}

static inline dom_string_memory_pool* getDOMPool(JSAXContextRef ctxt)
{
	return ((struct jdomcontext*)jsax_getContext(ctxt))->string_pool;
}

int dom_null(JSAXContextRef ctxt)
{
	DomInfo *data = getDOMInfo(ctxt);
	// no handle to the context
	CHECK_CONDITION_RETURN_PARSER_ERROR(data == NULL, 0,
	                                    &ctxt->m_error,
	                                    "null encountered without any context");

	SANITY_CHECK_POINTER(ctxt);
	SANITY_CHECK_POINTER(data->m_prev);

	if (data->m_value == NULL) {
		if (data->m_prev != NULL)
		{
			CHECK_CONDITION_RETURN_PARSER_ERROR(!jis_array(data->m_prev->m_value), 0,
			                                    &ctxt->m_error,
			                                    "Improper place for null");
			jarray_append(data->m_prev->m_value, jnull());
		}
		else data->m_value = jnull();
	} else if (jis_string(data->m_value)) {
		CHECK_CONDITION_RETURN_PARSER_ERROR(!jis_object(data->m_prev->m_value), 0,
		                                    &ctxt->m_error,
		                                    "Improper place for null");
		jobject_put(data->m_prev->m_value, data->m_value, jnull());
		data->m_value = NULL;
	} else {
		jerror_set(&ctxt->m_error, JERROR_TYPE_SYNTAX, "Improper place for null. Value portion of key-value pair but not a key");
		return 0;
	}

	return 1;
}

int dom_boolean(JSAXContextRef ctxt, bool value)
{
	DomInfo *data = getDOMInfo(ctxt);
	CHECK_CONDITION_RETURN_PARSER_ERROR(data == NULL, 0,
	                                    &ctxt->m_error,
	                                    "boolean encountered without any context");

	if (data->m_value == NULL) {
		if (data->m_prev != NULL)
		{
			CHECK_CONDITION_RETURN_PARSER_ERROR(!jis_array(data->m_prev->m_value), 0,
			                                    &ctxt->m_error,
			                                    "Improper place for boolean");
			jarray_append(data->m_prev->m_value, jboolean_create(value));
		}
		else data->m_value = jboolean_create(value);
	} else if (jis_string(data->m_value)) {
		CHECK_CONDITION_RETURN_PARSER_ERROR(!jis_object(data->m_prev->m_value), 0,
		                                    &ctxt->m_error,
		                                    "Improper place for boolean");
		jobject_put(data->m_prev->m_value, data->m_value, jboolean_create(value));
		data->m_value = NULL;
	} else {
		jerror_set(&ctxt->m_error, JERROR_TYPE_SYNTAX, "Improper place for boolean");
		return 0;
	}

	return 1;
}

int dom_number(JSAXContextRef ctxt, const char *number, size_t numberLen)
{
	DomInfo *data = getDOMInfo(ctxt);
	dom_string_memory_pool *pool = getDOMPool(ctxt);

	jvalue_ref jnum;

	CHECK_CONDITION_RETURN_PARSER_ERROR(data == NULL, 0,
	                                    &ctxt->m_error,
	                                    "number encountered without any context");
	CHECK_CONDITION_RETURN_PARSER_ERROR(number == 0, 0,
	                                    &ctxt->m_error,
	                                    "Null pointer number string");
	CHECK_CONDITION_RETURN_PARSER_ERROR(numberLen == 0, 0,
	                                    &ctxt->m_error,
	                                    "unexpected - numeric string doesn't actually contain a number");

	jnum = createOptimalNumber(pool, data->m_optInformation, number, numberLen);

	do {
		if (data->m_value == NULL) {
			if (data->m_prev != NULL)
			{
				if (UNLIKELY(!jis_array(data->m_prev->m_value))) break;
				jarray_append(data->m_prev->m_value, jnum);
			}
			else data->m_value = jnum;
		}
		else if (jis_string(data->m_value)) {
			if (UNLIKELY(!jis_object(data->m_prev->m_value))) break;
			jobject_put(data->m_prev->m_value, data->m_value, jnum);
			data->m_value = NULL;
		}
		else break;

		return 1;
	}
	while (false);

	jerror_set(&ctxt->m_error, JERROR_TYPE_SYNTAX, "Improper place for number");
	j_release(&jnum);
	return 0;
}

int dom_string(JSAXContextRef ctxt, const char *string, size_t stringLen)
{
	DomInfo *data = getDOMInfo(ctxt);
	dom_string_memory_pool *pool = getDOMPool(ctxt);

	CHECK_CONDITION_RETURN_PARSER_ERROR(data == NULL, 0,
	                                    &ctxt->m_error,
	                                    "string encountered without any context");

	jvalue_ref jstr = createOptimalString(pool, data->m_optInformation, string, stringLen);

	do {
		if (data->m_value == NULL) {
			if (data->m_prev != NULL)
			{
				if (UNLIKELY(!jis_array(data->m_prev->m_value))) break;
				jarray_append(data->m_prev->m_value, jstr);
			}
			else data->m_value = jstr;
		}
		else if (jis_string(data->m_value)) {
			if (UNLIKELY(!jis_object(data->m_prev->m_value))) break;
			jobject_put(data->m_prev->m_value, data->m_value, jstr);
			data->m_value = NULL;
		}
		else break;

		return 1;
	}
	while (false);

	jerror_set(&ctxt->m_error, JERROR_TYPE_SYNTAX, "Improper place for string");
	j_release(&jstr);
	return 0;
}

int dom_object_start(JSAXContextRef ctxt)
{
	DomInfo *data = getDOMInfo(ctxt);
	jvalue_ref newParent;
	DomInfo *newChild;

	CHECK_CONDITION_RETURN_PARSER_ERROR(data == NULL, 0,
	                                    &ctxt->m_error,
	                                    "object encountered without any context");

	newParent = jobject_create();
	newChild = calloc(1, sizeof(DomInfo));

	if (UNLIKELY(newChild == NULL || !jis_valid(newParent))) {
		jerror_set(&ctxt->m_error, JERROR_TYPE_SYNTAX, "Failed to allocate space for new object");
		j_release(&newParent);
		free(newChild);
		return 0;
	}
	newChild->m_prev = data;
	newChild->m_optInformation = data->m_optInformation;
	changeDOMInfo(ctxt, newChild);

	if (data->m_prev != NULL) {
		if (jis_array(data->m_prev->m_value)) {
			assert(data->m_value == NULL);
			jarray_append(data->m_prev->m_value, jvalue_copy(newParent));
		} else {
			assert(jis_object(data->m_prev->m_value));
			if (UNLIKELY(!jis_string(data->m_value)))
			{
				jerror_set(&ctxt->m_error, JERROR_TYPE_SYNTAX, "Improper place for a child object");
				j_release(&newParent);
				return 0;
			}
			jobject_put(data->m_prev->m_value, data->m_value, jvalue_copy(newParent));
		}
	}

	data->m_value = newParent;

	return 1;
}

int dom_object_key(JSAXContextRef ctxt, const char *key, size_t keyLen)
{
	DomInfo *data = getDOMInfo(ctxt);

	CHECK_CONDITION_RETURN_PARSER_ERROR(data == NULL, 0,
	                                    &ctxt->m_error,
	                                    "object key encountered without any context");
	CHECK_CONDITION_RETURN_PARSER_ERROR(data->m_value != NULL, 0,
	                                    &ctxt->m_error,
	                                    "Improper place for an object key");
	CHECK_CONDITION_RETURN_PARSER_ERROR(data->m_prev == NULL, 0,
	                                    &ctxt->m_error,
	                                    "object key encountered without any parent object");
	CHECK_CONDITION_RETURN_PARSER_ERROR(!jis_object(data->m_prev->m_value), 0,
	                                    &ctxt->m_error,
	                                    "object key encountered without any parent object");

	// We try to optimize memory utilization here for larger JSONs. Common
	// case is to have similar JSON objects throughout the system (consider
	// keys like returnValue, subscription etc. We will share the keys via
	// common hash table. If lookup fails, we create a new instance for the
	// key.
	data->m_value = keyDictionaryLookup(key, keyLen);

	return 1;
}

int dom_object_end(JSAXContextRef ctxt)
{
	DomInfo *data = getDOMInfo(ctxt);
	CHECK_CONDITION_RETURN_PARSER_ERROR(data == NULL, 0,
	                                    &ctxt->m_error,
	                                    "object end encountered without any context");
	CHECK_CONDITION_RETURN_PARSER_ERROR(data->m_value != NULL, 0,
	                                    &ctxt->m_error,
	                                    "mismatch between key/value count");
	CHECK_CONDITION_RETURN_PARSER_ERROR(!jis_object(data->m_prev->m_value), 0,
	                                    &ctxt->m_error,
	                                    "object end encountered, but not in an object");

	assert(data->m_prev != NULL);
	changeDOMInfo(ctxt, data->m_prev);
	if (data->m_prev->m_prev != NULL)
	{
		j_release(&data->m_prev->m_value);
		// 0xdeadbeef may be written in debug mode, which fools the code
		data->m_prev->m_value = NULL;
	}
	free(data);

	return 1;
}

int dom_array_start(JSAXContextRef ctxt)
{
	DomInfo *data = getDOMInfo(ctxt);
	jvalue_ref newParent;
	DomInfo *newChild;
	CHECK_CONDITION_RETURN_PARSER_ERROR(data == NULL, 0,
	                                    &ctxt->m_error,
	                                    "object encountered without any context");

	newParent = jarray_create(NULL);
	newChild = calloc(1, sizeof(DomInfo));
	if (UNLIKELY(newChild == NULL || !jis_valid(newParent))) {
		jerror_set(&ctxt->m_error, JERROR_TYPE_SYNTAX, "Failed to allocate space for new array node");
		j_release(&newParent);
		free(newChild);
		return 0;
	}
	newChild->m_prev = data;
	newChild->m_optInformation = data->m_optInformation;
	changeDOMInfo(ctxt, newChild);

	if (data->m_prev != NULL) {
		if (jis_array(data->m_prev->m_value)) {
			assert(data->m_value == NULL);
			jarray_append(data->m_prev->m_value, jvalue_copy(newParent));
		} else {
			assert(jis_object(data->m_prev->m_value));
			if (UNLIKELY(!jis_string(data->m_value))) {
				jerror_set(&ctxt->m_error, JERROR_TYPE_SYNTAX, "improper place for a child object");
				j_release(&newParent);
				return 0;
			}
			jobject_put(data->m_prev->m_value, data->m_value, jvalue_copy(newParent));
		}
	}

	data->m_value = newParent;

	return 1;
}

int dom_array_end(JSAXContextRef ctxt)
{
	DomInfo *data = getDOMInfo(ctxt);
	CHECK_CONDITION_RETURN_PARSER_ERROR(data == NULL, 0,
	                                    &ctxt->m_error,
	                                    "array end encountered without any context");
	CHECK_CONDITION_RETURN_PARSER_ERROR(data->m_value != NULL, 0,
	                                    &ctxt->m_error,
	                                    "key/value for array");
	CHECK_CONDITION_RETURN_PARSER_ERROR(!jis_array(data->m_prev->m_value), 0,
	                                    &ctxt->m_error,
	                                    "array end encountered, but not in an array");

	assert(data->m_prev != NULL);
	changeDOMInfo(ctxt, data->m_prev);
	if (data->m_prev->m_prev != NULL)
	{
		j_release(&data->m_prev->m_value);
		data->m_prev->m_value = NULL;
	}
	free(data);

	return 1;
}

// Do not release original_ptr. It could be on a stack
static void dom_cleanup(DomInfo *dom_info, DomInfo *original_ptr)
{
	while (dom_info && dom_info != original_ptr)
	{
		DomInfo *cur_dom_info = dom_info;
		dom_info = dom_info->m_prev;

		j_release(&cur_dom_info->m_value);
		free(cur_dom_info);
	}
}

jvalue_ref jdom_create(raw_buffer input, const jschema_ref schema, jerror **err)
{
	jvalue_ref jval = jinvalid();
	struct jdomparser parser;

	jdomparser_init(&parser, schema);
	parser.context.string_pool = dom_string_memory_pool_create();

	if (jdomparser_feed(&parser, input.m_str, input.m_len) && jdomparser_end(&parser)) {
		jval = jdomparser_get_result(&parser);
	}
	else if (err && !(*err)) {
		*err = parser.saxparser.internalCtxt.m_error;
		parser.saxparser.internalCtxt.m_error = NULL;
	}

	jdomparser_deinit(&parser);
	dom_string_memory_pool_destroy(parser.context.string_pool);

	return jval;
}

jvalue_ref jdom_parse(raw_buffer input, JDOMOptimizationFlags optimizationMode, JSchemaInfoRef schemaInfo)
{
	// create parser
	struct jdomparser parser;
	if (!jdomparser_init_old(&parser, schemaInfo, optimizationMode)) {
		return jinvalid();
	}

	if (!jdomparser_feed(&parser, input.m_str, input.m_len) || !jdomparser_end((&parser))) {
		jdomparser_deinit(&parser);
		return jinvalid();
	}

	jvalue_ref jval = jdomparser_get_result(&parser);

	jdomparser_deinit(&parser);

	return jval;
}

jvalue_ref jdom_fcreate(const char *file, const jschema_ref schema, jerror **err)
{
	CHECK_POINTER_RETURN_VALUE(schema, jinvalid());

	_jbuffer buf = {
		.buffer = { 0 },
		.destructor = NULL
	};
	jvalue_ref result = jinvalid();

	if (!j_fopen(file, &buf, err))
		return result;

	result = jdom_create(buf.buffer, schema, err);

	if (UNLIKELY(!jis_valid(result))) {
		buf.destructor(&buf);
	} else {
		result->m_file = buf;
	}

	return result;
}

jvalue_ref jdom_parse_file(const char *file, JSchemaInfoRef schemaInfo, JFileOptimizationFlags flags)
{
	CHECK_POINTER_RETURN_NULL(file);
	CHECK_POINTER_RETURN_NULL(schemaInfo);

	_jbuffer buf = {
		.buffer = { 0 },
		.destructor = NULL
	};
	jvalue_ref result = jinvalid();

	if (!j_fopen(file, &buf, NULL))
		return result;

	result = jdom_parse(buf.buffer, DOMOPT_INPUT_OUTLIVES_WITH_NOCHANGE, schemaInfo);

	if (UNLIKELY(!jis_valid(result))) {
		buf.destructor(&buf);
	} else {
		result->m_file = buf;
	}

	return result;
}

void jsax_changeContext(JSAXContextRef saxCtxt, void *userCtxt)
{
	saxCtxt->ctxt = userCtxt;
}

void* jsax_getContext(JSAXContextRef saxCtxt)
{
	return saxCtxt->ctxt;
}

int my_bounce_start_map(void *ctxt)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;
	assert(spring->m_handlers->yajl_start_map);

	ValidationEvent e = validation_event_obj_start();
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	return spring->m_handlers->yajl_start_map(ctxt);
}

int my_bounce_map_key(void *ctxt, const unsigned char *str, yajl_size_t strLen)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;
	assert(spring->m_handlers->yajl_map_key);

	ValidationEvent e = validation_event_obj_key((char const *) str, strLen);
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	return spring->m_handlers->yajl_map_key(ctxt, str, strLen);
}

int my_bounce_end_map(void *ctxt)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;
	assert(spring->m_handlers->yajl_end_map);

	ValidationEvent e = validation_event_obj_end();
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	return spring->m_handlers->yajl_end_map(ctxt);
}

int my_bounce_start_array(void *ctxt)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;
	assert(spring->m_handlers->yajl_start_array);

	ValidationEvent e = validation_event_arr_start();
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	return spring->m_handlers->yajl_start_array(ctxt);
}

int my_bounce_end_array(void *ctxt)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;
	assert(spring->m_handlers->yajl_end_array);

	ValidationEvent e = validation_event_arr_end();
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	return spring->m_handlers->yajl_end_array(ctxt);
}

int my_bounce_string(void *ctxt, const unsigned char *str, yajl_size_t strLen)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;
	assert(spring->m_handlers->yajl_string);

	ValidationEvent e = validation_event_string((char const *) str, strLen);
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	return spring->m_handlers->yajl_string(ctxt, str, strLen);
}

int my_bounce_number(void *ctxt, const char *numberVal, yajl_size_t numberLen)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;
	assert(spring->m_handlers->yajl_number);

	ValidationEvent e = validation_event_number(numberVal, numberLen);
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	return spring->m_handlers->yajl_number(ctxt, numberVal, numberLen);
}

int my_bounce_boolean(void *ctxt, int boolVal)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;
	assert(spring->m_handlers->yajl_boolean);

	ValidationEvent e = validation_event_boolean(boolVal);
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	return spring->m_handlers->yajl_boolean(ctxt, boolVal);
}

int my_bounce_null(void *ctxt)
{
	JSAXContextRef spring = (JSAXContextRef)ctxt;
	assert(spring->m_handlers->yajl_null);

	ValidationEvent e = validation_event_null();
	if (!validation_check(&e, spring->validation_state, ctxt))
		return false;

	return spring->m_handlers->yajl_null(ctxt);
}

static yajl_callbacks my_bounce =
{
	my_bounce_null,
	my_bounce_boolean,
	NULL, // yajl_integer,
	NULL, // yajl_double
	my_bounce_number,
	my_bounce_string,
	my_bounce_start_map,
	my_bounce_map_key,
	my_bounce_end_map,
	my_bounce_start_array,
	my_bounce_end_array,
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Default property injection

static bool inject_default_jnull(void *ctxt, jvalue_ref ref)
{
	JSAXContextRef context = (JSAXContextRef)ctxt;
	return context->m_handlers->yajl_null(context);
}

//Helper function for jobject_to_string_append()
static bool inject_default_jkeyvalue(void *ctxt, jvalue_ref ref)
{
	JSAXContextRef context = (JSAXContextRef)ctxt;
	raw_buffer raw = jstring_deref(ref)->m_data;
	return context->m_handlers->yajl_map_key(context, (unsigned char*)raw.m_str, raw.m_len);
}

static bool inject_default_jobject_start(void *ctxt, jvalue_ref ref)
{
	JSAXContextRef context = (JSAXContextRef)ctxt;
	return context->m_handlers->yajl_start_map(context);
}

static bool inject_default_jobject_end(void *ctxt, jvalue_ref ref)
{
	JSAXContextRef context = (JSAXContextRef)ctxt;
	return context->m_handlers->yajl_end_map(context);
}

static bool inject_default_jarray_start(void *ctxt, jvalue_ref ref)
{
	JSAXContextRef context = (JSAXContextRef)ctxt;
	return context->m_handlers->yajl_start_array(context);
}

static bool inject_default_jarray_end(void *ctxt, jvalue_ref ref)
{
	JSAXContextRef context = (JSAXContextRef)ctxt;
	return context->m_handlers->yajl_end_array(context);
}

static bool inject_default_jnumber_raw(void *ctxt, jvalue_ref ref)
{
	JSAXContextRef context = (JSAXContextRef)ctxt;
	raw_buffer raw = jnum_deref(ref)->value.raw;
	return context->m_handlers->yajl_number(context, raw.m_str, raw.m_len);
}

static bool inject_default_jnumber_double(void *ctxt, jvalue_ref ref)
{
	char buf[24];
	int len = snprintf(buf, sizeof(buf), "%.14lg", jnum_deref(ref)->value.floating);
	JSAXContextRef context = (JSAXContextRef)ctxt;
	return context->m_handlers->yajl_number(context, buf, len);
}

static bool inject_default_jnumber_int(void *ctxt, jvalue_ref ref)
{
	char buf[24];
	int len = snprintf(buf, sizeof(buf), "%" PRId64, jnum_deref(ref)->value.integer);
	JSAXContextRef context = (JSAXContextRef)ctxt;
	return context->m_handlers->yajl_number(context, buf, len);
}

static bool inject_default_jstring(void *ctxt, jvalue_ref ref)
{
	JSAXContextRef context = (JSAXContextRef)ctxt;
	raw_buffer raw = jstring_deref(ref)->m_data;
	return context->m_handlers->yajl_string(context, (unsigned char*)raw.m_str, raw.m_len);
}

static bool inject_default_jbool(void *ctxt, jvalue_ref ref)
{
	JSAXContextRef context = (JSAXContextRef)ctxt;
	return context->m_handlers->yajl_boolean(context, jboolean_deref(ref)->value);
}

static struct TraverseCallbacks traverse = {
	inject_default_jnull,
	inject_default_jbool,
	inject_default_jnumber_int,
	inject_default_jnumber_double,
	inject_default_jnumber_raw,
	inject_default_jstring,
	inject_default_jobject_start,
	inject_default_jkeyvalue,
	inject_default_jobject_end,
	inject_default_jarray_start,
	inject_default_jarray_end,
};

static bool on_default_property(ValidationState *s, char const *key, jvalue_ref value, void *ctxt)
{
	JSAXContextRef spring = (JSAXContextRef) ctxt;

	if (!spring->m_handlers->yajl_map_key(ctxt, (unsigned char const *) key, strlen(key)))
		return false;

	return jvalue_traverse(value, &traverse, spring);
}

static bool has_array_duplicates(ValidationState *s, void *ctxt)
{
	assert(ctxt);
	DomInfo *data = getDOMInfo((JSAXContextRef) ctxt);
	assert(data && data->m_prev && data->m_prev->m_value && jis_array(data->m_prev->m_value));

	return jarray_has_duplicates(data->m_prev->m_value);
}

static void validation_error(ValidationState *s, ValidationErrorCode error, void *ctxt)
{
	assert(ctxt);
	JSAXContextRef spring = (JSAXContextRef) ctxt;
	if (spring->m_errors && spring->m_errors->m_schema)
	{
		spring->m_error_code = error;
		spring->m_errors->m_schema(spring->m_errors->m_ctxt, spring);
	}
}

static Notification jparse_notification =
{
	.default_property_func = &on_default_property,
	.has_array_duplicates = &has_array_duplicates,
	.error_func = &validation_error,
};

static bool handle_yajl_error(yajl_status parseResult,
                              yajl_handle handle,
                              const char *buf, int buf_len,
                              JSchemaInfoRef schemaInfo,
                              PJSAXContext *internalCtxt)
{
	switch (parseResult)
	{
	case yajl_status_ok:
		return true;
	case yajl_status_client_canceled:
		if (!schemaInfo || !schemaInfo->m_errHandler ||
		    !schemaInfo->m_errHandler->m_unknown(schemaInfo->m_errHandler->m_ctxt, internalCtxt))
		{
			return false;
		}
		return true;
#if YAJL_VERSION < 20000
	case yajl_status_insufficient_data:
		if (!schemaInfo || !schemaInfo->m_errHandler ||
		    !schemaInfo->m_errHandler->m_parser(schemaInfo->m_errHandler->m_ctxt, internalCtxt))
		{
			return false;
		}
		return true;
#endif
	case yajl_status_error:
	default:
		internalCtxt->errorDescription = (char*)yajl_get_error(handle, 1, (unsigned char *)buf, buf_len);
		if (!schemaInfo || !schemaInfo->m_errHandler ||
		    !schemaInfo->m_errHandler->m_unknown(schemaInfo->m_errHandler->m_ctxt, internalCtxt))
		{
			yajl_free_error(handle, (unsigned char*)internalCtxt->errorDescription);
			return false;
		}
		yajl_free_error(handle, (unsigned char*)internalCtxt->errorDescription);
		return true;
	}
}

static bool jsax_parse_internal(PJSAXCallbacks *callbacks,
                                raw_buffer input,
                                const jschema_ref schema,
                                void **callback_ctxt,
                                jerror **err)
{
	struct jsaxparser parser;
	jsaxparser_init(&parser, schema, callbacks, callback_ctxt);

	if (!jsaxparser_feed(&parser, input.m_str, input.m_len) || !jsaxparser_end(&parser)) {
		if (err && !(*err))
		{
			*err = parser.internalCtxt.m_error;
			parser.internalCtxt.m_error = NULL;
		}
		jsaxparser_deinit(&parser);

		return false;
	}

	jsaxparser_deinit(&parser);
	return true;
}

// TODO: deprecated
static bool jsax_parse_internal_old(PJSAXCallbacks *callbacks,
                                    raw_buffer input,
                                    JSchemaInfoRef schemaInfo,
                                    void **callback_ctxt)
{
	struct jsaxparser parser;
	if (!jsaxparser_init_old(&parser, schemaInfo, callbacks, callback_ctxt))
		return false;

	if (!jsaxparser_feed(&parser, input.m_str, input.m_len) || !jsaxparser_end(&parser)) {
		jsaxparser_deinit(&parser);
		return false;
	}

	jsaxparser_deinit(&parser);

	return true;
}

bool jsax_parse_with_callbacks(raw_buffer input, const jschema_ref schema,
                               PJSAXCallbacks *callbacks, void *callback_ctxt,
                               jerror **err)
{
	return jsax_parse_internal(callbacks, input, schema, callback_ctxt, err);
}

bool jsax_parse_ex(PJSAXCallbacks *parser, raw_buffer input, JSchemaInfoRef schemaInfo, void **ctxt)
{
	return jsax_parse_internal_old(parser, input, schemaInfo, ctxt);
}

bool jsax_parse(PJSAXCallbacks *parser, raw_buffer input, JSchemaInfoRef schema)
{
	return jsax_parse_ex(parser, input, schema, NULL);
}

static bool jerr_parser(void *ctxt, JSAXContextRef parseCtxt)
{
	jsaxparser_ref parser = (jsaxparser_ref)ctxt;
	jerror_set_formatted(&parser->internalCtxt.m_error, JERROR_TYPE_SYNTAX, "Parser error %d: %s",
	                     parseCtxt->m_error_code, parseCtxt->errorDescription);
	return false;
}

static bool jerr_schema(void *ctxt, JSAXContextRef parseCtxt)
{
	jsaxparser_ref parser = (jsaxparser_ref)ctxt;

	const char *errorDescription = ValidationGetErrorMessage(parseCtxt->m_error_code);
	if (errorDescription) {
		jerror_set_formatted(&parser->internalCtxt.m_error, JERROR_TYPE_SCHEMA, "%d: %s",
		                     parseCtxt->m_error_code, errorDescription);
	}

	return false;
}


static bool jerr_unknown(void *ctxt, JSAXContextRef parseCtxt)
{
	jsaxparser_ref parser = (jsaxparser_ref)ctxt;
	jerror_set_formatted(&parser->internalCtxt.m_error, JERROR_TYPE_SYNTAX, "Parser error %d: %s",
	                     parseCtxt->m_error_code, parseCtxt->errorDescription);

	return false;
}

// TODO: deprecated
static bool err_parser(void *ctxt, JSAXContextRef parseCtxt)
{
	struct jsaxparser *parser = (struct jsaxparser*)ctxt;
	if (parser && parser->schemaInfo && parser->schemaInfo->m_errHandler && parser->schemaInfo->m_errHandler->m_parser)
		parser->schemaInfo->m_errHandler->m_parser(parser->schemaInfo->m_errHandler->m_ctxt, parseCtxt);
	return false;
}

// TODO: deprecated
static bool err_schema(void *ctxt, JSAXContextRef parseCtxt)
{
	struct jsaxparser *parser = (struct jsaxparser *)ctxt;
	if (parser && parser->schemaInfo && parser->schemaInfo->m_errHandler && parser->schemaInfo->m_errHandler->m_schema)
		parser->schemaInfo->m_errHandler->m_schema(parser->schemaInfo->m_errHandler->m_ctxt, parseCtxt);

	if (parser->schemaError) {
		g_free(parser->schemaError);
		parser->schemaError = NULL;
	}

	const char *errorDescription = ValidationGetErrorMessage(parseCtxt->m_error_code);
	if (errorDescription) {
		parser->schemaError = g_strdup_printf("Schema error: %s", errorDescription);
	}

	return false;
}

// TODO: deprecated
static bool err_unknown(void *ctxt, JSAXContextRef parseCtxt)
{
	struct jsaxparser *parser = (struct jsaxparser *)ctxt;
	if (parser && parser->schemaInfo && parser->schemaInfo->m_errHandler && parser->schemaInfo->m_errHandler->m_unknown)
		parser->schemaInfo->m_errHandler->m_unknown(parser->schemaInfo->m_errHandler->m_ctxt, parseCtxt);

	return false;
}

jsaxparser_ref jsaxparser_alloc_memory()
{
	return malloc(sizeof(struct jsaxparser));
}

void jsaxparser_free_memory(jsaxparser_ref parser)
{
	free(parser);
}

jsaxparser_ref jsaxparser_new(const jschema_ref schema, PJSAXCallbacks *callbacks, void *callback_ctxt)
{
	jsaxparser_ref parser = jsaxparser_alloc_memory();
	if (parser) {
		jsaxparser_init(parser, schema, callbacks, callback_ctxt);
	}

	return parser;
}

jsaxparser_ref jsaxparser_create(JSchemaInfoRef schemaInfo, PJSAXCallbacks *callback, void *callback_ctxt)
{
	jsaxparser_ref parser = jsaxparser_alloc_memory();
	if (parser) {
		if (!jsaxparser_init_old(parser, schemaInfo, callback, callback_ctxt)) {
			jsaxparser_free_memory(parser);
			parser = NULL;
		}
	}

	return parser;
}

void jsaxparser_release(jsaxparser_ref *parser)
{
	jsaxparser_deinit(*parser);
	jsaxparser_free_memory(*parser);
}

void jsaxparser_init(jsaxparser_ref parser, const jschema_ref schema, PJSAXCallbacks *callback, void *callback_ctxt)
{
	memset(parser, 0, sizeof(struct jsaxparser) - sizeof(mem_pool_t));

	parser->validator = NOTHING_VALIDATOR;
	parser->uri_resolver = NULL;
	parser->schemaInfo = NULL;
	if (schema)
	{
		parser->validator = schema->validator;
		parser->uri_resolver = schema->uri_resolver;
	}

	if (callback == NULL) {
		parser->yajl_cb = no_callbacks;
	} else {
		parser->yajl_cb.yajl_null = callback->m_null ? (pj_yajl_null)callback->m_null : no_callbacks.yajl_null;
		parser->yajl_cb.yajl_boolean = callback->m_boolean ? (pj_yajl_boolean)callback->m_boolean : no_callbacks.yajl_boolean;
		parser->yajl_cb.yajl_integer = NULL;
		parser->yajl_cb.yajl_double = NULL;
		parser->yajl_cb.yajl_number = callback->m_number ? (pj_yajl_number)callback->m_number : no_callbacks.yajl_number;
		parser->yajl_cb.yajl_string = callback->m_string ? (pj_yajl_string)callback->m_string : no_callbacks.yajl_string;
		parser->yajl_cb.yajl_start_map = callback->m_objStart ? (pj_yajl_start_map)callback->m_objStart : no_callbacks.yajl_start_map;
		parser->yajl_cb.yajl_map_key = callback->m_objKey ? (pj_yajl_map_key)callback->m_objKey : no_callbacks.yajl_map_key;
		parser->yajl_cb.yajl_end_map = callback->m_objEnd ? (pj_yajl_end_map)callback->m_objEnd : no_callbacks.yajl_end_map;
		parser->yajl_cb.yajl_start_array = callback->m_arrStart ? (pj_yajl_start_array)callback->m_arrStart : no_callbacks.yajl_start_array;
		parser->yajl_cb.yajl_end_array = callback->m_arrEnd ? (pj_yajl_end_array)callback->m_arrEnd : no_callbacks.yajl_end_array;
	}

	parser->errorHandler.m_parser = jerr_parser;
	parser->errorHandler.m_schema = jerr_schema;
	parser->errorHandler.m_unknown = jerr_unknown;
	parser->errorHandler.m_ctxt = parser;

	PJSAXContext __internalCtxt =
	{
		.ctxt = callback_ctxt,
		.m_handlers = &parser->yajl_cb,
		.m_errors = &parser->errorHandler,
		.m_error_code = 0,
		.errorDescription = NULL,
		.validation_state = &parser->validation_state,
		.m_error = NULL
	};
	parser->internalCtxt = __internalCtxt;

	validation_state_init(&(parser->validation_state),
	                        parser->validator,
	                        parser->uri_resolver,
	                        &jparse_notification);

	mempool_init(&parser->memory_pool);
	yajl_alloc_funcs allocFuncs = {
		mempool_malloc,
		mempool_realloc,
		mempool_free,
		&parser->memory_pool
	};
	const bool allow_comments = true;

#if YAJL_VERSION < 20000
	yajl_parser_config yajl_opts =
	{
		allow_comments,
		0, // currently only UTF-8 will be supported for input.
	};

	parser->handle = yajl_alloc(&my_bounce, &yajl_opts, &allocFuncs, &parser->internalCtxt);
#else
	parser->handle = yajl_alloc(&my_bounce, &allocFuncs, &parser->internalCtxt);
	yajl_config(parser->handle, yajl_allow_comments, allow_comments ? 1 : 0);

	// currently only UTF-8 will be supported for input.
	yajl_config(parser->handle, yajl_dont_validate_strings, 1);
#endif // YAJL_VERSION
}

// TODO: Deprecated. Use jsaxparser_init instead
bool jsaxparser_init_old(jsaxparser_ref parser, JSchemaInfoRef schemaInfo, PJSAXCallbacks *callback, void *callback_ctxt)
{
	memset(parser, 0, sizeof(struct jsaxparser) - sizeof(mem_pool_t));

	parser->validator = NOTHING_VALIDATOR;
	parser->uri_resolver = NULL;
	parser->schemaInfo = schemaInfo;
	if (schemaInfo && schemaInfo->m_schema)
	{
		parser->validator = schemaInfo->m_schema->validator;
		parser->uri_resolver = schemaInfo->m_schema->uri_resolver;
	}

	if (callback == NULL) {
		parser->yajl_cb = no_callbacks;
	} else {
		parser->yajl_cb.yajl_null = callback->m_null ? (pj_yajl_null)callback->m_null : no_callbacks.yajl_null;
		parser->yajl_cb.yajl_boolean = callback->m_boolean ? (pj_yajl_boolean)callback->m_boolean : no_callbacks.yajl_boolean;
		parser->yajl_cb.yajl_integer = NULL;
		parser->yajl_cb.yajl_double = NULL;
		parser->yajl_cb.yajl_number = callback->m_number ? (pj_yajl_number)callback->m_number : no_callbacks.yajl_number;
		parser->yajl_cb.yajl_string = callback->m_string ? (pj_yajl_string)callback->m_string : no_callbacks.yajl_string;
		parser->yajl_cb.yajl_start_map = callback->m_objStart ? (pj_yajl_start_map)callback->m_objStart : no_callbacks.yajl_start_map;
		parser->yajl_cb.yajl_map_key = callback->m_objKey ? (pj_yajl_map_key)callback->m_objKey : no_callbacks.yajl_map_key;
		parser->yajl_cb.yajl_end_map = callback->m_objEnd ? (pj_yajl_end_map)callback->m_objEnd : no_callbacks.yajl_end_map;
		parser->yajl_cb.yajl_start_array = callback->m_arrStart ? (pj_yajl_start_array)callback->m_arrStart : no_callbacks.yajl_start_array;
		parser->yajl_cb.yajl_end_array = callback->m_arrEnd ? (pj_yajl_end_array)callback->m_arrEnd : no_callbacks.yajl_end_array;
	}

	parser->errorHandler.m_parser = err_parser;
	parser->errorHandler.m_schema = err_schema;
	parser->errorHandler.m_unknown = err_unknown;
	parser->errorHandler.m_ctxt = parser;

	validation_state_init(&(parser->validation_state),
	                        parser->validator,
	                        parser->uri_resolver,
	                        &jparse_notification);

	PJSAXContext __internalCtxt =
	{
		.ctxt = (callback_ctxt != NULL ? callback_ctxt : NULL),
		.m_handlers = &parser->yajl_cb,
		.m_errors = &parser->errorHandler,
		.m_error_code = 0,
		.errorDescription = NULL,
		.validation_state = &parser->validation_state,
		.m_error = NULL
	};
	parser->internalCtxt = __internalCtxt;

	mempool_init(&parser->memory_pool);
	yajl_alloc_funcs allocFuncs = {
		mempool_malloc,
		mempool_realloc,
		mempool_free,
		&parser->memory_pool
	};
	const bool allow_comments = true;

#if YAJL_VERSION < 20000
	yajl_parser_config yajl_opts =
	{
		allow_comments,
		0, // currently only UTF-8 will be supported for input.
	};

	parser->handle = yajl_alloc(&my_bounce, &yajl_opts, &allocFuncs, &parser->internalCtxt);
#else
	parser->handle = yajl_alloc(&my_bounce, &allocFuncs, &parser->internalCtxt);
	yajl_config(parser->handle, yajl_allow_comments, allow_comments ? 1 : 0);

	// currently only UTF-8 will be supported for input.
	yajl_config(parser->handle, yajl_dont_validate_strings, 1);
#endif // YAJL_VERSION

	return true;
}

static bool jsaxparser_process_error(jsaxparser_ref parser, const char *buf, int buf_len, bool final_stage)
{
	if (
#if YAJL_VERSION < 20000
		(final_stage || yajl_status_insufficient_data != parser->status) &&
#endif
		!handle_yajl_error(parser->status, parser->handle, buf, buf_len, parser->schemaInfo, &parser->internalCtxt) )
	{
		if (parser->yajlError) {
			yajl_free_error(parser->handle, (unsigned char*)parser->yajlError);
			parser->yajlError = NULL;
		}
		parser->yajlError = (char*)yajl_get_error(parser->handle, 1, (unsigned char*)buf, buf_len);
		jerror_set(&parser->internalCtxt.m_error, JERROR_TYPE_SYNTAX, parser->yajlError);
		return false;
	}

	return true;
}

const char *jsaxparser_get_error(jsaxparser_ref parser)
{
	SANITY_CHECK_POINTER(parser);

	if (parser->schemaError)
		return parser->schemaError;

	if (parser->yajlError)
		return parser->yajlError;

	if (parser->internalCtxt.m_error)
		return parser->internalCtxt.m_error->message;

	return NULL;
}

bool jsaxparser_feed(jsaxparser_ref parser, const char *buf, int buf_len)
{
	parser->status = yajl_parse(parser->handle, (unsigned char *)buf, buf_len);

	return jsaxparser_process_error(parser, buf, buf_len, false);
}

bool jsaxparser_end(jsaxparser_ref parser)
{
#if YAJL_VERSION < 20000
	parser->status = yajl_parse_complete(parser->handle);
#else
	parser->status = yajl_complete_parse(parser->handle);
#endif

	return jsaxparser_process_error(parser, "", 0, true);
}

void jsaxparser_deinit(jsaxparser_ref parser)
{
	if (parser->yajlError) {
		yajl_free_error(parser->handle, (unsigned char*)parser->yajlError);
		parser->yajlError = NULL;
	}

	if (parser->schemaError) {
		g_free(parser->schemaError);
		parser->schemaError = NULL;
	}

	validation_state_clear(&parser->validation_state);

	if (parser->handle) {
		yajl_free(parser->handle);
		parser->handle = NULL;
	}

	jerror_free(parser->internalCtxt.m_error);
}

/**
 * DomParser pool type for YAJL parser
 */
typedef struct domparser_pool_t {
	struct jdomparser stack[DOM_POOL_SIZE];
	bool used[DOM_POOL_SIZE];
	pthread_mutex_t lock;
} domparser_pool;

static domparser_pool dompool = {.lock = PTHREAD_MUTEX_INITIALIZER};

jdomparser_ref jdomparser_alloc_memory()
{
	jdomparser_ref res;
	int i = 0;

	pthread_mutex_lock(&dompool.lock);
	for(; i < DOM_POOL_SIZE; i++) {
		if (!dompool.used[i])
			break;
	}
	if (i < DOM_POOL_SIZE) {
		res = &dompool.stack[i];
		dompool.used[i] = 1;
	}
	pthread_mutex_unlock(&dompool.lock);

	if (i == DOM_POOL_SIZE)
		res = malloc(sizeof(struct jdomparser));
	assert(res);

	return res;
}

void jdomparser_free_memory(jdomparser_ref parser)
{
	if (parser < &dompool.stack[0] || (char*)parser >= (char*)&dompool.stack + sizeof(dompool)) {
		free(parser);
	} else {
		int i = parser - &dompool.stack[0];
		pthread_mutex_lock(&dompool.lock);
		dompool.used[i] = 0;
		pthread_mutex_unlock(&dompool.lock);
	}
}

jdomparser_ref jdomparser_new(const jschema_ref schema)
{
	jdomparser_ref parser = jdomparser_alloc_memory();
	if (parser) {
		jdomparser_init(parser, schema);
	}

	return parser;
}

jdomparser_ref jdomparser_create(JSchemaInfoRef schemaInfo, JDOMOptimizationFlags optimizationMode)
{
	jdomparser_ref parser = jdomparser_alloc_memory();
	if (parser) {
		if (!jdomparser_init_old(parser, schemaInfo, optimizationMode)) {
			jdomparser_free_memory(parser);
			parser = NULL;
		}
	}

	return parser;
}

void jdomparser_release(jdomparser_ref *parser)
{
	jdomparser_deinit(*parser);
	jdomparser_free_memory(*parser);
}

static PJSAXCallbacks dom_callbacks = {
	dom_object_start,
	dom_object_key,
	dom_object_end,
	dom_array_start,
	dom_array_end,
	dom_string,
	dom_number,
	dom_boolean,
	dom_null
};

void jdomparser_init(jdomparser_ref parser, const jschema_ref schema)
{
	memset(&parser->topLevelContext, 0, sizeof(parser->topLevelContext));
	memset(&parser->context, 0, sizeof(parser->context));

	parser->context.context = &parser->topLevelContext;

	jsaxparser_init(&parser->saxparser, schema, &dom_callbacks, &parser->context);
}

bool jdomparser_init_old(jdomparser_ref parser, JSchemaInfoRef schemaInfo, JDOMOptimizationFlags optimizationMode)
{
	memset(&parser->topLevelContext, 0, sizeof(parser->topLevelContext));
	memset(&parser->context, 0, sizeof(parser->context));

	parser->context.context = &parser->topLevelContext;

	return jsaxparser_init_old(&parser->saxparser, schemaInfo, &dom_callbacks, &parser->context);
}

bool jdomparser_feed(jdomparser_ref parser, const char *buf, int buf_len)
{
	return jsaxparser_feed(&parser->saxparser, buf, buf_len);
}

bool jdomparser_end(jdomparser_ref parser)
{
	return jsaxparser_end(&parser->saxparser);
}

void jdomparser_deinit(jdomparser_ref parser)
{
	DomInfo *context = getDOMInfo(&parser->saxparser.internalCtxt);
	if (context != &parser->topLevelContext) {
		dom_cleanup(context, &parser->topLevelContext);
	}

	j_release(&parser->topLevelContext.m_value);

	jsaxparser_deinit(&parser->saxparser);
}

const char *jdomparser_get_error(jdomparser_ref parser)
{
	return jsaxparser_get_error(&parser->saxparser);
}

jvalue_ref jdomparser_get_result(jdomparser_ref parser)
{
	return jvalue_copy(parser->topLevelContext.m_value);
}
