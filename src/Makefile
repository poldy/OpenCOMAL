.POSIX:
.DELETE_ON_ERROR:

#
# OpenComal -- a free Comal implementation
#
# This file is part of the OpenComal package.
# (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
#
# The OpenComal package is covered by the GNU General Public
# License. See doc/LICENSE for more information.
#

SOURCES:=lex.yy.c pdccloop.c pdccmd.c pdcenv.c pdcexec.c pdcexp.c pdcext.c \
	pdcfree.c pdcid.c pdclexs.c pdclinux.c pdclist.c pdcmain.c pdcmem.c \
	pdcmisc.c pdcpars.tab.c pdcparss.c pdcprog.c pdcrun.c pdcscan.c \
	pdcseg.c pdcsqash.c pdcstr.c pdcsym.c pdcval.c pdcmod.c

TARG1:=../bin/opencomal
OBJ1:=pdcpars.tab.o lex.yy.o pdcmain.o pdcmisc.o pdccmd.o  pdclexs.o  \
	pdcid.o  pdcscan.o pdcparss.o pdcenv.o pdcsym.o pdcexec.o pdclist.o \
	pdcfree.o pdcexp.o pdcmem.o pdcsqash.o pdcstr.o pdcprog.o pdcext.o \
	pdcseg.o pdcval.o pdccloop.o pdcmod.o

TARG2:=../bin/opencomalrun
OBJ2:=pdcmain.o pdcmisc.o \
	pdcid.o  pdcscan.o pdcenv.o pdcsym.o pdcexec.o pdcfree.o \
	pdcexp.o pdcmem.o pdcsqash.o pdcstr.o pdcprog.o pdcext.o pdcseg.o \
	pdcval.o pdcrun.o pdcmod.o
OS:=pdclinux.o 

# I recommend using c++ for compile-time warnings only, and building for release with the C compiler
CC:=../build/ccd-gcc
CFLAGS+=-Wall -Wextra -DOS_LINUX -D_FORTIFY_SOURCE=2
MKCATDEFS:=../tools/mkcatdefs
ifdef DEBUG
	CFLAGS+=-O0 -ggdb
else
	CFLAGS+=-DNDEBUG -Os -flto
	LDFLAGS+=-Os -flto
endif
ifeq ($(REALCC),c++)
	CFLAGS+=-Weffc++ -std=c++14
else
	CFLAGS+=-std=gnu11
endif
ifneq ($(OPSYS),macos)
	-include /usr/share/hardening-includes/hardening.make
	CFLAGS += $(HARDENING_CFLAGS)
	LDFLAGS += $(HARDENING_LDFLAGS)
endif
MIN_CFLAGS:=$(CFLAGS)
ifeq ($(OPSYS),macos)
	CFLAGS+=-I/usr/local/opt/readline/include
	TARG_LDFLAGS:=-L/usr/local/opt/readline/lib -lreadline -lncurses -liconv
else
	CFLAGS+=$(shell ncursesw5-config --cflags)
	TARG_LDFLAGS:=-lreadline -lm $(shell ncursesw5-config --libs)
endif

.PHONY: all
all: build $(TARG1) $(TARG2) ../bin/en.cat ../bin/ga.cat

$(TARG1):  $(OBJ1) $(OS)
	$(CC) -o $(TARG1) $(OBJ1) $(OS) $(LDFLAGS) $(TARG_LDFLAGS)

$(TARG2):  $(OBJ2) $(OS)
	$(CC) -o $(TARG2) $(OBJ2) $(OS) $(LDFLAGS) $(TARG_LDFLAGS)

.PHONY: build
build:		
	../tools/bumpbuild BUILD
	../tools/genversion

pdcpars.tab.c: pdcpars.y
	bison -vd pdcpars.y

lex.yy.c: pdclex.l
	flex pdclex.l

ifeq ($(OPSYS),macos)
../bin/%.cat: %.tmp
	LANG=en_GB.ISO8859-15 gencat $@ $^

%.tmp: %.msg
	LANG=en_GB.ISO8859-15 $(MKCATDEFS) msgnrs.h $(filter-out $(MKCATDEFS),$^) > $@

mkcatdefs.o: ../tools/mkcatdefs.c
	$(CC) $(MIN_CFLAGS) -c $^

$(MKCATDEFS): mkcatdefs.o
	$(CC) $(LDFLAGS) -o $@ $^
	strip $@

msgnrs.h: en.tmp

en.tmp: $(MKCATDEFS)
else
../bin/%.cat: %.msg
	LANG=ga_IE@euro gencat -H msgnrs.h -o $@ $^

msgnrs.h: ../bin/en.cat
endif

pdcmain.o: msgnrs.h

.PHONY: almostclean
almostclean:	
	$(RM) *.o *.obj *.map *.OBJ *~ *.d
	$(RM) lex.yy.c pdcpars.tab.* pdcpars.output
	$(RM) pdcpars.c pdclex.c pdcpars.h

.PHONY: clean
clean: almostclean
	$(RM) $(TARG1) $(TARG2)
	$(RM) ../bin/*.exe ../bin/*.EXE
	$(RM) $(MKCATDEFS)

DEPS		:= $(SOURCES:%.c=%.o.d)
-include	$(DEPS)
