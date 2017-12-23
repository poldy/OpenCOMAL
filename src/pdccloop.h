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
 * OpenComal MAIN command loop header file
 */

#ifndef PDCCLOOP_H
#define PDCCLOOP_H

extern const char *sys_interpreter(void);
extern int process_comal_line(struct comal_line *line);
extern struct comal_line *crunch_line(char *line);
extern void comal_loop(int newstate);
extern void pdc_go(int argc, char **argv);

#endif
