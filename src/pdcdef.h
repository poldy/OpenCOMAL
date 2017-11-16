/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal Command & data structure definitions */

#ifndef PDCDEF_H
#define PDCDEF_H

#include "pdcmem.h"

struct my_list {
	struct my_list *next;
	/* variable part here */
};

struct env_list {
	struct env_list *next;
	struct comal_env *env;
};

struct string {
	long len;
	char s[1];
};

static inline struct string *
STR_ALLOC(unsigned int p, long x)
{
	return (struct string *)mem_alloc(p, sizeof(struct string) + x);
}
static inline struct string *
STR_ALLOC_PRIVATE(struct mem_pool *p, long x)
{
	return (struct string *)mem_alloc_private(p, sizeof(struct string) + x);
}
static inline struct string *
STR_REALLOC(void *s, long l)
{
	return (struct string *)mem_realloc(s, sizeof(struct string) + l);
}

enum SYM_TYPE { S_ERROR, S_VAR, S_NAME, S_PROCVAR, S_FUNCVAR };
enum VAL_TYPE { V_ERROR, V_INT, V_FLOAT, V_STRING, V_ARRAY };


struct id_rec {
	struct id_rec *left;
	struct id_rec *right;
	enum VAL_TYPE type;
	char name[1];
};

struct id_list {
	struct id_list *next;
	struct id_rec *id;
};

struct name_rec {
	struct sym_env *env;
	struct expression *exp;
};


struct arr_dim {
	struct arr_dim *next;
	long bottom;
	long top;
};


struct arr_des {
	struct arr_dim *dimroot;
	int nrdims;
	long nritems;
};

union var_data {
	long num[1];
	double fnum[1];
	struct string *str[1];
	void *vref;
};

struct var_item {
	enum VAL_TYPE type;
	int ref;
	struct arr_des *array;
	long strlen;
	union var_data data;
};


struct sym_item {
	struct sym_item *next;
	struct id_rec *id;
	enum SYM_TYPE symtype;
	union {
		void *ptr;
		struct comal_line *pfline;
		struct var_item *var;
		struct name_rec *name;
	} data;
};


struct sym_env {
	struct sym_env *prev;
	struct sym_env *aliasenv;
	int closed;
	struct sym_item *itemroot;
	char *name;
	struct comal_line *curproc;
	int level;
};


struct two_exp {
	struct expression *exp1;
	struct expression *exp2;
};

struct exp_id {
	struct id_rec *id;
	struct exp_list *exproot;
};

struct exp_sid {
	struct id_rec *id;
	struct exp_list *exproot;
	struct two_exp *twoexp;
};


struct exp_substr {
	struct expression *exp;
	struct two_exp twoexp;
};

enum optype { T_UNUSED, T_CONST, T_UNARY, T_BINARY, T_INTNUM, T_FLOAT,
	T_SUBSTR, T_STRING, T_ID, T_SID, T_SYS, T_SYSS,
	T_EXP_IS_NUM, T_EXP_IS_STRING, T_ARRAY, T_SARRAY
};

/*
 * Strange though it sounds, we need this struct to encapsulate a
 * double value in the parse tree.
 * "Why?" you might ask. Well, the issue is that values that can
 * be represented with infinite precision in the decimal system (like 
 * for instance 5.3) can not be represented as accurately in most binary
 * representations of floating point. The proof of this is left as an
 * exercise to the reader, but look up the relevant chapters in Donald 
 * Knuth's magnum opus "The Art of Computing" if you want some help.
 * Because of this, if we want to be able to accurately LIST a program
 * we need to keep track of the string that the user entered into the
 * program. The binary value is maintained as well for performing the
 * calculations
 */
struct dubbel {
	double val;
	char *text;
};

union exp_data {
        long num;
        struct dubbel fnum;
        struct string *str;
        struct expression *exp;
        struct two_exp twoexp;
        struct exp_id expid;
        struct exp_sid expsid;
        struct exp_substr expsubstr;
        struct exp_list *exproot;
};

struct expression {
	enum optype optype;
	int op;
	union exp_data e;
};

struct two_num {
	long num1;
	long num2;
};

struct list_cmd {
	struct string *str;
	struct two_num twonum;
	struct id_rec *id;
};

struct exp_list {
	struct exp_list *next;
	struct expression *exp;
};

struct dim_ension {
	struct dim_ension *next;
	struct expression *bottom;
	struct expression *top;
};

struct dim_list {
	struct dim_list *next;
	struct id_rec *id;
	struct dim_ension *dimensionroot;
	struct expression *strlen;
};

