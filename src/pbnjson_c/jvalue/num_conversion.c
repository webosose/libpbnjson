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

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <assert.h>
#include <inttypes.h>

#include <jtypes.h>

#include "num_conversion.h"
#include <compiler/nonnull_attribute.h>
#include <compiler/pure_attribute.h>
#include <compiler/builtins.h>
#include "../liblog.h"

#define PJSON_MAX_INT INT32_MAX
#define PJSON_MIN_INT INT32_MIN

#define PJSON_MAX_INT64 INT64_MAX
#define PJSON_MIN_INT64 INT64_MIN

#define PJSON_MAX_INT_IN_DBL INT64_C(0x1FFFFFFFFFFFFF)
#define PJSON_MIN_INT_IN_DBL -PJSON_MAX_INT_IN_DBL

#ifdef PJSON_SAFE_NOOP_CONVERSION
static ConversionResult ji32_noop(int32_t value, int32_t *result)
{
	CHECK_POINTER_RETURN_VALUE(result, CONV_BAD_ARGS);
	*result = value;
	return CONV_OK;
}

static ConversionResult ji64_noop(int64_t value, int64_t *result)
{
	CHECK_POINTER_RETURN_VALUE(result, CONV_BAD_ARGS);
	*result = value;
	return CONV_OK;
}

static ConversionResult jdouble_noop(double value, double *result)
{
	CHECK_POINTER_RETURN_VALUE(result, CONV_BAD_ARGS);
	*result = value;
	return CONV_OK;
}
#else
#define ji32_noop NULL
#define ji64_noop NULL
#define jdouble_noop NULL
#endif /* PJSON_SAFE_NOOP_CONVERSION */

/// mark components as not-a-number
static void numberNaN(number_components *components)
{
	components->flags = CONV_NOT_A_NUM;
}

/// Parse exponent that do not fit into int64_t
static void numberExpOverflow(number_components *components)
{
	if (components->exponent > 0) // positive overflow (number overflow)
	{
		components->fraction = UINT64_MAX;
		components->exponent = INT64_MAX;
		components->flags = components->sign < 0 ? CONV_NEGATIVE_OVERFLOW : CONV_POSITIVE_OVERFLOW;
	}
	else // negative overflow (too small to catch)
	{
		components->fraction = 0; // almost zero represented as zero
		components->exponent = 0;
		components->flags = CONV_PRECISION_LOSS;
	}
}

/// parse exponent part of number (right after 'e' separator)
static void numberParseExp(number_components *components, const char *ptr, const char *end)
{
	assert(ptr < end);
	const int64_t exp = components->exponent;
	int64_t value = 0;
	int sign = 1;

	// process sign
	switch (*ptr)
	{
	case '-':
		sign = -1;
		// fall through case label
	case '+':
		if (UNLIKELY(++ptr == end)) return numberNaN(components);
		break;
	}

	// process digits
	for (; ptr < end; ++ptr)
	{
		switch (*ptr)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				const int digit = (int)*ptr - 48;  // 48 ->'0'
				if (UNLIKELY(exp + sign*value < (INT64_MIN+digit)/10 || exp + sign*value > (INT64_MAX-digit)/10))
				{
					components->exponent += sign*value;
					return numberExpOverflow(components);
				}
				value = value*10 + digit;
			}
			break;
		default:
			return numberNaN(components);
		}
	}
	components->exponent += sign*value;
}

/// scan rest of decimal digits that will not fit into int64_t
static void numberParseDecimalLoss(number_components *components, const char *ptr, const char *end)
{
	ConversionResultFlags flags = components->flags;

	// skip zeroes
	for (; ptr < end && (flags & CONV_PRECISION_LOSS) == 0; ++ptr)
	{
		switch (*ptr)
		{
		case '0':
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			flags |= CONV_PRECISION_LOSS; // also marks end of this for-loop
			break;
		case 'e':
		case 'E':
			if (UNLIKELY(ptr == end)) return numberNaN(components);
			components->flags = flags;
			return numberParseExp(components, ++ptr, end);
		default:
			return numberNaN(components);
		}
	}

	// skip rest digits
	for (; ptr < end; ++ptr)
	{
		switch (*ptr)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			break;
		case 'e':
		case 'E':
			if (UNLIKELY(ptr == end)) return numberNaN(components);
			components->flags = flags;
			return numberParseExp(components, ++ptr, end);
		default:
			return numberNaN(components);
		}
	}

	components->flags = flags;
}

