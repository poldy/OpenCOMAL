/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal Misc Routines */

#include "pdcglob.h"
#include "pdcid.h"
#include "pdcstr.h"
#include "pdcsym.h"
#include "pdcsys.h"
#include "pdcexec.h"
#include "pdclist.h"
#include "pdcmisc.h"

#include <math.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>


struct int_trace {
	struct int_trace *next;
	char *name;
	int *value;
};

PRIVATE struct int_trace *tr_root = NULL;


PUBLIC void my_nl(int stream)
{
	if (sel_outfile && stream == MSG_PROGRAM) {
		if (fputc('\n', sel_outfile) == EOF)
			run_error(SELECT_ERR,
				  "Error when writing to SELECT OUTPUT file %s",
				  strerror(errno));
	} else
		sys_nl(stream);
}


PUBLIC void my_put(int stream, const char *buf, long len)
{
	if (sel_outfile && stream == MSG_PROGRAM) {
		if (fputs(buf, sel_outfile) == EOF)
			run_error(SELECT_ERR,
				  "Error when writing to SELECT OUTPUT file %s",
				  strerror(errno));
	} else
		sys_put(stream, buf, len);
}


PUBLIC void my_printf(int stream, int newline, const char *s, ...)
{
	char buf[MAX_LINELEN];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, MAX_LINELEN, s, ap);
	va_end(ap);
	my_put(stream, buf, -1L);

	if (newline)
		my_nl(stream);
}


PUBLIC void fatal(const char *s, ...)
{
	char buf[140];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, 140, s, ap);
	va_end(ap);
	my_printf(MSG_ERROR, 1, "FATAL error: %s", buf);

	longjmp(RESTART, ERR_FATAL);
}


PUBLIC void *my_reverse(void *root)
{
	struct my_list *last = NULL;
	struct my_list *walk = (struct my_list *) root;
	struct my_list *next;

	while (walk) {
		next = walk->next;
		walk->next = last;
		last = walk;
		walk = next;
	}

	return last;
}


PUBLIC void free_list(struct my_list *root)
{
	if (!root)
		return;

	while (root)
		root = (struct my_list *)mem_free(root);
}


PUBLIC int exp_list_of_nums(struct exp_list *root)
{
	while (root) {
		if (root->exp->optype != T_EXP_IS_NUM)
			return 0;

		root = root->next;
	}

	return 1;
}


PUBLIC int check_changed()
{
	if (!curenv->changed)
		return 1;

	return sys_yn(MSG_DIALOG,
		      "Latest changes have not yet been saved! Proceed? ");
}

PUBLIC int check_changed_any()
{
	struct env_list *walk = env_root;
	int any_changes = 0;

	while (walk) {
		if (walk->env->changed) {
			my_printf(MSG_DIALOG, 1,
				  "Environment %s contains unsaved changes!",
				  walk->env->envname);
			any_changes = 1;
		}

		walk = walk->next;
	}

	if (any_changes)
		return sys_yn(MSG_DIALOG, "Proceed? ");

	return 1;
}


PUBLIC void puts_line(int stream, struct comal_line *line)
{
	char buf[MAX_LINELEN];
	char *buf2;

	if (line) {
		buf2 = buf;
		line_list(&buf2, line);
		my_printf(stream, 1, buf);
	} else
		my_printf(stream, 1, "<NULL Line>");
}


PUBLIC int nr_items(struct my_list *list)
{
	int n = 0;

	while (list) {
		n++;
		list = list->next;
	}

	return n;
}


PUBLIC long d2int(double x, int whole)
{
	double max = INT_MAX;
	double min = INT_MIN;

	if (x > max || x < min)
		run_error(F2INT1_ERR,
			  "Floating point value too large to convert to integer");

	if (whole)
		if (floor(x) != x)
			run_error(F2INT2_ERR,
				  "Floating point value contains a fractional part (is not whole)");

	return (long) x;
}


PUBLIC int type_match1(struct id_rec *id, struct expression *exp)
{
	return !((id->type == V_STRING && exp->optype == T_EXP_IS_NUM) ||
		 (id->type != V_STRING && exp->optype == T_EXP_IS_STRING));
}


