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

extern struct comal_env *env_new(const char *name);
extern struct comal_env *env_find(char *name);
extern void clean_runenv(struct comal_env *env);
extern void clear_env(struct comal_env *env);

#endif
