/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal line squashing functions for save/load purposes */

#include "pdcglob.h"
#include "pdcid.h"
#include "pdcmisc.h"
#include "pdcsqash.h"
#include "pdcexec.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

PRIVATE void sqash_exp(struct expression *exp);
PRIVATE void sqash_horse(struct comal_line *line);

PRIVATE struct expression *expand_exp();
PRIVATE struct comal_line *expand_horse();


PRIVATE char *sqash_buf;
PRIVATE int sqash_hwm;
PRIVATE unsigned sqash_i;
PRIVATE int sqash_file;


PRIVATE void sqash_flush()
{
	if (sqash_i > 0) {
		if (write(sqash_file, sqash_buf, sqash_i) <= 0) {
			close(sqash_file);
			run_error(SQASH_ERR,
				  "Error when writing to file: %s",
				  strerror(errno));
		}

		sqash_i = 0;
	}
}


PRIVATE void sqash_put(const void *data, unsigned size)
{
	if (size > SQASH_BUFSIZE)
		fatal("Sqash_put() size overflow");

	if (sqash_i + size > SQASH_BUFSIZE)
		sqash_flush();

	memcpy(sqash_buf + sqash_i, data, size);
	sqash_i += size;
}


PRIVATE void sqash_putc(char c)
{
	if (sqash_i == SQASH_BUFSIZE)
		sqash_flush();

	*(sqash_buf + sqash_i) = c;
	sqash_i++;
}


PRIVATE void sqash_putlong(char code, long l)
{
	if (code)
		sqash_putc(code);

	sqash_put(&l, sizeof(long));
}


PRIVATE void sqash_putint(char code, int i)
{
	if (code)
		sqash_putc(code);

	sqash_put(&i, sizeof(int));
}


PRIVATE void sqash_putstr(int code, struct string *s)
{
	if (s) {
		sqash_putlong(code, s->len);
		sqash_put(s->s, s->len);
	} else
		sqash_putc(SQ_EMPTYSTRING);
}


PRIVATE void sqash_putstr2(int code, const char *s)
{
	if (s) {
		int i;

		i = strlen(s);
		sqash_putint(code, i);
		sqash_put(s, i + 1);
	} else
		sqash_putc(SQ_EMPTYSTRING);
}

PRIVATE void sqash_putdouble(char code, struct dubbel *d)
{
	if (code)
		sqash_putc(code);

	sqash_put(&d->val, sizeof(double));
	sqash_putstr2(SQ_DOUBLE,d->text);
}



PRIVATE void sqash_putid(struct id_rec *id)
{
	if (id)
		sqash_putstr2(SQ_ID, id->name);
	else
		sqash_putstr2(SQ_ID, NULL);
}


PRIVATE void sqash_explist(struct exp_list *exproot)
{
	sqash_putc(SQ_EXPLIST);

	while (exproot) {
		sqash_exp(exproot->exp);
		exproot = exproot->next;
	}

	sqash_putc(SQ_ENDEXPLIST);
}

PRIVATE void sqash_idlist(struct id_list *root)
{
	sqash_putc(SQ_IDLIST);

	while (root) {
		sqash_putid(root->id);
		root = root->next;
	}

	sqash_putc(SQ_ENDIDLIST);
}

PRIVATE void sqash_twoexp(struct two_exp *twoexp)
{
	if (!twoexp)
		sqash_putc(SQ_EMPTYTWOEXP);
	else if (twoexp->exp1 && twoexp->exp1==twoexp->exp2) {
		sqash_putc(SQ_ONETWOEXP);
		sqash_exp(twoexp->exp1);
	} else {
		sqash_putc(SQ_TWOEXP);
		sqash_exp(twoexp->exp1);
		sqash_exp(twoexp->exp2);
	}
}


