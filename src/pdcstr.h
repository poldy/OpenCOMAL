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
 * String routines header file
 */

#ifndef PDCSTR_H
#define PDCSTR_H

#include <string.h>

/** Duplicate a string using memory from a specified pool */
extern char    *my_strdup(int pool, const char *s);

/** Compare two COMAL strings */
extern int      str_cmp(struct string *s1, struct string *s2);

/** Convert a C string to a COMAL string */
extern struct string *str_make(int pool, const char *s);

/** Create an empty COMAL string from a pool */
extern struct string *str_make2(int pool, long len);

/** Append one COMAL strings to another */
extern struct string *str_cat(struct string *s1, struct string *s2);

/** Search for one COMAL string in another, like strstr in C */
extern long     str_search(struct string *needle, struct string *haystack);

/*
 * Copy a COMAL string 
 */
extern struct string *str_cpy(struct string *s1, struct string *s2);

/** Copies a substring of string 2 to string 1 */
extern struct string *str_partcpy(struct string *s1, struct string *s2,
                                  long from, long to);

/** Copies string 2 to a substring of string 1 */
extern struct string *str_partcpy2(struct string *s1, struct string *s2,
                                   long from, long to);

/** Duplicate a COMAL string */
extern struct string *str_dup(int pool, struct string *s);

/** Duplicate a COMAL string, but with a maximum bound */
extern struct string *str_maxdup(int pool, struct string *s, long n);

/** Extend the storage allocated for a COMAL string */
extern void     str_extend(int pool, struct string **s, long newlen);

/** Convert a string from Latin-9 to UTF8 encoding */
extern char    *str_ltou(const char *lstr);

/**
 * Like strncpy, but guarantees that @c dest will be NUL-terminated.
 * @param[in,out] dest destination string
 * @param[in] src source string
 * @param[in] n size of @c dest
 */
static inline char *
term_strncpy(char *dest, const char *src, size_t n)
{
    strncpy(dest, src, n - 1);
    dest[n - 1] = '\0';
    return dest;
}

#endif