/// parse rest of integer digits to beef up an exponent
static void numberParseIntegerExp(number_components *components, const char *ptr, const char *end)
{
	assert(components->exponent >= 0);
	ConversionResultFlags flags = components->flags;
	int64_t exp = components->exponent;

	// scan through leading zeroes to calculate exponent
	for (; ptr < end && (flags & CONV_PRECISION_LOSS) == 0; ++ptr)
	{
		switch (*ptr)
		{
		case '0':
			++exp;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			++exp;
			flags |= CONV_PRECISION_LOSS; // also marks end of this for-loop
			break;
		case '.':
			if (UNLIKELY(++ptr == end)) return numberNaN(components);
			components->flags = flags;
			components->exponent = exp;
			return numberParseDecimalLoss(components, ptr, end);
		case 'e':
		case 'E':
			if (UNLIKELY(ptr == end)) return numberNaN(components);
			components->flags = flags;
			components->exponent = exp;
			return numberParseExp(components, ++ptr, end);
		default:
			return numberNaN(components);
		}
	}

	// count rest of digits
	for (; ptr < end; ++ptr)
	{
		switch (*ptr)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			++exp;
			break;
		case '.':
			if (UNLIKELY(++ptr == end)) return numberNaN(components);
			components->flags = flags;
			components->exponent = exp;
			return numberParseDecimalLoss(components, ptr, end);
		case 'e': case 'E':
			if (UNLIKELY(ptr == end)) return numberNaN(components);
			components->flags = flags;
			components->exponent = exp;
			return numberParseExp(components, ++ptr, end);
		default:
			return numberNaN(components);
		}
	}

	components->flags = flags;
	components->exponent = exp;
}

/// parse decimal part of number (right after dot)
/// @note this state will be skipped if int64_t can't hold any digits more
static void numberParseDecimal(number_components *components, const char *ptr, const char *end)
{
	assert(components->exponent >= 0);
	uint64_t value = components->fraction;
	int64_t exp = components->exponent;
	int zeroes = 0;
	int64_t base = 10;

	// scan through leading zeroes
	for (; ptr < end && *ptr == '0'; ++ptr)
	{
		++zeroes;
		base *= 10;
	}

	switch (*ptr)
	{
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		// re-align value to make exponent equal to zero
		for (; exp > 0; --exp)
		{
			if (UNLIKELY(value > UINT64_MAX/base))
			{
				components->fraction = value;
				components->exponent = exp;
				return numberParseDecimalLoss(components, ptr, end);
			}
			value *= 10;
		}
		break;
	default: ;
	}

	for (; ptr < end; ++ptr)
	{
		if (UNLIKELY(value > UINT64_MAX/base))
		{
			components->fraction = value;
			components->exponent = exp;
			return numberParseDecimalLoss(components, ptr, end);
		}
		switch (*ptr)
		{
		case '0':
			++zeroes;
			base *= 10;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			value = value*base + ((int)*ptr - 48);  // 48 -> '0'
			exp -= zeroes+1;
			base = 10;
			zeroes = 0;
			break;
		case 'e':
		case 'E':
			if (UNLIKELY(ptr == end)) return numberNaN(components);
			components->fraction = value;
			components->exponent = exp;
			return numberParseExp(components, ++ptr, end);
		default:
			return numberNaN(components);
		}
	}
	components->fraction = value;
	components->exponent = exp;
}

