/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
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
extern int ext_nl(void);
extern int ext_page(void);
extern int ext_cursor(int x, int y);
extern int ext_put(int stream, const char *buf, long len);

#endif
