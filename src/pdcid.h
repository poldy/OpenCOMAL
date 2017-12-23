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
 * OpenComal id routines header file
 */

#ifndef PDCID_H
#define PDCID_H

extern int id_eql(struct id_rec *id1, struct id_rec *id2);
extern struct id_rec *id_search(char *id);

#endif
