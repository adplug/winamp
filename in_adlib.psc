; AdPlug Winamp plugin Pimpscript
; Copyright (c) 1999 - 2002 Simon Peter <dn.tlp@gmx.net>

; Variables you can use in many of the strings:
;   $PROGRAMFILES (usually c:\program files)
;   $WINDIR (usually c:\windows)
;   $SYSDIR (usually c:\windows\system)
;   $DESKTOP (the desktop directory for the current user)
;   $INSTDIR (whatever the install directory ends up being)
;   $VISDIR  (visualization plug-in directory. DO NOT USE IN DefaultDir)
;   $DSPDIR  (dsp plug-in directory. DO NOT USE IN DefaultDir)

Name AdPlug Winamp plugin v1.2
Text This will install the AdPlug Winamp plugin on your computer.
OutFile in_adlib-1.2.exe

; File listing section
; --------------------

SetOutPath $DSPDIR
AddFile Release\in_adlib.dll
AddFile in_adlib.txt
AddFile COPYING
AddFile AUTHORS

; Post-install execute section
; ----------------------------

ExecFile "$WINDIR\notepad.exe" $DSPDIR\in_adlib.txt
