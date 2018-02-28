// Copyright (c) 2009-2018 LG Electronics, Inc.
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


#include "jobject.h"
#include "jvalue_stringify.h"

#include "jobject_internal.h"
#include "jtraverse.h"
#include "gen_stream.h"

static bool to_string_append_jnull(void *ctxt, jvalue_ref jref)
{
	JStreamRef generating = (JStreamRef)ctxt;
	return generating->null_value(generating) != NULL;
}

//Helper function for jobject_to_string_append()
static bool to_string_append_jkeyvalue(void *ctxt, jvalue_ref jref)
{
	JStreamRef generating = (JStreamRef)ctxt;
	raw_buffer raw = jstring_deref(jref)->m_data;
	return generating->o_key(generating, raw) != NULL;
}

static bool to_string_append_jobject_start(void *ctxt, jvalue_ref jref)
{
	JStreamRef generating = (JStreamRef)ctxt;
	return generating->o_begin(generating) != NULL;
}

static bool to_string_append_jobject_end(void *ctxt, jvalue_ref jref)
{
	JStreamRef generating = (JStreamRef)ctxt;
	return generating->o_end(generating) != NULL;
}

static bool to_string_append_jarray_start(void *ctxt, jvalue_ref jref)
{
	JStreamRef generating = (JStreamRef)ctxt;
	return generating->a_begin(generating) != NULL;
}

static bool to_string_append_jarray_end(void *ctxt, jvalue_ref jref)
{
	JStreamRef generating = (JStreamRef)ctxt;
	return generating->a_end(generating) != NULL;
}

static bool to_string_append_jnumber_raw(void *ctxt, jvalue_ref jref)
{
	JStreamRef generating = (JStreamRef)ctxt;
	return generating->number(generating, jnum_deref(jref)->value.raw) != NULL;
}

static bool to_string_append_jnumber_double(void *ctxt, jvalue_ref jref)
{
	JStreamRef generating = (JStreamRef)ctxt;
	return generating->floating(generating, jnum_deref(jref)->value.floating) != NULL;
}

static bool to_string_append_jnumber_int(void *ctxt, jvalue_ref jref)
{
	JStreamRef generating = (JStreamRef)ctxt;
	return generating->integer(generating, jnum_deref(jref)->value.integer) != NULL;
}

static inline bool to_string_append_jstring(void *ctxt, jvalue_ref jref)
{
	JStreamRef generating = (JStreamRef)ctxt;
	raw_buffer raw = jstring_deref(jref)->m_data;
	return generating->string(generating, raw) != NULL;
}

static inline bool to_string_append_jbool(void *ctxt, jvalue_ref jref)
{
	JStreamRef generating = (JStreamRef)ctxt;
	return generating->boolean(generating, jboolean_deref(jref)->value) != NULL;
}

static struct TraverseCallbacks traverse = {
	to_string_append_jnull,
	to_string_append_jbool,
	to_string_append_jnumber_int,
	to_string_append_jnumber_double,
	to_string_append_jnumber_raw,
	to_string_append_jstring,
	to_string_append_jobject_start,
	to_string_append_jkeyvalue,
	to_string_append_jobject_end,
	to_string_append_jarray_start,
	to_string_append_jarray_end,
};

static const char *jvalue_tostring_internal(jvalue_ref val, JSchemaInfoRef schemainfo, const char *indent)
{
	if (UNLIKELY(val == NULL))
		return NULL;

	_jbuffer *str = &val->m_string;
	if (str->destructor) {
		str->destructor(str);
	}
	// remove this check in 3.0
	if (schemainfo && !jvalue_check_schema(val, schemainfo)) {
		return NULL;
	}
	JStreamRef generating = jstreamInternal(TOP_None, indent);
	if (UNLIKELY(generating == NULL)) {
		return NULL; // OOM
	}
	if (UNLIKELY(!jvalue_traverse(val, &traverse, generating))) {
		return NULL; // We are not expecting that something goes wrong
	}

	val->m_string = (_jbuffer){
		j_cstr_to_buffer(generating->finish(generating, NULL)),
		_jbuffer_free
	};

	return val->m_string.buffer.m_str;
}

const char *jvalue_tostring_schemainfo(jvalue_ref val, const JSchemaInfoRef schemainfo)
{
	if (!jschema_resolve_ex(schemainfo->m_schema, schemainfo->m_resolver))
		return NULL;

	return jvalue_tostring_internal(val, schemainfo, NULL);
}

const char *jvalue_tostring_simple(jvalue_ref val)
{
	return jvalue_tostring_internal(val, NULL, NULL);
}

const char *jvalue_tostring(jvalue_ref val, const jschema_ref schema)
{
	JSchemaInfo schemainfo;

	jschema_info_init(&schemainfo, schema, NULL, NULL);

	return jvalue_tostring_internal(val, &schemainfo, NULL);
}

const char* jvalue_stringify(jvalue_ref val)
{
	return jvalue_tostring_internal(val, NULL, NULL);
}

const char* jvalue_prettify(jvalue_ref val, const char *indent)
{
	return jvalue_tostring_internal(val, NULL, indent);
}
