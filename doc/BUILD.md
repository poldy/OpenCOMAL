# OpenComal Build Instructions

I am: Jos Visser <josv@osp.nl>  
Date: Wed Sep 18 17:42:58 CEST 2002

OK. And now for the hard part.... :-)

## Building for Linux

This section describes how to build OpenComal for the Linux platform. 
OpenComal currently has none of the fancy autoconf features that you 
might be used too, so some manual Makefile hacking might be in order. 
On the other hand, OpenComal on Linux uses strictly ANSI C features 
plus ncurses for its screen/keyboard handling, so it should not be too 
difficult to get it going.

It builds and compiles flawlessly on my laptop. For reference, I run 
Rock Linux (dRock 1.6.1-rc), which consists of:

- Linux jadzia 2.4.19-rc1 #10 Wed Sep 4 16:54:55 CEST 2002 i686 unknown
- gcc version 2.95.3 20010315 (release)
- ncurses 5.2
- glibc 2.2.5
- GNU ld version 2.12.90.0.4 20020408
- GNU assembler version 2.12.90.0.4 (i686-pc-linux-gnu) using BFD version 2.12.90.0.4 20020408
- flex version 2.5.4
- bison (GNU Bison) 1.32

If your Linux system looks more or less like that, a simple "make" will 
suffice. If not, some Makefile hacking is in order. I can not really 
help you with this, but if you are getting some strange error messages
and can not get OpenComal to compile, please contact me!

(Addition by [Christian Pietsch](https://github.com/pietsch):)
For building this under Debian, your can install requirements like this:
sudo apt-get install build-essential flex bison libncurses5-dev

(Addition by [poldy](https://github.com/poldy):)
OpenComal now depends on the wide variant of ncurses, so substitute libncursesw5-dev above.

## Building for macOS

(Support for macOS was contributed by [poldy](https://github.com/poldy).)

- Install the "readline" package from Homebrew.
- Then run "make OPSYS=macos".
- Then run "make install PREFIX=$HOME" (or wherever).
- This was tested on "High Sierra".

## Hacking

The following build system features are intended for maintainers only.

For development I usually compile with a C++ compiler (for the stricter checking) instead.
You can do this by passing make a REALCC=c++ argument.

Passing make DEBUG=1 will also enable the debug logs (note that these also need a -d command-line option) and assertions.

I also ran cppcheck, a static analyser, on the source code.
The command line I used was `cppcheck --enable=all --std=c11 --std=posix .`.
For this, and the compiler warnings, the usual advice applies that at least the root causes of the warnings should be *understood*.
