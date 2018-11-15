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
 * OpenComal header file for UNIX
 *
 * This module should depend on the following interfaces *only*:
 *
 * - ISO C11
 * - <a href="http://pubs.opengroup.org/onlinepubs/9699919799/">X/Open 7, incorporating POSIX 2008</a>
 * - <a href="https://publications.opengroup.org/c094">X/Open Curses, Issue 7</a>
 *
 * Testing has been done on Linux and macOS.
 */

#ifndef PDCUNIX_H
#define PDCUNIX_H

#define HUGE_POINTER            /**< no need for this in real OS's */
#define O_BINARY 	0

#define HOST_OS		"UNIX"
#define HOST_OS_CODE	1       /**< Change when adding another OS! */
#define VERSION		"0.3"
#define CLI		""      /**< Command Line Interpreter */

#define FLEX

#define HAS_ROUND

#include <limits.h>
#include <unistd.h>
#include <errno.h>

#endif
