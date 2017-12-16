/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal routines to list program lines */

#include "pdcglob.h"
#include "pdclexs.h"
#include "pdcparss.h"
#include "pdcmisc.h"
#include "pdclist.h"
#include <string.h>

PUBLIC int show_exec = 0;

PRIVATE void list_horse(char **buf, struct comal_line *line);
PRIVATE void list_explist(char **buf, struct exp_list *exproot, int parens);
PUBLIC void list_exp(char **buf, struct expression *exp);
PRIVATE void list_char(char **buf, char c);


PRIVATE void list_text(char **buf, const char *txt)
{
	strcpy(*buf, txt);
	(*buf) += strlen(txt);
}


PRIVATE void list_sym(char **buf, int sym)
{
	list_text(buf, lex_sym(sym));
}


PRIVATE void list_symsp(char **buf, int sym)
{
	list_sym(buf, sym);
	list_char(buf, ' ');
}


PRIVATE void list_char(char **buf, char c)
{
	**buf = c;
	(*buf)++;
}


PRIVATE void list_comma(char **buf, int *first, char c)
{
	if (!*first) {
		list_char(buf, c);
		list_char(buf, ' ');
	} else
		*first = 0;
}


PRIVATE void list_string(char **buf, char str[])
{
	int i;

	list_char(buf, '"');

	for (i = 0; str[i]; i++) {
		unsigned char c;

		c = str[i];

		if (c == '"')
			list_text(buf,"\"\"");
		else {
			if (c < ' ')
				list_char(buf, '\\');

			if (c < ' ')
				switch (str[i]) {
				case '\r':
					c = 'r';
					break;
				case '\n':
					c = 'n';
					break;
				case '\t':
					c = 't';
					break;
				case 7:
					c = 'b';
					break;
				default:
					c = '?';
					break;
				}
			else if (c=='\\')
				list_char(buf,c);
	
			list_char(buf, c);
		}
	}

	list_char(buf, '"');
}

PRIVATE void list_idlist(char **buf, struct id_list *root)
{
	int first = 1;

	if (!root)
		return;

	while (root) {
		list_comma(buf, &first, ',');
		list_text(buf, root->id->name);
		root = root->next;
	}
}

PRIVATE void list_twoexp(char **buf, struct two_exp *twoexp, const char
			 *inter, int compuls)
{
	/* 
	 * This case is used in substring expressions like
	 * a$(:n#:)
	 */
	if (twoexp->exp1 && twoexp->exp1==twoexp->exp2) {
		list_text(buf, inter);
		list_exp(buf, twoexp->exp1);
		list_text(buf, inter);
	}
	else {
		if (twoexp->exp1)
			list_exp(buf, twoexp->exp1);

		if (compuls || (twoexp->exp1 && twoexp->exp2))
			list_text(buf, inter);

		if (twoexp->exp2)
			list_exp(buf, twoexp->exp2);
	}
}

PRIVATE void list_rnd(char **buf, struct expression *exp)
{
	list_sym(buf,rndSYM);

	if (exp->e.twoexp.exp1 || exp->e.twoexp.exp2) {
		list_char(buf,'(');

		if (exp->e.twoexp.exp1) {
			list_exp(buf,exp->e.twoexp.exp1);
			list_char(buf,',');
		}

		if (exp->e.twoexp.exp2)
			list_exp(buf,exp->e.twoexp.exp2);

		list_char(buf,')');
	}
}

PRIVATE void list_get(char **buf, struct expression *exp)
{
	list_sym(buf,getSYM);
	list_char(buf,'(');
	list_exp(buf,exp->e.twoexp.exp1);
	list_char(buf,',');
	list_exp(buf,exp->e.twoexp.exp2);
	list_char(buf,')');
}

