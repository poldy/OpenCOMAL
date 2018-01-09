/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal program management */

#include "pdcglob.h"
#include "pdclexs.h"
#include "pdcmisc.h"
#include "pdcid.h"
#include "pdcsym.h"
#include "pdcmod.h"
#include "pdcseg.h"
#include "msgnrs.h"
#include "pdcscan.h"
#include "pdcstr.h"

#include <string.h>

#define SCAN_STACK_SIZE		(MAX_INDENT)

PRIVATE struct {
	int sym;
	struct comal_line *line;
} scan_stack[SCAN_STACK_SIZE];
PRIVATE int scan_sp;

enum scan_entry_special { NOT_SPECIAL, SHORT_FORM, EXIT, PROCFUNC, EPROCFUNC,
	DATA_STAT, CASE, RETRY, EXPORT, USE
};

struct scan_entry {
	int sym;
	int atindent0;
	int leavessym;
	int expectsym1;
	int expectsym2;
	enum scan_entry_special special;
};


PRIVATE struct scan_entry scan_tab[] = {
	{caseSYM, 0, whenSYM, 0, 0, CASE},
	{dataSYM, 0, 0, 0, 0, DATA_STAT},
	{elifSYM, 0, elifSYM, elifSYM, 0, NOT_SPECIAL},
	{elseSYM, 0, endifSYM, elifSYM, 0, NOT_SPECIAL},
	{endcaseSYM, 0, 0, whenSYM, endcaseSYM, NOT_SPECIAL},
	{endforSYM, 0, 0, endforSYM, 0, NOT_SPECIAL},
	{endfuncSYM, 0, 0, endfuncSYM, 0, EPROCFUNC},
	{endifSYM, 0, 0, endifSYM, elifSYM, NOT_SPECIAL},
	{endloopSYM, 0, 0, endloopSYM, 0, NOT_SPECIAL},
	{endprocSYM, 0, 0, endprocSYM, 0, EPROCFUNC},
	{endmoduleSYM, 0, 0, endmoduleSYM, 0, EPROCFUNC},
	{endtrapSYM, 0, 0, endtrapSYM, handlerSYM, NOT_SPECIAL},
	{endwhileSYM, 0, 0, endwhileSYM, 0, NOT_SPECIAL},
	{exitSYM, 0, 0, 0, 0, EXIT},
	{exportSYM, 0, 0, 0, 0, EXPORT},
	{forSYM, 0, endforSYM, 0, 0, SHORT_FORM},
	{funcSYM, 0, endfuncSYM, 0, 0, PROCFUNC},
	{handlerSYM, 0, endtrapSYM, handlerSYM, 0, NOT_SPECIAL},
	{ifSYM, 0, elifSYM, 0, 0, SHORT_FORM},
	{loopSYM, 0, endloopSYM, 0, 0, NOT_SPECIAL},
	{otherwiseSYM, 0, endcaseSYM, whenSYM, 0, NOT_SPECIAL},
	{procSYM, 0, endprocSYM, 0, 0, PROCFUNC},
	{moduleSYM, 1, endmoduleSYM, 0, 0, PROCFUNC},
	{retrySYM, 0, 0, 0, 0, RETRY},
	{repeatSYM, 0, untilSYM, 0, 0, SHORT_FORM},
	{trapSYM, 0, handlerSYM, 0, 0, NOT_SPECIAL},
	{untilSYM, 0, 0, untilSYM, 0, NOT_SPECIAL},
	{useSYM, 0, 0, 0, 0, USE},
	{whenSYM, 0, whenSYM, whenSYM, 0, NOT_SPECIAL},
	{whileSYM, 0, endwhileSYM, 0, 0, SHORT_FORM},
	{0, 0, 0, 0, 0, NOT_SPECIAL}
};


PRIVATE void scan_stack_push(int sym, struct comal_line *line)
{
	if (scan_sp >= SCAN_STACK_SIZE)
		fatal("Internal scan stack overflow");

	scan_stack[scan_sp].sym = sym;
	scan_stack[scan_sp].line = line;
	scan_sp++;
}


PRIVATE void scan_stack_peek(int *sym, struct comal_line **line)
{
	if (!scan_sp) {
		*sym = -1;
		*line = NULL;
	} else {
		*sym = scan_stack[scan_sp - 1].sym;
		*line = scan_stack[scan_sp - 1].line;
	}
}


