/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal program management functions */

#include "pdcglob.h"
#include "pdcsqash.h"
#include "pdcsym.h"
#include "pdcprog.h"
#include "pdcscan.h"
#include "pdcfree.h"
#include "pdcmisc.h"
#include "pdcenv.h"
#include "pdcexec.h"
#include "pdcmod.h"


PUBLIC void prog_addline(struct comal_line *line)
{
	struct comal_line *work = curenv->progroot;
	struct comal_line *last = NULL;
	int scan = 0;

	while (work && work->ld->lineno < line->ld->lineno) {
		last = work;
		work = work->ld->next;
	}

	if (!work || work->ld->lineno > line->ld->lineno)
		line->ld->next = work;
	else {
		line->ld->next = work->ld->next;
		scan = assess_scan(work);
		line_free(work, 1);
	}

	if (last) {
		last->ld->next = line;

		if (scan_nescessary(last) == STRUCTURE_START)
			line->ld->indent = last->ld->indent + INDENTION;
		else
			line->ld->indent = last->ld->indent;
	} else
		curenv->progroot = line;

	curenv->changed = true;

	if (!scan)
		scan = assess_scan(line);

	if (scan)
		prog_structure_scan();
}


PUBLIC int prog_del(struct comal_line **root, long from, long to, int
		    mainprog)
{
	struct comal_line *work = *root;
	struct comal_line *last = NULL;
	struct comal_line *next;
	int scan = 0;

	while (work && work->ld->lineno < from) {
		last = work;
		work = work->ld->next;
	}

	while (work && work->ld->lineno <= to) {
		next = work->ld->next;

		if (!scan)
			scan = assess_scan(work);

		line_free(work, mainprog);
		work = next;
	}

	if (last)
		last->ld->next = work;
	else
		*root = work;

	return scan;
}


PUBLIC long prog_highest_line()
{
	struct comal_line *work = curenv->progroot;
	struct comal_line *last = NULL;

	if (!work)
		return 0L;

	while (work) {
		last = work;
		work = work->ld->next;
	}

	return last->ld->lineno;
}

PUBLIC void prog_total_scan()
{
	char errtxt[MAX_LINELEN];
	struct comal_line *errline = NULL;

	DBG_PRINTF(true, "Total scanning...");

	curenv->scan_ok = scan_scan(NULL, errtxt, &errline);

	if (!curenv->scan_ok) {
		if (errline)
			puts_line(MSG_ERROR, errline);

		my_printf(MSG_ERROR, true, errtxt);
	}
}


PUBLIC void prog_new()
{
	clear_env(curenv);
	mem_freepool_private(curenv->program_pool);
}


PUBLIC void prog_load(char *fn)
{
	DBG_PRINTF(true, "LOADing %s", fn);

	prog_new();
	curenv->progroot = expand_fromfile(fn);
	curenv->changed = false;
}


PUBLIC void prog_run()
{
	clean_runenv(curenv);

	if (runfilename) {
		prog_load(runfilename);
		prog_structure_scan();
		mem_free(runfilename);
		runfilename = NULL;
	}

	curenv->curenv = ROOTENV();

	prog_total_scan();

	if (curenv->scan_ok) {
		mod_initall();
		exec_seq(curenv->progroot);
	}
}
