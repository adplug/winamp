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

class FileTypes
{
	public:

		void				add(const char *type, const char *name, bool ignore = false);

		char *				export(char *buf);

		bool				grata(const char *fname);

		int					get_size();

		bool				get_ignore(int i);
		void				set_ignore(int i, bool val);

		const char *		get_ignore_list();
		void				set_ignore_list(const char *ignore_list);

		const char *		get_name(int i);

	private:

		struct t_filetype_data
		{
			vector<string>	type;
			vector<string>	name;
			vector<bool>	ignore;
		} work;

		string				xstrlist;
};
