/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal line execution routines and related */

#include "pdcglob.h"
#include "pdclexs.h"
#include "pdcexp.h"
#include "pdcmisc.h"
#include "pdcsym.h"
#include "pdcid.h"
#include "pdcstr.h"
#include "pdcsqash.h"
#include "pdcseg.h"
#include "pdccloop.h"
#include "pdcval.h"
#include "pdcprog.h"
#include "pdcexec.h"
#include "pdcdsys.h"
#include "pdcmod.h"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>


#ifdef MSDOS
#include <io.h>
#endif


PRIVATE void *return_result;	/* For comms of FUNC results */
PRIVATE enum VAL_TYPE return_type;

PRIVATE int exec_seq3(void);
PRIVATE int exec_seq2(void);

PUBLIC void run_error(int error, const char *s, ...)
{
	char *buf;
	char buf2[MAX_LINELEN];
	va_list ap;

	va_start(ap, s);

	vsnprintf(buf2, MAX_LINELEN, s, ap);
	va_end(ap);

	curenv->error = curenv->lasterr = error;
	mem_free(curenv->lasterrmsg);
	curenv->lasterrmsg = my_strdup(MISC_POOL, buf2);

	buf = (char *)mem_alloc(MISC_POOL, MAX_LINELEN);

	if (curenv->running == RUNNING) {
		snprintf(buf, MAX_LINELEN, "Error %d: \"%s\" at line %ld", error, buf2,
			curenv->curline->ld->lineno);
		curenv->errline = curenv->curline;
		curenv->lasterrline = curenv->curline->ld->lineno;
	} else {
		snprintf(buf, MAX_LINELEN, "Error %d: \"%s\"", error, buf2);
		curenv->errline = NULL;
		curenv->lasterrline = 0;
	}

	curenv->errmsg = buf;

	longjmp(ERRBUF, 666);
}


PRIVATE void exec_temphalt(const char *reason)
{
	puts_line(MSG_DIALOG, curenv->curline);
	my_printf(MSG_DIALOG, 1, reason);

	comal_loop(HALTED);
}

PRIVATE void exec_stop(struct comal_line *line)
{
	char *reason;
	struct string *result;
	enum VAL_TYPE type;

	if (line->lc.exp) {
		calc_exp(line->lc.exp, (void **) &result, &type);
		reason = my_strdup(MISC_POOL, result->s);
		mem_free(result);
	} else
		reason =
		    my_strdup(MISC_POOL,
			      "OpenComal's warp engines answered full stop");

	exec_temphalt(reason);
}

PRIVATE void exec_newvar(struct id_rec *id, struct var_item *var)
{
	struct sym_env *env = sym_newvarenv(curenv->curenv);

	if (!sym_enter(env, id, S_VAR, var))
		run_error(VAR_ERR, "Variable %s exists", id->name);
}


PRIVATE void *exec_lval(struct expression *exp, enum VAL_TYPE *type,
			struct var_item **var, long *strlen)
{
	struct id_rec *id = exp->e.expid.id;
	void *lval = exp_lval(exp, type, var, strlen);

	/*
	 * Lvalue (variable) does not exist. Let's create a new one 
	 */
	if (!lval) {
		if (strlen)
			*strlen = DEFAULT_STRLEN;

		*type = id->type;

		if (strlen)
			*var = var_newvar(*type, NULL, *strlen);
		else
			*var = var_newvar(*type, NULL, 0L);

		exec_newvar(id, *var);
		lval = &(*var)->data;
	}

	DBG_PRINTF(1, "Exec_LVAL returns %p", lval);

	return lval;
}


PRIVATE void parm_enter(struct sym_env *env, struct id_rec *id,
			enum SYM_TYPE type, void *ptr, const char *kind)
{
	if (!sym_enter(env, id, type, ptr))
		run_error(PARM_ERR, "%s parameter %s already present",
			  kind, id->name);
}


PRIVATE struct comal_line *routine_search_horse(struct id_rec *id,
						int type,
						struct comal_line *root)
{
	DBG_PRINTF(1, "Searching routine %s", id->name);

	while (root) {
		DBG_PRINTF(1, "  Examining %s",
			  root->lc.pfrec.id->name);

		if (root->cmd == type && root->lc.pfrec.id == id)
			return root;

		root = root->lc.pfrec.proclink;
	}

	return NULL;
}


PRIVATE struct comal_line *routine_search(struct id_rec *id, int type)
{
	struct sym_item *sym;
	struct comal_line *father;

	if (type == funcSYM)
		sym = sym_search(curenv->curenv, id, S_FUNCVAR);
	else
		sym = sym_search(curenv->curenv, id, S_PROCVAR);

	if (sym)
		return sym->data.pfline;

	father = curenv->curenv->curproc;

	while (father) {
		struct comal_line *procline;

		procline =
		    routine_search_horse(id, type,
					 father->lc.pfrec.localproc);

		if (procline)
			return procline;

		father = father->lc.pfrec.fatherproc;
	}

	return routine_search_horse(id, type, curenv->globalproc);
}


PRIVATE void call_enter(struct sym_env *env, struct id_rec *id,
			struct expression *exp, enum SYM_TYPE type,
			int otype)
{
	struct id_rec *procid = exp_of_id(exp);
	struct comal_line *pfline = routine_search(procid, otype);

	if (!pfline)
		run_error(PARM_ERR, "%s %s not found", lex_sym(otype),
			  procid->name);

	if (id->type != procid->type)
		run_error(PARM_ERR, "FUNC types differ (%s, %s)",
			  procid->name, id->name);

	parm_enter(env, id, type, pfline, "PROC/FUNC");
}


PRIVATE struct arr_dim *dim_copy(struct arr_dim *a)
{
	struct arr_dim *d2root = NULL;
	struct arr_dim *d2;
	struct arr_dim *d = a;

	while (d) {
		d2 = (struct arr_dim *)mem_alloc(RUN_POOL, sizeof(struct arr_dim));
		d2->bottom = d->bottom;
		d2->top = d->top;
		d2->next = d2root;
		d2root = d2;

		d = d->next;
	}

	return (struct arr_dim *)my_reverse(d2root);
}

PRIVATE void parm_array_val(struct sym_env *env, struct id_rec *id,
			    struct expression *exp)
{
	struct arr_dim *arrdim;
	char HUGE_POINTER *lval;
	char HUGE_POINTER *lval2;
	enum VAL_TYPE type;
	long strl;
	long nr;
	struct var_item *var,*lvar;
	int size;

	check_lval(exp);
	lval = (char *)exp_lval(exp, &type, &lvar, &strl);

	if (type!=V_ARRAY)
		run_error(PARM_ERR, "Parameter %s should be an array()",id->name);

	DBG_PRINTF(1, "id=%s(%d), type=%d", id->name,
		  id->type, type);

	if (lvar->type != id->type)
		run_error(PARM_ERR,
			  "Type difference in value pass of array");

	type = lvar->type;
	size = type_size(type);
	arrdim = dim_copy(lvar->array->dimroot);
	var = var_newvar(type, arrdim, strl);
	nr = lvar->array->nritems;
	lval2 = (char *)&var->data;

	if (type != V_STRING)
		memcpy(lval2,lval,nr*size);
	else
		while (nr) {
			*(struct string **) lval2 =
			    str_dup(RUN_POOL, *(struct string **) lval);
			lval+=size;
			lval2+=size;
			nr--;
		}

	parm_enter(env, id, S_VAR, var, "Value()");
}