PRIVATE void scan_stack_pop(int *sym, struct comal_line **line)
{
	scan_stack_peek(sym, line);

	if (*sym != -1)
		--scan_sp;
}


PRIVATE void scan_stack_search(int sym1, int sym2, int sym3, int *result,
			       struct comal_line **stkline)
{
	int i;

	for (i = scan_sp - 1; i >= 0; i--) {
		int sym;

		sym=scan_stack[i].sym;

		if (sym == sym1 || sym == sym2 || sym==sym3) {
			*result = sym;

			if (stkline)
				*stkline = scan_stack[i].line;

			return;
		}
	}

	*result = 0;
	*stkline = NULL;

	return;
}


PRIVATE struct comal_line *routine_search_horse(struct id_rec *id,
						int type,
						struct comal_line *root)
{
	while (root) {
		if (root->cmd == type && root->lc.pfrec.id == id)
			return root;

		root = root->lc.pfrec.proclink;
	}

	return NULL;
}


PRIVATE struct comal_line *routine_search(struct id_rec *id, int type,
					  struct comal_line *curproc)
{
	struct comal_line *procline;
	struct comal_line *father = curproc;

	while (father) {
		procline =
		    routine_search_horse(id, type,
					 father->lc.pfrec.localproc);

		if (procline)
			return procline;

		father = father->lc.pfrec.fatherproc;
	}

	procline=routine_search_horse(id, type, curenv->globalproc);

	if (!procline)
		procline=mod_search_routine(id,type);

	return procline;
}


/*
 * SCAN pass 2
 *
 * Traverse the entire segment and find all USEs
 */
PRIVATE int scan_pass2(struct seg_des *seg, char *errtxt,
		       struct comal_line **errline)
{
	struct comal_line *curline;
	struct id_list *walk;

	*errline=NULL;

	FOR_EACH_LINE(seg,curline) 
		if (curline->cmd == useSYM)
			/*
			 * Process each identifier of the USE statement
			 */
			for (walk=curline->lc.idroot; walk; walk=walk->next)
				if (!mod_use(seg,walk->id,errtxt,errline)) {
					if (!*errline) *errline=curline;
					return 0;
				}

	return 1;
}


PRIVATE int scan_pass4(struct seg_des *seg, char *errtxt,
		       struct comal_line **errline)
{
	struct comal_line *curline;
	struct comal_line *theline;
	struct comal_line *procline = NULL;
	struct comal_line *walkline;
	struct comal_line *walk;
	struct parm_list *pwalk;
	struct exp_id *proccall;
	const char *err = NULL;
	int dummy;
	int procfound;
	int sp;

	if (seg)
		curline = seg->lineroot;
	else
		curline = curenv->progroot;

	scan_sp = 0;

