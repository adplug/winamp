/*
  Copyright (c) 1999 - 2007 Simon Peter <dn.tlp@gmx.net>
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

#ifdef DEBUG
#	include "debug.h"
#endif

HINSTANCE		myInstance;
HWND			*myWindow;

extern In_Module	mod;

Config			config;
FileTypes		filetypes;
MyPlayer		my_player;
GuiDlgAbout		dlg_about;
GuiDlgConfig		dlg_config;
GuiDlgInfo		dlg_info;

void wa2_Init()
{
  myInstance = mod.hDllInstance;
  myWindow = &mod.hMainWindow;
}

void wa2_Quit()
{
  free(mod.FileExtensions);
  config.set_ignored(filetypes.get_ignore_list());
  config.save();
}

int wa2_IsOurFile(char *fn)
{
  // ignored ?
  if (filetypes.grata(fn))
    {
      CSilentopl silent;

      // try to make adplug player
      CPlayer *p = CAdPlug::factory(fn,&silent);

      if (p)
	{
	  delete p;
	  return 1;
	}
    }

  return 0;
}

void wa2_GetFileInfo(char *file, char *title, int *length_in_ms)
{
  const char *my_file;

  // current file ?
  if ((!file) || (!(*file)))
    my_file = my_player.get_file();
  else
    {
      // our file ?
      if (!wa2_IsOurFile(file))
	return;

      my_file = file;
    }

  // set default info
  if (title)
    strcpy(title,strrchr(my_file,'\\')+1);

  if (length_in_ms)
    *length_in_ms = 0;

  // try to get real info
  CSilentopl silent;

  CPlayer *p = CAdPlug::factory(my_file,&silent);

  if (p)
    {
      if (title)
	if (!p->gettitle().empty())
	  strcpy(title,p->gettitle().c_str());

      if (length_in_ms)
	if (file)
	  *length_in_ms = my_player.get_length(my_file,my_player.get_subsong());
	else
	  *length_in_ms = my_player.get_length(my_file,DFL_SUBSONG);

      delete p;
    }
}

int wa2_Play(char *fn)
{
  return my_player.play(fn);
}

void wa2_Pause()
{
  my_player.pause();
}

void wa2_UnPause()
{
  my_player.unpause();
}

int wa2_IsPaused()
{
  return my_player.is_paused();
}

void wa2_Stop()
{
  my_player.stop();
}

int wa2_GetLength()
{
  return my_player.get_length();
}

int wa2_GetOutputTime()
{
  return my_player.get_position();
}

void wa2_SetOutputTime(int time_in_ms)
{
  my_player.seek(time_in_ms);
}

void wa2_SetVolume(int volume)
{
  my_player.set_volume(volume);
}

void wa2_SetPan(int pan)
{
  my_player.set_panning(pan);
}

void wa2_EqSet(int on, char data[10], int preamp)
{
  return;
}

void wa2_DlgAbout(HWND hwndParent)
{
  dlg_about.open(hwndParent);
}

void wa2_DlgConfig(HWND hwndParent)
{
  dlg_config.open(hwndParent);
}

int wa2_DlgInfo(char *file, HWND hwndParent)
{
  return dlg_info.open(file,hwndParent);
}

In_Module mod =
  {
    IN_VER,
    PLUGIN_VER,
    0,
    0,
    0,
    1,
    1,
    wa2_DlgConfig,
    wa2_DlgAbout,
    wa2_Init,
    wa2_Quit,
    wa2_GetFileInfo,
    wa2_DlgInfo,
    wa2_IsOurFile,
    wa2_Play,
    wa2_Pause,
    wa2_UnPause,
    wa2_IsPaused,
    wa2_Stop,
    wa2_GetLength,
    wa2_GetOutputTime,
    wa2_SetOutputTime,
    wa2_SetVolume,
    wa2_SetPan,
    0,0,0,0,0,0,0,0,0,
    0,0,
    wa2_EqSet,
    0,
    0
  };

extern "C" __declspec(dllexport) In_Module *winampGetInModule2()
{
  t_config_data	cfg;

#ifdef DEBUG
  debug_init();
#endif
  config.load();
  config.get(&cfg);

  mod.UsesOutputPlug = config.useoutputplug;

  filetypes.add("a2m", "Adlib Tracker 2 Modules (*.A2M)");
  filetypes.add("adl", "Westwood ADL Files (*.ADL)");
  filetypes.add("amd", "AMUSIC Modules (*.AMD)");
  filetypes.add("bam", "Bob's Adlib Music Files (*.BAM)");
  filetypes.add("cff", "BoomTracker 4 Modules (*.CFF)");
  filetypes.add("cmf", "Creative Adlib Music Files (*.CMF)");
  filetypes.add("cmf", "SoundFX Macs Opera (*.CMF)");
  filetypes.add("d00", "Packed EdLib Modules (*.D00)");
  filetypes.add("dfm", "Digital-FM Modules (*.DFM)");
  filetypes.add("dmo", "TwinTeam Modules (*.DMO)");
  filetypes.add("dro", "DOSBox Raw OPL v1.0 and v2.0 Files (*.DRO)");
  filetypes.add("dtm", "DeFy Adlib Tracker Modules (*.DTM)");
  filetypes.add("got", "God of Thunder Music (*.GOT)");
  filetypes.add("hsc", "HSC-Tracker Modules (*.HSC)");
  filetypes.add("hsp", "Packed HSC-Tracker Modules (*.HSP)");
  filetypes.add("imf;wlf;adlib", "Apogee IMF Files (*.IMF;*.WLF;*.ADLIB)");
  filetypes.add("ims", "IMPlay Song Files (*.IMS)");
  filetypes.add("jbm", "JBM Adlib Music Files (*.JBM)");
  filetypes.add("ksm", "Ken Silverman's Music Files (*.KSM)");
  filetypes.add("laa", "LucasArts Adlib Audio Files (*.LAA)");
  filetypes.add("lds", "LOUDNESS Sound System Files (*.LDS)");
  filetypes.add("m", "Ultima 6 Music Files (*.M)");
  filetypes.add("mad", "Mlat Adlib Tracker Modules (*.MAD)");
  filetypes.add("mdi", "AdLib MIDIPlay Files (*.MDI)");
  filetypes.add("mid;kar", "MIDI Audio Files (*.MID;*.KAR)");
  filetypes.add("mkj", "MKJamz Audio Files (*.MKJ)");
  filetypes.add("msc", "AdLib MSCplay (*.MSC)");
  filetypes.add("mtk", "MPU-401 Trakker Modules (*.MTK)");
  filetypes.add("mus", "AdLib MIDI Music Files (*.MUS)");
  filetypes.add("rad", "Reality Adlib Tracker Modules (*.RAD)");
  filetypes.add("raw", "Rdos RAW Files (*.RAW)");
  filetypes.add("rix", "Softstar RIX OPL Music Files (*.RIX)");
  filetypes.add("rol", "Adlib Visual Composer Modules (*.ROL)");
  if(!cfg.s3m_workaround) {
    filetypes.add("s3m", "Scream Tracker 3 Modules (*.S3M)");
  }
  filetypes.add("sa2", "Surprise! Adlib Tracker 2 Modules (*.SA2)");
  filetypes.add("sat", "Surprise! Adlib Tracker Modules (*.SAT)");
  filetypes.add("sci", "Sierra Adlib Audio Files (*.SCI)");
  filetypes.add("sng", "Adlib Tracker 1.0 Modules (*.SNG)");
  filetypes.add("sng", "Faust Music Creator Modules (*.SNG)");
  filetypes.add("sng", "SNGPlay Files (*.SNG)");
  filetypes.add("xad", "eXotic Adlib Files (*.XAD)");
  filetypes.add("xms", "XMS-Tracker Modules (*.XMS)");
  filetypes.add("xsm", "eXtra Simple Music Files (*.XSM)");

  filetypes.set_ignore_list(config.get_ignored());

  mod.FileExtensions = filetypes.export_filetypes((char *)malloc(4096));

  return &mod;
}
