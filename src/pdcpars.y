%{
#define YYDEBUG 1

/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */


/* The OpenComal parser */

#define PDCPARS

#include "pdcglob.h"
#include "pdcparss.h"
#include "pdcmisc.h"
#include "pdcid.h"
#include "pdcprog.h"
#include "msgnrs.h"

#include <string.h>
#include <limits.h>

#define yyunion(x,y)	( (*(x)) = (*(y)) )

PUBLIC struct comal_line c_line;

PRIVATE void p_error(const char *msg);

extern int yylex();

%}

%union
	{
		long num;
		int inum;
		bool flag;
		struct string *str;
		struct id_rec *id;
		struct id_list *id_root;
		struct dubbel dubbel;
		struct expression *exp;
		struct { struct id_rec *id; int array; } oneparm;
		struct two_num twonum;
		struct two_exp twoexp;
		struct two_exp *twoexpp;
		struct comal_line cl;
		struct comal_line *pcl;
		struct exp_list *expptr;
		struct ext_rec *extptr;
		struct dim_list *dimptr;
		struct dim_ension *dimensionptr;
		struct parm_list *parmptr;
		struct print_list *printptr;
		struct import_list *importptr;
		struct input_modifier *imod;
		struct when_list *whenptr;
		struct assign_list *assignptr;
		struct open_rec openrec;

	}

%token	andSYM 
%token	andthenSYM
%token	appendSYM 
%token	atSYM
%token	autoSYM
%token	becomesSYM 
%token	becplusSYM
%token	becminusSYM
%token	bitandSYM
%token	bitorSYM
%token	bitxorSYM
%token	caseSYM 
%token	chdirSYM
%token	closedSYM 
%token	closeSYM 
%token	colonSYM
%token	commaSYM 
%token	contSYM 
%token	createSYM
%token	cursorSYM
%token	dataSYM 
%token	delSYM 
%token	delaySYM
%token	dimSYM
%token  dirSYM
%token	divideSYM 
%token	divSYM 
%token	doSYM 
%token	downtoSYM 
%token	dynamicSYM
%token	editSYM
%token	elifSYM 
%token	elseSYM
%token	endcaseSYM 
%token	endforSYM 
%token	endfuncSYM 
%token	endifSYM 
%token	endloopSYM
%token	endprocSYM 
%token	endSYM 
%token  endmoduleSYM
%token	endtrapSYM 
%token	endwhileSYM 
%token	envSYM 
%token	enterSYM
%token	eolnSYM 
%token	eorSYM 
%token	eqlSYM 
%token	execSYM 
%token	exitSYM
%token	exportSYM
%token	externalSYM
%token	fileSYM
%token	forSYM 
%token	funcSYM 
%token	geqSYM 
%token	getSYM
%token	gtrSYM 
%token	handlerSYM
%token	ifSYM 
%token	importSYM 
%token	inputSYM 
%token	inSYM
%token	leqSYM 
%token	listSYM 
%token	localSYM
%token	loadSYM 
%token	loopSYM 
%token	lparenSYM 
%token	lssSYM 
%token	minusSYM 
%token	mkdirSYM
%token	modSYM 
%token	moduleSYM 
%token	nameSYM
%token	neqSYM 
%token	newSYM
%token	nullSYM 
%token	ofSYM 
%token	openSYM 
%token	orSYM 
%token	orthenSYM
%token	osSYM 
%token	otherwiseSYM 
%token	pageSYM
%token	plusSYM 
%token	powerSYM
%token	printSYM 
%token	procSYM 
%token	quitSYM
%token	randomSYM 
%token	randomizeSYM
%token	readSYM 
%token	read_onlySYM 
%token	refSYM
%token	renumberSYM 
%token	repeatSYM 
%token	reportSYM
%token	restoreSYM 
%token  retrySYM
%token	returnSYM 
%token	rmdirSYM
%token	rndSYM
%token	rparenSYM 
%token	runSYM 
%token	saveSYM 
%token	scanSYM
%token	select_inputSYM 
%token	select_outputSYM 
%token	semicolonSYM
%token	staticSYM
%token	stepSYM 
%token	stopSYM 
%token	sysSYM 
%token	syssSYM
%token	thenSYM 
%token	timesSYM 
%token	toSYM 
%token	traceSYM
%token	trapSYM 
%token  unitSYM
%token	untilSYM 
%token	useSYM 
%token	usingSYM 
%token	whenSYM 
%token	whileSYM 
%token	writeSYM

%token	<inum>		rnSYM rsSYM tnrnSYM tnrsSYM tsrnSYM tonrsSYM tsrsSYM escSYM zoneSYM
%token	<dubbel>	floatnumSYM
%token	<id>		idSYM intidSYM stringidSYM
%token	<num>		intnumSYM
%token	<str>		remSYM stringSYM

%left	andSYM orSYM eorSYM andthenSYM orthenSYM
%left	eqlSYM neqSYM lssSYM gtrSYM leqSYM geqSYM
%left	plusSYM minusSYM
%left	timesSYM divideSYM inSYM divSYM modSYM
%left	bitandSYM bitorSYM bitxorSYM
%right	powerSYM
%left	USIGN

%type	<twonum>	line_range renumlines autolines
%type	<id>		optid numid id optid2
%type	<str>		optrem optfilename
%type	<expptr>	exp_list lval_list
%type	<dimptr>	dim_list dim_item local_list local_item
%type	<dimensionptr>	dim_ensions opt_dim_ensions dim_ension_list dim_ension
%type	<exp>		optstep optexp lvalue numlvalue numlvalue2 strlvalue
%type	<exp>		exp numexp stringexp xid numexp2 stringexp2 opt_stringexp
%type	<exp>		string_factor strlvalue2 optnumlvalue opt_arg
%type	<exp>		opt_numexp using_designator
%type	<extptr>	opt_external
%type	<twoexp>	file_designator substr_spec substr_spec2 at_designator
%type	<twoexpp>	optfile optat
%type	<inum>		relop pr_sep optpr_sep 
%type	<inum>		todownto nassign sassign assign1 assign2 plusorminus
%type	<imod>		input_modifier
%type	<flag>		optclosed optread_only
%type	<parmptr>	procfunc_head parmlist parmitem 
%type	<printptr>	print_list prnum_list
%type	<importptr>	import_list
%type	<oneparm>	oneparm
%type	<openrec>	open_type
%type	<whenptr>	when_list when_numitem when_stritem when_numlist 
%type	<whenptr>	when_strlist
%type	<assignptr>	assign_list assign_item
%type	<id_root>	idlist idSYMlist

