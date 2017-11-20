/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal routines to handle external segments */

#include "pdcglob.h"
#include "pdcsqash.h"
#include "pdcmisc.h"
#include "pdcscan.h"
#include "pdcexec.h"
#include "pdcexp.h"
#include "pdcprog.h"
#include "pdcseg.h"


PUBLIC void seg_total_scan(struct seg_des *seg)
{
	char errtxt[MAX_LINELEN];
	struct comal_line *errline = NULL;

	DBG_PRINTF(1, "External Total scanning...");

	if (!scan_scan(seg, errtxt, &errline))
		run_error(SCAN_ERR, "SCAN error: %s", errtxt);
}


PRIVATE void seg_proccheck(struct comal_line *mline, struct comal_line
			   *sline)
{
	struct proc_func_rec *mpf = &mline->lc.pfrec;
	struct proc_func_rec *spf = &sline->lc.pfrec;
	struct parm_list *mwalk = mpf->parmroot;
	struct parm_list *swalk = spf->parmroot;

	if (mline->cmd != sline->cmd)
		run_error(EXT_ERR,
			  "PROC/FUNC mismatch in external segment");

	if (mpf->id != spf->id)
		run_error(EXT_ERR,
			  "EXTERNAL PROC/FUNC not in external segment");

	if (mpf->closed && !spf->closed)
		run_error(EXT_ERR,
			  "CLOSED mismatch with PROC/FUNC in external segment");

	/*
	 * If the PROC/FUNC definition in the main program does not contain 
	 * a parameter list, the the parm check is OK. The external definition
	 * is the correct one.
	 */
	if (!mwalk)
		return;

	/*
	 * Compare the paremeter lists of the definitions in the main program
	 * and in the external segment.
	 */
	while (mwalk && swalk) {
		if (mwalk->ref != swalk->ref
		    || mwalk->array != swalk->array
		    || mwalk->id->type != swalk->id->type)
			run_error(EXT_ERR,
				  "Specified parameter list mismatch with external PROC/FUNC definition");

		mwalk = mwalk->next;
		swalk = swalk->next;
	}

	/*
	 * If at this point both mwalk and swalk are NULL, the parameter lists in the main
	 * program and the external segment are identical. If not, give an error...
	 */
	if ( (mwalk && !swalk) || (swalk && !mwalk) )
		run_error(EXT_ERR,
		  	"Number of parameters differ in external PROC/FUNC definition");
}


PUBLIC struct seg_des *seg_static_load(struct comal_line *line)
{
	struct proc_func_rec *pf = &line->lc.pfrec;
	struct string *name;
	enum VAL_TYPE type;
	struct seg_des *seg;

	calc_exp(pf->external->filename, (void **) &name, &type);
	seg = (struct seg_des *)mem_alloc(RUN_POOL, sizeof(struct seg_des));
	seg->lineroot = expand_fromfile(name->s);
	mem_free(name);
	seg->extdef = line;
	seg->save_localproc = line->lc.pfrec.localproc;
	seg_total_scan(seg);
	seg_proccheck(line, seg->procdef);
	seg->prev = NULL;

	pf->external->seg = seg;

	return seg;
}


PUBLIC struct seg_des *seg_dynamic_load(struct comal_line *line)
{
	struct seg_des *seg = seg_static_load(line);

	line->lc.pfrec.external->seg = NULL;
	seg->prev = curenv->segroot;
	curenv->segroot = seg;

	return seg;
}

PUBLIC struct seg_des *seg_static_free(struct seg_des *seg)
{
	prog_del(&seg->lineroot, 0, INT_MAX, 0);
	seg->extdef->lc.pfrec.localproc = seg->save_localproc;

	return (struct seg_des *)mem_free(seg);
}


PUBLIC struct seg_des *seg_dynamic_free(struct seg_des *seg)
{
	if (curenv->segroot != seg)
		fatal("Internal seg_free() error #1");

	return curenv->segroot = seg_static_free(seg);
}


PUBLIC void seg_allfree()
{
	struct seg_des *walk = curenv->segroot;
	struct comal_line *curline = curenv->progroot;

	while (walk)
		walk = seg_dynamic_free(walk);

	curenv->segroot = NULL;

	while (curline) {
		if ((curline->cmd == procSYM || curline->cmd == funcSYM || curline->cmd==moduleSYM)
		    && curline->lc.pfrec.external
		    && curline->lc.pfrec.external->seg) {
			seg_static_free(curline->lc.pfrec.external->seg);
			curline->lc.pfrec.external->seg = NULL;
		}

		curline = curline->ld->next;
	}
}

PUBLIC struct comal_line *seg_root(struct seg_des *seg) 
{
	if (seg)
		return seg->lineroot;

	return curenv->progroot;
}
