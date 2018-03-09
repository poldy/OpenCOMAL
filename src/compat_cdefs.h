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

#ifndef __printflike
#define __printflike(x, y) \
        __attribute__((format(printf, (x), (y))))
#endif

#ifndef __my_unused
#define __my_unused \
        __attribute__((unused))
#endif

#endif  // __APPLE__

#ifndef __my_nonnull
#define __my_nonnull(...) \
	__attribute__((__nonnull__(__VA_ARGS__)))
#endif

#ifndef __malloc
#define __malloc \
	__attribute__((__malloc__))
#endif

#endif