%type	<cl>		comal_line command list_cmd
%type	<cl>		program_line complex_stat simple_stat complex_1word
%type	<cl>		simple_1word case_stat data_stat elif_stat for_stat
%type	<cl>		func_stat if_stat proc_stat until_stat when_stat
%type	<cl>		while_stat label_stat close_stat del_stat dim_stat
%type	<cl>		exec_stat import_stat input_stat open_stat os_stat
%type	<cl>		print_stat read_stat restore_stat return_stat run_stat
%type	<cl>		select_out_stat stop_stat sys_stat write_stat assign_stat
%type	<cl>		select_in_stat exit_stat trace_stat cursor_stat chdir_stat
%type	<cl>		rmdir_stat mkdir_stat repeat_stat
%type	<cl>		local_stat trap_stat dir_stat unit_stat static_stat
%type	<cl>		module_stat export_stat use_stat
%type	<cl>		randomize_stat report_stat delay_stat zone_stat create_stat

%type	<pcl>		optsimple_stat 

%start	a_comal_line

%%

a_comal_line	:	comal_line eolnSYM
			{
				c_line=$1;
				YYACCEPT;
			}
		|	error eolnSYM
			{
				p_error(catgets(catdesc, ParsSet, ParsSynErr, "Syntax error"));
				yyerrok;
				c_line.cmd=0;
				YYACCEPT;
			}
		;
		
comal_line	:	command 
			{
				$$=$1;
				$$.ld=NULL;
			}
		|	intnumSYM program_line optrem
			{
				$$=$2;
				$$.ld=(struct comal_line_data *)mem_alloc(PARSE_POOL,sizeof(struct comal_line_data));
				$$.ld->lineno=$1;
				$$.ld->rem=$3;
				$$.lineptr=NULL;
			}
		|	simple_stat
			{
				$$=$1;
				$$.ld=NULL;
			}
		|	optrem
			{
				$$.cmd=0;
			}
		;
		
optrem		:	remSYM
		|	/* epsilon */
			{
				$$=NULL;
			}
		;
	
		
command		:	quitSYM
			{
				$$.cmd=quitSYM;
			}
		|	list_cmd
		|	saveSYM optfilename
			{
				$$.cmd=saveSYM;
				$$.lc.str=$2;
			}
		|	loadSYM optfilename
			{
				$$.cmd=loadSYM;
				$$.lc.str=$2;
			}
		|	enterSYM stringSYM
			{
				$$.cmd=enterSYM;
				$$.lc.str=$2;
			}
		|	envSYM optid2
			{
				$$.cmd=envSYM;
				$$.lc.id=$2;
			}
		|	runSYM
			{
				$$.cmd=COMMAND(runSYM);
				$$.lc.str=NULL;
			}
		|	newSYM
			{
				$$.cmd=newSYM;
			}
		|	scanSYM
			{
				$$.cmd=scanSYM;
			}
		|	autoSYM autolines
			{
				$$.cmd=autoSYM;
				$$.lc.twonum=$2;
			}
		|	contSYM
			{
				$$.cmd=contSYM;
			}
		|	delSYM line_range
			{
				$$.cmd=COMMAND(delSYM);
				$$.lc.twonum=$2;
			}
		|	editSYM line_range
			{
				$$.cmd=editSYM;
				$$.lc.twonum=$2;
			}
		|	renumberSYM renumlines
			{
				$$.cmd=renumberSYM;
				$$.lc.twonum=$2;
			}
		;

list_cmd	:	listSYM line_range
			{
				$$.cmd=listSYM;
				$$.lc.listrec.str=NULL;
				$$.lc.listrec.twonum=$2;
				$$.lc.listrec.id=NULL;
			}
		|	listSYM stringSYM
			{
				$$.cmd=listSYM;
				$$.lc.listrec.str=$2;
				$$.lc.listrec.twonum.num1=0;
				$$.lc.listrec.twonum.num2=INT_MAX;
				$$.lc.listrec.id=NULL;
			}
		|	listSYM line_range commaSYM stringSYM
			{
				$$.cmd=listSYM;
				$$.lc.listrec.str=$4;
				$$.lc.listrec.twonum=$2;
				$$.lc.listrec.id=NULL;
			}
		|	listSYM id
			{
				$$.cmd=listSYM;
				$$.lc.listrec.str=NULL;
				$$.lc.listrec.id=$2;
			}
		|	listSYM id commaSYM stringSYM
			{
				$$.cmd=listSYM;
				$$.lc.listrec.str=$4;
				$$.lc.listrec.id=$2;
			}
		;

line_range	:	/* epsilon */
			{
				$$.num1=0;	$$.num2=INT_MAX;
			}
		|	intnumSYM
			{
				$$.num1=$1;	$$.num2=$1;
			}
		|	minusSYM intnumSYM
			{
				$$.num1=0;	$$.num2=$2;
			}
		|	intnumSYM minusSYM
			{
				$$.num1=$1;	$$.num2=INT_MAX;
			}
		|	intnumSYM minusSYM intnumSYM
			{
				$$.num1=$1;	$$.num2=$3;
			}
		;		
		
renumlines	:	intnumSYM
			{
				$$.num1=$1;	$$.num2=10;
			}
		|	intnumSYM commaSYM intnumSYM
			{
				$$.num1=$1;	$$.num2=$3;
			}
		|	commaSYM intnumSYM
			{
				$$.num1=10;	$$.num2=$2;
			}
		|	/* epsilon */
			{
				$$.num1=10;	$$.num2=10;
			}
		;

autolines	:	intnumSYM
			{
				$$.num1=$1;	$$.num2=10;
			}
		|	intnumSYM commaSYM intnumSYM
			{
				$$.num1=$1;	$$.num2=$3;
			}
		|	commaSYM intnumSYM
			{
				$$.num1=prog_highest_line()+$2;	
				$$.num2=$2;
			}
		|	/* epsilon */
			{
				$$.num1=prog_highest_line()+10;	
				$$.num2=10;
			}
		;
		
program_line	:	complex_stat
		|	simple_stat
		|	/* epsilon */
			{
				$$.cmd=0;
			}
		;
		
complex_stat	:	case_stat
		|	data_stat
		|	elif_stat
		|	exit_stat
		|	for_stat
		|	func_stat
		|	if_stat
		|	proc_stat
		|	module_stat
		|	until_stat
		|	when_stat
		|	while_stat
		|	repeat_stat
		|	label_stat
		|	complex_1word
		;
		
