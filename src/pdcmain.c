/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* Main file of OpenComal */

#define EXTERN

#include "pdcglob.h"
#include "pdcenv.h"
#include "pdccloop.h"
#include "pdcexp.h"
#include "pdcmisc.h"
#include "version.h"

#include <string.h>
#include <unistd.h>

#define LOCNAME_MAX 32

extern int yydebug;

#ifdef TURBOC
unsigned _stklen = 32768;
#endif

PUBLIC int main(int argc, char *argv[])
{
        int c;
        int errflg = 0;
        char *locname, nlocname[LOCNAME_MAX];
        int utf8_suffixlen, lang_countrylen;
        const char *utf8_suffix = ".utf8", *latin_suffix = "@euro";
        locale_t loc;

        if ((locname = setlocale(LC_ALL, "")) == NULL) {
		perror("setlocale");
		return EXIT_FAILURE;
	}
        utf8_suffixlen = strlen(utf8_suffix);
        lang_countrylen = strlen(locname) - utf8_suffixlen;
        if (strncmp(locname + lang_countrylen, utf8_suffix, utf8_suffixlen) == 0) {
                strncpy(nlocname, locname, lang_countrylen);
                nlocname[lang_countrylen] = '\0';
                strncat(nlocname, latin_suffix, LOCNAME_MAX - lang_countrylen - 1);
                loc = duplocale(LC_GLOBAL_LOCALE);
                if (loc == (locale_t)0) {
                        perror("duplocale");
                        return EXIT_FAILURE;
                }
                latin_loc = newlocale(LC_ALL_MASK, nlocname, loc);
                if (latin_loc == (locale_t)0) {
                        perror("newlocale");
                        return EXIT_FAILURE;
                }
        }

#ifdef NDEBUG
        while ((c = getopt(argc, argv, "y")) != -1) {
                switch (c) {
#else
        while ((c = getopt(argc, argv, "dy")) != -1) {
                switch (c) {
                case 'd':
                        comal_debug++;
                        break;
#endif
                case 'y':
                        yydebug++;
                        break;
                case '?':
                        fprintf(stderr, "Unrecognised option: '-%c'\n", optopt);
                        errflg++;
                }
        }
        if (errflg) {
#ifdef NDEBUG
                fprintf(stderr, "usage: %s [-y] ...\n", argv[0]);
#else
                fprintf(stderr, "usage: %s [-dy] ...\n", argv[0]);
#endif
                return EXIT_FAILURE;
        }

	sys_init();
	copyright = "(c) Copyright 1992-2002  Jos Visser <josv@osp.nl>";

#if defined(OS_WIN32) || defined(OS_MSDOS)
	my_printf(MSG_DIALOG, 1,
		  "OpenComal -- A free Comal implementation (version %s; %s)",
		  OPENCOMAL_VERSION,HOST_OS);
#else
	my_printf(MSG_DIALOG, 1,
		  "OpenComal -- A free Comal implementation (version %s; %s; build %s)",
		  OPENCOMAL_VERSION,HOST_OS,OPENCOMAL_BUILD);
#endif

	my_printf(MSG_DIALOG, 1, "             %s", copyright);
	my_printf(MSG_DIALOG, 1, "             Built on %s at approximately %s", __DATE__, __TIME__);
	my_nl(MSG_DIALOG);
	my_printf(MSG_DIALOG, 1,"OpenComal is licensed under the GNU General Public License (GPL) version 2");
	my_printf(MSG_DIALOG, 1,"(The GPL contains a very nice statement on WARRANTY; you might want to read it)");
	my_nl(MSG_DIALOG);

	mem_init();

	runfilename = NULL;
	curenv = env_new("nirvana");
	entering = 0;
	sel_infile = NULL;
	sel_outfile = NULL;

	pdc_go(argc - optind + 1, &(argv[optind - 1]));

	if (setjmp(RESTART) == 0) {
		if (sel_infile)
			if (fclose(sel_infile))
				perror
				    ("Error when closing SELECT INPUT file");

		if (sel_outfile)
			if (fclose(sel_outfile))
				perror
				    ("Error when closing SELECT OUTPUT file");

		clear_env(curenv);
		mem_tini();
	}

	sys_tini();
        freelocale(latin_loc);

	return 0;
}
