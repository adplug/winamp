/*
  Copyright (c) 1999 - 2006 Simon Peter <dn.tlp@gmx.net>
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

#define REVERSE(d) (LOBYTE(HIWORD(d))) + (LOBYTE(LOWORD(d))<<16) + (HIBYTE(LOWORD(d))<<8) + (HIBYTE(HIWORD(d))<<24)

extern HINSTANCE myInstance;

GuiCtrlHyperlink hyperlink;

void GuiDlgAbout::open(HWND parent)
{
  DialogBoxParam(myInstance,MAKEINTRESOURCE(IDD_ABOUT),parent,(DLGPROC)DlgProc_Wrapper,(LPARAM)this);
}

BOOL APIENTRY GuiDlgAbout::DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef DEBUG
  //printf("GuiDlgAbout::DlgProc(): Message 0x%08X received.\n",message);
#endif
  BYTE *logo    = (BYTE *)LockResource(LoadResource(myInstance,FindResource(myInstance,MAKEINTRESOURCE(IDB_LOGO),RT_BITMAP)));
  char *license = (char *)LockResource(LoadResource(myInstance,FindResource(myInstance,MAKEINTRESOURCE(IDR_TEXT_LICENSE),"TEXT")));
  char *history = (char *)LockResource(LoadResource(myInstance,FindResource(myInstance,MAKEINTRESOURCE(IDR_TEXT_HISTORY),"TEXT")));

  TCITEM tci;

  switch (message)
    {
    case WM_INITDIALOG:

      tab_hwnd = NULL;

      // init tab control
      tci.mask = TCIF_TEXT;

      tci.pszText = " General ";
      SendDlgItemMessage(hwndDlg,IDC_ATABS,TCM_INSERTITEM,0,(LPARAM)&tci);
		
      tci.pszText = " License ";
      SendDlgItemMessage(hwndDlg,IDC_ATABS,TCM_INSERTITEM,1,(LPARAM)&tci);
		
      tci.pszText = " What's New ";
      SendDlgItemMessage(hwndDlg,IDC_ATABS,TCM_INSERTITEM,2,(LPARAM)&tci);

      // set default tab index
      SendDlgItemMessage(hwndDlg,IDC_ATABS,TCM_SETCURSEL,0,0);


    case WM_SYSCOLORCHANGE:
    case WM_UPDATE:

      // delete old tab window
      if (tab_hwnd)
	{
	  DestroyWindow(tab_hwnd);

	  tab_hwnd = NULL;
	}

      // display new tab window
      tab_index = (int)SendDlgItemMessage(hwndDlg,IDC_ATABS,TCM_GETCURSEL,0,0);

      switch (tab_index)
	{
	case 0:

	  // fix logo
	  *(DWORD *)&logo[0x28 + (logo[0x428] << 2)] = REVERSE(GetSysColor(COLOR_BTNFACE));

	  tab_hwnd = CreateDialogParam(myInstance,MAKEINTRESOURCE(IDD_ABT_ADPLUG),GetDlgItem(hwndDlg,IDC_ATABWND),(DLGPROC)TabDlgProc_Wrapper,(LPARAM)this);

	  // plugin
	  SetDlgItemText(tab_hwnd,IDC_PLUGIN_VER,PLUGIN_VER " (" __DATE__ /*" " __TIME__ */")");

	  break;

	case 1:
					
	  tab_hwnd = CreateDialogParam(myInstance,MAKEINTRESOURCE(IDD_ABT_LICENSE),GetDlgItem(hwndDlg,IDC_ATABWND),(DLGPROC)TabDlgProc_Wrapper,(LPARAM)this);

	  // license
	  SetDlgItemText(tab_hwnd,IDC_LICENSE,license);

	  break;

	case 2:

	  tab_hwnd = CreateDialogParam(myInstance,MAKEINTRESOURCE(IDD_ABT_HISTORY),GetDlgItem(hwndDlg,IDC_ATABWND),(DLGPROC)TabDlgProc_Wrapper,(LPARAM)this);

	  // history
	  SetDlgItemText(tab_hwnd,IDC_HISTORY,history);

	  break;
	}

      return FALSE;


    case WM_NOTIFY:
      switch (((NMHDR *)lParam)->code)
	{
	case TCN_SELCHANGE:
	  PostMessage(hwndDlg,WM_UPDATE,0,0);
	  return FALSE;
	}


    case WM_COMMAND:
      switch (LOWORD(wParam))
	{
	case IDCANCEL:
	  EndDialog(hwndDlg,wParam);
	  return 0;
	}
    }

  return FALSE;
}

BOOL APIENTRY GuiDlgAbout::TabDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef DEBUG
  //printf("GuiDlgAbout::TabDlgProc(%d): Message 0x%08X received.\n",tab_index,message);
#endif

  switch (message)
    {
    case WM_INITDIALOG:

      switch (tab_index)
	{
	case 0:

	  /* authors */

	  hyperlink.add(GetDlgItem(hwndDlg,IDC_URL_SP),"dn.tlp@gmx.net", "mailto:dn.tlp@gmx.net");
	  hyperlink.add(GetDlgItem(hwndDlg,IDC_URL_NK),"riven@ok.ru",    "mailto:riven@ok.ru");

	  /* sites */

	  hyperlink.add(GetDlgItem(hwndDlg,IDC_URL_ADPLUG),  "adplug.sourceforge.net",   "http://adplug.sourceforge.net");
	  hyperlink.add(GetDlgItem(hwndDlg,IDC_URL_CHIPTUNE),"chiptunes.back2roots.org", "http://chiptunes.back2roots.org");
	  hyperlink.add(GetDlgItem(hwndDlg,IDC_URL_MALF),    "www.astercity.net/~malf",  "http://www.astercity.net/~malf");

	  break;

	case 1:

	  /* gnu */

	  hyperlink.add(GetDlgItem(hwndDlg,IDC_URL_GNU),"www.gnu.org/licenses/lgpl.txt","http://www.gnu.org/licenses/lgpl.txt");

	  break;
	}

      // move tab content on top
      SetWindowPos(hwndDlg,HWND_TOP,3,22,0,0,SWP_NOSIZE);

      return FALSE;
    }

  return FALSE;
}

BOOL APIENTRY GuiDlgAbout::DlgProc_Wrapper(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message == WM_INITDIALOG)
    SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);

  GuiDlgAbout *the = (GuiDlgAbout *)GetWindowLong(hwndDlg,GWL_USERDATA);

  if (!the)
    return FALSE;

  return the->DlgProc(hwndDlg,message,wParam,lParam);
}

BOOL APIENTRY GuiDlgAbout::TabDlgProc_Wrapper(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message == WM_INITDIALOG)
    SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);

  GuiDlgAbout *the = (GuiDlgAbout *)GetWindowLong(hwndDlg,GWL_USERDATA);

  if (!the)
    return FALSE;

  return the->TabDlgProc(hwndDlg,message,wParam,lParam);
}