	while (curline) {
		theline = line_2line(curline);

		if (!theline)
			theline = curline;

		if (theline->cmd == procSYM || theline->cmd == funcSYM || theline->cmd == moduleSYM) {
			scan_stack_push(0, procline);
			procline = theline;
		} else if (theline->cmd == endprocSYM
			   || theline->cmd == endfuncSYM || theline->cmd==endmoduleSYM)
			scan_stack_pop(&dummy, &procline);
		else if (seg && !procline && theline->cmd != 0) {
			term_strncpy(errtxt,
			       "Non-// program lines in external segment outside PROC/FUNC def", MAX_LINELEN);
			*errline = curline;
			return 0;
		} else if (theline->cmd == importSYM) {
			if (!procline) // || !procline->lc.pfrec.closed)
				err =
				    "IMPORT only allowed within PROCs/FUNCs or MODULEs";
		} else if (theline->cmd == localSYM) {
			if (!procline || procline->lc.pfrec.closed)
				err =
				    "LOCAL only allowed within open PROCs/FUNCs";
		} else if (theline->cmd == staticSYM) {
			if (!procline || procline->cmd == moduleSYM)
				err =
				    "STATIC only inside a PROC or FUNC please";
		} else if (theline->cmd == returnSYM) {
			if (!procline)
				err =
				    "Can only RETURN from inside a PROC, FUNC or MODULE";
			else if (procline->cmd == funcSYM) {
				if (!theline->lc.exp)
					err =
					    "This statement (inside a FUNC) must RETURN an expression";
				else if (!type_match1
					 (procline->lc.pfrec.id,
					  theline->lc.exp))
					err =
					    "This statement RETURNs the wrong type of expression";
			} else if (theline->lc.exp)
				err =
				    "This statement (inside a PROC or MODULE) must not RETURN an expression";
		} else if (theline->cmd == execSYM) {
			proccall = &theline->lc.exp->e.expid;

			if (!routine_search
			    (proccall->id, procSYM, procline)) {
				procfound = 0;

				if (procline) {
					sp = scan_sp;
					walkline = procline;

					while (!procfound && walkline) {
						pwalk =
						    walkline->lc.pfrec.
						    parmroot;

						while (pwalk
						       && pwalk->id !=
						       proccall->id)
							pwalk =
							    pwalk->next;

						procfound = (pwalk
							     && pwalk->
							     ref ==
							     procSYM);

						if (!sp)
							walkline = NULL;
						else {
							sp--;
							walkline =
							    scan_stack[sp].
							    line;
						}
					}
				}

				if (!procfound
				    && !sys_call_scan(proccall->id,
						      proccall->exproot,
						      errtxt)) {
					*errline = curline;
					return 0;
				}
			}
		} else if (theline->cmd == restoreSYM) {
			if (seg) {
				walk = curenv->progroot;

				while (walk && walk->cmd != dataSYM)
					walk = walk->ld->next;

				if (!walk)
					err =
					    "RESTORE in external segment without any DATA line in main program";

				theline->lineptr = walk;
			} else if (!curenv->datalptr)
				err =
				    "RESTORE without any DATA line in program";
			else if (theline->lc.id) {
				walk = curenv->progroot;

				while (walk
				       && !(walk->cmd == idSYM
					    && walk->lc.id ==
					    theline->lc.id))
					walk = walk->ld->next;

				if (!walk)
					err = "Label not found";
				else {
					while (walk
					       && walk->cmd != dataSYM)
						walk = walk->ld->next;

					if (!walk)
						err =
						    "No DATA statements follow the label";
				}

				theline->lineptr = walk;
			} else
				theline->lineptr = curenv->datalptr;

		}

		if (err) {
			term_strncpy(errtxt, err, MAX_LINELEN);
			*errline = curline;
			return 0;
		}

		curline = curline->ld->next;
	}

	return 1;
}


PRIVATE int check_case(struct comal_line *fromline, char *errtxt,
		       struct comal_line **errline)
{
	do
		fromline = fromline->ld->next;
	while (fromline && fromline->cmd == 0);

	if (fromline->cmd != whenSYM) {
		*errline = fromline;
		term_strncpy(errtxt, "CASE should be followed by WHEN", MAX_LINELEN);
		return 0;
	}

	return 1;
}


PRIVATE int do_special1(struct scan_entry *p, struct comal_line *theline,
			struct comal_line *curline,
			struct comal_line **errline, char *errtxt,
			int *level, struct comal_line **procroot,
			struct seg_des *seg)
{
	int sym;
	struct comal_line *lineptr;
	int i;

