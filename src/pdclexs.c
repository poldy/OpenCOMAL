/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* Lex support routines */

#define PDCPARS
#include "pdcglob.h"
#undef PDCPARS
#include "pdclexs.h"
#include "pdcid.h"
#include "pdcmisc.h"
#include "pdcstr.h"
#include "pdcparss.h"

#ifdef OS_MSDOS
#include "pdcpars.h"
#else
#include "pdcpars.tab.h"
#endif

#include <stdlib.h>
#include <string.h>

extern char *yytext;
//extern YYSTYPE yylval;

PRIVATE struct {
	int sym;
	int func;
	const char *txt;
} lexemetab[] = {
	{
	minusSYM, 0, "-"}, {
	timesSYM, 0, "*"}, {
	plusSYM, 0, "+"}, {
	divideSYM, 0, "/"}, {
	powerSYM, 0, "^"}, {
	lparenSYM, 0, "("}, {
	rparenSYM, 0, ")"}, {
	eqlSYM, 0, "="}, {
	becomesSYM, 0, ":="}, {
	becplusSYM, 0, ":+"}, {
	becminusSYM, 0, ":-"}, {
	lssSYM, 0, "<"}, {
	leqSYM, 0, "<="}, {
	neqSYM, 0, "<>"}, {
	gtrSYM, 0, ">"}, {
	geqSYM, 0, ">="}, {
	colonSYM, 0, ":"}, {
	semicolonSYM, 0, ";"}, {
	commaSYM, 0, ","}, {
	tnrnSYM, _ABS, "ABS"}, {
	tnrnSYM, _ACS, "ACS"}, {
	andSYM, 0, "AND"}, {
	andthenSYM, 0, "AND THEN"}, {
	appendSYM, 0, "APPEND"}, {
	tnrnSYM, _ASN, "ASN"}, {
	tnrnSYM, _ATN, "ATN"}, {
	autoSYM, 0, "AUTO"}, {
	bitandSYM, 0, "BITAND"}, {
	bitorSYM, 0, "BITOR"}, {
	bitxorSYM, 0, "BITXOR"}, {
	caseSYM, 0, "CASE"}, {
	chdirSYM, 0, "CHDIR"}, {
	tnrsSYM, _CHR, "CHR$"}, {
	closeSYM, 0, "CLOSE"}, {
	closedSYM, 0, "CLOSED"}, {
	contSYM, 0, "CON"}, {
	rnSYM, _CURCOL, "CURCOL"}, {
	rnSYM, _CURROW, "CURROW"}, {
	cursorSYM, 0, "CURSOR"}, {
	tnrnSYM, _COS, "COS"}, {
	dataSYM, 0, "DATA"}, {
	tnrnSYM, _DEG, "DEG"}, {
	delaySYM, 0, "DELAY"}, {
	delSYM, 0, "DELETE"}, {
	delSYM, 0, "DEL"}, {
	dimSYM, 0, "DIM"}, {
	dirSYM, 0, "DIR"}, {
	rsSYM, _DIR, "DIR$"}, {
	divSYM, 0, "DIV"}, {
	doSYM, 0, "DO"}, {
	downtoSYM, 0, "DOWNTO"}, {
	dynamicSYM, 0, "DYNAMIC"}, {
	editSYM, 0, "EDIT"}, {
	elifSYM, 0, "ELIF"}, {
	elseSYM, 0, "ELSE"}, {
	endSYM, 0, "END"}, {
	endcaseSYM, 0, "ENDCASE"}, {
	endforSYM, 0, "ENDFOR"}, {
	endforSYM, 0, "NEXT"}, {
	endfuncSYM, 0, "ENDFUNC"}, {
	endifSYM, 0, "ENDIF"}, {
	endloopSYM, 0, "ENDLOOP"}, {
	endprocSYM, 0, "ENDPROC"}, {
	endtrapSYM, 0, "ENDTRAP"}, {
	enterSYM, 0, "ENTER"}, {
	endwhileSYM, 0, "ENDWHILE"}, {
	envSYM, 0, "ENV"}, {
	rnSYM, _EOD, "EOD"}, {
	tnrnSYM, _EOF, "EOF"}, {
	eorSYM, 0, "EOR"}, {
	endmoduleSYM, 0, "ENDMODULE"}, {
	rnSYM, _ERR, "ERR"}, {
	rnSYM, _ERRLINE, "ERRLINE"}, {
	rsSYM, _ERRTEXT, "ERRTEXT$"}, {
	escSYM, _ESC, "ESC"}, {
	execSYM, 0, "EXEC"}, {
	exitSYM, 0, "EXIT"}, {
	exportSYM, 0, "EXPORT"}, {
	externalSYM, 0, "EXTERNAL"}, {
	tnrnSYM, _EXP, "EXP"}, {
	rnSYM, _FALSE, "FALSE"}, {
	fileSYM, 0, "FILE"}, {
	forSYM, 0, "FOR"}, {
	tnrnSYM, _FRAC, "FRAC"}, {
	rnSYM, _FREEFILE, "FREEFILE"}, {
	funcSYM, 0, "FUNC"}, {
	importSYM, 0, "IMPORT"}, {
	getSYM, _GET, "GET$"}, {
	importSYM, 0, "GLOBAL"}, {
	handlerSYM, 0, "HANDLER"}, {
	ifSYM, 0, "IF"}, {
	tonrsSYM, _INKEY, "INKEY$"}, {
	inSYM, 0, "IN"}, {
	inputSYM, 0, "INPUT"}, {
	tnrnSYM, _INT, "INT"}, {
	rsSYM, _KEY, "KEY$"}, {
	tsrnSYM, _LEN, "LEN"}, {
	listSYM, 0, "LIST"}, {
	tnrnSYM, _LOG, "LOG"}, {
	localSYM, 0, "LOCAL"}, {
	tsrsSYM, _LOWER, "LOWER$"}, {
	loadSYM, 0, "LOAD"}, {
	tnrnSYM, _LOG10, "LOG10"}, {
	loopSYM, 0, "LOOP"}, {
	mkdirSYM, 0, "MKDIR"}, {
	modSYM, 0, "MOD"}, {
	moduleSYM, 0, "MODULE"}, {
	nameSYM, 0, "NAME"}, {
	newSYM, 0, "NEW"}, {
	tnrnSYM, _NOT, "NOT"}, {
	nullSYM, 0, "NULL"}, {
	ofSYM, 0, "OF"}, {
	openSYM, 0, "OPEN"}, {
	orSYM, 0, "OR"}, {
	orthenSYM, 0, "OR THEN"}, {
	tsrnSYM, _ORD, "ORD"}, {
	osSYM, 0, "OS"}, {
	otherwiseSYM, 0, "OTHERWISE"}, {
	pageSYM, 0, "PAGE"}, {
	osSYM, 0, "PASS"}, {
	rnSYM, _PI, "PI"}, {
	printSYM, 0, "PRINT"}, {
	procSYM, 0, "PROC"}, {
	quitSYM, 0, "QUIT"}, {
	quitSYM, 0, "BYE"}, {
	tnrnSYM, _RAD, "RAD"}, {
	randomizeSYM, 0, "RANDOMIZE"}, {
	randomSYM, 0, "RANDOM"}, {
	readSYM, 0, "READ"}, {
	read_onlySYM, 0, "READ ONLY"}, {
	refSYM, 0, "REF"}, {
	renumberSYM, 0, "RENUMBER"}, {
	renumberSYM, 0, "RENUM"}, {
	repeatSYM, 0, "REPEAT"}, {
	reportSYM, 0, "REPORT"}, {
	restoreSYM, 0, "RESTORE"}, {
	retrySYM, 0, "RETRY"}, {
	returnSYM, 0, "RETURN"}, {
	rmdirSYM, 0, "RMDIR"}, {
	tnrnSYM, _ROUND, "ROUND"}, {
	rndSYM, _RND , "RND"}, {
	runSYM, 0, "RUN"}, {
	saveSYM, 0, "SAVE"}, {
	select_outputSYM, 0, "SELECT OUTPUT"}, {
	select_inputSYM, 0, "SELECT INPUT"}, {
	scanSYM, 0, "SCAN"}, {
	staticSYM, 0, "STATIC"}, {
	sysSYM, 0, "SYS"}, {
	syssSYM, 0, "SYS$"}, {
	tnrsSYM, _SPC, "SPC$"}, {
	tnrnSYM, _SGN, "SGN"}, {
	tnrnSYM, _SIN, "SIN"}, {
	tnrnSYM, _SQR, "SQR"}, {
	stepSYM, 0, "STEP"}, {
	stopSYM, 0, "STOP"}, {
	tnrsSYM, _STR, "STR$"}, {
	tnrsSYM, _TAB, "TAB"}, {
	tnrnSYM, _TAN, "TAN"}, {
	thenSYM, 0, "THEN"}, {
	toSYM, 0, "TO"}, {
	traceSYM, 0, "TRACE"}, {
	trapSYM, 0, "TRAP"}, {
	rnSYM, _TRUE, "TRUE"}, {
	unitSYM, 0, "UNIT"}, {
	rsSYM, _UNIT, "UNIT$"}, {
	untilSYM, 0, "UNTIL"}, {
	tsrsSYM, _UPPER, "UPPER$"}, {
	useSYM, 0, "USE"}, {
	usingSYM, 0, "USING"}, {
	tsrnSYM, _VAL, "VAL"}, {
	whenSYM, 0, "WHEN"}, {
	whileSYM, 0, "WHILE"}, {
	writeSYM, 0, "WRITE"}, {
	zoneSYM, _ZONE, "ZONE"}, {
	0, 0, "<undefined>"}
};

