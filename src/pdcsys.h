/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal header file for OS dependent routines */

#ifndef PDCSYS_H
#define PDCSYS_H

// #define INT_MAX               (~(1L<<(8*sizeof(long)-1)))
#define MAXUNSIGNED	((unsigned)~0)
// #define INT_MIN               (1L<<(8*sizeof(long)-1))

#include "pdcdsys.h"

#ifdef OS_LINUX
#include "pdclinux.h"
#endif

#ifdef OS_WIN32
#include "pdcwin32.h"
#endif

#ifdef OS_MSDOS
#include "pdcmsdos.h"
#endif

#endif
