// Copyright (c) 2009-2024 LG Electronics, Inc.
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

#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <compiler/inline_attribute.h>
#include <compiler/nonnull_attribute.h>
#include <compiler/builtins.h>
#include <math.h>
#include <inttypes.h>

#include <jobject.h>

#include <sys_malloc.h>
#include <sys/mman.h>
#include "jobject_internal.h"
#include "liblog.h"
#include "jvalue/num_conversion.h"
#include "jerror_internal.h"
#include "jschema_types_internal.h"
#include "jparse_stream_internal.h"
#include "jtraverse.h"
#include "validation/validation_state.h"
#include "validation/validation_event.h"
#include "validation/validation_api.h"
#include "validation/nothing_validator.h"

typedef struct {
	JErrorCallbacksRef callbacks;
	jvalue_ref jvalue;
	ValidationState *validation_state;
} ValidationContext;

static bool check_schema_jnull(void *ctxt, jvalue_ref ref)
{
	ValidationContext *context = (ValidationContext*)ctxt;
	if (!jis_valid(ref))
	{
		struct __JSAXContext fake_sax_ctxt = {
			.m_errors = context->callbacks,
			.m_error_code = VEC_UNEXPECTED_VALUE,
			.errorDescription = "jinvalid() cannot be validated against any schema"
		};

		context->callbacks->m_parser(context->callbacks->m_ctxt, &fake_sax_ctxt);
		return false;
	}
	ValidationEvent e = validation_event_null();
	return validation_check(&e, context->validation_state, context);
}

//Helper function for jobject_to_string_append()
static bool check_schema_jkeyvalue(void *ctxt, jvalue_ref ref)
{
	ValidationContext *context = (ValidationContext*)ctxt;
	raw_buffer raw = jstring_deref(ref)->m_data;
	ValidationEvent e = validation_event_obj_key(raw.m_str, raw.m_len);
	return validation_check(&e, context->validation_state, context);
}

static bool check_schema_jobject_start(void *ctxt, jvalue_ref ref)
{
	ValidationContext *context = (ValidationContext*)ctxt;
	ValidationEvent e = validation_event_obj_start();
	return validation_check(&e, context->validation_state, context);
}

static bool check_schema_jobject_end(void *ctxt, jvalue_ref ref)
{
	ValidationContext *context = (ValidationContext*)ctxt;
	context->jvalue = ref;
	ValidationEvent e = validation_event_obj_end();
	return validation_check(&e, context->validation_state, context);
}

static bool check_schema_jarray_start(void *ctxt, jvalue_ref ref)
{
	ValidationContext *context = (ValidationContext*)ctxt;
	ValidationEvent e = validation_event_arr_start();
	return validation_check(&e, context->validation_state, context);
}

static bool check_schema_jarray_end(void *ctxt, jvalue_ref ref)
{
	ValidationContext *context = (ValidationContext*)ctxt;
	context->jvalue = ref;
	ValidationEvent e = validation_event_arr_end();
	return validation_check(&e, context->validation_state, context);
}

static bool check_schema_jnumber_raw(void *ctxt, jvalue_ref ref)
{
	ValidationContext *context = (ValidationContext*)ctxt;
	raw_buffer raw = jnum_deref(ref)->value.raw;
	ValidationEvent e = validation_event_number(raw.m_str, raw.m_len);
	return validation_check(&e, context->validation_state, context);
}

static bool check_schema_jnumber_double(void *ctxt, jvalue_ref ref)
{
	char buf[24];
	int len = snprintf(buf, sizeof(buf), "%.14lg", jnum_deref(ref)->value.floating);
	ValidationContext *context = (ValidationContext*)ctxt;
	ValidationEvent e = validation_event_number(buf, len);
	return validation_check(&e, context->validation_state, context);
}

static bool check_schema_jnumber_int(void *ctxt, jvalue_ref ref)
{
	char buf[24];
	int len = snprintf(buf, sizeof(buf), "%" PRId64, jnum_deref(ref)->value.integer);
	ValidationContext *context = (ValidationContext*)ctxt;
	ValidationEvent e = validation_event_number(buf, len);
	return validation_check(&e, context->validation_state, context);
}

static bool check_schema_jstring(void *ctxt, jvalue_ref ref)
{
	ValidationContext *context = (ValidationContext*)ctxt;
	raw_buffer raw = jstring_deref(ref)->m_data;
	ValidationEvent e = validation_event_string(raw.m_str, raw.m_len);
	return validation_check(&e, context->validation_state, context);
}

static bool check_schema_jbool(void *ctxt, jvalue_ref ref)
{
	ValidationContext *context = (ValidationContext*)ctxt;
	ValidationEvent e = validation_event_boolean(jboolean_deref(ref)->value);
	return validation_check(&e, context->validation_state, context);
}

