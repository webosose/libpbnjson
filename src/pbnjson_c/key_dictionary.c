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

#include "key_dictionary.h"
#include "jobject.h"
#include "jobject_internal.h"
#include "liblog.h"

#include <assert.h>
#include <glib.h>

// We maintain a hash table with known object properties names in it.
// They are referenced without ownership, but whenever a key is about to be
// destroyed, we get a notification to remove the key from the dictionary.
// Races of destructors against lookups should be treated carefully!

static GHashTable *key_dictionary;   /// Set of interned keys (custom jstring *)
static pthread_once_t key_dictionary_initialized = PTHREAD_ONCE_INIT;
static pthread_mutex_t key_dictionary_mutex = PTHREAD_MUTEX_INITIALIZER;

static void keyDictionaryInit(void)
{
	key_dictionary = g_hash_table_new_full(ObjKeyHash, ObjKeyEqual,
	                                       NULL, NULL);
}

static void keyStringDtor(void *buffer)
{
	assert(key_dictionary != NULL);

	jstring_inline *jstr = (jstring_inline *) ((char*)buffer - offsetof(jstring_inline, m_buf));
	//SANITY_CHECK_JSTR_BUFFER((jvalue_ref) jstr);
	// TODO: sanity check that we remove same pointer

	pthread_mutex_lock(&key_dictionary_mutex);
	bool removed = g_hash_table_steal(key_dictionary, jstr);
	assert(removed);
	(void) removed;
	pthread_mutex_unlock(&key_dictionary_mutex);

	SANITY_CLEAR_MEMORY(jstr->m_header.m_data.m_str, jstr->m_header.m_data.m_len);
}

static jvalue_ref allocKeyString(raw_buffer str)
{
	jstring_inline *new_str = (jstring_inline*) calloc(1, sizeof(jstring_inline) + str.m_len);
	SANITY_CHECK_POINTER(new_str);
	jvalue_init((jvalue_ref)new_str, JV_STR);

	memcpy(new_str->m_buf, str.m_str, str.m_len);
	// Notify us about key destruction to remove it from the dictionary
	new_str->m_header.m_dealloc = keyStringDtor;
	new_str->m_header.m_data.m_str = new_str->m_buf;
	new_str->m_header.m_data.m_len = str.m_len;

	return (jvalue_ref) new_str;
}

jvalue_ref keyDictionaryLookup(const char *key, size_t keyLen)
{
	jstring jkey =
	{
		.m_value = {
			.m_refCnt = 1,
			.m_type = JV_STR,
		},
		.m_data = {
			.m_str = key,
			.m_len = keyLen,
		},
	};

	jvalue_ref jstr;

	pthread_once(&key_dictionary_initialized, keyDictionaryInit);

	// To tackle race against key destruction, we'll be detecting keys being
	// destructed at the moment by looking at their reference count. If no
	// other owning references are active, we'll retry lookup.

	while (true) {
		pthread_mutex_lock(&key_dictionary_mutex);

		if (g_hash_table_lookup_extended(key_dictionary, &jkey, (gpointer *) &jstr, NULL)) {
			// If we picked up a key being destroyed, skip it and try to look up again.
			if (UNLIKELY(g_atomic_int_add(&jstr->m_refCnt, 1) <= 0)) {
				assert(jstr->m_refCnt > 0 && "We share ownership of just copied value");
				// Note that if the thread that is destructing the jstr doesn't proceed with
				// destruction before our second iteration, we may pick up the same instance
				// again, increasing its counter further. Thus, we properly decrease counter
				// for now. It's impossible that our decrement could result in
				// destruction, therefore we decrement the counter ourselves, not via j_release().
				(void) g_atomic_int_dec_and_test(&jstr->m_refCnt);
				pthread_mutex_unlock(&key_dictionary_mutex);
				continue;
			}
			pthread_mutex_unlock(&key_dictionary_mutex);
			return jstr;
		}

		// No suitable key found in the dictionary, create one and put to the dictionary.
		jstr = allocKeyString(j_str_to_buffer(key, keyLen));
		g_hash_table_insert(key_dictionary, jstr, NULL);

		pthread_mutex_unlock(&key_dictionary_mutex);
		return jstr;
	}
}