PRIVATE void sqash_exp(struct expression *exp)
{
	if (!exp) {
		sqash_putc(SQ_EMPTYEXP);
		return;
	}

	sqash_putint(SQ_EXP, exp->optype);

	switch (exp->optype) {
	case T_CONST:
		sqash_putint(0, exp->op);
		break;

	case T_INTNUM:
		sqash_putlong(0, exp->e.num);
		break;

	case T_FLOAT:
		sqash_putdouble(0, &exp->e.fnum);
		break;

	case T_UNARY:
		sqash_putint(0, exp->op);
		sqash_exp(exp->e.exp);
		break;

	case T_SYS:
	case T_SYSS:
		sqash_explist(exp->e.exproot);
		break;

	case T_SUBSTR:
		sqash_exp(exp->e.expsubstr.exp);
		sqash_twoexp(&exp->e.expsubstr.twoexp);
		break;

	case T_ID:
		sqash_putid(exp->e.expid.id);
		sqash_explist(exp->e.expid.exproot);
		break;

	case T_ARRAY:
	case T_SARRAY:
		sqash_putid(exp->e.expid.id);
		break;

	case T_BINARY:
		sqash_putint(0, exp->op);
		sqash_twoexp(&exp->e.twoexp);
		break;

	case T_STRING:
		sqash_putstr(SQ_STRING, exp->e.str);
		break;

	case T_SID:
		sqash_putid(exp->e.expsid.id);
		sqash_explist(exp->e.expsid.exproot);
		sqash_twoexp(exp->e.expsid.twoexp);
		break;

	case T_EXP_IS_NUM:
	case T_EXP_IS_STRING:
		sqash_exp(exp->e.exp);
		break;

	default:
		fatal("Sqash exp default action");
	}
}


PRIVATE void sqash_ifwhile(struct comal_line *line)
{
	sqash_exp(line->lc.ifwhilerec.exp);
	sqash_horse(line->lc.ifwhilerec.stat);
}


PRIVATE void sqash_dim(struct comal_line *line)
{
	struct dim_list *work = line->lc.dimroot;

	while (work) {
		sqash_putid(work->id);

		if (work->dimensionroot) {
			struct dim_ension *work2 = work->dimensionroot;

			while (work2) {
				sqash_putc(SQ_1DIMENSION);
				sqash_exp(work2->bottom);
				sqash_exp(work2->top);
				work2 = work2->next;
			}
		}

		sqash_exp(work->strlen);
		work = work->next;
	}
}


PRIVATE void sqash_for(struct comal_line *line)
{
	struct for_rec *f = &line->lc.forrec;

	sqash_putint(0, f->mode);
	sqash_exp(f->lval);
	sqash_exp(f->from);
	sqash_exp(f->to);
	sqash_exp(f->step);
	sqash_horse(f->stat);
}


PRIVATE void sqash_input(struct comal_line *line)
{
	struct input_rec *i = &line->lc.inputrec;

	if (i->modifier) {
		sqash_putint(SQ_MODIFIER, i->modifier->type);

		switch (i->modifier->type) {
		case fileSYM:
			sqash_twoexp(&i->modifier->twoexp);
			break;

		case stringSYM:
			sqash_putstr(SQ_STRING, i->modifier->str);
			break;

		case atSYM:
			sqash_twoexp(&i->modifier->twoexp);
			sqash_exp(i->modifier->len);
			sqash_putstr(SQ_STRING, i->modifier->str);
			break;

		default:
			fatal("Input modifier incorrect (sqash)");
		}
	}

	sqash_explist(i->lvalroot);
}


PRIVATE void sqash_printlist(struct print_list *printroot)
{
	while (printroot) {
		sqash_exp(printroot->exp);
		sqash_putint(0, printroot->pr_sep);
		printroot = printroot->next;
	}
}


PRIVATE void sqash_print(struct comal_line *line)
{
	struct print_rec *p = &line->lc.printrec;

	if (p->modifier) {
		sqash_putint(SQ_MODIFIER, p->modifier->type);

		switch (p->modifier->type) {
		case fileSYM:
			sqash_twoexp(p->modifier->data.twoexp);
			break;

		case usingSYM:
			sqash_exp(p->modifier->data.str);
			break;

		case atSYM:
			sqash_twoexp(p->modifier->data.twoexp);
			break;

		default:
			fatal("Print modifier incorrect (sqash)");
		}
	}

	sqash_printlist(p->printroot);
	sqash_putint(0, p->pr_sep);
}


PRIVATE void sqash_assign(struct assign_list *assignroot)
{
	while (assignroot) {
		sqash_exp(assignroot->lval);
		sqash_putint(0, assignroot->op);
		sqash_exp(assignroot->exp);
		assignroot = assignroot->next;
	}
}


