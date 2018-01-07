/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal routines to free lines */

#include "pdcglob.h"
#include "pdclexs.h"
#include "pdcparss.h"
#include "pdcmisc.h"
#include "pdcseg.h"
#include "pdcsym.h"
#include "pdcfree.h"


PRIVATE void free_exp(struct expression *exp);
PRIVATE void free_horse(struct comal_line *line);


PRIVATE void free_explist(struct exp_list *exproot)
{
	while (exproot) {
		free_exp(exproot->exp);
		exproot = (struct exp_list *)mem_free(exproot);
	}
}


PRIVATE void free_twoexp(struct two_exp *twoexp)
{
	free_exp(twoexp->exp1);
	
	if (twoexp->exp2!=twoexp->exp1)
		free_exp(twoexp->exp2);

	twoexp->exp1=0;
	twoexp->exp2=0;
}


PRIVATE void free_exp(struct expression *exp)
{
	if (!exp)
		return;

	switch (exp->optype) {
	case T_CONST:
	case T_INTNUM:
	case T_ARRAY:
	case T_SARRAY:
		break;

	case T_FLOAT:
		mem_free(exp->e.fnum.text);
		break;

	case T_UNARY:
		free_exp(exp->e.exp);
		break;

	case T_SYS:
	case T_SYSS:
		free_explist(exp->e.exproot);
		break;

	case T_SUBSTR:
		free_exp(exp->e.expsubstr.exp);
		free_twoexp(&exp->e.expsubstr.twoexp);
		break;

	case T_ID:
		free_explist(exp->e.expid.exproot);
		break;

	case T_BINARY:
		free_twoexp(&exp->e.twoexp);
		break;

	case T_STRING:
		mem_free(exp->e.str);
		break;

	case T_SID:
		free_explist(exp->e.expsid.exproot);

		if (exp->e.expsid.twoexp) {
			free_twoexp(exp->e.expsid.twoexp);
			mem_free(exp->e.expsid.twoexp);
		}

		break;

	case T_EXP_IS_NUM:
	case T_EXP_IS_STRING:
		free_exp(exp->e.exp);
		break;

	default:
		fatal("Free exp default action");
	}

	mem_free(exp);
}


PRIVATE void free_ifwhile(struct comal_line *line)
{
	free_exp(line->lc.ifwhilerec.exp);
	free_horse(line->lc.ifwhilerec.stat);
}


PRIVATE void free_dim(struct comal_line *line)
{
	struct dim_list *work = line->lc.dimroot;

	while (work) {
		if (work->dimensionroot) {
			struct dim_ension *work2 = work->dimensionroot;

			while (work2) {
				free_exp(work2->bottom);
				free_exp(work2->top);
				work2 = (struct dim_ension *)mem_free(work2);
			}
		}

		free_exp(work->strlen);

		work = (struct dim_list *)mem_free(work);
	}
}


PRIVATE void free_for(struct comal_line *line)
{
	struct for_rec *f = &line->lc.forrec;

	if (line->lineptr)
		line->lineptr->lineptr = NULL;

	free_exp(f->lval);
	free_exp(f->from);
	free_exp(f->to);
	free_exp(f->step);
	free_horse(f->stat);
}


PRIVATE void free_input(struct comal_line *line)
{
	struct input_rec *i = &line->lc.inputrec;

	if (i->modifier) {
		switch (i->modifier->type) {
		case fileSYM:
			free_twoexp(&i->modifier->twoexp);
			break;

		case stringSYM:
			mem_free(i->modifier->str);
			break;

		case atSYM:
			free_twoexp(&i->modifier->twoexp);
			if (i->modifier->len != NULL) {
				free_exp(i->modifier->len);
			}
			if (i->modifier->str != NULL) {
				mem_free(i->modifier->str);
			}
			break;

		default:
			fatal("Input modifier incorrect (free)");
		}

		mem_free(i->modifier);
	}

	free_explist(i->lvalroot);
}


PRIVATE void free_printlist(struct print_list *printroot)
{
	while (printroot) {
		free_exp(printroot->exp);
		printroot = (struct print_list *)mem_free(printroot);
	}
}


PRIVATE void free_print(struct comal_line *line)
{
	struct print_rec *p = &line->lc.printrec;

	if (p->modifier) {
		switch (p->modifier->type) {
		case fileSYM:
			free_twoexp(p->modifier->data.twoexp);
			break;

		case usingSYM:
			free_exp(p->modifier->data.str);
			break;

		case atSYM:
			free_twoexp(p->modifier->data.twoexp);
			break;

		default:
			fatal("Print modifier incorrect (free)");
		}

		mem_free(p->modifier);
	}

	free_printlist(p->printroot);
}