PRIVATE void decode_parmlist(struct sym_env *env, struct parm_list *plist,
			     struct exp_list *elist)
{
	void *result;
	enum VAL_TYPE type;
	struct var_item *var,*lvar;
	struct name_rec *name;
	void *lval;
	long strlen;

	while (plist && elist) {
		if (plist->ref == refSYM) {
			check_lval(elist->exp);
			lval =
			    exp_lval(elist->exp, &type, &lvar, &strlen);

			if (plist->id->type != lvar->type)
				run_error(PARM_ERR,
					  "REF parameter %s wrong type",
					  plist->id->name);

			if (plist->array && type!=V_ARRAY)
				run_error(PARM_ERR,
					  "REF parameter %s should be an array",
					  plist->id->name);
			else if (!plist->array && type==V_ARRAY)
				run_error(PARM_ERR,
					  "REF parameter %s should not be an array",
					  plist->id->name);

			var = var_refvar(lvar, type, strlen, lval);
			parm_enter(env, plist->id, S_VAR, var, "REF");
		} else if (plist->ref == nameSYM) {
			if (!type_match1(plist->id, elist->exp))
				run_error(PARM_ERR,
					  "NAME expression is of the wrong type");

			name = name_new(curenv->curenv, elist->exp);
			parm_enter(env, plist->id, S_NAME, name, "NAME");
		} else if (plist->ref == procSYM)
			call_enter(env, plist->id, elist->exp, S_PROCVAR,
				   procSYM);
		else if (plist->ref == funcSYM)
			call_enter(env, plist->id, elist->exp, S_FUNCVAR,
				   funcSYM);
		else {
			if (plist->array)
				parm_array_val(env, plist->id, elist->exp);
			else {
				calc_exp(elist->exp, &result, &type);
				var =
				    var_newvar(plist->id->type, NULL,
					       INT_MAX);
				val_copy(&var->data, result, var->type,
					 type);
				val_free(result, type);
				parm_enter(env, plist->id, S_VAR, var,
					   "Value");
			}
		}

		plist = plist->next;
		elist = elist->next;
	}

	if (plist && !elist)
		run_error(PARM_ERR, "Too few parameters provided");
	else if (!plist && elist)
		run_error(PARM_ERR, "Too much parameters provided");
}

/*
 * This function is way too complicated...
 */
PUBLIC void exec_call(struct expression *exp, int calltype, void **result,
		      enum VAL_TYPE *type)
{
	struct comal_line *pfline;
	struct comal_line *curline;
	int wasrunning;
	struct sym_env *env=NULL;
	struct sym_env *save_env = curenv->curenv;
	struct seg_des *seg = NULL;
	struct ext_rec *ext = NULL;
	int dynseg_loaded = 0;
	struct comal_line *modline=NULL;
	struct sym_env *modenv = NULL;
	struct sym_env *aliasenv = NULL;

	/*
	 * If we are running in the command loop, first scan the program
	 * before we execute something...
	 */
	if (curenv->running == CMDLOOP) {
		if (!curenv->scan_ok)
			prog_total_scan();

		if (!curenv->scan_ok)
			run_error(DIRECT_ERR,
				  "Execution of direct command aborted due to program structure errors");
	}

	/*
	 * Search the Comal line with the proc/func definition
	 */
	pfline = routine_search(exp->e.expid.id, calltype);

	/*
	 * If we did not find it, it might be an exported proc/func
	 * form a module...
	 */
	if (!pfline) {
		pfline=mod_search_routine(exp->e.expid.id,calltype);

		/*
		 * Or it might be an extension (pdcext.c)
		 */
		if (!pfline) {
			if (sys_call
		    	(exp->e.expid.id, exp->e.expid.exproot, calltype,
		     	result, type))
				return;

			run_error(UNFUNC_ERR, "Unknown identifier %s",
			  	exp->e.expid.id->name);
		}

		/*
		 * The routine was found in an OpenComal module. Next to
		 * the line of the proc/func, also record the line where
		 * the module definition lives...
		 */
		modline=pfline->lc.pfrec.fatherproc;

		/*
		 * Also record the module's static environment
		 */	
		modenv=modline->lc.pfrec.staticenv;
	}

	/*
	 * Is this an externally defined procedure or function?
	 */
	ext = pfline->lc.pfrec.external;

	if (ext) {
		if (ext->seg)
			seg = ext->seg;
		else if (ext->dynamic == staticSYM
			 || (exp_of_string(ext->filename)
			     && ext->dynamic != dynamicSYM))
			seg = seg_static_load(pfline);
		else {
			seg = seg_dynamic_load(pfline);
			dynseg_loaded = 1;
		}

		pfline = seg->procdef;
	}

	/*
	 * This is a call to an EXPORTed routine in a MODULE. We have 
	 * to insert the MODULEs static environment in front of the new
	 * PROC/FUNC environment. This is done by inserting an alias
	 * symbol environment in the chain that refers to the static
	 * environment that was created when the module was initialised.
	 */
	if (modenv) {
		aliasenv = sym_newenv(1,save_env,modenv,modline,
			modline->lc.pfrec.id->name);
		env = sym_newenv(pfline->lc.pfrec.closed, aliasenv, 
			NULL, pfline, exp->e.expid.id->name);
	} else
		env = sym_newenv(pfline->lc.pfrec.closed, save_env, 
			NULL, pfline, exp->e.expid.id->name);

	decode_parmlist(env, pfline->lc.pfrec.parmroot,
			exp->e.expid.exproot);

	curenv->curenv = env;
	curline = curenv->curline;
	curenv->curline = pfline->ld->next;
	wasrunning = curenv->running;
	curenv->running = RUNNING;

	if (exec_seq2() != returnSYM)
		fatal("Internal CALL/RETURN error #1");

	curenv->curline = curline;
	env = sym_freeenv(env, 0);

	if (aliasenv) {
		if (aliasenv!=env) 
			fatal("exec_call internal error #1");

		sym_freeenv(aliasenv,0);
	}

	curenv->curenv = save_env;
	curenv->running = wasrunning;

	if (dynseg_loaded)
		seg_dynamic_free(seg);

	if (calltype == funcSYM) {
		*result = return_result;
		*type = return_type;
	}
}

PRIVATE void do_call(struct expression *exp, int calltype)
{
	void *val = 0;
	enum VAL_TYPE type = V_ERROR;

	exec_call(exp, calltype, &val, &type);

	if (val != 0)
		fatal("Internal error in do_call(), pdcexec.c");
}


PRIVATE void run_chain(struct comal_line *line, int endsym)
{
	while (line->cmd != endsym)
		line = line->lineptr;

	curenv->curline = line;
}


PRIVATE void exec_restore(struct comal_line *line)
{
	struct comal_line *walk;

	if (line->ld) {
		curenv->datalptr = line->lineptr;
		curenv->dataeptr = curenv->datalptr->lc.exproot;
		return;
	}

	walk = curenv->progroot;

	if (line->lc.id) {
		while (walk
		       && !(walk->cmd == idSYM
			    && walk->lc.id == line->lc.id))
			walk = walk->ld->next;

		if (!walk)
			run_error(LABEL_ERR, "Label %s not found",
				  line->lc.id->name);
	}

	while (walk && walk->cmd != dataSYM)
		walk = walk->ld->next;

	if (!walk)
		run_error(DATA_ERR, "No DATA statements found");
	else {
		curenv->datalptr = walk;
		curenv->dataeptr = walk->lc.exproot;
	}
}


