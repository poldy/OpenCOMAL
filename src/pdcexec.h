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

/** Signal a run-time error  */
extern void run_error(int error, const char *s, ...) __printflike(2, 3);

/** Call a PROC/FUNC definition */
extern void exec_call(struct expression *exp, int calltype, void **result,
		      enum VAL_TYPE *type);

/** Install an exception handler */
extern int exec_trap(struct comal_line *line);

/** Return the metadata for an open file */
extern struct file_rec *fsearch(long i);

/** Read from a binary format file */
extern void do_readfile(struct two_exp *twoexp, struct exp_list *lvalroot);

/** Read from in-program DATA statements */
extern void read_data(struct comal_line *line);

/** Implement the READ keyword */
extern void exec_read(struct comal_line *line);

/** Write to a binary format file */
extern void exec_write(struct comal_line *line);

/** Print to a text format file */
extern void print_file(struct two_exp *twoexp,
		       struct print_list *printroot, int pr_sep, struct expression *using_modifier);

/** Input from a text format file */
extern void input_file(struct two_exp *twoexp, struct exp_list *lvalroot);

/** Execute one line of the program */
extern int exec_line(struct comal_line *line);

/** Execute a sequence of lines, in one environment */
extern void exec_seq(struct comal_line *line);

/** Initialise any module initialisation statements */
extern void exec_mod_init(struct comal_line *line);

#endif