PUBLIC int lex_string_flatten()
{
	PRIVATE char finalstring[255];

	char *src=yytext+1;	/* skip past initial " */
	char *dest=finalstring;

	while (*src) {
		if (*src == '\\' && *(src+1)) {
			src++;
			switch (*src) {
				case '"':
					*dest = '"';
					break;

				case 'r':
					*dest = '\r';
					break;

				case 'n':
					*dest = '\n';
					break;

				case 't':
					*dest = '\t';
					break;

				case '\b':
					*dest = (char) 7;
					break;

				default:
					*dest = *src;
				}

				src++;
		} else if (*src=='"')
			if (!*(src+1))
				break;
			else if (*(src+1)=='"') {
				src+=2;
				*dest='"';
			} else {
				src++;
				*dest='"';
			}
		else if (*src=='\r' || *src=='\n') {
			src++;
			dest--;	/* to compensate for the coming dest++ */
		} else  {
			*dest=*src;
			src++;
		}

		dest++;
	}

	*dest=0;

	yylval.str = str_make(PARSE_POOL, finalstring);

	return stringSYM;
}


PUBLIC int lex_floatnum()
{
	char *endptr;

	yylval.dubbel.val = strtod(yytext, &endptr);
	yylval.dubbel.text=yytext;

	if (*endptr)
		pars_error("Error converting numeric value %s", yytext);

	return floatnumSYM;
}