simple_stat	:	close_stat
		|	chdir_stat
		|	create_stat
		|	cursor_stat
		|	del_stat
		|	delay_stat
		|	dim_stat
		|	dir_stat
		|	local_stat
		|	static_stat
		|	exec_stat
		|	export_stat
		|	import_stat
		|	input_stat
		|	mkdir_stat
		|	open_stat
		|	os_stat
		|	print_stat
		|	randomize_stat
		|	read_stat
		|	report_stat
		|	restore_stat
		|	return_stat
		|	rmdir_stat
		|	run_stat
		|	select_out_stat
		|	select_in_stat
		|	stop_stat
		|	sys_stat
		|	trace_stat
		|	trap_stat
		|	unit_stat
		|	use_stat
		|	write_stat
		|	zone_stat
		|	xid
			{
				$$.cmd=execSYM;
				$$.lc.exp=$1;
			}
		|	assign_stat
		|	simple_1word
		;

complex_1word	:	elseSYM
			{
				$$.cmd=elseSYM;
			}
		|	endcaseSYM
			{
				$$.cmd=endcaseSYM;
			}
		|	endfuncSYM optid
			{
				$$.cmd=endfuncSYM;
			}
		|	endifSYM
			{
				$$.cmd=endifSYM;
			}
		|	loopSYM
			{
				$$.cmd=loopSYM;
			}
		|	endloopSYM
			{
				$$.cmd=endloopSYM;
			}
		|	endprocSYM optid2
			{
				$$.cmd=endprocSYM;
			}
		|	endwhileSYM
			{
				$$.cmd=endwhileSYM;
			}
		|	endforSYM optnumlvalue
			{
				$$.cmd=endforSYM;
			}
		|	otherwiseSYM
			{
				$$.cmd=otherwiseSYM;
			}
		|	repeatSYM
			{
				$$.cmd=repeatSYM;
				$$.lc.ifwhilerec.exp=NULL;
				$$.lc.ifwhilerec.stat=NULL;
			}
		|	trapSYM
			{
				$$.cmd=trapSYM;
				$$.lc.traprec.esc=0;
			}
		|	handlerSYM
			{
				$$.cmd=handlerSYM;
			}
		|	endtrapSYM
			{
				$$.cmd=endtrapSYM;
			}
		|	endmoduleSYM optid2
			{
				$$.cmd=endmoduleSYM;
			}
		;

simple_1word	:	nullSYM
			{
				$$.cmd=nullSYM;
			}
		|	endSYM
			{
				$$.cmd=endSYM;
			}
		|	exitSYM
			{
				$$.cmd=exitSYM;
				$$.lc.exp=NULL;
			}
		|	pageSYM
			{
				$$.cmd=pageSYM;
			}
		|	retrySYM
			{
				$$.cmd=retrySYM;
			}
		;
		
module_stat	:	moduleSYM idSYM opt_external
			{
				$$.cmd=moduleSYM;
				$$.lc.pfrec.id=$2;
				$$.lc.pfrec.closed=true;
				$$.lc.pfrec.external=$3;
				$$.lc.pfrec.staticenv=NULL;
			}
		;

use_stat	:	useSYM idSYMlist
			{
				$$.cmd=useSYM;
				$$.lc.idroot=$2;
			}
		;

export_stat	:	exportSYM idlist
			{
				$$.cmd=exportSYM;
				$$.lc.idroot=$2;
			}
		;

case_stat	:	caseSYM exp optof
			{
				$$.cmd=caseSYM;
				$$.lc.exp=$2;
			}
		;
		
close_stat	:	closeSYM
			{
				$$.cmd=closeSYM;
				$$.lc.exproot=NULL;
			}
		|	closeSYM optfileS exp_list
			{
				$$.cmd=closeSYM;
				$$.lc.exproot=(struct exp_list *)my_reverse($3);
			}
		;

create_stat	:	createSYM stringexp commaSYM numexp commaSYM numexp
	    		{
				$$.cmd=createSYM;
				$$.lc.createrec.filename=$2;
				$$.lc.createrec.top=$4;
				$$.lc.createrec.reclen=$6;
			}
		;

cursor_stat	:	cursorSYM numexp commaSYM numexp
			{
				$$.cmd=cursorSYM;
				$$.lc.twoexp.exp1=$2;
				$$.lc.twoexp.exp2=$4;
			}
		;
		
chdir_stat	:	chdirSYM stringexp
			{
				$$.cmd=chdirSYM;
				$$.lc.exp=$2;
			}
		;

rmdir_stat	:	rmdirSYM stringexp
			{
				$$.cmd=rmdirSYM;
				$$.lc.exp=$2;
			}
		;

mkdir_stat	:	mkdirSYM stringexp
			{
				$$.cmd=mkdirSYM;
				$$.lc.exp=$2;
			}
		;

data_stat	:	dataSYM exp_list
			{
				$$.cmd=dataSYM;
				$$.lc.exproot=(struct exp_list *)my_reverse($2);
			}
		;

del_stat	:	delSYM stringexp
			{
				$$.cmd=delSYM;
				$$.lc.exp=$2;
			}
		;

delay_stat	:	delaySYM numexp
	    		{
				$$.cmd=delaySYM;
				$$.lc.exp=$2;
			}
		;

dir_stat	:	dirSYM opt_stringexp
			{
				$$.cmd=dirSYM;
				$$.lc.exp=$2;
			}
		;

unit_stat	:	unitSYM stringexp
			{
				$$.cmd=unitSYM;
				$$.lc.exp=$2;
			}
		;

static_stat	:	staticSYM local_list
			{
				$$.cmd=staticSYM;
				$$.lc.dimroot=(struct dim_list *)my_reverse($2);
			}
		;


local_stat	:	localSYM local_list
			{
				$$.cmd=localSYM;
				$$.lc.dimroot=(struct dim_list *)my_reverse($2);
			}
		;

local_list	:	local_list commaSYM local_item
			{
				$$=$3;
				$$->next=$1;
			}
		|	local_item
			{
				$$=$1;
				$$->next=NULL;
			}
		;
		
local_item	:	numid opt_dim_ensions
			{
				$$=pars_dimlist_item($1,NULL,$2);
			}
		|	stringidSYM
			{
				$$=pars_dimlist_item($1,NULL,NULL);
			}
		|	stringidSYM ofSYM numexp
			{
				$$=pars_dimlist_item($1,$3,NULL);
			}
		|	stringidSYM dim_ensions of numexp
			{
				$$=pars_dimlist_item($1,$4,$2);
			}
		|	stringidSYM dim_ensions
			{
				$$=pars_dimlist_item($1,NULL,$2);
			}
		;


dim_stat	:	dimSYM dim_list
			{
				$$.cmd=dimSYM;
				$$.lc.dimroot=(struct dim_list *)my_reverse($2);
			}
		;
		
