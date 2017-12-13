/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal own string routines */

#include <string.h>
#include "pdcglob.h"
#include "pdcstr.h"


PUBLIC char *my_strdup(int pool, const char *s)
{
	long l = strlen(s);
	char *t = (char *)mem_alloc(pool, l + 1);
	char *result;

	result = strncpy(t, s, l);
	t[l] = '\0';
	return result;
}


PUBLIC int str_cmp(struct string *s1, struct string *s2)
{
	char HUGE_POINTER *w1 = s1->s;
	char HUGE_POINTER *w2 = s2->s;

	return strcoll_l(w1, w2, latin_loc);
}


PUBLIC struct string *str_make(int pool, const char *s)
{
	long l = strlen(s);
	struct string *work = STR_ALLOC(pool, l);

	work->len = l;
	strncpy(work->s, s, l);
	work->s[l] = '\0';

	return work;
}

PUBLIC struct string *str_make2(int pool, long len)
{
	struct string *work = STR_ALLOC(pool, len);

	work->len = len;
	
        memset(work->s, ' ', len);

	return work;
}

PUBLIC struct string *str_cat(struct string *s1, struct string *s2)
{
	char HUGE_POINTER *w1 = s1->s;

	s1->len += s2->len;
	strncat(w1, s2->s, s2->len);

	return s1;
}


PUBLIC long str_search(struct string *needle, struct string *haystack)
{
	char HUGE_POINTER *h = haystack->s;
	char HUGE_POINTER *n = needle->s;

        if (*n != '\0' && (h = strstr(h, n)) != NULL) {
                return h - haystack->s + 1;
        }
        return 0L;
}


PUBLIC struct string *str_cpy(struct string *s1, struct string *s2)
{
	s1->len = s2->len;
	strncpy(s1->s, s2->s, s2->len);
	s1->s[s2->len] = '\0';

	return s1;
}

PUBLIC struct string *str_ncpy(struct string *s1, struct string *s2, long n)
{
	s1->len = n;
	strncpy(s1->s, s2->s,n);
	s1->s[n] = '\0';

	return s1;
}

/*
 * This routine copies a substring of string 2 to string 1
 */
PUBLIC struct string *str_partcpy(struct string *s1, struct string *s2,
				  long from, long to)
{
	char HUGE_POINTER *w1 = s1->s;
	char HUGE_POINTER *w2 = s2->s;

	s1->len = to - from + 1;
	w2 = w2 + from - 1;

        strncpy(w1, w2, to - from + 1);

        w1[to - from + 1] = '\0';

	return s1;
}

/*
 * This routine copies string 2 to a substring of string 1
 */
PUBLIC struct string *str_partcpy2(struct string *s1, struct string *s2,
				  long from, long to)
{
	char HUGE_POINTER *w1 = s1->s+from-1; /* Comal strings start at offset 1 */
	char HUGE_POINTER *w2 = s2->s;

#ifdef STD_STRFNS
        // This call breaks the "padding with spaces" test in common_string.lst
        strncpy(w1, w2, to - from + 1);
#else
	while (from <= to && *w2)
		*w1 = *w2, w1++, w2++, from++;
#endif

	return s1;
}

PUBLIC struct string *str_dup(int pool, struct string *s)
{
	struct string *work = STR_ALLOC(pool, s->len);

	str_cpy(work, s);

	return work;
}


PUBLIC struct string *str_maxdup(int pool, struct string *s, long max)
{
	struct string *work;
	long len=(max<s->len)?max:s->len;

	work = STR_ALLOC(pool, len);
	str_ncpy(work, s, len);

	return work;
}

PUBLIC void str_extend(int pool, struct string **s, long newlen)
{
	struct string *work;
	char HUGE_POINTER *t;

	if ((*s) && (*s)->len>=newlen) return;

	work=STR_ALLOC(pool,newlen);
	str_cpy(work,*s);
	t=&work->s[(*s)->len];
	
        memset(t, ' ', newlen - (*s)->len);
        t[newlen - (*s)->len] = '\0';
	work->len=newlen;
	mem_free(*s);
	*s=work;
}

PUBLIC char *str_ltou(const char *lstr)
{
	static char ustr[PATH_MAX];
	const char *inbuf;
	char *outbuf;
	size_t inbytesleft, outbytesleft;

	inbuf = lstr;
	inbytesleft = strlen(lstr);
	outbuf = ustr;
	outbytesleft = PATH_MAX;
	iconv(latin_to_utf8, (char **)&inbuf, &inbytesleft, &outbuf, &outbytesleft);
	*outbuf = '\0';
	return ustr;
}
