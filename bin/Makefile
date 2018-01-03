.POSIX:
.DELETE_ON_ERROR:

.PHONY: install
install:
	strip opencomal opencomalrun
	../build/install opencomal -d $(PREFIX)/bin
	../build/install opencomalrun -d $(PREFIX)/bin
	cp en.cat opencomal.cat
ifeq ($(OPSYS),macos)
	../build/install opencomal.cat -d $(PREFIX)/locale/en_GB.UTF-8
	cp ga.cat opencomal_ga.cat
	../build/install opencomal_ga.cat -d $(PREFIX)/locale/en_GB.UTF-8
	$(RM) opencomal_ga.cat
else
	../build/install opencomal.cat -d $(PREFIX)/locale/en_IE.utf8
	cp ga.cat opencomal.cat
	../build/install opencomal.cat -d $(PREFIX)/locale/ga_IE.utf8
endif
	$(RM) opencomal.cat
