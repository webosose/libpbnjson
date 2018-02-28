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

#pragma once

#include "feature.h"
#include "validator_fwd.h"
#include <stdbool.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Object patternProperties class */
typedef struct _ObjectPatternProperties
{
	Feature base;       /**< @brief Base class */
	GSList *patterns;   /**< @brief List of pairs (regex, validator) */
} ObjectPatternProperties;

/** @brief Constructor */
ObjectPatternProperties* object_pattern_properties_new(void);

/** @brief Increment reference counter. */
ObjectPatternProperties* object_pattern_properties_ref(ObjectPatternProperties *o);

/** @brief Decrement reference counter. Once it drops to zero, the object is destroyed. */
void object_pattern_properties_unref(ObjectPatternProperties *o);

/** @brief Add another pair (pattern, validator) to the list.
 *
 * The pattern is compiled, the validator is moved. The validator is freed if pattern compilation fails.
 *
 * @param[in] o This object
 * @param[in] pattern Regular expression in ECMA-262 dialect in {"patternProperties": {"pattern": {...}}}
 * @param[in] pattern_len Count of bytes in pattern
 * @param[in] v Validator for subschema of property value in {"patternProperties": {"pattern": {...}}}
 *
 * @return false if regex failed to compile, true otherwise.
 */
bool object_pattern_properties_add(ObjectPatternProperties *o, const char *pattern, size_t pattern_len, Validator *v);

/** @brief Pick up a validator for given property name (key)
 *
 * Look through the list of regexes and return a validator for all the matched patterns.
 * If a single pattern matches, the validator just for it is returned.
 * If multiple patterns match, a new combined validator is created to continue with.
 *
 * @param[in] o This object
 * @param[in] key Property name to match against list of patterns
 * @return An instance of a validator to apply to the property value, needs to be unreffed.
 */
Validator* object_pattern_properties_find(ObjectPatternProperties *o, const char *key);

/** @brief Visit contained validators. */
void object_pattern_properties_visit(ObjectPatternProperties *o,
                                     VisitorEnterFunc enter_func, VisitorExitFunc exit_func,
                                     void *ctxt);

#ifdef __cplusplus
}  // extern "C"
#endif