PUBLIC struct id_rec *exp_of_id(struct expression *exp)
{
	struct id_rec *id = NULL;

	if (exp->optype != T_EXP_IS_NUM && exp->optype != T_EXP_IS_STRING)
		fatal("Exp_id() internal error #1");

	exp = exp->e.exp;

	if (exp->optype == T_ID)
		id = exp->e.expid.id;
	else if (exp->optype == T_SID) {
		id = exp->e.expsid.id;

		if (exp->e.expsid.twoexp)
			run_error(ID_ERR,
				  "Expression should be an identifier only (no substring spec.)");
	} else
		run_error(ID_ERR, "Expression is not an identifier");

	if (exp->e.expid.exproot)
		run_error(ID_ERR,
			  "Expression should be an identifier only (no parm's or dim's)");

	return id;
}

PUBLIC int exp_of_string(struct expression *exp)
{
	if (exp->optype != T_EXP_IS_STRING)
		return 0;

	exp = exp->e.exp;

	return (exp->optype == T_STRING);
}


PUBLIC char *exp_cmd(struct expression *exp)
{
	return (exp_of_id(exp))->name;
}


PUBLIC struct comal_line *search_line(long l, int exact)
{
	struct comal_line *work = curenv->progroot;

	DBG_PRINTF(1, "Searching line %ld", l);

	while (work && work->ld->lineno < l)
		work = work->ld->next;

	if (!work)
		return NULL;

	if (exact && work->ld->lineno != l)
		work = NULL;
#ifndef NDEBUG
        if (comal_debug) {
	        if (work) {
		        puts_line(MSG_DEBUG, work);
                } else {
		        DBG_PRINTF(1, "Returning NULL");
                }
        }
#endif
	return work;
}

extern struct comal_line c_line;

