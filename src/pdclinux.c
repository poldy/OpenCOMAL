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

#define _XOPEN_SOURCE_EXTENDED

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
#include <iconv.h>

#include <readline/readline.h>
#include <readline/history.h>

#define HALFDELAY 2

#define CHECK(fn, ...) { \
        if ((fn)(__VA_ARGS__) == ERR) { \
                errw(#fn); \
        } \
}

PRIVATE int escape = 0;

PRIVATE int paged = 0, pagern;
PRIVATE int getx,gety;
PRIVATE char *edit_line;
PRIVATE Keymap keymap;
PRIVATE iconv_t latin_to_utf8, utf8_to_latin;
PRIVATE bool is_visual_mode = false;

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

PRIVATE void errw(const char *msg)
{
        if (is_visual_mode) {
                endwin();
        }
        fprintf(stderr, "%s\n", msg);
        exit(EXIT_FAILURE);
}

PRIVATE void int_handler(int signum)
{
	escape = 1;
	signal(SIGINT, int_handler);
}

PRIVATE char wc_to_latin(wchar_t wc)
{
	char uc[MB_LEN_MAX], lc;
	int l;
	char *inbuf, *outbuf;
	size_t inbytesleft, outbytesleft;

	l = wctomb(uc, wc);
	inbuf = uc;
	inbytesleft = l;
	outbuf = &lc;
	outbytesleft = 1;
	iconv(utf8_to_latin, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
	return lc;
}
		
PRIVATE int my_getch_horse(wint_t *c, char *lc)
{
	int status;

	while (1) {
		status = get_wch(c);

                switch (status) {
                case KEY_CODE_YES:
		        if (*c == KEY_RESIZE) {
                                CHECK(refresh);
			        continue;
                        }
                        return KEY_CODE_YES;
                        break;
                case OK:
                        *lc = wc_to_latin(*c);
		        if (escape) {
			        return ERR;
                        } else if (*lc=='\f') {
                                CHECK(clearok, curscr, TRUE);
                                CHECK(refresh);
                                continue;
		        }
			return OK;
                        break;
                case ERR:
                        return ERR;
                        break;
                default:
                        return ERR;
                        break;
		}
	}

	return ERR;
}

PRIVATE int my_getch()
{
	wint_t c;
	int i;
	unsigned char lc;
        int status;

	while (1) {
		status = my_getch_horse(&c, (char *)&lc);

                switch (status) {
                case ERR:
			if (escape) {
				rl_done=1;
				return 10;
			}
			continue;
                        break;
                case OK:
		        if (lc==10 || lc==13)  {
			        return lc;
                        } else if (lc<' ') {
			        CHECK(beep);
			        continue;
		        }
			return lc;
                        break;
                case KEY_CODE_YES:
		        for (i=0; my_keymap[i].curses_key; i++) {
			        if (c==my_keymap[i].curses_key) {
				        return my_keymap[i].internal_key;
                                }
                        }
		        CHECK(beep);
                        return ERR;
                        break;
                default:
                        return ERR;
                        break;
                }
	}

        return ERR;
}


PRIVATE int curses_getc(FILE *in)
{
	return my_getch();
}

PRIVATE void addlstr(const char *lstr)
{
	char ustr[MAX_LINELEN];
	const char *inbuf;
	char *outbuf;
	size_t inbytesleft, outbytesleft;
	wchar_t wstr[MAX_LINELEN];

	if (lstr == NULL) {
		return;
	}
	inbuf = lstr;
	inbytesleft = strlen(lstr);
	outbuf = ustr;
	outbytesleft = MAX_LINELEN;
	iconv(latin_to_utf8, (char **)&inbuf, &inbytesleft, &outbuf, &outbytesleft);
	*outbuf = '\0';
	mbstowcs(wstr, ustr, MAX_LINELEN);
	CHECK(addwstr, wstr);
}

PRIVATE void curses_redisplay()
{
	CHECK(move,gety,getx);
	addlstr(rl_line_buffer);
	CHECK(clrtoeol);
	CHECK(move,gety+rl_point/COLS,getx+rl_point%COLS);
	CHECK(refresh);
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

	if (started_up) {
                return 0;
        }

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

PRIVATE void init_ncurses(void)
{
	if (initscr() == NULL) {
                errw("initscr");
        }
        is_visual_mode = true;
        if (has_colors()) {
                CHECK(start_color);
                CHECK(use_default_colors);
        }
	CHECK(scrollok, stdscr, TRUE);
        CHECK(idlok, stdscr, TRUE);
	CHECK(noecho);
	CHECK(keypad, stdscr, TRUE);
	CHECK(halfdelay, HALFDELAY);
        CHECK(nonl);
        CHECK(intrflush, NULL, FALSE);
        curs_set(2);
}

PRIVATE void init_readline(void)
{
	rl_prep_term_function=NULL;
	rl_deprep_term_function=NULL;
	rl_readline_name="OpenComal";
	rl_redisplay_function=curses_redisplay;
	rl_pre_input_hook=pre_input;
	rl_getc_function=curses_getc;
	rl_startup_hook=startup;
	rl_catch_signals=0;
	rl_catch_sigwinch=0;
        rl_change_environment=0;
}

PRIVATE void init_iconv(void)
{
	latin_to_utf8 = iconv_open("utf8", "latin-9");
	utf8_to_latin = iconv_open("latin-9", "utf8");
}

PRIVATE void screen_init(void)
{
        init_ncurses();
        init_readline();
        init_iconv();
}

PRIVATE void deinit_ncurses(void)
{
	CHECK(endwin);
        is_visual_mode = false;
}

PRIVATE void deinit_iconv(void)
{
	iconv_close(latin_to_utf8);
	iconv_close(utf8_to_latin);
}

PRIVATE void screen_tini(void)
{
        deinit_ncurses();
        deinit_iconv();
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

        CHECK(reset_shell_mode);
        CHECK(putp, exit_ca_mode);
	fflush(stdout);
	rc = system(cmd);
	fputs("\nPress return to continue...", stdout);
        fflush(stdout);
	while (getchar() != '\n')
		;
        CHECK(putp, enter_ca_mode);
        CHECK(reset_prog_mode);
        CHECK(clearok, curscr, TRUE);
        CHECK(refresh);

	return rc;
}

PUBLIC void sys_setpaged(int n)
{
	paged = n;
	pagern = LINES;
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
	if (stream == MSG_ERROR) {
		CHECK(attron, A_REVERSE);
        }

	addlstr(buf);

	if (stream == MSG_ERROR) {
		CHECK(attroff, A_REVERSE);
        }

        CHECK(refresh);
}

PUBLIC void sys_put(int stream, const char *buf, long len)
{
	int buf_lines;

	if (len < 0)
		len = strlen(buf);

	buf_lines = len / COLS;

	if ((len % COLS) > 0)
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
					pagern = LINES;
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
	CHECK(erase);
}


PUBLIC void sys_cursor(FILE * f, long y, long x)
{
	ext_cursor(y, x);
	if (y == 0) {
		y = sys_currow();
	}
	if (x == 0) {
		x = sys_curcol();
	}
	CHECK(move, y - 1, x - 1);
}


PUBLIC int sys_curcol(void)
{
	int y, x;

	getyx(stdscr, y, x);
	return x + 1;
}


PUBLIC int sys_currow(void)
{
	int y, x;

	getyx(stdscr, y, x);
	return y + 1;
}


PUBLIC void sys_nl(int stream)
{
	ext_nl();
	CHECK(addch, '\n');
}


PUBLIC int sys_yn(int stream, const char *prompt)
{
	do_put(stream, prompt, strlen(prompt));

	for (;;) {
		char c;

		c = my_getch();

		if (sys_escape() || c == 'n' || c == 'N') {
			CHECK(addstr, "No\n");
			return 0;
		} else if (c == 'y' || c == 'Y') {
			CHECK(addstr, "Yes\n");
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
	addlstr(prompt);
	getyx(stdscr,gety,getx);
	addlstr(line);
	CHECK(refresh);
	strncpy(line,readline(""),maxlen-1);
	line[maxlen-1] = '\0';
	CHECK(move,gety+rl_end/COLS,getx+rl_end%COLS);
	CHECK(addch, '\n');

	escape=sys_escape();

	if (!escape) {
                add_history(line);
        }

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
		if (!buf) {
                        buf=(char *)malloc(buf_size);
                }

		if (getcwd(buf,buf_size)!=NULL) {
                        return buf;
                }

		if (errno==ERANGE) { /* buffer too small */
			buf_size+=1024;
			free(buf);
			buf=0;
		} else {
			run_error(DIRS_ERR,strerror(errno));
                }
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

	if (!f) {
                run_error(DIRS_ERR,strerror(errno));
        }

	sys_setpaged(1);

	while (fgets(line,254,f)) {
		sys_put(MSG_PROGRAM,line,-1);
        }

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
	static char result[2] = {'\0','\0'};
	int status=ERR;
        wint_t c;

	/*
	 * -1 means neverending delay 
	 */
	if (delay<0) {
		while (status==ERR && !escape) {
			status=my_getch_horse(&c, result);
                }
	} else if (delay==0) {
		CHECK(raw);
		CHECK(nodelay,stdscr,TRUE);
		status=my_getch_horse(&c, result);
		CHECK(halfdelay, HALFDELAY);
	} else {
		CHECK(halfdelay, delay*10);
		status=my_getch_horse(&c, result);
		CHECK(halfdelay, HALFDELAY);
	}

	if (status == ERR) {
                *result='\0';
        }

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
