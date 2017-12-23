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
 * OpenComal program control header file
 */

#ifndef PDCPROG_H
#define PDCPROG_H

extern void prog_addline(struct comal_line *line);
extern int prog_del(struct comal_line **root, long from, long to,
		    int mainprog);
extern long prog_highest_line(void);
extern void prog_total_scan(void);
extern void prog_new(void);
extern void prog_load(char *fn);
extern void prog_run(void);

#endif
