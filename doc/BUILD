----------------------------
OpenComal Build Instructions
----------------------------

I am: Jos Visser <josv@osp.nl>
Date: Wed Sep 18 17:42:58 CEST 2002

OK. And now for the hard part.... :-)

------------------
Building for Linux
------------------

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

(Addition by Christian Pietsch <https://github.com/pietsch>:)
For building this under Debian, your can install requirements like this:
sudo apt-get install build-essential flex bison libncurses5-dev

-------------------------------
Building for MsDos and/or Win32
-------------------------------

I build OpenComal for MsDos and Win32 in a virtual Windows 98 PC running
under the control of VMWare on my Linux laptop. Apart from plain Win98 I
installed:

- VIM 6.1 (*the* editor; see http://www.vim.org)
- Borland C/C++ 4.0
- GNU utilities for Win32
  (see http://unxutils.sourceforge.net)
  This gives me rm, bison, flex and other tools needed to build
  OpenComal.

Then I use Borland make with Makefile.w32 or Makefile.dos to build the
binaries.

------------------
Building for macOS
------------------

(Support for macOS was contributed by poldy <https://github.com/poldy>.)

Install the "readline" package from Homebrew.
Then run "make OPSYS=macos".
This was tested on "High Sierra".