PRIVATE void free_assign(struct assign_list *assignroot)
{
	while (assignroot) {
		free_exp(assignroot->lval);
		free_exp(assignroot->exp);
		assignroot = (struct assign_list *)mem_free(assignroot);
	}
}


PRIVATE void free_whenlist(struct when_list *whenroot)
{
	while (whenroot) {
		free_exp(whenroot->exp);
		whenroot = (struct when_list *)mem_free(whenroot);
	}
}


PRIVATE void free_horse(struct comal_line *line)
{
	if (!line)
		return;
#ifndef NDEBUG
        if (comal_debug) {
	        DBG_PRINTF(0, "Freeing line: ");
	        puts_line(MSG_DEBUG, line);
        }
#endif
	switch (line->cmd) {
	case 0:
	case quitSYM:
	case newSYM:
	case contSYM:
	case elseSYM:
	case endSYM:
	case endcaseSYM:
	case endifSYM:
	case endloopSYM:
	case endwhileSYM:
	case otherwiseSYM:
	case nullSYM:
	case retrySYM:
	case trapSYM:
	case pageSYM:
	case handlerSYM:
	case restoreSYM:
	case idSYM:
	case loopSYM:
	case endtrapSYM:
	case endforSYM:
	case endprocSYM:
	case endfuncSYM:
	case endmoduleSYM:
		break;

	case execSYM:
	case caseSYM:
	case runSYM:
	case delSYM:
	case chdirSYM:
	case mkdirSYM:
	case rmdirSYM:
	case stopSYM:
	case osSYM:
	case dirSYM:
	case unitSYM:
	case select_outputSYM:
	case select_inputSYM:
	case returnSYM:
	case elifSYM:
	case exitSYM:
	case traceSYM:
	case untilSYM:
	case randomizeSYM:
	case reportSYM:
	case delaySYM:
	case zoneSYM:
		free_exp(line->lc.exp);
		break;

	case closeSYM:
	case sysSYM:
	case dataSYM:
		free_explist(line->lc.exproot);
		break;

	case cursorSYM:
		free_twoexp(&line->lc.twoexp);
		break;

	case useSYM:
	case exportSYM:
		free_list((struct my_list *)line->lc.idroot);
		break;

	case staticSYM:
	case localSYM:
	case dimSYM:
		free_dim(line);
		break;

	case forSYM:
		free_for(line);
		break;

	case funcSYM:
	case procSYM:
	case moduleSYM:
		if (line->lineptr)
			line->lineptr->lineptr = NULL;

		if (line->lc.pfrec.external) {
			if (line->lc.pfrec.external->seg)
				seg_static_free(line->lc.pfrec.external->
						seg);

			free_exp(line->lc.pfrec.external->filename);
			mem_free(line->lc.pfrec.external);
		}

		if (line->lc.pfrec.staticenv)
			sym_freeenv(line->lc.pfrec.staticenv,0);

		free_list((struct my_list *) line->lc.pfrec.parmroot);
		break;

	case importSYM:
		free_list((struct my_list *) line->lc.importrec.
			  importroot);
		break;

	case ifSYM:
		free_ifwhile(line);
		break;

	case inputSYM:
		free_input(line);
		break;

	case openSYM:
		free_exp(line->lc.openrec.filenum);
		free_exp(line->lc.openrec.filename);
		free_exp(line->lc.openrec.reclen);
		break;
	
	case createSYM:
		free_exp(line->lc.createrec.filename);
		free_exp(line->lc.createrec.top);
		free_exp(line->lc.createrec.reclen);
		break;

	case printSYM:
		free_print(line);
		break;

	case readSYM:
		if (line->lc.readrec.modifier) {
			free_twoexp(line->lc.readrec.modifier);
			mem_free(line->lc.readrec.modifier);
		}

		free_explist(line->lc.readrec.lvalroot);
		break;

	case whenSYM:
		free_whenlist(line->lc.whenroot);
		break;

	case repeatSYM:
	case whileSYM:
		free_ifwhile(line);
		break;

	case writeSYM:
		free_twoexp(&line->lc.writerec.twoexp);
		free_explist(line->lc.writerec.exproot);
		break;

	case becomesSYM:
		free_assign(line->lc.assignroot);
		break;

	default:
		fatal("List default action (free)");
	}

	if (line->ld) {
		if (line->ld->rem)
			mem_free(line->ld->rem);

		mem_free(line->ld);
	}

	mem_free(line);
}


PUBLIC void line_free(struct comal_line *line, int mainprog)
{
	free_horse(line);

	if (mainprog)
		curenv->changed = 1;
}
