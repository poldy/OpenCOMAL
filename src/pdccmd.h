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
 * Processing of the interpreters direct commands
 */

#ifndef PDCCMD_H
#define PDCCMD_H

/** Scan all of the entered program */
extern void cmd_scan(struct comal_line *line);

/** Execute a command */
extern bool cmd_exec(struct comal_line *line);

#endif
