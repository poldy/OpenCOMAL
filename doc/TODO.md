Basically, too much to start documenting here.... :-)

But, what about:

- [ ] ~Remove internal reliance on chr$(0) as end-of-string~
- [ ] A Windowing interface (gtk-server-motif?)
- [ ] We could do with some comments in the source tree... :-) (Doxygen?)
- [ ] A better SAVEd file format that can withstand some changes in the 
  internal format (XDR?)
- [ ] ~A rewrite in C++ (has been long on my mind)~
- [ ] Internationalisation of messages (X/Open message catalogs?)
- [x] Fix the dependencies in the Makefile
- [ ] Heavily recursive Comal programs mess up the interpreter a bit. Maybe 
  it's a good idea to rewrite a bit of the cental interpreter to not
  recurse into itself too much. Or keep track of the stack size and
  pose a limit on the heap size the interpreter can allocate.
