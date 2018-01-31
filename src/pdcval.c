/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* Routines to manipulate OpenComal internal data values */

#define _XOPEN_SOURCE 700

#include "pdcglob.h"
#include "pdcstr.h"
#include "pdcmisc.h"
#include "pdcexec.h"
#include "pdcsym.h"
#include "pdcexp.h"
#include "pdcval.h"

PRIVATE void val_print_array(int stream, struct var_item *var) 
{
	long n=var->array->nritems;
	char *data=(char *)var_data(var);
	enum VAL_TYPE type=var->type;
	int size=type_size(type);
	
	while (n>0) {
		if (type==V_STRING) {
			val_print(stream,*(struct string **)data,V_STRING);
		        my_put(stream,"\n",1);
                } else {
			val_print(stream,data,type);
		        my_put(stream," ",1);
                }
		data+=size;
		n--;
	}
}

PUBLIC void val_print(int stream, void *result, enum VAL_TYPE type)
{
	char buf[64];
	char *pptr = buf;
	long len = -1L;

	switch (type) {
	case V_INT:
		snprintf(buf, 64, "%ld", *(long *) result);
		break;

	case V_FLOAT:
		snprintf(buf, 64, "%g", *(double *) result);
		break;

	case V_STRING:
		if (result) {
			pptr = ((struct string *) result)->s;
			len = ((struct string *) result)->len;
		} else
			len=0;

		break;

	case V_ARRAY:
		val_print_array(stream,(struct var_item *)result);
		len=0;
		break;

	default:
		fatal("val_print() default action");
	}

	if (len) my_put(stream, pptr, len);
}


PUBLIC void val_copy(void *to, void *from, enum VAL_TYPE ttype, enum
		     VAL_TYPE ftype)
{
	if (ttype == V_STRING) {
		if (ftype != V_STRING)
			run_error(VALUE_ERR,
				  "Wrong type (must be string)");

		if (*(struct string **)to) mem_free(*(struct string **)to);

		*(struct string **) to = (struct string *)str_dup(RUN_POOL, (struct string *)from);
	} else if (ttype == V_FLOAT)
		if (ftype == V_FLOAT)
			*(double *) to = *(double *) from;
		else if (ftype == V_INT)
			*(double *) to = *(long *) from;
		else
			run_error(VALUE_ERR,
				  "Wrong type (must be numeric)");
	else if (ttype == V_INT)
		if (ftype == V_INT)
			*(long *) to = *(long *) from;
		else if (ftype == V_FLOAT)
			*(long *) to = d2int(*(double *) from, 0);
		else
			run_error(VALUE_ERR,
				  "Wrong type (must be semi-integer numeric)");
	else
		fatal("Val_copy() default action");

}


PUBLIC void val_free(void *result, enum VAL_TYPE type)
{
	if (type == V_STRING)
		mem_free(result);
	else if (type != V_ARRAY)
		cell_free(result);
}

PUBLIC double val_double(struct expression *exp) {
	void *result;
	enum VAL_TYPE type;
	double d = 0.0;

	calc_exp(exp,&result,&type);

	if (type==V_FLOAT)
		d=*(double *)result;
	else if (type==V_INT)
		d=*(long *)result;
	else
		fatal("val_double internal error #1");

	cell_free(result);
	return d;
}

