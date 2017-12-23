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
 * OpenComal header file for Linux
 */

#ifndef PDCLINUX_H
#define PDCLINUX_H

#define HUGE_POINTER		/* no need for this in real OS's */
#define O_BINARY 	0

#define HOST_OS		"Linux"
#define HOST_OS_CODE	1	/* Change when adding another OS! */
#define VERSION		"0.2"
#define CLI		""	/* Command Line Interpreter */

#define UNIX
#define LINUX
#define FLEX

#define HAS_ROUND

#include <limits.h>
#include <unistd.h>
#include <errno.h>

#endif
