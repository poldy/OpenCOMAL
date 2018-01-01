------------------------------
OpenComal Quick Start Tutorial
------------------------------

I am: Jos Visser <josv@osp.nl>
Date: Sun Sep  8 10:17:20 CEST 2002

1. If you are lucky, you have picked up a distribution with the 
   opencomal binaries compiled and ready to run. If not, first
   consult the file BUILD on how to compile OpenComal. Then, after
   compiling, return here.

2. Start OpenComal ('opencomal' binary). The screen clears and the
   following appears:

      OpenComal -- A COMAL implementation
                   (c) Copyright 1992-2002  Jos Visser <josv@osp.nl>

      Warning: This is a Beta/Preliminary version! Please check the documentation

      $

3. Congratulations. You just started OpenComal!

4. Enter the following command:

      $ auto

   You can now enter your first OpenComal program. The line numbers 
   appear automagically :-)

5. Enter the following program:

     10 for f=1 to 10
     20 print f
     30 next
     40 print "OpenComal rules!"
     50 ^C

   You quit program enter mode using Ctrl+C.

6. Now give the 'list' command. The output looks like this:

       10 FOR f:=1 TO 10 DO
       20   PRINT f
       30 ENDFOR
       40 PRINT "OpenComal rules!"

7. Now 'run' the program... Did it do what you expected?

8. Save the program:

   $ save "first.cml"

9. And now exit the interpreter with the 'bye' command.

10. And finally, run the program with the 'opencomalrun' runtime-only 
    program:

    opencomalrun first.cml

    If nothing seems to happen, your computer is too fast! (OpenComal 
    uses the ncurses library and restores the screen to its original 
    content upon termination...

11. Now read 'doc/OpenComal.txt'...