PRIVATE void sqash_whenlist(struct when_list *whenroot)
{
	while (whenroot) {
		sqash_exp(whenroot->exp);
		sqash_putint(0, whenroot->op);
		whenroot = whenroot->next;
	}
}


PRIVATE void sqash_parmlist(struct parm_list *root)
{
	while (root) {
		sqash_putc(SQ_1PARM);
		sqash_putid(root->id);
		sqash_putint(0, root->ref);
		sqash_putint(0, root->array);
		root = root->next;
	}
}


PRIVATE void sqash_importlist(struct import_list *root)
{
	while (root) {
		sqash_putc(SQ_1PARM);
		sqash_putid(root->id);
		sqash_putint(0, root->array);
		root = root->next;
	}
}


PRIVATE void sqash_horse(struct comal_line *line)
{
	if (!line) {
		sqash_putc(SQ_EMPTYLINE);
		return;
	}

	sqash_putint(SQ_LINE, line->cmd);

	if (line->ld) {
		sqash_putc(SQ_LD);
		sqash_putlong(0, line->ld->lineno);
		sqash_putstr(SQ_REM, line->ld->rem);
	}

	switch (line->cmd) {
	case 0:
	case elseSYM:
	case endSYM:
	case endcaseSYM:
	case endmoduleSYM:
	case endfuncSYM:
	case endifSYM:
	case endloopSYM:
	case endprocSYM:
	case endwhileSYM:
	case endforSYM:
	case otherwiseSYM:
	case nullSYM:
	case retrySYM:
	case pageSYM:
	case handlerSYM:
	case loopSYM:
	case endtrapSYM:
		break;

	case trapSYM:
		sqash_putint(0, line->lc.traprec.esc);
		break;

	case idSYM:
	case restoreSYM:
		sqash_putid(line->lc.id);
		break;

	case useSYM:
	case exportSYM:
		sqash_idlist(line->lc.idroot);
		break;

	case execSYM:
	case caseSYM:
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
	case stopSYM:
	case elifSYM:
	case exitSYM:
	case traceSYM:
	case untilSYM:
	case randomizeSYM:
	case reportSYM:
	case delaySYM:
	case zoneSYM:
		sqash_exp(line->lc.exp);
		break;

	case closeSYM:
	case sysSYM:
	case dataSYM:
		sqash_explist(line->lc.exproot);
		break;

	case cursorSYM:
		sqash_twoexp(&line->lc.twoexp);
		break;

	case localSYM:
	case staticSYM:
	case dimSYM:
		sqash_dim(line);
		break;

	case forSYM:
		sqash_for(line);
		break;

	case funcSYM:
	case procSYM:
	case moduleSYM:
		sqash_putid(line->lc.pfrec.id);
		sqash_putint(0, line->lc.pfrec.closed);
		sqash_parmlist(line->lc.pfrec.parmroot);

		if (line->lc.pfrec.external) {
			sqash_putint(0, line->lc.pfrec.external->dynamic);
			sqash_exp(line->lc.pfrec.external->filename);
		} else
			sqash_putc(SQ_NOEXTERNAL);

		break;

	case importSYM:
		sqash_putid(line->lc.importrec.id);
		sqash_importlist(line->lc.importrec.importroot);
		break;

	case ifSYM:
		sqash_ifwhile(line);
		break;

	case inputSYM:
		sqash_input(line);
		break;

	case openSYM:
		sqash_exp(line->lc.openrec.filenum);
		sqash_exp(line->lc.openrec.filename);
		sqash_exp(line->lc.openrec.reclen);
		sqash_putint(0, line->lc.openrec.read_only);
		sqash_putint(0, line->lc.openrec.type);
		break;
	
	case createSYM:
		sqash_exp(line->lc.createrec.filename);
		sqash_exp(line->lc.createrec.top);
		sqash_exp(line->lc.createrec.reclen);
		break;

	case printSYM:
		sqash_print(line);
		break;

	case readSYM:
		sqash_twoexp(line->lc.readrec.modifier);
		sqash_explist(line->lc.readrec.lvalroot);
		break;

	case whenSYM:
		sqash_whenlist(line->lc.whenroot);
		break;

	case repeatSYM:
	case whileSYM:
		sqash_ifwhile(line);
		break;

	case writeSYM:
		sqash_twoexp(&line->lc.writerec.twoexp);
		sqash_explist(line->lc.writerec.exproot);
		break;

	case becomesSYM:
		sqash_assign(line->lc.assignroot);
		break;

	default:
		fatal("cmd switch default action (sqash_horse)");
	}
}


