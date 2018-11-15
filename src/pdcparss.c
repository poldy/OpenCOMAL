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
 * Parser Support Routines 
 */

#define _XOPEN_SOURCE 700

#include "pdcglob.h"
#include "pdcmisc.h"
#include "pdclexs.h"
#include "pdcstr.h"
#include "pdcparss.h"

#include <stdarg.h>
#include <stdbool.h>

#include "fmt.h"

PRIVATE int     pars_error_happened = 0;
PRIVATE char    pars_errtxt[MAX_LINELEN];

static inline bool
ISARRAY(struct expression *e)
{
    return (e && (e->optype == T_ARRAY || e->optype == T_SARRAY));
}
static inline bool
ISARRAY2(struct expression *f)
{
    return (f && ISARRAY(f->e.exp));
}

PUBLIC void
yyerror(const char *s __my_unused)
{
    /*
     * No action here 
     */
}

PUBLIC struct id_list *
pars_idlist_item(struct id_rec *id, struct id_list *next)
{
    struct id_list *work =
        (struct id_list *) mem_alloc(PARSE_POOL, sizeof(struct id_list));

    work->id = id;
    work->next = next;

    return work;
}

PUBLIC struct exp_list *
pars_explist_item(struct expression *exp, struct exp_list *next)
{
    struct exp_list *work =
        (struct exp_list *) mem_alloc(PARSE_POOL, sizeof(struct exp_list));

    work->exp = exp;
    work->next = next;

    return work;
}


PUBLIC struct print_list *
pars_printlist_item(int pr_sep,
                    struct expression *exp, struct print_list *next)
{
    struct print_list *work = (struct print_list *) mem_alloc(PARSE_POOL,
                                                              sizeof(struct
                                                                     print_list));

    work->pr_sep = pr_sep;
    work->exp = exp;
    work->next = next;

    return work;
}


PUBLIC struct dim_list *
pars_dimlist_item(struct id_rec *id,
                  struct expression *strlen, struct dim_ension *root)
{
    struct dim_list *work =
        (struct dim_list *) mem_alloc(PARSE_POOL, sizeof(struct dim_list));

    work->id = id;
    work->strlen = strlen;
    work->dimensionroot = (struct dim_ension *) my_reverse(root);

    return work;
}


PUBLIC struct when_list *
pars_whenlist_item(int op, struct expression *exp, struct when_list *next)
{
    struct when_list *work = (struct when_list *) mem_alloc(PARSE_POOL,
                                                            sizeof(struct
                                                                   when_list));

    work->op = op;
    work->exp = exp;
    work->next = next;

    return work;
}


PUBLIC struct assign_list *
pars_assign_item(int op, struct expression *lval, struct expression *rval)
{
    struct assign_list *work = (struct assign_list *) mem_alloc(PARSE_POOL,
                                                                sizeof
                                                                (struct
                                                                 assign_list));

    if (op != becomesSYM && (ISARRAY(lval) || ISARRAY2(rval)))
        pars_error
            ("Semi-complex assignment (e.g. :+ and :-) not supported with arrays");

    work->op = op;
    work->lval = lval;
    work->exp = rval;
    work->next = NULL;

    return work;
}


#define GETEXP(x) struct expression *work=(struct expression *)mem_alloc(PARSE_POOL,sizeof(int)+sizeof(enum optype)+(x))

PUBLIC struct expression *
pars_exp_const(int op)
{
    GETEXP(0);

    work->optype = T_CONST;
    work->op = op;

    return work;
}

PUBLIC struct expression *
pars_exp_unary(int op, struct expression *exp)
{
    GETEXP(sizeof(struct expression *));

    work->optype = T_UNARY;

    work->op = op;
    work->e.exp = exp;

    if (exp && ISARRAY(exp))
        pars_error("Arrays may not be part of an expression");

    return work;
}


PUBLIC struct expression *
pars_exp_sys(int sym, enum optype type, struct exp_list *exproot)
{
    GETEXP(sizeof(struct exp_list *));

    work->optype = type;
    work->op = sym;

    work->e.exproot = exproot;

    return work;
}