PUBLIC void list_exp(char **buf, struct expression *exp)
{
	char cvtbuf[64];

	if (!exp)
		return;

	switch (exp->optype) {
	case T_CONST:
		list_sym(buf, exp->op);
		break;

	case T_UNARY:
		if (exp->op != lparenSYM)
			list_sym(buf, exp->op);

		if (exp->op != minusSYM)
			list_char(buf, '(');

		list_exp(buf, exp->e.exp);

		if (exp->op != minusSYM)
			list_char(buf, ')');

		break;

	case T_SYS:
		list_sym(buf, sysSYM);
		list_explist(buf, exp->e.exproot, 1);
		break;

	case T_SYSS:
		list_sym(buf, syssSYM);
		list_explist(buf, exp->e.exproot, 1);
		break;

	case T_BINARY:
		if (exp->op==_RND)
			list_rnd(buf,exp);
		else if (exp->op==_GET)
			list_get(buf,exp);
		else
			list_twoexp(buf, &exp->e.twoexp, lex_opsym(exp->op), 1);
		break;

	case T_INTNUM:
		list_text(buf, ltoa(exp->e.num, cvtbuf, 10));
		break;

	case T_FLOAT:
		list_text(buf, exp->e.fnum.text);
		break;

	case T_STRING:
		list_string(buf, exp->e.str->s);
		break;

	case T_SUBSTR:
		list_exp(buf, exp->e.expsubstr.exp);
		list_char(buf, '(');
		list_twoexp(buf, &exp->e.expsubstr.twoexp, ":", 1);
		list_char(buf, ')');
		break;

	case T_ID:
		list_text(buf, exp->e.expid.id->name);
		list_explist(buf, exp->e.expid.exproot, 1);
		break;

	case T_ARRAY:
	case T_SARRAY:
		list_text(buf, exp->e.expid.id->name);
		list_text(buf,"()");
		break;

	case T_SID:
		list_text(buf, exp->e.expsid.id->name);
		list_explist(buf, exp->e.expsid.exproot, 1);

		if (exp->e.expsid.twoexp) {
			list_char(buf, '(');
			list_twoexp(buf, exp->e.expsid.twoexp, ":", 1);
			list_char(buf, ')');
		}

		break;

	case T_EXP_IS_NUM:
	case T_EXP_IS_STRING:
		list_exp(buf, exp->e.exp);
		break;

	default:
		list_text(buf, "<error: list exp default action>");
	}
}


PRIVATE void list_expsp(char **buf, struct expression *exp)
{
	list_exp(buf, exp);
	list_char(buf, ' ');
}


PRIVATE void list_explist(char **buf, struct exp_list *exproot, int parens)
{
	int first = 1;

	if (!exproot)
		return;

	if (parens)
		list_char(buf, '(');

	while (exproot) {
		list_comma(buf, &first, ',');
		list_exp(buf, exproot->exp);
		exproot = exproot->next;
	}

	if (parens)
		list_char(buf, ')');
}


PRIVATE void list_repeat(char **buf, struct comal_line *line)
{
	if (line->lc.ifwhilerec.exp) {
		list_symsp(buf, line->cmd);

		if (line->lc.ifwhilerec.stat) {
			list_horse(buf, line->lc.ifwhilerec.stat);
			list_char(buf, ' ');
		}

		list_symsp(buf, untilSYM);
		list_exp(buf, line->lc.ifwhilerec.exp);
	} else
		list_sym(buf, line->cmd);
}

PRIVATE void list_ifwhile(char **buf, int thendosym, struct comal_line
			  *line)
{
	list_symsp(buf, line->cmd);
	list_expsp(buf, line->lc.ifwhilerec.exp);
	list_sym(buf, thendosym);

	if (line->lc.ifwhilerec.stat) {
		list_char(buf, ' ');
		list_horse(buf, line->lc.ifwhilerec.stat);
	}
}