static void error_callback(ValidationState *s, ValidationErrorCode error, void *ctxt)
{
	JErrorCallbacksRef callbacks = ((ValidationContext *) ctxt)->callbacks;
	if (!callbacks)
		return;
	if (callbacks && callbacks->m_schema)
	{
		struct __JSAXContext fake_sax_ctxt =
		{
			.m_errors = callbacks,
			.m_error_code = error,
		};
		callbacks->m_schema(callbacks->m_ctxt, &fake_sax_ctxt);
	}
}

static bool has_array_duplicates(ValidationState *s, void *ctxt)
{
	return jarray_has_duplicates(((ValidationContext *) ctxt)->jvalue);
}

static Notification jvalue_check_notification =
{
	.error_func = &error_callback,
	.has_array_duplicates = &has_array_duplicates,
};

static struct TraverseCallbacks traverse = {
	check_schema_jnull,
	check_schema_jbool,
	check_schema_jnumber_int,
	check_schema_jnumber_double,
	check_schema_jnumber_raw,
	check_schema_jstring,
	check_schema_jobject_start,
	check_schema_jkeyvalue,
	check_schema_jobject_end,
	check_schema_jarray_start,
	check_schema_jarray_end,
};

static bool on_default_property(ValidationState *s, char const *key, jvalue_ref value, void *_ctxt)
{
	ValidationContext *ctxt = (ValidationContext *)_ctxt;
	assert(ctxt->jvalue);
	return jobject_put(ctxt->jvalue, jstring_create(key), jvalue_duplicate(value));
}

static Notification jvalue_apply_notification =
{
	.error_func = &error_callback,
	.has_array_duplicates = &has_array_duplicates,
	.default_property_func = &on_default_property,
};

static inline void set_error_from_context(struct __JSAXContext *ctx, jerror_type type, jerror** err)
{
	if (err)
	{
		const char* error_text = ctx->errorDescription ? ctx->errorDescription : "unknown error";
		jerror_set_formatted(err, type, "%d: %s", ctx->m_error_code, error_text);
	}
}

static bool cb_parser_error(void *ctxt, JSAXContextRef parseCtxt)
{
	set_error_from_context((struct __JSAXContext*)parseCtxt,
	                       JERROR_TYPE_SYNTAX, (jerror **)ctxt);
	return false;
}

static bool cb_schema_error(void *ctxt, JSAXContextRef parseCtxt)
{
	set_error_from_context((struct __JSAXContext*)parseCtxt,
	                       JERROR_TYPE_SCHEMA, (jerror**)ctxt);
	return false;
}

static bool cb_unknown_error(void *ctxt, JSAXContextRef parseCtxt)
{
	set_error_from_context((struct __JSAXContext*)parseCtxt,
	                       JERROR_TYPE_INTERNAL, (jerror**)ctxt);
	return false;
}

static bool jvalue_schema_work(jvalue_ref jref, const jschema_ref schema, JErrorCallbacksRef cb,
                               Notification *notifications)
{
	assert(schema != NULL);

	ValidationState validation_state = { 0 };
	validation_state_init(&validation_state,
	                      schema->validator,
	                      schema->uri_resolver,
	                      notifications);

	ValidationContext ctxt = {
		.callbacks = cb,
		.jvalue = jref,
		.validation_state = &validation_state,
	};

	bool retVal = jvalue_traverse(jref, &traverse, &ctxt);
	validation_state_clear(&validation_state);

	return retVal;
}

bool jvalue_check_schema(jvalue_ref val, const JSchemaInfoRef schema)
{
	if (val == NULL)
	{
		return false;
	}

	if (val->m_type != JV_OBJECT && val->m_type != JV_ARRAY)
	{
		return false;
	}

	if (schema == NULL)
	{
		return false;
	}

	return jvalue_schema_work(val, schema->m_schema, schema->m_errHandler, &jvalue_check_notification);
}

bool jvalue_validate(const jvalue_ref val, const jschema_ref schema, jerror **err)
{
	struct JErrorCallbacks cb =
	{
		.m_parser  = cb_parser_error,
		.m_schema  = cb_schema_error,
		.m_unknown = cb_unknown_error,
		.m_ctxt    = err
	};

	return jvalue_schema_work(val, schema, &cb, &jvalue_check_notification);
}

bool jvalue_apply_schema(jvalue_ref val, const JSchemaInfoRef schema)
{
	if (val == NULL)
	{
		return false;
	}

	if (val->m_type != JV_OBJECT && val->m_type != JV_ARRAY)
	{
		return false;
	}

	if (schema == NULL)
	{
		return false;
	}

	return jvalue_schema_work(val, schema->m_schema, schema->m_errHandler,
	                          &jvalue_apply_notification);
}

bool jvalue_validate_apply(jvalue_ref val, const jschema_ref schema, jerror **err)
{
	struct JErrorCallbacks cb =
	{
		.m_parser  = cb_parser_error,
		.m_schema  = cb_schema_error,
		.m_unknown = cb_unknown_error,
		.m_ctxt    = err
	};
	return jvalue_schema_work(val, schema, &cb, &jvalue_apply_notification);
}