// parse integer part of number (after sign and before decimal/exponent)
static void numberParseInteger(number_components *components, const char *ptr, const char *end)
{
	uint64_t value;
	int zeroes = 0;
	int64_t base = 10;

	switch (*ptr)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		value = (int)*ptr - 48;  // 48 -> '0'
		break;
	default:
		return numberNaN(components);
	}

	for (++ptr; ptr < end; ++ptr)
	{
		switch (*ptr)
		{
		case '0':
			if (UNLIKELY(value > UINT64_MAX/base || base > INT64_MAX/10))
			{
				components->fraction = value;
				components->exponent += zeroes;
				return numberParseIntegerExp(components, ptr, end);
			}
			++zeroes;
			base *= 10;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				const int digit = (int)(*ptr) - 48;  // 48 -> '0'
				if (UNLIKELY(value > (UINT64_MAX-digit)/base))
				{
					components->fraction = value;
					components->exponent += zeroes;
					return numberParseIntegerExp(components, ptr, end);
				}
				value = value*base + digit;
				zeroes = 0;
				base = 10;
			}
			break;
		case '.':
			components->fraction = value;
			components->exponent += zeroes;
			return numberParseDecimal(components, ++ptr, end);
		case 'e':
		case 'E':
			if (UNLIKELY(ptr == end)) return numberNaN(components);
			components->fraction = value;
			components->exponent += zeroes;
			return numberParseExp(components, ++ptr, end);
		default:
			return numberNaN(components);
		}
	}
	components->fraction = value;
	components->exponent += zeroes;
}

/// Parse number into a components
void numberParse(number_components *components, raw_buffer str)
{
	*components = (number_components) {
		.fraction = 0,
		.exponent = 0,
		.sign = 1,
		.flags = CONV_OK,
	};
	const char *ptr = str.m_str, *end = str.m_str + str.m_len;

	// empty number is not a number
	if (UNLIKELY(ptr == end)) return numberNaN(components);

	// parse sign
	switch (*ptr)
	{
	case '-':
		components->sign = -1;
		// fall through case label
	case '+':
		if (UNLIKELY(++ptr == end)) return numberNaN(components);
		break;
	}
	return numberParseInteger(components, ptr, end);
}

ConversionResultFlags jstr_to_i32(raw_buffer *str, int32_t *result)
{
	ConversionResultFlags status1, status2 = CONV_GENERIC_ERROR;
	int64_t bigResult = 0;
	status1 = jstr_to_i64(str, &bigResult);

	if (UNLIKELY(status1 == CONV_NOT_A_NUM))
		return CONV_NOT_A_NUM;

	status2 = ji64_to_i32(bigResult, result);
	if (UNLIKELY(CONV_HAS_OVERFLOW(status2)))
		return status2;

	// note: status1 may indicate precision loss
	return status1 | status2;
}

