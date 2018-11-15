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
 * OpenComal expression evaluation header file
 */

#ifndef PDCEXP_H
#define PDCEXP_H

/** Retrieve an lval expression */
extern void    *exp_lval(struct expression *exp, enum VAL_TYPE *type,
                         struct var_item **var, long *strlen);

/** Calculate the value of/reduce to normal form a compound expression */
extern void     calc_exp(struct expression *exp, void **result,
                         enum VAL_TYPE *type);

/** Calculate the value of an expression in an integer context */
extern long     calc_intexp(struct expression *exp);

/** Calculate the value of an expression in a logical context */
extern int      calc_logexp(struct expression *exp);

#endif
