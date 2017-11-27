/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal SYS routines for LINUX */

#include "pdcglob.h"
#include "pdcmisc.h"
#include "pdcext.h"
#include "pdcexec.h"
#include "pdcsys.h"

#include <signal.h>
#include <string.h>
#include <curses.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <term.h>

#include <readline/readline.h>
#include <readline/history.h>

#define HALFDELAY 2

PRIVATE int escape = 0;

PRIVATE int width, height;
PRIVATE int paged = 0, pagern;
PRIVATE int getx,gety;
PRIVATE char *edit_line;
PRIVATE Keymap keymap;

PRIVATE struct {
	int curses_key;
	int internal_key;
	const char *function;
} my_keymap[] = {
	{	KEY_HOME,	1,	"beginning-of-line"		},
	{	KEY_END,	2,	"end-of-line"			},
	{	KEY_RIGHT,	3,	"forward-char"			},
	{	KEY_LEFT,	4,	"backward-char"			},
	{	KEY_UP,		5,	"previous-history"		},
	{	KEY_DOWN,	6,	"next-history"			},
	{	KEY_DC,		7,	"delete-char"			},
	{	KEY_BACKSPACE,	8,	"backward-delete-char"		},
	//{	KEY_IC,		9,	"overwrite-mode"		},
	{	10,		10,	"accept-line"			},
	{	13,		13,	"accept-line"			},
	{	0,		0,	NULL				}
	
};

PRIVATE void int_handler(int signum)
{
	escape = 1;
	signal(SIGINT, int_handler);
}

PRIVATE int my_getch_horse()
{
	int c;

	while (1) {
		c = getch();

		if (c == KEY_RESIZE) {
			getmaxyx(stdscr, height, width);
			continue;
		} else if (escape)
			return ERR;
		else
			break;
	}

	return c;
}
		
PRIVATE int my_getch()
{
	int c;
	int i;

	while (1) {
		c = my_getch_horse();

		if (c==ERR) {
			if (escape) {
				rl_done=1;
				return 10;
			} else
				continue;
		} else if (c==10 || c==13) 
			break;
		else if (c<' ') {
			beep();
			continue;
		} else if (c<256) 
			break;

		for (i=0; my_keymap[i].curses_key; i++)
			if (c==my_keymap[i].curses_key)
				return my_keymap[i].internal_key;

		beep();
	}

	return c;
}


PRIVATE int curses_getc(FILE *in)
{
	return my_getch();
}

PRIVATE void curses_redisplay()
{
	move(gety,getx);
	addstr(rl_line_buffer);
	clrtoeol();
	move(gety+rl_point/width,getx+rl_point%width);
	refresh();
}

PRIVATE void curses_prep(int meta_flag)
{
}

PRIVATE void curses_deprep()
{
}

PRIVATE int pre_input()
{
	rl_insert_text(edit_line);

	return 0;
}

PRIVATE int startup()
{
	int i;
	static int started_up=0;

	if (started_up) return 0;

	keymap=rl_make_keymap();
	rl_set_keymap(keymap);

	for (i=0; my_keymap[i].curses_key; i++) {
		rl_command_func_t *func;

		func=rl_named_function(my_keymap[i].function);

		if (func)
			rl_bind_key(my_keymap[i].internal_key,func);
		else
			printw("\nCan not map function: %s\n",my_keymap[i].function);
	}

	started_up=1;

	return 0;
}

PRIVATE void screen_init()
{
	initscr();
	scrollok(stdscr, TRUE);
	noecho();
	keypad(stdscr, TRUE);
	halfdelay(HALFDELAY);
	refresh();
	getmaxyx(stdscr, height, width);

	/*
	 * Readline initialization
	 */
	rl_prep_term_function=curses_prep;
	rl_deprep_term_function=curses_deprep;
	rl_readline_name="OpenComal";
	rl_redisplay_function=curses_redisplay;
	rl_pre_input_hook=pre_input;
	rl_getc_function=curses_getc;
	rl_startup_hook=startup;
	rl_catch_signals=0;
	rl_catch_sigwinch=0;
}

