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

#ifndef JOBJECT_INTERNAL_H_
#define JOBJECT_INTERNAL_H_

#include <stdbool.h>
#include <japi.h>
#include <jtypes.h>
#include <glib.h>
#include "jconversion.h"
#include "jerror.h"

#define ARRAY_BUCKET_SIZE (1 << 4)
#define OUTSIDE_ARR_BUCKET_RANGE(value) ((value) & (~(ARRAY_BUCKET_SIZE - 1)))

typedef struct _jbuffer {
	raw_buffer buffer;
	void (*destructor)(struct _jbuffer *);
} _jbuffer;

struct jvalue {
	JValueType m_type;
	int m_refCnt;
	_jbuffer m_string;
	_jbuffer m_file;
};

typedef struct PJSON_LOCAL jvalue jvalue;
typedef struct PJSON_LOCAL dom_string_memory_pool dom_string_memory_pool;

typedef struct PJSON_LOCAL {
	// m_value should always be the first field
	jvalue m_value;
	bool value;
} jbool;

_Static_assert(offsetof(jbool, m_value) == 0, "jbool and jbool.m_value should have the same addresses");

typedef enum {
	NUM_RAW,
	NUM_FLOAT,
	NUM_INT,
} JNumType;

typedef struct PJSON_LOCAL {
	// m_value should always be the first field
	jvalue m_value;
	union {
		raw_buffer raw;
		double floating;
		int64_t integer;
	} value;
	JNumType m_type;
	ConversionResultFlags m_error;
	jdeallocator m_rawDealloc;
} jnum;

_Static_assert(offsetof(jnum, m_value) == 0, "jnum and jnum.m_value should have the same addresses");

typedef struct PJSON_LOCAL {
	// m_value should always be the first field
	jvalue m_value;
	jdeallocator m_dealloc;
	raw_buffer m_data;
} jstring;

_Static_assert(offsetof(jstring, m_value) == 0, "jstring and jstring.m_value should have the same addresses");

typedef struct {
	jstring m_header;
	char m_buf[];
} jstring_inline;

typedef struct PJSON_LOCAL {
	// m_value should always be the first field
	jvalue m_value;
	jvalue_ref m_smallBucket[ARRAY_BUCKET_SIZE];
	jvalue_ref *m_bigBucket;
	ssize_t m_size;
	ssize_t m_capacity;
} jarray;

_Static_assert(offsetof(jarray, m_value) == 0, "jarray and jarray.m_value should have the same addresses");

typedef struct PJSON_LOCAL {
	// m_value should always be the first field
	jvalue m_value;
	GHashTable *m_members;
} jobject;

_Static_assert(offsetof(jobject, m_value) == 0, "jobject and jobject.m_value should have the same addresses");

extern PJSON_LOCAL jvalue JNULL;

void PJSON_LOCAL jvalue_init (jvalue_ref val, JValueType type);

PJSON_LOCAL bool jobject_init(jobject *obj);

extern PJSON_LOCAL int64_t jnumber_deref_i64(jvalue_ref num);

extern PJSON_LOCAL bool jboolean_deref_to_value(jvalue_ref boolean);

extern PJSON_LOCAL bool jbuffer_equal(raw_buffer buffer1, raw_buffer buffer2);

extern PJSON_LOCAL raw_buffer jnumber_deref_raw(jvalue_ref num);

extern PJSON_LOCAL bool jarray_has_duplicates(jvalue_ref arr);

inline static jbool* jboolean_deref(jvalue_ref boolean) { return (jbool*)boolean; }

inline static jnum* jnum_deref(jvalue_ref num) { return (jnum*)num; }

inline static jstring* jstring_deref(jvalue_ref str) { return (jstring*)str; }

inline static jarray* jarray_deref(jvalue_ref array) { return (jarray*)array; }

inline static jobject* jobject_deref(jvalue_ref array) { return (jobject*)array; }

void _jbuffer_munmap(_jbuffer *buf);
void _jbuffer_free(_jbuffer *buf);

jvalue_ref jstring_create_from_pool_internal(dom_string_memory_pool *pool, const char* data, size_t len);
jvalue_ref jnumber_create_from_pool_internal(dom_string_memory_pool *pool, const char* data, size_t len);

bool j_fopen(const char *file, _jbuffer *buf, jerror **err);
bool j_fopen2(int fd, _jbuffer *buf, jerror **err);

guint PJSON_LOCAL ObjKeyHash(gconstpointer key);
gboolean PJSON_LOCAL ObjKeyEqual(gconstpointer a, gconstpointer b);

#endif /* JOBJECT_INTERNAL_H_ */
