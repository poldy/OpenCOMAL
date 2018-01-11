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

extern void my_nl(int stream);

/** Write a tab to the specified stream */
extern void my_ht(int stream);

extern void my_put(int stream, const char *buf, long len);
extern void my_printf(int stream, bool newline, const char *s, ...);

/** Optionally print a debug message */
#ifdef NDEBUG
#define DBG_PRINTF(...)
#else
#define DBG_PRINTF(...) { if (comal_debug) { my_printf(MSG_DEBUG, __VA_ARGS__); } }
#endif

extern void fatal(const char *s, ...);
extern void *my_reverse(void *root);
extern void free_list(struct my_list *root);
extern bool exp_list_of_nums(struct exp_list *root);
extern bool check_changed(void);
extern bool check_changed_any(void);
extern void puts_line(int stream, struct comal_line *line);
extern int nr_items(struct my_list *list);
extern long d2int(double x, int whole);
extern int type_match1(struct id_rec *id, struct expression *exp);
extern struct id_rec *exp_of_id(struct expression *exp);
extern bool exp_of_string(struct expression *exp);
extern char *exp_cmd(struct expression *exp);
extern struct comal_line *search_line(long l, int exact);
extern int stat_size(int cmd);
extern void give_run_err(struct comal_line *line);
extern int type_size(enum VAL_TYPE t);
extern void check_lval(struct expression *exp);
extern int proclevel(struct comal_line *proc);
extern struct comal_line *line_2line(struct comal_line *line);
extern int line_2cmd(struct comal_line *line);
extern struct comal_line *stat_dup(struct comal_line *stat);
extern void trace_trace(void);
extern void trace_reset(void);
extern char *ltoa(long num, char *buf, int radix);

#ifndef HAS_STRLWR
extern void strupr(char *s);
extern void strlwr(char *s);
#endif

extern void remove_trailing(char *s, const char *trailing, const char *subst);
#ifndef HAS_ROUND
extern double my_round(double x);
#endif
extern double my_frac(double x);

#endif
