*VERSION'STANDARD$*: displays version and author information.

*FILE'EXISTS(filename$)*: Checks if the specified file (or
volume:path/file) exists and returns TRUE or FALSE.

*TYPE(filename$)*: Similar to the Windows TYPE command.
It gets characters from the specified file one by one and prints
them. The text can be paused with the space bar and aborted with
ESC. It first calls FILE'EXISTS, which eliminates the need for you
to do so.

*CLEARKEYS*: Clears all keys from the keyboard buffet so the next
KEYS, INKEYS, INPUT does not unintentionally pick them up.

*CLEARLINES(row1,row2)*: Clear all text between the specified lines,
inclusive.

*NUMBER(text$)*: Returns TRUE if the string passed to it contains
a valid number. It can be used to prevent the VAL() function from
causing an error.

*GETKEY$*: First clears the keyboard buffer, then goes into a loop
until a key is pressed. The key pressed is returned to the program.

*GETVALIDKEY$(valid$)*: Clears the keyboard buffer, goes into a
loop until a key from the string *valid$* is pressed, then returns
that key to the program.

*STRIPCHARS$(text$,char$)*: Removes all <char$> characters from the
end of <text$>.

*GETINFILE(REF infile$)*: Opens the file <infileS> for input. If
the file does not exist, the user is notified and prompted again
for the filename.

*GETOUTFILE(REF outfile$)*: Opens the file <outfile$> for writing.
If the file already exists, the user is asked for permission to
overwrite it or to enter another filename.

*GETFILES(REF infile$,REF outfile$)*: Combines the above two PROCs.
First asks for an input filename, then an output filename, and
verifies that the names are not the same and warns the user if they
are.

*QUICKSORT(REF array$(),first,last)*: This is the familiar Quicksort
PROC that has been published in COMAL today several times. It very
quickly sorts an array of strings. The sort can be confined to a
range of the array starting with element <first> and ending with
<last>.

*QUICKSORTNUM(REF array(),first,last)*: Like QUICKSORT, except this
one sorts numeric arrays. The above PROC will correctly sort numeric
arrays also if each number has the same number of digits.

*YES(prompt$)*: Used to get a YES or NO answer. For example:
```
IF yes("Quit now?") THEN END
```
It does not flash the cursor while awaiting input, but prints "YES"
or "NO" at the current cursor position after the Y or N key is
pressed.

*CENTER(text$)*: Centers the string, <text$> on the current screen
line. If more than 79 characters, the string is printed without
centering.

*CENTER'AT(row,text$)*: This PROC centers the string <text$> on row
<row>. If more than 79 characters, the string is printed without
centering.
