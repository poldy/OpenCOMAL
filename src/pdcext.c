/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal 'default' extensions */

#include "pdcglob.h"
#include "pdcstr.h"
#include "pdcmisc.h"
#include "pdcexec.h"
#include "pdcexp.h"
#include "pdcval.h"
#include "pdcsym.h"
#include "pdcext.h"
#include "version.h"
#include <string.h>
#include <assert.h>

struct inpfile_stkent {
	struct inpfile_stkent *next;
	FILE *inpfile;
};

PRIVATE struct inpfile_stkent *inpfile_root = NULL;
PRIVATE FILE *sys_inpfile = NULL;
PRIVATE FILE *sys_outfile = NULL;


PUBLIC void ext_init()
{
}


PUBLIC void ext_tini()
{
}


PUBLIC int ext_call_scan(struct id_rec *id, struct exp_list *exproot,
			 char *errtext)
{
	return -1;
}


PUBLIC int ext_call(struct id_rec *id, struct exp_list *exproot, int
		    calltype, void **result, enum VAL_TYPE *type)
{
	return 0;
}


PRIVATE int *is_flag(char *cmd)
{
	extern int yydebug;
	extern int show_exec;
	extern int short_circuit;
#ifndef NDEBUG
	extern int comal_debug;

	if (strcmp(cmd, "debug") == 0)
		return &comal_debug;
#endif
	if (strcmp(cmd, "yydebug") == 0)
		return &yydebug;

	if (strcmp(cmd, "show_exec") == 0)
		return &show_exec;

	if (strcmp(cmd, "prog_trace") == 0)
		return &curenv->trace;

	if (strcmp(cmd, "short_circuit") == 0)
		return &short_circuit;

	return NULL;
}


PUBLIC int ext_sys_exp(struct exp_list *exproot, void **result, enum
		       VAL_TYPE *type)
{
	char *cmd;

	cmd = exp_cmd(exproot->exp);
	exproot = exproot->next;

	if (strcmp(cmd, "version") == 0) {
		if (exproot)
			run_error(SYS_ERR,
				  "SYS(version) takes no parameters");

		*result = cell_alloc(FLOAT_CPOOL);
		sscanf(VERSION, "%lG", *(double **) result);
		*type = V_FLOAT;
	} else {
		int *flag;

		flag = is_flag(cmd);

		if (!flag)
			return -1;

		*result = cell_alloc(INT_CPOOL);
		**(long **) result = *flag;
		*type = V_INT;
	}

	return 0;
}


PUBLIC int ext_syss_exp(struct exp_list *exproot, struct string
			**result, enum VAL_TYPE *type)
{
	char *cmd;
	int *flag;
	const char *s;
	extern char *sys_interpreter();

	cmd = exp_cmd(exproot->exp);
	exproot = exproot->next;

	if (strcmp(cmd, "host") == 0) {
		if (exproot)
			run_error(SYS_ERR,
				  "SYS$(host) takes no parameters");

		s = HOST_OS;
	} else if (strcmp(cmd, "interpreter") == 0) {
		if (exproot)
			run_error(SYS_ERR,
				  "SYS$(interpreter) takes no parameters");

		s = sys_interpreter();
	} else if (strcmp(cmd, "version") == 0) {
		if (exproot)
			run_error(SYS_ERR,
				  "SYS$(version) takes no parameters");

		s = OPENCOMAL_VERSION;
	} else {
		flag = is_flag(cmd);

		if (!flag)
			return -1;

		s = *flag ? "on" : "off";
	}

	*result = str_make(RUN_POOL, s);
	*type = V_STRING;

	return 0;
}


PRIVATE void push_inpfile()
{
	struct inpfile_stkent *work = (struct inpfile_stkent *)mem_alloc(MISC_POOL, sizeof(*work));

	work->inpfile = sys_inpfile;
	work->next = inpfile_root;
	inpfile_root = work;
}


PRIVATE void pop_inpfile()
{
	struct inpfile_stkent *work = inpfile_root;

	if (!work)
		return;

	inpfile_root = work->next;
	sys_inpfile = work->inpfile;
	mem_free(work);
}


PRIVATE void pop_all_inpfiles()
{
	while (inpfile_root) {
		pop_inpfile();

		if (sys_inpfile)
			fclose(sys_inpfile);
	}
}


