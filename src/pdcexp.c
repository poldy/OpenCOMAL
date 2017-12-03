/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* Routines to calculate the results of expressions */

#include "pdcglob.h"
#include "pdcerr.h"
#include "pdcmisc.h"
#include "pdcid.h"
#include "pdcexp.h"
#include "pdcexec.h"
#include "pdcsym.h"
#include "pdcstr.h"
#include "pdcval.h"


#include <math.h>
#include <string.h>
#include <stdbool.h>

#ifdef HAS_ROUND
extern double round(double x);
#define ROUND round
#else
#define ROUND my_round
#endif

PRIVATE struct string e_s = { 0L, {'\0'} };
PRIVATE struct string *empty_string = &e_s;

PUBLIC int short_circuit = 0;


PUBLIC void *exp_lval(struct expression *exp, enum VAL_TYPE *type,
		      struct var_item **varp, long *strlen)
{
	struct id_rec *id;
	struct exp_list *exproot;
	struct sym_item *sym;
	int nr;
	const char *err = NULL;
	long index = 0;
	long l;
	struct exp_list *walke;
	struct arr_dim *walkd;
	union var_data HUGE_POINTER *vdata;
	void HUGE_POINTER *lval = NULL;

	if (exp->optype == T_EXP_IS_NUM || exp->optype == T_EXP_IS_STRING)
		exp = exp->e.exp;

	if (exp->optype != T_ID && exp->optype != T_SID && exp->optype != T_ARRAY && exp->optype != T_SARRAY)
		fatal("Exp_lval() internal error #1");

	id = exp->e.expid.id;
	exproot = exp->e.expid.exproot;
	sym = sym_search(curenv->curenv, id, S_VAR);

	if (!sym)
		return NULL;

	*varp = sym->data.var;
	*type = (*varp)->type;

	if (strlen)
		*strlen = (*varp)->strlen;

	if (!(*varp)->array && exproot)
		err = "Unexpected array indices";
	else if (!exproot) {
		lval=var_data(*varp);

		if ((*varp)->array) 
			*type=V_ARRAY;
	} else {
		nr = nr_items((struct my_list *) exproot);

		if (nr > (*varp)->array->nrdims)
			err = "Too many indices provided";
		else if (nr < (*varp)->array->nrdims)
			err = "Too few indices provided";
		else {
			walke = exproot;
			walkd = (*varp)->array->dimroot;

			while (walke && !curenv->error && !err) {
				l = calc_intexp(walke->exp);

				if (l < walkd->bottom)
					err =
					    "Index below DIMensioned bottom";
				else if (l > walkd->top)
					err =
					    "Index above DIMensioned top";
				else
					index =
					    index * (walkd->top -
						     walkd->bottom + 1) +
					    l - walkd->bottom;

				walke = walke->next;
				walkd = walkd->next;
			}

			if (err)
				run_error(ARRAY_ERR, err);

			DBG_PRINTF(1, "Array index=%ld",
				  index);

			vdata=(union var_data *)var_data(*varp);
			lval =
			    (char HUGE_POINTER *) vdata +
			    index * type_size(*type);
		}
	}

	if (err)
		run_error(ARRAY_ERR, err);

	DBG_PRINTF(1, "Exp_lval returns %p for %s", lval,
		  id->name);

	return lval;
}


PRIVATE void exp_const(struct expression *exp, void **result,
		       enum VAL_TYPE *type)
{
	switch (exp->op) {
	case _ERR:
		*result = val_int(curenv->lasterr, NULL, type);
		break;


	case _ERRLINE:
		*result = val_int(curenv->lasterrline, NULL, type);
		break;

	case _ERRTEXT:
		*result = str_make(RUN_POOL, curenv->lasterrmsg);
		*type = V_STRING;
		break;

	case _DIR:
		*result = str_make(RUN_POOL, sys_dir_string());
		*type = V_STRING;
		break;

	case _UNIT:
		*result = str_make(RUN_POOL, sys_unit_string());
		*type = V_STRING;
		break;

	case _KEY:
		*result=str_make(RUN_POOL, sys_key(0));
		*type = V_STRING;
		break;

	case _EOD:
		*result = val_int(!curenv->dataeptr, NULL, type);
		break;

	case _PI:
		*result = val_float(M_PI, NULL, type);
		break;

	case _FALSE:
		*result = val_int(false, NULL, type);
		break;

	case _TRUE:
		*result = val_int(true, NULL, type);
		break;
	
	case _CURCOL:
		*result = val_int(sys_curcol(), NULL, type);
		break;
	
	case _CURROW:
		*result = val_int(sys_currow(), NULL, type);
		break;
	
	case _ESC:
		*result = val_int(sys_escape(), NULL, type);
		break;

	default:
		fatal("exp_const default action");
	}
}