PRIVATE struct arr_dim *make_arrdim(struct dim_ension *d)
{
	struct arr_dim *root = NULL;
	struct arr_dim *work;

	while (d && !curenv->error) {
		work = GETCORE(RUN_POOL, struct arr_dim);

		if (d->bottom)
			work->bottom = calc_intexp(d->bottom);
		else
			work->bottom = DEFAULT_DIMBOTTOM;

		if (!curenv->error)
			work->top = calc_intexp(d->top);

		if (work->bottom > work->top)
			run_error(DIM_ERR, "Dimension error (top<bottom)");

		work->next = root;
		root = work;
		d = d->next;
	}

	root = (struct arr_dim *)my_reverse(root);

	return root;
}


PRIVATE void do_1dim(struct dim_list *dim, struct comal_line *line)
{
	struct arr_dim *arrdim = make_arrdim(dim->dimensionroot);
	struct sym_env *env;
	long strlen;

	if (dim->strlen)
		strlen = calc_intexp(dim->strlen);
	else
		strlen = DEFAULT_STRLEN;

	if (line->cmd == localSYM)
		env = curenv->curenv;
	else if (line->cmd == staticSYM) {
		env=curenv->curenv->curproc->lc.pfrec.staticenv;

		if (!env) {
	    		env=sym_newenv(1, NULL, NULL, NULL, 
				line->lc.pfrec.id->name);

			curenv->curenv->curproc->lc.pfrec.staticenv=env;
		} else
			if (sym_search(env,dim->id,S_VAR))
				return;
	} else
		env = sym_newvarenv(curenv->curenv);

	if (!sym_enter
	    (env, dim->id, S_VAR,
	     var_newvar(dim->id->type, arrdim, strlen)))
		run_error(VAR_ERR, "Variable %s exists", dim->id->name);
}


PRIVATE void exec_dim(struct comal_line *line)
{
	struct dim_list *work = line->lc.dimroot;

	while (work) {
		do_1dim(work, line);
		work = work->next;
	}
}

PRIVATE void do_int_assign(long HUGE_POINTER *to, long from, long nr)
{
	while (nr>0) {
		*to=from;
		to++;
		nr--;
	}
}


PRIVATE void do_float_assign(double HUGE_POINTER *to, double from, long nr)
{
	while (nr>0) {
		*to=from;
		to++;
		nr--;
	}
}

PRIVATE void do_num_assign(void *lval, enum VAL_TYPE ltype,
			   struct var_item *lvar, void *rval,
			   enum VAL_TYPE rtype, int must_free_mem)
{

	if (ltype==V_ARRAY) {
		if (lvar->type == V_INT)
			do_int_assign((long HUGE_POINTER *)lval,(rtype==V_INT)?*(long *)rval:d2int(*(double *)rval,0),lvar->array->nritems);
		else
			do_float_assign((double HUGE_POINTER *)lval,(rtype==V_FLOAT)?*(double *)rval:(double)*(long *)rval,lvar->array->nritems);
	} else 
		val_copy(lval,rval,ltype,rtype);

	if (must_free_mem)
		cell_free(rval);
}

PRIVATE void do_num_array_assign(struct var_item *to, struct var_item *from)
{
	void *from_data,*to_data;

	if (!from->array || !to->array)
		fatal("do_num_array_assign internal error #1");

	if (from->array->nritems!=to->array->nritems)
		fatal("do_num_array_assign internal error #2");

	if (from->type!=to->type)
		fatal("do_num_array_assign internal error #3");

	from_data=var_data(from);
	to_data=var_data(to);

	memcpy(to_data,from_data,from->array->nritems*type_size(from->type));
}


PRIVATE void do_calc_fromto(struct two_exp *twoexp, long *from, long *to)
{
	const char *err = NULL;

	if (twoexp) {
		if (twoexp->exp1)
			*from = calc_intexp(twoexp->exp1);
		else
			*from = 1;

		if (twoexp->exp2)
			if (twoexp->exp2==twoexp->exp1)
				*to=*from;
			else
				*to = calc_intexp(twoexp->exp2);
		else
			*to = INT_MAX;

		if (*from > *to)
			err = "Substring specifier incorrect (from>to)";
		else if (*from < 1)
			err = "Substring specifier start < 1";

		if (err)
			run_error(SUBSTR_ERR, err);
	} else
		*from = *to = 0;
}
	

PRIVATE void do_str_assign(struct string **lval, struct two_exp *twoexp,
			   long strl, struct string *rval, int must_free_mem)
{
	long from, to;

	/*
	 * Calculate the substring expression tied to the lval (the variable
	 * to be assigned to. from/to denote the parts of the string that
	 * must be assigned.
	 */
	do_calc_fromto(twoexp, &from, &to);

	/*
	 * Do we have a full string assignment?
	 */
	if (!from) {
		/*
	 	* Free the value to be assigned to
	 	*/
		if (*lval) mem_free(*lval);

		/*
		 * If the expression to be assigned exists in the heap.
		 * we just switch the pointers. Otherwise we must make
		 * a copy
		 */
		if (must_free_mem) {
			*lval=rval;
			must_free_mem=0;
		} else
			*lval=str_maxdup(RUN_POOL,rval,strl);
	} else {
		/*
		 * We have a substring assignment on our hands
		 */

		/*
		 * If the target string does not exist yet -> create it
		 */
		if (!*lval)
			*lval=str_make2(RUN_POOL,to);

		/*
		 * If the upper bound of the substring assignment is
		 * beyond the length of the string -> extend the string
		 */
		if ((*lval)->len<to)
			str_extend(RUN_POOL,lval,to);

		str_partcpy2(*lval,rval,from,to);
	}

	if (must_free_mem)
		mem_free(rval);
}

PRIVATE void do_str_array_assign(struct var_item *lvar, void *rval, enum VAL_TYPE rtype, int must_free_mem)
{
	struct string *str = NULL;
	struct var_item *var;
	struct string **from, **to;
	long nr;

	to=&lvar->data.str[0];

	if (rtype==V_STRING) {
		str=(struct string *)rval;

		for (nr=lvar->array->nritems; nr; --nr) {
			if (*to) mem_free(*to);

			*to=str_dup(RUN_POOL,str);
			to++;
		}
	} else if (rtype==V_ARRAY) {
		var=(struct var_item *)rval;
		from=&var->data.str[0];

		for (nr=lvar->array->nritems; nr; --nr) {
			if (*to) mem_free(*to);

			*to=str_dup(RUN_POOL,*from);
			to++;
			from++;
		}
	} else 
		fatal("do_str_array_assign internal error #1");

	if (rtype==V_STRING && must_free_mem) mem_free(str);
	
}


PRIVATE void do_assign2(struct expression *lval, void *rval,
			enum VAL_TYPE rtype, int must_free_mem)
{
	void *lvalptr;
	struct var_item *lvar;
	long strl;
	enum VAL_TYPE ltype;

	lvalptr = exec_lval(lval, &ltype, &lvar, &strl);

	if (ltype!=V_ARRAY && rtype==V_ARRAY)
		run_error(ARRAY_ERR,"You are trying to assign an array to simple variable %s. Why?",
			lval->e.expid.id->name);

	if (ltype==V_ARRAY && rtype==V_ARRAY) {
		if (lvar->array->nritems!=((struct var_item *)rval)->array->nritems)
			run_error(ARRAY_ERR,"Arrays are of unequal sizes");

		if (lvar->type!=((struct var_item *)rval)->type)
			run_error(ARRAY_ERR,"Arrays are of different type (int/float mismatch)");
	}