PRIVATE struct {
	int cmd;
	int size;
} sizetab[] = {
	{
	0, 0}, {
	quitSYM, 0}, {
	saveSYM, sizeof(c_line.lc.str)}
	, {
	scanSYM, 0}
	, {
	loadSYM, sizeof(c_line.lc.str)}
	, {
	enterSYM, sizeof(c_line.lc.str)}
	, {
	envSYM, sizeof(c_line.lc.id)}
	, {
	COMMAND(runSYM), sizeof(c_line.lc.str)}
	, {
	newSYM, 0}
	, {
	autoSYM, sizeof(c_line.lc.twonum)}
	, {
	contSYM, 0}
	, {
	COMMAND(delSYM), sizeof(c_line.lc.twonum)}
	, {
	editSYM, sizeof(c_line.lc.twonum)}
	, {
	renumberSYM, sizeof(c_line.lc.twonum)}
	, {
	listSYM, sizeof(c_line.lc.listrec)}
	, {
	dirSYM, sizeof(c_line.lc.exp)}
	, {
	unitSYM, sizeof(c_line.lc.exp)}
	, {
	useSYM, sizeof(c_line.lc.idroot)}
	, {
	exportSYM, sizeof(c_line.lc.idroot)}
	, {
	execSYM, sizeof(c_line.lc.exp)}
	, {
	elseSYM, 0}
	, {
	endSYM, 0}
	, {
	endcaseSYM, 0}
	, {
	endfuncSYM, 0}
	, {
	endifSYM, 0}
	, {
	loopSYM, 0}
	, {
	endloopSYM, 0}
	, {
	endprocSYM, 0}
	, {
	endwhileSYM, 0}
	, {
	endforSYM, 0}
	, {
	otherwiseSYM, 0}
	, {
	repeatSYM, sizeof(c_line.lc.ifwhilerec)}
	, {
	trapSYM, sizeof(c_line.lc.traprec)}
	, {
	handlerSYM, 0}
	, {
	endtrapSYM, 0}
	, {
	retrySYM, 0}
	, {
	nullSYM, 0}
	, {
	stopSYM, sizeof(c_line.lc.exp)}
	, {
	exitSYM, sizeof(c_line.lc.exp)}
	, {
	caseSYM, sizeof(c_line.lc.exp)}
	, {
	chdirSYM, sizeof(c_line.lc.exp)}
	, {
	closeSYM, sizeof(c_line.lc.exproot)}
	, {
	cursorSYM, sizeof(c_line.lc.twoexp)}
	, {
	dataSYM, sizeof(c_line.lc.exproot)}
	, {
	delSYM, sizeof(c_line.lc.exp)}
	, {
	dimSYM, sizeof(c_line.lc.dimroot)}
	, {
	staticSYM, sizeof(c_line.lc.dimroot)}
	, {
	localSYM, sizeof(c_line.lc.dimroot)}
	, {
	elifSYM, sizeof(c_line.lc.exp)}
	, {
	exitSYM, sizeof(c_line.lc.exp)}
	, {
	execSYM, sizeof(c_line.lc.exp)}
	, {
	forSYM, sizeof(c_line.lc.forrec)}
	, {
	funcSYM, sizeof(c_line.lc.pfrec)}
	, {
	ifSYM, sizeof(c_line.lc.ifwhilerec)}
	, {
	importSYM, sizeof(c_line.lc.importrec)}
	, {
	inputSYM, sizeof(c_line.lc.inputrec)}
	, {
	mkdirSYM, sizeof(c_line.lc.exp)}
	, {
	openSYM, sizeof(c_line.lc.openrec)}
	, {
	osSYM, sizeof(c_line.lc.exp)}
	, {
	pageSYM, 0}
	, {
	printSYM, sizeof(c_line.lc.printrec)}
	, {
	procSYM, sizeof(c_line.lc.pfrec)}
	, {
	readSYM, sizeof(c_line.lc.readrec)}
	, {
	restoreSYM, sizeof(c_line.lc.id)}
	, {
	returnSYM, sizeof(c_line.lc.exp)}
	, {
	rmdirSYM, sizeof(c_line.lc.exp)}
	, {
	runSYM, sizeof(c_line.lc.exp)}
	, {
	select_outputSYM, sizeof(c_line.lc.exp)}
	, {
	select_inputSYM, sizeof(c_line.lc.exp)}
	, {
	sysSYM, sizeof(c_line.lc.exproot)}
	, {
	untilSYM, sizeof(c_line.lc.exp)}
	, {
	traceSYM, sizeof(c_line.lc.exp)}
	, {
	whenSYM, sizeof(c_line.lc.whenroot)}
	, {
	whileSYM, sizeof(c_line.lc.ifwhilerec)}
	, {
	writeSYM, sizeof(c_line.lc.writerec)}
	, {
	becomesSYM, sizeof(c_line.lc.assignroot)}
	, {
	endmoduleSYM, 0}
	, {
	moduleSYM, sizeof(c_line.lc.pfrec)}
	, {
	idSYM, sizeof(c_line.lc.id)}
	, {
	randomizeSYM, sizeof(c_line.lc.exp)}
	, {
	reportSYM, sizeof(c_line.lc.exp)}
	, {
	-1, -1}
};


PUBLIC int stat_size(int cmd)
{
	int i = 0;

	while (sizetab[i].cmd >= 0 && sizetab[i].cmd != cmd)
		i++;

	if (sizetab[i].cmd < 0)
		fatal("stat_size() internal error");

	i = sizetab[i].size + sizeof(struct comal_line) -
	    sizeof(union line_contents);

	DBG_PRINTF(1, "Stat_size returns %d", i);

	return i;
}


PUBLIC void give_run_err(struct comal_line *line)
{
	if (curenv->error) {
		if (line)
			puts_line(MSG_ERROR, line);

		my_printf(MSG_ERROR, 1, curenv->errmsg);
		mem_free(curenv->errmsg);
		curenv->errmsg = NULL;
		curenv->error = 0;
	}
}


PUBLIC int type_size(enum VAL_TYPE t)
{
	if (t == V_INT)
		return sizeof(long);
	else if (t == V_FLOAT)
		return sizeof(double);
	else if (t == V_STRING)
		return sizeof(struct string *);
	else
		fatal("type_size(%d) invalid type", t);

	/* NOTREACHED */
	return 0;
}


