/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* String routines header file */

#ifndef PDCSTR_H
#define PDCSTR_H

extern char *my_strdup(int pool, const char *s);
extern int str_cmp(struct string *s1, struct string *s2);
extern struct string *str_make(int pool, const char *s);
extern struct string *str_make2(int pool, long len);
extern struct string *str_cat(struct string *s1, struct string *s2);
extern long str_search(struct string *needle, struct string *haystack);
extern struct string *str_cpy(struct string *s1, struct string *s2);
extern struct string *str_partcpy(struct string *s1, struct string *s2, long from, long to);
extern struct string *str_partcpy2(struct string *s1, struct string *s2, long from, long to);
extern struct string *str_dup(int pool, struct string *s);
extern struct string *str_maxdup(int pool, struct string *s, long n);
extern void str_extend(int pool, struct string **s, long newlen);

#endif