	switch (p->special) {
	case CASE:
		return check_case(theline, errtxt, errline);

	case DATA_STAT:
		if (seg) {
			term_strncpy(errtxt,
			       "DATA not allowed in external segment", MAX_LINELEN);
			return 0;
		}

		if (curenv->datalptr)
			break;

		curenv->datalptr = curline;
		curenv->dataeptr = curline->lc.exproot;

		break;

	case EXIT:
		scan_stack_search(endloopSYM, -1, -1, &sym, &lineptr);

		if (sym == 0) {
			term_strncpy(errtxt, "Can only EXIT from within a loop", MAX_LINELEN);
			*errline = curline;
			return 0;
		}

		break;

	/*
	 * An EXPORT may only be done on the most outer level of a MODULE.
	 */
	case EXPORT:
		if (scan_sp !=1 || scan_stack[scan_sp-1].sym != endmoduleSYM) {
			term_strncpy(errtxt, "Can not have EXPORT here", MAX_LINELEN);
			*errline = curline;
			return 0;
		}

		break;

	/*
	 * A USE may be done on the most outer level of the main program or a
	 * PROC, FUNC or MODULE
	 */
	case USE:
		if (scan_sp>0) {
			sym=scan_stack[scan_sp-1].sym;

			if (sym!=endprocSYM && sym!=endfuncSYM && sym!=endmoduleSYM) {
				term_strncpy(errtxt, "Can not have USE here", MAX_LINELEN);
				*errline = curline;
				return 0;
			}
		}

		break;

	case RETRY:
		scan_stack_search(endtrapSYM, -1, -1, &sym, &lineptr);

		if (sym == 0) {
			term_strncpy(errtxt, "Can only RETRY from within the HANDLER part of a TRAP/ENDTRAP", MAX_LINELEN);
			*errline = curline;
			return 0;
		}

		break;


	case PROCFUNC:
		if (routine_search_horse
		    (theline->lc.pfrec.id, theline->cmd, *procroot)) {
			snprintf(errtxt, MAX_LINELEN, "%s %s multiply defined",
				lex_sym(theline->cmd),
				theline->lc.pfrec.id->name);
			*errline = curline;
			return 0;
		}

		/*
		 * Search the scanning stack to see whether this PROC/FUNC definition
		 * is nested inside another control structure (not being another PROC,
		 * FUNC or MODULE.
		 */
		for (i=scan_sp-1; i>=0; i--) {
			sym=scan_stack[i].sym;

			/*
			 * The check for sym!=0 is because scan_pass4 pushes a 0 on the scan
			 * stack when it encounters a proc/func. I can't remember why I did
			 * that, but it's probably for a good cause, so better check it here
			 */
			if (sym!=0 && sym!=endprocSYM && sym!=endfuncSYM && sym!=endmoduleSYM) {
				term_strncpy(errtxt, "PROC/FUNC may not be defined here", MAX_LINELEN);
				*errline = curline;
				return 0;
			}
		}

		theline->lc.pfrec.proclink = *procroot;
		theline->lc.pfrec.level = *level;
		scan_stack_search(endprocSYM, endfuncSYM, endmoduleSYM, &sym, &lineptr);

		if (seg && !lineptr)
			theline->lc.pfrec.fatherproc =
			    seg->extdef->lc.pfrec.fatherproc;
		else
			theline->lc.pfrec.fatherproc = lineptr;

		if (!theline->lc.pfrec.external) {
			*procroot = NULL;
			(*level)++;
		} else
			*procroot = theline;

		break;

	case EPROCFUNC:
		scan_stack_peek(&sym, &lineptr);

		if (sym == endprocSYM || sym == endfuncSYM || sym==endmoduleSYM) {
			lineptr->lc.pfrec.localproc = *procroot;
			*procroot = lineptr;
			(*level)--;
		}

		break;

	case NOT_SPECIAL:
	case SHORT_FORM:
		break;
	}

	return 1;
}


PUBLIC int scan_scan(struct seg_des *seg, char *errtxt,
		     struct comal_line **errline)
{
	struct comal_line *curline;
	struct comal_line *theline;
	struct comal_line *lineptr;
	struct scan_entry *p;
	int sym;
	int cmd2;
	struct comal_line *procroot = NULL;
	int level;

	if (seg) {
		curline = seg->lineroot;
		level = seg->extdef->lc.pfrec.level;
	} else {
		curline = curenv->progroot;
		level = 0;
	}

	scan_sp = 0;
	*errline=NULL;
	*errtxt=0;

	while (curline) {
		int skip_processing;

		theline = line_2line(curline);

		if (theline)
			cmd2 = theline->cmd;
		else {
			theline = curline;
			cmd2 = 0;
		}

		skip_processing = 0;

		if (scan_sp == MAX_INDENT) {
			snprintf(errtxt, MAX_LINELEN,
				"Too much control structure nesting (max. %d)",
				MAX_INDENT);
			*errline = curline;
			return 0;
		}

		for (p = scan_tab; p->sym && p->sym != theline->cmd; p++);

		if (p->sym) {
			if ((p->special == SHORT_FORM && cmd2) ||
			    (p->special == PROCFUNC
			     && theline->lc.pfrec.external)
			    || (theline->cmd == trapSYM
				&& theline->lc.traprec.esc))
				skip_processing = 1;	/* Prevent indention */

			/*
			 * Check whether this symbol should only occur at indent 0
			 */
			if (p->atindent0 && scan_sp!=0) {
				snprintf(errtxt,MAX_LINELEN,"Can not have %s here",lex_sym(p->sym));
				*errline=curline;
				return 0;
			}

			if (!do_special1
			    (p, theline, curline, errline, errtxt, &level,
			     &procroot, seg))
				return 0;

			if (!skip_processing) {
				if (p->expectsym1 || p->expectsym2) {
					scan_stack_pop(&sym, &lineptr);

					if (sym != p->expectsym1
					    && sym != p->expectsym2) {
						if (sym != -1)
							snprintf(errtxt,
                                                                MAX_LINELEN,
								"%s expected, not %s",
								lex_sym
								(sym),
								lex_sym
								(theline->
								 cmd));
						else
							snprintf(errtxt,
                                                                MAX_LINELEN,
								"Unexpected %s",
								lex_sym
								(theline->
								 cmd));

						*errline = curline;
						return 0;
					} else {
						theline->lineptr = lineptr;
						lineptr->lineptr = theline;
					}
				}

				if (p->leavessym)
					scan_stack_push(p->leavessym,
							curline);
			}
		}

		curline = curline->ld->next;
	}

