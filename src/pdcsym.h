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
 * OpenComal symbol table and related stuff header file
 */

#ifndef PDCSYM_H
#define PDCSYM_H

/** Create a new (optionally lexical) environment */
extern struct sym_env *sym_newenv(bool closed, struct sym_env *prev,
                                  struct sym_env *alias,
                                  struct comal_line *curproc,
                                  const char *name);

/** Search for a named environment */
extern struct sym_env *search_env(char *name, struct sym_env *start);

/** Pop the environment stack back to the specified level */
extern struct sym_env *search_env_level(int level, struct sym_env *start);

/** Pop the environment stack back to the level where variables are defined in the current PROC/FUNC */
extern struct sym_env *sym_newvarenv(struct sym_env *env);

/** Enter a new symbol in an environment */
extern struct sym_item *sym_enter(struct sym_env *env, struct id_rec *id,
                                  enum SYM_TYPE type, void *ptr);

/** Search for a symbol in the currently-visible environments */
extern struct sym_item *sym_search(struct sym_env *env, struct id_rec *id,
                                   enum SYM_TYPE type);

/** Free an environment, and optionally all subsequent environments in the list */
extern struct sym_env *sym_freeenv(struct sym_env *env, int recur);

/** Create the metadata record for a new variable */
extern struct var_item *var_newvar(enum VAL_TYPE type,
                                   struct arr_dim *arrdim, long strlen);

/** Create a new var entry for a REF variable */
extern struct var_item *var_refvar(struct var_item *lvar,
                                   enum VAL_TYPE type, long strlen,
                                   void *vref);

/** Create a new name record */
extern struct name_rec *name_new(struct sym_env *env,
                                 struct expression *exp);

/**
 * Return a pointer to the data area of a variable.
 * If this
 * variable is a REFerence variable, this function takes that
 * into account...
 */
extern void    *var_data(struct var_item *var);

/** Print a symbol environment */
extern void     sym_list(struct sym_env *env, int recurse);

/** Create a new root environment */
static inline struct sym_env *
ROOTENV(void)
{
    return sym_newenv(1, NULL, NULL, NULL, "_program");
}

#endif