	if (lval->optype == T_ID || lval->optype == T_ARRAY)
		if (rtype == V_INT || rtype == V_FLOAT)
			do_num_assign(lvalptr, ltype, lvar, rval, rtype, must_free_mem);
		else if (rtype==V_ARRAY)
			do_num_array_assign(lvar,(struct var_item *)rval);
		else
			run_error(TYPE_ERR,"Type mismatch in assignment (numeric expected)");
	else if (lval->optype == T_SID || lval->optype == T_SARRAY) 
		if (ltype==V_ARRAY)
			do_str_array_assign(lvar,rval,rtype,must_free_mem);
		else if (rtype == V_STRING)
			if (ltype!=V_STRING)
				fatal("do_assign2 internal error #4");
			else
				do_str_assign((struct string **)lvalptr, lval->e.expsid.twoexp, strl, (struct string *)rval, must_free_mem);
		else
			run_error(TYPE_ERR,"Type mismatch in assignment (string expected)");
	else
		fatal("do_assign2 internal error #3");
}


PRIVATE void do_assign1(struct expression *lval, int op,
			struct expression *rval)
{
	void *result;
	enum VAL_TYPE type;
	struct expression exp;

	if (op == becomesSYM)
		calc_exp(rval, &result, &type);
	else {
		exp.optype = T_BINARY;

		switch (op) {
		case becplusSYM:
			exp.op = plusSYM;
			break;
		case becminusSYM:
			exp.op = minusSYM;
			break;

		default:
			fatal
			    ("Assign2 complex assignop switch default action");
		}

		exp.e.twoexp.exp1 = lval;
		exp.e.twoexp.exp2 = rval;

		calc_exp(&exp, &result, &type);
	}

	do_assign2(lval, result, type, 1);
}


PRIVATE void exec_assign(struct comal_line *line)
{
	struct assign_list *work = line->lc.assignroot;

	while (work) {
		do_assign1(work->lval, work->op, work->exp);
		work = work->next;
	}
}


PRIVATE void exec_end()
{
	if (curenv->running != RUNNING) 
		if (!sys_yn(MSG_DIALOG,"Are you sure? "))
			return;

	longjmp(RESTART, PROG_END);
}


PRIVATE int exec_while(struct comal_line *line)
{
	struct ifwhile_rec *w = &line->lc.ifwhilerec;

	if (w->stat)
		while (calc_logexp(w->exp)) {
			int retcode;

			retcode = exec_line(w->stat);

			if (retcode)
				return retcode;
	} else if (!calc_logexp(w->exp))
		curenv->curline = line->lineptr->ld->next;

	return 0;
}

PRIVATE int exec_repeat(struct comal_line *line)
{
	int retcode;
	struct ifwhile_rec *w = &line->lc.ifwhilerec;

	do {
		if (w->stat) {
			retcode = exec_line(w->stat);

			if (retcode) return retcode;
		}
	} while (!calc_logexp(w->exp));

	return 0;
}


PUBLIC int exec_trap(struct comal_line *line)
{
	jmp_buf save_err;
	static int retcode;
	struct seg_des *seg_marker;

	retcode = 0;

	if (line->lc.traprec.esc) {
		curenv->escallowed = (line->lc.traprec.esc == plusSYM);

		return 0;
	}

	memcpy(save_err, ERRBUF, sizeof(jmp_buf));
	seg_marker = curenv->segroot;
	curenv->nrtraps++;

	retry:

	if (setjmp(ERRBUF) == 0) {
		curenv->curline = line->ld->next;
		retcode = exec_seq3();
	}

	curenv->nrtraps--;
	memcpy(ERRBUF, save_err, sizeof(jmp_buf));

	if (curenv->error) {
		curenv->error = 0;
		mem_free(curenv->errmsg);
		curenv->errmsg = NULL;
		curenv->curline = line->lineptr->ld->next;

		while (curenv->segroot != seg_marker)
			seg_dynamic_free(curenv->segroot);

		if (line->lineptr->cmd == handlerSYM) {
			retcode = exec_seq3();

			if (retcode == retrySYM )
				goto retry;
			else if (retcode != endtrapSYM)
				return retcode;
			else {
				curenv->curline =
				    curenv->curline->ld->next;
				return 0;
			}
		}
	} else if (retcode == handlerSYM)
		curenv->curline = curenv->curline->lineptr->ld->next;
	else if (retcode == endtrapSYM)
		curenv->curline = curenv->curline->ld->next;
	else
		return retcode;

	return 0;
}


PRIVATE int exec_exit(struct comal_line *line)
{
	if (curenv->running != RUNNING)
		run_error(DIRECT_ERR,
			  "Can't EXIT in direct mode (use QUIT to leave OpenComal)");

	if (!line->lc.exp || calc_logexp(line->lc.exp))
		return exitSYM;

	return 0;
}

PRIVATE int exec_loop(struct comal_line *line)
{
	int retcode = endloopSYM;

	while (retcode == endloopSYM) {
		curenv->curline = line->ld->next;
		retcode = exec_seq3();
	}

	if (retcode == exitSYM) {
		retcode = 0;
		curenv->curline = line->lineptr->ld->next;
	}

	return retcode;
}


PRIVATE int exec_if(struct comal_line *line)
{
	struct ifwhile_rec *i = &line->lc.ifwhilerec;
	struct comal_line *elif = line->lineptr;

	if (calc_logexp(i->exp))
		if (i->stat)
			return exec_line(i->stat);
		else
			return 0;
	else if (i->stat)
		return 0;
	else {
		while (elif->cmd == elifSYM) {
			if (calc_logexp(elif->lc.exp)) {
				curenv->curline = elif->ld->next;

				return 0;
			}

			elif = elif->lineptr;
		}

		curenv->curline = elif->ld->next;	/* after else or endif */
	}

	return 0;
}


PRIVATE int exec_for(struct comal_line *line)
{
	struct for_rec *f = &line->lc.forrec;
	enum VAL_TYPE ltype;
	struct var_item *var;
	long lto, lstep;
	double dto, dstep;
	void *eresult;
	enum VAL_TYPE etype;
	void *lval;
	int stopfor = 0;
	int retcode;

	lval = exec_lval(f->lval, &ltype, &var, NULL);

	if (ltype==V_ARRAY)
		run_error(FOR_ERR,
			  "FOR loop variable may not be an array");

	calc_exp(f->from, &eresult, &etype);
	val_copy(lval, eresult, ltype, etype);
	cell_free(eresult);

	calc_exp(f->to, &eresult, &etype);

	if (ltype == V_FLOAT) {
		val_copy(&dto, eresult, V_FLOAT, etype);

		if (f->step) {
			cell_free(eresult);
			calc_exp(f->step, &eresult, &etype);
			val_copy(&dstep, eresult, V_FLOAT, etype);

			if (dstep == 0)
				run_error(FOR_ERR,
					  "STEP value must not be 0");
		} else
			dstep = 1;

		if (f->mode == downtoSYM)
			dstep = -dstep;

		DBG_PRINTF(1,
			  "FOR from %lf to %lf step %lf",
			  *(double *) lval, dto, dstep);
	} else {
		val_copy(&lto, eresult, V_INT, etype);

		if (f->step) {
			cell_free(eresult);
			calc_exp(f->step, &eresult, &etype);
			val_copy(&lstep, eresult, V_INT, etype);

			if (lstep == 0)
				run_error(FOR_ERR,
					  "STEP value must not be 0");
		} else
			lstep = 1;

		if (f->mode == downtoSYM)
			lstep = -lstep;

		DBG_PRINTF(1,
			  "FOR from %ld to %ld step %ld",
			  *(long *) lval, lto, lstep);
	}

	cell_free(eresult);

	while (true) {
#ifndef NDEBUG
                if (comal_debug) {
		        if (ltype == V_INT) {
			        DBG_PRINTF(1,
				        "FOR loop, lval=%p, %ld", lval,
				        *(long *) lval);
                        } else {
			        DBG_PRINTF(1,
				        "FOR loop, lval=%p, %lf", lval,
				        *(double *) lval);
		        }
                }
#endif
		if (ltype == V_INT)
			if (lstep > 0)
				stopfor = (*(long *) lval > lto);
			else
				stopfor = (*(long *) lval < lto);
		else if (dstep > 0)
			stopfor = (*(double *) lval > dto);
		else
			stopfor = (*(double *) lval < dto);

		if (stopfor)
			break;

		if (f->stat) {
			retcode = exec_line(f->stat);

			if (retcode)
				return retcode;
		} else {
			curenv->curline = line->ld->next;

			retcode = exec_seq3();

			if (retcode == endforSYM)
				retcode = 0;
			else
				return retcode;
		}

		if (ltype == V_INT)
			*(long *) lval += lstep;
		else
			*(double *) lval += dstep;
	}

	if (!f->stat)
		curenv->curline = line->lineptr->ld->next;

	return 0;
}


