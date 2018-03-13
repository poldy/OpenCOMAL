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

/** Add a line to the current program, SCANning where necessary */
extern void prog_addline(struct comal_line *line);

/**
 * Delete a line from the current program.
 * This implements the DEL command.
 */
extern int prog_del(struct comal_line **root, long from, long to,
		    int mainprog);

/** Returns the current highest line number */
extern long prog_highest_line(void);

/** SCAN the whole program */
extern void prog_total_scan(void);

/** Delete the whole current program */
extern void prog_new(void);

/**
 * Load a tokenized program from disk.
 * The current program is replaced.
 */
extern void prog_load(char *fn);

/** Run the current program, or another one stored in a file */
extern void prog_run(void);

#endif
