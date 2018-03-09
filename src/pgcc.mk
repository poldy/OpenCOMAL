ifdef DEBUG
$(error DEBUG not supported on pgcc)
endif
CC:=$(REALCC)
CFLAGS+=-D_FORTIFY_SOURCE=2 -c11 -Xc
CFLAGS+=-DNDEBUG -fast -Mipa=fast,inline
LDFLAGS+=-fast -Mipa=fast,inline
