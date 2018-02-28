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

#ifndef SRC_PBNJSON_C_JERROR_INTERNAL_H_
#define SRC_PBNJSON_C_JERROR_INTERNAL_H_

#include <compiler/format_attribute.h>

typedef enum {
	JERROR_TYPE_SCHEMA = 0,
	JERROR_TYPE_VALIDATION,
	JERROR_TYPE_SYNTAX,
	JERROR_TYPE_INTERNAL,
	JERROR_TYPE_INVALID_PARAMETERS
} jerror_type;

typedef struct jerror {
	jerror_type type;
	char        *message;
} jerror;

void jerror_set(jerror **error, jerror_type type, const char *str);
void jerror_set_formatted(jerror **err, jerror_type type, const char *format, ...)
	PRINTF_FORMAT_FUNC(3, 4);

#endif /* SRC_PBNJSON_C_JERROR_INTERNAL_H_ */