	if (scan_sp) {
		snprintf(errtxt, MAX_LINELEN, "%d open control structure%s at EOP\n",
			scan_sp, scan_sp == 1 ? "" : "s");
		return 0;
	}

	if (seg) {
		seg->procdef = procroot;

		if (procroot->lc.pfrec.proclink) {
			term_strncpy(errtxt,
			       "More than 1 PROC/FUNC/MODULE in external segment", MAX_LINELEN);
			return 0;
		}
	} else
		curenv->globalproc = procroot;

	if (!scan_pass2(seg, errtxt, errline))
		return 0;

	if (!scan_pass4(seg, errtxt, errline))
		return 0;

	return 1;
}


/*
 * The prog_structure_scan is "only" used to get the indentation right...
 */
static inline int
INDENT(int x)
{
	return (x >= INDENTION * MAX_INDENT ? INDENTION * MAX_INDENT : x);
}

PUBLIC void prog_structure_scan()
{
	int indent = 0;
	struct comal_line *curline = curenv->progroot;
	struct scan_entry *p;

	DBG_PRINTF(1, "Structure scanning...");

	while (curline) {
		int skip_processing;
		int cmd2;

		cmd2 = line_2cmd(curline);
		skip_processing = 0;

		curline->ld->indent = INDENT(indent);

		for (p = scan_tab; p->sym && p->sym != curline->cmd; p++);

		if (p->sym) {
			if ((p->special == SHORT_FORM && cmd2) ||
			    (p->special == PROCFUNC
			     && curline->lc.pfrec.external)
			    || (curline->cmd == trapSYM
				&& curline->lc.traprec.esc))
				skip_processing = 1;	/* Prevent indention */

			if (!skip_processing) {
				if (p->expectsym1 || p->expectsym2) {
					indent -= INDENTION;

					if (indent < 0)
						indent = 0;

					curline->ld->indent =
					    INDENT(indent);
				}

				if (p->leavessym)
					indent += INDENTION;
			}
		}

		curline = curline->ld->next;
	}
}


PUBLIC int scan_nescessary(struct comal_line *line)
{
	int i;
	int cmd2 = line_2cmd(line);

	for (i = 0; scan_tab[i].sym; i++)
		if (scan_tab[i].sym == line->cmd
		    || scan_tab[i].sym == cmd2) {
			curenv->scan_ok = 0;

			if (scan_tab[i].leavessym)
				if (line_2cmd(line) != 0
				    ||
				    ((line->cmd == procSYM
				      || line->cmd == funcSYM
				      || line->cmd == moduleSYM)
				     && line->lc.pfrec.external)
				    || (line->cmd == trapSYM
					&& line->lc.traprec.esc))
					return NO_STRUCTURE;
				else
					return STRUCTURE_START;
			else if (scan_tab[i].expectsym1
				 || scan_tab[i].expectsym2)
				return STRUCTURE_END;
			else
				return NO_STRUCTURE;
		}

	return NO_STRUCTURE;
}


PUBLIC int assess_scan(struct comal_line *line)
{
	const char *msg = NULL;

	if (entering)
		return 0;

	if (line == curenv->curline) {
		msg = catgets(catdesc, ScanSet, ScanCurExec, "the current execution line");
        } else if (line == curenv->datalptr) {
		msg = catgets(catdesc, ScanSet, ScanCurData, "the current DATA line");
        } else if (scan_nescessary(line)) {
		msg = catgets(catdesc, ScanSet, ScanStrucLine, "a program structure line");
        }
	if (msg) {
		curenv->scan_ok = 0;

		if (curenv->running == HALTED && !curenv->con_inhibited) {
			my_printf(MSG_DIALOG, 1,
				  catgets(catdesc, ScanSet, ScanInhibCon, "Adding/Modifying/Deleting %s has inhibited CONtinuation"),
				  msg);
			curenv->con_inhibited = 1;
		}

		return 1;
	}

	return 0;
}
