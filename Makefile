.POSIX:
.DELETE_ON_ERROR:

# This Makefile is for people who just type make whenever
# they see a Makefile somewhere... :-)
#
# Reading the README is a better way

PREFIX?=/usr/local
PARALLEL?=-j -l 2.5
REALCC?=gcc
OPSYS?=linux

.PHONY: top
top: msg all

.PHONY: msg
msg:	
	@echo
	@echo Aha, you are one of these persons that types make
	@echo whenever they see a Makefile somewhere....
	@echo

.PHONY: all
all:
	+@cd src; $(MAKE) $(PARALLEL) DEBUG=$(DEBUG) OPSYS=$(OPSYS) REALCC=$(REALCC)

.PHONY: install
install:
	cd bin ; $(MAKE) install PREFIX=$(PREFIX) OPSYS=$(OPSYS) REALCC=$(REALCC)

.PHONY: clean
clean:
	cd src; $(MAKE) clean OPSYS=$(OPSYS) REALCC=$(REALCC)

.PHONY: tar
tar: src
	cd src; $(MAKE) almostclean OPSYS=$(OPSYS) REALCC=$(REALCC)
	-cd bin; strip opencomal opencomalrun
	tools/gentar