PUBLIC void sqash_2file(char *fname)
{
	struct comal_line *work = curenv->progroot;
	const char *s;

	sqash_file =
	    open(fname, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY,
		 S_IRUSR | S_IWUSR);

	if (sqash_file < 0)
		run_error(OPEN_ERR, "File open error: %s",
			  strerror(errno));

	sqash_buf = (char *)mem_alloc(MISC_POOL, SQASH_BUFSIZE);
	sqash_i = 0;

	for (s=SQ_MARKER; *s; s++)
		sqash_putc(*s);

	sqash_putc(HOST_OS_CODE);
	sqash_putstr2(SQ_CONTROL, HOST_OS);
	sqash_putint(0, SQ_VERSION);
	sqash_putstr2(SQ_CONTROL, SQ_COPYRIGHT_MSG);

	while (work) {
		sqash_horse(work);
		work = work->ld->next;
	}

	sqash_putstr2(SQ_CONTROL, SQ_COPYRIGHT_MSG);
	sqash_flush();

	mem_free(sqash_buf);

	if (close(sqash_file) < 0)
		run_error(CLOSE_ERR, "Error closing file: %s",
			  strerror(errno));
}


/***************/
/* EXPAND Part */
/***************/

PRIVATE void expand_read()
{
	sqash_hwm = read(sqash_file, sqash_buf, SQASH_BUFSIZE);

	if (sqash_hwm < 0) {
		close(sqash_file);
		run_error(SQASH_ERR, "Error when reading from file: %s",
			  strerror(errno));
	}

	sqash_i = 0;
}


PRIVATE char expand_getc()
{
	char c;

	if (sqash_i >= sqash_hwm)
		expand_read();

	c = *(sqash_buf + sqash_i);
	sqash_i++;

	return c;
}


PRIVATE void expand_get(void *data, unsigned size)
{
	int i;

	for (i = 0; i < size; i++)
		*((char *)data + i) = expand_getc();
}


PRIVATE char expand_peekc()
{
	char c;

	if (sqash_i >= sqash_hwm)
		expand_read();

	c = *(sqash_buf + sqash_i);

	return c;
}


PRIVATE void expand_check(char c)
{
	if (expand_getc() != c)
		fatal("Internal sqash/expand error #2");
}


PRIVATE long expand_getlong()
{
	long l;

	expand_get(&l, sizeof(long));

	return l;
}


PRIVATE int expand_getint()
{
	int i;

	expand_get(&i, sizeof(int));

	return i;
}


PRIVATE struct string *expand_getstr(int code)
{
	struct string *s = NULL;
	char c = expand_getc();

	if (c == code) {
		long l;

		l = expand_getlong();
		s = STR_ALLOC_PRIVATE(curenv->program_pool, l);
		expand_get(s->s, l);
		s->len = l;
	} else if (c != SQ_EMPTYSTRING)
		fatal("Internal sqash/expand error #1b");

	return s;
}


PRIVATE char *expand_getstr2(int code)
{
	char *s = NULL;
	char c = expand_getc();

	if (c == code) {
		int i;

		i = expand_getint();
		s = (char *)mem_alloc_private(curenv->program_pool, i + 1);
		expand_get(s, i + 1);
	} else if (c != SQ_EMPTYSTRING)
		fatal("Internal sqash/expand error #1");

	return s;
}


PRIVATE void expand_getdouble(struct dubbel *d)
{
	expand_get(&d->val, sizeof(double));
	d->text=expand_getstr2(SQ_DOUBLE);
}



PRIVATE struct id_rec *expand_getid()
{
	struct id_rec *id;
	char *s = expand_getstr2(SQ_ID);

	if (s) {
		id = id_search(s);
		mem_free(s);
	} else
		id = NULL;

	return id;
}


