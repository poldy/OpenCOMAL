/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal Environment management */

#include "pdcglob.h"
#include "pdcmisc.h"
#include "pdcstr.h"
#include "pdcseg.h"
#include "pdcid.h"
#include "pdcmod.h"
#include "pdcsym.h"
#include "pdcenv.h"
#include <string.h>
#include <stdio.h>

PUBLIC struct comal_env *env_new(const char *name)
{
	struct comal_env *work = GETCORE(MISC_POOL, struct comal_env);
	struct env_list *work2 = GETCORE(MISC_POOL, struct env_list);

	work->progroot = NULL;
	work->segroot = NULL;
	work->envname = my_strdup(MISC_POOL, name);
	work->scan_ok = false;
	work->rootenv = NULL;
	work->curenv = NULL;
	work->changed = false;
	work->name = NULL;
	work->curline = NULL;
	work->trace = 0;
	work->error = 0;
	work->errmsg = NULL;
	work->errline = NULL;
	work->datalptr = NULL;
	work->dataeptr = NULL;
	work->con_inhibited = false;
	work->running = 0;
	work->fileroot = NULL;
	work->lasterr = 0;
	work->lasterrmsg = my_strdup(MISC_POOL, "");
	work->errline = 0;
	work->escallowed = true;
	work->nrtraps = 0;
	work->program_pool = pool_new();

	work2->next = env_root;
	env_root = work2;
	work2->env = work;

	return work;
}

PUBLIC struct comal_env *env_find(char *name)
{
	struct env_list *walk = env_root;

	while (walk) {
		if (strcmp(walk->env->envname, name) == 0)
			break;

		walk = walk->next;
	}

	if (walk)
		return walk->env;

	my_printf(MSG_DIALOG, 1, "Creating new environment %s", name);

	return env_new(name);
}


PUBLIC void clean_runenv(struct comal_env *env)
{
	struct file_rec *fwalk = curenv->fileroot;
	struct comal_line *walk;

        DBG_PRINTF(1, "Cleaning runenv");

	/*
	 * Free all the existing static environments of procedures, functions
	 * and modules.
	 */
	FOR_EACH_LINE(NULL,walk)
		if (walk->cmd==procSYM || walk->cmd==funcSYM || walk->cmd==moduleSYM)
			if (walk->lc.pfrec.staticenv) {
				sym_freeenv(walk->lc.pfrec.staticenv,0);
				walk->lc.pfrec.staticenv=NULL;
			}

	env->lasterr = 0;
	mem_free(env->lasterrmsg);
	env->lasterrmsg = my_strdup(MISC_POOL, "");
	env->errline = 0;
	env->escallowed = true;
	env->nrtraps = 0;
	env->running = 0;
	env->curenv = NULL;

	env->datalptr = NULL;
	env->dataeptr = NULL;

	/*
	 * Close all open files
	 */
	while (fwalk) {
		DBG_PRINTF(1, "Closing comal file %ld",
			  fwalk->cfno);

		fclose(fwalk->hfptr);
		fwalk = fwalk->next;
	}

	curenv->fileroot = NULL;

	mod_freeall();
	seg_allfree();
	mem_freepool(RUN_POOL);
	trace_reset();
}


PUBLIC void clear_env(struct comal_env *env)
{
	clean_runenv(env);

	env->progroot = NULL;
	env->rootenv = NULL;
	env->changed = false;
	env->scan_ok = false;

	if (env->name)
		mem_free(env->name);

	env->name = NULL;
}