dim_list	:	dim_list commaSYM dim_item
			{
				$$=$3;
				$$->next=$1;
			}
		|	dim_item
			{
				$$=$1;
				$$->next=NULL;
			}
		;
		
dim_item	:	numid dim_ensions
			{
				$$=pars_dimlist_item($1,NULL,$2);
			}
		|	stringidSYM ofSYM numexp
			{
				$$=pars_dimlist_item($1,$3,NULL);
			}
		|	stringidSYM dim_ensions of numexp
			{
				$$=pars_dimlist_item($1,$4,$2);
			}
		|	stringidSYM dim_ensions
			{
				$$=pars_dimlist_item($1,NULL,$2);
			}
		;

of		:	ofSYM
		|	timesSYM
		;

opt_dim_ensions	:	dim_ensions
		|	/* epsilon */
			{
				$$=NULL;
			}
		;
				
dim_ensions	:	lparenSYM dim_ension_list rparenSYM
			{
				$$=$2;
			}
		;
		
dim_ension_list	:	dim_ension_list commaSYM dim_ension
			{
				$$=$3;
				$$->next=$1;
			}
		|	dim_ension
			{
				$$=$1;
				$$->next=NULL;
			}
		;
		
dim_ension	:	numexp
			{
				$$=(struct dim_ension *)mem_alloc(PARSE_POOL,sizeof(struct dim_ension));
				$$->bottom=NULL;
				$$->top=$1;
			}
		|	numexp colonSYM numexp
			{
				$$=(struct dim_ension *)mem_alloc(PARSE_POOL,sizeof(struct dim_ension));
				$$->bottom=$1;
				$$->top=$3;
			}
		|	numexp becminusSYM numexp
			{
				$$=(struct dim_ension *)mem_alloc(PARSE_POOL,sizeof(struct dim_ension));
				$$->bottom=$1;
				$$->top=pars_exp_unary(minusSYM,$3);
			}
		;

elif_stat	:	elifSYM numexp optthen
			{
				$$.cmd=elifSYM;
				$$.lc.exp=$2;
			}
		;

exit_stat	:	exitSYM ifwhen numexp
			{
				$$.cmd=exitSYM;
				$$.lc.exp=$3;
			}
		;
		
ifwhen		:	ifSYM
		|	whenSYM
		;
		
exec_stat	:	execSYM xid
			{
				$$.cmd=execSYM;
				$$.lc.exp=$2;
			}
		;

for_stat	:	forSYM numlvalue assign1 numexp todownto numexp optstep optdo optsimple_stat
			{
				$$.cmd=forSYM;
				$$.lc.forrec.lval=$2;
				$$.lc.forrec.from=$4;
				$$.lc.forrec.mode=$5;
				$$.lc.forrec.to=$6;
				$$.lc.forrec.step=$7;
				$$.lc.forrec.stat=$9;
			}
		;

todownto	:	toSYM
			{
				$$=toSYM;
			}
		|	downtoSYM
			{
				$$=downtoSYM;
			}
		;
		
optstep		:	stepSYM numexp
			{
				$$=$2;
			}
		|	/* epsilon */
			{
				$$=NULL;
			}
		;
		
func_stat	:	funcSYM id procfunc_head optclosed opt_external
			{
				$$.cmd=funcSYM;
				$$.lc.pfrec.id=$2;
				$$.lc.pfrec.parmroot=(struct parm_list *)my_reverse($3);
				$$.lc.pfrec.closed=$4;
				$$.lc.pfrec.external=$5;
				$$.lc.pfrec.staticenv=NULL;
			}
		;

if_stat		:	ifSYM numexp optthen optsimple_stat
			{
				$$.cmd=ifSYM;
				$$.lc.ifwhilerec.exp=$2;
				$$.lc.ifwhilerec.stat=$4;
			}
		;
		
import_stat	:	importSYM id colonSYM import_list
			{
				$$.cmd=importSYM;
				$$.lc.importrec.id=$2;
				$$.lc.importrec.importroot=(struct import_list *)my_reverse($4);
			}
		|	importSYM import_list
			{
				$$.cmd=importSYM;
				$$.lc.importrec.id=NULL;
				$$.lc.importrec.importroot=(struct import_list *)my_reverse($2);
			}
		;
		
import_list	:	import_list commaSYM oneparm		
			{
				$$=(struct import_list *)mem_alloc(PARSE_POOL,sizeof(struct import_list));
				$$->id=$3.id;
				$$->array=$3.array;
				$$->next=$1;				
			}
		|	oneparm
			{
				$$=(struct import_list *)mem_alloc(PARSE_POOL,sizeof(struct import_list));
				$$->id=$1.id;
				$$->array=$1.array;
				$$->next=NULL;
			}
		;

input_stat	:	inputSYM input_modifier lval_list optpr_sep
			{
				$$.cmd=inputSYM;
				$$.lc.inputrec.modifier=$2;
				$$.lc.inputrec.lvalroot=(struct exp_list *)my_reverse($3);
				$$.lc.inputrec.pr_sep=$4;
			}
		;
		
input_modifier	:	file_designator
			{
				$$=(struct input_modifier *)mem_alloc(PARSE_POOL,sizeof(struct input_modifier));
				$$->type=fileSYM;
				$$->twoexp=$1;
			}
		|	stringSYM colonSYM
			{
				$$=(struct input_modifier *)mem_alloc(PARSE_POOL,sizeof(struct input_modifier));
				$$->type=stringSYM;
				$$->str=$1;
			}
		|	atSYM numexp commaSYM numexp commaSYM numexp colonSYM stringSYM colonSYM
			{
				$$=(struct input_modifier *)mem_alloc(PARSE_POOL,sizeof(struct input_modifier));
				$$->type=atSYM;
				$$->twoexp.exp1=$2;
				$$->twoexp.exp2=$4;
				$$->len=$6;
				$$->str=$8;
			}
		|	at_designator stringSYM colonSYM
			{
				$$=(struct input_modifier *)mem_alloc(PARSE_POOL,sizeof(struct input_modifier));
				$$->type=atSYM;
				$$->twoexp=$1;
				$$->len=NULL;
				$$->str=$2;
			}
		|	at_designator
			{
				$$=(struct input_modifier *)mem_alloc(PARSE_POOL,sizeof(struct input_modifier));
				$$->type=atSYM;
				$$->twoexp=$1;
				$$->len=NULL;
				$$->str=NULL;
			}
		|	/* epsilon */
			{
				$$=NULL;
			}
		;

