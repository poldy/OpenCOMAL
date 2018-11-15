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
 * Parse Support Routines header file
 */

#ifndef PDCPARSS_H
#define PDCPARSS_H

#include "compat_cdefs.h"

/** yacc error handler, no action */
extern void     yyerror(const char *s);

extern int      yyparse(void);

/** Action to store one item in an expression list */
extern struct exp_list *pars_explist_item(struct expression *exp,
                                          struct exp_list *next);

/** Action to store one item in an identifier list */
extern struct id_list *pars_idlist_item(struct id_rec *id,
                                        struct id_list *next);

/** Action to store one item in a print list */
extern struct print_list *pars_printlist_item(int pr_sep,
                                              struct expression *exp,
                                              struct print_list *next);

/** Action to store one item in a dimension list */
extern struct dim_list *pars_dimlist_item(struct id_rec *id,
                                          struct expression *strlen,
                                          struct dim_ension *root);

/** Action to store one item in a dimension list */
extern struct when_list *pars_whenlist_item(int op,
                                            struct expression *exp,
                                            struct when_list *next);

/** Action to store an assignment */
extern struct assign_list *pars_assign_item(int op,
                                            struct expression *lval,
                                            struct expression *rval);

/** Action to store a constant expression */
extern struct expression *pars_exp_const(int op);

/** Action to store a unary expression */
extern struct expression *pars_exp_unary(int op, struct expression *exp);

/** Action to store a SYS() expression */
extern struct expression *pars_exp_sys(int sym, enum optype type,
                                       struct exp_list *exproot);

/** Action to store a binary expression */
extern struct expression *pars_exp_binary(int op, struct expression *exp1,
                                          struct expression *exp2);

/** Action to store an integer expression */
extern struct expression *pars_exp_int(long num);

/** Action to store a float expression */
extern struct expression *pars_exp_float(struct dubbel *d);

/** Action to store a string expression */
extern struct expression *pars_exp_string(struct string *str);

/** Action to store an identifier expression */
extern struct expression *pars_exp_id(int op, struct id_rec *id,
                                      struct exp_list *exproot);

/** Action to store an array expression */
extern struct expression *pars_exp_array(int op, struct id_rec *id,
                                         enum optype type);

/** Action to store a string ID expression */
extern struct expression *pars_exp_sid(struct id_rec *id,
                                       struct exp_list *exproot,
                                       struct two_exp *twoexp);

/** Action to store a substring expression */
extern struct expression *pars_exp_substr(struct expression *exp,
                                          struct two_exp *twoexp);

/** Action to store a numeric expression */
extern struct expression *pars_exp_num(struct expression *numexp);

/** Action to store a string expression */
extern struct expression *pars_exp_str(struct expression *strexp);

/** Action to store a RND() expression */
extern struct expression *pars_exp_rnd(struct expression *exp1,
                                       struct expression *exp2);

/**
 * Immediate error handler for all actions.
 * The actual processing is deferred until a call to @pars_handle_error .
 * @see pars_handle_error
 */
extern void     pars_error(const char *s, ...);

/**
 * Report any errors deferred from @c pars_error
 * @see pars_error
 */
extern int      pars_handle_error(void);

#endif
