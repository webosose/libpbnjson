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

#include "expression.h"
#include "../jobject_internal.h"
#include <glib.h>
#include <math.h>

typedef void (*Free)(SelEx *);
typedef jvalue_ref (*Eval)(SelEx *, jvalue_ref param);

struct _SelEx
{
	Free free;
	Eval eval;
	jvalue_ref value;
};


static bool jvalue_to_bool(jvalue_ref value)
{
	if (!jis_valid(value))
		return false;

	switch (jget_type(value))
	{
	case JV_NULL: return true;
	case JV_BOOL:
		{
			bool ret;
			jboolean_get(value, &ret);
			return ret;
		}
	case JV_NUM:
		{
			double ret;
			jnumber_get_f64(value, &ret);
			return fabs(ret - 0.0) > 1e-9;
		}
	case JV_STR:
		return true;
	default:
		break;
	}
	return true;
}

bool sel_ex_eval(SelEx *ex, jvalue_ref param)
{
	jvalue_ref a = ex->eval(ex, param);

	return jvalue_to_bool(a);
}

void sel_ex_free(SelEx *ex)
{
	if (ex)
		ex->free(ex);
}

static void destr_void(SelEx *ex) { }

static jvalue_ref eval_x(SelEx *ex, jvalue_ref param)
{
	return param;
}

SelEx* sel_ex_x()
{
	static SelEx x_expr = {
		.free = destr_void,
		.eval = eval_x,
	};
	return &x_expr;
}

static jvalue_ref eval_jvalue(SelEx *ex, jvalue_ref param)
{
	return ex->value;
}

static void sel_ex_destroy(SelEx *ex)
{
	j_release(&ex->value);
}

static void sel_ex_base_free(SelEx *ex)
{
	sel_ex_destroy(ex);
	g_slice_free(SelEx, ex);
}

SelEx* sel_ex_jvalue(jvalue_ref value)
{
	SelEx *ex = g_slice_new(SelEx);
	ex->free = sel_ex_base_free;
	ex->eval = eval_jvalue;
	ex->value = value;
	return ex;
}

// 1 -- true
// 0 -- false
// -1 -- invalid
typedef int (*compose_func)(jvalue_ref a, jvalue_ref b);

typedef struct
{
	SelEx base;
	SelEx *lhs;
	SelEx *rhs;
	compose_func composition;
} BinaryExpr;

static void bin_expr_free(SelEx *v)
{
	if (!v) return;

	BinaryExpr *e = (BinaryExpr *) v;
	if (e->lhs)
		e->lhs->free(e->lhs);
	if (e->rhs)
		e->rhs->free(e->rhs);
	sel_ex_destroy(v);
	g_slice_free(BinaryExpr, e);
}

static jvalue_ref eval_composition(SelEx *v, jvalue_ref ctxt)
{
	BinaryExpr *e = (BinaryExpr *) v;

	jvalue_ref a = e->lhs->eval(e->lhs, ctxt);
	jvalue_ref b = e->rhs->eval(e->rhs, ctxt);

	int r = e->composition(a, b);
	if (r == -1)
		return jinvalid();
	else
		v->value = r == 0 ? jboolean_false()
		                  : jboolean_true();
	return v->value;
}

SelEx* sel_ex_composition(SelEx *lhs, SelEx *rhs, compose_func composition)
{
	BinaryExpr *ret = g_slice_new(BinaryExpr);
	ret->base.free = bin_expr_free;
	ret->base.eval = eval_composition;
	ret->base.value = jboolean_create(false);
	ret->lhs = lhs;
	ret->rhs = rhs;
	ret->composition = composition;
	return (SelEx *) ret;
}

static int compare_equal(jvalue_ref a, jvalue_ref b)
{
	return jvalue_equal(a, b);
}

static int compare_not_equal(jvalue_ref a, jvalue_ref b)
{
	return !jvalue_equal(a, b);
}

static int compare_less_impl(jvalue_ref a, jvalue_ref b, bool true_on_equal)
{
	if (jget_type(a) != jget_type(b))
		return -1;

	if (jis_boolean(a) && jis_boolean(b))
	{
		if (true_on_equal)
			return jboolean_deref_to_value(a) <= jboolean_deref_to_value(b);
		return jboolean_deref_to_value(a) < jboolean_deref_to_value(b);
	}

	if (jis_number(a) && jis_number(b))
	{
		double x, y;
		jnumber_get_f64(a, &x);
		jnumber_get_f64(b, &y);
		if (true_on_equal)
			return x <= y;
		return x < y;
	}

	if (jis_string(a) && jis_string(b))
	{
		raw_buffer x = jstring_get_fast(a);
		raw_buffer y = jstring_get_fast(b);
		if (true_on_equal)
			return strcmp(x.m_str, y.m_str) <= 0;
		return strcmp(x.m_str, y.m_str) < 0;
	}

	// If two values of the same type are equal, we know for sure that
	// the first isn't less than the second one.
	if (!jvalue_equal(a, b))
		return -1;

	return true_on_equal;
}

static int compare_less(jvalue_ref a, jvalue_ref b)
{
	return compare_less_impl(a, b, false);
}

static int compare_lequal(jvalue_ref a, jvalue_ref b)
{
	return compare_less_impl(a, b, true);
}

static int compose_and(jvalue_ref a, jvalue_ref b)
{
	return jvalue_to_bool(a) && jvalue_to_bool(b);
}

static int compose_or(jvalue_ref a, jvalue_ref b)
{
	return jvalue_to_bool(a) || jvalue_to_bool(b);
}

SelEx* sel_ex_binop(SelEx *lhs, BinOpType type, SelEx *rhs)
{
	switch (type)
	{
	case BOP_EQUAL:
		return sel_ex_composition(lhs, rhs, compare_equal);
	case BOP_NOT_EQUAL:
		return sel_ex_composition(lhs, rhs, compare_not_equal);
	case BOP_LESS:
		return sel_ex_composition(lhs, rhs, compare_less);
	case BOP_GREATER:
		return sel_ex_composition(rhs, lhs, compare_less);
	case BOP_LEQUAL:
		return sel_ex_composition(lhs, rhs, compare_lequal);
	case BOP_GEQUAL:
		return sel_ex_composition(rhs, lhs, compare_lequal);
	case BOP_AND:
		return sel_ex_composition(lhs, rhs, compose_and);
	case BOP_OR:
		return sel_ex_composition(rhs, lhs, compose_or);
	default:
		assert(!"Unknown binary operation");
		abort(); // unreachable line
	}
}