PRIVATE void exec_case(struct comal_line *line)
{
	void *cresult;
	enum VAL_TYPE ctype;
	void *wresult;
	enum VAL_TYPE wtype;
	struct comal_line *whenline = line->lineptr;
	struct when_list *walk;
	int casefound = 0;

	calc_exp(line->lc.exp, &cresult, &ctype);

	while (!casefound && whenline->cmd != endcaseSYM) {
		if (whenline->cmd == otherwiseSYM) {
			curenv->curline = whenline->ld->next;
			casefound = 1;
		} else {
			walk = whenline->lc.whenroot;

			while (walk && !casefound) {
				calc_exp(walk->exp, &wresult, &wtype);
				casefound =
				    val_cmp(walk->op, cresult, wresult,
					    ctype, wtype);
				val_free(wresult, wtype);
				walk = walk->next;
			}

			if (casefound)
				curenv->curline = whenline->ld->next;
			else
				whenline = whenline->lineptr;
		}
	}

	val_free(cresult, ctype);
}


PUBLIC struct file_rec *fsearch(long i)
{
	struct file_rec *walk = curenv->fileroot;

	while (walk && walk->cfno != i)
		walk = walk->next;

	return walk;
}


PRIVATE void exec_open(struct comal_line *line)
{
	struct string *name;
	enum VAL_TYPE type;
	struct file_rec *frec;
	struct open_rec *o = &line->lc.openrec;
	int flags = 0;

	calc_exp(o->filename, (void **) &name, &type);
	frec = (struct file_rec *)mem_alloc(RUN_POOL, sizeof(struct file_rec));
	frec->cfno = calc_intexp(o->filenum);

	if (fsearch(frec->cfno))
		run_error(OPEN_ERR, "File %ld already open", frec->cfno);

	frec->mode = o->type;
	frec->read_only = 0;

	switch (frec->mode) {
	case readSYM:
		flags = O_RDONLY;
		frec->read_only = 1;
		break;

	case writeSYM:
		flags = O_RDWR | O_CREAT;
		break;

	case appendSYM:
		flags = O_RDWR | O_APPEND;
		break;

	case randomSYM:
		frec->reclen = calc_intexp(o->reclen);

		if (frec->reclen <= 0)
			run_error(OPEN_ERR, "Invalid record length %ld",
				  frec->reclen);

		if (o->read_only) {
			flags = O_RDONLY;
			frec->read_only = 1;
		} else
			flags = O_RDWR | O_CREAT;

		break;

	default:
		fatal("open filemode switch default action");
	}

	frec->hfno = open(name->s, flags | O_BINARY, S_IRUSR | S_IWUSR);

	if (frec->hfno == -1)
		run_error(OPEN_ERR, "OPEN error: %s", strerror(errno));

	frec->next = curenv->fileroot;
	curenv->fileroot = frec;
}


PRIVATE void exec_close(struct comal_line *line)
{
	struct exp_list *work = line->lc.exproot;
	struct file_rec *walk;

	if (!work) {
		walk = curenv->fileroot;

		while (walk) {
			DBG_PRINTF(1,
				  "Closing Comal file %ld",
				  walk->cfno);

			if (close(walk->hfno) == -1)
				run_error(CLOSE_ERR,
					  "Close error on file %ld: %s",
					  walk->cfno, strerror(errno));

			walk = (struct file_rec *)mem_free(walk);
		}

		curenv->fileroot = NULL;
	} else {
		while (work) {
			struct file_rec *last;
			long fno;

			fno = calc_intexp(work->exp);
			walk = curenv->fileroot;
			last = NULL;

			while (walk && walk->cfno != fno) {
				last = walk;
				walk = walk->next;
			}

			if (!walk)
				run_error(CLOSE_ERR, "File %ld not open",
					  fno);

			if (close(walk->hfno) == -1)
				run_error(CLOSE_ERR,
					  "CLOSE error on file %ld: %s",
					  walk->cfno, strerror(errno));
			else {
				if (last)
					last->next = walk->next;
				else
					curenv->fileroot = walk->next;

				mem_free(walk);
			}

			work = work->next;
		}
	}
}


PRIVATE struct file_rec *pos_file(struct two_exp *r)
{
	long cfno = calc_intexp(r->exp1);
	struct file_rec *f = fsearch(cfno);

	if (!f)
		run_error(POS_ERR, "File %ld not open", cfno);

	if (f->mode == randomSYM) {
		long recno;

		if (!r->exp2)
			run_error(POS_ERR,
				  "Record number needed for a RANDOM file");

		recno = calc_intexp(r->exp2);

		if (recno < 1)
			run_error(POS_ERR,
				  "Random file record number must be >=1");

		DBG_PRINTF(1,
			  "Positioning file %ld (host=%d) to record %ld, offset %ld",
			  f->cfno, f->hfno, recno,
			  (recno - 1) * f->reclen);

		if (lseek(f->hfno, (recno - 1) * f->reclen, SEEK_SET) ==
		    -1)
			run_error(POS_ERR,
				  "Random file positioning error: %s",
				  strerror(errno));
	}

	return f;
}


PRIVATE void read1(struct file_rec *f, struct id_rec *id, void **data,
		   enum VAL_TYPE *type, long *totsize)
{
	long size;
	long r;
	char c;

	*type = (enum VAL_TYPE) 0;
	r = read(f->hfno, &c, 1);
	*type = (enum VAL_TYPE)c;

	if (r > 0) {
		DBG_PRINTF(1,
			  "Reading a type %d from file", *type);

		if (*type != id->type)
			run_error(READ_ERR,
				  "INPUT/READ from file, wrong type (%s)",
				  id->name);

		switch (*type) {
		case V_FLOAT:
			*data = cell_alloc(FLOAT_CPOOL);
			size = sizeof(double);
			break;

		case V_INT:
			*data = cell_alloc(INT_CPOOL);
			size = sizeof(long);
			break;

		case V_STRING:
			r = read(f->hfno, &size, sizeof(long));
			break;

		default:
			fatal("read1 type switch default action");
		}

		if (r > 0) {
			DBG_PRINTF(1,
				  "Reading %ld bytes from file %ld (host %d)",
				  size, f->cfno, f->hfno);

			if (f->mode == randomSYM) {
				*totsize += size + 1;	/* 1 extra for type */

				if (*type == V_STRING)
					*totsize += sizeof(long);

				if (*totsize > f->reclen)
					run_error(READ_ERR,
						  "Random file record overflow");
			}

			if (*type == V_STRING) {
				*data = STR_ALLOC(RUN_POOL, size);
				r = read(f->hfno,
					    (*(struct string **) data)->s,
					    size);
				(*(struct string **) data)->len = size;
			} else
				r = read(f->hfno, (char *)*data, size);
		}
	}

