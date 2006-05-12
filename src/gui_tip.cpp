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

extern HINSTANCE myInstance;

GuiCtrlTooltip::GuiCtrlTooltip(HWND parent)
{
  tooltip_hwnd = NULL;
  trigger_hwnd = NULL;
  parent_hwnd  = parent;
}

GuiCtrlTooltip::~GuiCtrlTooltip()
{
  unsigned int i;

  for(i = 0; i < work.text.size(); i++) free(work.text[i]);
  for(i = 0; i < work.title.size(); i++) free(work.title[i]);
}

void GuiCtrlTooltip::add(HWND hwnd, const char *title, const char *text)
{
  // add tooltip data to list
  work.hwnd.push_back(hwnd);
  work.proc.push_back((WNDPROC)GetWindowLong(hwnd,GWL_WNDPROC));
  work.text.push_back(strdup(text));
  work.title.push_back(strdup(title));

  // change window parameters
  SetWindowLong(hwnd,GWL_USERDATA,(LONG)this);
  SetWindowLong(hwnd,GWL_WNDPROC,(LONG)WndProc);
}

void GuiCtrlTooltip::remove(int i)
{
  work.hwnd.erase(work.hwnd.begin()+i);
  work.proc.erase(work.proc.begin()+i);
  free(work.text[i]); work.text.erase(work.text.begin()+i);
  free(work.title[i]); work.title.erase(work.title.begin()+i);
}

void GuiCtrlTooltip::trigger(HWND hwnd)
{
  trigger_hwnd = hwnd;

  SendMessage(hwnd,BM_SETCHECK,BST_CHECKED,0);

  add(hwnd,"ToolTip trigger","You can enable/disable tooltips by changing the state of this checkbox");
}

LRESULT CALLBACK GuiCtrlTooltip::WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
#ifdef DEBUG
  //printf("GuiCtrlTooltip::WndProc(0x%08X): Message 0x%08X received.\n",hwnd,message);
#endif
  RECT rect;
  POINT pt,pt2;

  // get our object pointer
  GuiCtrlTooltip *the = (GuiCtrlTooltip *)GetWindowLong(hwnd,GWL_USERDATA);

  // get our index
  int i = 0;

  while (the->work.hwnd[i] != hwnd)
    i++;

  // yeah!
  switch (message)
    {
    case WM_MOUSEMOVE:
      GetCursorPos(&pt2);
      ScreenToClient(hwnd,&pt2);
      if ((the->pt1.x != pt2.x) || (the->pt1.y != pt2.y))
	{
	  the->hide(i);
	  SetTimer(hwnd,7,1000,NULL);
	  the->pt1 = pt2;
	}
      break;

    case WM_CAPTURECHANGED:
      KillTimer(hwnd,7);
      the->hide(i);
      break;

    case WM_TIMER:
      KillTimer(hwnd,7);
      GetCursorPos(&pt);
      ScreenToClient(hwnd,&pt);
      GetClientRect(hwnd,&rect);
      if (PtInRect(&rect,pt))
	the->show(i);
      return 0;
    }

  // default WinProc()
  LRESULT result = CallWindowProc(the->work.proc[i],hwnd,message,wParam,lParam);

  // window destruction
  if (message == WM_NCDESTROY)
    {
      the->hide(i);
      the->remove(i);
    }

  return result;
}

void GuiCtrlTooltip::show(int i)
{
  POINT pt;

  if (trigger_hwnd)
    if (IsWindow(trigger_hwnd))
      if (SendMessage(trigger_hwnd,BM_GETCHECK,0,0) == BST_UNCHECKED)
	return;

  GetCursorPos(&pt);

  tooltip_hwnd = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
				WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP/*| TTS_BALLOON*/,
				CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				parent_hwnd, NULL, myInstance,
				NULL);
  if (!tooltip_hwnd)
    return;

  SetWindowPos(tooltip_hwnd,
	       HWND_TOPMOST,
	       0,
	       0,
	       0,
	       0,
	       SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

  TOOLINFO ti;

  memset(&ti,0,sizeof(TOOLINFO));

  ti.cbSize   = sizeof(TOOLINFO);
  ti.uFlags   = TTF_TRACK | TTF_ABSOLUTE;
  ti.hwnd     = parent_hwnd;
  ti.hinst    = myInstance;
  ti.lpszText = LPSTR_TEXTCALLBACK;
  ti.lParam   = (long)work.text[i];

  SendMessage(tooltip_hwnd,TTM_ADDTOOL,0,(LPARAM)&ti);
  //SendMessage(tooltip_hwnd,TTM_SETTITLE,1,(LPARAM)work.title[i].c_str());
  SendMessage(tooltip_hwnd,TTM_TRACKPOSITION,0,MAKELONG(pt.x,pt.y+20));
  SendMessage(tooltip_hwnd,TTM_TRACKACTIVATE,1,(LPARAM)&ti);
}

void GuiCtrlTooltip::hide(int i)
{
  if (tooltip_hwnd)
    DestroyWindow(tooltip_hwnd);

  tooltip_hwnd = NULL;
}
