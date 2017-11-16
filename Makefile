# This Makefile is for people who just type make whenever
# they see a Makefile somewhere... :-)
#
# Reading the README is a better way

top:	msg all

msg:	
	@echo
	@echo Aha, you are one of these persons that types make
	@echo whenever they see a Makefile somewhere....
	@echo

all:
	@cd src; make

install:
	cd bin ; make install

clean:
	cd src; make clean

tar:	src
	cd src; make almostclean
	-cd bin; strip opencomal opencomalrun
	tools/gentar
