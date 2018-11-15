/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/*
 * PDComal header file for FreeBSD 
 */

#define HUGE_POINTER            /* no need for this in real OS's */
#define O_BINARY 	0

#define HOST_OS		"FreeBSD"
#define VERSION		"0.1"
#define CLI		""      /* Command Line Interpreter */

#define UNIX
#define FREEBSD
#define FLEX

#define yywrap() 1
#include <sys/errno.h>
