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

/** Interpret C-style character escapes */
extern int lex_string_flatten(void);

/**
 * Store the string value, and parse and store the binary value, of a float.
 * @see dubbel
 */
extern int lex_floatnum(void);

/** Parse and store the binary value of an int */
extern int lex_intnum(void);

/** Parse and store an identifier which may be a COMAL built-in or a definition by the programmer */
extern int lex_id(int sym);

/** Store a remark */
extern int lex_rem(void);

/** Get the text representation of a COMAL built-in symbol */
extern const char *lex_sym(int sym);

/** Get the text representation of a COMAL built-in infix operator */
extern const char *lex_opsym(int sym);

/* These are defined in pdclex.l */
extern int lex_leng(void);
extern void lex_unput(char c);
extern int yylex(void);
extern int lex_pos(void);
extern void lex_setinput(char *line);

#endif
