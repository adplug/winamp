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
#pragma comment(lib, "shlwapi.lib")

#define MSGA_WINAMP	"You must now restart Winamp after switching from Disk Writer to Emulator output mode."
#define MSGC_DISK 	"You have selected *full speed* and *endless* Disk Writing modes. This combination of options is not recommended."
#define MSGC_DATABASE	"An external Database could not be loaded!"
#define MSGE_XMPLAY	"Hardware OPL2 output is not supported when this plugin is used within XMPlay. An emulator must be used for output, instead."

#define DFL_EMU			emuwo
#define DFL_REPLAYFREQ		44100
#define DFL_HARMONIC		true
#define DFL_USE16BIT		true
#define DFL_STEREO		true
#define DFL_USEOUTPUT		DFL_EMU
#define DFL_USEOUTPUT_ALT	emunone
#define DFL_TESTLOOP		true
#define DFL_SUBSEQ		true
#define DFL_PRIORITY		4
#define DFL_STDTIMER		true
#define DFL_DISKDIR		".\\"
#define DFL_IGNORED		"19;"
#define DFL_DBFILE		"adplug.db"
#define DFL_USEDB		true
#define DFL_S3M_WORKAROUND	true

CAdPlugDatabase *Config::mydb = 0;

Config::Config()
{
  useoutputplug = true;
}

void Config::load()
{
  char bufstr[MAX_PATH+1], dbfile[MAX_PATH], curdir[MAX_PATH + 1];

  // get default path to .ini file
  GetModuleFileName(NULL,bufstr,MAX_PATH);

  _strlwr(strrchr(bufstr,'\\'));

  fname.assign(bufstr);
  fname.resize(fname.size() - 3);
  fname.append("ini");

  // load configuration from .ini file
  int bufval;

  bufval = GetPrivateProfileInt("in_adlib","replayfreq",DFL_REPLAYFREQ,fname.c_str());
  if (bufval != -1)
    next.replayfreq = bufval;

  bufval = GetPrivateProfileInt("in_adlib","harmonic",DFL_HARMONIC,fname.c_str());
  if (bufval != -1)
    next.harmonic = bufval ? true : false;

  bufval = GetPrivateProfileInt("in_adlib","use16bit",DFL_USE16BIT,fname.c_str());
  if (bufval != -1)
    next.use16bit = bufval ? true : false;

  bufval = GetPrivateProfileInt("in_adlib","stereo",DFL_STEREO,fname.c_str());
  if (bufval != -1)
    next.stereo = bufval ? true : false;

  bufval = GetPrivateProfileInt("in_adlib","useoutput",DFL_USEOUTPUT,fname.c_str());
  if (bufval != -1)
    next.useoutput = (enum t_output)bufval;

  bufval = GetPrivateProfileInt("in_adlib","useoutput_alt",DFL_USEOUTPUT_ALT,fname.c_str());
  if (bufval != -1)
    next.useoutput_alt = (enum t_output)bufval;

  if (GetCurrentDirectory(MAX_PATH, curdir))
  {
    strcat(curdir, "\\");
    GetPrivateProfileString("in_adlib", "diskdir", curdir, bufstr, MAX_PATH, fname.c_str());
    if (PathIsDirectory(bufstr))
      next.diskdir = bufstr;
    else
      next.diskdir = curdir;
  }
  else
    next.diskdir = DFL_DISKDIR;

  bufval = GetPrivateProfileInt("in_adlib","testloop",DFL_TESTLOOP,fname.c_str());
  if (bufval != -1)
    next.testloop = bufval ? true : false;

  bufval = GetPrivateProfileInt("in_adlib","subseq",DFL_SUBSEQ,fname.c_str());
  if (bufval != -1)
    next.subseq = bufval ? true : false;

  bufval = GetPrivateProfileInt("in_adlib","priority",DFL_PRIORITY,fname.c_str());
  if (bufval != -1)
    next.priority = bufval;

  bufval = GetPrivateProfileInt("in_adlib","stdtimer",DFL_STDTIMER,fname.c_str());
  if (bufval != -1)
    next.stdtimer = bufval ? true : false;

  GetPrivateProfileString("in_adlib","ignored",DFL_IGNORED,bufstr,MAX_PATH,fname.c_str());
  next.ignored = bufstr;

  // Build database default path (in winamp plugin directory)
  GetModuleFileName(GetModuleHandle("in_adlib"), dbfile, MAX_PATH);
  strcpy(strrchr(dbfile, '\\') + 1, DFL_DBFILE);

  GetPrivateProfileString("in_adlib","database",dbfile,bufstr,MAX_PATH,fname.c_str());
  next.db_file = bufstr;

  bufval = GetPrivateProfileInt("in_adlib","usedb",DFL_USEDB,fname.c_str());
  if (bufval != -1)
    next.usedb = bufval ? true : false;

  bufval = GetPrivateProfileInt("in_adlib","s3mworkaround",DFL_S3M_WORKAROUND,fname.c_str());
  if (bufval != -1) next.s3m_workaround = bufval ? true : false;

  apply(false);
}

