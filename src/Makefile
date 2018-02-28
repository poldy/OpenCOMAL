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
ifeq ($(REALCC),pgcc)
	CC:=$(REALCC)
else
	CC:=../build/ccd-gcc
endif
ifeq ($(REALCC),pgcc)
	CFLAGS+=-D_FORTIFY_SOURCE=2
else
	CFLAGS+=-Wall -Wextra -D_FORTIFY_SOURCE=2
endif
MKCATDEFS:=../tools/mkcatdefs
ifdef DEBUG
	CFLAGS+=-O0 -ggdb
	#LDFLAGS+=-lefence
else
	ifeq ($(REALCC),pgcc)
		CFLAGS+=-DNDEBUG -fast -Mipa=fast,inline
		LDFLAGS+=-fast -Mipa=fast,inline
	else
		CFLAGS+=-DNDEBUG -Os -flto=jobserver
		LDFLAGS+=-Os -flto=jobserver
	endif
endif
ifeq ($(REALCC),c++)
	CFLAGS+=-Weffc++ -std=c++14
else
	ifeq ($(REALCC),pgcc)
		CFLAGS+=-c11 -Xc
	else
		CFLAGS+=-std=c11
	endif
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
SOURCE_DATE_EPOCH=$(shell git log -l --pretty=%ct)
DATE_FMT = %Y-%m-%d
ifdef SOURCE_DATE_EPOCH
	BUILD_DATE ?= $(shell date -u -d "@$(SOURCE_DATE_EPOCH)" "+$(DATE_FMT)"  2>/dev/null || date -u -r "$(SOURCE_DATE_EPOCH)" "+$(DATE_FMT)" 2>/dev/null || date -u "+$(DATE_FMT)")
else
	BUILD_DATE ?= $(shell date "+$(DATE_FMT)")
endif
CFLAGS+='-DBUILD_DATE="$(BUILD_DATE)"'

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

pdcpars.tab.o: msgnrs.h

.PHONY: almostclean
almostclean:	
	$(RM) *.o *.obj *.map *.OBJ *~ *.d
	$(RM) lex.yy.c pdcpars.tab.* pdcpars.output
	$(RM) pdcpars.c pdclex.c pdcpars.h

.PHONY: clean
clean: almostclean
	$(RM) $(TARG1) $(TARG2)
	$(RM) ../bin/*.exe ../bin/*.EXE ../bin/*.cat
	$(RM) $(MKCATDEFS)

DEPS		:= $(SOURCES:%.c=%.o.d)
-include	$(DEPS)
