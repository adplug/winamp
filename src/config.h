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

#include <string>

#define DFL_SUBSONG 0
#define DFL_SNDBUFSIZE 576

enum t_output
{
	emuts,
	emuks,
	opl2,
	disk
};

struct t_config_data
{
	int            replayfreq;
	bool           use16bit;
	bool           stereo;
	enum t_output  useoutput;
	unsigned short adlibport;
	bool           testopl2;
	bool           testloop;
	bool           fastseek;
	int            priority;
	int            stdtimer;
	string         diskdir;
	string         ignored;
	bool           useoutputplug;
	string         db_file;
	bool           usedb;
	bool           s3m_workaround;
};

class Config
{
	public:

		Config();

		void			load();
		void			save();

		void			get(t_config_data *cfg);
		void			set(t_config_data *cfg);

		const char *	get_ignored();
		void			set_ignored(const char *ignore_list);

		bool			useoutputplug;

	private:

		void			apply(bool testout);
		bool			use_database();

		void			check();

		bool			test_opl2();
		bool			test_winnt();
		bool			test_xmplay();
		bool			test_porttalk();

		string					fname;
		static CAdPlugDatabase	*mydb;

		t_config_data	work, next;
};
