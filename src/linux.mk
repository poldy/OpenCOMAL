-include /usr/share/hardening-includes/hardening.make
CFLAGS += $(HARDENING_CFLAGS)
LDFLAGS += $(HARDENING_LDFLAGS)
TARG_CFLAGS:=$(shell ncursesw5-config --cflags)
TARG_LDFLAGS:=-lreadline -lm $(shell ncursesw5-config --libs)
