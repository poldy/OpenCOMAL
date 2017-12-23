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
 * Header file for lex support routines
 */

#ifndef PDCLEXS_H
#define PDCLEXS_H

extern int lex_string_flatten(void);
extern int lex_floatnum(void);
extern int lex_intnum(void);
extern int lex_id(int sym);
extern int lex_rem(void);
extern const char *lex_sym(int sym);
extern const char *lex_opsym(int sym);

/* These are defined in pdclex.l */
extern int lex_leng();
extern void lex_unput(char c);
extern int yylex();
extern int lex_pos();
extern void lex_setinput(char *line);

#endif