PRIVATE void *my_not(void **result, enum VAL_TYPE *type)
{
	long *l;

	if (*type == V_INT) {
		**(long **) result = !(**(long **) result);

		return *result;
	}

	l = (long int *)cell_alloc(INT_CPOOL);

	*l = ((**(double **) result) == 0);
	*type = V_INT;
	cell_free(*result);

	return l;
}


PRIVATE long my_eof2(long fno)
{
	struct file_rec *f = fsearch(fno);
	long result;
	extern int eof(int file);

	if (!f)
		run_error(EOF_ERR, "File not open");

	result = eof(f->hfno);

	if (result == -1)
		run_error(EOF_ERR, "Error when checking for EOF: %s",
			  strerror(errno));

	return result;
}


PRIVATE void *my_eof(void **result, enum VAL_TYPE *type)
{
	long *l;

	if (*type == V_INT) {
		**(long **) result = my_eof2(**(long **) result);

		return *result;
	}

	l = (long int *)cell_alloc(INT_CPOOL);

	*l = d2int(**(double **) result, 1);
	*l = my_eof2(*l);
	*type = V_INT;
	cell_free(*result);

	return l;
}


PRIVATE void *my_sgn(void **result, enum VAL_TYPE *type)
{
	long *l;

	if (*type == V_INT) {
		if (**(long **) result < 0)
			**(long **) result = -1;
		else if (**(long **) result > 0)
			**(long **) result = 1;
		else
			**(long **) result = 0;

		return *result;
	}

	l = (long int *)cell_alloc(INT_CPOOL);

	if (**(double **) result < 0)
		*l = -1;
	else if (**(double **) result > 0)
		*l = 1;
	else
		*l = 0;

	*type = V_INT;
	cell_free(*result);

	return l;
}


PRIVATE double my_rad(double x)
{
	return (x * M_PI) / 180;
}


PRIVATE double my_deg(double x)
{
	return (x * 180) / M_PI;
}

PRIVATE double my_int(double x)
{
	return floor(x);
}


PRIVATE double *my_val(struct string **result, enum VAL_TYPE *type)
{
	double *d = (double *)cell_alloc(FLOAT_CPOOL);
	char *endptr;

	*d = strtod((*result)->s, &endptr);

	if (*endptr)
		run_error(VAL_ERR,
			  "Conversion error when taking string VALue");

	mem_free(*result);
	*type = V_FLOAT;

	return d;
}


PRIVATE struct string *my_chr(void **result, enum VAL_TYPE *type)
{
	struct string *s = STR_ALLOC(RUN_POOL, 1);
	long num;

	if (*type == V_INT)
		num = **(long **) result;
	else
		num = d2int(**(double **) result, 1);

	if (num < 0 || num > 255)
		run_error(CHR_ERR, "Illegal value for CHR$ (0<=x<=255)");

	s->s[0] = (char) num;
	s->s[1] = '\0';
	s->len = 1;
	*type = V_STRING;
	cell_free(*result);

	return s;
}

PRIVATE struct string *my_spc(void **result, enum VAL_TYPE *type)
{
	struct string *s;
	long num;

	if (*type == V_INT)
		num = **(long **) result;
	else
		num = d2int(**(double **) result, 1);

	if (num < 0 )
		run_error(SPC_ERR, "Illegal parameter for SPC$ (<0)");

	s = STR_ALLOC(RUN_POOL, num);
	s->len=num;
	memset(s->s,' ',num);
	s->s[num]=0;
	*type = V_STRING;
	cell_free(*result);

	return s;
}


