/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* Processing of the interpreters direct commands */

#include "pdcglob.h"
#include "pdcsym.h"
#include "pdcmisc.h"
#include "pdccloop.h"
#include "pdcstr.h"
#include "pdcprog.h"
#include "pdcexec.h"
#include "pdcscan.h"
#include "pdclist.h"
#include "pdcsqash.h"
#include "pdcenv.h"
#include "msgnrs.h"
#include "pdccmd.h"
#include <string.h>


PRIVATE void cmd_list_horse(struct string *filename, long from, long to)
{
	char buf[MAX_LINELEN];
	char *buf2;
	FILE *listfile;
	struct comal_line *work = curenv->progroot;

	if (filename) {
		listfile = fopen(filename->s, "wt");

		if (!listfile)
			run_error(OPEN_ERR, "File open error %s",
				  strerror(errno));

		setvbuf(listfile, NULL, _IOFBF, TEXT_BUFSIZE);
	} else {
		listfile = NULL;
		sys_setpaged(1);
	}

	while (work && work->ld->lineno < from)
		work = work->ld->next;

	while (work && work->ld->lineno <= to) {
		if (sys_escape()) {
			my_printf(MSG_DIALOG, true, catgets(catdesc, CommonSet, CommonEscape, "Escape"));
			break;
		}

		buf2 = buf;
		line_list(&buf2, work);

		if (listfile)
			fprintf(listfile, "%s\n", buf);
		else
			my_printf(MSG_DIALOG, true, "%s", buf);

		work = work->ld->next;
	}

	if (listfile)
		fclose(listfile);
	else
		sys_setpaged(0);
}


PRIVATE bool cmd_list(struct comal_line *line)
{
	struct comal_line *curline;

	if (!line->lc.listrec.id)
		cmd_list_horse(line->lc.listrec.str,
			       line->lc.listrec.twonum.num1,
			       line->lc.listrec.twonum.num2);
	else {
		long from, to;
		int indent;

		curline = curenv->progroot;

		while (curline
		       &&
		       !((curline->cmd == procSYM
			  || curline->cmd == funcSYM
			  || curline->cmd == moduleSYM)
			 && curline->lc.pfrec.id == line->lc.listrec.id))
			curline = curline->ld->next;

		if (!curline)
			run_error(CMD_ERR, "PROC/FUNC/MODULE %s not found",
				  line->lc.listrec.id->name);

		from = curline->ld->lineno;
		indent = curline->ld->indent;
		curline = curline->ld->next;

		while (curline && curline->ld->indent > indent)
			curline = curline->ld->next;

		if (curline)
			to = curline->ld->lineno;
		else
			to = INT_MAX;

		cmd_list_horse(line->lc.listrec.str, from, to);

	}
        return false;
}


PRIVATE bool cmd_enter(struct comal_line *line)
{
	FILE *yyenter;
	char tline[MAX_LINELEN];
	int stoppen = 0;
	struct comal_line *aline;

	yyenter = fopen(line->lc.str->s, "rt");

	if (!yyenter)
		run_error(OPEN_ERR, "File open error: %s",
			  strerror(errno));

	setvbuf(yyenter, NULL, _IOFBF, TEXT_BUFSIZE);
	++entering;

	while (!stoppen) {
		stoppen = (fgets(tline, MAX_LINELEN - 1, yyenter) == NULL);

		if (stoppen) {
			if (!feof(yyenter))
				run_error(CMD_ERR,
					  "Error when reading ENTER file: %s",
					  strerror(errno));
		} else {
			aline = crunch_line(tline);

			if (!aline) {
				my_nl(MSG_DIALOG);
				my_printf(MSG_DIALOG, true,
					  "Ignored line: %s", tline);
				mem_freepool(PARSE_POOL);
			} else
				process_comal_line(aline);
		}
	}

	fclose(yyenter);
	--entering;
	prog_structure_scan();
        return false;
}


PRIVATE bool cmd_new(struct comal_line *line)
{
	if (check_changed()) {
		prog_new();
		mem_freepool(PARSE_POOL);

		longjmp(RESTART, JUST_RESTART);
	}
        return false;
}


PUBLIC bool cmd_scan(struct comal_line *line)
{
	prog_total_scan();
        return false;
}


PRIVATE bool cmd_save(struct comal_line *line)
{
	if (!line->lc.str && !curenv->name)
		run_error(CMD_ERR, "Missing program name (to SAVE)");
	else if (!curenv->progroot)
		run_error(CMD_ERR, "No program (to SAVE)");
	else {
		if (line->lc.str) {
			if (curenv->name)
				mem_free(curenv->name);

			curenv->name =
			    my_strdup(MISC_POOL, line->lc.str->s);
		} else
			my_printf(MSG_DIALOG, true, curenv->name);

		sqash_2file(curenv->name);
		curenv->changed = false;
	}
        return false;
}


PRIVATE bool cmd_load(struct comal_line *line)
{
	char *fn;

	if (!line->lc.str && !curenv->name)
		run_error(CMD_ERR, "Missing program name (to LOAD)");
	else if (check_changed()) {
		if (line->lc.str)
			fn = line->lc.str->s;
		else {
			fn = curenv->name;
			my_printf(MSG_DIALOG, true, fn);
		}

		fn = my_strdup(MISC_POOL, fn);
		prog_load(fn);
		curenv->name = fn;
		prog_structure_scan();
		mem_freepool(PARSE_POOL);

		longjmp(RESTART, JUST_RESTART);
	}
        return false;
}