PRIVATE struct exp_list *expand_explist()
{
	struct exp_list *root = NULL;
	struct exp_list *work;

	expand_check(SQ_EXPLIST);

	while (expand_peekc() != SQ_ENDEXPLIST) {
		work =
		    (struct exp_list *)mem_alloc_private(curenv->program_pool,
				      sizeof(struct exp_list));
		work->exp = expand_exp();
		work->next = root;
		root = work;
	}

	expand_getc();

	return (struct exp_list *)my_reverse(root);
}

PRIVATE struct id_list *expand_idlist()
{
	struct id_list *root = NULL;
	struct id_list *work;

	expand_check(SQ_IDLIST);

	while (expand_peekc() != SQ_ENDIDLIST) {
		work =
		    (struct id_list *)mem_alloc_private(curenv->program_pool,
				      sizeof(struct id_list));
		work->id = expand_getid();
		work->next = root;
		root = work;
	}

	expand_getc();

	return (struct id_list *)my_reverse(root);
}


PRIVATE void expand_twoexp(struct two_exp *work)
{
	char c=expand_getc();

	if (c == SQ_EMPTYTWOEXP)
		work->exp1=work->exp2=NULL;
	else if (c  == SQ_ONETWOEXP) {
		work->exp1 = expand_exp();
		work->exp2 = work->exp1;
	} else if (c == SQ_TWOEXP) {
		work->exp1 = expand_exp();
		work->exp2 = expand_exp();
	} else
		fatal("Internal twoexp expand error #1");
}

PRIVATE struct two_exp *expand_alloc_twoexp() {
	char c=expand_peekc();
	struct two_exp *work;

	if (c==SQ_EMPTYTWOEXP) {
		expand_getc();
		work=NULL;
	} else {
		work = (struct two_exp *)mem_alloc_private(curenv->program_pool, 
			sizeof(struct two_exp));
		expand_twoexp(work);
	}

	return work;
}


#define EXP_ALLOC(x)	exp=(struct expression *)mem_alloc_private(curenv->program_pool,sizeof(struct expression)-sizeof(union exp_data)+sizeof(x))
#define EXP_ALLOC0	exp=(struct expression *)mem_alloc_private(curenv->program_pool,sizeof(struct expression)-sizeof(union exp_data))

PRIVATE struct expression *expand_exp()
{
	char c = expand_getc();
	struct expression *exp = NULL;
	enum optype o;

	if (c == SQ_EMPTYEXP) {
		DBG_PRINTF(1, "Empty exp");

		return NULL;
	} else if (c != SQ_EXP)
		fatal("Internal sqash/expand error #3");

	o = (enum optype)expand_getint();

	switch (o) {
	case T_CONST:
		EXP_ALLOC0;
		exp->op = expand_getint();
		break;

	case T_INTNUM:
		EXP_ALLOC(long);
		exp->e.num = expand_getlong();
		break;

	case T_FLOAT:
		EXP_ALLOC(struct dubbel);
		expand_getdouble(&exp->e.fnum);
		break;

	case T_UNARY:
		EXP_ALLOC(struct expression *);
		exp->op = expand_getint();
		exp->e.exp = expand_exp();
		break;

	case T_SYS:
	case T_SYSS:
		EXP_ALLOC(struct exp_list *);
		exp->e.exproot = expand_explist();
		break;

	case T_SUBSTR:
		EXP_ALLOC(struct exp_substr);
		exp->e.expsubstr.exp = expand_exp();
		expand_twoexp(&exp->e.expsubstr.twoexp);
		break;

	case T_ID:
		EXP_ALLOC(struct exp_id);
		exp->e.expid.id = expand_getid();
		exp->e.expid.exproot = expand_explist();
		break;

	case T_ARRAY:
	case T_SARRAY:
		EXP_ALLOC(struct exp_id);
		exp->e.expid.id = expand_getid();
		break;
		
	case T_BINARY:
		EXP_ALLOC(struct two_exp);
		exp->op = expand_getint();
		expand_twoexp(&exp->e.twoexp);
		break;

	case T_STRING:
		EXP_ALLOC(char *);
		exp->e.str = expand_getstr(SQ_STRING);
		break;

	case T_SID:
		EXP_ALLOC(struct exp_sid);
		exp->e.expsid.id = expand_getid();
		exp->e.expsid.exproot = expand_explist();
		exp->e.expsid.twoexp=expand_alloc_twoexp();
		break;

	case T_EXP_IS_NUM:
	case T_EXP_IS_STRING:
		EXP_ALLOC(struct expression *);
		exp->e.exp = expand_exp();
		break;

	default:
		fatal("Expand exp default action");
	}

	exp->optype = o;

	DBG_PRINTF(1, "Exp expanded");

	return exp;
}