PRIVATE struct string *my_str(void **result, enum VAL_TYPE *type)
{
	char buf[30];

	if (*type == V_INT) {
		snprintf(buf, 30, "%ld", **(long **) result);
	} else {
		snprintf(buf, 30, "%G", **(double **) result);
	}

	*type = V_STRING;
	cell_free(*result);

	return str_make(RUN_POOL, buf);
}


PRIVATE void exp_unary(struct expression *expr, void **result,
		       enum VAL_TYPE *type)
{
	double (*dfunc) (double x) = NULL;
	void *(*mfunc) (void **result, enum VAL_TYPE *type) = NULL;
	char *s;

	/*
	 * This selection is necessary because INKEY has an optional
	 * argument...
	 */
	if (expr->e.exp)
		calc_exp(expr->e.exp, result, type);
	else {
		*result=NULL;
		*type=V_ERROR;
	}

	switch (expr->op) {
	case lparenSYM:
	case plusSYM:
		break;

	case minusSYM:
		val_neg(*result, *type);
		break;

	case _VAL:
		*result = my_val((struct string **)result, type);
		break;

	case _INKEY:
		if (!*result)
			s=sys_key(-1);
		else {
			long delay;

			if (*type==V_FLOAT)
				delay=(long)**(double **)result;
			else
				delay=(long)**(long **)result;

			cell_free(*result);
			s=sys_key(delay);
		}

		if (*s) my_printf(MSG_DIALOG,0,s);

		*result=str_make(RUN_POOL,s);
		*type=V_STRING;

		break;

	case _ORD:
		*result =
		    val_int((unsigned char)((*(struct string **) result)->s[0]), *result,
			    type);
		break;

	case _LOWER:
		strlwr((*(struct string **)result)->s);
		break;

	case _UPPER:
		strupr((*(struct string **)result)->s);
		break;

	case _LEN:
		*result =
		    val_int((*(struct string **) result)->len, *result,
			    type);
		break;

	case _CHR:
		*result = my_chr(result, type);
		break;

	case _SPC:
		*result=my_spc(result,type);
		break;

	case _STR:
		*result = my_str(result, type);
		break;

	case _SGN:
		mfunc = my_sgn;
		break;
	case _NOT:
		mfunc = my_not;
		break;
	case _EOF:
		mfunc = my_eof;
		break;

	case _RAD:
		dfunc = my_rad;
		break;
	case _DEG:
		dfunc = my_deg;
		break;
	case _FRAC:
		dfunc = my_frac;
		break;
	case _ROUND:
		dfunc = ROUND;
		break;
	case _ABS:
		dfunc = fabs;
		break;
	case _ACS:
		dfunc = acos;
		break;
	case _ASN:
		dfunc = asin;
		break;
	case _ATN:
		dfunc = atan;
		break;
	case _COS:
		dfunc = cos;
		break;
	case _EXP:
		dfunc = exp;
		break;
	case _INT:
		dfunc = my_int;
		break;
	case _LOG:
		dfunc = log;
		break;
	case _LOG10:
		dfunc = log10;
		break;
	case _SIN:
		dfunc = sin;
		break;
	case _SQR:
		dfunc = sqrt;
		break;
	case _TAN:
		dfunc = tan;
		break;

	default:
		fatal("calc_exp unary default action");
	}

	if (dfunc) {
		if (*type == V_INT) {
			double *d = (double *)cell_alloc(FLOAT_CPOOL);

			*d = **(long **) result;
			cell_free(*result);
			*result = d;
			*type = V_FLOAT;
		}

		**(double **) result = (*dfunc) (**(double **) result);
	} else if (mfunc)
		*result = (*mfunc) (result, type);
}


PRIVATE int relop(int op)
{
	return (op == eqlSYM || op == neqSYM || op == lssSYM
		|| op == gtrSYM || op == leqSYM || op == geqSYM);
}


PRIVATE int logop(int op)
{
	return (op == andSYM || op == orSYM || op == eorSYM ||
		op == andthenSYM || op == orthenSYM);
}


PRIVATE void check0(int zero)
{
	if (!zero)
		return;

	run_error(DIV0_ERR, "Division by Zero (0)");
}


PRIVATE void exp_binary_s(int op, void **result, enum VAL_TYPE *type,
			  struct string HUGE_POINTER * s1,
			  void *result2, enum VAL_TYPE type2)
			  