	if (r < 0)
		run_error(READ_ERR, "INPUT/READ file error: %s",
			  strerror(errno));
}


PUBLIC void do_readfile(struct two_exp *twoexp, struct exp_list *lvalroot)
{
	struct file_rec *f = pos_file(twoexp);
	struct exp_list *work = lvalroot;
	void *result;
	enum VAL_TYPE ltype;
	enum VAL_TYPE rtype;
	long totsize = 0;
	struct var_item *var;
	long strl;
	long nr;

	while (work) {
		int size;
		char HUGE_POINTER *lval;

		lval = (char *)exec_lval(work->exp, &ltype, &var, &strl);

		if (ltype==V_ARRAY) {
			nr = var->array->nritems;
			ltype = var->type;
		} else
			nr = 1;

		size=type_size(ltype);

		for (; nr > 0; --nr) {
			read1(f, work->exp->e.expid.id, &result, &rtype,
			      &totsize);

			if (ltype == V_STRING)
				do_str_assign((struct string **)lval, work->exp->e.expsid.twoexp, strl, (struct string *)result, 1);
			else
				do_num_assign((void *)lval, ltype, NULL, result,
					      rtype, 1);

			lval+=size;
		}

		work = work->next;
	}
}


PUBLIC void read_data(struct comal_line *line)
{
	struct exp_list *walk = line->lc.readrec.lvalroot;

	while (walk) {
		if (!curenv->dataeptr)
			run_error(EOD_ERR, "End of DATA");

		do_assign1(walk->exp, becomesSYM, curenv->dataeptr->exp);

		walk = walk->next;

		curenv->dataeptr = curenv->dataeptr->next;

		if (!curenv->dataeptr) {
			do
				curenv->datalptr =
				    curenv->datalptr->ld->next;
			while (curenv->datalptr
			       && curenv->datalptr->cmd != dataSYM);

			if (curenv->datalptr)
				curenv->dataeptr =
				    curenv->datalptr->lc.exproot;
		}
	}
}


PUBLIC void exec_read(struct comal_line *line)
{
	if (line->lc.readrec.modifier)
		do_readfile(line->lc.readrec.modifier,
			    line->lc.readrec.lvalroot);
	else
		read_data(line);
}


PRIVATE void write1(struct file_rec *f, void *data, enum VAL_TYPE type,
		    long *totsize)
{
	long size;
	long w;
	char c;

	switch (type) {
	case V_FLOAT:
		size = sizeof(double);
		break;
	case V_INT:
		size = sizeof(long);
		break;
	case V_STRING:
		if (data)
			size = ((struct string *) data)->len;
		else
			size = 0;
		break;
	default:
		fatal("write1 type switch default action");
	}

	if (f->mode == randomSYM) {
		*totsize += size + 1;	/* 1 extra for type byte */

		if (type == V_STRING)
			*totsize += sizeof(long);	/* str size */

		if (*totsize > f->reclen)
			run_error(WRITE_ERR,
				  "Random file record overflow");
	}

	c = (char) type;
	w = write(f->hfno, &c, 1);

	if (w > 0) {
		DBG_PRINTF(1,
			  "Writing %ld bytes to file %ld (host %d)",
			  size, f->cfno, f->hfno);

		if (type == V_STRING) {
			w = write(f->hfno, &size, sizeof(long));

			if (w > 0)
				w = write(f->hfno,
					     ((struct string *) data)->s,
					     size);
		} else
			w = write(f->hfno, (char *)data, size);
	}

	if (w < 0)
		run_error(WRITE_ERR, "File write error: %s",
			  strerror(errno));
}


PUBLIC void exec_write(struct comal_line *line)
{
	struct file_rec *f = pos_file(&line->lc.writerec.twoexp);
	struct exp_list *work = line->lc.writerec.exproot;
	enum VAL_TYPE type;
	long totsize = 0;
	char HUGE_POINTER *result;
	long nr;
	struct var_item *var;

	if (f->read_only)
		run_error(WRITE_ERR, "File open for READ (only)");

	while (work) {
		int size;

		calc_exp(work->exp, (void **)&result, &type);

		if (type==V_ARRAY) {
			var=(struct var_item *)result;
			result=(char *)&var->data.str[0];
			nr = var->array->nritems;
			type=var->type;
		}
		else
			nr = 1;

		size=type_size(type);

		for (; nr > 0; --nr) {
			write1(f, (type==V_STRING)?(void *)*(struct string **)result:(void *)result, type, &totsize);
			result+=size;
		}

		work = work->next;
	}
}


PUBLIC void print_file(struct two_exp *twoexp,
		       struct print_list *printroot)
{
	struct file_rec *f = pos_file(twoexp);
	struct print_list *work = printroot;
	void *result;
	enum VAL_TYPE type;
	long totsize = 0;

	if (f->read_only)
		run_error(WRITE_ERR, "File open for READ (only)");

	while (work) {
		calc_exp(work->exp, &result, &type);
		write1(f, result, type, &totsize);
		val_free(result, type);
		work = work->next;
	}
}


PRIVATE void print_con(struct print_list *printroot, int pr_sep)
{
	void *result;
	enum VAL_TYPE type;
	struct print_list *work = printroot;

	while (work) {
		calc_exp(work->exp, &result, &type);
		val_print(MSG_PROGRAM, result, type);
		val_free(result, type);
		work = work->next;
	}

	if (!pr_sep)
		my_nl(MSG_PROGRAM);
}


PRIVATE void format_using(char *u, char *p)
{
	int err = 0;
	char *s;
	int decpoint = 0;
	int width = 0, prec = 0;

	for (s = u; *s; s++) {
		width++;

		if (*s == '#') {
			if (decpoint)
				prec++;
		} else if (*s == '.')
			if (decpoint)
				err = 1;
			else
				decpoint = 1;
		else
			err = 1;

		if (err)
			run_error(USING_ERR,
				  "USING string format error @ %s", s);
	}

	sprintf(p, "%%%d.%dlf", width, prec);
}


PRIVATE void print_using(struct expression *str,
			 struct print_list *printroot, int pr_sep)
{
	struct string *usingstr;
	char floatusing[32];
	void *result;
	double d;
	enum VAL_TYPE type;
	struct print_list *work = printroot;

	calc_exp(str, (void **) &usingstr, &type);
	format_using(usingstr->s, floatusing);
	mem_free(usingstr);

	while (work) {
		calc_exp(work->exp, &result, &type);

		if (type == V_INT)
			d = *(long *) result;
		else
			d = *(double *) result;

		val_free(result, type);
		my_printf(MSG_PROGRAM, 0, floatusing, d);

		work = work->next;
	}

	if (!pr_sep)
		my_nl(MSG_PROGRAM);
}


PRIVATE void exec_print(struct comal_line *line)
{
	struct print_rec *p = &line->lc.printrec;

	if (p->modifier)
		if (p->modifier->type == usingSYM)
			print_using(p->modifier->data.str, p->printroot,
				    p->pr_sep);
		else
			print_file(&p->modifier->data.twoexp,
				   p->printroot);
	else
		print_con(p->printroot, p->pr_sep);
}


