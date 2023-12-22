/****************************************************************
 *
 * Copyright (c) 2015-2023 LG Electronics, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************/

%token_prefix TOKEN_
%left COMMA.
%left DOT.
%left COMBINATOR_PARENT COMBINATOR_SIBLINGS COMBINATOR_ANCESTOR.
%left OR.
%left AND.
%nonassoc LESS GREATER LEQUAL GEQUAL EQUAL NOTEQUAL.

%name JQueryParse
%start_symbol root
%extra_argument { jq_parser_context *context }
%token_type { raw_buffer }

%include {
#include <assert.h>
#include <stdio.h>

#include <glib.h>

#include "../jerror_internal.h"
#include "../jobject_internal.h"
#include "../jvalue/num_conversion.h"
#include "jquery_internal.h"
#include "expression.h"
}

%syntax_error {
    if (!TOKEN.m_str)
    {
        jerror_set(context->error, JERROR_TYPE_SYNTAX,
                   "Unexpected end of the query string");
    }
    else
    {
        jerror_set_formatted(context->error, JERROR_TYPE_SYNTAX,
                             "Unexpected token '%s' in the query string",
                             TOKEN.m_str);
    }
}


/* TODO: Set root query to A, when it will be properly
 * created for all non-terminals.
 */
root ::= query(B).
{
    context->root_pair = B;
}

%type query { jquery_pair }
%destructor query { jquery_free($$.deepest_query); }
query(A) ::= selectors_group(B).
{
    A = B;
}

// Selectors sequence.
%type selectors_group { jquery_pair }
%destructor selectors_group { jquery_free($$.deepest_query); }
selectors_group(A) ::= selector(B).
{
    A = B;
}
selectors_group(A) ::= selectors_group(B) COMMA selector(C).
{
    jquery_pair_ptr pair = g_new(struct __query_pair, 1);
    pair->first = B.deepest_query;
    pair->second = C.deepest_query;

    A = (jquery_pair){ .root_query = jquery_new(selector_or,
                                                pair,
                                                (query_context_destructor) jquery_pair_ptr_free,
                                                JQG_TYPE_SELF) };
    A.deepest_query = A.root_query;
}

%type selector { jquery_pair }
%destructor selector { jquery_free($$.deepest_query); }
selector(A) ::= simple_selector_sequence(B).
{
    A = B;
}
/* Parent and ancestor combinators implemented not optimally from
 * the root point of view. We could use left-to-right query chaining,
 * to filter some values gradually, but in this case, the combinators
 * will be not able to work with single values, which has itself as an
 * entry point, so generators can't generate the value from ancestor or
 * parent JSON node. Consequently, we work with parent links of *every*
 * incoming JSON value. */
// Whitespace
selector(A) ::= selector(B) COMBINATOR_ANCESTOR simple_selector_sequence(C).
{
    A = (jquery_pair){ .root_query = C.root_query,
                       .deepest_query = jquery_new(selector_ancestor, B.deepest_query,
                                                   (query_context_destructor)jquery_free,
                                                   JQG_TYPE_SELF) };
    A.deepest_query->parent_query = C.deepest_query;
}
// Greater
selector(A) ::= selector(B) COMBINATOR_PARENT simple_selector_sequence(C).
{
    A = (jquery_pair){ .root_query = C.root_query,
                       .deepest_query = jquery_new(selector_parent, B.deepest_query,
                                                   (query_context_destructor)jquery_free,
                                                   JQG_TYPE_SELF) };
    A.deepest_query->parent_query = C.deepest_query;
}
// Tilda
selector(A) ::= selector(B) COMBINATOR_SIBLINGS simple_selector_sequence(C).
{
    A = (jquery_pair){ .root_query = C.root_query,
                       .deepest_query = jquery_new(selector_sibling, B.deepest_query,
                                                   (query_context_destructor)jquery_free,
                                                   JQG_TYPE_SELF) };
    A.deepest_query->parent_query = C.deepest_query;
}

// Element with type, or without
%type simple_selector_sequence { jquery_pair }
%destructor simple_selector_sequence { jquery_free($$.deepest_query); }
// Note reverted order. Type selector pull key selector, not vice versa.
simple_selector_sequence(A) ::= type(B) key(C).
{
    A = (jquery_pair){ .root_query = C.root_query, .deepest_query = jquery_new(selector_type, (void *)B, NULL, JQG_TYPE_SELF) };
    A.deepest_query->parent_query = C.root_query;
}
simple_selector_sequence(A) ::= type(B).
{
    A = (jquery_pair){ .root_query = jquery_new(selector_type, (void *)B, NULL, JQG_TYPE_SELF) };
    A.deepest_query = A.root_query;
}
simple_selector_sequence(A) ::= key(B).
{
    A = B;
}

