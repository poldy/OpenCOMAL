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
	pdcfree.c pdcid.c pdclexs.c pdcunix.c pdclist.c pdcmain.c pdcmem.c \
	pdcmisc.c pdcpars.tab.c pdcparss.c pdcprog.c pdcrun.c pdcscan.c \
	pdcseg.c pdcsqash.c pdcstr.c pdcsym.c pdcval.c pdcmod.c long.c
SOURCES_CII:=../cii/src/except.c ../cii/src/fmt.c

TARG1:=../bin/opencomal
OBJ1:=pdcpars.tab.o lex.yy.o pdcmain.o pdcmisc.o pdccmd.o  pdclexs.o  \
	pdcid.o  pdcscan.o pdcparss.o pdcenv.o pdcsym.o pdcexec.o pdclist.o \
	pdcfree.o pdcexp.o pdcmem.o pdcsqash.o pdcstr.o pdcprog.o pdcext.o \
	pdcseg.o pdcval.o pdccloop.o pdcmod.o long.o

TARG2:=../bin/opencomalrun
OBJ2:=pdcmain.o pdcmisc.o \
	pdcid.o  pdcscan.o pdcenv.o pdcsym.o pdcexec.o pdcfree.o \
	pdcexp.o pdcmem.o pdcsqash.o pdcstr.o pdcprog.o pdcext.o pdcseg.o \
	pdcval.o pdcrun.o pdcmod.o long.o
OS:=pdcunix.o 
OBJ_CII:=../cii/src/except.o ../cii/src/fmt.o

ifdef DEBUG
	SOURCES_CII+=../cii/src/memchk.c ../cii/src/assert.c
	OBJ_CII+=../cii/src/memchk.o ../cii/src/assert.o
	CFLAGS+=-fsanitize=undefined
	LDFLAGS+=-fsanitize=undefined
else
	SOURCES_CII+=../cii/src/mem.c
	OBJ_CII+=../cii/src/mem.o
endif

.PHONY: all
all: build $(TARG1) $(TARG2) ../bin/en.cat ../bin/ga.cat

# I recommend using c++ for compile-time warnings only, and building for release with the C compiler
include $(REALCC).mk
MKCATDEFS:=../tools/mkcatdefs
include $(OPSYS).mk
CFLAGS+=-I../cii/include -I.
MIN_CFLAGS:=$(CFLAGS)
CFLAGS+=$(TARG_CFLAGS)
SOURCE_DATE_EPOCH=$(shell git log -l --pretty=%ct)
DATE_FMT = %Y-%m-%d
ifdef SOURCE_DATE_EPOCH
	BUILD_DATE ?= $(shell date -u -d "@$(SOURCE_DATE_EPOCH)" "+$(DATE_FMT)"  2>/dev/null || date -u -r "$(SOURCE_DATE_EPOCH)" "+$(DATE_FMT)" 2>/dev/null || date -u "+$(DATE_FMT)")
else
	BUILD_DATE ?= $(shell date "+$(DATE_FMT)")
endif
CFLAGS+='-DBUILD_DATE="$(BUILD_DATE)"'

$(OBJ1) $(OBJ2) $(OS): pdcpars.tab.h

$(TARG1):  $(OBJ1) $(OS) $(OBJ_CII)
	$(CC) -o $(TARG1) $(OBJ1) $(OS) $(OBJ_CII) $(LDFLAGS) $(TARG_LDFLAGS)

$(TARG2):  $(OBJ2) $(OS) $(OBJ_CII)
	$(CC) -o $(TARG2) $(OBJ2) $(OS) $(OBJ_CII) $(LDFLAGS) $(TARG_LDFLAGS)

.PHONY: build
build:		
	../tools/bumpbuild BUILD
	../tools/genversion

YFLAGS:=-vd -b pdcpars
pdcpars.tab.c: pdcpars.y
	$(YACC) $(YFLAGS) pdcpars.y

pdcpars.tab.h: pdcpars.tab.c

LFLAGS:=-X
lex.yy.c: pdclex.l
	$(LEX) $(LFLAGS) pdclex.l

pdcpars.tab.o: msgnrs.h

.PHONY: almostclean
almostclean:	
	$(RM) *.o *.obj *.map *.OBJ *~ *.d $(OBJ_CII)
	$(RM) lex.yy.c pdcpars.tab.* pdcpars.output
	$(RM) pdcpars.c pdclex.c pdcpars.h

.PHONY: clean
clean: almostclean
	$(RM) $(TARG1) $(TARG2)
	$(RM) ../bin/*.exe ../bin/*.EXE ../bin/*.cat
	$(RM) $(MKCATDEFS)

DEPS		:= $(SOURCES:%.c=%.o.d)
-include	$(DEPS)