open_stat	:	openSYM fileSYM numexp commaSYM stringexp commaSYM open_type
			{
				$$.cmd=openSYM;
				$$.lc.openrec=$7;
				$$.lc.openrec.filenum=$3;
				$$.lc.openrec.filename=$5;
			}
		;
		
open_type	:	readSYM
			{
				$$.type=readSYM;
			}
		|	writeSYM
			{
				$$.type=writeSYM;
			}
		|	appendSYM
			{
				$$.type=appendSYM;
			}
		|	randomSYM numexp optread_only
			{
				$$.type=randomSYM;
				$$.reclen=$2;
				$$.read_only=$3;
			}
		;

os_stat		:	osSYM stringexp
			{
				$$.cmd=osSYM;
				$$.lc.exp=$2;
			}
		;

print_stat	:	printi
			{
				$$.cmd=printSYM;
				$$.lc.printrec.modifier=NULL;
				$$.lc.printrec.printroot=NULL;
				$$.lc.printrec.pr_sep=0;
			}
		|	printi optat using_designator prnum_list optpr_sep
			{
				$$.cmd=printSYM;
				$$.lc.printrec.modifier=(struct print_modifier *)mem_alloc(PARSE_POOL,sizeof(struct print_modifier));
				$$.lc.printrec.modifier->type=atSYM;
				$$.lc.printrec.modifier->twoexp=$2;
				$$.lc.printrec.modifier->c_using=$3;
				$$.lc.printrec.printroot=(struct print_list *)my_reverse($4);
				$$.lc.printrec.pr_sep=$5;
			}
		|	printi optat print_list optpr_sep
			{
				$$.cmd=printSYM;
				$$.lc.printrec.modifier=(struct print_modifier *)mem_alloc(PARSE_POOL,sizeof(struct print_modifier));
				$$.lc.printrec.modifier->type=atSYM;
				$$.lc.printrec.modifier->twoexp=$2;
				$$.lc.printrec.printroot=(struct print_list *)my_reverse($3);
				$$.lc.printrec.pr_sep=$4;
			}
		|	printi file_designator print_list optpr_sep
			{
				$$.cmd=printSYM;
				$$.lc.printrec.modifier=(struct print_modifier *)mem_alloc(PARSE_POOL,sizeof(struct print_modifier));
				$$.lc.printrec.modifier->type=fileSYM;
				$$.lc.printrec.modifier->twoexp=(struct two_exp *)mem_alloc(PARSE_POOL,sizeof(struct two_exp));
				*($$.lc.printrec.modifier->twoexp)=$2;
				$$.lc.printrec.printroot=(struct print_list *)my_reverse($3);
				$$.lc.printrec.pr_sep=$4;
			}
		|	printi file_designator using_designator prnum_list optpr_sep
			{
				$$.cmd=printSYM;
				$$.lc.printrec.modifier=(struct print_modifier *)mem_alloc(PARSE_POOL,sizeof(struct print_modifier));
				$$.lc.printrec.modifier->type=fileSYM;
				$$.lc.printrec.modifier->twoexp=(struct two_exp *)mem_alloc(PARSE_POOL,sizeof(struct two_exp));
				*($$.lc.printrec.modifier->twoexp)=$2;
				$$.lc.printrec.modifier->c_using=$3;
				$$.lc.printrec.printroot=(struct print_list *)my_reverse($4);
				$$.lc.printrec.pr_sep=$5;
			}
		;
		
printi		:	printSYM
		|	semicolonSYM
		;
		
prnum_list	:	prnum_list pr_sep numexp
			{
				$$=pars_printlist_item($2,$3,$1);
			}
		|	numexp
			{
				$$=pars_printlist_item(0,$1,NULL);
			}
		;
		
print_list	:	print_list pr_sep exp
			{
				$$=pars_printlist_item($2,$3,$1);
			}
		|	exp
			{
				$$=pars_printlist_item(0,$1,NULL);
			}
		;

pr_sep		:	commaSYM
			{
				$$=commaSYM;
			}
		|	semicolonSYM
			{
				$$=semicolonSYM;
			}
		;
		
optpr_sep	:	pr_sep
		|	/* epsilon */
			{
				$$=0;
			}
		;
		
proc_stat	:	procSYM idSYM procfunc_head optclosed opt_external
			{
				$$.cmd=procSYM;
				$$.lc.pfrec.id=$2;
				$$.lc.pfrec.closed=$4;
				$$.lc.pfrec.external=$5;
				$$.lc.pfrec.parmroot=(struct parm_list *)my_reverse($3);
				$$.lc.pfrec.staticenv=NULL;
			}
		;

randomize_stat	:	randomizeSYM opt_numexp
	       		{
				$$.cmd=randomizeSYM;
				$$.lc.exp=$2;
			}

read_stat	:	readSYM optfile lval_list
			{
				$$.cmd=readSYM;
				$$.lc.readrec.modifier=$2;
				$$.lc.readrec.lvalroot=(struct exp_list *)my_reverse($3);
			}
		;

report_stat	:	reportSYM numexp
	    		{
				$$.cmd=reportSYM;
				$$.lc.exp=$2;
			}

restore_stat	:	restoreSYM optid2
			{
				$$.cmd=restoreSYM;
				$$.lc.id=$2;
			}
		;

return_stat	:	returnSYM optexp
			{
				$$.cmd=returnSYM;
				$$.lc.exp=$2;
			}
		;

run_stat	:	runSYM stringexp
			{
				$$.cmd=runSYM;
				$$.lc.exp=$2;
			}
		;		

select_out_stat	:	select_outputSYM stringexp
			{
				$$.cmd=select_outputSYM;
				$$.lc.exp=$2;
			}
		;


select_in_stat	:	select_inputSYM stringexp
			{
				$$.cmd=select_inputSYM;
				$$.lc.exp=$2;
			}
		;

stop_stat	:	stopSYM optexp
			{
				$$.cmd=stopSYM;
				$$.lc.exp=$2;
			}
		;

sys_stat	:	sysSYM exp_list
			{
				$$.cmd=sysSYM;
				$$.lc.exproot=(struct exp_list *)my_reverse($2);
			}
		;

until_stat	:	untilSYM numexp
			{
				$$.cmd=untilSYM;
				$$.lc.exp=$2;
			}
		;

trace_stat	:	traceSYM numexp
			{
				char *cmd=exp_cmd($2);
				
				if (strcmp(cmd,"on")!=0 && strcmp(cmd,"off")!=0)
					pars_error("TRACE \"on\" or \"off\"");
				
				$$.cmd=traceSYM;
				$$.lc.exp=$2;
			}
		;

trap_stat	:	trapSYM escSYM plusorminus
			{
				$$.cmd=trapSYM;
				$$.lc.traprec.esc=$3;
			}
		;		

