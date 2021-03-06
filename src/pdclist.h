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
 * OpenComal routines to list program lines
 */

#ifndef PDCLIST_H
#define PDCLIST_H

/** List one program line */
extern void     line_list(char **buf, struct comal_line *line);

#endif
