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
 * OpenComal Routines for external OpenComal modules
 */

#ifndef PDCMOD_H
#define PDCMOD_H

/** USE a module */
extern bool mod_use(struct seg_des *seg,struct id_rec *id, char *errtxt, struct comal_line **erline);

/** Find an OpenComal module */
extern struct mod_entry *mod_find(struct id_rec *id);

/**
 * Find an OpenCOMAL module in a COMAL segment.
 *
 * @note
 * Please note the absolutely confusing name of this function :-)
 */
extern struct comal_line *mod_find_def(struct seg_des *seg, struct id_rec *id);

/** Search for a PROC or FUNC in the table of exported routines */
extern struct comal_line *mod_search_routine(struct id_rec *id, int type);

/** Free the entire environment of loaded OpenComal modules */
extern void mod_freeall();

/** Initialize all registered modules */
extern void mod_initall();

#endif