struct for_rec {
	struct expression *lval;
	struct expression *from;
	int mode;
	struct expression *to;
	struct expression *step;
	struct comal_line *stat;
};

struct parm_list {
	struct parm_list *next;
	struct id_rec *id;
	int ref;
	int array;
};

struct import_list {
	struct import_list *next;
	struct id_rec *id;
	int array;
};

struct import_rec {
	struct id_rec *id;
	struct import_list *importroot;
};

struct ext_rec {
	int dynamic;
	struct expression *filename;
	struct seg_des *seg;
};

struct proc_func_rec {
	struct id_rec *id;
	int closed;
	struct ext_rec *external;
	struct parm_list *parmroot;
	struct sym_env *staticenv;
	int level;
	struct comal_line *proclink;
	struct comal_line *localproc;
	struct comal_line *fatherproc;
	struct seg_des *seg;
};

struct ifwhile_rec {
	struct expression *exp;
	struct comal_line *stat;
};

struct input_modifier {
	int type;
	union {
		struct two_exp twoexp;
		struct string *str;
	} data;
};

struct input_rec {
	struct input_modifier *modifier;
	struct exp_list *lvalroot;
};

struct open_rec {
	struct expression *filenum;
	struct expression *filename;
	int type;
	struct expression *reclen;
	int read_only;
};

struct print_list {
	struct print_list *next;
	int pr_sep;
	struct expression *exp;
};

struct print_modifier {
	int type;
	union {
		struct expression *str;
		struct two_exp twoexp;
	} data;
};

struct print_rec {
	struct print_modifier *modifier;
	struct print_list *printroot;
	int pr_sep;
};

struct read_rec {
	struct two_exp *modifier;
	struct exp_list *lvalroot;
};

struct when_list {
	struct when_list *next;
	int op;
	struct expression *exp;
};

struct write_rec {
	struct two_exp twoexp;
	struct exp_list *exproot;
};

struct trap_rec {
	int esc;
};

struct assign_list {
	struct assign_list *next;
	int op;
	struct expression *lval;
	struct expression *exp;
};

struct comal_line_data {
	struct comal_line *next;
	long lineno;
	int indent;
	struct string *rem;
};

union line_contents {
	struct string *str;
	int inum;
	struct id_list *idroot;
	struct id_rec *id;
	struct two_num twonum;
	struct two_exp twoexp;
	struct list_cmd listrec;
	struct expression *exp;
	struct exp_list *exproot;
	struct dim_list *dimroot;
	struct for_rec forrec;
	struct ifwhile_rec ifwhilerec;
	struct import_rec importrec;
	struct input_rec inputrec;
	struct open_rec openrec;
	struct print_rec printrec;
	struct proc_func_rec pfrec;
	struct trap_rec traprec;
	struct read_rec readrec;
	struct when_list *whenroot;
	struct write_rec writerec;
	struct assign_list *assignroot;
};

struct comal_line {
	struct comal_line_data *ld;
	struct comal_line *lineptr;
	int cmd;
	union line_contents lc;
};


struct file_rec {
	struct file_rec *next;
	long cfno;
	int hfno;
	int mode;
	int read_only;
	long reclen;
};


struct seg_des {
	struct seg_des *prev;
	struct comal_line *extdef;
	struct comal_line *procdef;
	struct comal_line *lineroot;
	struct comal_line *save_localproc;
};

/*
 * Element of the exported function table
 */
struct mod_func_entry {
	struct mod_func_entry	*next;
	struct id_rec		*id;
	struct comal_line	*line;
	struct mod_entry	*module;
};

/*
 * Element of the module table
 */
struct mod_entry {
	struct mod_entry	*next;
        struct id_rec           *id;
        struct comal_line       *line;
	int			being_registered;
	int			has_been_scanned;
	struct seg_des 		*seg;
};


#define RUNNING		1
#define HALTED		2
#define CMDLOOP		3

struct comal_env {
	char *envname;

	struct sym_env *rootenv;
	struct sym_env *curenv;
	struct comal_line *globalproc;

	struct comal_line *progroot;
	struct seg_des *segroot;
	struct comal_line *curline;
	struct comal_line *datalptr;
	struct exp_list *dataeptr;
	struct file_rec *fileroot;
	struct mem_pool *program_pool;

	int running;
	int trace;
	int escallowed;
	int nrtraps;

	char *name;
	int scan_ok;
	int changed;
	int con_inhibited;

	int error;
	struct comal_line *errline;
	char *errmsg;

	int lasterr;
	char *lasterrmsg;
	long lasterrline;
};

#define	FOR_EACH_LINE(seg,var) for (var=seg_root(seg); var; var=var->ld->next)
#endif