{
	struct string HUGE_POINTER *s2;
	long n;
	char HUGE_POINTER *t;

	if (op == plusSYM) {
		s2=(struct string HUGE_POINTER *)result2;
		s1 = STR_REALLOC(s1, s1->len + s2->len);
		*result = str_cat(s1, s2);
		*type = V_STRING;
		mem_free(s2);
	} else if (op == inSYM) {
		s2=(struct string HUGE_POINTER *)result2;
		*result = val_int(str_search(s1, s2), NULL, type);
		mem_free(s1);
		mem_free(s2);
	} else if (op == timesSYM) {
		n=val_mustbelong(result2,type2,1);
		s2=str_make2(RUN_POOL,n);

		for (t=s2->s; n; n--, t+=s1->len) {
			strncpy(t,s1->s,s1->len);
			t[s1->len] = '\0';
		}

		*result=s2;
		*type=V_STRING;
	} else
		fatal("exp_binary_s illegal non-relop");
}


PRIVATE void exp_binary_i(int op, void **result, enum VAL_TYPE *type,
			  void *v1, void *v2)
{
        long *i1, *i2, res;

        i1 = (long *)v1;
        i2 = (long *)v2;
	*result = NULL;

	switch (op) {
	case powerSYM:
		*result =
		    val_float(pow((double) *i1, (double) *i2), i1, type);
		break;

	case divideSYM:
		val_intdiv(i1, i2, result, type);
		break;
	case plusSYM:
		val_intadd(i1, i2, result, type);
		break;
	case minusSYM:
		val_intsub(i1, i2, result, type);
		break;
	case timesSYM:
		val_intmul(i1, i2, result, type);
		break;

	case divSYM:
		check0(*i2 == 0);
		res = *i1 / *i2;
		if ((*i1 % *i2) != 0 && (*i1 ^ *i2) < 0) {
			res--;
		}
		*i1 = res;
		break;
	case modSYM:
		check0(*i2 == 0);
		res = *i1 % *i2;
		if (res != 0 && (*i1 ^ *i2) < 0) {
			res += *i2;
		}
		*i1 = res;
		break;
	
	case bitandSYM:
		*i1 = *i1 & *i2;
		break;
	case bitorSYM:
		*i1 = *i1 | *i2;
		break;
	case bitxorSYM:
		*i1 = *i1 ^ *i2;
		break;

	default:
		fatal("exp_binary_i non-relop switch default action");
	}

	if (*result == NULL) {
		*type = V_INT;
		cell_free(i2);
		*result = i1;
	}
}


PRIVATE void exp_binary_f(int op, void **result, enum VAL_TYPE *type,
			  void *v1, void *v2)
{
        double *f1, *f2;
	long i1, i2;

        f1 = (double *)v1;
        f2 = (double *)v2;
	switch (op) {
	case powerSYM:
		*f1 = pow(*f1, *f2);
		break;
	case plusSYM:
		*f1 = *f1 + *f2;
		break;
	case minusSYM:
		*f1 = *f1 - *f2;
		break;
	case timesSYM:
		*f1 = *f1 ** f2;
		break;

	case divideSYM:
		check0(*f2 == 0);
		*f1 = *f1 / *f2;
		break;

	case modSYM:
		check0(*f2 == 0);
		*f1 = fmod(*f1, *f2);
		break;

	case divSYM:
		check0(*f2 == 0);
		*f1 = floor(*f1 / *f2);
		break;

	case bitandSYM:
		i1 = d2int(*f1, 1);
		i2 = d2int(*f2, 1);
		*f1 = i1 & i2;
		break;
	case bitorSYM:
		i1 = d2int(*f1, 1);
		i2 = d2int(*f2, 1);
		*f1 = i1 | i2;
		break;
	case bitxorSYM:
		i1 = d2int(*f1, 1);
		i2 = d2int(*f2, 1);
		*f1 = i1 ^ i2;
		break;

	default:
		fatal("exp_binary_f non-relop switch default action");
	}

	*type = V_FLOAT;
	cell_free(f2);
	*result = f1;
}