// Values types
%type type { JValueType }
type(A) ::= TYPE_OBJECT. { A = JV_OBJECT; }
type(A) ::= TYPE_ARRAY. { A = JV_ARRAY; }
type(A) ::= TYPE_NUMBER. { A = JV_NUM; }
type(A) ::= TYPE_STRING. { A = JV_STR; }
type(A) ::= TYPE_BOOLEAN. { A = JV_BOOL; }
type(A) ::= TYPE_NULL. { A = JV_NULL; }

// Values
%type key { jquery_pair }
%destructor key { jquery_free($$.deepest_query); }
key(A) ::= class(B).
{
    A = B;
}
key(A) ::= class(B) COLON pseudo(C).
{
    // TODO: remove condition, when all pseudo will be implemented
    if (NULL != C.root_query)
    {
        B.deepest_query = C.deepest_query;
        C.root_query->parent_query = B.root_query;
    }
    A = B;
}
key(A) ::= COLON pseudo(B).
{
    A = B;
}

/* TODO: Do we need incompatible with json_string names?
 * class ::= DOT name. */
%type class { jquery_pair }
%destructor class { jquery_free($$.deepest_query); }
class(A) ::= DOT json_string(B).
{
    A = (jquery_pair){ .root_query = jquery_new(selector_key, B, g_free, JQG_TYPE_SELF) };
    A.deepest_query = A.root_query;
}
class(A) ::= ASTERISK.
{
    A = (jquery_pair){ .root_query = jquery_new(selector_all, NULL, NULL, JQG_TYPE_SELF) };
    A.deepest_query = A.root_query;
}

/* Note that pseudo-elements are restricted to one per selector and
 * occur only in the last simple_selector_sequence.
 */
%type pseudo { jquery_pair }
%destructor pseudo { jquery_free($$.deepest_query); }
pseudo(A) ::= pseudo_class_name(B).
{
    A = B;
}
pseudo(A) ::= FUNCTION_NTH_CHILD LPAREN array_index(B) RPAREN.
{
    A = (jquery_pair){ .root_query = jquery_new(selector_nth_child, GINT_TO_POINTER(B), NULL, JQG_TYPE_SELF) };
    A.deepest_query = A.root_query;
}
pseudo(A) ::= FUNCTION_NTH_LAST_CHILD LPAREN array_index(B) RPAREN.
{
    A = (jquery_pair){ .root_query = jquery_new(selector_nth_child, GINT_TO_POINTER(-B), NULL, JQG_TYPE_SELF) };
    A.deepest_query = A.root_query;
}
pseudo(A) ::= KEYWORD_HAS LPAREN selectors_group(B) RPAREN.
{
    /* Since :has combinator checks recursively its element,
     * we need to add recursive generator, for our underlying
     * query */
    B.root_query->parent_query = jquery_new(selector_all, NULL, NULL, JQG_TYPE_RECURSIVE);
    A = (jquery_pair){
        .root_query = jquery_new(selector_has,
                                 B.deepest_query,
                                 (query_context_destructor) jquery_free,
                                 JQG_TYPE_SELF)
        };
    A.deepest_query = A.root_query;
}
pseudo(A) ::= KEYWORD_EXPR LPAREN expr(B) RPAREN.
{
    A = (jquery_pair){
        .root_query = jquery_new(selector_expr,
                                 B, (query_context_destructor) sel_ex_free,
                                 JQG_TYPE_RECURSIVE)
        };
    A.deepest_query = A.root_query;
}

pseudo(A) ::= KEYWORD_CONTAINS LPAREN json_string(B) RPAREN.
{
    A = (jquery_pair){ .root_query = jquery_new(selector_contains, B, g_free, JQG_TYPE_SELF) };
    A.deepest_query = A.root_query;
}
pseudo(A) ::= KEYWORD_VAL LPAREN val(B) RPAREN.
{
    A = (jquery_pair){ .root_query = jquery_new(selector_value, B,
                                                (query_context_destructor) j_release_helper,
                                                JQG_TYPE_SELF) };
    A.deepest_query = A.root_query;
}

