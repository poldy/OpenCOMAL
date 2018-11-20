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
 * OpenComal symbol table and related stuff 
 */

#define _XOPEN_SOURCE 700

#include "pdcnana.h"
#include "pdcglob.h"
#include "pdcid.h"
#include "pdcstr.h"
#include "pdcmisc.h"
#include "pdclist.h"
#include "pdcval.h"
#include "pdcsym.h"

#include <string.h>
#include <stdbool.h>

PUBLIC struct sym_env *
sym_newenv(bool closed, struct sym_env *prev,
           struct sym_env *alias,
           struct comal_line *curproc, const char *name)
{
    struct sym_env *work = GETCORE(RUN_POOL, struct sym_env);

    if (comal_debug) {
        VL((false,
            "Handing out new env %s at %p, prev=%p for: ",
            name, work, prev));
        puts_line(MSG_DEBUG, curproc);
    }
    work->prev = prev;
    work->aliasenv = alias;
    work->closed = closed;
    work->itemroot = NULL;
    work->name = my_strdup(RUN_POOL, name);
    work->curproc = curproc;

    return work;
}


PUBLIC struct sym_env *
search_env(char *name, struct sym_env *start)
{
    struct sym_env *work = start;

    while (work && strcmp(name, work->name) != 0)
        work = work->prev;

    return work;
}


PUBLIC struct sym_env *
search_env_level(int level, struct sym_env *start)
{
    struct sym_env *work = start;

    while (work && proclevel(work->curproc) > level)
        work = work->prev;

    return work;
}


PUBLIC struct sym_env *
sym_newvarenv(struct sym_env *env)
{
    int             level = proclevel(env->curproc);

    while (!env->closed) {
        level--;

        while (proclevel(env->curproc) != level)
            env = env->prev;
    }

    return env;
}


PUBLIC struct sym_item *
sym_enter(struct sym_env *env, struct id_rec *id,
          enum SYM_TYPE type, void *ptr)
{
    struct sym_item *work = env->itemroot;

    DBG_PRINTF(true,
               "Entering Symbol %s (type %d) in table %p",
               id->name, type, env);

    while (work && !(work->id == id && work->symtype == type))
        work = work->next;

    if (work) {
        DBG_PRINTF(true,
                   "Illegally found SYM at %p, %s", work, work->id->name);

        return NULL;
    }

    work = GETCORE(RUN_POOL, struct sym_item);
    work->id = id;
    work->symtype = type;
    work->data.ptr = ptr;

    work->next = env->itemroot;
    env->itemroot = work;

    return work;
}


PRIVATE struct sym_item *
search_horse(struct sym_env *env, struct id_rec *id, enum SYM_TYPE type)
{
    struct sym_item *work;

    if (!env)
        return NULL;

    if (env->aliasenv)
        return search_horse(env->aliasenv, id, type);

    work = env->itemroot;

    DBG_PRINTF(true,
               "Searching Symbol %s (type %d) in table %p",
               id->name, type, env);

    while (work && !(work->id == id && work->symtype == type)) {
        DBG_PRINTF(true, "  Examining symbol %s", work->id->name);

        work = work->next;
    }

    if (!work && env->curproc && env != env->curproc->lc.pfrec.staticenv) {
        DBG_PRINTF(true, "  Not found, starting to look in static env");

        work = search_horse(env->curproc->lc.pfrec.staticenv, id, type);
    }
    if (comal_debug) {
        if (work) {
            VL((true, "Found SYM at %p, %s, type %d", work,
                work->id->name, work->symtype));
        } else {
            VL((true, "Returning NULL"));
        }
    }
    return work;
}


PUBLIC struct sym_item *
sym_search(struct sym_env *env, struct id_rec *id, enum SYM_TYPE type)
{
    int             level = proclevel(env->curproc);
    struct sym_item *ret;

    while (true) {
        ret = search_horse(env, id, type);

        if (ret || env->closed)
            break;

        level--;

        while (proclevel(env->curproc) > level)
            env = env->prev;
    }

    DBG_PRINTF(true, "Returning %p", ret);

    return ret;
}


PRIVATE void
free_var(struct var_item *var)
{
    if (!var->ref) {
        long            nritems;

        if (var->array) {
            nritems = var->array->nritems;
            free_list((struct my_list *) var->array);
        } else
            nritems = 1;

        switch (var->type) {
        case V_INT:
        case V_FLOAT:
            break;

        case V_STRING:
            for (--nritems; nritems >= 0; nritems--)
                if (var->data.str[nritems])
                    mem_free(var->data.str[nritems]);

            break;

        default:
            IP(false, "free_var default action");
        }
    }

    mem_free(var);
}


PRIVATE struct sym_item *
free_symitem(struct sym_item *item)
{
    struct sym_item *next = item->next;

    DBG_PRINTF(true, "Free item %s", item->id->name);

    switch (item->symtype) {
    case S_PROCVAR:
    case S_FUNCVAR:
        break;

    case S_VAR:
        free_var(item->data.var);
        break;

    case S_NAME:
        mem_free(item->data.name);
        break;

    default:
        IP(false, "free_symitem default action");
    }

    mem_free(item);

    return next;
}