PRIVATE void exec_selfile(FILE ** f, struct expression *exp, const char *mode)
{
	struct string *result;
	enum VAL_TYPE type;

	if (*f)
		if (fclose(*f))
			run_error(SELECT_ERR,
				  "Error when closing current SELECT file: %s",
				  strerror(errno));

	calc_exp(exp, (void **) &result, &type);

	if (result->len == 0)
		*f = NULL;
	else {
		*f = fopen(result->s, mode);

		if (!*f)
			run_error(SELECT_ERR,
				  "Error when opening new SELECT file: %s",
				  strerror(errno));
	}

	mem_free(result);
}


PRIVATE void exec_import(struct comal_line *line)
{
	struct import_list *work = line->lc.importrec.importroot;
	struct sym_item *importsym;
	struct var_item *var;
	struct sym_env *env;
	static int import_warning=1;

	if (line->lc.importrec.id)
		env =
		    search_env(line->lc.importrec.id->name,
			       curenv->curenv->prev);
	else
		env =
		    search_env_level(curenv->curenv->curproc->lc.pfrec.
				     level - 1, curenv->curenv);

	if (!env)
		run_error(IMPORT_ERR, "Environment not found");

	while (work) {
		importsym = sym_search(env, work->id, S_VAR);

		if (!importsym) {
			/* UniComal compatibility; IMPORT of PROC/FUNC */
			if (!routine_search_horse(work->id,procSYM,curenv->curenv->curproc) &&
			    !routine_search_horse(work->id,funcSYM,curenv->curenv->curproc))
				run_error(IMPORT_ERR,
				  	"IMPORTable item \"%s\" not found",
				  	work->id->name);
			else if (import_warning) {
				puts_line(MSG_DIALOG,line);
				my_printf(MSG_DIALOG,1,"Warning: IMPORT of PROC/FUNC not necessary in OpenComal");
				import_warning=0;
			}
		} else {

			var = importsym->data.var;

			if (var->array && !work->array)
				run_error(IMPORT_ERR,
				  	"IMPORTable symbol %s is an array",
				  work->id->name);
			else if (!var->array && work->array)
				run_error(IMPORT_ERR,
				  	"IMPORTable symbol %s is not an array",
				  	work->id->name);

			var =
		    	var_refvar(var, var->array?V_ARRAY:var->type, var->strlen,
			       	&var->data);

			if (!sym_enter(curenv->curenv, work->id, S_VAR, var))
				run_error(IMPORT_ERR,
				  	"IMPORTable symbol %s already present in this environment",
				  	work->id->name);
		}

		work = work->next;
	}
}


PUBLIC void input_file(struct two_exp *twoexp, struct exp_list *lvalroot)
{
	struct file_rec *f = pos_file(twoexp);
	struct exp_list *work = lvalroot;
	void *result;
	enum VAL_TYPE type;
	long totsize = 0;

	while (work) {
		read1(f, work->exp->e.expid.id, &result, &type, &totsize);
		do_assign2(work->exp, result, type, 1);
		work = work->next;
	}
}


PRIVATE int input_line(char *s, const char *p)
{
	int esc;

	if (sel_infile) {
		if (!fgets(s, MAX_LINELEN - 1, sel_infile)) {
			if (feof(sel_infile)) {
				fclose(sel_infile);
				sel_infile = NULL;

				DBG_PRINTF(1,
					  "Closing SELINPUT file");
			} else
				run_error(SELECT_ERR,
					  "Read error when reading SELECT INPUT file");
		} else {
			*(s + strlen(s) - 1) = '\0';	/* overwrite \n */
			return 0;
		}
	}

	esc = sys_get(MSG_PROGRAM, s, MAX_LINELEN, p);

	return esc;
}


PRIVATE void input_con(struct string *prompt, struct exp_list *lvalroot)
{
	struct exp_list *work = lvalroot;
	enum VAL_TYPE type;
	char line[MAX_LINELEN];
	char field[MAX_LINELEN];
	char *j;
	char *i = NULL;
	int nr;
	int n;
	int quote;
	int esc = 0;
	const char *p;

	if (prompt)
		p = prompt->s;
	else
		p = "? ";

	esc = input_line(line, p);
	i = line;

	while (work) {
		void *data;
		int must_free_mem;

		while (esc) {
			if (curenv->running == RUNNING) {
				if (curenv->escallowed)
					exec_temphalt("Escape from INPUT");

				my_printf(MSG_DIALOG, 1,
					  "Please re-INPUT from start");
				work = lvalroot;
				esc = input_line(line, p);
				i = line;
			} else
				run_error(ESCAPE_ERR, "Escape");
		}

		type = work->exp->e.expid.id->type;

		while (true) {
			while (*i && isspace_l(*i, latin_loc))
				i++;

			if (*i)
				break;
			else {
				esc = input_line(line, "?? ");
				i = line;
			}
		}

		DBG_PRINTF(1,
			  "Assessing \"%s\" for input type %d", i,
			  type);

		n = 0;
		data = field;
		must_free_mem = 0;

		switch (type) {
		case V_INT:
			nr = sscanf(i, "%ld%n", (long *) field, &n);
			i += n;
			break;

		case V_FLOAT:
			nr = sscanf(i, "%lf%n", (double *) field, &n);
			i += n;
			break;

		case V_STRING:
			j = i;
			n = -1;

			quote = (*j == '"');

			if (quote)
				j++;

			while (*j && !((*j == ',' && !quote)
				       || (*j == '"' && quote))) {
				++n;
				field[n] = *j;
				j++;
			}

			while (isspace_l(field[n], latin_loc)
			       && n >= 0)
				--n;

			field[n + 1] = '\0';
			i += n + 1;

			if (quote)
				i += 2;	/* Past the quotes */

			nr = 1;
			data = str_make(RUN_POOL, field);
			must_free_mem = 1;
			break;

		default:
			fatal("In input_con; default switch reached");
			break;
		}

		if (nr != 1)
			run_error(INPUT_ERR, "Input error @ %s", i);

		while (*i && isspace_l(*i, latin_loc))
			i++;

		if (*i == ',')
			i++;

		do_assign2(work->exp, data, type, must_free_mem);
		work = work->next;
	}

	if (*i)
		my_printf(MSG_DIALOG, 1, "Extra input ignored");
}


PRIVATE void exec_input(struct comal_line *line)
{
	struct input_rec *i = &line->lc.inputrec;

	if (!i->modifier)
		input_con(NULL, i->lvalroot);
	else if (i->modifier->type == fileSYM)
		input_file(&i->modifier->data.twoexp, i->lvalroot);
	else
		input_con(i->modifier->data.str, i->lvalroot);
}


PRIVATE void exec_run(struct comal_line *line)
{
	extern char *runfilename;
	struct string *result;
	enum VAL_TYPE type;

	if (curenv->changed) {
		puts_line(MSG_DIALOG, curenv->curline);

		if (!sys_yn
		    (MSG_DIALOG,
		     "Current environment contains unsaved changes! RUN anyway? "))
			run_error(RUN_ERR, "RUN aborted");
	}

	calc_exp(line->lc.exp, (void **) &result, &type);
	runfilename = my_strdup(MISC_POOL, result->s);
	mem_free(result);

	DBG_PRINTF(1, "About to go RUNning: %s",
		  runfilename);

	longjmp(RESTART, RUN);
}


