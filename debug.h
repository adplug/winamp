/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999, 2000, 2001 Simon Peter, <dn.tlp@gmx.net>, et al.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * debug.h - Enables console output for Win32-based applications, by Simon Peter (dn.tlp@gmx.net)
 *
 * Include this header to enable run-time debugging output within AdPlug winamp plugin.
 * Call debug_init() first, before doing any ordinary console output.
 * debug_init() opens a console window and initializes console output.
 */

#include <stdio.h>
#include <io.h>
#include <windows.h>
#include <fcntl.h>
#include <conio.h>

void debug_init(void)
{
	int hCrt;
	FILE *hf;

	AllocConsole();
	hCrt = _open_osfhandle((long) GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT);
	hf = _fdopen( hCrt, "w" );
	*stdout = *hf;
	setvbuf( stdout, NULL, _IONBF, 0 );
	puts("Debug Started.");
}
