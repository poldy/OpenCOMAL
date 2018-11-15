/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/** @file
 * OpenComal Comal error codes
 */

#ifndef PDCERR_H
#define PDCERR_H

/*
 * Run error codes 
 */

#define NO_RUN_ERR	0
#define LABEL_ERR	1       /**< Label not found */
#define DATA_ERR	2       /**< No DATA statements found */
#define DEL_ERR		3       /**< Delete "file" failed */
#define NORETURN_ERR	4       /**< ENDFUNC without RETURN */
#define DIRECT_ERR	5       /**< Error when executing simple_stat in direct mode */
#define F2INT1_ERR	6       /**< floating point too large to convert to int */
#define F2INT2_ERR	7       /**< floating point contains frac part */
#define VAL_ERR		8       /**< VAL() failed */
#define CHR_ERR		9       /**< CHR$() of <0 || >255 */
#define DIV0_ERR	10      /**< Division by 0 */
#define OS_ERR		12      /**< OS command error */
#define NIMP_ERR	13      /**< Not yet implemented error */
#define VAR_ERR		14      /**< Variable exists already */
#define DIM_ERR		15      /**< Top dimension larger then bottom dimension */
#define ARRAY_ERR	16      /**< Various errors with array indices */
#define SUBSTR_ERR	17      /**< Substring specifier out of bounds */
#define FOR_ERR		18      /**< Error in FOR/ENDFOR loop */
#define UNFUNC_ERR	19      /**< Unknown function */
#define PARM_ERR	20      /**< Parameter error in PROC/FUNC call */
#define VALUE_ERR	21      /**< Error copying values */
#define OPEN_ERR	22      /**< File open error */
#define CLOSE_ERR	23      /**< File close error */
#define POS_ERR		24      /**< File positioning error */
#define WRITE_ERR	25      /**< File WRITE error */
#define EOF_ERR		26      /**< Error when checking for EOF */
#define EOD_ERR		27      /**< EOD at READ */
#define TYPE_ERR	28      /**< Type mixup with read & input etc. */
#define READ_ERR	29      /**< I/O error at file READ */
#define IMPORT_ERR	30      /**< IMPORT error */
#define NAME_ERR	31      /**< Error in processing a NAME */
#define ESCAPE_ERR	32      /**< Escape Pressed @ INPUT in direct mode */
#define MATH_ERR	33      /**< Mathematics Error */
#define MEM_ERR		34      /**< Memory Error */
#define SELECT_ERR	35      /**< Error in select input/output */
#define USING_ERR	36      /**< USING string format error */
#define INPUT_ERR	37      /**< Error in input */
#define SQASH_ERR	38      /**< Error at save/load (Sqash) */
#define SYS_ERR		39      /**< Error at SYS, SYS() or SYS$() */
#define CMD_ERR		40      /**< Error in OpenComal command */
#define RUN_ERR		41      /**< Error when executing RUN "filename" */
#define CURSOR_ERR	42      /**< Error in CURSOR statement */
#define ID_ERR		43      /**< Error in expression (exp_id()) */
#define LVAL_ERR	44      /**< Expression is not an lvalue */
#define SCAN_ERR	45      /**< Error in SCAN of external segment */
#define EXT_ERR		46      /**< Error in call of external proc/func */
#define EXT2_ERR	47      /**< Error in call of extension proc/func */
#define DIRS_ERR	48      /**< Error determining current working dir */
#define SPC_ERR		49      /**< Error in parameter to SPC$ */
#define DIR_ERR		50      /**< Error in MKDIR, CHDIR of RMDIR */
#define RND_ERR		51      /**< Error in the arguments for RND */

#endif
