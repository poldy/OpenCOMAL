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
 * This header file contains declarations global to the entire OpenComal interpreter.
 */

#ifndef PDCGLOB_H
#define PDCGLOB_H

#include "pdcconst.h"
#include "pdcsys.h"
#include "pdcdef.h"
#include "pdcmem.h"
#include "pdcerr.h"
#include "pdcmsg.h"
#include "pdcfunc.h"

#ifndef PDCPARS
#include "pdcpars.tab.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#if defined(__APPLE__) && defined(__MACH__)
#include <xlocale.h>
#else
#include <locale.h>
#endif
#include <iconv.h>

#define PRIVATE static
#define PUBLIC

#ifndef EXTERN
#define EXTERN extern
#endif

#define NO_STRUCTURE	(0)	/* return values from scan_nescessary */
#define STRUCTURE_START	(1)	/* return values from scan_nescessary */
#define STRUCTURE_END	(2)

#define COMMAND(x)	(32767-x)	/* to distinguish between statements & command from (e.q. RUN, DEL) */

/* RESTART entry codes */

#define JUST_RESTART	1	/* Nothing special, restart interpreter */
#define QUIT		2	/* Restart code = QUIT */
#define RUN		3	/* Restart code = RUN */
#define PROG_END	4	/* Restart code = end program */
#define ERR_FATAL	666	/* fatal error occurred */

EXTERN jmp_buf RESTART;		/* restart entry in the interpreter after error */
EXTERN jmp_buf ERRBUF;		/* Continue point after run_error */

EXTERN struct comal_env *curenv;	/* Current COMAL environment */
EXTERN int entering;		/* ENTER in progress */
#ifndef NDEBUG
EXTERN int comal_debug;		/* Internal debugging switch */
#endif

EXTERN FILE *sel_outfile;	/* For select output */
EXTERN FILE *sel_infile;	/* For select input */

EXTERN const char *copyright;
EXTERN char *runfilename;

EXTERN struct env_list *env_root;

EXTERN locale_t latin_loc;
EXTERN iconv_t latin_to_utf8;

#endif
