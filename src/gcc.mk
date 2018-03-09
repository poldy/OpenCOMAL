CC:=../build/ccd-gcc
CFLAGS+=-Wall -Wextra -D_FORTIFY_SOURCE=2
ifdef DEBUG
	CFLAGS+=-O0 -ggdb
	#LDFLAGS+=-lefence
else
	CFLAGS+=-DNDEBUG -Os
	LDFLAGS+=-Os
	ifeq ($(OPSYS),macos)
		CFLAGS+=-flto
		LDFLAGS+=-flto
	else
		CFLAGS+=-flto=jobserver
		LDFLAGS+=-flto=jobserver
	endif
endif
ifeq ($(REALCC),gcc)
	CFLAGS+=-std=c11
endif
