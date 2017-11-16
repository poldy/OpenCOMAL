/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

#ifndef PDCCMD_H
#define PDCCMD_H

extern int cmd_scan(struct comal_line *line);
extern int cmd_exec(struct comal_line *line, int *result);

#endif