PRIVATE void screen_tini()
{
	endwin();
}

PUBLIC void sys_init()
{
	ext_init();
	signal(SIGINT, int_handler);
	screen_init();
}


PUBLIC void sys_tini()
{
	ext_tini();
	screen_tini();
}

PUBLIC int sys_system(char *cmd)
{
	int rc;

        reset_shell_mode();
        putp(exit_ca_mode);
	fflush(stdout);
	rc = system(cmd);
	fputs("\nPress return to continue...", stdout);
        fflush(stdout);
	while (getchar() != '\n');
        putp(enter_ca_mode);
        reset_prog_mode();
        refresh();

	return rc;
}

PUBLIC void sys_setpaged(int n)
{
	paged = n;
	pagern = height;
}


PUBLIC void  sys_rand(long *result, long *scale)
{
	*result = rand();
	*scale=RAND_MAX;
}


PUBLIC int sys_escape()
{
	if (escape) {
		escape = 0;

		return 1;
	}

	return 0;
}

PRIVATE void do_put(int stream, const char *buf, long len)
{
	if (stream == MSG_ERROR)
		attron(A_REVERSE);

	addstr(buf);

	if (stream == MSG_ERROR)
		attroff(A_REVERSE);

	refresh();
}

PUBLIC void sys_put(int stream, const char *buf, long len)
{
	int buf_lines;

	if (len < 0)
		len = strlen(buf);

	buf_lines = len / width;

	if ((len % width) > 0)
		buf_lines++;

	ext_put(stream, buf, len);
	do_put(stream, buf, len);

	if (paged) {
		pagern -= buf_lines;

		if (pagern <= 0) {

			while (true) {
				int c;

				c = my_getch();

				if (c == ' ') {
					pagern = height;
					break;
				} else if (c == '\n') {
					pagern--;
					break;
				} else if (c == 'q') {
					escape = 1;
					paged = 0;
					break;
				}
			}
		}
	}
}


PUBLIC void sys_page(FILE * f)
{
	ext_page();
	erase();
}


PUBLIC void sys_cursor(FILE * f, long x, long y)
{
	ext_cursor(x, y);
	move(y, x);
	refresh();
}


PUBLIC void sys_nl(int stream)
{
	ext_nl();
	addch('\n');
	refresh();
}


PUBLIC void sys_screen_readjust()
{
}


PUBLIC int sys_yn(int stream, const char *prompt)
{
	do_put(stream, prompt, strlen(prompt));

	for (;;) {
		char c;

		c = my_getch();

		if (sys_escape() || c == 'n' || c == 'N') {
			addstr("No\n");
			return 0;
		} else if (c == 'y' || c == 'Y') {
			addstr("Yes\n");
			return 1;
		}
	}
}


PRIVATE int do_get(int stream, char *line, int maxlen, const char *prompt,
		   int cursor)
{
	int escape=0;

	rl_num_chars_to_read=maxlen-1;
	edit_line=line;
	addstr(prompt);
	getyx(stdscr,gety,getx);
	addstr(line);
	refresh();
	strncpy(line,readline(""),maxlen-1);
	line[maxlen-1] = '\0';
	move(gety+rl_end/width,getx+rl_end%width);
	addch('\n');

	escape=sys_escape();
	refresh();

	if (!escape) add_history(line);

	return escape;
}


PUBLIC int sys_get(int stream, char *line, int maxlen, const char *prompt)
{
	if (ext_get(stream, line, maxlen, prompt))
		return 0;
	else
		return do_get(stream, line, maxlen, prompt, 1);
}


PUBLIC int sys_edit(int stream, char line[], int maxlen, int cursor)
{
	return do_get(stream, line, maxlen, NULL, cursor);
}


PUBLIC int sys_call_scan(struct id_rec *id, struct exp_list *exproot,
			 char *errtext)
{
	int rc = ext_call_scan(id, exproot, errtext);

	if (rc == -1) {
		sprintf(errtext, "PROCedure %s not found", id->name);
		return 0;
	}

	return rc;
}

