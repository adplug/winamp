; AdPlug Winamp plugin Pimpscript
; Copyright (c) 1999 - 2003 Simon Peter <dn.tlp@gmx.net>

; Variables you can use in many of the strings:
;   $PROGRAMFILES (usually c:\program files)
;   $WINDIR (usually c:\windows)
;   $SYSDIR (usually c:\windows\system)
;   $DESKTOP (the desktop directory for the current user)
;   $INSTDIR (whatever the install directory ends up being)
;   $VISDIR  (visualization plug-in directory. DO NOT USE IN DefaultDir)
;   $DSPDIR  (dsp plug-in directory. DO NOT USE IN DefaultDir)

Name AdPlug Winamp plugin v1.4
Text This will install the AdPlug Winamp plugin on your computer.
OutFile in_adlib-1.4.exe

; File listing section
; --------------------

SetOutPath $DSPDIR
AddFile Release\in_adlib.dll
AddFile ..\README
AddFile \Dokumente und Einstellungen\Simon\Eigene Dateien\adplug.db

; Post-install execute section
; ----------------------------

ExecFile "$WINDIR\notepad.exe" $DSPDIR\README
