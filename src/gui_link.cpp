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

enum     HL_STATE     { HL_NORMAL=0,  HL_HOVER,     HL_WARM      };
COLORREF HL_COLOR[] = { RGB(0,0,188), RGB(0,0,255), RGB(222,0,0) };

void GuiCtrlHyperlink::add(HWND hwnd, const char *text, const char *link)
{
	// add hyperlink data to list
	work.hwnd.push_back(hwnd);
	work.proc.push_back((WNDPROC)GetWindowLong(hwnd,GWL_WNDPROC));
	work.text.push_back(text);
	work.link.push_back(link);
	work.state.push_back(HL_NORMAL);

	// change window parameters
	SetWindowText(hwnd,"");
	SetWindowLong(hwnd,GWL_USERDATA,(LONG)this);
	SetWindowLong(hwnd,GWL_WNDPROC,(LONG)WndProc);
}

void GuiCtrlHyperlink::remove(int i)
{
	work.hwnd.erase(work.hwnd.begin()+i);
	work.proc.erase(work.proc.begin()+i);
	work.text.erase(work.text.begin()+i);
	work.link.erase(work.link.begin()+i);
	work.state.erase(work.state.begin()+i);
}

LRESULT CALLBACK GuiCtrlHyperlink::WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
#ifdef _DEBUG
	//printf("GuiCtrlHyperlink::WndProc(): Message 0x%08X received.\n",message);
#endif
	RECT rect;
	POINT pt;

	// get our object pointer
	GuiCtrlHyperlink *the = (GuiCtrlHyperlink *)GetWindowLong(hwnd,GWL_USERDATA);

	// get our index
	int i = 0;

	while (the->work.hwnd[i] != hwnd)
		i++;

	// yeah!
	switch (message)
	{
		case WM_CAPTURECHANGED:
			the->set_normal(i);
			return 0;

		case WM_LBUTTONDOWN:
			the->set_warm(i);
			return 0;

		case WM_LBUTTONUP:
			if (the->is_warm(i))
			  the->goto_link(i);
			return 0;

		case WM_MOUSEMOVE:
			GetClientRect(hwnd,&rect);
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			if (!PtInRect(&rect,pt))
			  ReleaseCapture();
			return 0;

		case WM_NCHITTEST:
			return HTCLIENT;

		case WM_PAINT:
			the->paint(i,true);
			return 0;

		case WM_SETCURSOR:
			SetCapture(hwnd);
			the->set_hover(i);
			return 0;
	}

	// default WinProc()
	LRESULT result = CallWindowProc(the->work.proc[i],hwnd,message,wParam,lParam);

	// window destruction ?
	if (message == WM_NCDESTROY)
	{
		the->remove(i);
	}

	return result;
}

void GuiCtrlHyperlink::goto_link(int i)
{
	ShellExecute(NULL,"open","iexplore",work.link[i].c_str(),NULL,SW_SHOWDEFAULT);
}

bool GuiCtrlHyperlink::is_warm(int i)
{
	return (work.state[i] == HL_WARM);
}

void GuiCtrlHyperlink::paint(int i, bool msg)
{
	HWND hwnd;
	RECT rect;
	PAINTSTRUCT ps;
	HDC hdc;
	HFONT hfont;
	LOGFONT lf;
	COLORREF clr;

	hwnd = work.hwnd[i];
	clr  = HL_COLOR[work.state[i]];

	// get client area
	GetClientRect(hwnd,&rect);

	if (msg)
		hdc = BeginPaint(hwnd,&ps);
	else
		hdc = GetDC(hwnd);

	// set underlined font
	GetObject(GetStockObject(DEFAULT_GUI_FONT),sizeof(LOGFONT),&lf);
	lf.lfUnderline = TRUE;
	hfont = CreateFontIndirect(&lf);
	SelectObject(hdc,hfont);

	// set hyperlink text
	SetTextColor(hdc,clr);
	SetBkColor(hdc,GetSysColor(COLOR_BTNFACE));
	TextOut(hdc,0,0,work.text[i].c_str(),work.text[i].size());

	DeleteObject(hfont);

	if (msg)
		EndPaint(hwnd,&ps);
	else
		ReleaseDC(hwnd,hdc);
}

void GuiCtrlHyperlink::set_normal(int i)
{
	work.state[i] = HL_NORMAL;
	SetCursor(LoadCursor(NULL,IDC_ARROW));
	paint(i);
}

void GuiCtrlHyperlink::set_hover(int i)
{
	work.state[i] = HL_HOVER;
	SetCursor(LoadCursor(NULL,/*IDC_HAND*/ IDC_ARROW));
	paint(i);
}

void GuiCtrlHyperlink::set_warm(int i)
{
	work.state[i] = HL_WARM;
	paint(i);
}
