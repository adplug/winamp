/*
  Copyright (c) 1999 - 2002 Simon Peter <dn.tlp@gmx.net>
  Copyright (c) 2002 Nikita V. Kalaganov <riven@ok.ru>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "plugin.h"

extern HINSTANCE myInstance;
extern HWND *myWindow;
extern MyPlayer my_player;

GuiDlgInfo::GuiDlgInfo()
{
	fileinfo_hwnd   = NULL;
	fileinfo_player = NULL;
}

int GuiDlgInfo::open(const char *file, HWND parent)
{
	// some checks
	if (fileinfo_hwnd)
	{
		if (!fileinfo_file.compare(file))
			return 0;

		if (fileinfo_player != my_player.get_player())
			delete fileinfo_player;
	}

	// current file ?
	if (!strcmp(file,my_player.get_file()) && my_player.is_playing())
		fileinfo_player = my_player.get_player();
	else
	{
		fileinfo_player = CAdPlug::factory(file,&silent);
		if (!fileinfo_player)
		{
			if (fileinfo_hwnd)
			{
				DestroyWindow(fileinfo_hwnd);
				fileinfo_hwnd = NULL;
			}
			return 0;
		}
	}

	// save file
	fileinfo_file.assign(file);

	// create window, if not exist
	if (!fileinfo_hwnd)
		fileinfo_hwnd = CreateDialogParam(myInstance,MAKEINTRESOURCE(IDD_FILEINFO),parent,(DLGPROC)DlgProc_Wrapper,(LPARAM)this);
	else
		SendMessage(fileinfo_hwnd,WM_UPDATE_ALL,0,0);

	return 1;
}

void GuiDlgInfo::dock()
{
	if (!fileinfo_hwnd)
		return;

	// already ok ?
	if (fileinfo_player == my_player.get_player())
		return;

	delete fileinfo_player;

	fileinfo_player = my_player.get_player();

	SendMessage(fileinfo_hwnd,WM_UPDATE_ALL,0,0);
}

void GuiDlgInfo::undock()
{
	if (!fileinfo_hwnd)
		return;

	// already ok ?
	if (fileinfo_player != my_player.get_player())
		return;

	fileinfo_player = CAdPlug::factory(fileinfo_file.c_str(),&silent);
	if (!fileinfo_player)
	{
		DestroyWindow(fileinfo_hwnd);
		fileinfo_hwnd = NULL;
	}

	SendMessage(fileinfo_hwnd,WM_UPDATE_ALL,0,0);
}

void GuiDlgInfo::update(bool all)
{
	if (fileinfo_hwnd)
		if (fileinfo_player == my_player.get_player())
			PostMessage(fileinfo_hwnd,all ? WM_UPDATE_ALL : WM_UPDATE,0,0);
}

BOOL APIENTRY GuiDlgInfo::DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i,spos = 0;
	char bufstr[10];
	string bufxstr;

	switch (message)
	{
		case WM_INITDIALOG:
		case WM_UPDATE_ALL:

			// set "title"/"author"/"type"
			SetDlgItemText(hwndDlg,IDC_TITLE,fileinfo_player->gettitle().insert(0," ").c_str());
			SetDlgItemText(hwndDlg,IDC_AUTHOR,fileinfo_player->getauthor().insert(0," ").c_str());
			SetDlgItemText(hwndDlg,IDC_FORMAT,fileinfo_player->gettype().insert(0," ").c_str());

			// set "instruments"
			bufxstr.erase();
			for (i=0;i<fileinfo_player->getinstruments();i++)
			{
				if (i < 9)
					sprintf(bufstr,"0%u - ",i+1);
				else
					sprintf(bufstr,"%u - ",i+1);

				bufxstr += bufstr + fileinfo_player->getinstrument(i);

				if (i < fileinfo_player->getinstruments() - 1)
					bufxstr += "\r\n";
			}
			SetDlgItemText(hwndDlg,IDC_INSTLIST,bufxstr.c_str());

  			// set "description" (ANSI "\n" to Windows "\r\n")
			bufxstr = fileinfo_player->getdesc();
			while ((spos = bufxstr.find('\n',spos)) != bufxstr.npos)
			{
				bufxstr.insert(spos,"\r");
				spos += 2;
			}
			SetDlgItemText(hwndDlg,IDC_DESCRIPTION,bufxstr.c_str());

			// set "subsong" slider
			SendDlgItemMessage(hwndDlg,IDC_SUBSONGSLIDER,TBM_SETRANGE,(WPARAM)FALSE,(LPARAM)MAKELONG(1,fileinfo_player->getsubsongs()));

			if (fileinfo_player == my_player.get_player())
				SendDlgItemMessage(hwndDlg,IDC_SUBSONGSLIDER,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)(my_player.get_subsong() + 1));
			else
				SendDlgItemMessage(hwndDlg,IDC_SUBSONGSLIDER,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)(DFL_SUBSONG + 1));

			SetDlgItemInt(hwndDlg,IDC_ORDERS,fileinfo_player->getorders(),FALSE);
			SetDlgItemInt(hwndDlg,IDC_PATTERNS,fileinfo_player->getpatterns(),FALSE);

			// reset "info" data
			fi_order = fi_pattern = fi_row = fi_speed = 255;
			fi_refresh = -1;


		case WM_UPDATE:

			// update "subsong" info and slider
			SetDlgItemInt(hwndDlg,IDC_SUBSONGMIN,1,FALSE);

			if (fileinfo_player == my_player.get_player())
				SetDlgItemInt(hwndDlg,IDC_SUBSONG,my_player.get_subsong() + 1,FALSE);
			else
				SetDlgItemText(hwndDlg,IDC_SUBSONG,"--");

			SetDlgItemInt(hwndDlg,IDC_SUBSONGMAX,fileinfo_player->getsubsongs(),FALSE);

			// update "info"
			if (fi_order != fileinfo_player->getorder())
			{
				fi_order = fileinfo_player->getorder();

				SetDlgItemInt(hwndDlg,IDC_ORDER,fi_order,FALSE);
			}
			if (fi_pattern != fileinfo_player->getpattern())
			{
				fi_pattern = fileinfo_player->getpattern();

				SetDlgItemInt(hwndDlg,IDC_PATTERN,fi_pattern,FALSE);
			}
			if (fi_row != fileinfo_player->getrow())
			{
				fi_row = fileinfo_player->getrow();

				SetDlgItemInt(hwndDlg,IDC_ROW,fi_row,FALSE);
			}
			if (fi_speed != fileinfo_player->getspeed())
			{
				fi_speed = fileinfo_player->getspeed();

				SetDlgItemInt(hwndDlg,IDC_SPEED,fi_speed,FALSE);
			}
			if (fi_refresh != fileinfo_player->getrefresh())
			{
				fi_refresh = fileinfo_player->getrefresh();

				sprintf(bufstr,"%.2f hz",fi_refresh);
				SetDlgItemText(hwndDlg,IDC_TIMER,bufstr);
			}

			return TRUE;


		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDCANCEL:

					// close window
					if (fileinfo_player != my_player.get_player())
					{
						delete fileinfo_player;

					}

					DestroyWindow(hwndDlg);

					fileinfo_player = NULL;
					fileinfo_hwnd = NULL;

					return TRUE;
			}


		case WM_HSCROLL:
			switch(GetDlgCtrlID((HWND)lParam))
			{
				case IDC_SUBSONGSLIDER:

					// subsong changing
					if (fileinfo_player == my_player.get_player())
					{
						int newsubsong = SendDlgItemMessage(hwndDlg,IDC_SUBSONGSLIDER,TBM_GETPOS,0,0);

						if ((newsubsong - 1) != my_player.get_subsong())
						{
							my_player.set_subsong(newsubsong - 1);

							SetDlgItemInt(hwndDlg,IDC_SUBSONG,newsubsong,FALSE);
						}
					}

					return TRUE;
			}
	}

	return FALSE;
}

BOOL APIENTRY GuiDlgInfo::DlgProc_Wrapper(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_INITDIALOG)
		SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);

	GuiDlgInfo *the = (GuiDlgInfo *)GetWindowLong(hwndDlg,GWL_USERDATA);

	if (!the)
		return FALSE;

	return the->DlgProc(hwndDlg,message,wParam,lParam);
}
