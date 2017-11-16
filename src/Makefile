#
# OpenComal -- a free Comal implementation
#
# This file is part of the OpenComal package.
# (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
#
# The OpenComal package is covered by the GNU General Public
# License. See doc/LICENSE for more information.
#

SOURCES=lex.yy.c pdccloop.c pdccmd.c pdcenv.c pdcexec.c pdcexp.c pdcext.c \
	pdcfree.c pdcid.c pdclexs.c pdclinux.c pdclist.c pdcmain.c pdcmem.c \
	pdcmisc.c pdcpars.tab.c pdcparss.c pdcprog.c pdcrun.c pdcscan.c \
	pdcseg.c pdcsqash.c pdcstr.c pdcsym.c pdcval.c pdcmod.c

TARG1=../bin/opencomal
OBJ1=pdcpars.tab.o lex.yy.o pdcmain.o pdcmisc.o pdccmd.o  pdclexs.o  \
	pdcid.o  pdcscan.o pdcparss.o pdcenv.o pdcsym.o pdcexec.o pdclist.o \
	pdcfree.o pdcexp.o pdcmem.o pdcsqash.o pdcstr.o pdcprog.o pdcext.o \
	pdcseg.o pdcval.o pdccloop.o pdcmod.o

TARG2=../bin/opencomalrun
OBJ2=pdcmain.o pdcmisc.o \
	pdcid.o  pdcscan.o pdcenv.o pdcsym.o pdcexec.o pdcfree.o \
	pdcexp.o pdcmem.o pdcsqash.o pdcstr.o pdcprog.o pdcext.o pdcseg.o \
	pdcval.o pdcrun.o pdcmod.o
OS=pdclinux.o 
OSLIB=-lncurses -L/usr/local/opt/readline/lib -lreadline
STATIC=
#PROFILE=-pg
PROFILE=

# I recommend using c++ for compile-time warnings only, and building for release with the C compiler
ifeq ($(CC),c++)
COPTS=-c -Wall -Wextra -Weffc++ -std=c++14 -DOS_LINUX $(PROFILE) -flto
else
COPTS=-c -Wall -Wextra -std=gnu11 -DOS_LINUX $(PROFILE) -flto -D_FORTIFY_SOURCE=2
endif
ifeq ($(OPSYS),macos)
COPTS+=-Oz -I/usr/local/opt/readline/include
else
COPTS+=-Os -fomit-frame-pointer
endif
LIBS=$(OSLIB)
ifneq ($(OPSYS),macos)
LIBS+=-lm
endif

LDOPTS=$(STATIC) $(LIBS) -flto

%.d: %.c
		set -e; $(CC) -M $(CPPFLAGS) $< \
			| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
			[ -s $@ ] || rm -f $@

all:		build $(TARG1) $(TARG2)

.depend:	
		makedepend -f .depend *.c *.y *.l

$(TARG1):	 $(OBJ1) $(OS)
		$(CC) $(PROFILE) -o $(TARG1) $(OBJ1) $(OS) $(LDOPTS)

$(TARG2):	 $(OBJ2) $(OS)
		$(CC) $(PROFILE) -o $(TARG2) $(OBJ2) $(OS) $(LDOPTS)

build:		
		../tools/bumpbuild BUILD
		../tools/genversion

pdcpars.tab.c:	pdcpars.y
		bison -vd pdcpars.y

lex.yy.c:	pdclex.l
		flex pdclex.l

almostclean:	
		-rm *.o *.obj *.map *.OBJ *~ *.d
		-rm lex.yy.c pdcpars.tab.* pdcpars.output
		-rm pdcpars.c pdclex.c pdcpars.h

clean:		almostclean
		-rm $(TARG1) $(TARG2)
		-rm ../bin/*.exe ../bin/*.EXE

.c.o:
		$(CC) $(COPTS) $*.c 

include $(SOURCES:.c=.d)

