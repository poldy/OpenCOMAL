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

extern void ext_init(void);
extern void ext_tini(void);
extern int ext_call_scan(struct id_rec *id, struct exp_list *exproot,
			 char *errtext);
extern int ext_call(struct id_rec *id, struct exp_list *exproot,
		    int calltype, void **result, enum VAL_TYPE *type);
extern int ext_sys_exp(struct exp_list *exproot, void **result,
		       enum VAL_TYPE *type);
extern int ext_syss_exp(struct exp_list *exproot, struct string **result,
			enum VAL_TYPE *type);
extern int ext_sys_stat(struct exp_list *exproot);
extern int ext_get(int stream, char *line, int maxlen, const char *prompt);
extern bool ext_nl(void);

/** Handle writing a tab to a redirected output file */
extern bool ext_ht(void);

extern void ext_page(FILE *f);

/** Handle clearing to EOL in a redirected output file */
extern void ext_clrtoeol(FILE *f);

extern void ext_cursor(FILE *f, int y, int x);
extern bool ext_put(int stream, const char *buf, long len);

#endif
