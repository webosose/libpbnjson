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

#ifndef __JQUERY_H_
#define __JQUERY_H_

#include "jerror.h"
#include "jobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file
 * Contains API to query JSON values from object. It allows to collect necessaru objects based on written string request.
 * It significantly decreased code size with hardcoded query logic. It makes code reliable and supportable.
 */

typedef struct jquery* jquery_ptr;

/**
 * @brief Construct jquery from the query string
 * @param str jquery sting
 * @param err pbnjson error information.
 * @return precompiled query object
 */
jquery_ptr jquery_create(const char *str, jerror **err);

/**
 * @brief Init jquery with jvalue
 * @param query jquery
 * @param JSON
 * @param err pbnjson error information.
 * @return true, if init process was success finish
 */
bool jquery_init(jquery_ptr query, jvalue_ref JSON,  jerror **err);

/**
 * @brief Get next jquery result for current JSON, which was set with jquery_init,
 * for current JSON, set by previous jquery_init call
 * @param next pointer to jquery
 * @return next jvalue or NULL if next value not found
 */
jvalue_ref jquery_next(jquery_ptr next);

/**
 * @brief Free jquery_ptr memory
 * @param query query to free.
 */
void jquery_free(jquery_ptr query);

#ifdef __cplusplus
}
#endif

#endif // __JQUERY_H_
