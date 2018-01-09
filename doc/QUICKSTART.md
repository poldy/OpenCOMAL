# OpenComal Quick Start Tutorial

I am: Jos Visser <josv@osp.nl>  
Date: Sun Sep  8 10:17:20 CEST 2002

1. If you are lucky, you have picked up a distribution with the 
   opencomal binaries compiled and ready to run. If not, first
   consult [BUILD.md](BUILD.md) on how to compile OpenComal. Then, after
   compiling, return here.

1. OpenComal uses the Latin-9 character encoding internally, but
   the rest of the world has settled on UTF-8.
   The optimal setup is where LANG is set to a UTF-8 locale like
   ga_IE.utf8 (Linux) or en_GB.UTF-8 (macOS);
   and a ga_IE@euro (Linux) or en_GB.ISO8859-15 (macOS) locale also exists on the system.
   This should allow the entry and storage of accented characters.

   Unfortunately, there is a "feature" where this only works for
   locales that have a '.' decimal point separator, breaking da_DK which I
   was interested in for nostalgia purposes :-)

   You can get Linux (at least Debian) to generate new locale setup files by
   editing /etc/locale.gen and running "sudo locale-gen".
   I don't know of anything similar for macOS.

1. OpenComal can optionally use different languages for output messages. Setting
   the LANG environment variable as above should be all that is required on Linux.
   On macOS you may also need to specify a command-line argument, e.g. "-m
   opencomal_ga.cat". You can define an alias for this by adding "alias
   opencomal='opencomal -l opencomal_ga.cat'" to your $ENV file.

   Note that this feature is completely optional and the fallback is to traditional
   English messages.

2. Start OpenComal ('opencomal' binary). The screen clears and something like the
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
       50 delay 5
       60 ^C

   You quit program enter mode using Ctrl+C.

6. Now give the 'list' command. The output looks like this:

       10 FOR f:=1 TO 10 DO
       20   PRINT f
       30 ENDFOR
       40 PRINT "OpenComal rules!"
       50 DELAY 5

7. Now 'run' the program... Did it do what you expected?

8. Save the program:

       $ save "first.cml"

9. And now exit the interpreter with the 'bye' command.

10. And finally, run the program with the 'opencomalrun' runtime-only 
    program:

        opencomalrun first.cml

11. Now read 'doc/OpenComal.txt'...
