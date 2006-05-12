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
extern GuiDlgInfo dlg_info;

DWORD WINAPI MyPlayer::callback_emuts(LPVOID lpParameter)
{
  MyPlayer *the = (MyPlayer *)lpParameter;

  long toadd = 0;
  bool stopped = false;

  // allocate sound buffer (2x for dsp)
  int sampsize = 1;
  if (the->work.use16bit)
    sampsize *= 2;
  if (the->work.stereo)
    sampsize *= 2;
  char *sndbuf = (char *)malloc(DFL_SNDBUFSIZE*sampsize*2);

  /* ! */

  while (the->plr.playing)
    {
      // seek requested ?
      if (the->plr.seek != -1)
	{
	  // backward seek ?
	  if (the->plr.seek < the->plr.outtime)
	    {
	      the->player->rewind(the->plr.subsong);
	      the->plr.outtime = 0.0f;
	    }

	  // seek to needed position
	  while ((the->plr.outtime < the->plr.seek) && the->player->update())
	    the->plr.outtime += 1000/the->player->getrefresh();

	  mod.outMod->Flush((int)the->plr.outtime);

	  the->plr.seek = -1;
	}

      // wait for unpausing
      if (the->plr.paused)
	{
	  mod.outMod->Flush(mod.outMod->GetOutputTime());

	  while (the->plr.paused)
	    Sleep(10);

	  continue;
	}

      // update replayer
      if (stopped && the->work.testloop)
	{
	  if (!mod.outMod->IsPlaying())
	    {
	      PostMessage(*myWindow,WM_WA_MPEG_EOF,0,0);
	      break;
	    }
   
	  Sleep(10);

	  continue;
	}

      // fill sound buffer
      long towrite = DFL_SNDBUFSIZE;
      char *sndbufpos = sndbuf;

      while (towrite > 0)
	{
	  while (toadd < 0)
	    {
	      toadd += the->work.replayfreq;
	      stopped = !the->player->update();
	      the->plr.outtime += 1000/the->player->getrefresh();
	    }

	  long i = min(towrite,(long)(toadd/the->player->getrefresh()+4)&~3);

	  the->output.emu->update((short *)sndbufpos,i);

	  sndbufpos += i * sampsize;
	  towrite -= i;
	  toadd -= (long)(i * the->player->getrefresh());
	}

      // update dsp
      towrite = mod.dsp_dosamples((short *)sndbuf,DFL_SNDBUFSIZE,(the->work.use16bit ? 16 : 8),(the->work.stereo ? 2 : 1),the->work.replayfreq);
      towrite *= sampsize;

      // wait for output plugin
      while (mod.outMod->CanWrite() < towrite)
	Sleep(10);

      // write sound buffer
      mod.outMod->Write(sndbuf,towrite);

      // vis
      mod.SAAddPCMData(sndbuf,(the->work.stereo ? 2 : 1),(the->work.use16bit ? 16 : 8),mod.outMod->GetWrittenTime());
      mod.VSAAddPCMData(sndbuf,(the->work.stereo ? 2 : 1),(the->work.use16bit ? 16 : 8),mod.outMod->GetWrittenTime());

      // update FileInfo
      dlg_info.update();
    }

  free(sndbuf);

  return 0;
}

void CALLBACK MyPlayer::callback_opl2(UINT wTimerID,UINT msg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
  MyPlayer *the = (MyPlayer *)dwUser;

  // paused ?
  if (the->plr.paused)
    return;

  // seek requested ?
  if (the->plr.seek != -1)
    {
      timeKillEvent(the->thread.opl2);

      // backward seek ?
      if (the->plr.seek < the->plr.outtime)
	{
	  the->player->rewind(the->plr.subsong);
	  the->plr.outtime = 0.0f;
	}

      // seek to needed position
      if (the->work.fastseek)
	the->output.real->setnowrite();

      the->output.real->setquiet();

      while ((the->plr.outtime < the->plr.seek) && the->player->update())
	the->plr.outtime += 1000/the->player->getrefresh();

      the->output.real->setquiet(false);

      if (the->work.fastseek)
	the->output.real->setnowrite(false);

      the->plr.seek = -1;

      the->thread.opl2 = timeSetEvent((UINT)(1000/the->player->getrefresh()),0,callback_opl2,(DWORD)the,TIME_PERIODIC);
    }

  // update replayer
  if (!the->player->update() && the->work.testloop)
    {
      PostMessage(*myWindow,WM_WA_MPEG_EOF,0,0);
      return;
    }

  // refresh rate changed ?
  if (the->refresh != the->player->getrefresh())
    {
      timeKillEvent(the->thread.opl2);

      the->refresh = the->player->getrefresh();

      the->thread.opl2 = timeSetEvent((UINT)(1000/the->refresh),0,callback_opl2,(DWORD)the,TIME_PERIODIC);
    }

  // update FileInfo, if needed
  dlg_info.update();

  the->plr.outtime += 1000/the->refresh;
}

DWORD WINAPI MyPlayer::callback_disk(LPVOID lpParameter)
{
  MyPlayer *the = (MyPlayer *)lpParameter;

  while (the->plr.playing)
    if (!the->plr.paused)
      {
	// seek requested ?
	if (the->plr.seek != -1)
	  {
	    // backward seek ?
	    if (the->plr.seek < the->plr.outtime)
	      {
		the->player->rewind(the->plr.subsong);
		the->plr.outtime = 0.0f;
	      }

	    // seek to needed position
	    the->output.disk->setnowrite();

	    while ((the->plr.outtime < the->plr.seek) && the->player->update())
	      the->plr.outtime += 1000/the->player->getrefresh();

	    the->output.disk->setnowrite(false);

	    the->plr.seek = -1;
	  }

	// update disk writer
	the->output.disk->update(the->player);

	// update replayer
	if (!the->player->update() && the->work.testloop)
	  {
	    PostMessage(*myWindow,WM_WA_MPEG_EOF,0,0);
	    break;
	  }

	the->plr.outtime += 1000/the->player->getrefresh();

	// delay, if normal timing
	if (the->work.stdtimer)
	  {
	    // update FileInfo, if needed
	    dlg_info.update();

	    Sleep((DWORD)(1000/the->player->getrefresh()));
	  }
      }

  return 0;
}
