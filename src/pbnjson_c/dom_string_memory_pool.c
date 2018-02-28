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

#include "dom_string_memory_pool.h"

#include <glib.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>

// Chunk holds data buffer and current used size from its buffer and ref count
// When chunk allocate memory it incs ref count and when it marks is as freed it decs count
typedef struct dom_string_memory_chunk {
	struct dom_string_memory_chunk *prev;
	volatile int ref;

	size_t size;
	size_t used;
	char data[];
} dom_string_memory_chunk;

// Pool is reversed single list with preferred chunk size
typedef struct dom_string_memory_pool {
	dom_string_memory_chunk *tail;
} dom_string_memory_pool;

// Find chunk that has size + meta information available memory
static dom_string_memory_chunk* find_chunk_with_available_memory(dom_string_memory_pool* pool, size_t size)
{
	for (dom_string_memory_chunk* chunk = pool->tail; chunk; chunk = chunk->prev)
	{
		if (chunk->used + size <= chunk->size)
			return chunk;
	}

	return NULL;
}

// Store pointer to chunk from wich memory will be allocated
static inline char* store_self(dom_string_memory_chunk* chunk)
{
	char *marker = chunk->data + chunk->used;

	*((dom_string_memory_chunk**)marker) = chunk;
	chunk->used += sizeof(dom_string_memory_chunk*);
	marker += sizeof(dom_string_memory_chunk*);

	g_atomic_int_inc(&chunk->ref);
	return marker;
}

static dom_string_memory_chunk* dom_string_memory_pool_chunk_create(size_t size)
{
	dom_string_memory_chunk* chunk = (dom_string_memory_chunk*)malloc(sizeof(dom_string_memory_chunk) + size);

	chunk->ref  = 0;
	chunk->prev = NULL;
	chunk->used = 0;
	chunk->size = size;

	return chunk;
}

static void dom_string_memory_pool_chunk_unref(dom_string_memory_chunk* chunk)
{
	if (g_atomic_int_dec_and_test(&chunk->ref))
	{
		size_t page = getpagesize();
		char *start = (char *)chunk;
		char *end = start + chunk->size;

		start += (page - (intptr_t)start) % page;
		end -= (intptr_t)end % page;

		if (start < end)
		{
			madvise(start, end - start, MADV_DONTNEED);
		}
		free(chunk);
	}
}

dom_string_memory_pool* dom_string_memory_pool_create()
{
	dom_string_memory_pool* pool = (dom_string_memory_pool*)malloc(sizeof(dom_string_memory_pool));
	pool->tail = NULL;
	return pool;
}

void dom_string_memory_pool_destroy(dom_string_memory_pool* pool)
{
	free(pool);
}

void *dom_string_memory_pool_alloc(dom_string_memory_pool* pool, size_t raw)
{
	size_t size = sizeof(dom_string_memory_chunk*) + raw;
	dom_string_memory_chunk *chunk = find_chunk_with_available_memory(pool, size);
	if (!chunk)
	{
		chunk = dom_string_memory_pool_chunk_create(MAX(size, 16u * getpagesize()));
		chunk->prev = pool->tail;
		pool->tail = chunk;
	}

	char *marker = store_self(chunk);
	chunk->used += raw;

	return marker;
}

void dom_string_memory_pool_mark_as_free(void *ptr)
{
	dom_string_memory_chunk *chunk = *(dom_string_memory_chunk**)((char*)ptr - sizeof(dom_string_memory_chunk*));
	dom_string_memory_pool_chunk_unref(chunk);
}
