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

#ifndef __JQUERY_EXPRESSION_H_
#define __JQUERY_EXPRESSION_H_

#include "pbnjson.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _SelEx SelEx;


bool sel_ex_eval(SelEx *ex, jvalue_ref param);
void sel_ex_free(SelEx *ex);

typedef enum {
	BOP_EQUAL = 0,
	BOP_NOT_EQUAL,
	BOP_LESS,
	BOP_GREATER,
	BOP_LEQUAL,
	BOP_GEQUAL,
	BOP_AND,
	BOP_OR
} BinOpType;

SelEx* sel_ex_x();
SelEx* sel_ex_jvalue(jvalue_ref value);

SelEx* sel_ex_binop(SelEx *lhs, BinOpType type, SelEx *rhs);


#ifdef __cplusplus
} //extern "C"
#endif

#endif //__JQUERY_EXPRESSION_H_
