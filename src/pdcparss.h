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

extern void yyerror(const char *s);
extern int yyparse();

extern struct exp_list *pars_explist_item(struct expression *exp,
					  struct exp_list *next);
extern struct id_list *pars_idlist_item(struct id_rec *id,
					  struct id_list *next);
extern struct print_list *pars_printlist_item(int pr_sep,
					      struct expression *exp,
					      struct print_list *next);
extern struct dim_list *pars_dimlist_item(struct id_rec *id,
					  struct expression *strlen,
					  struct dim_ension *root);
extern struct when_list *pars_whenlist_item(int op,
					    struct expression *exp,
					    struct when_list *next);
extern struct assign_list *pars_assign_item(int op,
					    struct expression *lval,
					    struct expression *rval);
extern struct expression *pars_exp_const(int op);
extern struct expression *pars_exp_unary(int op, struct expression *exp);
extern struct expression *pars_exp_sys(int sym, enum optype type,
				       struct exp_list *exproot);
extern struct expression *pars_exp_binary(int op, struct expression *exp1,
					  struct expression *exp2);
extern struct expression *pars_exp_int(long num);
extern struct expression *pars_exp_float(struct dubbel *d);
extern struct expression *pars_exp_string(struct string *str);
extern struct expression *pars_exp_id(int op, struct id_rec *id,
				      struct exp_list *exproot);
extern struct expression *pars_exp_array(int op, struct id_rec *id, enum optype type);
extern struct expression *pars_exp_sid(struct id_rec *id,
				       struct exp_list *exproot,
				       struct two_exp *twoexp);
extern struct expression *pars_exp_substr(struct expression *exp,
					  struct two_exp *twoexp);
extern struct expression *pars_exp_num(struct expression *numexp);
extern struct expression *pars_exp_str(struct expression *strexp);
extern struct expression *pars_exp_rnd(struct expression *exp1, struct expression *exp2);
extern void pars_error(const char *s, ...);
extern int pars_handle_error(void);

#endif
