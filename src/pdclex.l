%{
/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */


/********** LEXICAL ANALYSIS for OpenComal **********/

#include "pdcglob.h"
#include "pdclexs.h"
#include "pdcparss.h"
#include "pdcmisc.h"

/* #define PDCLEX_DEBUG 1 */

PRIVATE int pdc_input();	/* Definition for FLEX */

#ifdef FLEX
#undef YY_INPUT
#define YY_INPUT(buf,result,maxsize) { *buf=pdc_input(); result=1; }
#endif

#ifdef PC_LEX
#undef input
#define input() pdc_input()
#endif

#undef unput
#define YY_NO_UNPUT

#define UNPUTMAXSTACKSIZE 	(MAX_LINELEN)

PRIVATE char curline[MAX_LINELEN];
PRIVATE int lineptr=0;

PRIVATE char unputstack[UNPUTMAXSTACKSIZE];
PRIVATE int unputsp=0;

#ifdef PDCLEX_DEBUG
#define RETURN(x) { int i=x; printf("FLEX RETURNS:%d (%s)\n",i,yytext); return i; }
#else
#define RETURN(x) return x;
#endif

%}

A			[Aa]
B			[Bb]
C			[Cc]
D			[Dd]
E			[Ee]
F			[Ff]
G			[Gg]
H			[Hh]
I			[Ii]
J			[Jj]
K			[Kk]
L			[Ll]
M			[Mm]
N			[Nn]
O			[Oo]
P			[Pp]
Q			[Qq]
R			[Rr]
S			[Ss]
T			[Tt]
U			[Uu]
V			[Vv]
W			[Ww]
X			[Xx]
Y			[Yy]

number			[0-9]
letter			[A-Za-z'_\xa6\xa8\xb4\xb8\xbe\xc0-\xd6\xd8-\xf6\xf8-\xff]
netter			[A-Za-z'_\xa6\xa8\xb4\xb8\xbe\xc0-\xd6\xd8-\xf6\xf8-\xff0-9]
exp			[Ee][\-\+]?{number}+

%%
[ \t]+						;
\r?\n						RETURN(eolnSYM);
{R}{E}{A}{D}[ \t]+{O}{N}{L}{Y}			RETURN(read_onlySYM);
{S}{E}{L}{E}{C}{T}[ \t]+{O}{U}{T}{P}{U}{T}	RETURN(select_outputSYM);
{S}{E}{L}{E}{C}{T}[ \t]+{I}{N}{P}{U}{T}		RETURN(select_inputSYM);
{A}{N}{D}[ \t]+{T}{H}{E}{N}			RETURN(andthenSYM);
{O}{R}[ \t]+{T}{H}{E}{N}			RETURN(orthenSYM);
\-						RETURN(minusSYM);
\*						RETURN(timesSYM);
\+						RETURN(plusSYM);
\/						RETURN(divideSYM);
\^						RETURN(powerSYM);
\(						RETURN(lparenSYM);
\)						RETURN(rparenSYM);
\?						RETURN(printSYM);
\=						RETURN(eqlSYM);
\:=						RETURN(becomesSYM);
\:\+						RETURN(becplusSYM);
\:\-						RETURN(becminusSYM);
\<						RETURN(lssSYM);
\<=						RETURN(leqSYM);
\<>						RETURN(neqSYM);
!=						RETURN(neqSYM);
\>						RETURN(gtrSYM);
\>=						RETURN(geqSYM);
\:						RETURN(colonSYM);
\;						RETURN(semicolonSYM);
\,						RETURN(commaSYM);
\#						RETURN(fileSYM);
\/\/[^\r\n]*					RETURN(lex_rem());
\"([^\\]\"\"|\\\"|[^\"\n])*\"?			RETURN(lex_string_flatten());
{letter}{netter}*				RETURN(lex_id(idSYM));
{letter}{netter}*\#				RETURN(lex_id(intidSYM));
{letter}{netter}*\$				RETURN(lex_id(stringidSYM));
{number}+					RETURN(lex_intnum());
{number}+{exp}					RETURN(lex_floatnum());
{number}+\.{number}+({exp})?			RETURN(lex_floatnum());
.						pars_error("Unexpected character 0x%02x",*yytext);
%%

PUBLIC int yywrap()
	{
		return 1;
	}

PRIVATE void pdc_unput(char c)
	{

		#ifdef PDCLEX_DEBUG
			printf("PDCLEX UNPUT:%c\n",c);
		#endif

		if (unputsp==UNPUTMAXSTACKSIZE)
			fatal("UNPUT buffer overflow");
			
		unputstack[unputsp++]=c;
	}


PRIVATE int pdc_input2()
	{
		if (unputsp>0) return unputstack[--unputsp];
	
		if (curline[lineptr]=='\0') return '\n';
		
		return curline[lineptr++];
	}


PRIVATE int pdc_input()
	{
		char c=pdc_input2();

		#ifdef PDCLEX_DEBUG
			printf("PDCLEX INPUT:%c(%d)\n",c,(int)c);
		#endif

		return c;
	}

PUBLIC int lex_pos()
	{
		return lineptr-unputsp-1+3-yyleng;
	}


PUBLIC void lex_setinput(char *line)
	{
		#ifdef FLEX
			yyrestart(NULL);
		#endif

		term_strncpy(curline,line,MAX_LINELEN);
		lineptr=0;
	}
		
	
PUBLIC void lex_unput(char c)
	{
		pdc_unput(c);
	}


PUBLIC int lex_leng()
	{
		return yyleng;
	}

