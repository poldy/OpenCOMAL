# OpenComal Samples

This directory contains a bunch of OpenComal samples. They are all in 
"LIST" format (ASCII file format), which means that they must be ENTERed
instead of LOADed. E.g.:

    $ NEW
    $ ENTER "verbaal.lst"

(That particular program generates the Dutch textual representation of 
a number). There are also a lot of programs that test specific OpenComal
functions and statements...

Another interesting one is "slpl.lst", which is an interpreter (in OpenComal)
for a simple list oriented programming language not completely unlike LISP.

The examples/ directory contains some general example programs. The one doing
decompression calculations is seriously broken, so please do not dive using 
the program's output as a guideline!!!!!!!

The tests/ directory contains programs that test all OpenComal's statements
and functions. To do a quick test of everything, follow this procedure:

- enter "lst2sq.lst"
- save "lst2sq.sq"
- run
  (this enters and saves each program)
- new
- load "gentest.sq"
- run

The autorep and truefalse programs in the examples directory appears
courtesy of Dick Klingens.

Share and Enjoy!

++Jos.nl