plusorminus	:	plusSYM
			{
				$$=plusSYM;
			}
		|	minusSYM
			{
				$$=minusSYM;
			}
		;
				
when_stat	:	whenSYM when_list
			{
				$$.cmd=whenSYM;
				$$.lc.whenroot=(struct when_list *)my_reverse($2);
			}
		;

when_list	:	when_numlist
		|	when_strlist
		;
				
when_numlist	:	when_numlist commaSYM when_numitem
			{
				$$=$3;
				$$->next=$1;
			}
		|	when_numitem
		;
		
when_numitem	:	relop numexp
			{
				$$=pars_whenlist_item($1,$2,NULL);
			}
		|	numexp
			{
				$$=pars_whenlist_item(eqlSYM,$1,NULL);
			}
		;
				
when_strlist	:	when_strlist commaSYM when_stritem
			{
				$$=$3;
				$$->next=$1;
			}
		|	when_stritem
		;
		
when_stritem	:	relop stringexp
			{
				$$=pars_whenlist_item($1,$2,NULL);
			}
		|	stringexp
			{
				$$=pars_whenlist_item(eqlSYM,$1,NULL);
			}
		|	inSYM stringexp
			{
				$$=pars_whenlist_item(inSYM,$2,NULL);
			}
		;
		
relop		:	gtrSYM
			{
				$$=gtrSYM;
			}
		|	lssSYM
			{
				$$=lssSYM;
			}
		|	eqlSYM
			{
				$$=eqlSYM;
			}
		|	neqSYM
			{
				$$=neqSYM;
			}
		|	geqSYM
			{
				$$=geqSYM;
			}
		|	leqSYM
			{
				$$=leqSYM;
			}
		;

while_stat	:	whileSYM numexp optdo optsimple_stat
			{
				$$.cmd=whileSYM;
				$$.lc.ifwhilerec.exp=$2;
				$$.lc.ifwhilerec.stat=$4;
			}
		;

repeat_stat	:	repeatSYM simple_stat untilSYM numexp
			{
				$$.cmd=repeatSYM;
				$$.lc.ifwhilerec.exp=$4;
				$$.lc.ifwhilerec.stat=stat_dup(&$2);
			}
		;

write_stat	:	writeSYM file_designator exp_list
			{
				$$.cmd=writeSYM;
				$$.lc.writerec.twoexp=$2;
				$$.lc.writerec.exproot=(struct exp_list *)my_reverse($3);
			}
		;

zone_stat	:	zoneSYM numexp
	    		{
				$$.cmd=zoneSYM;
				$$.lc.exp=$2;
			}
		;

assign_stat	:	assign_list
			{
				$$.cmd=becomesSYM;
				$$.lc.assignroot=(struct assign_list *)my_reverse($1);
			}
		;
		
assign_list	:	assign_list semicolonSYM assign_item
			{
				$$=$3;
				$$->next=$1;
			}
		|	assign_item
			{
				$$->next=NULL;
			}
		;
		
assign_item	:	numlvalue nassign numexp
			{
				$$=pars_assign_item($2,$1,$3);
			}
		|	strlvalue sassign stringexp
			{
				$$=pars_assign_item($2,$1,$3);
			}
		;

nassign		:	assign1
		|	assign2
		;

sassign		:	assign1
		|	becplusSYM
			{
				$$=becplusSYM;
			}
		;
				
assign1		:	eqlSYM
			{
				$$=becomesSYM;
			}
		|	becomesSYM
			{
				$$=becomesSYM;
			}
		;
		
assign2		:	becplusSYM
			{
				$$=becplusSYM;
			}
		|	becminusSYM
			{
				$$=becminusSYM;
			}
		;
		
label_stat	:	idSYM colonSYM
			{
				$$.cmd=idSYM;
				$$.lc.id=$1;
			}
		;
		
xid		:	idSYM
			{
				$$=pars_exp_id(idSYM,$1,NULL);
			}
		|	idSYM lparenSYM exp_list rparenSYM
			{
				$$=pars_exp_id(idSYM,$1,$3);
			}
		|	idSYM lparenSYM opt_commalist rparenSYM
			{
				$$=pars_exp_array(idSYM,$1,T_ARRAY);
			}
		;

exp		:	numexp
		|	stringexp
		;
				
numexp		:	numexp2
			{
				$$=pars_exp_num($1);
			}
		;
		
