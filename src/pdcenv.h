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
 * OpenComal environment handling header file
 */

#ifndef PDCENV_H
#define PDCENV_H

/** Create a new environment */
extern struct comal_env *env_new(const char *name);

/** Return a named environment */
extern struct comal_env *env_find(char *name);

/** Return the current program to its initial state */
extern void clean_runenv(struct comal_env *env);

/** Totally clear an environment, including the current program */
extern void clear_env(struct comal_env *env);

#endif