PRIVATE void list_dim(char **buf, struct comal_line *line)
{
	struct dim_list *work = line->lc.dimroot;
	int first = 1;

	list_symsp(buf, line->cmd);

	while (work) {
		list_comma(buf, &first, ',');
		list_text(buf, work->id->name);

		if (work->dimensionroot) {
			int first = 1;
			struct dim_ension *work2 = work->dimensionroot;

			list_char(buf, '(');

			while (work2) {
				list_comma(buf, &first, ',');

				if (work2->bottom) {
					list_exp(buf, work2->bottom);
					list_char(buf, ':');
				}

				list_exp(buf, work2->top);

				work2 = work2->next;
			}

			list_char(buf, ')');
		}

		if (work->strlen) {
			list_char(buf, ' ');
			list_symsp(buf, ofSYM);
			list_exp(buf, work->strlen);
		}

		work = work->next;
	}
}


PRIVATE void list_file(char **buf, struct two_exp *twoexp)
{
	list_symsp(buf, fileSYM);
	list_twoexp(buf, twoexp, ",", 0);
	list_text(buf, ": ");
}


PRIVATE void list_for(char **buf, struct comal_line *line)
{
	struct for_rec *f = &line->lc.forrec;

	list_symsp(buf, line->cmd);
	list_exp(buf, f->lval);
	list_sym(buf, becomesSYM);
	list_expsp(buf, f->from);
	list_symsp(buf, f->mode);
	list_expsp(buf, f->to);

	if (f->step) {
		list_symsp(buf, stepSYM);
		list_expsp(buf, f->step);
	}

	list_sym(buf, doSYM);

	if (f->stat) {
		list_char(buf, ' ');
		list_horse(buf, f->stat);
	}
}


PRIVATE void list_input(char **buf, struct comal_line *line)
{
	struct input_rec *i = &line->lc.inputrec;

	list_symsp(buf, line->cmd);

	if (i->modifier)
		switch (i->modifier->type) {
		case fileSYM:
			list_file(buf, &i->modifier->data.twoexp);
			break;

		case stringSYM:
			list_string(buf, i->modifier->data.str->s);
			list_text(buf, ": ");
			break;

		default:
			fatal("Input modifier incorrect");
		}

	list_explist(buf, i->lvalroot, 0);
}


PRIVATE void list_printlist(char **buf, struct print_list *printroot)
{
	while (printroot) {
		if (printroot->pr_sep)
			list_sym(buf, printroot->pr_sep);

		list_exp(buf, printroot->exp);
		printroot = printroot->next;
	}
}


PRIVATE void list_print(char **buf, struct comal_line *line)
{
	struct print_rec *p = &line->lc.printrec;

	list_symsp(buf, line->cmd);

	if (p->modifier)
		switch (p->modifier->type) {
		case fileSYM:
			list_file(buf, &p->modifier->data.twoexp);
			break;

		case usingSYM:
			list_symsp(buf, usingSYM);
			list_exp(buf, p->modifier->data.str);
			list_text(buf, ": ");
			break;

		default:
			fatal("Print modifier incorrect");
		}

	list_printlist(buf, p->printroot);

	if (p->pr_sep)
		list_sym(buf, p->pr_sep);
}


PRIVATE void list_assign(char **buf, struct comal_line *line)
{
	struct assign_list *work = line->lc.assignroot;
	int first = 1;

	while (work) {
		list_comma(buf, &first, ';');
		list_exp(buf, work->lval);
		list_sym(buf, work->op);
		list_exp(buf, work->exp);
		work = work->next;
	}
}


PRIVATE void list_parms(char **buf, struct parm_list *root)
{
	int first = 1;

	if (!root)
		return;

	list_char(buf, '(');

	while (root) {
		list_comma(buf, &first, ',');

		if (root->ref)
			list_symsp(buf, root->ref);

		list_text(buf, root->id->name);

		if (root->array)
			list_text(buf, "()");

		root = root->next;
	}

	list_char(buf, ')');
}


