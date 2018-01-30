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
 * OpenComal routines to free lines
 */

#ifndef PDCFREE_H
#define PDCFREE_H

/** Free all storage for one line of a program */
extern void line_free(struct comal_line *line, int mainprog);

#endif
