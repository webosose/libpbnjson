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

#ifndef JNUM_CONVERSION_INTERNAL_H_
#define JNUM_CONVERSION_INTERNAL_H_

#include <stdint.h>
#include <jconversion.h>
#include <jtypes.h>
#include <japi.h>
#include <stdlib.h>
#include <compiler/nonnull_attribute.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represents number components for further forming an integer or double
 *
 * @note Since double contains only 52 bits of fraction we can easily fit
 *       them in int64_t.
 */
typedef struct
{
	uint64_t fraction;
	int64_t exponent;
	int sign;
	ConversionResultFlags flags;
} number_components;

/**
 * Parse number to a number_components
 */
PJSON_LOCAL void numberParse(number_components *components, raw_buffer str);

PJSON_LOCAL ConversionResultFlags jstr_to_i32(raw_buffer *str, int32_t *result);
PJSON_LOCAL ConversionResultFlags jstr_to_i64(raw_buffer *str, int64_t *result);
PJSON_LOCAL ConversionResultFlags jstr_to_double(raw_buffer *str, double *result);
PJSON_LOCAL ConversionResultFlags jdouble_to_i32(double value, int32_t *result);
PJSON_LOCAL ConversionResultFlags jdouble_to_i64(double value, int64_t *result);
PJSON_LOCAL ConversionResultFlags jdouble_to_str(double value, raw_buffer *str);
PJSON_LOCAL ConversionResultFlags ji32_to_i64(int32_t value, int64_t *result);
PJSON_LOCAL ConversionResultFlags ji32_to_double(int32_t value, double *result);
PJSON_LOCAL ConversionResultFlags ji32_to_str(int32_t value, raw_buffer *str);
PJSON_LOCAL ConversionResultFlags ji64_to_i32(int64_t value, int32_t *result);
PJSON_LOCAL ConversionResultFlags ji64_to_double(int64_t value, double *result);
PJSON_LOCAL ConversionResultFlags ji64_to_str(int64_t value, raw_buffer *str);

#ifdef __cplusplus
}
#endif

#endif /* JNUM_CONVERSION_INTERNAL_H_ */
