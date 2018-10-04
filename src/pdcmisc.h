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
 * Misc routines header file
 */

#ifndef PDCMISC_H
#define PDCMISC_H

#include "compat_cdefs.h"

/** Write a nl to the specified stream */
extern void my_nl(int stream);

/** Write a tab to the specified stream */
extern void my_ht(int stream);

/** Write a string to the specified stream */
extern void my_put(int stream, const char *buf, long len);

/** Write a formatted string to the specified stream */
extern void my_printf(int stream, bool newline, const char *s, ...) __printflike(3, 4);

/** Optionally print a debug message */
#ifdef NDEBUG
#define DBG_PRINTF(...)
#else
#define DBG_PRINTF(...) { if (comal_debug) { my_printf(MSG_DEBUG, __VA_ARGS__); } }
#endif

/** Abort execution of the current program and return to the command loop */
extern void fatal(const char *s);

/**
 * Reverse a list.
 * @public @memberof my_list
 */
extern void *my_reverse(void *root);

/**
 * Free all nodes in a list.
 * @public @memberof my_list
 */
extern void free_list(struct my_list *root);

/** Check that an expression is a list of numbers */
extern bool exp_list_of_nums(struct exp_list *root);

/** Get confirmation to proceed if there are unsaved changes in the current environment */
extern bool check_changed(void);

/** Get confirmation to proceed if there are unsaved changes in any environment */
extern bool check_changed_any(void);

/** List a program line to the specified stream */
extern void puts_line(int stream, struct comal_line *line);

/**
 * Count the length of a list.
 * @public @memberof my_list
 */
extern int nr_items(struct my_list *list);

/** Convert a float with no fractional part to an int */
extern long d2int(double x, int whole);

/** Test type compatibility, for NAME params & return values */
extern int type_match1(struct id_rec *id, struct expression *exp);

/** Get the identifier out of an expression */
extern struct id_rec *exp_of_id(struct expression *exp);

/** Test if an expression is a string or not */
extern bool exp_of_string(struct expression *exp);

/** Return the identifier in an expression in string form */
extern char *exp_cmd(struct expression *exp);

/** Search for a program line with a particular number */
extern struct comal_line *search_line(long l, int exact);

/** Get the number of bytes required to store a tokenised statement */
extern int stat_size(int cmd);

/** Output a run-time error to the user */
extern void give_run_err(struct comal_line *line);

/** Return the number of bytes to store a COMAL type */
extern int type_size(enum VAL_TYPE t);

/** Check that an expression is usable as an lvalue */
extern void check_lval(struct expression *exp);

/** Return the nesting level of a PROC or FUNC */
extern int proclevel(struct comal_line *proc);

/**
 * Return the @em actual COMAL statement in a line.
 * i.e. in most cases the line, but in case of a short form FOR, 
 * IF, WHILE or REPEAT it returns the "body" of the short form
 * structure statement.
 */
extern struct comal_line *line_2line(struct comal_line *line);

/** Return the command for a program line */
extern int line_2cmd(struct comal_line *line);

/** Copy a program line */
extern struct comal_line *stat_dup(struct comal_line *stat);

/** Convert a long to a string in base 10 */
extern char *ltoa(long num, char *buf);

#ifndef HAS_STRLWR

/** Convert a string to upper case in-place, for the current locale */
extern void strupr(char *s);

/** Convert a string to lower case in-place, for the current locale */
extern void strlwr(char *s);

#endif

/** Remove a trailing string, if it's there */
extern void remove_trailing(char *s, const char *trailing, const char *subst);

#ifndef HAS_ROUND

/**
 * Round a float.
 * +-0.5 rounds towards +infinity, i.e. we want floored and not symmetric MOD/DIV.
 */
extern double my_round(double x);

#endif

/**
 * Get the fractional part of a float.
 * For negative numbers this is ceil(x) - x, i.e. we want floored and not symmetric MOD/DIV.
 */
extern double my_frac(double x);

#endif
