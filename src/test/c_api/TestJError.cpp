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

#include <gtest/gtest.h>
#include <pbnjson.h>

extern "C" {
#include "../../pbnjson_c/jerror_internal.h"
}

TEST(JError, JErrorFree)
{
	jerror *err = NULL;
	jerror_free(err);
	EXPECT_FALSE(err);

	jerror_set(&err, JERROR_TYPE_INTERNAL, "OOM");
	EXPECT_TRUE(err);
	jerror_free(err);
	EXPECT_TRUE(err);
}

TEST(JError, JErrorSet)
{
	jerror *err = NULL;
	char buf[24];

	jerror_set(&err, JERROR_TYPE_INTERNAL, "123");
	int len = jerror_to_string(err, buf, 24);
	EXPECT_TRUE(len == 19);
	EXPECT_FALSE(strcmp(buf, "Internal error. 123"));
	jerror_free(err);
	err = NULL;
	memset(buf, 0, 24);

	jerror_set(&err, JERROR_TYPE_INTERNAL, "123456789");
	EXPECT_TRUE(err);
	len = jerror_to_string(err, buf, 24);
	EXPECT_TRUE(len == 25);
	EXPECT_FALSE(strcmp(buf, "Internal error. 1234567"));

	jerror_free(err);
}

TEST(JError, JErrorSetFormatted)
{
	jerror *err = NULL;
	char buf[24];

	jerror_set_formatted(&err, JERROR_TYPE_INTERNAL, "%d", 123);
	int len = jerror_to_string(err, buf, 24);
	EXPECT_TRUE(len == 19);
	EXPECT_FALSE(strcmp(buf, "Internal error. 123"));
	jerror_free(err);
	err = NULL;
	memset(buf, 0, 24);

	jerror_set_formatted(&err, JERROR_TYPE_INTERNAL, "%d", 123456789);
	EXPECT_TRUE(err);
	len = jerror_to_string(err, buf, 24);
	EXPECT_TRUE(len == 25);
	EXPECT_FALSE(strcmp(buf, "Internal error. 1234567"));

	jerror_free(err);
}
