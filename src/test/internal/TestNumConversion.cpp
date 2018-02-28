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

#include "jvalue/num_conversion.h"
#include "jobject.h"

#include <math.h>

#include <gtest/gtest.h>

namespace {
	const char veryLargeNumber[] = "645458489754321564894654151561684894456464513215648946543132189489461321684.2345646544e509";

	void expectNumberParse(number_components expected, raw_buffer str)
	{
		SCOPED_TRACE("numberParse() input: " + std::string(str.m_str, str.m_len));
		number_components components;
		numberParse(&components, str);
		EXPECT_EQ(expected.flags, components.flags);
		if (expected.flags != CONV_NOT_A_NUM)
		{
			EXPECT_EQ(expected.fraction, components.fraction);
			EXPECT_EQ(expected.exponent, components.exponent);
			EXPECT_EQ(expected.sign, components.sign);
		}
	}
}

TEST(TestNumConversion, numberParse)
{
	expectNumberParse({5, 0, 1, CONV_OK}, J_CSTR_TO_BUF("5"));
	expectNumberParse({42, 0, 1, CONV_OK}, J_CSTR_TO_BUF("42"));
	expectNumberParse({42, 0, -1, CONV_OK}, J_CSTR_TO_BUF("-42"));
	expectNumberParse({42, 0, 1, CONV_OK}, J_CSTR_TO_BUF("+42"));
	expectNumberParse({42, 0, 1, CONV_OK}, J_CSTR_TO_BUF("042"));
	expectNumberParse({0, 0, 0, CONV_NOT_A_NUM}, J_CSTR_TO_BUF("-"));
	expectNumberParse({0, 0, 0, CONV_NOT_A_NUM}, J_CSTR_TO_BUF("+"));
	expectNumberParse({5, -1, 1, CONV_OK}, J_CSTR_TO_BUF("0.5"));
	expectNumberParse({5, -2, 1, CONV_OK}, J_CSTR_TO_BUF("0.05"));
	expectNumberParse({5, -1, 1, CONV_OK}, J_CSTR_TO_BUF("0.50"));
	expectNumberParse({5, -1, 1, CONV_OK}, J_CSTR_TO_BUF("0.500000000000000000000000000000"));
	expectNumberParse({5, -1, 1, CONV_PRECISION_LOSS}, J_CSTR_TO_BUF("0.500000000000000000000000000001"));
	expectNumberParse({5, 1, 1, CONV_OK}, J_CSTR_TO_BUF("50"));
	expectNumberParse({5, 1, 1, CONV_OK}, J_CSTR_TO_BUF("50.0"));
	expectNumberParse({5, 30, 1, CONV_OK}, J_CSTR_TO_BUF("5000000000000000000000000000000"));
	expectNumberParse({5, 30, 1, CONV_PRECISION_LOSS}, J_CSTR_TO_BUF("5000000000000000000000000000001"));
	expectNumberParse({42, -5, 1, CONV_OK}, J_CSTR_TO_BUF("0.00042"));
	expectNumberParse({42, -5, 1, CONV_OK}, J_CSTR_TO_BUF("4.2e-4"));
	expectNumberParse({122037626, -6, -1, CONV_OK}, J_CSTR_TO_BUF("-122.037626"));
	expectNumberParse({1, 1000000000000000000, 1, CONV_OK}, J_CSTR_TO_BUF("1e1000000000000000000"));
	expectNumberParse({1, -1000000000000000000, 1, CONV_OK}, J_CSTR_TO_BUF("1e-1000000000000000000"));
	expectNumberParse({1, -1000000000000000000, -1, CONV_OK}, J_CSTR_TO_BUF("-1e-1000000000000000000"));
	expectNumberParse({UINT64_MAX, INT64_MAX, 1, CONV_POSITIVE_OVERFLOW},
	                  J_CSTR_TO_BUF("1e10000000000000000000"));
	expectNumberParse({UINT64_MAX, INT64_MAX, -1, CONV_NEGATIVE_OVERFLOW},
	                  J_CSTR_TO_BUF("-1e10000000000000000000"));
	expectNumberParse({0, 0, 1, CONV_PRECISION_LOSS}, J_CSTR_TO_BUF("1e-10000000000000000000"));
	expectNumberParse({0, 0, -1, CONV_PRECISION_LOSS}, J_CSTR_TO_BUF("-1e-10000000000000000000"));
	expectNumberParse({6454584897543215648u, 565, 1, CONV_PRECISION_LOSS}, J_CSTR_TO_BUF(veryLargeNumber));
	expectNumberParse({9223372036854775807, 0, 1, CONV_OK}, J_CSTR_TO_BUF("9223372036854775807"));
	expectNumberParse({9223372036854775807, 0, -1, CONV_OK}, J_CSTR_TO_BUF("-9223372036854775807"));
	expectNumberParse({9223372036854775808u, 0, 1, CONV_OK}, J_CSTR_TO_BUF("9223372036854775808"));
	expectNumberParse({9223372036854775808u, 0, -1, CONV_OK}, J_CSTR_TO_BUF("-9223372036854775808"));
	expectNumberParse({9223372036854775809u, 0, -1, CONV_OK}, J_CSTR_TO_BUF("-9223372036854775809"));
	expectNumberParse({18446744073709551615u, 0, 1, CONV_OK}, J_CSTR_TO_BUF("18446744073709551615"));
	expectNumberParse({1844674407370955161u, 1, 1, CONV_PRECISION_LOSS}, J_CSTR_TO_BUF("18446744073709551616"));
	expectNumberParse({1, 9223372036854775807, 1, CONV_OK}, J_CSTR_TO_BUF("1e9223372036854775807"));
	expectNumberParse({1, -9223372036854775807, 1, CONV_OK}, J_CSTR_TO_BUF("1e-9223372036854775807"));
	expectNumberParse({1, -9223372036854775807-1, 1, CONV_OK}, J_CSTR_TO_BUF("1e-9223372036854775808"));
	expectNumberParse({UINT64_MAX, INT64_MAX, 1, CONV_POSITIVE_OVERFLOW}, J_CSTR_TO_BUF("1e92233720368547758070"));
	expectNumberParse({UINT64_MAX, INT64_MAX, 1, CONV_POSITIVE_OVERFLOW}, J_CSTR_TO_BUF("1e9223372036854775808"));
	expectNumberParse({10596046447755, -13, 1, CONV_OK}, J_CSTR_TO_BUF("1.05960464477550000000"));
}

