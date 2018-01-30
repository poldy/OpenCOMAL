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
 * OpenComal 'default' extensions
 */

#ifndef PDCEXT_H
#define PDCEXT_H

/** Initialise all extensions */
extern void ext_init(void);

/** Cleanup after all extensions */
extern void ext_tini(void);

/**
 * Hook to SCAN any extensions.
 * Not currently used.
 */
extern int ext_call_scan(struct id_rec *id, struct exp_list *exproot,
			 char *errtext);

/**
 * Hook to EXEC any extensions.
 * Not currently used.
 */
extern bool ext_call(struct id_rec *id, struct exp_list *exproot,
		    int calltype, void **result, enum VAL_TYPE *type);

/**
 * Evaluate any SYS(???) expressions
 * Unless you require backward-compability with some other implementation,
 * this is the recommended way to extend the interpreter.
 */
extern int ext_sys_exp(struct exp_list *exproot, void **result,
		       enum VAL_TYPE *type);

/**
 * Evaluate any SYS$(???) expressions
 * Unless you require backward-compability with some other implementation,
 * this is the recommended way to extend the interpreter.
 */
extern int ext_syss_exp(struct exp_list *exproot, struct string **result,
			enum VAL_TYPE *type);

/**
 * Execute any "SYS ???" statements
 * Unless you require backward-compability with some other implementation,
 * this is the recommended way to extend the interpreter.
 */
extern int ext_sys_stat(struct exp_list *exproot);

/** Handle input from a stream after "SYS SYSIN" */
extern bool ext_get(int stream, char *line, int maxlen, const char *prompt);

/** Handle writing a nl after "SYS SYSOUT" */
extern bool ext_nl(void);

/** Handle writing a tab after "SYS SYSOUT" */
extern bool ext_ht(void);

/** Handle writing a np after "SYS SYSOUT" */
extern void ext_page(FILE *f);

/** Handle clearing to EOL after "SYS SYSOUT" */
extern void ext_clrtoeol(FILE *f);

/** Handle moving the cursor after "SYS SYSOUT" */
extern void ext_cursor(FILE *f, int y, int x);

/** Handle output to a stream after "SYS SYSOUT" */
extern bool ext_put(int stream, const char *buf, long len);

#endif
