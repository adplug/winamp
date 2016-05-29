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

class MyPlayer
{
 public:

  int		play(const char *fname);
  void		stop();

  void		pause();
  void		unpause();

  void		seek(int pos);

  const char *	get_file();
  CPlayer *	get_player();
  int		get_position();
  int		get_length();
  int		get_length(const char *fname, int subsong);
  int		get_subsong();

  int		is_playing();
  int		is_paused();

  void		set_subsong(int subsong);
  void		set_panning(int pan);
  void		set_volume(int vol);

 private:

  Copl *	opl_init();
  void		opl_done();

  bool		output_init();
  void		output_done();

  bool		thread_init();
  void		thread_done();

  const char *	get_diskfile(string fname);

  static DWORD WINAPI	callback_emuts(LPVOID lpParameter);
  static void CALLBACK	callback_opl2(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2);
  static DWORD WINAPI	callback_disk(LPVOID lpParameter);

  t_config_data		work;

  struct t_player_data {
    string		fname;
    int			playing;
    int			paused;
    unsigned int	subsong;
    unsigned int	maxsubsong;
    float		outtime;
    unsigned long	fulltime;
    int			seek;
    int			volume;
  } plr;

  struct t_output_data {
    Copl *		emu;
    CDiskopl *		disk;
  } output;

  union {
    HANDLE		emuts;
    UINT		opl2;
    HANDLE		disk;
  } thread;

  CPlayer *	player;

  HMIDIOUT	midiout;

  TIMECAPS	tc;
  int		maxlatency;
  float		refresh;
  string	diskfile;
};
