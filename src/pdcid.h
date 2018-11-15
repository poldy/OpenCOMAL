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

/**
 * Compare two identifiers by their handles.
 * @return 1 if they are equal and 0 if the two identifiers are unequal
 */
extern int      id_eql(struct id_rec *id1, struct id_rec *id2);

/**
 * Search for an identifier and if it is not present installs it.
 * @return a handle for the identifier
 */
extern struct id_rec *id_search(char *id);

#endif