PUBLIC void check_lval(struct expression *exp)
{
	struct sym_item *sym;

	if (exp->optype == T_EXP_IS_NUM || exp->optype == T_EXP_IS_STRING)
		exp = exp->e.exp;

	if (exp->optype != T_ID && exp->optype != T_SID && exp->optype!=T_ARRAY && exp->optype!=T_SARRAY)
		run_error(LVAL_ERR, "Expression is not an lvalue");

	sym = sym_search(curenv->curenv, exp->e.expid.id, S_VAR);

	if (!sym)
		run_error(LVAL_ERR,
			  "Lvalue is not a variable in current environment");

	if (exp->optype == T_SID && exp->e.expsid.twoexp)
		run_error(LVAL_ERR,
			  "This string lvalue cannot have a substring specifier");
}


PUBLIC int proclevel(struct comal_line *proc)
{
	int i;

#ifndef NDEBUG
	if (comal_debug) {
		DBG_PRINTF(0, "ProcLevel of ");
		puts_line(MSG_DEBUG, proc);
	}
#endif
	if (proc)
		i = proc->lc.pfrec.level;
	else
		i = -1;

	DBG_PRINTF(1, "  Returns %d", i);

	return i;
}

/*
 * This method returns the *actual* Comal statement in a line.
 * I.e. in most cases the line, but in case of a short form FOR, 
 * IF, WHILE or REPEAT it returns the "body" of the short form
 * structure statement.
 */
PUBLIC struct comal_line *line_2line(struct comal_line *line)
{
	switch (line->cmd) {
	case forSYM:
		return line->lc.forrec.stat;

	case ifSYM:
	case whileSYM:
	case repeatSYM:
		return line->lc.ifwhilerec.stat;
	}

	return NULL;
}


PUBLIC int line_2cmd(struct comal_line *line)
{
	line = line_2line(line);

	if (line)
		return line->cmd;

	return 0;
}


PUBLIC struct comal_line *stat_dup(struct comal_line *stat)
{
	int memsize = stat_size(stat->cmd);
	struct comal_line *work;
	char *to;
	char *from;

	work = (struct comal_line *)mem_alloc(PARSE_POOL, memsize);

	for (to = (char *) work, from = (char *) stat; memsize > 0;
	     memsize--, to++, from++)
		*to = *from;

	return work;
}


PUBLIC void trace_trace()
{
	struct int_trace *work = tr_root;

	while (work) {
		my_printf(MSG_TRACE, 1, "Trace %s value = %d", work->name,
			  *(work->value));
		work = work->next;
	}
}


PUBLIC void trace_reset()
{
	struct int_trace *work = tr_root;

	while (work)
		work = (struct int_trace *)mem_free(work);
}

#ifndef EVIL32

PUBLIC void strupr(char *s)
{
	while (*s) {
                *s = toupper_l(*s, latin_loc);

		s++;
	}
}

PUBLIC void strlwr(char *s)
{
	while (*s) {
                *s = tolower_l(*s, latin_loc);

		s++;
	}
}

#endif

#ifdef UNIX

PUBLIC char *ltoa(long num, char *buf, int radix)
{
        assert(radix == 10);
	sprintf(buf, "%ld", num);

	return buf;
}

PUBLIC int eof(int f)
{
	char dummy;

	if (read(f, &dummy, 1) == 0)
		return 1;

	lseek(f, -1L, SEEK_CUR);

	return 0;
}
#endif

PUBLIC void remove_trailing(char *s, const char *trailer, const char *subst) 
{
	int l=strlen(s);
	int m=strlen(trailer);

	if (m>l) return;

	if (strcmp(s+l-m,trailer)==0)
		strcpy(s+l-m,subst);

	return;
}

PUBLIC double my_frac(double x)
{
	if (x>=0)
		return x-floor(x);

	return -(ceil(x)-x);
}

#ifndef HAS_ROUND
PUBLIC double my_round(double x)
{
       double d=my_frac(x);

       if (fabs(d)>=0.5)
               return ceil(x);

       return floor(x);
}
#endif
