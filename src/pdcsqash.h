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
 * OpenComal line squashing functions for save/load purposes header file
 */

#ifndef PDCSQASH_H
#define PDCSQASH_H

#define SQ_COPYRIGHT_MSG	"OpenComal/Sqash (c) 1992-2002 Muppet Lab"
#define SQ_VERSION		0x1254
#define SQ_MARKER		"SqAsH"

/*
 * The sqash items 
 */

#define SQ_ERROR		0
#define SQ_STRING		1
#define SQ_LINE			2
#define SQ_INT			3
#define SQ_DOUBLE		4
#define SQ_EMPTYSTRING		5
#define SQ_EXPLIST		6
#define SQ_EMPTYEXP		7
#define SQ_EXP			8
#define SQ_EMPTYTWOEXP		9
#define SQ_EMPTYLINE		10
#define SQ_1DIMENSION		11
#define SQ_MODIFIER		12
#define SQ_1PARM		13
#define SQ_ENDEXPLIST		14
#define SQ_ID			15
#define SQ_CONTROL		16
#define SQ_REM			17
#define SQ_LD			18
#define SQ_NOEXTERNAL		19
#define SQ_ONETWOEXP		20
#define SQ_TWOEXP		21
#define SQ_IDLIST		22
#define SQ_ENDIDLIST		23

/** Tokenize & save the current program to a file */
extern void     sqash_2file(char *fname);

/** Load & detokenize a file, overwriting the current program */
extern struct comal_line *expand_fromfile(char *fname);

#endif
