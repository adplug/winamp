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

class GuiDlgAbout
{
	public:

		void					open(HWND parent);

	private:

		static BOOL APIENTRY	DlgProc_Wrapper(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
		static BOOL APIENTRY	TabDlgProc_Wrapper(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

		BOOL APIENTRY			DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
		BOOL APIENTRY			TabDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

		HWND					tab_hwnd;
		int						tab_index;
};
