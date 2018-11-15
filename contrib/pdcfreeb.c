/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/*
 * PDCOMAL SYS routines for FreeBSD 
 */

#include "pdcglob.h"
#include "pdcmisc.h"
#include <signal.h>

PRIVATE int     escape = 0;

PRIVATE void
int_handler()
{
    escape = 1;
    signal(SIGINT, int_handler);
}

PUBLIC void
sys_init()
{
    ext_init();
    signal(SIGINT, int_handler);
}


PUBLIC void
sys_tini()
{
    ext_tini();
}


PUBLIC double
sys_rand(arg)
     long            arg;
{
    long            n = rand();

    if (arg == 0)
        if (n == 0)
            return 0L;
        else
            return 1 / n;

    return (n * arg) / (~(1L << (8 * sizeof(int) - 1)));
}


PUBLIC int
sys_escape()
{
    if (escape) {
        escape = 0;

        return 1;
    }

    return 0;
}


PUBLIC void
sys_put(stream, buf, len)
     int             stream;
     char           *buf;
     long            len;
{
    ext_put(stream, buf, len);
    fputs(buf, stdout);
}


PUBLIC void
sys_page(f)
     FILE           *f;
{
    ext_page();
    return;
    run_error(NIMP_ERR, "PAGE not implemented");
}


PUBLIC void
sys_cursor(f, x, y)
     FILE           *f;
     long            x;
     long            y;
{
    ext_cursor(x, y);
    return;
    run_error(NIMP_ERR, "CURSOR not implemented");
}


PUBLIC void
sys_nl(stream)
     int             stream;
{
    ext_nl();
    putchar('\n');
}


PUBLIC void
sys_screen_readjust()
{
}

PUBLIC int
sys_yn(stream, s)
     int             stream;
     char           *s;
{
    char            buf[10];
    int             i;

    for (;;) {
        my_printf(MSG_DIALOG, 0, s);
        fgets(buf, 9, stdin);

        for (i = 0; i < 9 && buf[i]; i++)
            if (buf[i] == 'y' || buf[i] == 'Y')
                return 1;
            else if (buf[i] == 'n' || buf[i] == 'N');
        return 0;
    }
}


PRIVATE int
do_get(stream, line, maxlen, prompt)
     char           *line;
     int             maxlen;
     char           *prompt;
{
    if (prompt)
        my_printf(stream, 0, prompt);

    if (!fgets(line, maxlen, stdin)) {
        *line = 0;

        if (!feof(stdin))
            perror("Error on input");

        return 1;
    }

    *(line + strlen(line) - 1) = '\0';

    if (sys_escape())
        return 1;

    return 0;
}


PUBLIC int
sys_get(stream, line, maxlen, prompt)
     int             stream;
     char           *line;
     int             maxlen;
     char           *prompt;
{
    if (ext_get(stream, line, maxlen, prompt))
        return 0;
    else
        return do_get(stream, line, maxlen, prompt, 1);
}


PUBLIC int
sys_edit(stream, line, maxlen, cursor)
     int             stream;
     char            line[];
     int             maxlen;
     int             cursor;
{
    int             i;

    for (i = 0; line[i]; i++) {
        if (i == cursor)
            putchar('!');

        putchar(line[i]);
    }

    putchar('\n');

    return do_get(stream, line, maxlen, "E ");
}


PUBLIC void    *
sys_alloc(size)
     long            size;
{
    if (size > MAXUNSIGNED)
        return NULL;

    return calloc(1, (unsigned) size);
}


PUBLIC void    *
sys_realloc(block, newsize)
     void           *block;
     long            newsize;
{
    if (newsize > MAXUNSIGNED)
        return NULL;

    return realloc(block, (unsigned) newsize);
}


PUBLIC void
sys_free(p)
     void           *p;
{
    free(p);
}


PUBLIC int
sys_call_scan(id, exproot, errtext)
     struct id_rec  *id;
     struct exp_list *exproot;
     char           *errtext;
{
    int             rc = ext_call_scan(id, exproot, errtext);

    if (rc == -1) {
        sprintf(errtext, "PROCedure %s not found", id->name);
        return 0;
    }

    return rc;
}


PUBLIC int
sys_call(id, exproot, calltype, result, type)
     struct id_rec  *id;
     struct exp_list *exproot;
     int             calltype;
     void          **result;
     enum VAL_TYPE  *type;
{
    return ext_call(id, exproot, calltype, result, type);
}


PUBLIC void
sys_sys_exp(exproot, result, type)
     struct exp_list *exproot;
     void          **result;
     enum VAL_TYPE  *type;
{
    if (ext_sys_exp(exproot, result, type) == -1)
        run_error(SYS_ERR, "Unknown SYS() command");
}


PUBLIC void
sys_syss_exp(exproot, result, type)
     struct exp_list *exproot;
     char          **result;
     enum VAL_TYPE  *type;
{
    if (ext_syss_exp(exproot, result, type) == -1)
        run_error(SYS_ERR, "Unknown SYS$() command");
}




PUBLIC int
sys_sys_stat(exproot)
     struct exp_list *exproot;
{
    int             rc = ext_sys_stat(exproot);

    if (rc == -1)
        run_error(SYS_ERR, "Unknown SYS statement command");

    return rc;
}