PUBLIC int val_cmp(int op, void *r1, void *r2, enum VAL_TYPE t1, enum
		   VAL_TYPE t2)
{
	double d1, d2;
	int cmp;

	if (t1 == V_STRING) {
		if (t2 != V_STRING)
			run_error(VALUE_ERR,
				  "Wrong type (must be string)");

		cmp = str_cmp((struct string *)r1, (struct string *)r2);
	} else if (t1 == V_INT && t2 == V_INT) {
		if (*(long *) r1 == *(long *) r2)
			cmp = 0;
		else if (*(long *) r1 > *(long *) r2)
			cmp = 1;
		else
			cmp = -1;
	} else {
		if (t1 == V_INT)
			d1 = *(long *) r1;
		else
			d1 = *(double *) r1;

		if (t2 == V_INT)
			d2 = *(long *) r2;
		else
			d2 = *(double *) r2;

		if (d1 == d2)
			cmp = 0;
		else if (d1 > d2)
			cmp = 1;
		else
			cmp = -1;
	}

	switch (op) {
	case 0:
		break;
	case eqlSYM:
		cmp = (cmp == 0);
		break;
	case neqSYM:
		cmp = (cmp != 0);
		break;
	case lssSYM:
		cmp = (cmp < 0);
		break;
	case gtrSYM:
		cmp = (cmp > 0);
		break;
	case leqSYM:
		cmp = (cmp <= 0);
		break;
	case geqSYM:
		cmp = (cmp >= 0);
		break;

	default:
		fatal("val_cmp relop default action");
	}

	return cmp;
}


PUBLIC void val_neg(void *value, enum VAL_TYPE type)
{
	if (type == V_INT)
		*(long *) value = -*(long *) value;
	else
		*(double *) value = -*(double *) value;
}

PUBLIC long *val_int(long i, void *ptr, enum VAL_TYPE *type)
{
	long *p = (long *)cell_alloc(INT_CPOOL);

	if (ptr)
		val_free(ptr, *type);

	*p = i;
	*type = V_INT;

	return p;
}


PUBLIC double *val_float(double f, void *ptr, enum VAL_TYPE *type)
{
	double *p = (double *)cell_alloc(FLOAT_CPOOL);

	if (ptr)
		val_free(ptr, *type);

	*p = f;
	*type = V_FLOAT;

	return p;
}


PUBLIC void val_intadd(long *v1, long *v2, void **result, enum VAL_TYPE
		       *type)
{
	int minv1 = *v1 < 0;
	int minv2 = *v2 < 0;
	long v3 = *v1 + *v2;
	int do_float = 0;

	*result = v1;
	*type = V_INT;

	if (minv1 != minv2) {
		/* epsilon */ ;
	} else if (minv1)
		do_float = v3 >= 0;
	else
		do_float = v3 < 0;

	if (do_float) {
		*result =
		    val_float((double) *v1 + (double) *v2, NULL, type);
		cell_free(v1);
	} else
		*v1 = v3;

	cell_free(v2);
}


PUBLIC void val_intsub(long *v1, long *v2, void **result, enum VAL_TYPE
		       *type)
{
	*v2 = -*v2;
	val_intadd(v1, v2, result, type);
}


PUBLIC void val_intmul(long *v1, long *v2, void **result, enum VAL_TYPE
		       *type)
{
	long v3 = *v1 * *v2;

	if ((v3 % *v1 != 0) || (v3 % *v2 != 0)) {
		*result =
		    val_float((double) *v1 * (double) *v2, NULL, type);
		cell_free(v1);
	} else {
		*result = v1;
		*v1 = v3;
		*type = V_INT;
	}

	cell_free(v2);
}


PUBLIC void val_intdiv(long *v1, long *v2, void **result, enum VAL_TYPE
		       *type)
{
	if (*v2 == 0)
		run_error(DIV0_ERR, "Divide by Zero (0)");

	if (*v1 % *v2 == 0) {
		*v1 = *v1 / *v2;
		*type = V_INT;
		*result = v1;
	} else {
		*result =
		    val_float((double) *v1 / (double) *v2, NULL, type);
		cell_free(v1);
	}

	cell_free(v2);
}

PUBLIC long val_mustbelong(void *value, enum VAL_TYPE type, int freeit) 
{
	long n = 0;

	if (type==V_INT) 
		n=*(long *)value;
	else if (type==V_FLOAT)
		n=(long)*(double *)value;
	else
		fatal("val_mustbelong internal error #1");

	if (freeit) cell_free(value);

	return n;
}