PRIVATE int exec_return(struct comal_line *line)
{
	void *result;
	enum VAL_TYPE type;

	if (curenv->running != RUNNING)
		run_error(DIRECT_ERR, "Can't RETURN in command mode");

	if (line->lc.exp) {
		calc_exp(line->lc.exp, &result, &type);
		return_type = curenv->curenv->curproc->lc.pfrec.id->type;

		/* return <string> from numeric func checked in SCAN */

		if (return_type == type)
			return_result = result;
		else {
			if (return_type == V_INT) {
				return_result = cell_alloc(INT_CPOOL);
				*(long *) return_result =
				    d2int(*(double *) result, 1);
			} else {
				return_result = cell_alloc(FLOAT_CPOOL);
				*(double *) return_result =
				    *(long *) result;
			}

			cell_free(result);
		}
	}

	return returnSYM;
}



PUBLIC int exec_line(struct comal_line *line)
{
	enum VAL_TYPE type;
	struct string *result;

	if (sys_escape() && curenv->escallowed)
		exec_temphalt("Escape");

	if (!line)
		exec_end();

	if (curenv->trace && curenv->running == RUNNING) {
		puts_line(MSG_TRACE, curenv->curline);
		trace_trace();
	}

	switch (line->cmd) {
	case 0:
	case idSYM:
	case dataSYM:
	case endcaseSYM:
	case endifSYM:
	case nullSYM:
	case useSYM:
	case exportSYM:
		break;

	case repeatSYM:
		if (line->lc.ifwhilerec.exp)
			exec_repeat(line);
		
		break;

	case untilSYM:
		if (!calc_logexp(line->lc.exp))
			curenv->curline = line->lineptr;

		break;

	case loopSYM:
		return exec_loop(line);

	case osSYM:
		calc_exp(line->lc.exp, (void **) &result, &type);

		if (sys_system(result->s) == -1)
			run_error(OS_ERR, "OS command failed");

		mem_free(result);
		break;

	case delSYM:
		calc_exp(line->lc.exp, (void **) &result, &type);

		if (unlink(result->s) == -1 && errno != ENOENT) {
			run_error(DEL_ERR,
				  "DELete of %s failed (Read Only?)",
				  result->s);
		}

		mem_free(result);
		break;

	case chdirSYM:
		calc_exp(line->lc.exp, (void **) &result, &type);
		sys_chdir(result->s);
		mem_free(result);
		break;

	case mkdirSYM:
		calc_exp(line->lc.exp, (void **) &result, &type);
		sys_mkdir(result->s);
		mem_free(result);
		break;

	case rmdirSYM:
		calc_exp(line->lc.exp, (void **) &result, &type);
		sys_rmdir(result->s);
		mem_free(result);
		break;

	case unitSYM:
		calc_exp(line->lc.exp, (void **) &result, &type);
		sys_unit(result->s);
		mem_free(result);
		break;

	case dirSYM:
		if (line->lc.exp) {
			calc_exp(line->lc.exp, (void **) &result, &type);
			sys_dir(result->s);
			mem_free(result);
		} else
			sys_dir("");

		break;

	case staticSYM:
	case localSYM:
	case dimSYM:
		exec_dim(line);
		break;

	case printSYM:
		exec_print(line);
		break;

	case pageSYM:
		sys_page(sel_outfile);
		break;

	case cursorSYM:
		sys_cursor(sel_outfile, calc_intexp(line->lc.twoexp.exp1),
			   calc_intexp(line->lc.twoexp.exp2));
		break;

	case elseSYM:
	case endwhileSYM:
	case otherwiseSYM:
		curenv->curline = line->lineptr;
		break;

	case endloopSYM:
		return endloopSYM;

	case ifSYM:
		return exec_if(line);

	case elifSYM:
		run_chain(line, endifSYM);
		break;

	case whenSYM:
		run_chain(line, endcaseSYM);
		break;

	case restoreSYM:
		exec_restore(line);
		break;

	case becomesSYM:
		exec_assign(line);
		break;

	case sysSYM:
		return sys_sys_stat(line->lc.exproot);

	case runSYM:
		exec_run(line);
		break;

	case trapSYM:
		return exec_trap(line);

	case forSYM:
		return exec_for(line);

	case execSYM:
		do_call(line->lc.exp, procSYM);
		break;

	case returnSYM:
		return exec_return(line);

	case caseSYM:
		exec_case(line);
		break;

	case openSYM:
		exec_open(line);
		break;

	case closeSYM:
		exec_close(line);
		break;

	case importSYM:
		exec_import(line);
		break;

	case select_outputSYM:
		exec_selfile(&sel_outfile, line->lc.exp, "at");
		break;

	case select_inputSYM:
		exec_selfile(&sel_infile, line->lc.exp, "rt");
		break;

	case inputSYM:
		exec_input(line);
		break;

	case writeSYM:
		exec_write(line);
		break;

	case readSYM:
		exec_read(line);
		break;

	case whileSYM:
		return exec_while(line);

	case endSYM:
		exec_end();
		break;

	case stopSYM:
		exec_stop(line);
		break;

	case endforSYM:
	case endtrapSYM:
	case handlerSYM:
		return line->cmd;

	case endmoduleSYM:
	case endprocSYM:
		return returnSYM;

	case endfuncSYM:
		run_error(NORETURN_ERR, "Should have RETURNed by now");

	case moduleSYM:
	case funcSYM:
	case procSYM:
		if (!line->lc.pfrec.external)
			curenv->curline = line->lineptr->ld->next;

		break;

	case traceSYM:
		curenv->trace = (strcmp(exp_cmd(line->lc.exp), "on") == 0);
		break;

	case exitSYM:
		return exec_exit(line);

	case retrySYM:
		return retrySYM;
	
	case randomizeSYM:
		if (line->lc.exp) {
			sys_randomize(calc_intexp(line->lc.exp));
		} else {
			sys_randomize(time(NULL));
		}

		break;
	
	case reportSYM:
		run_error(calc_intexp(line->lc.exp), "");
		break;

	default:
		fatal("exec_line default action");
	}

	return 0;
}


PRIVATE int exec_seq3()
{
	int retcode = 0;

	while (!retcode) {
		struct comal_line *eline;
		
		eline = curenv->curline;
		retcode = exec_line(eline);

		if (eline == curenv->curline && !retcode)
			curenv->curline = curenv->curline->ld->next;
	}

	return retcode;
}


PRIVATE int exec_seq2()
{
	jmp_buf save_err;
	static int ret;

	ret = 0;

	memcpy(save_err, ERRBUF, sizeof(jmp_buf));

	setjmp(ERRBUF);

	while (ret == 0) {
		if (curenv->error) {
			if (curenv->nrtraps)
				longjmp(save_err, 666);

			give_run_err(curenv->errline);

			DBG_PRINTF(1,
				  "Starting secondary COMAL loop");

			comal_loop(HALTED);
		}

		ret = exec_seq3();
	}

	memcpy(ERRBUF, save_err, sizeof(jmp_buf));

	return ret;
}


PUBLIC void exec_seq(struct comal_line *line)
{
	curenv->curline = line;
	curenv->running = RUNNING;

	exec_seq2();
}

PUBLIC void exec_mod_init(struct comal_line *line)
{
	struct sym_env *env;
	struct sym_env *oldenv=curenv->curenv;

	DBG_PRINTF(1,"Initialising module %s",
		line->lc.pfrec.id->name);
	env =
	    sym_newenv(line->lc.pfrec.closed, NULL, NULL, line, line->lc.pfrec.id->name);

	curenv->curenv = env;
	exec_seq(line->ld->next);
	line->lc.pfrec.staticenv=env;
	curenv->curenv = oldenv;
}