PUBLIC struct sym_env *
sym_freeenv(struct sym_env *env, int recur)
{
    do {
        struct sym_item *work;

        DBG_PRINTF(true, "Free env %p", env);

        work = env->itemroot;

        while (work)
            work = free_symitem(work);

        mem_free(env->name);
        env->name = NULL;
        env = (struct sym_env *) mem_free(env);
    }
    while (env && recur);

    return env;
}


PRIVATE struct arr_des *
make_arrdes(struct arr_dim *arrdim)
{
    struct arr_des *arrdes = GETCORE(RUN_POOL, struct arr_des);
    struct arr_dim *walk = arrdim;
    int             nrdims = 0;
    long            nritems = 1;

    while (walk) {
        nrdims++;
        nritems = nritems * (walk->top - walk->bottom + 1);

        walk = walk->next;
    }

    arrdes->dimroot = arrdim;
    arrdes->nrdims = nrdims;
    arrdes->nritems = nritems;

    return arrdes;
}


PUBLIC struct var_item *
var_newvar(enum VAL_TYPE type, struct arr_dim *arrdim, long strlen)
{
    struct arr_des *arrdes;
    struct var_item *work;
    long            nritems;

    if (arrdim) {
        arrdes = make_arrdes(arrdim);
        nritems = arrdes->nritems;
    } else {
        arrdes = NULL;
        nritems = 1;
    }

    work =
        (struct var_item *) mem_alloc(RUN_POOL,
                                      sizeof(struct var_item) -
                                      sizeof(union var_data) +
                                      type_size(type) * nritems);

    work->type = type;
    work->ref = 0;
    work->strlen = strlen;
    work->array = arrdes;

    return work;
}

PUBLIC struct var_item *
var_refvar(struct var_item *lvar, enum VAL_TYPE type, long strlen,
           void *vref)
{
    struct var_item *work;

    DBG_PRINTF(true, "NEW RefVar pointing at %p", vref);

    work =
        (struct var_item *) mem_alloc(RUN_POOL,
                                      sizeof(struct var_item) -
                                      sizeof(union var_data) +
                                      sizeof(void *));

    work->type = lvar->type;
    work->ref = 1;
    work->strlen = strlen;
    work->data.vref = vref;

    if (type == V_ARRAY)
        work->array = lvar->array;
    else
        work->array = 0;

    return work;
}


PUBLIC struct name_rec *
name_new(struct sym_env *env, struct expression *exp)
{
    struct name_rec *work =
        (struct name_rec *) mem_alloc(RUN_POOL, sizeof(struct name_rec));

    work->env = env;
    work->exp = exp;

    return work;
}

PUBLIC void    *
var_data(struct var_item *var)
{
    if (!var->ref)
        return &var->data;

    return var->data.vref;
}

PRIVATE void
sym_list_error(struct sym_item *sym __my_unused)
{
}

PRIVATE void
sym_list_var(struct sym_item *sym)
{
    struct var_item *var = sym->data.var;

    val_print(MSG_DIALOG, &var->data, var->type);
}

PRIVATE void
sym_list_name(struct sym_item *sym __my_unused)
{
}

PRIVATE void
sym_list_proc(struct sym_item *sym __my_unused)
{
}

/*
 * Print a symbol environment 
 */
PRIVATE void
sym_list_horse(struct sym_env *env)
{
    char            line[256];
    char           *buf = line;
    struct sym_item *walk;
    const char     *s = NULL;
    void            (*fun)(struct sym_item *) = NULL;

    my_printf(MSG_DIALOG, false, "Symbol environment: ");

    if (env->curproc) {
        line_list(&buf, env->curproc);
        my_printf(MSG_DIALOG, true, "%s", line);
    } else
        my_printf(MSG_DIALOG, true, "Global");

    if (env->aliasenv)
        env = env->aliasenv;

    for (walk = env->itemroot; walk; walk = walk->next) {

        switch (walk->symtype) {
        case S_ERROR:
            s = "Error";
            fun = sym_list_error;
            break;
        case S_VAR:
            s = "Variable";
            fun = sym_list_var;
            break;
        case S_NAME:
            s = "NAME";
            fun = sym_list_name;
            break;
        case S_PROCVAR:
            s = "PROC";
            fun = sym_list_proc;
            break;
        case S_FUNCVAR:
            s = "FUNC";
            fun = sym_list_proc;
            break;
        default:
            IP(false, "sym_list_horse: Err#1");
        }

        my_printf(MSG_DIALOG, false, "  Item: %s (is %s) Value: ",
                  walk->id->name, s);
        (*fun) (walk);
        my_nl(MSG_DIALOG);
    }
}

PUBLIC void
sym_list(struct sym_env *env, int recurse)
{
    if (!env)
        return;

    do {
        sym_list_horse(env);
        env = env->prev;
    } while (env && recurse);
}