PRIVATE void list_import(char **buf, struct comal_line *line)
{
	struct import_list *work = line->lc.importrec.importroot;
	int first = 1;

	list_symsp(buf, line->cmd);

	if (line->lc.importrec.id) {
		list_text(buf, line->lc.importrec.id->name);
		list_text(buf, ": ");
	}

	while (work) {
		list_comma(buf, &first, ',');
		list_text(buf, work->id->name);

		if (work->array)
			list_text(buf, "()");

		work = work->next;
	}
}


PRIVATE void list_whenlist(char **buf, struct when_list *whenroot)
{
	int first = 1;

	while (whenroot) {
		list_comma(buf, &first, ',');

		if (whenroot->op != eqlSYM)
			list_sym(buf, whenroot->op);

		list_exp(buf, whenroot->exp);
		whenroot = whenroot->next;
	}
}


PRIVATE void list_pf(char **buf, struct comal_line *line)
{
	struct proc_func_rec *pf = &line->lc.pfrec;
	struct ext_rec *ext = pf->external;

	list_symsp(buf, line->cmd);
	list_text(buf, pf->id->name);
	list_parms(buf, pf->parmroot);

	if (pf->closed && line->cmd!=moduleSYM) {
		list_char(buf, ' ');
		list_sym(buf, closedSYM);
	}

	if (ext) {
		list_char(buf, ' ');

		if (ext->dynamic)
			list_symsp(buf, ext->dynamic);

		list_symsp(buf, externalSYM);
		list_exp(buf, ext->filename);
	}
}


