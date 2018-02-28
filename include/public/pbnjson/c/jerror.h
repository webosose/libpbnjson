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

#ifndef INCLUDE_PUBLIC_PBNJSON_C_JERROR_H_
#define INCLUDE_PUBLIC_PBNJSON_C_JERROR_H_

#include <string.h>
#include "japi.h"

/**
 * @brief Contains error details, occured during JSON processing
 */
typedef struct jerror jerror;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Duplicates passed error. If error is NULL it returns NULL.
 *
 * @param other pbnjson error to copy.
 * @return The newly created jerror with same data as the other.
 */
PJSON_API jerror *jerror_duplicate(const jerror *other);

/**
 * Free allocated space for jerror. If error is NULL it simply returns.
 *
 * @param error pbnjson error information to free.
 */
PJSON_API void jerror_free(jerror *error);

/**
 * Get textual representation of the occurred error, can be used for logging.
 *
 * @param error pbnjson error information.
 * @param str   string buffer for a resulting string.
 * @param size  size of the buffer.
 * @return The number of characters that would have been written if size had been
 *         sufficiently large, not counting the terminating null character.
 */
PJSON_API int jerror_to_string(jerror *error, char *str, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_PUBLIC_PBNJSON_C_JERROR_H_ */
