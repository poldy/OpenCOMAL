/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal's ID routines */

#define _XOPEN_SOURCE 700

#include "pdcglob.h"
#include "pdcmisc.h"
#include "pdcid.h"
#include "pdcstr.h"
#include <string.h>

PRIVATE struct id_rec *id_root = NULL;

/* With id_eql you can compare two identifiers by their handles. The caller
 * gets 1 if they are equal and 0 if the two identifiers are unequal.
 * STRCMP is used instead of pointer comparision because pointers to the
 * same memory location can take different values on an 8086 CPU environment. 
 */

PUBLIC int id_eql(struct id_rec *id1, struct id_rec *id2)
{
	return (strcmp(id1->name, id2->name) == 0) ? 1 : 0;
}


/* The private function install builds a new record in memory for the
 * specified id name. Memory is allocated from the free pool.
 */

PRIVATE struct id_rec *install(char *idname)
{
	struct id_rec *work;
	int l = strlen(idname);

	work = (struct id_rec *)mem_alloc(MISC_POOL, sizeof(struct id_rec) + l);
	work->left = work->right = NULL;
	term_strncpy(work->name, idname, l + 1);

	switch (work->name[l - 1]) {
	case '#':
		work->type = V_INT;
		break;
	case '$':
		work->type = V_STRING;
		break;
	default:
		work->type = V_FLOAT;
		break;
	}

	return work;
}

/* The next routine does the horse work of searching and installing an
 * identifier.
 */

PRIVATE struct id_rec *id_horse(char *idname)
{
	struct id_rec *walk = id_root;
	struct id_rec *lastone = NULL;
	struct id_rec *installed;
	enum { atroot, overleft, overright } lastchoice = atroot;
	int found = 0;

	while (walk != NULL && !found) {
		int cmp = strcoll_l(idname, walk->name, latin_loc);

		lastone = walk;

		if (cmp < 0) {
			lastchoice = overleft;
			walk = walk->left;
		} else if (cmp > 0) {
			lastchoice = overright;
			walk = walk->right;
		} else
			found = 1;
	}

	if (found)
		return walk;

	installed = install(idname);

	if (lastchoice == atroot)
		return (id_root = installed);
	else if (lastchoice == overleft)
		return (lastone->left = installed);
	else
		return (lastone->right = installed);
}


/* id_search searches for an identifier and if it is not present installs it.
 * A handle for the identifier is returned.
 */

PUBLIC struct id_rec *id_search(char *id)
{
	char idname[MAX_IDLEN];

	term_strncpy(idname, id, MAX_IDLEN);
	strlwr(idname);
	return (struct id_rec *) id_horse(idname);
}
