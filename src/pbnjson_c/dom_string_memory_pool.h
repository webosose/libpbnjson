// Copyright (c) 2016-2024 LG Electronics, Inc.
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

#ifndef DOM_STRING_MEMORY_POOL_H_
#define DOM_STRING_MEMORY_POOL_H_

#include <stddef.h>

/**
	Memory pool for strings in dom creation lifetime. It means that pool
	should be destroyed after dom is created. All memory is self refcounted,
	so it will be destroyed if last string in allocated block is gone.
*/

typedef struct dom_string_memory_pool dom_string_memory_pool;

dom_string_memory_pool*
dom_string_memory_pool_create();

void
dom_string_memory_pool_destroy(dom_string_memory_pool*);

void*
dom_string_memory_pool_alloc(dom_string_memory_pool* pool, size_t raw);

void
dom_string_memory_pool_mark_as_free(void *ptr);

#endif //DOM_STRING_MEMORY_POOL_H_
