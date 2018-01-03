.POSIX:
.DELETE_ON_ERROR:

.PHONY: install
install:
	strip opencomal opencomalrun
	../build/install opencomal -d $(PREFIX)/bin
	../build/install opencomalrun -d $(PREFIX)/bin
	cp en.cat opencomal.cat
	../build/install opencomal.cat -d $(PREFIX)/locale/en
	cp ga.cat opencomal.cat
	../build/install opencomal.cat -d $(PREFIX)/locale/ga_IE.utf8
	$(RM) opencomal.cat