PUBLIC struct expression *
pars_exp_binary(int op, struct expression *exp1, struct expression *exp2)
{
    GETEXP(sizeof(struct two_exp));

    work->optype = T_BINARY;
    work->op = op;
    work->e.twoexp.exp1 = exp1;
    work->e.twoexp.exp2 = exp2;

    if (ISARRAY(exp1) || ISARRAY(exp2))
        pars_error("Arrays may not be part of an expression");

    return work;
}

PUBLIC struct expression *
pars_exp_int(long num)
{
    GETEXP(sizeof(long));

    work->optype = T_INTNUM;
    work->e.num = num;

    return work;
}

PUBLIC struct expression *
pars_exp_float(struct dubbel *d)
{
    GETEXP(sizeof(struct dubbel));

    work->optype = T_FLOAT;
    work->e.fnum.val = d->val;
    work->e.fnum.text = my_strdup(PARSE_POOL, d->text);

    return work;
}

PUBLIC struct expression *
pars_exp_string(struct string *str)
{
    GETEXP(sizeof(struct string *));

    work->optype = T_STRING;
    work->e.str = str;

    return work;
}

PUBLIC struct expression *
pars_exp_id(int op, struct id_rec *id, struct exp_list *exproot)
{
    GETEXP(sizeof(struct exp_id));

    work->optype = T_ID;
    work->op = op;
    work->e.expid.id = id;
    work->e.expid.exproot = (struct exp_list *) my_reverse(exproot);

    return work;
}

PUBLIC struct expression *
pars_exp_array(int op, struct id_rec *id, enum optype type)
{
    GETEXP(sizeof(struct exp_id));

    work->optype = type;
    work->op = op;
    work->e.expid.id = id;

    return work;
}


PUBLIC struct expression *
pars_exp_sid(struct id_rec *id,
             struct exp_list *exproot, struct two_exp *twoexp)
{
    GETEXP(sizeof(struct exp_sid));

    work->optype = T_SID;
    work->op = stringidSYM;
    work->e.expsid.id = id;
    work->e.expsid.exproot = (struct exp_list *) my_reverse(exproot);

    if (!twoexp)
        work->e.expsid.twoexp = NULL;
    else {
        work->e.expsid.twoexp =
            (struct two_exp *) mem_alloc(PARSE_POOL,
                                         sizeof(struct two_exp));
        *work->e.expsid.twoexp = *twoexp;
    }

    return work;
}


PUBLIC struct expression *
pars_exp_substr(struct expression *exp, struct two_exp *twoexp)
{
    GETEXP(sizeof(struct exp_substr));

    if (ISARRAY(exp))
        pars_error("You may not take a substring of an array in this way");


    work->optype = T_SUBSTR;
    work->e.expsubstr.exp = exp;
    work->e.expsubstr.twoexp = *twoexp;

    return work;
}


PUBLIC struct expression *
pars_exp_num(struct expression *numexp)
{
    GETEXP(sizeof(struct expression *));

    work->optype = T_EXP_IS_NUM;
    work->e.exp = numexp;

    return work;
}


PUBLIC struct expression *
pars_exp_str(struct expression *strexp)
{
    GETEXP(sizeof(struct expression *));

    work->optype = T_EXP_IS_STRING;
    work->e.exp = strexp;

    return work;
}


PUBLIC void
pars_error(const char *s, ...)
{
    va_list_box     box;

    if (pars_error_happened)
        return;

    va_start(box.ap, s);

    pars_error_happened = lex_pos();
    Fmt_vsfmt(pars_errtxt, MAX_LINELEN, s, &box);
    va_end(box.ap);
}


PUBLIC int
pars_handle_error(void)
{
    int             i = pars_error_happened;

    if (i) {
        my_printf(MSG_ERROR, true, "%s", pars_errtxt);
        mem_freepool(PARSE_POOL);
        pars_error_happened = 0;
    }

    return i;
}