PRIVATE void expand_ifwhile(struct comal_line *line)
{
	line->lc.ifwhilerec.exp = expand_exp();
	line->lc.ifwhilerec.stat = expand_horse();
}


PRIVATE void expand_dim(struct comal_line *line)
{
	struct dim_list *root = NULL;
	struct dim_list *work = NULL;
	struct dim_ension *droot;
	struct dim_ension *dwork;

	while (expand_peekc() == SQ_ID) {
		work =
		    (struct dim_list *)mem_alloc_private(curenv->program_pool,
				      sizeof(struct dim_list));
		work->next = root;
		root = work;

		work->id = expand_getid();
		droot = NULL;

		while (expand_peekc() == SQ_1DIMENSION) {
			expand_getc();
			dwork =
			    (struct dim_ension *)mem_alloc_private(curenv->program_pool,
					      sizeof(struct dim_ension));
			dwork->next = droot;
			droot = dwork;
			dwork->bottom = expand_exp();
			dwork->top = expand_exp();
		}

		work->dimensionroot = (struct dim_ension *)my_reverse(droot);
		work->strlen = expand_exp();
	}

	line->lc.dimroot = (struct dim_list *)my_reverse(work);
}


PRIVATE void expand_for(struct comal_line *line)
{
	struct for_rec *f = &line->lc.forrec;

	f->mode = expand_getint();
	f->lval = expand_exp();
	f->from = expand_exp();
	f->to = expand_exp();
	f->step = expand_exp();
	f->stat = expand_horse();
}


PRIVATE void expand_input(struct comal_line *line)
{
	struct input_rec *i = &line->lc.inputrec;

	if (expand_peekc() == SQ_MODIFIER) {
		expand_getc();
		i->modifier =
		    (struct input_modifier *)mem_alloc_private(curenv->program_pool,
				      sizeof(*i->modifier));
		i->modifier->type = expand_getint();

		switch (i->modifier->type) {
		case fileSYM:
			expand_twoexp(&i->modifier->twoexp);
			break;

		case stringSYM:
			i->modifier->str = expand_getstr(SQ_STRING);
			break;

		case atSYM:
			expand_twoexp(&i->modifier->twoexp);
			i->modifier->len = expand_exp();
			i->modifier->str = expand_getstr(SQ_STRING);
			break;

		default:
			fatal("Input modifier incorrect (expand)");
		}
	}

	i->lvalroot = expand_explist();
}


PRIVATE struct print_list *expand_printlist()
{
	struct print_list *root = NULL;
	struct print_list *work;

	while (expand_peekc() == SQ_EXP) {
		work =
		    (struct print_list *)mem_alloc_private(curenv->program_pool,
				      sizeof(struct print_list));
		work->next = root;
		root = work;
		work->exp = expand_exp();
		work->pr_sep = expand_getint();
	}

	return (struct print_list *)my_reverse(root);
}


PRIVATE void expand_print(struct comal_line *line)
{
	struct print_rec *p = &line->lc.printrec;

	if (expand_peekc() == SQ_MODIFIER) {
		expand_getc();
		p->modifier =
		    (struct print_modifier *)mem_alloc_private(curenv->program_pool,
				      sizeof(*p->modifier));
		p->modifier->type = expand_getint();

		switch (p->modifier->type) {
		case fileSYM:
			expand_twoexp(p->modifier->data.twoexp);
			break;

		case usingSYM:
			p->modifier->data.str = expand_exp();
			break;

		case atSYM:
			expand_twoexp(p->modifier->data.twoexp);
			break;

		default:
			fatal("Print modifier incorrect (expand)");
		}
	}

	p->printroot = expand_printlist();
	p->pr_sep = expand_getint();
}