PRIVATE void list_horse(char **buf, struct comal_line *line)
{
	if (!line) return;

	switch (line->cmd) {
	case 0:
		break;

	case runSYM:
	case delSYM:
	case chdirSYM:
	case rmdirSYM:
	case mkdirSYM:
	case osSYM:
	case dirSYM:
	case unitSYM:
	case select_outputSYM:
	case select_inputSYM:
	case returnSYM:
	case elifSYM:
	case traceSYM:
	case untilSYM:
	case randomizeSYM:
	case reportSYM:
	case delaySYM:
	case zoneSYM:
		list_symsp(buf, line->cmd);
		list_exp(buf, line->lc.exp);
		break;

	case exitSYM:
		list_sym(buf, line->cmd);

		if (line->lc.exp) {
			list_char(buf, ' ');
			list_symsp(buf, whenSYM);
			list_exp(buf, line->lc.exp);
		}

		break;

	case stopSYM:
		list_sym(buf, line->cmd);

		if (line->lc.exp) {
			list_char(buf, ' ');
			list_exp(buf, line->lc.exp);
		}

		break;

	case elseSYM:
	case endSYM:
	case endcaseSYM:
	case endifSYM:
	case endloopSYM:
	case endwhileSYM:
	case otherwiseSYM:
	case loopSYM:
	case nullSYM:
	case retrySYM:
	case pageSYM:
	case handlerSYM:
	case endtrapSYM:
		list_sym(buf, line->cmd);
		break;

	case useSYM:
	case exportSYM:
		list_symsp(buf, line->cmd);
		list_idlist(buf, line->lc.idroot);
		break;

	case repeatSYM:
		list_repeat(buf, line);
		break;

	case trapSYM:
		list_sym(buf, line->cmd);

		if (line->lc.traprec.esc) {
			list_char(buf, ' ');
			list_sym(buf, escSYM);
			list_sym(buf, line->lc.traprec.esc);
		}

		break;

	case execSYM:
		if (show_exec)
			list_symsp(buf, execSYM);

		list_exp(buf, line->lc.exp);
		break;

	case caseSYM:
		list_symsp(buf, line->cmd);
		list_expsp(buf, line->lc.exp);
		list_sym(buf, ofSYM);
		break;

	case cursorSYM:
		list_symsp(buf, line->cmd);
		list_twoexp(buf, &line->lc.twoexp, ",", 1);
		break;

	case closeSYM:
		list_sym(buf, line->cmd);

		if (line->lc.exproot) {
			list_char(buf, ' ');
			list_symsp(buf, fileSYM);
			list_explist(buf, line->lc.exproot, 0);
		}

		break;


	case sysSYM:
	case dataSYM:
		list_symsp(buf, line->cmd);
		list_explist(buf, line->lc.exproot, 0);
		break;

	case staticSYM:
	case localSYM:
	case dimSYM:
		list_dim(buf, line);
		break;

	case forSYM:
		list_for(buf, line);
		break;

	case funcSYM:
	case procSYM:
	case moduleSYM:
		list_pf(buf, line);
		break;

	case ifSYM:
		list_ifwhile(buf, thenSYM, line);
		break;

	case importSYM:
		list_import(buf, line);
		break;

	case inputSYM:
		list_input(buf, line);
		break;

	case openSYM:
		list_symsp(buf, line->cmd);
		list_symsp(buf, fileSYM);
		list_exp(buf, line->lc.openrec.filenum);
		list_text(buf, ", ");
		list_exp(buf, line->lc.openrec.filename);
		list_text(buf, ", ");
		list_symsp(buf, line->lc.openrec.type);

		if (line->lc.openrec.reclen) {
			list_exp(buf, line->lc.openrec.reclen);

			if (line->lc.openrec.read_only) {
				list_char(buf, ' ');
				list_sym(buf, read_onlySYM);
			}
		}

		break;

	case printSYM:
		list_print(buf, line);
		break;

	case readSYM:
		list_symsp(buf, line->cmd);

		if (line->lc.readrec.modifier)
			list_file(buf, line->lc.readrec.modifier);

		list_explist(buf, line->lc.readrec.lvalroot, 0);
		break;

	case endmoduleSYM:
		list_sym(buf, line->cmd);

		if (line->lineptr) {
			list_char(buf, ' ');
			list_text(buf, line->lineptr->lc.id->name);
		}

		break;


	case endfuncSYM:
	case endprocSYM:
		list_sym(buf, line->cmd);

		if (line->lineptr) {
			list_char(buf, ' ');
			list_text(buf, line->lineptr->lc.pfrec.id->name);
		}

		break;

	case endforSYM:
		list_sym(buf, line->cmd);

		if (line->lineptr) {
			list_char(buf, ' ');
			list_exp(buf, line->lineptr->lc.forrec.lval);
		}

		break;

	case restoreSYM:
		list_sym(buf, line->cmd);

		if (line->lc.id) {
			list_char(buf, ' ');
			list_text(buf, line->lc.id->name);
		}

		break;


	case whenSYM:
		list_symsp(buf, line->cmd);
		list_whenlist(buf, line->lc.whenroot);
		break;

	case whileSYM:
		list_ifwhile(buf, doSYM, line);
		break;

	case writeSYM:
		list_symsp(buf, line->cmd);
		list_file(buf, &line->lc.writerec.twoexp);
		list_explist(buf, line->lc.writerec.exproot, 0);
		break;

	case becomesSYM:
		list_assign(buf, line);
		break;

	case idSYM:
		list_text(buf, line->lc.id->name);
		list_char(buf, ':');
		break;

	default:
		list_text(buf, "<error: List default action>");
	}
}


PUBLIC void line_list(char **buf, struct comal_line *line)
{
	if (!line) {
		sprintf(*buf,"<no line>");
		(*buf) += 10;
		return;
	}

	if (line->ld) {
		int i;

		sprintf(*buf, "%9ld  ", line->ld->lineno);
		(*buf) += 10;

		for (i = 0; i < line->ld->indent; i++)
			list_char(buf, ' ');
	}

	list_horse(buf, line);

	if (line->ld)
		if (line->ld->rem) {
			if (line->cmd == 0)
				list_text(buf, "//");
			else
				list_text(buf, "  //");

			list_text(buf, line->ld->rem->s);
		}

	list_char(buf, '\0');
}
