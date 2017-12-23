/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/** @file
 * OpenComal internal data value manipulation header file
 */

#ifndef PDCVAL_H
#define PDCVAL_H

extern void val_print_array(int stream, struct var_item *var);
extern void val_print(int stream, void *result, enum VAL_TYPE type);
extern void val_copy(void *to, void *from, enum VAL_TYPE ttype, enum VAL_TYPE ftype);
extern void val_free(void *result, enum VAL_TYPE type);
extern int val_cmp(int op, void *r1, void *r2, enum VAL_TYPE t1, enum VAL_TYPE t2);
extern void val_neg(void *value, enum VAL_TYPE type);
extern long *val_int(long i, void *ptr, enum VAL_TYPE *type);
extern double *val_float(double f, void *ptr, enum VAL_TYPE *type);
extern double val_double(struct expression *exp);
extern long val_mustbelong(void *val, enum VAL_TYPE type, int freeit);
extern void val_intadd(long *v1, long *v2, void **result,
		       enum VAL_TYPE *type);
extern void val_intsub(long *v1, long *v2, void **result,
		       enum VAL_TYPE *type);
extern void val_intmul(long *v1, long *v2, void **result,
		       enum VAL_TYPE *type);
extern void val_intdiv(long *v1, long *v2, void **result,
		       enum VAL_TYPE *type);

#endif