PRIVATE struct assign_list *expand_assign()
{
	struct assign_list *work;
	struct assign_list *root = NULL;

	while (expand_peekc() == SQ_EXP) {
		work =
		    (struct assign_list *)mem_alloc_private(curenv->program_pool,
				      sizeof(struct assign_list));
		work->next = root;
		root = work;

		work->lval = expand_exp();
		work->op = expand_getint();
		work->exp = expand_exp();

		DBG_PRINTF(1, "Assign expanded");
	}

	return (struct assign_list *)my_reverse(root);
}


PRIVATE struct when_list *expand_whenlist()
{
	struct when_list *work;
	struct when_list *root = NULL;

	while (expand_peekc() == SQ_EXP) {
		work =
		    (struct when_list *)mem_alloc_private(curenv->program_pool,
				      sizeof(struct when_list));
		work->next = root;
		root = work;

		work->exp = expand_exp();
		work->op = expand_getint();
	}

	return (struct when_list *)my_reverse(root);
}


PRIVATE struct parm_list *expand_parmlist()
{
	struct parm_list *root = NULL;
	struct parm_list *work;

	while (expand_peekc() == SQ_1PARM) {
		expand_getc();
		work =
		    (struct parm_list *)mem_alloc_private(curenv->program_pool,
				      sizeof(struct parm_list));
		work->next = root;
		root = work;

		work->id = expand_getid();
		work->ref = expand_getint();
		work->array = expand_getint();
	}

	return (struct parm_list *)my_reverse(root);
}


PRIVATE struct import_list *expand_importlist()
{
	struct import_list *root = NULL;
	struct import_list *work;

	while (expand_peekc() == SQ_1PARM) {
		expand_getc();
		work =
		    (struct import_list *)mem_alloc_private(curenv->program_pool,
				      sizeof(struct import_list));
		work->next = root;
		root = work;

		work->id = expand_getid();
		work->array = expand_getint();
	}

	return (struct import_list *)my_reverse(root);
}


PRIVATE struct comal_line *expand_horse()
{
	struct comal_line *line;
	char c = expand_getc();
	int cmd;

	if (c == SQ_EMPTYLINE)
		return NULL;
	else if (c != SQ_LINE)
		fatal("Sqash/expand internal error #5");

	cmd = expand_getint();
	line = (struct comal_line *)mem_alloc_private(curenv->program_pool, stat_size(cmd));
	line->cmd = cmd;

	if (expand_peekc() == SQ_LD) {
		expand_getc();
		line->ld =
		    (struct comal_line_data *)mem_alloc_private(curenv->program_pool,
				      sizeof(*line->ld));
		line->ld->lineno = expand_getlong();
		line->ld->rem = expand_getstr(SQ_REM);
	}

	switch (line->cmd) {
	case 0:
	case elseSYM:
	case endSYM:
	case endcaseSYM:
	case endfuncSYM:
	case endifSYM:
	case endloopSYM:
	case endprocSYM:
	case endwhileSYM:
	case endforSYM:
	case otherwiseSYM:
	case nullSYM:
	case retrySYM:
	case pageSYM:
	case handlerSYM:
	case loopSYM:
	case endtrapSYM:
	case endmoduleSYM:
		break;

	case trapSYM:
		line->lc.traprec.esc = expand_getint();
		break;

	case idSYM:
	case restoreSYM:
		line->lc.id = expand_getid();
		break;

	case useSYM:
	case exportSYM:
		line->lc.idroot=expand_idlist();
		break;

	case execSYM:
	case caseSYM:
	case runSYM:
	case delSYM:
	case chdirSYM:
	case mkdirSYM:
	case rmdirSYM:
	case osSYM:
	case dirSYM:
	case unitSYM:
	case select_outputSYM:
	case stopSYM:
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
		line->lc.exp = expand_exp();
		break;

	case closeSYM:
	case sysSYM:
	case dataSYM:
		line->lc.exproot = expand_explist();
		break;

	case cursorSYM:
		expand_twoexp(&line->lc.twoexp);
		break;

	case localSYM:
	case staticSYM:
	case dimSYM:
		expand_dim(line);
		break;

	case forSYM:
		expand_for(line);
		break;

	case funcSYM:
	case procSYM:
	case moduleSYM:
		line->lc.pfrec.id = expand_getid();
		line->lc.pfrec.closed = expand_getint();
		line->lc.pfrec.parmroot = expand_parmlist();

		if (expand_peekc() == SQ_NOEXTERNAL) {
			expand_getc();
			line->lc.pfrec.external = NULL;
		} else {
			line->lc.pfrec.external =
			    (struct ext_rec *)mem_alloc_private(curenv->program_pool,
					      sizeof(struct ext_rec));
			line->lc.pfrec.external->dynamic = expand_getint();
			line->lc.pfrec.external->filename = expand_exp();
			line->lc.pfrec.external->seg = NULL;
		}

		break;

	case importSYM:
		line->lc.importrec.id = expand_getid();
		line->lc.importrec.importroot = expand_importlist();
		break;

	case ifSYM:
		expand_ifwhile(line);
		break;

	case inputSYM:
		expand_input(line);
		break;

	case openSYM:
		line->lc.openrec.filenum = expand_exp();
		line->lc.openrec.filename = expand_exp();
		line->lc.openrec.reclen = expand_exp();
		line->lc.openrec.read_only = expand_getint();
		line->lc.openrec.type = expand_getint();
		break;
	
	case createSYM:
		line->lc.createrec.filename = expand_exp();
		line->lc.createrec.top = expand_exp();
		line->lc.createrec.reclen = expand_exp();
		break;

	case printSYM:
		expand_print(line);
		break;

	case readSYM:
		line->lc.readrec.modifier=expand_alloc_twoexp();
		line->lc.readrec.lvalroot = expand_explist();
		break;

	case whenSYM:
		line->lc.whenroot = expand_whenlist();
		break;

	case repeatSYM:
	case whileSYM:
		expand_ifwhile(line);
		break;

	case writeSYM:
		expand_twoexp(&line->lc.writerec.twoexp);
		line->lc.writerec.exproot = expand_explist();
		break;

	case becomesSYM:
		line->lc.assignroot = expand_assign();
		break;

	default:
		fatal("cmd switch default action (expand_horse)");
	}
#ifndef NDEBUG
	if (comal_debug) {
		DBG_PRINTF(0, "Line expanded: ");
		puts_line(MSG_DEBUG, line);
	}
#endif
	return line;
}