PRIVATE int logval(void *value, enum VAL_TYPE type)
{
	if (type == V_INT)
		return *(long *) value != 0;
	else if (type == V_FLOAT)
		return *(double *) value != 0;
	else
		fatal("Logval of non-num type");

	/* NOTREACHED */
	return 0;
}


PRIVATE int exp_binary_l(int op, struct expression *exp1,
			 struct expression *exp2)
{
	void *result;
	enum VAL_TYPE type;
	int i1, i2;

	calc_exp(exp1, &result, &type);
	i1 = logval(result, type);
	cell_free(result);

	if (op == andthenSYM || (op == andSYM && short_circuit)) {
		if (!i1)
			return 0;
		else
			op = andSYM;
	} else {
		if (op == orthenSYM || (op == orSYM && short_circuit)) {
			if (i1)
				return 1;
			else
				op = orSYM;
		}
	}

	calc_exp(exp2, &result, &type);
	i2 = logval(result, type);
	cell_free(result);

	switch (op) {
	case andSYM:
		return i1 && i2;
	case orSYM:
		return i1 || i2;
	case eorSYM:
		return (i1 && !i2) || (i2 && !i1);

	default:
		fatal("exp_binary_l relop switch default action");
	}

	/* NOTREACHED */
	return 0;
}

/*
 * Calculate a new RND value. sys_rand is called to determine a new 
 * random number from the system and the scale in which this random
 * number has been generated (typically RAND_MAX). The exact working
 * of this function then depends on the presence of exp1 and exp2 in
 * this T_BINARY node of the expression tree:
 *
 * RND -> exp1==exp2==NULL -> gives random number in [0,1]
 * RND(x) -> exp1==NULL, exp2==x -> gives whole random number in [0,x]
 * RND(x,y) -> exp1==x, exp2==y -> gives whole random number in [x,y]
 */
PRIVATE void exp_rnd(struct expression *exp, double **result,
			enum VAL_TYPE *type)
{
	long n,max;
	double d,d1,d2;
	struct expression *exp1=exp->e.twoexp.exp1;
	struct expression *exp2=exp->e.twoexp.exp2;

	sys_rand(&n,&max);
	d=(double)n/(double)max;

	if (exp1==NULL && exp2==NULL)
		; /* Do nothing, d has the right value */
	else if (exp1==NULL && exp2!=NULL) {
		d2=floor(val_double(exp2));
		d=ROUND(d*d2);
	} else if (exp1!=NULL && exp2!=NULL) {
		d1=floor(val_double(exp1));
		d2=floor(val_double(exp2));

		if (d1>=d2) run_error(RND_ERR,"RND(x,y) argument error (x>=y)");

		d=ROUND((d2-d1)*d+d1);
	} else
		fatal("exp_rnd internal error #1");

	*result = (double *)cell_alloc(FLOAT_CPOOL);
	**result = d;
	*type = V_FLOAT;
}

PRIVATE void exp_binary(struct expression *exp, void **result,
			enum VAL_TYPE *type)
{
	void *result1, *result2;
	enum VAL_TYPE type1, type2;
	void (*func) (int op, void **result, enum VAL_TYPE *type, void *v1, void *v2) = NULL;
	int cmp;

	if (logop(exp->op))
		*result =
		    val_int(exp_binary_l
			    (exp->op, exp->e.twoexp.exp1,
			     exp->e.twoexp.exp2), NULL, type);
	else if (exp->op==_RND)
		exp_rnd(exp,(double **)result,type);
	else {
		calc_exp(exp->e.twoexp.exp1, &result1, &type1);
		calc_exp(exp->e.twoexp.exp2, &result2, &type2);

		if (relop(exp->op)) {
			cmp =
			    val_cmp(exp->op, result1, result2, type1,
				    type2);
			val_free(result1, type1);
			val_free(result2, type2);
			*result = val_int(cmp, NULL, type);
		} else if (type1==V_STRING)
			exp_binary_s(exp->op, result, type, (struct string *)result1, result2, type2);
		else {
			if (type1 != type2) {
				if (type1 == V_INT)
					result1 =
					    val_float(*(long *) result1,
						      result1, &type1);
				else
					result2 =
					    val_float(*(long *) result2,
						      result2, &type2);
			}

			switch (type1) {
			case V_INT:
				func = exp_binary_i;
				break;
			case V_FLOAT:
				func = exp_binary_f;
				break;

			default:
				fatal
				    ("exp_binary subexp type default action");
			}

			(*func) (exp->op, result, type, result1, result2);
		}
	}
}


