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

extern HWND *myWindow;
extern In_Module mod;
extern Config config;
extern GuiDlgInfo dlg_info;

int thread_priority[] = {
  THREAD_PRIORITY_IDLE,
  THREAD_PRIORITY_LOWEST,
  THREAD_PRIORITY_BELOW_NORMAL,
  THREAD_PRIORITY_NORMAL,
  THREAD_PRIORITY_ABOVE_NORMAL,
  THREAD_PRIORITY_HIGHEST,
  THREAD_PRIORITY_TIME_CRITICAL
};

int MyPlayer::play(const char *fname)
{
  Copl *opl;

  config.get(&work);

  // new file ?
  if (plr.fname.compare(fname))
    {
      plr.subsong = DFL_SUBSONG;
      plr.fname = fname;
    }

  // init opl
  opl = opl_init();
  if (!opl)
    return 2;

  // init adplug player
  player = CAdPlug::factory(plr.fname.c_str(),opl);
  if (!player)
    {
      opl_done();
      return -1;
    }

  // init output
  if (!output_init())
    {
      delete player;
      opl_done();
      return 2;
    }

  // init player data
  plr.playing = 1;
  plr.paused = 0;
  plr.maxsubsong = player->getsubsongs();
  plr.outtime = 0.0f;
  plr.fulltime = get_length(plr.fname.c_str(),plr.subsong);
  plr.seek = -1;

  // rewind song
  player->rewind(plr.subsong);

  // set winamp info
  mod.SetInfo(120000,work.replayfreq/1000,(work.stereo ? 2 : 1),1);

  // init thread
  if (!thread_init())
    {
      plr.playing = 0;
      output_done();
      delete player;
      opl_done();
      return 2;
    }

  dlg_info.dock();	
	
  return 0;
}

void MyPlayer::stop()
{
  if (!plr.playing)
    return;

  plr.playing = 0;

  thread_done();
  output_done();

  dlg_info.undock();

  delete player;
  opl_done();
}

int MyPlayer::is_playing()
{
  return plr.playing;
}

void MyPlayer::pause()
{
  plr.paused = 1;
}

void MyPlayer::unpause()
{
  plr.paused = 0;
}

int MyPlayer::is_paused()
{
  return plr.paused;
}

const char *MyPlayer::get_file()
{
  return plr.fname.c_str();
}

CPlayer *MyPlayer::get_player()
{
  return player;
}

int MyPlayer::get_subsong()
{
  return plr.subsong;
}

void MyPlayer::set_subsong(int subsong)
{
  plr.subsong = subsong;

  SendMessage(*myWindow,WM_COMMAND,WINAMP_BUTTON2,0);
}

int MyPlayer::get_length()
{
  return plr.fulltime;
}

int MyPlayer::get_length(const char *fname, int subsong)
{
  CSilentopl silent;

  CPlayer *p = CAdPlug::factory(fname,&silent);
  if (!p)
    return 0;

  int fulltime = p->songlength(subsong);

  delete p;

  return fulltime;
}

int MyPlayer::get_position()
{
  int outtime;

  switch (work.useoutput)
    {
    case emuts:
    case emuks:
      outtime = mod.outMod->GetOutputTime();
      break;
    case disk:
      outtime = (int)plr.outtime;
      break;
    }

  return outtime;
}

void MyPlayer::seek(int pos)
{
  plr.seek = pos;
}

void MyPlayer::set_volume(int vol)
{
  if (!plr.playing)
    config.get(&work);

  plr.volume = (int)(63 - vol/(255/63));

  switch (work.useoutput)
    {
    case emuts:
    case emuks:
      mod.outMod->SetVolume(vol);
      break;
    case disk:
      // disk writer does not obey volume control
      break;
    }
}

void MyPlayer::set_panning(int pan)
{
  if (!plr.playing)
    config.get(&work);

  switch (work.useoutput)
    {
    case emuts:
    case emuks:
      mod.outMod->SetPan(pan);
      break;

    case disk:
      // Disk writer does not support panning
      break;
    }
}

