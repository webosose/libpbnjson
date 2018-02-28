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

#ifndef __JQUERY_GEN_DECLARATIONS_H_
#define __JQUERY_GEN_DECLARATIONS_H_

// Lemon parser functions
void *JQueryParseAlloc(void *(*mallocProc)(size_t));
void JQueryParseFree(void *p, void (*freeProc)(void*));
void JQueryParse(void *yyp, int yymajor, raw_buffer yyminor, jq_parser_context *context);
//void JQueryParseTrace(FILE *TraceFILE, char *zTracePrompt);

// Flex lexer functions
typedef void* yyscan_t;
int JQueryScan_lex_init (yyscan_t *scanner);
int JQueryScan_lex (yyscan_t scanner);
int JQueryScan_lex_destroy (yyscan_t scanner);
void *JQueryScan__scan_string (const char *str, yyscan_t scanner);
char *JQueryScan_get_text(yyscan_t scanner);
int JQueryScan_get_leng(yyscan_t yyscanner);
void JQueryScan_set_extra (void* user_defined, yyscan_t yyscanner);

#endif //__JQUERY_GEN_DECLARATIONS_H_