PUBLIC char *sys_dir_string() 
{
	static int buf_size=1024;
	static char *buf=0;

	while (true) {
		if (!buf) buf=(char *)malloc(buf_size);

		if (getcwd(buf,buf_size)!=NULL) return buf;

		if (errno==ERANGE) { /* buffer too small */
			buf_size+=1024;
			free(buf);
			buf=0;
		} else
			run_error(DIRS_ERR,strerror(errno));
	}
}

PUBLIC void sys_dir(const char *pattern) {
	FILE *f;
	int l=strlen(pattern);
	char *buf=(char *)malloc(8+l);
	char line[256];
	
	strncpy(buf,"ls -l ",7);
	buf[7] = '\0';
	strncat(buf,pattern,l);
	f=popen(buf,"r");

	if (!f) run_error(DIRS_ERR,strerror(errno));

	sys_setpaged(1);

	while (fgets(line,254,f))
		sys_put(MSG_PROGRAM,line,-1);

	sys_setpaged(0);
	pclose(f);
	free(buf);
}

PUBLIC const char *sys_unit_string() 
{
	return "C:"; /* :-) */
}

PUBLIC void sys_unit(char *unit)
{
}

PUBLIC void sys_chdir(char *dir)
{
	if (chdir(dir)<0)
		run_error(DIR_ERR,strerror(errno));
}

PUBLIC void sys_mkdir(char *dir)
{
	if (mkdir(dir,0777)<0)
		run_error(DIR_ERR,strerror(errno));
} 

PUBLIC void sys_rmdir(char *dir)
{
	if (rmdir(dir)<0)
		run_error(DIR_ERR,strerror(errno));
}

PUBLIC char *sys_key(long delay)
{
	static char result[2] = {0,0};
	int c=ERR;

	/*
	 * -1 means neverending delay 
	 */
	if (delay<0) {
		while (c==ERR && !escape) 
			c=my_getch_horse();
	} else if (delay==0) {
		raw();
		nodelay(stdscr,TRUE);
		c=my_getch_horse();
		halfdelay(HALFDELAY);
	} else {
		halfdelay(delay*10);
		c=my_getch_horse();
		halfdelay(HALFDELAY);
	}

	if (c<0) c=0;

	*result=c;
	return result;
}


PUBLIC int sys_call(struct id_rec *id, struct exp_list *exproot, int
		    calltype, void **result, enum VAL_TYPE *type)
{
	return ext_call(id, exproot, calltype, result, type);
}


PUBLIC void sys_sys_exp(struct exp_list *exproot, void **result, enum
			VAL_TYPE *type)
{
        char *cmd;

        cmd = exp_cmd(exproot->exp);

        if (strcmp(cmd, "sbrk") == 0) {
                if (exproot->next)
                        run_error(SYS_ERR,
                                  "SYS(sbrk) takes no further parameters");

                *result = cell_alloc(INT_CPOOL);
		**( (long **) result )=(long)sbrk(0);
                *type = V_INT;
	} else if (strcmp(cmd, "now") == 0) {
                if (exproot->next)
                        run_error(SYS_ERR,
                                  "SYS(now) takes no further parameters");

                *result = cell_alloc(INT_CPOOL);
		**( (long **) result )=(long)time(0);
                *type = V_INT;
        } else if (ext_sys_exp(exproot, result, type) == -1)
		run_error(SYS_ERR, "Unknown SYS() command");
}


PUBLIC void sys_syss_exp(struct exp_list *exproot, struct string **result, enum
			 VAL_TYPE *type)
{
	if (ext_syss_exp(exproot, result, type) == -1)
		run_error(SYS_ERR, "Unknown SYS$() command");
}


PUBLIC int sys_sys_stat(struct exp_list *exproot)
{
	int rc = ext_sys_stat(exproot);

	if (rc == -1)
		run_error(SYS_ERR, "Unknown SYS statement command");

	return rc;
}