PUBLIC int ext_sys_stat(struct exp_list *exproot)
{
	void *result;
	struct string *name;
	enum VAL_TYPE type;
	int *flag;
	char *cmd;

	flag = is_flag(exp_cmd(exproot->exp));

	if (flag) {
		exproot = exproot->next;

		if (!exproot)
			run_error(SYS_ERR, "Expected 'on' or 'off'");

		cmd = exp_cmd(exproot->exp);

		if (strcmp(cmd, "on") == 0)
			*flag = 1;
		else if (strcmp(cmd, "off") == 0)
			*flag = 0;
		else
			run_error(SYS_ERR, "Specify 'on' or 'off' please");

		return 0;
	}

	cmd = exp_cmd(exproot->exp);

	if (strcmp(cmd, "sysin") == 0) {
		if (!exproot->next)
			run_error(SYS_ERR,
				  "Too few parameters for SYS sysin");

		if (exproot->next->next)
			run_error(SYS_ERR,
				  "Too much parameters for SYS sysin");

		calc_exp(exproot->next->exp, (void **) &name, &type);

		if (type != V_STRING)
			run_error(TYPE_ERR,
				  "Sysin parameter not a string");

		if (name->len == 0)
			pop_all_inpfiles();
		else {
			push_inpfile();

			sys_inpfile = fopen(name->s, "rt");

			if (!sys_inpfile) {
				pop_inpfile();
				run_error(SYS_ERR,
					  "Error opening sysin: %s",
					  strerror(errno));
			}
		}

		val_free(name, type);

		return 0;
	} else if (strcmp(cmd, "sysout") == 0) {
		if (!exproot->next)
			run_error(SYS_ERR,
				  "Too few parameters for SYS sysout");

		if (exproot->next->next)
			run_error(SYS_ERR,
				  "Too much parameters for SYS sysout");

		calc_exp(exproot->next->exp, (void **) &name, &type);

		if (type != V_STRING)
			run_error(TYPE_ERR,
				  "Sysin parameter not a string");

		if (name->s[0] == '\0') {
			fclose(sys_outfile);
			sys_outfile = NULL;
		} else
			sys_outfile = fopen(name->s, "wt");

		val_free(name, type);

		if (!sys_outfile && name->s[0])
			run_error(SYS_ERR, "Error opening sysout: %s",
				  strerror(errno));

		return 0;
	} else if (strcmp(cmd, "memdump") == 0) {
		if (!exproot->next)
			run_error(SYS_ERR,
				  "Too few parameters for SYS memdump");

		if (exproot->next->next)
			run_error(SYS_ERR,
				  "Too much parameters for SYS memdump");

		calc_exp(exproot->next->exp, &result, &type);

		if (type != V_INT)
			run_error(TYPE_ERR, "Integer expected");

		mem_debug((*(long *) result));
		val_free(result, type);

		return 0;
	} else if (strcmp(cmd,"listvars") == 0) {
		if (exproot->next)
			run_error(SYS_ERR,"No parameters allowed for SYS listvars");

		sym_list(curenv->curenv,1);

		return 0;
	} else if (strcmp(cmd,"clrtoeol") == 0) {
		if (exproot->next) {
			run_error(SYS_ERR,"No parameters allowed for SYS clrtoeol");
		}

		sys_clrtoeol(sel_outfile);

		return 0;
	}

	return -1;
}


PUBLIC int ext_get(int stream, char *line, int maxlen, const char *prompt)
{
	line[0] = '\0';

	if (sys_inpfile) {
		int eof;

		if (fgets(line, maxlen, sys_inpfile))
			return 1;

		eof = feof(sys_inpfile);
		fclose(sys_inpfile);
		pop_inpfile();

		if (!eof)
			run_error(SYS_ERR, "Error reading sysin: %s",
				  strerror(errno));

		return ext_get(stream, line, maxlen, prompt);
	}

	return 0;
}


PUBLIC bool ext_nl(void)
{
	if (sys_outfile != NULL) {
		fputc('\n', sys_outfile);
		return true;
	} else {
		return false;
	}
}


PUBLIC bool ext_ht(void)
{
	if (sys_outfile != NULL) {
		fputc('\t', sys_outfile);
		return true;
	} else {
		return false;
	}
}


PUBLIC void ext_page(FILE *f)
{
	assert(f != NULL);
	fputc('\f', f);
}


PUBLIC void ext_clrtoeol(FILE *f)
{
	assert(f != NULL);
}


PUBLIC void ext_cursor(FILE *f, int y, int x)
{
	assert(f != NULL);
}


PUBLIC bool ext_put(int stream, const char *buf, long len)
{
	if (sys_outfile != NULL) {
		fputs(buf, sys_outfile);
		return true;
	} else {
		return false;
	}
}
