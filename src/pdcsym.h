/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal symbol table and related stuff header file */

#define ROOTENV		sym_newenv(1,NULL,NULL, NULL,"_program")

extern struct sym_env *sym_newenv(int closed, struct sym_env *prev,
				  struct sym_env *alias, struct 
				  comal_line *curproc, char *name);
extern struct sym_env *search_env(char *name, struct sym_env *start);
extern struct sym_env *search_env_level(int level, struct sym_env *start);
extern struct sym_env *sym_newvarenv(struct sym_env *env);
extern struct sym_item *sym_enter(struct sym_env *env, struct id_rec *id,
				  int type, void *ptr);
extern struct sym_item *sym_search(struct sym_env *env, struct id_rec *id,
				   int type);
extern struct sym_env *sym_freeenv(struct sym_env *env, int recur);
extern void sym_reparent_env(struct sym_env *env, struct sym_env *newparent);
extern struct var_item *var_newvar(int type, struct arr_dim *arrdim,
				   long strlen);
extern struct var_item *var_refvar(struct var_item *lvar, enum VAL_TYPE type, long strlen, void *vref);
extern struct name_rec *name_new(struct sym_env *env,
				 struct expression *exp);
extern void *var_data(struct var_item *var);
extern void sym_list(struct sym_env *env, int recurse);