numexp2		:	numexp2 eqlSYM numexp2
			{
				$$=pars_exp_binary(eqlSYM,$1,$3);
			}
		|	numexp2 neqSYM numexp2		
			{
				$$=pars_exp_binary(neqSYM,$1,$3);
			}
		|	numexp2 lssSYM numexp2		
			{
				$$=pars_exp_binary(lssSYM,$1,$3);
			}
		|	numexp2 gtrSYM numexp2		
			{
				$$=pars_exp_binary(gtrSYM,$1,$3);
			}
		|	numexp2 leqSYM numexp2		
			{
				$$=pars_exp_binary(leqSYM,$1,$3);
			}
		|	numexp2 geqSYM numexp2		
			{
				$$=pars_exp_binary(geqSYM,$1,$3);
			}
		|	numexp2 andSYM numexp2		
			{
				$$=pars_exp_binary(andSYM,$1,$3);
			}
		|	numexp2 andthenSYM numexp2		
			{
				$$=pars_exp_binary(andthenSYM,$1,$3);
			}
		|	numexp2 orSYM numexp2		
			{
				$$=pars_exp_binary(orSYM,$1,$3);
			}
		|	numexp2 orthenSYM numexp2		
			{
				$$=pars_exp_binary(orthenSYM,$1,$3);
			}
		|	numexp2 eorSYM numexp2		
			{
				$$=pars_exp_binary(eorSYM,$1,$3);
			}
		|	numexp2 bitandSYM numexp2
			{
				$$=pars_exp_binary(bitandSYM,$1,$3);
			}
		|	numexp2 bitorSYM numexp2
			{
				$$=pars_exp_binary(bitorSYM,$1,$3);
			}
		|	numexp2 bitxorSYM numexp2
			{
				$$=pars_exp_binary(bitxorSYM,$1,$3);
			}
		|	numexp2 plusSYM numexp2		
			{
				$$=pars_exp_binary(plusSYM,$1,$3);
			}
		|	numexp2 minusSYM numexp2		
			{
				$$=pars_exp_binary(minusSYM,$1,$3);
			}
		|	numexp2 timesSYM numexp2		
			{
				$$=pars_exp_binary(timesSYM,$1,$3);
			}
		|	numexp2 divideSYM numexp2		
			{
				$$=pars_exp_binary(divideSYM,$1,$3);
			}
		|	numexp2 powerSYM numexp2		
			{
				$$=pars_exp_binary(powerSYM,$1,$3);
			}
		|	numexp2 divSYM numexp2		
			{
				$$=pars_exp_binary(divSYM,$1,$3);
			}
		|	numexp2 modSYM numexp2		
			{
				$$=pars_exp_binary(modSYM,$1,$3);
			}
		|	stringexp2 eqlSYM stringexp2
			{
				$$=pars_exp_binary(eqlSYM,$1,$3);
			}
		|	stringexp2 neqSYM stringexp2
			{
				$$=pars_exp_binary(neqSYM,$1,$3);
			}
		|	stringexp2 lssSYM stringexp2
			{
				$$=pars_exp_binary(lssSYM,$1,$3);
			}
		|	stringexp2 gtrSYM stringexp2
			{
				$$=pars_exp_binary(gtrSYM,$1,$3);
			}
		|	stringexp2 leqSYM stringexp2
			{
				$$=pars_exp_binary(leqSYM,$1,$3);
			}
		|	stringexp2 geqSYM stringexp2
			{
				$$=pars_exp_binary(geqSYM,$1,$3);
			}
		|	stringexp2 inSYM stringexp2
			{
				$$=pars_exp_binary(inSYM,$1,$3);
			}
		|	minusSYM numexp2 %prec USIGN
			{
				$$=pars_exp_unary(minusSYM,$2);
			}
		|	plusSYM numexp2 %prec USIGN
			{
				$$=pars_exp_unary(plusSYM,$2);
			}
		|	intnumSYM
			{
				$$=pars_exp_int($1);
			}
		|	floatnumSYM
			{
				$$=pars_exp_float(&$1);
			}
		|	numlvalue2
		|	tsrnSYM lparenSYM stringexp2 rparenSYM
			{
				$$=pars_exp_unary($1,$3);
			}
		|	tnrnSYM lparenSYM numexp2 rparenSYM
			{
				$$=pars_exp_unary($1,$3);
			}
		|	rndSYM
			{
				$$=pars_exp_binary(_RND,NULL,NULL);
			}
		|	rndSYM lparenSYM numexp2 rparenSYM
			{
				$$=pars_exp_binary(_RND,NULL,$3);
			}
		|	rndSYM lparenSYM numexp2 commaSYM numexp2 rparenSYM
			{
				$$=pars_exp_binary(_RND,$3,$5);
			}
		|	rnSYM
			{
				$$=pars_exp_const($1);
			}
		|	escSYM
			{
				$$=pars_exp_const($1);
			}
		|	zoneSYM
			{
				$$=pars_exp_const($1);
			}
		|	sysSYM lparenSYM exp_list rparenSYM
			{
				$$=pars_exp_sys(sysSYM,T_SYS,$3);
			}
		|	lparenSYM numexp2 rparenSYM
			{
				$$=pars_exp_unary(lparenSYM,$2);
			}
		;

stringexp	:	stringexp2
			{
				$$=pars_exp_str($1);
			}
		;

stringexp2	:	stringexp2 plusSYM string_factor
			{
				$$=pars_exp_binary(plusSYM,$1,$3);
			}
		|	string_factor
		|	stringexp2 timesSYM numexp2
			{
				$$=pars_exp_binary(timesSYM,$1,$3);
			}
		;

opt_stringexp	:	stringexp
		|	/* epsilon */
			{
				$$=NULL;
			}
		;

opt_numexp	:	numexp
	   	|	/* epsilon */
			{
				$$=NULL;
			}
		;

string_factor	:	strlvalue2
		|	string_factor substr_spec
			{
				$$=pars_exp_substr($1,&$2);
			}
		|	tnrsSYM lparenSYM numexp2 rparenSYM
			{
				$$=pars_exp_unary($1,$3);
			}
		|	tsrsSYM lparenSYM stringexp2 rparenSYM
			{
				$$=pars_exp_unary($1,$3);
			}
		|	rsSYM
			{
				$$=pars_exp_const($1);
			}
		|	tonrsSYM opt_arg
			{
				$$=pars_exp_unary($1,$2);
			}
		|	stringSYM
			{
				$$=pars_exp_string($1);
			}
		|	syssSYM lparenSYM exp_list rparenSYM
			{
				$$=pars_exp_sys(syssSYM,T_SYSS,$3);
			}
		|	lparenSYM stringexp2 rparenSYM
			{
				$$=pars_exp_unary(lparenSYM,$2);
			}
		|	getSYM lparenSYM numexp2 commaSYM numexp2 rparenSYM
			{
				$$=pars_exp_binary(_GET,$3,$5);
			}
		;

opt_arg		:	lparenSYM numexp2 rparenSYM
			{
				$$=$2;
			}
		|	/* epsilon */
			{
				$$=NULL;
			}
		;
				
substr_spec	:	lparenSYM substr_spec2 rparenSYM
			{
				$$=$2;
			}
		;
		
substr_spec2	:	numexp colonSYM numexp
			{
				$$.exp1=$1;
				$$.exp2=$3;
			}
		|	colonSYM numexp
			{
				$$.exp1=NULL;
				$$.exp2=$2;
			}
		|	numexp colonSYM
			{
				$$.exp1=$1;
				$$.exp2=NULL;
			}
		|	colonSYM numexp colonSYM
			{
				$$.exp1=$2;
				$$.exp2=$2;
			}
		;
	
optnumlvalue	:	numlvalue
		|	/* epsilon */
			{
				$$=NULL;
			}
		;
			
optexp		:	exp
		|	/* epsilon */
			{
				$$=NULL;
			}	
		;

optid		:	id
		|	/* epsilon */
			{
				$$=NULL;
			}
		;

optid2		:	idSYM
		|	/* epsilon */
			{
				$$=NULL;
			}
		;
		
optfile		:	file_designator
			{
				$$=(struct two_exp *)mem_alloc(PARSE_POOL,sizeof(struct two_exp));
				
				*($$)=$1;
			}
		|	/* epsilon */
			{	
				$$=NULL;
			}
		;

optfileS	:	fileSYM
		|	/* epsilon */
		;
				
lval_list	:	lval_list commaSYM lvalue
			{
				$$=pars_explist_item($3,$1);
			}
		|	lvalue
			{
				$$=pars_explist_item($1,NULL);
			}
		;
		
lvalue		:	numlvalue
		|	strlvalue
		;
	
numlvalue	:	numlvalue2
			{
				if (!exp_list_of_nums($1->e.expid.exproot))
					pars_error("Indices of numeric lvalue \"%s\" must be numerics",$1->e.expid.id->name);
			}
		;
			
