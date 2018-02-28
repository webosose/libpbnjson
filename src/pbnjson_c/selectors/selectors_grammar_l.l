%{
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

#include "glib.h"

#include "selectors_grammar_y.h"
#include "../jerror_internal.h"
%}

%x VAL EXPR STRING
%option reentrant noyywrap stack
%option prefix="JQueryScan_"

chars [A-Za-z_0-9\-]
integer [0-9]+
fraction "."{integer}
number "-"?{integer}{fraction}?
int_number "-"?{integer}
ows " "*

%%
{ows}","{ows} return TOKEN_COMMA;
{ows}">"{ows} return TOKEN_COMBINATOR_PARENT;
{ows}"~"{ows} return TOKEN_COMBINATOR_SIBLINGS;
" "+          return TOKEN_COMBINATOR_ANCESTOR;
"." return TOKEN_DOT;
"*" return TOKEN_ASTERISK;
":" return TOKEN_COLON;
"(" return TOKEN_LPAREN;
")" return TOKEN_RPAREN;
<INITIAL,VAL,EXPR>"\"" {
    yy_push_state(STRING, yyscanner);
}

"has" return TOKEN_KEYWORD_HAS;
"val" {
    yy_push_state(VAL, yyscanner);
    return TOKEN_KEYWORD_VAL;
}
"expr" {
    yy_push_state(EXPR, yyscanner);
    return TOKEN_KEYWORD_EXPR;
}
"contains" return TOKEN_KEYWORD_CONTAINS;

"root"        return TOKEN_PSEUDO_CLASSNAME_ROOT;
"first-child" return TOKEN_PSEUDO_CLASSNAME_FIRST_CHILD;
"last-child"  return TOKEN_PSEUDO_CLASSNAME_LAST_CHILD;
"only-child"  return TOKEN_PSEUDO_CLASSNAME_ONLY_CHILD;
"empty"       return TOKEN_PSEUDO_CLASSNAME_EMPTY;

"nth-child"      return TOKEN_FUNCTION_NTH_CHILD;
"nth-last-child" return TOKEN_FUNCTION_NTH_LAST_CHILD;

"object"  return TOKEN_TYPE_OBJECT;
"array"   return TOKEN_TYPE_ARRAY;
"number"  return TOKEN_TYPE_NUMBER;
"string"  return TOKEN_TYPE_STRING;
"boolean" return TOKEN_TYPE_BOOLEAN;
"null"    return TOKEN_TYPE_NULL;

<VAL,EXPR>
{
    " "+
    "(" return TOKEN_LPAREN;
    ")" {
        yy_pop_state(yyscanner);
        return TOKEN_RPAREN;
    }
    "true"  return TOKEN_TRUE;
    "false" return TOKEN_FALSE;
    "null"  return TOKEN_NULL;
}

<EXPR>
{
    "&&"    return TOKEN_AND;
    "||"    return TOKEN_OR;
    "<"     return TOKEN_LESS;
    ">"     return TOKEN_GREATER;
    "<="    return TOKEN_LEQUAL;
    ">="    return TOKEN_GEQUAL;
    "="     return TOKEN_EQUAL;
    "!="    return TOKEN_NOTEQUAL;
    "x"     return TOKEN_X;
}

<INITIAL,VAL,EXPR>{int_number} return TOKEN_JSON_INT_NUMBER;
<INITIAL,VAL,EXPR>{number}     return TOKEN_JSON_NUMBER;
<INITIAL,VAL,EXPR>{chars}+     return TOKEN_JSON_STRING;

<STRING>
{
    ([[:print:]]{-}[\"])* {
        return TOKEN_JSON_STRING;
    }
    "\"" {
        yy_pop_state(yyscanner);
    }
}

<INITIAL,VAL,EXPR,STRING>. {
    char *symbol = g_strescape(yyget_text(yyscanner), NULL);

    jerror_set_formatted(yyget_extra(yyscanner),
                         JERROR_TYPE_SYNTAX,
                         "Unexpected symbol '%s' in the query string",
                         symbol);
    g_free(symbol);

    return 0;
}
<<EOF>>  return 0;