PRIVATE bool cmd_auto(struct comal_line *line)
{
	char buf[MAX_LINELEN];
	char *buf2;
	int direct_cmd = 0;
	long nr = line->lc.twonum.num1;
	long step = line->lc.twonum.num2;
        bool result = false;

	while (!direct_cmd) {
		struct comal_line *work;
		struct comal_line *aline;

		if (nr < 0)
			return false;	/* nr<0 after nr+=step past INT_MAX */

		work = search_line(nr, 1);

		if (work) {
			buf2 = buf;
			line_list(&buf2, work);
		} else {
			snprintf(buf, MAX_LINELEN, "%9ld  ", nr);
		}

		if (sys_edit(MSG_DIALOG, buf, MAX_LINELEN, 11))
			return false;

		aline = crunch_line(buf);

		direct_cmd = !aline->ld;
		result = process_comal_line(aline);
		nr += step;
	}
        return result;
}


PRIVATE bool cmd_cont(struct comal_line *line)
{
	if (curenv->running != HALTED)
		run_error(CMD_ERR, "CONtinuation not possible");
        return true;
}


PRIVATE bool cmd_run(struct comal_line *line)
{
	mem_freepool(PARSE_POOL);

	longjmp(RESTART, RUN);
        return false;
}



PRIVATE bool cmd_del(struct comal_line *line)
{
	long from = line->lc.twonum.num1;
	long to = line->lc.twonum.num2;

	if (from == 0 && to == INT_MAX)
		run_error(CMD_ERR,
			  "Please mention a line number range with DEL");

	if (prog_del(&curenv->progroot, from, to, 1))
		prog_structure_scan();
        return false;
}


PRIVATE bool cmd_edit(struct comal_line *line)
{
	char buf[MAX_LINELEN];
	char *buf2;
	long nr = line->lc.twonum.num1;
	long to = line->lc.twonum.num2;
	struct comal_line *work;
        bool result = false;

	while (!result) {
		struct comal_line *aline;

		if (nr > to || nr < 0)
			return false;	/* nr<0 after nr++ at INT_MAX */

		work = search_line(nr, 0);

		if (!work)
			return false;

		nr = work->ld->lineno + 1;

		buf2 = buf;
		line_list(&buf2, work);

		if (sys_edit(MSG_DIALOG, buf, MAX_LINELEN, 10))
			return false;

		aline = crunch_line(buf);
		result = process_comal_line(aline);
	}
        return result;
}


PRIVATE void renum_horse(long num, long step)
{
	struct comal_line *work = curenv->progroot;

	while (work) {
		work->ld->lineno = num;
		num += step;

		if (num < work->ld->lineno) {	/* overflow */
			my_printf(MSG_ERROR, true,
				  "Renumber overflow, renumbering 1,1");
			renum_horse(1L, 1L);
			return;
		}

		work = work->ld->next;
	}
}


PRIVATE bool cmd_renumber(struct comal_line *line)
{
	renum_horse(line->lc.twonum.num1, line->lc.twonum.num2);
        return false;
}


PRIVATE bool cmd_quit(struct comal_line *line)
{
	if (check_changed_any())
		longjmp(RESTART, QUIT);
        return false;
}

PRIVATE void cmd_env_list()
{
	struct env_list *walk = env_root;
	char buf[MAX_LINELEN];
	char *buf2;

	while (walk) {
		if (walk->env->progroot) {
			buf2 = buf;
			line_list(&buf2, walk->env->progroot);
		} else {
			term_strncpy(buf, "       <No program>", MAX_LINELEN);
		}

		my_printf(MSG_DIALOG, true, "Environment %s%s:",
			  walk->env->envname,
			  walk->env == curenv ? " [current]" : "");
		my_printf(MSG_DIALOG, true, buf);

		walk = walk->next;

	}
}

PRIVATE bool cmd_env(struct comal_line *line)
{
	if (line->lc.id == NULL)
		cmd_env_list();
	else
		curenv = env_find(line->lc.id->name);
        return false;
}


PRIVATE struct {
	int sym;
	bool (*func) (struct comal_line *line);
} cmdtab[] = {
	{
	listSYM, cmd_list}, {
	saveSYM, cmd_save}, {
	loadSYM, cmd_load}, {
	editSYM, cmd_edit}, {
	enterSYM, cmd_enter}, {
	envSYM, cmd_env}, {
	COMMAND(runSYM), cmd_run}, {
	newSYM, cmd_new}, {
	scanSYM, cmd_scan}, {
	autoSYM, cmd_auto}, {
	contSYM, cmd_cont}, {
	COMMAND(delSYM), cmd_del}, {
	renumberSYM, cmd_renumber}, {
	quitSYM, cmd_quit}, {
	0, NULL}
};


PUBLIC bool cmd_exec(struct comal_line *line, bool *result)
{
	int i;

	for (i = 0; cmdtab[i].sym && cmdtab[i].sym != line->cmd; i++);

	if (!cmdtab[i].sym)
		return false;

	*result = cmdtab[i].func(line);

	return true;
}
