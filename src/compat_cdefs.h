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
 *
 * @todo
 * - __pure
 * - __used
 * - __warn_unused_result
 * - __bounded
 * - __returns_twice
 * - __only_inline
 * - __packed
 * - __aligned
 * - __dso_public N/A
 * - __dso_hidden N/A
 */

#ifndef COMPAT_CDEFS_H
#define COMPAT_CDEFS_H

#ifdef __APPLE__
#include <sys/cdefs.h>
#define __my_unused __unused
#else

#include <sys/cdefs.h>

#ifndef __dead
#define __dead \
        __attribute__((__noreturn__))
#endif

#ifndef __my_unused
#define __my_unused \
        __attribute__((unused))
#endif

#endif                          // __APPLE__

#ifndef __malloc
#define __malloc \
	__attribute__((__malloc__))
#endif

#endif
