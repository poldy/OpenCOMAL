# OpenCOMAL

This is a slightly patched version of [Jos Visser](http://www.josvisser.nl/)'s
[OpenComal](http://www.josvisser.nl/opencomal/)
[0.2.6](http://www.josvisser.nl/opencomal/opencomal-0.2.6.tar.gz) (stable branch).


## FAQ

### Why this fork?

When trying to run the latest stable or instable OpenComal on more or
less recent versions of Linux (Ubuntu 8.04 and Ubuntu 10.04), the
included Linux binaries `opencomal` and `opencomalrun` immediately
crashed on startup. For me, they only worked on ancient Ubuntu
6.06. Simply re-compiling the sources did not help, so I tracked down
the problem and fixed it in the C sources.

So after 3 or 5 years of being unavailable on new Linux distributions,
the year 2012 starts with the return of COMAL, the much improved,
PASCAL-flavoured dialect of the (in)famous BASIC programming language.

### Does Jos Visser, the original author, endorse this fork?

I don't know. He did not answer my e-mail. Legally there should not
be any problem because OpenComal is licensed under the GPL.

### I do not see a new line number after entering `auto`. Is this wrong?

Never mind, just type anything, and it will appear. Or hit the
backspace key first (kudos to Harald Arnesen for this hint).

### When will you implement feature X?

I will not. As far as I am concerned, OpenCOMAL is just a fun
retrocomputing project that allows you to run old COMAL code on Linux
systems without requiring an emulator, and to find out how programming
felt like in the seventies or eighties of the previous century. Having
said that, I will consider pull requests that come with a nice
description and readable code.


## Further Reading

### included:
* [original README](README.orig)
* [original TODO](doc/TODO)
* [original documentation](doc/)

### external:
* [Børge R. Christensen's COMAL Reference Guide (for COMAL 0.14 on the C-64)](http://www.c64-wiki.de/index.php/COMAL_Reference_Guide)
* Wikipedia has useful COMAL entries in
  [German](https://de.wikipedia.org/wiki/COMAL),
  [English](https://en.wikipedia.org/wiki/COMAL), and
  [Polish](https://pl.wikipedia.org/wiki/Comal).
