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

/** Print a value to the specified stream */
extern void val_print(int stream, void *result, enum VAL_TYPE type);

/** Copy a value, optionally performing data type conversion */
extern void val_copy(void *to, void *from, enum VAL_TYPE ttype, enum VAL_TYPE ftype);

/** Free the storage for a value */
extern void val_free(void *result, enum VAL_TYPE type);

/** Perform the specified comparison operation between two values */
extern int val_cmp(int op, void *r1, void *r2, enum VAL_TYPE t1, enum VAL_TYPE t2);

/** Negate a numeric value */
extern void val_neg(void *value, enum VAL_TYPE type);

/** Store a new integer value */
extern long *val_int(long i, void *ptr, enum VAL_TYPE *type);

/** Store a new float value */
extern double *val_float(double f, void *ptr, enum VAL_TYPE *type);

/** Return the value of a numeric expression as a C "double" */
extern double val_double(struct expression *exp);

/** Return the value of a numeric expression as a C "long" */
extern long val_mustbelong(void *val, enum VAL_TYPE type, int freeit);

/** Add 2 ints, converting to float in case of overflow */
extern void val_intadd(long *v1, long *v2, void **result,
		       enum VAL_TYPE *type);

/** Subtract 2 ints, converting to float in case of underflow */
extern void val_intsub(long *v1, long *v2, void **result,
		       enum VAL_TYPE *type);

/** Multiply 2 ints, converting to float in case of overflow */
extern void val_intmul(long *v1, long *v2, void **result,
		       enum VAL_TYPE *type);

/** Divide 2 ints, converting to float when the result is not whole */
extern void val_intdiv(long *v1, long *v2, void **result,
		       enum VAL_TYPE *type);

#endif
