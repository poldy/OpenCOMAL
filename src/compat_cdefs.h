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
 * Wrapper around BSD sys/cdefs.h annotations
 * @todo copy the definitions from libbsd for Linux
 */

#ifndef COMPAT_CDEFS_H
#define COMPAT_CDEFS_H

#ifdef __APPLE__

#include <sys/cdefs.h>
#define __my_unused __unused

#else

#include <bsd/sys/cdefs.h>
#define __dead __dead2

#ifndef __my_unused
# if LIBBSD_GCC_VERSION >= 0x0300
#  define __my_unused __attribute__((unused))
# else
#  define __my_unused
# endif
#endif

#endif

#endif
