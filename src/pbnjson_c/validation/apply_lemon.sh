#!/bin/bash

# Copyright (c) 2009-2018 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

STRIP_LINES=false
STRIP_FILE=

OPTIND=1
while getopts l: opt; do
	case "$opt" in
		l) STRIP_LINES=true
		   STRIP_FILE="$OPTARG" ;;
	esac
done
shift $((OPTIND-1))
[[ "$1" = '--' ]] && shift

lemon="$1"; shift
binary_dir="$(readlink -f "$1")"; shift
source_dir="$(readlink -f "$1")"; shift
file="$1"; shift

keyword_pattern="KEY_[0-9A-Za-z_]*"

found_keywords=`grep -v any_object_key $source_dir/$file | grep -o "$keyword_pattern" | sort -u`
all_keywords=`grep any_object_key $source_dir/$file | grep -o "$keyword_pattern" | sort -u`

expected_tokens=`echo "$all_keywords" | grep -v NOT_KEYWORD`
token_keywords=`grep TOKEN $source_dir/schema_keywords.gperf | grep -o "TOKEN_${keyword_pattern}" | \
	sed 's/TOKEN_//' | sort -u`

if [ "$found_keywords" != "$all_keywords" ]; then
	echo >&2 "any_object_key doesn't contain all the keywords"
	diff <(echo "$found_keywords") <(echo "$all_keywords")
	exit 1
fi

if [ "$expected_tokens" != "$token_keywords" ]; then
	echo >&2 "schema_keywords.gperf doesn't contain all the keywords"
	diff <(echo "$token_keywords") <(echo "$expected_tokens")
	exit 1
fi

if [ "$source_dir" != "$binary_dir" ]; then
	ln -sf "$source_dir/$file" "$binary_dir/" || \
		cp -af "$source_dir/$file" "$binary_dir/" || \
		exit 1
fi
"$lemon" "$@" "$binary_dir/$file" || exit 1
if [ "$STRIP_LINES" = "true" ]; then
	sed -i -e '/^#line/d' "$STRIP_FILE"
fi
