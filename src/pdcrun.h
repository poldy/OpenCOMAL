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

extern const char *sys_interpreter(void);
extern const char *lex_sym(int sym);
extern void line_list(char **buf, struct comal_line *line);
extern void comal_loop(int newstate);
extern void pdc_go(int argc, char **argv);

#endif
