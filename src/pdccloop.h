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

/** Return the interpreter name */
extern const char *sys_interpreter(void);

/** Process one line of input into the interpreter, either a command or a program line */
extern bool     process_comal_line(struct comal_line *line);

/** Parse a program line */
extern struct comal_line *crunch_line(char *line);

/** Main loop of interpreter, get a line and parse or execute it */
extern void     comal_loop(int newstate);

/** Entry point for development environment, including error handlers */
extern void     pdc_go(int argc, char **argv);

#endif
