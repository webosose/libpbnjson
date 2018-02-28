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

#include <glib.h>
#include <stdio.h>

#include "jerror.h"
#include "jerror_internal.h"

static const char *error_type_str[] = {
	[JERROR_TYPE_SCHEMA]             = "Schema",
	[JERROR_TYPE_VALIDATION]         = "Validation",
	[JERROR_TYPE_SYNTAX]             = "Syntax",
	[JERROR_TYPE_INTERNAL]           = "Internal",
	[JERROR_TYPE_INVALID_PARAMETERS] = "Invalid parameters"
};

jerror *jerror_duplicate(const jerror *other)
{
	jerror *copy = NULL;
	if (other)
	{
		copy = g_slice_new(jerror);
		copy->type = other->type;
		copy->message = g_strdup(other->message);
	}
	return copy;
}

void jerror_free(jerror *err)
{
	if (err) {
		g_free(err->message);
		g_slice_free(jerror, err);
	}
}

int jerror_to_string(jerror *err, char *str, size_t size)
{
	if (!err) return -1;

	return snprintf(str, size, "%s error. %s", error_type_str[err->type], err->message);
}

/******************************************************************************
 * Internal jerror functions
 *****************************************************************************/

static jerror *jerror_new(jerror_type type, const char *str)
{
	jerror *err = g_slice_new(jerror);
	err->type = type;
	err->message = g_strdup(str);
	return err;
}

/**
 * Function to set the jerror.
 *
 * @param err  pbnjson error information.
 * @param id   id of the error.
 * @param type jerror type.
 * @param str  error message.
 */
void jerror_set(jerror **err, jerror_type type, const char *str)
{
	if (!err || *err) // we are not reporting errors or the first error has been reported already
		return;

	*err = jerror_new(type, str);
}

/**
 * Function to set the jerror with formatted message.
 *
 * @param err    pbnjson error information.
 * @param id     id of the error.
 * @param type   jerror type.
 * @param format error message format.
 * @param ...    arguments.
 */
void jerror_set_formatted(jerror **err, jerror_type type, const char *format, ...)
{
	if (!err || *err)
		return;

	va_list args;
	va_start (args, format);

	*err = jerror_new(type, NULL);
	(*err)->message = g_strdup_vprintf(format, args);

	va_end (args);
}