%type pseudo_class_name { jquery_pair }
%destructor pseudo_class_name { jquery_free($$.deepest_query); }
pseudo_class_name(A) ::= PSEUDO_CLASSNAME_ROOT.
{
    A = (jquery_pair){ .root_query = jquery_new(selector_root, NULL, NULL, JQG_TYPE_SELF) };
    A.deepest_query = A.root_query;
}
pseudo_class_name(A) ::= PSEUDO_CLASSNAME_FIRST_CHILD.
{
    A = (jquery_pair){ .root_query = jquery_new(selector_nth_child, GINT_TO_POINTER(1), NULL, JQG_TYPE_SELF) };
    A.deepest_query = A.root_query;
}
pseudo_class_name(A) ::= PSEUDO_CLASSNAME_LAST_CHILD.
{
    A = (jquery_pair){ .root_query = jquery_new(selector_nth_child, GINT_TO_POINTER(-1), NULL, JQG_TYPE_SELF) };
    A.deepest_query = A.root_query;
}
pseudo_class_name(A) ::= PSEUDO_CLASSNAME_ONLY_CHILD.
{
    A = (jquery_pair){ .root_query = jquery_new(selector_only_child, NULL, NULL, JQG_TYPE_SELF) };
    A.deepest_query = A.root_query;
}
pseudo_class_name(A) ::= PSEUDO_CLASSNAME_EMPTY.
{
    A = (jquery_pair){ .root_query = jquery_new(selector_empty, NULL, NULL, JQG_TYPE_SELF) };
    A.deepest_query = A.root_query;
}

// Internal ':expr' expressions
%type expr { SelEx * }
%destructor expr { sel_ex_free($$); }

expr(A) ::= expr(B) AND expr(C).
{
    A = sel_ex_binop(B, BOP_AND, C);
}

expr(A) ::= expr(B) OR expr(C).
{
    A = sel_ex_binop(B, BOP_OR, C);
}

expr(A) ::= expr(B) binop(C) val_ex(D).
{
    A = sel_ex_binop(B, C, D);
}

expr(A) ::= LPAREN expr(B) RPAREN.
{
    A = B;
}

expr(A) ::= val_ex(B).
{
    A = B;
}

%type val_ex { SelEx * }
%destructor val_ex { sel_ex_free($$); }

val_ex(A) ::= val(B).
{
    A = sel_ex_jvalue(B);
}

val_ex(A) ::= X. // Input of an expression
{
    A = sel_ex_x();
}

/* Internal binary operations (Should be consistent with c++ ops?)
 * Original operators:
 * '*' | '/' | '%' | '+' | '-' | '<=' | '>=' | '$='
 * '^=' | '*=' | '>' | '<' | '=' | '!=' | '&&' | '||'
 */
%type binop { BinOpType }
binop(A) ::= LESS.
{ A = BOP_LESS; }
binop(A) ::= GREATER.
{ A = BOP_GREATER; }
binop(A) ::= LEQUAL.
{ A = BOP_LEQUAL; }
binop(A) ::= GEQUAL.
{ A = BOP_GEQUAL; }
binop(A) ::= EQUAL.
{ A = BOP_EQUAL; }
binop(A) ::= NOTEQUAL.
{ A = BOP_NOT_EQUAL; }

// JSON values
%type val { jvalue_ref }
%destructor val { j_release(&$$); }
val(A) ::= JSON_INT_NUMBER(B).
{
    int64_t _int;
    raw_buffer _rb = j_str_to_buffer(B.m_str, B.m_len);
    jstr_to_i64(&_rb, &_int);
    A = jnumber_create_i64(_int);
}
val(A) ::= JSON_NUMBER(B).
{
    double _double;
    raw_buffer _rb = j_str_to_buffer(B.m_str, B.m_len);
    jstr_to_double(&_rb, &_double);
    A = jnumber_create_f64(_double);
}
val(A) ::= JSON_STRING(B).
{
    A = jstring_create_copy(j_str_to_buffer(B.m_str, B.m_len));
}
val(A) ::= TRUE.
{
    A = jboolean_create(true);
}
val(A) ::= FALSE.
{
    A = jboolean_create(false);
}
val(A) ::= NULL.
{
    A = jnull();
}

%type json_string { char * }
%destructor json_string { g_free($$); }
json_string(A) ::= JSON_STRING(B).
{
    A = g_strndup(B.m_str, B.m_len);
}

%type array_index { int }
array_index(A) ::= JSON_INT_NUMBER(B).
{
    A = 0;
    raw_buffer _rb = j_str_to_buffer(B.m_str, B.m_len);

    if (jstr_to_i32(&_rb, (int32_t *) &A) != CONV_OK || A == 0)
    {
        char *scpy = g_strndup(B.m_str, B.m_len);
        jerror_set_formatted(context->error, JERROR_TYPE_SYNTAX,
                             "Invalid array index in array children selector: %s. Must be a nonzero int32 value",
                             scpy);
        g_free(scpy);
    }
}
