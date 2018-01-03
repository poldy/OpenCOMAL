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
 * OpenComal SYS routines.
 * This is the OS abstraction layer.
 */

#ifndef PDCDSYS_H
#define PDCDSYS_H

#include "pdcdef.h"

#include <stdio.h>
#include <stdbool.h>

/** Initialise the OS support layer */
extern void sys_init(void);

/** Cleanup the OS support layer before exiting */
extern void sys_tini(void);

/** Generate a random number */
extern void sys_rand(long *result, long *scale);

/** Randomizes the random number generator */
extern void sys_randomize(long seed);

/** Test whether or not the escape (^C) key has been pressed */
extern int sys_escape(void);

/** Output a string to a stream */
extern void sys_put(int stream, const char *buf, long len);

/** Clear the screen */
extern void sys_page(FILE * f);

/** Erase the current line to the right of the cursor to the end of the current line */
extern void sys_clrtoeol(FILE * f);

/** Pass a command to the OS command interpreter */
extern int sys_system(char *cmd);

/** Configure text output to wait for a key to be pressed after each screenful */
extern void sys_setpaged(int flag);

/** Move the cursor to the specified location */
extern void sys_cursor(FILE * f, long y, long x);

/** Returns the current column position of the cursor on the text screen */
extern int sys_curcol(void);

/** Returns the current row of the text screen that the cursor is on */
extern int sys_currow(void);

/** Output a newline to the specified stream */
extern void sys_nl(int stream);

/** Move the cursor to the next tab stop */
extern void sys_ht(int stream);

/** Get the current tab stop interval */
extern long sys_zone_num(void);

/** Set the tab stop interval */
extern void sys_zone(long size);

/** Get a yes or no answer to a question */
extern int sys_yn(int stream, const char *s);

/** Get a line of text */
extern int sys_get(int stream, char *line, int maxlen, const char *prompt);

/** Edit a line of text */
extern int sys_edit(int stream, char *line, int maxlen, int cursor);

/** Get the current working directory */
extern char *sys_dir_string();

/** Print a directory listing */
extern void sys_dir(const char *pattern);

/** Get the drive letter/number */
extern const char *sys_unit_string();

/** Return a string containing enough spaces to get to a column */
extern const char *sys_tab_string(long col);

/** Set the drive letter/number */
extern void sys_unit(char *unit);

/** Change the current working directory */
extern void sys_chdir(char *dir);

/** Remove a directory */
extern void sys_rmdir(char *dir);

/** Create a directory */
extern void sys_mkdir(char *dir);

/** Get one keypress */
extern char *sys_key(long delay);

/** Scan a line containing an statement that is an extension */
extern int sys_call_scan(struct id_rec *id, struct exp_list *exproot,
			 char *errtext);

/**
 * Call a statement that is an extension
 * @todo Should I have used pdcext.c for all the statements I added?
 */
extern int sys_call(struct id_rec *id, struct exp_list *exproot,
		    int calltype, void **result, enum VAL_TYPE *type);

/** Evaluation a SYS(arg) expression */
extern void sys_sys_exp(struct exp_list *exproot, void **result, enum
			VAL_TYPE *type);

/** Evaluation a SYS$(arg) expression */
extern void sys_syss_exp(struct exp_list *exproot, struct string **result, enum
			 VAL_TYPE *type);

/** Evaluate a "SYS arg" statement */
extern int sys_sys_stat(struct exp_list *exproot);

#endif
