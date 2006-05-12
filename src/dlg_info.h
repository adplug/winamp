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

class GuiDlgInfo
{
 public:

  GuiDlgInfo();

  int	open(const char *file, HWND parent);

  void	dock();
  void	undock();

  void	update(bool all = false);

 private:

  static BOOL APIENTRY	DlgProc_Wrapper(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

  BOOL APIENTRY		DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

  string	fileinfo_file;
  CPlayer *	fileinfo_player;
  HWND		fileinfo_hwnd;

  unsigned int	fi_order;
  unsigned int	fi_pattern;
  unsigned int	fi_row;
  unsigned int	fi_speed;
  float		fi_refresh;

  CSilentopl	silent;
};