ConversionResultFlags jstr_to_i64(raw_buffer *str, int64_t *result)
{
	static_assert(-INT64_MAX == INT64_MIN+1, "int64_t negative range is bigger exactly by one than positive range");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverflow"
	static_assert(INT64_MAX+1 == INT64_MIN, "int64_t positive overflow by one leads to INT64_MIN the one we may want to store");
	static_assert(-1 * INT64_MIN == INT64_MIN, "Overflowed positive int64_t as INT64_MIN can be multiplied by -1 and still be the smae number");
#pragma GCC diagnostic pop

	// here is some work-around to bring -INT64_MIN in uint64_t without overflow
	const uint64_t uint64_neg_max = (uint64_t)(-(INT64_MIN+1))+1;

	CHECK_POINTER_RETURN_VALUE(str->m_str, CONV_BAD_ARGS);
	CHECK_POINTER_RETURN_VALUE(result, CONV_BAD_ARGS);

	number_components components;
	numberParse(&components, *str);

	if (UNLIKELY(components.flags == CONV_NOT_A_NUM))
		return CONV_NOT_A_NUM;

	if (UNLIKELY(CONV_HAS_PRECISION_LOSS(components.flags) && components.exponent > 0))
	{
		// any precision loss with fraction stored in uint64_t and positive
		// exponent means that we definitely can't fit this number in int64_t
		if (components.sign > 0)
		{
			*result = INT64_MAX;
			return  CONV_POSITIVE_OVERFLOW;
		}
		else
		{
			*result = INT64_MIN;
			return  CONV_NEGATIVE_OVERFLOW;
		}
	}

	int sign = components.sign;
	uint64_t fraction = components.fraction;
	int64_t exp = components.exponent;
	if (UNLIKELY(exp < 0))
	{
		// re-align exp to make it non-negative with precision loss
		components.flags |= CONV_PRECISION_LOSS;
		for (; exp < 0; ++exp) fraction /= 10;
	}

	// check int64_t overflow and re-align exponent if positive
	if (sign > 0)
	{
		if (UNLIKELY(fraction > INT64_MAX))
		{
			*result = INT64_MAX;
			return CONV_POSITIVE_OVERFLOW;
		}
		for (; exp > 0; --exp)
		{
			if (UNLIKELY(fraction > INT64_MAX/10))
			{
				*result = INT64_MAX;
				return CONV_POSITIVE_OVERFLOW;
			}
			fraction *= 10;
		}
	}
	else
	{
		if (UNLIKELY(fraction > uint64_neg_max))
		{
			*result = INT64_MIN;
			return CONV_NEGATIVE_OVERFLOW;
		}
		for (; exp > 0; --exp)
		{
			if (UNLIKELY(fraction > uint64_neg_max/10))
			{
				*result = INT64_MIN;
				return CONV_NEGATIVE_OVERFLOW;
			}
			fraction *= 10;
		}
	}

	*result = sign * (int64_t)fraction;
	return components.flags;
}

ConversionResultFlags jstr_to_double(raw_buffer *str, double *result)
{
	static_assert(FLT_RADIX == 2, "Support only double with binary exponent");

	CHECK_POINTER_RETURN_VALUE(str->m_str, CONV_BAD_ARGS);
	CHECK_POINTER_RETURN_VALUE(result, CONV_BAD_ARGS);

	number_components components;
	numberParse(&components, *str);

	if (UNLIKELY(components.flags == CONV_NOT_A_NUM))
	{
		*result = NAN;
		return CONV_NOT_A_NUM;
	}

	if (UNLIKELY(CONV_HAS_OVERFLOW(components.flags)))
	{
		*result = components.sign > 0 ? INFINITY: -INFINITY;
		return components.flags;
	}

	// check if we'll be able to fit our digits in double
	if (components.fraction >= (int64_t)1<<DBL_MANT_DIG ||
	         components.exponent < DBL_MIN_EXP)
		components.flags |= CONV_PRECISION_LOSS;

	// note that original code used two doubles which is equal to long double
	*result = components.fraction * powl(10.0, components.exponent) * components.sign;
	if (isinf(*result))
	{
		components.flags = (components.sign > 0) ? CONV_POSITIVE_OVERFLOW : CONV_NEGATIVE_OVERFLOW;
	}
	return components.flags;
}

ConversionResultFlags jdouble_to_i32(double value, int32_t *result)
{
	CHECK_POINTER_RETURN_VALUE(result, CONV_BAD_ARGS);
	
	if (isnan(value) != 0) {
		PJ_LOG_WARN("attempting to convert nan to int");
		*result = 0;
		return CONV_NOT_A_NUM;
	}

	switch (isinf(value)) {
		case 0:
			break;
		case 1:
			PJ_LOG_WARN("attempting to convert +infinity to int");
			*result = PJSON_MAX_INT;
			return CONV_POSITIVE_INFINITY;
		case -1:
			PJ_LOG_WARN("attempting to convert -infinity to int");
			*result = PJSON_MIN_INT;
			return CONV_NEGATIVE_INFINITY;
		default:
			PJ_LOG_ERR("unknown result from isinf for %lf", value);
			return CONV_GENERIC_ERROR;
	}

	if (value > PJSON_MAX_INT) {
		PJ_LOG_WARN("attempting to convert double %lf outside of int range", value);
		*result = PJSON_MAX_INT;
		return CONV_POSITIVE_OVERFLOW;
	}

	if (value < PJSON_MIN_INT) {
		PJ_LOG_WARN("attempting to convert double %lf outside of int range", value);
		*result = PJSON_MIN_INT;
		return CONV_NEGATIVE_OVERFLOW;
	}

	*result = (int32_t) value;
	if (fabs(*result - value) > 1e-9) {
		PJ_LOG_INFO("conversion of double %lf results in integer with different value", value);
		return CONV_PRECISION_LOSS;
	}

	return CONV_OK;
}

