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
#include <winioctl.h>

#define MSGA_WINAMP    "You must restart Winamp after switching from own to standard output."
#define MSGC_DISK      "You selected GODSPEED ENDLESS Disk Writing mode."
#define	MSGE_OPL2      "OPL2 chip on given port was not detected." "\n\n" \
                       "Emulated output forced."
#define MSGE_WINNT     "You can't use OPL2 output under Windows NT/2000/XP." "\n\n" \
                       "Emulated output forced."
#define MSGE_XMPLAY    "You can't use own output under XMPlay." "\n\n" \
                       "Emulated output forced."

#define DFL_EMU        emuts
#define DFL_REPLAYFREQ 44100
#define DFL_USE16BIT   true
#define DFL_STEREO     false
#define DFL_USEOUTPUT  DFL_EMU
#define DFL_ADLIBPORT  0x388
#define DFL_TESTOPL2   true
#define DFL_TESTLOOP   true
#define DFL_FASTSEEK   false
#define DFL_PRIORITY   4
#define DFL_STDTIMER   true
#define DFL_DISKDIR    "C:\\"
#define DFL_IGNORED    "15;"

Config::Config()
{
	useoutputplug = true;
}

void Config::load()
{
	char bufstr[MAX_PATH+1];

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

	bufval = GetPrivateProfileInt("in_adlib","use16bit",DFL_USE16BIT,fname.c_str());
	if (bufval != -1)
		next.use16bit = bufval ? true : false;

	bufval = GetPrivateProfileInt("in_adlib","stereo",DFL_STEREO,fname.c_str());
	if (bufval != -1)
		next.stereo = bufval ? true : false;

	bufval = GetPrivateProfileInt("in_adlib","useoutput",DFL_USEOUTPUT,fname.c_str());
	if (bufval != -1)
		next.useoutput = (enum t_output)bufval;

	GetPrivateProfileString("in_adlib","adlibport","0",bufstr,5,fname.c_str());
	if (strcmp(bufstr,"0"))
		sscanf(bufstr,"%x",&next.adlibport);
	else
		next.adlibport = DFL_ADLIBPORT;

	bufval = GetPrivateProfileInt("in_adlib","testopl2",DFL_TESTOPL2,fname.c_str());
	if (bufval != -1)
		next.testopl2 = bufval ? true : false;

	GetPrivateProfileString("in_adlib","diskdir",DFL_DISKDIR,bufstr,MAX_PATH,fname.c_str());
	if (SetCurrentDirectory(bufstr))
		next.diskdir = bufstr;
	else
		next.diskdir = DFL_DISKDIR;

	bufval = GetPrivateProfileInt("in_adlib","testloop",DFL_TESTLOOP,fname.c_str());
	if (bufval != -1)
		next.testloop = bufval ? true : false;

	bufval = GetPrivateProfileInt("in_adlib","fastseek",DFL_FASTSEEK,fname.c_str());
	if (bufval != -1)
		next.fastseek = bufval ? true : false;

	bufval = GetPrivateProfileInt("in_adlib","priority",DFL_PRIORITY,fname.c_str());
	if (bufval != -1)
		next.priority = bufval;

	bufval = GetPrivateProfileInt("in_adlib","stdtimer",DFL_STDTIMER,fname.c_str());
	if (bufval != -1)
		next.stdtimer = bufval ? true : false;

	GetPrivateProfileString("in_adlib","ignored",DFL_IGNORED,bufstr,MAX_PATH,fname.c_str());
		next.ignored = bufstr;

	apply(false);
}

void Config::save()
{
	char bufstr[11];

	WritePrivateProfileString("in_adlib","replayfreq",_itoa(next.replayfreq,bufstr,10),fname.c_str());
	WritePrivateProfileString("in_adlib","use16bit",_itoa(next.use16bit,bufstr,10),fname.c_str());
	WritePrivateProfileString("in_adlib","stereo",_itoa(next.stereo,bufstr,10),fname.c_str());
	WritePrivateProfileString("in_adlib","useoutput",_itoa(next.useoutput,bufstr,10),fname.c_str());
	WritePrivateProfileString("in_adlib","adlibport",_itoa(next.adlibport,bufstr,16),fname.c_str());
	WritePrivateProfileString("in_adlib","testopl2",_itoa(next.testopl2,bufstr,10),fname.c_str());
	WritePrivateProfileString("in_adlib","testloop",_itoa(next.testloop,bufstr,10),fname.c_str());
	WritePrivateProfileString("in_adlib","fastseek",_itoa(next.fastseek,bufstr,10),fname.c_str());
	WritePrivateProfileString("in_adlib","priority",_itoa(next.priority,bufstr,10),fname.c_str());
	WritePrivateProfileString("in_adlib","stdtimer",_itoa(next.stdtimer,bufstr,10),fname.c_str());
	WritePrivateProfileString("in_adlib","diskdir",next.diskdir.c_str(),fname.c_str());
	WritePrivateProfileString("in_adlib","ignored",next.ignored.c_str(),fname.c_str());
}

