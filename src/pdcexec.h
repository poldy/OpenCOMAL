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
 * Line execution routines header file
 */

#ifndef PDCEXEC_H
#define PDCEXEC_H

extern void run_error(int error, const char *s, ...);
extern void exec_call(struct expression *exp, int calltype, void **result,
		      enum VAL_TYPE *type);
extern int exec_trap(struct comal_line *line);
extern struct file_rec *fsearch(long i);
extern void do_readfile(struct two_exp *twoexp, struct exp_list *lvalroot);
extern void read_data(struct comal_line *line);
extern void exec_read(struct comal_line *line);
extern void exec_write(struct comal_line *line);
extern void print_file(struct two_exp *twoexp,
		       struct print_list *printroot, int pr_sep, struct expression *using_modifier);
extern void input_file(struct two_exp *twoexp, struct exp_list *lvalroot);
extern int exec_line(struct comal_line *line);
extern void exec_seq(struct comal_line *line);
extern void exec_mod_init(struct comal_line *line);

#endif