PRIVATE void exp_intnum(struct expression *exp, void **vresult,
			enum VAL_TYPE *type)
{
        long *result;

	result = (long int *)cell_alloc(INT_CPOOL);
	*result = exp->e.num;
	*type = V_INT;
        *vresult = result;
}


PRIVATE void exp_float(struct expression *exp, void **vresult,
		       enum VAL_TYPE *type)
{
        double *result;

	result = (double *)cell_alloc(FLOAT_CPOOL);
	*result = exp->e.fnum.val;
	*type = V_FLOAT;
        *vresult = result;
}


PRIVATE void exp_string(struct expression *exp, void **vresult,
			enum VAL_TYPE *type)
{
        struct string *result;

	result = str_dup(RUN_POOL, exp->e.str);
	*type = V_STRING;
        *vresult = result;
}


PRIVATE int exp_name(struct id_rec *id, struct exp_list *exproot,
		     void **result, enum VAL_TYPE *type)
{
	struct sym_item *sym = sym_search(curenv->curenv, id, S_NAME);
	struct sym_env *save_env;
	enum VAL_TYPE ntype;

	if (!sym)
		return 0;

	if (exproot)
		run_error(NAME_ERR,
			  "NAMEs can't take parameters or dimensions");

	save_env = curenv->curenv;
	curenv->curenv = sym->data.name->env;
	calc_exp(sym->data.name->exp, result, type);
	curenv->curenv = save_env;

	ntype = id->type;

	if (ntype != *type) {
		if (ntype == V_INT)
			*result =
			    val_int(d2int(**(double **) result, 0),
				    *result, type);
		else
			*result =
			    val_float((double) **(long **) result, *result,
				      type);
	}

	return 1;
}


PRIVATE void exp_id(struct expression *exp, void **result,
		    enum VAL_TYPE *type)
{
	struct var_item *var;
	void *lval = exp_lval(exp, type, &var, NULL);

	if (!lval) {
		if (!exp_name
		    (exp->e.expid.id, exp->e.expid.exproot, result, type))
			exec_call(exp, funcSYM, result, type);

		return;
	}

	if (*type==V_ARRAY)
		run_error(ARRAY_ERR, "Missing array indices for %s",exp->e.expid.id->name);
	else if (*type == V_INT) {
		*result = cell_alloc(INT_CPOOL);
		**(long **) result = *(long *) lval;
	} else {
		*result = cell_alloc(FLOAT_CPOOL);
		**(double **) result = *(double *) lval;
	}
}

PRIVATE void exp_array(struct expression *exp, void **result,
		    enum VAL_TYPE *type)
{
	struct var_item *var;
	void *lval = exp_lval(exp, type, &var, NULL);

	/*
	 * The expression is not an lvalue (array). Maybe it's wrong
	 * syntax for a function call? Let's try to run it as a function
	 * instead...
	 */
	if (!lval) {
		exec_call(exp,funcSYM,result,type);
		return;
	}

	/*
	 * If we got here, the lval should be an array. This can also be
	 * checked by if (var->array) ...
	 */
	if (*type!=V_ARRAY)
		run_error(ARRAY_ERR, "%s is not an array",exp->e.expid.id->name);

	/*
	 * For arrays, the result pointer points to the var item. Since the parse
	 * tree of an expression containing an array() can not contain anything
	 * else, we're fine...
	 */
	*result=var;
}

PRIVATE void do_substr(struct string *tresult, struct string **result,
		       struct two_exp *twoexp)
{
	long from, to;
	const char *err = NULL;

	if (twoexp->exp1)
		from = calc_intexp(twoexp->exp1);
	else
		from = 1;

	if (twoexp->exp2)
		to = calc_intexp(twoexp->exp2);
	else
		to = tresult->len;

	if (from > to)
		err = "Substring specifier incorrect (from>to)";
	else if (from < 1)
		err = "Substring specifier start < 1";
	else if (to > tresult->len)
		err = "Substring specifier end > string length";

	if (err)
		run_error(SUBSTR_ERR, err);