void Config::check()
{
	if ((next.useoutput == emuts) || (next.useoutput == emuks))
		next.useoutputplug = true;
	else
		next.useoutputplug = false;

	if (!next.useoutputplug)
		if (test_xmplay())
		{
			next.useoutput = DFL_EMU;
			next.useoutputplug = true;

			MessageBox(NULL,MSGE_XMPLAY,"AdPlug :: Error",MB_ICONERROR | MB_TASKMODAL);
		}

	if (next.useoutput == opl2)
		if (test_winnt())
			if (!test_porttalk())
			{
				next.useoutput = DFL_EMU;
				next.useoutputplug = true;

				MessageBox(NULL,MSGE_WINNT,"AdPlug :: Error",MB_ICONERROR | MB_TASKMODAL);
			}

	if (next.useoutput == opl2)
		if (next.testopl2)
		{
			next.useoutput = DFL_EMU;
			next.useoutputplug = true;

			MessageBox(NULL,MSGE_OPL2,"AdPlug :: Error",MB_ICONERROR | MB_TASKMODAL);
		}

	if (next.useoutput == disk)
		if (!next.stdtimer)
			if (!next.testloop)
				MessageBox(NULL,MSGC_DISK,"AdPlug :: Caution",MB_ICONWARNING | MB_TASKMODAL);

	if (next.useoutputplug > useoutputplug)
		MessageBox(NULL,MSGA_WINAMP,"AdPlug :: Attention",MB_ICONINFORMATION | MB_TASKMODAL);
}

void Config::apply(bool testout)
{
	check();

	work.replayfreq = next.replayfreq;
	work.use16bit   = next.use16bit;
	work.stereo     = next.stereo;
	work.adlibport  = next.adlibport;
	work.testopl2   = next.testopl2;
	work.testloop   = next.testloop;
	work.fastseek   = next.fastseek;
	work.priority   = next.priority;
	work.stdtimer   = next.stdtimer;
	work.diskdir    = next.diskdir;
	work.ignored    = next.ignored;

	if (!testout || (next.useoutputplug <= useoutputplug))
	{
		work.useoutput = next.useoutput;
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

void Config::set_ignored(const char *list)
{
	next.ignored = list;
}

bool Config::test_opl2()
{
	CRealopl tmp(next.adlibport);

	return tmp.detect();
}

bool Config::test_winnt()
{
	OSVERSIONINFO ver;

	ver.dwOSVersionInfoSize = sizeof(ver);

	GetVersionEx(&ver);

	if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
		return true;

	return false;
}

bool Config::test_xmplay()
{
	return GetModuleHandle("xmplay.exe") ? true : false;
}

bool Config::test_porttalk()
/* Enables I/O port permissions on Windows NT, using the PortTalk device driver.
 * Returns true on success. Returns false if PortTalk isn't installed.
 */
{
	DWORD BytesReturned, our_pid;
	HANDLE PortTalk_Driver;

	// Try to open PortTalk driver
	if((PortTalk_Driver = CreateFile("\\\\.\\PortTalk",GENERIC_READ,0,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE) {
		puts("porttalk_enable(): PortTalk not installed.");
		return false;
	}

	// Reset I/O permission map (deny all access)
	if(!DeviceIoControl(PortTalk_Driver,
		CTL_CODE(40000, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS),
		NULL,0,NULL,0,&BytesReturned,NULL)) {
		puts("porttalk_enable(): Error on resetting I/O permission map!");
		CloseHandle(PortTalk_Driver);
		return false;
	}

	// Set I/O permission map (exclusive access to all ports)
	if(!DeviceIoControl(PortTalk_Driver,
		CTL_CODE(40000, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS),
		NULL,0,NULL,0,&BytesReturned,NULL)) {
		puts("porttalk_enable(): Error on setting I/O permission map!");
		CloseHandle(PortTalk_Driver);
		return false;
	}

	// Enable I/O permissions on ourself
	our_pid = GetCurrentProcessId();
	printf("porttalk_enable(): Our process ID is %u.\n",our_pid);
	if(!DeviceIoControl(PortTalk_Driver,
		CTL_CODE(40000, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS),
		&our_pid,4,NULL,0,&BytesReturned,NULL)) {
		puts("porttalk_enable(): Error on establishing I/O permissions on our process!");
		CloseHandle(PortTalk_Driver);
		return false;
	}

	CloseHandle(PortTalk_Driver);
	Sleep(1);	// Very important !! Wait for device driver to carry out our requests.
	return true;
}