PUBLIC int lex_intnum()
{
	char *c = yytext;

	yylval.num = 0L;

	while (*c) {
		yylval.num = yylval.num * 10 + *c - '0';

		if (yylval.num < 0)
			return lex_floatnum();

		c++;
	}

	return intnumSYM;
}


PUBLIC int lex_id(int sym)
{
	int i;

	strupr(yytext);

	for (i = 0; lexemetab[i].sym; i++)
		if (strcmp(yytext, lexemetab[i].txt) == 0) {
			yylval.inum = lexemetab[i].func;
			return lexemetab[i].sym;
		}

	yylval.id = id_search(yytext);

	return sym;
}


PUBLIC int lex_rem()
{
	remove_trailing(yytext,"\r\n","\n");
	yylval.str = str_make(PARSE_POOL, yytext + 2);

	return remSYM;
}


PUBLIC const char *lex_sym(int sym)
{
	int i;

	for (i = 0; lexemetab[i].sym &&
	     lexemetab[i].sym != sym && lexemetab[i].func != sym; i++);

	return lexemetab[i].txt;
}


PUBLIC const char *lex_opsym(int sym)
{
	const char *s = lex_sym(sym);

	switch (sym) {
	case andSYM:
		return " AND ";
	case bitandSYM:
		return " BITAND ";
	case bitorSYM:
		return " BITOR ";
	case bitxorSYM:
		return " BITXOR ";
	case divSYM:
		return " DIV ";
	case eorSYM:
		return " EOR ";
	case inSYM:
		return " IN ";
	case modSYM:
		return " MOD ";
	case orSYM:
		return " OR ";
	case andthenSYM:
		return " AND THEN ";
	case orthenSYM:
		return " OR THEN ";
	}

	return s;
}
