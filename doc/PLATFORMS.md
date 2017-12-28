--------------------------
OpenComal Platform support
--------------------------

I am:	Jos Visser <josv@osp.nl>
Date: 	Wed Sep 18 17:39:19 CEST 2002

As of version 0.2.4 the following platforms are supported:

- Linux
- MsDos
- Win32 (Windows 95, 98, ME, XP, NT, 2000)

OpenComal has been carefully constructed to abstract all system
dependent routines in a separate module/file, with clear entry points.
Thus, porting to another platform is not all too difficult... :-)

The Linux dependent file (pdclinux.c) should be real easy to use on
other Unix operating systems as well... Once upon a time I had OpenComal
running on FreeBSD, SunOS and HP-UX. I have received favourable reports
about OpenComal running on BSD/OS 4.3 (with the Linux routines).

If you're contemplating a port, please do not hesitate to contact me.