ConversionResultFlags jdouble_to_i64(double value, int64_t *result)
{
	CHECK_POINTER_RETURN_VALUE(result, CONV_BAD_ARGS);
	
	if (isnan(value) != 0) {
		PJ_LOG_WARN("attempting to convert nan to int64");
		*result = 0;
		return CONV_NOT_A_NUM;
	}

	switch (isinf(value)) {
		case 0:
			break;
		case 1:
			PJ_LOG_WARN("attempting to convert +infinity to int");
			*result = PJSON_MAX_INT64;
			return CONV_POSITIVE_INFINITY;
		case -1:
			PJ_LOG_WARN("attempting to convert -infinity to int");
			*result = PJSON_MIN_INT64;
			return CONV_NEGATIVE_INFINITY;
		default:
			PJ_LOG_ERR("unknown result from isinf for %lf", value);
			return CONV_GENERIC_ERROR;
	}

	if (value > PJSON_MAX_INT64) {
		PJ_LOG_WARN("attempting to convert double %lf outside of int64 range", value);
		*result = PJSON_MAX_INT64;
		return CONV_POSITIVE_OVERFLOW;
	}

	if (value < PJSON_MIN_INT64) {
		PJ_LOG_WARN("attempting to convert double %lf outside of int64 range", value);
		*result = PJSON_MIN_INT64;
		return CONV_NEGATIVE_OVERFLOW;
	}

	if (value > PJSON_MAX_INT_IN_DBL || value < PJSON_MIN_INT_IN_DBL) {
		PJ_LOG_INFO("conversion of double %lf to integer potentially has precision loss", value);
		*result = (int64_t)value;
		return CONV_PRECISION_LOSS;
	}

	*result = (int64_t) value;
	if (fabs(*result - value) > 1e-9) {
		PJ_LOG_INFO("conversion of double %lf results in integer with different value", value);
		return CONV_PRECISION_LOSS;
	}
	return CONV_OK;
}

ConversionResultFlags ji32_to_i64(int32_t value, int64_t *result)
{
	CHECK_POINTER_RETURN_VALUE(result, CONV_BAD_ARGS);
	*result = value;
	return CONV_OK;
}

ConversionResultFlags ji32_to_double(int32_t value, double *result)
{
	CHECK_POINTER_RETURN_VALUE(result, CONV_BAD_ARGS);
	*result = value;
	return CONV_OK;
}

ConversionResultFlags ji64_to_i32(int64_t value, int32_t *result)
{
	if (value > PJSON_MAX_INT) {
		PJ_LOG_WARN("overflow converting %"PRId64 " to int32", value);
		*result = PJSON_MAX_INT;
		return CONV_POSITIVE_OVERFLOW;
	}
	if (value < PJSON_MIN_INT) {
		PJ_LOG_WARN("overflow converting %"PRId64 " to int32", value);
		*result = PJSON_MIN_INT;
		return CONV_NEGATIVE_OVERFLOW;
	}
	*result = (int32_t) value;
	return CONV_OK;
}

ConversionResultFlags ji64_to_double(int64_t value, double *result)
{
	CHECK_POINTER_RETURN_VALUE(result, CONV_BAD_ARGS);
	if (value > PJSON_MAX_INT_IN_DBL || value < PJSON_MIN_INT_IN_DBL) {
		PJ_LOG_INFO("conversion of integer %"PRId64 " to a double will result in precision loss when doing reverse", value);
		*result = (double)value;
		return CONV_PRECISION_LOSS;
	}
	*result = (double)value;
	return CONV_OK;
}
