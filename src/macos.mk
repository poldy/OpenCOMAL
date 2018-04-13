TARG_CFLAGS+=-I/usr/local/opt/readline/include
TARG_LDFLAGS:=-L/usr/local/opt/readline/lib -lreadline -lncurses -liconv
../bin/%.cat: %.tmp
	LANG=en_GB.ISO8859-15 gencat $@ $^

%.tmp: %.msg
	LANG=en_GB.ISO8859-15 $(MKCATDEFS) msgnrs.h $(filter-out $(MKCATDEFS),$^) > $@

mkcatdefs.o: ../tools/mkcatdefs.c
	$(CC) $(MIN_CFLAGS) -c $^

$(MKCATDEFS): mkcatdefs.o $(OBJ_CII)
	$(CC) $(LDFLAGS) -o $@ $^
	strip $@

msgnrs.h: en.tmp

en.tmp: $(MKCATDEFS)