void Config::save()
{
  char bufstr[11];

  WritePrivateProfileString("in_adlib","replayfreq",_itoa(next.replayfreq,bufstr,10),fname.c_str());
  WritePrivateProfileString("in_adlib","harmonic",_itoa(next.harmonic,bufstr,10),fname.c_str());
  WritePrivateProfileString("in_adlib","use16bit",_itoa(next.use16bit,bufstr,10),fname.c_str());
  WritePrivateProfileString("in_adlib","stereo",_itoa(next.stereo,bufstr,10),fname.c_str());
  WritePrivateProfileString("in_adlib","useoutput",_itoa(next.useoutput,bufstr,10),fname.c_str());
  WritePrivateProfileString("in_adlib","useoutput_alt",_itoa(next.useoutput_alt,bufstr,10),fname.c_str());
  WritePrivateProfileString("in_adlib","testloop",_itoa(next.testloop,bufstr,10),fname.c_str());
  WritePrivateProfileString("in_adlib","subseq",_itoa(next.subseq,bufstr,10),fname.c_str());
  WritePrivateProfileString("in_adlib","priority",_itoa(next.priority,bufstr,10),fname.c_str());
  WritePrivateProfileString("in_adlib","stdtimer",_itoa(next.stdtimer,bufstr,10),fname.c_str());
  WritePrivateProfileString("in_adlib","diskdir",next.diskdir.c_str(),fname.c_str());
  WritePrivateProfileString("in_adlib","ignored",next.ignored.c_str(),fname.c_str());
  WritePrivateProfileString("in_adlib","database",next.db_file.c_str(),fname.c_str());
  WritePrivateProfileString("in_adlib","usedb",_itoa(next.usedb,bufstr,10),fname.c_str());
  WritePrivateProfileString("in_adlib","s3mworkaround",_itoa(next.s3m_workaround,bufstr,10),fname.c_str());
}

void Config::check()
{
  switch (next.useoutput) {
  case emuts:
  case emuks:
  case emuwo:
    next.useoutputplug = true;
    break;
  default:
    next.useoutputplug = false;
    break;
  }

  if (!next.useoutputplug)
    if (test_xmplay())
      {
	next.useoutput = DFL_EMU;
	next.useoutputplug = true;

	MessageBox(NULL,MSGE_XMPLAY,"AdPlug :: Error",MB_ICONERROR | MB_TASKMODAL);
      }

  if (next.useoutput == disk)
    if (!next.stdtimer)
      if (!next.testloop)
	MessageBox(NULL,MSGC_DISK,"AdPlug :: Caution",MB_ICONWARNING | MB_TASKMODAL);

  if (next.useoutputplug > useoutputplug)
    MessageBox(NULL,MSGA_WINAMP,"AdPlug :: Attention",MB_ICONINFORMATION | MB_TASKMODAL);

  if (!use_database())
    MessageBox(NULL,MSGC_DATABASE,"AdPlug :: Caution",MB_ICONWARNING | MB_TASKMODAL);
}

void Config::apply(bool testout)
{
  check();

  work.replayfreq	= next.replayfreq;
  work.harmonic		= next.harmonic;
  work.use16bit		= next.use16bit;
  work.stereo		= next.stereo;
  work.testloop		= next.testloop;
  work.subseq		= next.subseq;
  work.priority		= next.priority;
  work.stdtimer		= next.stdtimer;
  work.diskdir		= next.diskdir;
  work.ignored		= next.ignored;
  work.db_file		= next.db_file;
  work.usedb		= next.usedb;
  work.s3m_workaround	= next.s3m_workaround;

  if (!testout || (next.useoutputplug <= useoutputplug))
    {
      work.useoutput = next.useoutput;
      work.useoutput_alt = next.useoutput_alt;
      useoutputplug  = next.useoutputplug;
    }
}

void Config::get(t_config_data *cfg)
{
  *cfg = work;
}

void Config::set(t_config_data *cfg)
{
  next = *cfg;

  apply(true);
}

const char *Config::get_ignored()
{
  return work.ignored.c_str();
}

void Config::set_ignored(const char *ignore_list)
{
  next.ignored = ignore_list;
}

bool Config::use_database()
{
  bool success = true;

  if(mydb) { delete mydb; mydb = 0; }
  if(next.usedb) {
    mydb = new CAdPlugDatabase;
    success = mydb->load(next.db_file);
  }
  CAdPlug::set_database(mydb);

  return success;
}

bool Config::test_xmplay()
{
  return GetModuleHandle("xmplay.exe") ? true : false;
}
