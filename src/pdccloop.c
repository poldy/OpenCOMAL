/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* Main file of OpenComal Command loop */

#define _XOPEN_SOURCE 700

#include <stdbool.h>
#include <string.h>
#include "pdcglob.h"
#include "pdcsym.h"
#include "pdcmisc.h"
#include "pdcparss.h"
#include "pdcprog.h"
#include "pdccmd.h"
#include "pdcexec.h"
#include "pdclexs.h"
#include "pdcenv.h"
#include "msgnrs.h"
#include "pdccloop.h"

PUBLIC const char *sys_interpreter(void)
{
	return "OpenComal";
}


PUBLIC bool process_comal_line(struct comal_line *line)
{
        bool result = false;

	if (!line)
		return false;

	if (line->cmd >= 0) {
		if (line->ld) {
			prog_addline(line);
			mem_shiftmem(PARSE_POOL, curenv->program_pool);
		} else if (line->cmd != 0) {
			if (setjmp(ERRBUF) == 0)
				if (!cmd_exec(line, &result)) {
					if (!curenv->curenv)
						curenv->curenv = ROOTENV();

					exec_line(line);
				}

			give_run_err(NULL);
		}
	}

	mem_freepool(PARSE_POOL);
        return result;
}


PUBLIC struct comal_line *crunch_line(char *line)
{
	extern struct comal_line c_line;
	struct comal_line *work;

	while (true) {
		int rc;
		int errpos;

		lex_setinput(line);
		rc = yyparse();

		if (rc)
			pars_error("Arfle Barfle Gloop?");

		errpos = pars_handle_error();

		if (!errpos) {
			work = stat_dup(&c_line);

			return work;
		}

		remove_trailing(line,"\n","");
		remove_trailing(line,"\r","");

		if (sys_edit(MSG_DIALOG, line, MAX_LINELEN)) {
			my_printf(MSG_DIALOG, true, "%s", catgets(catdesc, CommonSet, CommonEscape, "Escape"));
			return NULL;
		}
	}
}


PUBLIC void comal_loop(int newstate)
{
	char line[MAX_LINELEN];
	struct comal_line *aline;
	jmp_buf save_err;
	const char *prompt="$ ";
        bool ret = false;

	curenv->running = newstate;
	memcpy(save_err, ERRBUF, sizeof(jmp_buf));

        do {
		if (curenv->running==HALTED) 
			prompt=catgets(catdesc, CLoopSet, CLoopHaltedPrompt, "(halted)$ ");

		if (!sys_get(MSG_DIALOG, line, MAX_LINELEN, prompt)) {
			aline = crunch_line(line);
			ret = process_comal_line(aline);

			if (curenv->con_inhibited) {
				curenv->con_inhibited = false;
				longjmp(RESTART, JUST_RESTART);
			}
		} else
			my_printf(MSG_DIALOG, true, "%s", catgets(catdesc, CommonSet, CommonEscape, "Escape"));
	} while (!ret);

	curenv->running = RUNNING;
	memcpy(ERRBUF, save_err, sizeof(jmp_buf));
}


PUBLIC void pdc_go(int argc __my_unused, char *argv[] __my_unused)
{
	int restart_err;

	restart_err = setjmp(RESTART);

        DBG_PRINTF(true, "Interpreter restart code: %d",
		  restart_err);

	if (restart_err == PROG_END)
		clean_runenv(curenv);

	if (restart_err == RUN)
		prog_run();

	if (restart_err != QUIT)
		comal_loop(CMDLOOP);
}
