# OpenCOMAL

(Tá [leagan Gaeilge](LÉIGHMÉ.md) den doiciméad seo ar fáil.)

This is a patched version of [Jos Visser](http://www.josvisser.nl/)'s
[OpenComal](http://www.josvisser.nl/opencomal/)
[0.2.7](http://www.josvisser.nl/opencomal/opencomal-0.2.7-pre1-work.tar.gz) (unstable branch).

## FAQ

### Why this fork?

The previous maintainer, pietsch <https://github.com/pietsch>, fixed
bugs so that OpenComal could run on then-current versions of Linux.

I took over the project because I am interested in practical
methods for improving the quality of old projects (similar to the
[ntpsec](https://www.ntpsec.org/) project, although I am still
some way behind them). OpenComal is a nice size to run experiments
on. One of the nice things about the COMAL language itself is
that it has a vendor-neutral standard with several implementions.
Lately I've also been trying to bring OpenComal closer to compliance
with those standards:
* [COMAL Kernel spec](http://datamuseum.dk/wiki/COMAL/standardization#TeleNova.2C_Nyn.C3.A4shamn.2C_Sweden._March_13_to_16.2C_1985)
* [Common COMAL](https://computerarchive.org/files/computer/newsletters/comal-today/COMAL_Today_Issue_24.pdf), pages 21-46
* [The "STANDARD" package](https://computerarchive.org/files/computer/newsletters/comal-today/COMAL_Today_Issue_27.pdf), pages 47-52

One regression is that I removed support for platforms that I don't
build and test, MS-DOS and Win32.
I'd love to get a pull request re-enabling these.
The last version containing the needed files is tagged as `last_dos_w32_version`.

### Does Jos Visser, the original author, endorse this fork?

I don't know. He did not answer the previous maintainer's e-mail
and I never sent him one. Legally there should not
be any problem because OpenComal is licensed under the GPL.

### I do not see a new line number after entering `auto`. Is this wrong?

Never mind, just type anything, and it will appear. Or hit the
backspace key first (kudos to Harald Arnesen for this hint).

### When will you implement feature X?

I'm reluctant to do this, unless the feature is in one of the above
standards. I *am* interested in bug reports and pull requests
though. I think OpenCOMAL definitely has a niche in retrocomputing
and may possibly have one in education, e.g. for other domain experts
that don't have time to learn Computer Science.

## Further Reading

### included:
* [original README](README.orig)
* [TODO](https://github.com/poldy/OpenCOMAL/wiki/TODO.md)
* [original documentation](doc/)

### external:
* [Børge R. Christensen's COMAL Reference Guide (for COMAL 0.14 on the C-64)](http://www.c64-wiki.de/index.php/COMAL_Reference_Guide)
* Wikipedia has useful COMAL entries in
  [German](https://de.wikipedia.org/wiki/COMAL),
  [English](https://en.wikipedia.org/wiki/COMAL), and
  [Polish](https://pl.wikipedia.org/wiki/Comal).