numlvalue2	:	xid
		|	intidSYM
			{
				$$=pars_exp_id(intidSYM,$1,NULL);
			}
		|	intidSYM lparenSYM exp_list rparenSYM
			{
				$$=pars_exp_id(intidSYM,$1,$3);
			}
		|	intidSYM lparenSYM opt_commalist rparenSYM
			{
				$$=pars_exp_array(intidSYM,$1,T_ARRAY);
			}
		;
		
strlvalue	:	strlvalue2
			{
				if (!exp_list_of_nums($1->e.expsid.exproot))
					pars_error("Indices of string lvalue \"%s\" must be numerics",$1->e.expsid.id->name);
			}
		;
		
strlvalue2	:	stringidSYM
			{
				$$=pars_exp_sid($1,NULL,NULL);
			}
		|	stringidSYM lparenSYM exp_list rparenSYM
			{
				$$=pars_exp_sid($1,$3,NULL);
			}
		|	stringidSYM substr_spec
			{
				$$=pars_exp_sid($1,NULL,&$2);
			}
		|	stringidSYM lparenSYM exp_list rparenSYM substr_spec
			{
				$$=pars_exp_sid($1,$3,&$5);
			}
		|	stringidSYM lparenSYM opt_commalist rparenSYM
			{
				$$=pars_exp_array(intidSYM,$1,T_SARRAY);
			}
		;
		
file_designator	:	fileSYM numexp colonSYM
			{
				$$.exp1=$2;
				$$.exp2=NULL;
			}
		|	fileSYM numexp commaSYM numexp colonSYM
			{
				$$.exp1=$2;
				$$.exp2=$4;
			}
		;

at_designator	:	atSYM numexp commaSYM numexp colonSYM
	      		{
				$$.exp1=$2;
				$$.exp2=$4;
			}
		;

optat		:	at_designator
			{
				$$=(struct two_exp *)mem_alloc(PARSE_POOL,sizeof(struct two_exp));
				*($$)=$1;
			}
		|	/* epsilon */
			{
				$$=NULL;
			}
		;

using_designator:	usingSYM stringexp colonSYM
			{
				$$=$2;
			}

opt_external	:	externalSYM stringexp
			{
				$$=(struct ext_rec *)mem_alloc(PARSE_POOL,sizeof(struct ext_rec));
				
				$$->dynamic=0;
				$$->filename=$2;
				$$->seg=NULL;
			}
		|	dynamicSYM externalSYM stringexp
			{
				$$=(struct ext_rec *)mem_alloc(PARSE_POOL,sizeof(struct ext_rec));
				
				$$->dynamic=dynamicSYM;
				$$->filename=$3;
				$$->seg=NULL;
			}
		|	staticSYM externalSYM stringexp
			{
				$$=(struct ext_rec *)mem_alloc(PARSE_POOL,sizeof(struct ext_rec));
				
				$$->dynamic=staticSYM;
				$$->filename=$3;
				$$->seg=NULL;
			}
		|	/* epsilon */
			{
				$$=NULL;
			}
		;
		
procfunc_head	:	lparenSYM parmlist rparenSYM
			{
				$$=$2;
			}
		|	/* epsilon */
			{
				$$=NULL;
			}
		;
		
parmlist	:	parmlist commaSYM parmitem
			{
				$$=$3;
				$$->next=$1;
			}
		|	parmitem
			{
				$$=$1;
				$$->next=NULL;
			}
		;
		
parmitem	:	oneparm
			{
				$$=(struct parm_list *)mem_alloc(PARSE_POOL,sizeof(struct parm_list));
				$$->id=$1.id;
				$$->array=$1.array;
				$$->ref=0;
			}
		|	refSYM oneparm
			{
				$$=(struct parm_list *)mem_alloc(PARSE_POOL,sizeof(struct parm_list));
				$$->id=$2.id;
				$$->array=$2.array;
				$$->ref=refSYM;
			}
		|	nameSYM id
			{
				$$=(struct parm_list *)mem_alloc(PARSE_POOL,sizeof(struct parm_list));
				$$->id=$2;
				$$->array=0;
				$$->ref=nameSYM;
			}
		|	procSYM idSYM
			{
				$$=(struct parm_list *)mem_alloc(PARSE_POOL,sizeof(struct parm_list));
				$$->id=$2;
				$$->array=0;
				$$->ref=procSYM;
			}
		|	funcSYM id
			{
				$$=(struct parm_list *)mem_alloc(PARSE_POOL,sizeof(struct parm_list));
				$$->id=$2;
				$$->array=0;
				$$->ref=funcSYM;
			}	
		;
		
oneparm		:	id
			{
				$$.id=$1;
				$$.array=0;
			}
		|	id lparenSYM opt_commalist rparenSYM
			{
				$$.id=$1;
				$$.array=1;
			}
		;
		
id		:	numid
		|	stringidSYM
		;
				
numid		:	idSYM
		|	intidSYM
		;

idlist		:	id commaSYM idlist
			{
				$$=pars_idlist_item($1,$3);
			}
		|	id
			{
				$$=pars_idlist_item($1,NULL);
			}
		;

idSYMlist	:	idSYM commaSYM idSYMlist
			{
				$$=pars_idlist_item($1,$3);
			}
		|	idSYM
			{
				$$=pars_idlist_item($1,NULL);
			}
		;

opt_commalist	:	opt_commalist commaSYM
		|	/* epsilon */
		;

exp_list	:	exp_list commaSYM exp
			{
				$$=pars_explist_item($3,$1);
			}
		|	exp
			{
				$$=pars_explist_item($1,NULL);
			}
		;
		
optsimple_stat	:	simple_stat
			{
				if ($1.cmd<0)
					$$=NULL;
				else
				{
					$$=stat_dup(&$1);
					$$->ld=NULL;
				}
			}
		|	/* epsilon */
			{
				$$=NULL;
			}
		;

optfilename	:	stringSYM
		|	/* epsilon */
			{
				$$=NULL;
			}
		;
		
optof		:	ofSYM
		|	/* epsilon */
		;

optdo		:	doSYM
		|	/* epsilon */
		;

optthen		:	thenSYM
		|	/* epsilon */
		;
		
optread_only	:	read_onlySYM
			{
				$$=true;
			}
		|	/* epsilon */
			{
				$$=false;
			}
		;	

optclosed	:	closedSYM
			{
				$$=true;
			}
		|	/* epsilon */
			{
				$$=false;
			}
		;	

%%

PRIVATE void p_error(const char *s)
	{
		pars_error("%s", s);
		yyclearin;
	}
	