TEST(TestNumConversion, jstr_to_double)
{
	ASSERT_EQ(0.00042, 4.2e-4);

	double fval;
	raw_buffer buf = J_CSTR_TO_BUF("4.2e-4");
	EXPECT_EQ(CONV_OK, jstr_to_double(&buf, &fval));
	EXPECT_EQ(0.00042, fval);

	double fval_alt;
	buf = J_CSTR_TO_BUF("0.00042");
	EXPECT_EQ(CONV_OK, jstr_to_double(&buf, &fval_alt));
	EXPECT_EQ(4.2e-4, fval_alt);
	EXPECT_EQ(fval_alt, fval);

	buf = J_CSTR_TO_BUF("-122.037626");
	EXPECT_EQ(CONV_OK, jstr_to_double(&buf, &fval));
	EXPECT_EQ(-122.037626, fval);

	buf = J_CSTR_TO_BUF("1.0000000000000001");
	EXPECT_EQ(CONV_PRECISION_LOSS, jstr_to_double(&buf, &fval));
	EXPECT_EQ(1.0, fval);

	buf = J_CSTR_TO_BUF(veryLargeNumber);
	EXPECT_TRUE(CONV_HAS_POSITIVE_OVERFLOW(jstr_to_double(&buf, &fval)));
	EXPECT_EQ(INFINITY, fval);

	buf = J_CSTR_TO_BUF("-9223372036854775808");
	EXPECT_EQ(CONV_PRECISION_LOSS, jstr_to_double(&buf, &fval));
	EXPECT_EQ(-9223372036854775808.0, fval);

	std::string str;
	// Build a string of '1' (digit one) followed by 308 zeroes ('0') that
	// should be equivalent to 1e+308
	str.resize(309, '0');
	str[0] = '1';
	buf = {str.data(), str.size()};
	EXPECT_EQ(CONV_OK, jstr_to_double(&buf, &fval));
	EXPECT_EQ(1e308, fval);
}

TEST(TestNumConversion, jstr_to_i64)
{
	int64_t ival;
	int32_t i32val;
	raw_buffer buf = J_CSTR_TO_BUF("1.0e0");
	EXPECT_EQ(CONV_OK, jstr_to_i64(&buf, &ival));
	EXPECT_EQ(1, ival);

	/*
	buf = J_CSTR_TO_BUF("0.5");
	EXPECT_EQ(CONV_PRECISION_LOSS, jstr_to_i64(&buf, &ival));
	EXPECT_EQ(1, ival) << "should round 0.5 to 1";
	*/

	buf = J_CSTR_TO_BUF("18446744073709551616");
	EXPECT_EQ(CONV_POSITIVE_OVERFLOW, jstr_to_i64(&buf, &ival));
	EXPECT_EQ(INT64_MAX, ival);

	buf = J_CSTR_TO_BUF("9223372036854775807");
	EXPECT_EQ(CONV_OK, jstr_to_i64(&buf, &ival));
	EXPECT_EQ(9223372036854775807, ival);

	buf = J_CSTR_TO_BUF("-9223372036854775808");
	EXPECT_EQ(CONV_OK, jstr_to_i64(&buf, &ival));
	EXPECT_EQ(-9223372036854775807-1, ival);

	buf = J_CSTR_TO_BUF("9223372036854775808");
	EXPECT_EQ(CONV_POSITIVE_OVERFLOW, jstr_to_i64(&buf, &ival));
	EXPECT_EQ(INT64_MAX, ival);

	buf = J_CSTR_TO_BUF("1.05960464477550000000");
	EXPECT_EQ(CONV_PRECISION_LOSS, jstr_to_i64(&buf, &ival));
	EXPECT_EQ(1, ival);

	buf = J_CSTR_TO_BUF("1.05960464477550000000");
	EXPECT_EQ(CONV_PRECISION_LOSS, jstr_to_i32(&buf, &i32val));
	EXPECT_EQ(1, i32val);

	buf = J_CSTR_TO_BUF("-2147483648");
	EXPECT_EQ(CONV_OK, jstr_to_i32(&buf, &i32val));
	EXPECT_EQ(-2147483648, i32val);

	buf = J_CSTR_TO_BUF("-2147483649");
	EXPECT_EQ(CONV_NEGATIVE_OVERFLOW, jstr_to_i32(&buf, &i32val));
	EXPECT_EQ(-2147483648, i32val);

	buf = J_CSTR_TO_BUF("2147483647");
	EXPECT_EQ(CONV_OK, jstr_to_i32(&buf, &i32val));
	EXPECT_EQ(2147483647, i32val);

	buf = J_CSTR_TO_BUF("2147483648");
	EXPECT_EQ(CONV_POSITIVE_OVERFLOW, jstr_to_i32(&buf, &i32val));
	EXPECT_EQ(2147483647, i32val);
}
