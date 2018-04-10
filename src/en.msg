$ Messages for the OpenCOMAL interpreter.
$quote "

$set Main
NewlocaleFailed "warning: Setting locale failed.\nwarning: Please check that the locale \"%s\" is supported and installed on your system.\nwarning: Falling back to the global locale (\"%s\").\n"
BadOpt "Unrecognised option: '-%c'\n"
Usage "usage: %s [-y] [-m <msg-catalog>] ...\n"
Copyright "             (c) Copyright 1992-2002  Jos Visser <josv@osp.nl>"
Banner "OpenComal -- A free Comal implementation (version %s; %s; build %s)"
Built "             Last modified on "
Lic1 "OpenComal is licensed under the GNU General Public License (GPL) version 3"
Lic2 "(The GPL contains a very nice statement on WARRANTY; you might want to read it)"
ReqOp "Option -%c requires an operand\n"

$set CLoop
HaltedPrompt "(halted)$ "

$set Exec
Stopped "OpenComal's warp engines answered full stop"
Sure "Are you sure? "
InputEsc "Escape from INPUT"
CantExit "Can't EXIT in direct mode (use BYE to leave OpenCOMAL)"

$set Misc
NotSaved "Latest changes have not yet been saved! Proceed? "
EnvUnsaved "Environment %s contains unsaved changes!"
Proceed "Proceed? "

$set UNIX
No "No\n"
Yes "Yes\n"

$set Scan
CurExec "the current execution line"
CurData "the current DATA line"
StrucLine "a program structure line"
InhibCon "Adding/Modifying/Deleting %s has inhibited CONtinuation"

$set Pars
SynErr "Syntax error"

$set Common
Escape "Escape"
