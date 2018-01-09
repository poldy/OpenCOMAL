# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- Ported to macOS
- Added CURROW, CURCOL, BITAND, BITOR, BITXOR, RANDOMIZE, REPORT, ESC
  keywords.
- Added GET$, ZONE, DELAY, TAB keywords.
- Extended PRINT and INPUT to take optional AT modifier.
- Added support for Latin-9 charset in strings, comments & identifiers.
- Added [standard.lst](../samples/examples/standard.lst)
- Start Doxygen markup of public header files. WIP
- Start extraction & translation of strings, also WIP.
- Allow "USING" after "PRINT FILE" & "PRINT AT".
- Added [gtk_server.lst](../samples/examples/gtk_server.lst) & [mbutton.lst](../samples/examples/mbutton.lst)

### Changed
- Many changes were made to align better with Common COMAL & pass the test suite.
- Changed CURSOR arguments from (x, y) to (row, column).
- Changed DIV, MOD to be floored rather than symmetric
- Renamed LN to LOG and LOG to LOG10.

### Removed
- Temporarily removed support for MS-DOS and Win32, because I can't
  build or test them. I'd love if someone updated the support and
  sent in a pull request (git tag "[last_dos_w32_version](https://github.com/poldy/OpenCOMAL/releases/tag/last_dos_w32_version)" is the
  last version containing the files).

## [0.2.7]:
- Started implementing the MODULE concept. See "whitepaper1.txt"
  in the doc/ directory for more information.
- Changed the meaning of "END" to remove the complete variable
  environment. Also made "END" executable as a command (so as to
  end an interrupted program".
- Changed the OpenComal prompt to "(halted)$ " if the
  interpreter is in HALTED state (program was stopped but could
  be continued).
- Added the STATIC command (also see "whitepaper1.txt".)

## 0.2.6:
- New sys_edit() for Win32 by Gary Lake.
- Modified REF implementation so that array element can be a REF
  parameter
- Removed irritating bug in allocating new floating point exp
  element memory that messed up the memory (GETEXP(double) in
  pars_exp_float).
- Solved signedness problem in characters > 127
  (e.g. print ord(chr$(129)) now prints 129 as expected)
- Now allowed for substring specification like
  a$(:n#:). This resulted in changes to the SAVE format that I
  think are upward compatible from previous releases.
- There now exists a one line version of REPEAT:
       100 REPEAT a$:+"x" UNTIL LEN(A$)=20
  This changed the SAVE format (again... :-/ )
- Added UPPER$(<stringexp>) and LOWER$(<stringexp>)
* Declared 0.2.6-pre1
- Overhauled the RND function. Now does:
  RND 
  RND(10)
  RND(-10,10)
* Declared 0.2.6-pre2
- Added RETRY (in HANDLER part of ENDTRAP). This lead to the
  first "goto" statement in the OpenComal source! :-)
  And, guess what, the SAVE format changed because of adding a
  new keyword...  :-(
- Added FRAC and ROUND.

## 0.2.5:
- Patched my development system so the precompiled binaries for Linux
  no longer have a depedency on libtinfo.so.5
- Added SYS(sbrk) to the Linux code (returns the current end 
  address of the interpreter's data segment).
- Fixed small error in interpreting variable argument list in
  pars_error (pdcparss.c).
- Lexical analysis error "Unexpected character" now gives hex
  code of the unexpected character.
- Fixed error when pressing ^C during editing an errounous
  ENTERed line.
- Fixed editing of ENTERed lines in error (at least under Linux).
- Fixed various errors related to ENTERing files in MsDos format
  under Linux (CRLF issues)...
- Added BYE as an alias for QUIT
- Allowed for the IMPORT of PROCs and FUNCs. This is not
  necessary in OpenComal (the IMPORT), but it was in UniComal.
  OpenComal allows for the IMPORT, but it is a NOP. Upon first
  execution, a warning is given...
* pre2:
- Added the DIR, DIR$, UNIT and UNIT$ statements and functions.
  (this also lead to changes in the internal symbols, and hence
  in changes in the SAVE file format).
- Solved error in comparing parameter lists of externally
  defined procedures/functions with the stub in the main
  program.
- Comments appeared in the code! Has somebody gone mad? :-)
* pre3:
- Made it possible to use "" inside a string to denote a single
  "; e.g. print """OpenComal"" rulez!"
- Added != as a token identical to <> (neqSYM)
  This to make it easier for C/C++/Java hackers like me to use
  Comal :-)
- I hacked away extensively at the array handling code. An array
  is now a valid rvalue, so the following is now possible:
        a$()=b$()
  The () brackets are necessary if the array is an rvalue, and
  not when it's an lvalue.
  When confronted with an array rvalue that it does not know
  (unDIMmed array), the interpreter tries to execute it as a
  function call.
* pre4:
- Changed the way floating point constants are being maintained
  in the expressions parse trees. Now both the text and the
  binary values are maintained. This makes it possible to LIST
  the program exactly as the user typed it, regardless of internal
  inaccuracies in binary floating point systems.
  But, the SAVE file format changed again because of this :-(
- Added SYS(now) under Linux; gives the number of seconds sinds 
  the epoch (1-1-1970)
- Added the SPC$ function
- Added SYS$(version); gives the version of the OpenComal 
  interpreter
- I plugged GNU ReadLine into the Linux version. This makes 
  editing lines much friendlier. The history mechanism is 
  supported too (there's still a small error in editing a large 
  line (larger>width) on the bottom line of the window...).
* pre5:
- Solved a problem with the DIR command on Win32 and MsDos; 
  dir "*.obj" gave a "Bad command or filename" due to an error
  in creating the operating system command (sys_dir()).
- Solved a bug in MsDos OpenComal where calling an operating system
  command had no effect on the cursor position, even when the
  called command produced text output. The cursor command is now 
  being reread after an OS command.
- Solved a bug in LISTing string constants that contained a \. The
  backslash was not properly escaped...
- Added PASS as an alias for OS
- Added KEY$ and INKEY$
- Added CHDIR, MKDIR and RMDIR
* final
- Solved a problem with READ type mismatches (e.g. READ a with 
  the current DATA expression a string).

## 0.2.4:
- "Official" MsDos & Win32 support
- Changed the format of the SAVE file to include a marker and
  the HOST_OS_CODE of the OS the file was created with

## 0.2.3:
- Uhhh, not much, but 0.2.2 was never released due to a CVS tagging
  mistake...

## 0.2.2:
- The ENV command has been added (various impacts on memory mgmt etc).
- Bug: Segfault when WRITE-ing unassigned elements of a string array
- Bug: Forgot to set scan_ok=0 after a LOAD
- Bug: mem_free called free instead of sys_free
- Bug: Someone called free instead of mem_free
- Small syntax error in pdcpars.y fixed (thanks: Thierry B.)
  (was caught by newer and less forgiving bison 1.35)

## 0.2.1:
- Various small bugs
- Compatibility with BSD/OS 4.3 (thanks: Kurt. J. Lidl)

## 0.2.0:
- First free release of OpenComal

[Unreleased]: https://github.com/poldy/OpenCOMAL/compare/v0.2.7-pre1...HEAD
[0.2.7]: https://github.com/poldy/OpenCOMAL/compare/v0.2.6...v0.2.7
