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

#ifndef JNUM_CONVERSION_H_
#define JNUM_CONVERSION_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NOTE: All the values must represent binary flags or be
 * defined in terms of a binary OR of other values.
 */
typedef enum {
	/// - conversion is ok
	CONV_OK = 0,
	/// Used to limit the value to the largest which can be stored in an integer. for doubles, it will be positive infinity
	CONV_POSITIVE_OVERFLOW = 0x1,
	/// Used to limit the value to the minimum which can be stored in an integer. for doubles, it will be negative infinity
	CONV_NEGATIVE_OVERFLOW = 0x2,
	/* CONV_POSITIVE_OVERFLOW | CONV_NEGATIVE_OVERFLOW */
	/// @brief Used to limit the value to the largest and the minimum, which can store in number.
	///
	/// Used to limit the value to the largest and the minimum, which can store in number.
#define CONV_OVERFLOW (CONV_POSITIVE_OVERFLOW | CONV_NEGATIVE_OVERFLOW)
	///  Used to limit the value to the largest which can be stored in an integer
	CONV_INFINITY = 0x4,
	/// @brief Used to limit the value to the largest which can stored in the number
	///
	/// Used to limit the value to the largest which can stored in the number
#define CONV_POSITIVE_INFINITY (CONV_POSITIVE_OVERFLOW | CONV_INFINITY)
	/// @brief Used to limit the value to the minimum which can store in the number
	///
	/// Used to limit the value to the minimum which can be stored in the number
#define CONV_NEGATIVE_INFINITY (CONV_NEGATIVE_OVERFLOW | CONV_INFINITY)
	/// bit is set if a double is requested but the int cannot be represented perfectly or an int is requested but the double has floating point
	CONV_PRECISION_LOSS = 0x8,
	/// not a numeric data
	CONV_NOT_A_NUM = 0x10,
	/// returned if the type is not a string - the raw string representation is still returned as appropriate
	CONV_NOT_A_STRING = 0x20,
	/// returned if the type is not a boolean - the value written is always the boolean approximation
	CONV_NOT_A_BOOLEAN = 0x40,
	/// if an attempt is made to get the raw number from a JSON Number backed by a numeric primitive
	CONV_NOT_A_RAW_NUM = 0x80,
	/// if the provided arguments are bogus (MUST NOT OVERLAP WITH ANY OTHER ERROR CODES)
	CONV_BAD_ARGS = 0x40000000,
	/// if some other unspecified error occurred (MUST NOT OVERLAP WITH ANY OTHER ERROR CODES)
	CONV_GENERIC_ERROR = 0x80000000
} ConversionResult;

/**
 * \typedef ConversionResultFlags
 * Set of conversion result flags
 */
typedef unsigned int ConversionResultFlags;

/// @brief Check if return_code has positive overflow
///
/// Check if return_code has positive overflow
#define CONV_HAS_POSITIVE_OVERFLOW(return_code)  ((return_code) & CONV_POSITIVE_OVERFLOW)

/// @brief Check if return_code has negative overflow
///
/// Check if return_code has negative overflow
#define CONV_HAS_NEGATIVE_OVERFLOW(return_code)  ((return_code) & CONV_NEGATIVE_OVERFLOW)

/// @brief Check if return_code has overflow
///
/// Check if return_code has overflow
#define CONV_HAS_OVERFLOW(return_code)       (((return_code) & CONV_OVERFLOW) != 0)

/// @brief Check if return_code has infinity
///
/// Check if return_code has infinity
#define CONV_HAS_INFINITY(return_code)       (((return_code) & CONV_INFINITY) != 0)

/// @brief Check if return_code has positive infinity
///
/// Check if return_code has positive infinity
#define CONV_HAS_POSITIVE_INFINITY(return_code) ((return_code) & CONV_POSITIVE_INFINITY)

/// @brief Check if return_code has negative infinity
///
/// Check if return_code has negative infinity
#define CONV_HAS_NEGATIVE_INFINITY(return_code) ((return_code) & CONV_NEGATIVE_INFINITY)

/// @brief Check if return_code has CONV_PRECISION_LOSS
///
/// Check if return_code has CONV_PRECISION_LOSS
#define CONV_HAS_PRECISION_LOSS(return_code) (((return_code) & CONV_PRECISION_LOSS) != 0)

/// @brief Check if return_code has not a numeric data
///
/// Check if return_code has not a numeric data
#define CONV_HAS_NOT_A_NUM(return_code)      (((return_code) & CONV_NOT_A_NUM) != 0)

/// @brief Check if return_code has the provided arguments are bogus
///
/// Check if return_code has the provided arguments are bogus
#define CONV_IS_BAD_ARGS(return_code)        ((return_code) & CONV_BAD_ARGS)

/// @brief Check if return_code has some other unspecified error
///
/// Check if return_code has some other unspecified error
#define CONV_IS_GENERIC_ERROR(return_code)   ((return_code) & CONV_GENERIC_ERROR)

#ifdef __cplusplus
}
#endif

#endif /* JNUM_CONVERSION_H_ */
