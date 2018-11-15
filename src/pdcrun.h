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
 * Main file of OpenComal Command loop -- runtime only version
 */

#ifndef PDCRUN_H
#define PDCRUN_H

/** Return the interpreter name, "OpenComalRun" for this version */
extern const char *sys_interpreter(void);

/** Stub for the lexer, should never be called */
extern const char *lex_sym(int sym);

/** Stub for LIST in the development environment, should never be called */
extern void     line_list(char **buf, struct comal_line *line);

/** Stub for the command-loop in the development environment, exits the interpreter if called */
extern void     comal_loop(int newstate);

/** Run the tokenized COMAL program passed in on the command line */
extern void     pdc_go(int argc, char **argv);

#endif