PUBLIC struct comal_line *expand_fromfile(char *fname)
{
	struct comal_line *root = NULL;
	struct comal_line *line;
	struct comal_line *last = NULL;
	char *checkstr;
	const char *s;
	extern int eof(int file);

	sqash_file = open(fname, O_RDONLY | O_BINARY);

	if (sqash_file < 0)
		run_error(OPEN_ERR, "File open error: %s",
			  strerror(errno));

	sqash_buf = (char *)mem_alloc(MISC_POOL, SQASH_BUFSIZE);
	sqash_i = MAXUNSIGNED;

	for (s=SQ_MARKER; *s; s++)
		if (expand_getc()!=*s)
			run_error(SQASH_ERR, "Not an OpenComal SAVE file");

	if (expand_getc()!=HOST_OS_CODE)
		run_error(SQASH_ERR,
			  "File not an OpenComal file saved under this OS");

	checkstr = expand_getstr2(SQ_CONTROL);

	if (strcmp(checkstr, HOST_OS) != 0)
		run_error(SQASH_ERR,
			  "File not an OpenComal file saved under this OS");

	mem_free(checkstr);

	if (expand_getint() != SQ_VERSION)
		run_error(SQASH_ERR,
			  "File has been saved under a different version of the Sqasher");

	checkstr = expand_getstr2(SQ_CONTROL);

	if (strcmp(checkstr, SQ_COPYRIGHT_MSG) != 0)
		fatal("Internal sqash/expand error #6");

	mem_free(checkstr);

	while (expand_peekc() == SQ_LINE) {
		line = expand_horse();

		if (last)
			last->ld->next = line;
		else
			root = line;

		last = line;
	}

	checkstr = expand_getstr2(SQ_CONTROL);

	if (strcmp(checkstr, SQ_COPYRIGHT_MSG) != 0)
		fatal("Internal sqash/expand error #7");

	if (!eof(sqash_file) || sqash_i < sqash_hwm)
		fatal("Internal sqash/expand error #8");

	mem_free(checkstr);
	mem_free(sqash_buf);

	if (close(sqash_file) < 0)
		run_error(CLOSE_ERR, "Error closing file: %s",
			  strerror(errno));

	return root;
}