Copl *MyPlayer::opl_init()
{
  Copl *opl = NULL;

  if (work.useoutput == emuts)

  if (work.harmonic == true) {
    Copl *a = new CEmuopl(work.replayfreq, work.use16bit, false);
    Copl *b = new CEmuopl(work.replayfreq, work.use16bit, false);
    opl = output.emu = new CSurroundopl(a, b, work.use16bit);
    // CSurroundopl now owns "a" and "b", and will free it upon destruction.
  } else {
    opl = output.emu = new CEmuopl(work.replayfreq, work.use16bit, work.stereo);
  }

  if (work.useoutput == emuks)

  if (work.duelsynth == true) {
    Copl *a = new CEmuopl(work.replayfreq, work.use16bit, false);
    Copl *b = new CKemuopl(work.replayfreq, work.use16bit, false);
    opl = output.emu = new CSurroundopl(a, b, work.use16bit);
    // CSurroundopl now owns "a" and "b", and will free it upon destruction.
  } else {
    opl = output.emu = new CKemuopl(work.replayfreq,work.use16bit,work.stereo);
  }

  if (work.useoutput == disk)
    opl = output.disk = new CDiskopl(get_diskfile(plr.fname));

  return opl;
}

void MyPlayer::opl_done()
{
  if (work.useoutput == emuts)
    delete output.emu;
  if (work.useoutput == emuks)
    delete output.emu;
  if (work.useoutput == disk)
    delete output.disk;
}

bool MyPlayer::output_init()
{
  switch (work.useoutput)
    {
    case emuts:
    case emuks:
      maxlatency = mod.outMod->Open(work.replayfreq,(work.stereo ? 2 : 1),(work.use16bit ? 16 : 8),-1,-1);
      if (maxlatency < 0)
	return false;
      mod.outMod->SetVolume(-666);
      mod.SAVSAInit(maxlatency,work.replayfreq);
      mod.VSASetInfo((work.stereo ? 2 : 1),work.replayfreq);
      break;
    case disk:
      // no init necessary
      break;
    }

  return true;
}

void MyPlayer::output_done()
{
  switch (work.useoutput)
    {
    case emuts:
    case emuks:
      mod.SAVSADeInit();
      mod.outMod->Close();
      break;
    case disk:
      // no deinit necessary
      break;
    }
}

bool MyPlayer::thread_init()
{
  DWORD tmpdword;

  switch (work.useoutput)
    {
    case emuts:
    case emuks:
      thread.emuts = (HANDLE)CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)callback_emuts,(void *)this,0,&tmpdword);
      if (!thread.emuts)
	return false;
      SetThreadPriority(thread.emuts,thread_priority[work.priority]);
      break;
    case disk:
      thread.disk = (HANDLE)CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)callback_disk,(void *)this,0,&tmpdword);
      if (!thread.disk)
	return false;
      SetThreadPriority(thread.disk,thread_priority[work.priority]);
      break;
    }

  return true;
}

void MyPlayer::thread_done()
{
  switch (work.useoutput)
    {
    case emuts:
    case emuks:
      if (WaitForSingleObject(thread.emuts,(DWORD)(7*1000/player->getrefresh())) == WAIT_TIMEOUT)
	TerminateThread(thread.emuts,0);
      CloseHandle(thread.emuts);
      break;
    case disk:
      if (WaitForSingleObject(thread.disk,(DWORD)(7*1000/player->getrefresh())) == WAIT_TIMEOUT)
	TerminateThread(thread.disk,0);
      CloseHandle(thread.disk);
      break;
    }
}

const char *MyPlayer::get_diskfile(string fname)
{
  char bufstr[11];

  diskfile.assign(work.diskdir);
  diskfile.append(strrchr(fname.c_str(),'\\'));

  if (plr.subsong)
    {
      _itoa(plr.subsong,bufstr,10);

      diskfile.append(".");
      diskfile.append(bufstr);
    }

  diskfile.append(".raw");

  return diskfile.c_str();
}