	*result = STR_ALLOC(RUN_POOL, to - from + 1);
	str_partcpy(*result, tresult, from, to);
}


PRIVATE void exp_sid(struct expression *exp, void **vresult,
		     enum VAL_TYPE *type)
{
	struct var_item *var;
	long strl;
	struct string *tresult = NULL;
	struct string **lval = (struct string **)exp_lval(exp, type, &var, &strl);
        struct string *result = NULL;

	if (!lval) {
		if (!exp_name
		    (exp->e.expsid.id, exp->e.expsid.exproot, (void **)&tresult,
		     type))
			exec_call(exp, funcSYM, (void **) &tresult, type);

		lval = &tresult;
	} else if (*type==V_ARRAY)
		run_error(ARRAY_ERR, "Missing string array indices on %s",exp->e.expsid.id->name);

	if (!*lval)
		lval = &empty_string;

	if (exp->e.expsid.twoexp)
		do_substr(*lval, &result, exp->e.expsid.twoexp);
	else if (tresult) {
		result = tresult;
		tresult = NULL;
	} else
		result = str_dup(RUN_POOL, *lval);

	if (tresult)
		mem_free(tresult);
        *vresult = result;
}


PRIVATE void exp_substr(struct expression *exp, void **result, enum
			VAL_TYPE *type)
{
	void *tresult;

	calc_exp(exp->e.expsubstr.exp, &tresult, type);
	do_substr((struct string *)tresult, (struct string **)result, &exp->e.expsubstr.twoexp);
	mem_free(tresult);
}


PRIVATE void exp_sys(struct expression *exp, void **result, enum
		     VAL_TYPE *type)
{
	sys_sys_exp(exp->e.exproot, result, type);
}


PRIVATE void exp_syss(struct expression *exp, void **vresult,
		      enum VAL_TYPE *type)
{
        struct string *result = NULL;

	sys_syss_exp(exp->e.exproot, &result, type);
        *vresult = result;
}


PRIVATE void exp_reexp(struct expression *exp, void **result, enum
		       VAL_TYPE *type)
{
	calc_exp(exp->e.exp, result, type);
}



PRIVATE struct {
	enum optype type;
	void (*func) (struct expression *exp, void **result, enum VAL_TYPE *type);
} exptab[] = {
	{
	T_EXP_IS_NUM, exp_reexp}, {
	T_EXP_IS_STRING, exp_reexp}, {
	T_ID, exp_id}, {
	T_ARRAY, exp_array}, {
	T_SID, exp_sid}, {
	T_SARRAY, exp_array}, {
	T_INTNUM, exp_intnum}, {
	T_FLOAT, exp_float}, {
	T_STRING, exp_string}, {
	T_CONST, exp_const}, {
	T_BINARY, exp_binary}, {
	T_UNARY, exp_unary}, {
	T_SUBSTR, exp_substr}, {
	T_SYS, exp_sys}, {
	T_SYSS, exp_syss}, {
	T_UNUSED, 0}
};


PUBLIC void calc_exp(struct expression *exp, void **result,
		     enum VAL_TYPE *type)
{
	int i;

	if (!exp)
		fatal("Calc_exp finds (null) expression");

	for (i = 0; exptab[i].type && exptab[i].type != exp->optype; i++);

	if (exptab[i].type)
		exptab[i].func(exp, result, type);
	else
		fatal("calc_exp, optype does not occur in table");
}


PUBLIC long calc_intexp(struct expression *exp)
{
	long num = 0;
	void *result;
	enum VAL_TYPE type;

	calc_exp(exp, &result, &type);

	if (type == V_INT)
		num = *(long *) result;
	else if (type == V_FLOAT)
		num = d2int(*(double *) result, 1);
	else
		fatal("calc_intexp wrong type");

	cell_free(result);

	return num;
}


PUBLIC int calc_logexp(struct expression *exp)
{
	void *result;
	enum VAL_TYPE type;
	int log = 0;

	calc_exp(exp, &result, &type);

	if (type == V_INT)
		log = (*(long *) result) != 0;
	else if (type == V_FLOAT)
		log = (*(double *) result) != 0;
	else
		fatal("calc_logexp wrong type");

	cell_free(result);

	return log;
}
