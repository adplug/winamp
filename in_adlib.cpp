/*
  AdPlug Winamp2 frontend

  Copyright (c) 1999 - 2002 Simon Peter <dn.tlp@gmx.net>
  Copyright (c)	2002 Nikita V. Kalaganov <riven@ok.ru>

  This plugin is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This plugin is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this plugin; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <windows.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <shlobj.h>
#include <stdio.h>
#include <string.h>
#include <richedit.h>
#include "resource.h"

#ifdef _DEBUG
  #include "debug.h"
#endif

extern "C"
{
  #include "in2.h"
  #include "frontend.h"
}

#include <adplug/adplug.h>
#include <adplug/emuopl.h>
//#include <adplug/kemuopl.h>
#include <adplug/diskopl.h>
#include <adplug/realopl.h>
#include <adplug/silentopl.h>

        /* outputs */

enum    output              {emuts, emuks, disk, opl2};

        /* constants */

#define PLUGINVER           "AdPlug Winamp2 frontend v1.4"
//
#define SNDBUFSIZE          576
//
#define DFL_EMU             emuts
#define DFL_REPLAYFREQ      44100
#define DFL_USE16BIT        true
#define DFL_STEREO          false
#define DFL_USEOUTPUT       DFL_EMU
#define DFL_TESTOPL2        true
#define DFL_DISKDIR         "C:\\"
#define DFL_AUTOEND         true
#define DFL_FASTSEEK        false
#define DFL_PRIORITY        4
#define DFL_STDTIMER        true
#define DFL_SUBSONG         0
//
#define WM_WA_MPEG_EOF      WM_USER+2
#define WM_AP_UPDATE        WM_USER+100
#define WM_AP_UPDATE_ALL	WM_USER+101

        /* supported filetypes */

struct
{
        char            extension[12];
        char            description[80];
        bool            ignore;
} filetypes[] = {
        "a2m\0",        "Adlib Tracker 2 Modules (*.A2M)\0",                false,
        "amd\0",        "AMUSIC Adlib Tracker Modules (*.AMD)\0",           false,
        "bam\0",        "Bob's Adlib Music Format (*.BAM)\0",               false,
        "cmf\0",        "Creative Music Files (*.CMF)\0",                   false,
        "d00\0",        "EdLib Modules (*.D00)\0",                          false,
        "dfm\0",        "Digital-FM Modules (*.DFM)\0",                     false,
        "hsc\0",        "HSC-Tracker Modules (*.HSC)\0",                    false,
        "hsp\0",        "Packed HSC-Tracker Modules (*.HSP)\0",             false,
        "imf;wlf\0",    "Apogee IMF Files (*.IMF;*.WLF)\0",                 false,
        "ksm\0",        "Ken Silverman's Music Format (*.KSM)\0",           false,
        "laa\0",        "LucasArts Adlib Audio Files (*.LAA)\0",            false,
        "lds\0",        "LOUDNESS Modules (*.LDS)\0",                       false,
        "m\0",          "Ultima 6 Music Format (*.M)\0",                    false,
        "mad\0",        "Mlat Adlib Tracker Modules (*.MAD)\0",             false,
        "mid\0",        "MIDI Audio Files (*.MID)\0",                       false,
        "mkj\0",        "MKJamz Audio Files (*.MKJ)\0",                     false,
        "mtk\0",        "MPU-401 Trakker Modules (*.MTK)\0",                false,
        "rad\0",        "Reality Adlib Tracker Modules (*.RAD)\0",          false,
        "raw\0",        "RdosPlay RAW Files (*.RAW)\0",                     false,
        "rol\0",        "Adlib Visual Composer (*.ROL)\0",                  false,
        "s3m\0",        "Screamtracker 3 Adlib Modules (*.S3M)\0",          false,
        "sa2\0",        "Surprise! Adlib Tracker 2 Modules (*.SA2)\0",      false,
        "sat\0",        "Surprise! Adlib Tracker Modules (*.SAT)\0",        false,
        "sci\0",        "Sierra Adlib Audio Files (*.SCI)\0",               false,
        "sng\0",        "SNGPlay Files (*.SNG)\0",                          false,
        "sng\0",        "Faust Music Creator Modules (*.SNG)\0",            false,
		"sng\0",		"Adlib Tracker Modules (*.SNG)\0",					false,
        "xad\0",        "Exotic Adlib Music Format (*.XAD)\0",              false,
        "xms\0",        "XMS-Tracker (*.XMS)\0",                            false
};

#define FTIGNORE     ";mid;"
#define FTSIZE       sizeof(filetypes)
#define FTELEMCOUNT  sizeof(filetypes)/sizeof(filetypes[0])

        /* thread priority */

int thread_priority[] =
{
        THREAD_PRIORITY_IDLE,
        THREAD_PRIORITY_LOWEST,
        THREAD_PRIORITY_BELOW_NORMAL,
        THREAD_PRIORITY_NORMAL,
        THREAD_PRIORITY_ABOVE_NORMAL,
        THREAD_PRIORITY_HIGHEST,
        THREAD_PRIORITY_TIME_CRITICAL
};

        /* variables */

In_Module       mod;
//
struct
{
        int             replayfreq, nextreplayfreq;
        bool            use16bit,   nextuse16bit;
        bool            stereo,     nextstereo;
        enum output     useoutput,  nextuseoutput;
        unsigned short  adlibport,  nextadlibport;
        bool            testopl2,   nexttestopl2;
        bool            testloop,   nexttestloop;
        bool            fastseek,   nextfastseek;
        int             priority,   nextpriority;
        int             stdtimer,   nextstdtimer;
        char            *diskdir,   *nextdiskdir;
        int                         nextuseoutputplug;
} cfg;
//
char            cfgfile[_MAX_PATH];
//
struct
{
        CEmuopl         *emu;
        CDiskopl        *disk;
        CRealopl        *real;
        CSilentopl      *silent;
} out;
//
struct
{
        char            file[_MAX_PATH];
        int             playing;
        int             paused;
        unsigned int    subsong;
        unsigned int    maxsubsong;
        float           outtime;
        unsigned long   fulltime;
        int             seek;
        int             volume;
        union
        {
                HANDLE          emuts;
                UINT            opl2;
                HANDLE          disk;
        } thread;
} plr;
//
CPlayer         *player = NULL;
//
MIDIOUTCAPS     mc;
TIMECAPS        tc;
HMIDIOUT        midiout;
//
char            FileInfoFile[_MAX_PATH];
HWND            FileInfoWnd = NULL;
CPlayer         *FileInfoPlayer;
//
HWND            AboutTabWnd = NULL;
int             AboutTabIndex;
//
char            *diskfile = NULL;

        /* private functions */

char *lowstring(char *s)
{
  char *lps = s;

  while (*lps)
    *lps++ = tolower(*lps);

  return s;
}

bool test_opl2()
{
  CRealopl temp_adlib(cfg.nextadlibport);

  return temp_adlib.detect();
}

bool test_os()
{
  OSVERSIONINFO ver;

  ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

  GetVersionEx(&ver);

  if(ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
    return true;

  return false;
}

bool test_xmplay()
{
  return GetModuleHandle("xmplay.exe") ? true : false;
}

void config_test()
{
  if ((cfg.nextuseoutput == emuts) || (cfg.nextuseoutput == emuks))
    cfg.nextuseoutputplug = 1;
  else
    cfg.nextuseoutputplug = 0;

  // XMPlay ?
  if (!cfg.nextuseoutputplug && test_xmplay())
  {
    cfg.nextuseoutput = DFL_EMU;
    cfg.nextuseoutputplug = 1;

    MessageBox(mod.hMainWindow, "Using own output is impossible under XMPLAY!\n"
                     "\n"
                     "Switching to emulation mode.","AdPlug :: Error",MB_OK | MB_ICONERROR);
  }

  // WinNT ?
  if ((cfg.nextuseoutput == opl2) && test_os())
  {
    cfg.nextuseoutput = DFL_EMU;
    cfg.nextuseoutputplug = 1;

    MessageBox(mod.hMainWindow, "OPL2 hardware replay is not possible on Windows NT/XP!\n"
                     "\n"
                     "Switching to emulation mode.","AdPlug :: Error",MB_OK | MB_ICONERROR);
  }

  // No OPL ?
  if ((cfg.nextuseoutput == opl2) && cfg.nexttestopl2 && !test_opl2())
  {
    cfg.nextuseoutput = DFL_EMU;
    cfg.nextuseoutputplug = 1;

    MessageBox(mod.hMainWindow, "OPL2 chip not detected!\n"
                     "\n"
                     "Switching to emulation mode.","AdPlug :: Error",MB_OK | MB_ICONERROR);
  }

  // Non-standard timer and No loop detection ?
  if ((cfg.nextuseoutput == disk) && !cfg.nextstdtimer && !cfg.nexttestloop)
    MessageBox(mod.hMainWindow, "HYPERSPEED ENDLESS Disk Writing mode selected!","AdPlug :: Warning", MB_OK | MB_ICONWARNING);

  // Own -> Standard output switching ?
  if (cfg.nextuseoutputplug > mod.UsesOutputPlug)
    MessageBox(mod.hMainWindow,"Switching from own to standart output detected.\n"
                     "\n"
                     "Winamp must be restarted.","AdPlug :: Warning",MB_OK | MB_ICONWARNING);
}

void config_apply(bool to)
{
  cfg.replayfreq = cfg.nextreplayfreq;
  cfg.use16bit   = cfg.nextuse16bit;
  cfg.stereo     = cfg.nextstereo;
  cfg.adlibport  = cfg.nextadlibport;
  cfg.testopl2   = cfg.nexttestopl2;
  cfg.testloop   = cfg.nexttestloop;
  cfg.fastseek   = cfg.nextfastseek;
  cfg.priority   = cfg.nextpriority;
  cfg.stdtimer   = cfg.nextstdtimer;
  cfg.diskdir    = cfg.nextdiskdir;

  if (!to || (cfg.nextuseoutputplug <= mod.UsesOutputPlug))
  {
    cfg.useoutput      = cfg.nextuseoutput;
    mod.UsesOutputPlug = cfg.nextuseoutputplug;
  }
}

bool test_filetype(char *fn)
{
  char *tmpstr = strrchr(fn,'.');
  if (!tmpstr)
    return false;

  char *p = (char *)malloc(strlen(++tmpstr)+1);
  if (!p)
    return false;

  strcpy(p,tmpstr);

  for (int i=0;i<FTELEMCOUNT;i++)
  {
    char *ext = filetypes[i].extension;
    char *str = strstr(ext,lowstring(p));

    if (str)
    {
      // for "aaa;bbb;ccc" and "ccc"
	  if (strlen(p) == strlen(str))
	  {
		free(p);
		return true;
	  }

      if (str[strlen(p)] == ';')
      {
        // for "aaa;bbb;ccc" and "aaa"
        if (ext == str)
		{
		  free(p);
		  return true;
		}

        // for "aaa;bbb;ccc" and "bbb"
		if (ext[str-ext-1] == ';')
		{
		  free(p);
		  return true;
		}
      }
    }
  }

  free(p);

  return false;
}

const char *build_raw_name(char *fn)
{
  char bufstr[11];

  // free diskfile
  if (diskfile)
    free(diskfile);

  diskfile = (char *)malloc(strlen(cfg.diskdir) + strlen(strrchr(fn,'\\')) + 16);

  // diskfile = "(&diskdir)\(&fn).(&subsong).raw"
  strcpy(diskfile,cfg.diskdir);
  strcat(diskfile,strrchr(fn,'\\'));
  if (plr.subsong > 0)
  {
    strcat(diskfile,".");
    _itoa(plr.subsong,bufstr,10);
    strcat(diskfile,bufstr);
  }
  strcat(diskfile,".raw");

#ifdef _DEBUG
  printf("Disk Writer output file = %s\n",diskfile);
#endif

  return diskfile;
}

int get_song_length(char *fn, int subsong)
{
  unsigned long sl = 0;

  CPlayer *p = CAdPlug::factory(fn,out.silent);
  if (p)
  {
    sl = CAdPlug::songlength(p,subsong);
    delete p;
  }

  return sl;
}

Copl *opl_init()
{
  Copl *opl;

  if (cfg.useoutput == emuts)
    opl = out.emu = new CEmuopl(cfg.replayfreq,cfg.use16bit,cfg.stereo);
  if (cfg.useoutput == opl2)
    opl = out.real = new CRealopl(cfg.adlibport);
  if (cfg.useoutput == disk)
    opl = out.disk = new CDiskopl(build_raw_name(plr.file));

  return opl;
}

void opl_done()
{
  if (cfg.useoutput == emuts)
    delete out.emu;
  if (cfg.useoutput == opl2)
    delete out.real;
  if (cfg.useoutput == disk)
    delete out.disk;
}

/* -------- window procedures ----------------------------- */

BOOL APIENTRY AboutTabDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  BYTE *logo = (BYTE *)LockResource(LoadResource(mod.hDllInstance,FindResource(mod.hDllInstance,MAKEINTRESOURCE(IDB_LOGO),RT_BITMAP)));

  int x,y;
  BYTE bmobc;
  BYTE *bmptr;

  char url1[50],url2[50];

#ifdef _DEBUG
  printf("AboutTabDlgProc(): Message 0x%08X received.\n",message);
#endif

  switch (message)
  {
    case WM_SETFONT:

      // apply transparency to logo
      if (AboutTabIndex == 0)
	  {
        *(DWORD *)&logo[0x424] = GetSysColor(COLOR_BTNFACE);

        bmptr = &logo[0x428];
        bmobc = *bmptr;

        for(x=0;x<(*(WORD *)&logo[4]);x++)
          for(y=0;y<(*(WORD *)&logo[8]);y++)
		  {
            if (*bmptr == bmobc)
              *bmptr = 0xff;

            bmptr++;
          }
      }

      return TRUE;


    case WM_INITDIALOG:

      // move tab content on top
      SetWindowPos(hwndDlg,HWND_TOP,13,33,0,0,SWP_NOSIZE);

      return TRUE;


    case WM_NOTIFY:
      switch (((NMHDR *)lParam)->code)
      {
        case EN_LINK:
          if (((ENLINK *)lParam)->msg == WM_LBUTTONDOWN)
		  {
            SendDlgItemMessage(AboutTabWnd,((NMHDR *)lParam)->idFrom,WM_KILLFOCUS,0,0);

            if (((NMHDR *)lParam)->idFrom == IDC_URL_AUTHORS)
              strcpy(url1,"mailto://");
            else
              strcpy(url1,"http://");

            SendDlgItemMessage(AboutTabWnd,((NMHDR *)lParam)->idFrom,EM_SETSEL,((ENLINK *)lParam)->chrg.cpMin,((ENLINK *)lParam)->chrg.cpMax);
            SendDlgItemMessage(AboutTabWnd,((NMHDR *)lParam)->idFrom,EM_GETSELTEXT,0,(LPARAM)url2);

            ShellExecute(hwndDlg,"open",strcat(url1,url2),NULL,NULL,SW_SHOWNORMAL);
		  }

          return TRUE;
      }
  }

  return FALSE;
}

BOOL APIENTRY AboutDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  const char *license = (char *)LockResource(LoadResource(mod.hDllInstance,FindResource(mod.hDllInstance,MAKEINTRESOURCE(IDR_TEXT_LICENSE),"TEXT")));
  const char *history = (char *)LockResource(LoadResource(mod.hDllInstance,FindResource(mod.hDllInstance,MAKEINTRESOURCE(IDR_TEXT_HISTORY),"TEXT")));

  TCITEM tci;
  CHARFORMAT2 cf2;

  cf2.cbSize    = sizeof(CHARFORMAT);
  cf2.dwMask    = CFM_LINK;
  cf2.dwEffects = CFE_LINK;

  switch (message)
  {
    case WM_INITDIALOG:

      // init tab control
      tci.mask = TCIF_TEXT;

      tci.pszText = "General";
      SendDlgItemMessage(hwndDlg,IDC_ATABS,TCM_INSERTITEM,0,(LPARAM)&tci);
		
      tci.pszText = "License";
      SendDlgItemMessage(hwndDlg,IDC_ATABS,TCM_INSERTITEM,1,(LPARAM)&tci);
		
      tci.pszText = "What's New";
      SendDlgItemMessage(hwndDlg,IDC_ATABS,TCM_INSERTITEM,2,(LPARAM)&tci);

      // set default tab index
      SendDlgItemMessage(hwndDlg,IDC_ATABS,TCM_SETCURSEL,0,0);


    case WM_AP_UPDATE:

      // delete old tab window
      if (AboutTabWnd)
      {
        DestroyWindow(AboutTabWnd);

        AboutTabWnd = NULL;
      }

      // display new tab window
      AboutTabIndex = (int)SendDlgItemMessage(hwndDlg,IDC_ATABS,TCM_GETCURSEL,0,0);

      switch (AboutTabIndex)
      {
        case 0:

          AboutTabWnd = CreateDialog(mod.hDllInstance,MAKEINTRESOURCE(IDD_ABT_ADPLUG),hwndDlg,(DLGPROC)AboutTabDlgProc);

        /* authors */

          SendDlgItemMessage(AboutTabWnd,IDC_URL_AUTHORS,EM_SETBKGNDCOLOR,0,GetSysColor(COLOR_BTNFACE));
          SendDlgItemMessage(AboutTabWnd,IDC_URL_AUTHORS,EM_SETEVENTMASK,0,(LPARAM)ENM_LINK);
          SendDlgItemMessage(AboutTabWnd,IDC_URL_AUTHORS,WM_KILLFOCUS,0,0);

          SetDlgItemText(AboutTabWnd,IDC_URL_AUTHORS,

                          PLUGINVER " (" __DATE__ ")\n\nCopyright (c) 1"
                          "999 - 2002 Simon Peter <dn.tlp@gmx.net>" "\n"
                          "Copyright (c) 2002 Nikita V. Kalaganov <rive"
                          "n@ok.ru>"

                         );

          SendDlgItemMessage(AboutTabWnd,IDC_URL_AUTHORS,EM_SETSEL,83,97);
		  SendDlgItemMessage(AboutTabWnd,IDC_URL_AUTHORS,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf2);

          SendDlgItemMessage(AboutTabWnd,IDC_URL_AUTHORS,EM_SETSEL,139,150);
		  SendDlgItemMessage(AboutTabWnd,IDC_URL_AUTHORS,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf2);

        /* links */

          SendDlgItemMessage(AboutTabWnd,IDC_URL_LINKS,EM_SETEVENTMASK,0,(LPARAM)ENM_LINK);
          SendDlgItemMessage(AboutTabWnd,IDC_URL_LINKS,EM_SETBKGNDCOLOR,0,GetSysColor(COLOR_BTNFACE));

          SetDlgItemText(AboutTabWnd,IDC_URL_LINKS,

                          "[ get latest version at adplug.sourceforge.net ]\n\n"
                          "[ get modules at www.chiptune.de ]\t[ get trackers"
                          " at www.astercity.net/~malf ]"

                         );

          SendDlgItemMessage(AboutTabWnd,IDC_URL_LINKS,EM_SETSEL,24,46);
		  SendDlgItemMessage(AboutTabWnd,IDC_URL_LINKS,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf2);

          SendDlgItemMessage(AboutTabWnd,IDC_URL_LINKS,EM_SETSEL,67,82);
		  SendDlgItemMessage(AboutTabWnd,IDC_URL_LINKS,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf2);

          SendDlgItemMessage(AboutTabWnd,IDC_URL_LINKS,EM_SETSEL,103,126);
		  SendDlgItemMessage(AboutTabWnd,IDC_URL_LINKS,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf2);

          break;

        case 1:

          AboutTabWnd = CreateDialog(mod.hDllInstance,MAKEINTRESOURCE(IDD_ABT_LICENSE),hwndDlg,(DLGPROC)AboutTabDlgProc);

        /* license */

          SendDlgItemMessage(AboutTabWnd,IDC_LICENSE,WM_KILLFOCUS,0,0);

          SetDlgItemText(AboutTabWnd,IDC_LICENSE,license);

        /* link */

          SendDlgItemMessage(AboutTabWnd,IDC_URL_GNU,EM_SETEVENTMASK,0,(LPARAM)ENM_LINK);
          SendDlgItemMessage(AboutTabWnd,IDC_URL_GNU,EM_SETBKGNDCOLOR,0,GetSysColor(COLOR_BTNFACE));

          SetDlgItemText(AboutTabWnd,IDC_URL_GNU,

                          "Read GNU LGPL: www.gnu.org/licenses/lgpl.txt"

                         );

          SendDlgItemMessage(AboutTabWnd,IDC_URL_GNU,EM_SETSEL,15,45);
		  SendDlgItemMessage(AboutTabWnd,IDC_URL_GNU,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf2);

          break;

        case 2:

          AboutTabWnd = CreateDialog(mod.hDllInstance,MAKEINTRESOURCE(IDD_ABT_HISTORY),hwndDlg,(DLGPROC)AboutTabDlgProc);

        /* history */

          SendDlgItemMessage(AboutTabWnd,IDC_HISTORY,WM_KILLFOCUS,0,0);

          SetDlgItemText(AboutTabWnd,IDC_HISTORY,history);

          break;
      }

	  return TRUE;


    case WM_NOTIFY:
      switch (((NMHDR *)lParam)->code)
      {
        case TCN_SELCHANGE:

          PostMessage(hwndDlg,WM_AP_UPDATE,0,0);
          return TRUE;
      }


    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDCANCEL:
          
          EndDialog(hwndDlg,wParam);
          return TRUE;
      }
  }

  return FALSE;
}

BOOL APIENTRY ConfigOutputDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  char bufstr[5];
  unsigned long buf;

  switch (message)
  {
    case WM_INITDIALOG:
      return TRUE;


    case WM_NOTIFY:
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_SETACTIVE:

          // set "output"
          switch (cfg.nextuseoutput)
          {
            case emuts:
              CheckRadioButton(hwndDlg,IDC_OUTTS,IDC_OUTOPL2,IDC_OUTTS);
              break;
            case emuks:
              CheckRadioButton(hwndDlg,IDC_OUTTS,IDC_OUTOPL2,IDC_OUTKS);
              break;
            case opl2:
              CheckRadioButton(hwndDlg,IDC_OUTTS,IDC_OUTOPL2,IDC_OUTOPL2);
              break;
            case disk:
              CheckRadioButton(hwndDlg,IDC_OUTTS,IDC_OUTOPL2,IDC_OUTDISK);
              break;
          }

          // set "frequency"
          switch (cfg.nextreplayfreq)
          {
            case 11025:
              CheckRadioButton(hwndDlg,IDC_FREQ1,IDC_FREQC,IDC_FREQ1);
              break;
            case 22050:
              CheckRadioButton(hwndDlg,IDC_FREQ1,IDC_FREQC,IDC_FREQ2);
              break;
            case 44100:
              CheckRadioButton(hwndDlg,IDC_FREQ1,IDC_FREQC,IDC_FREQ3);
              break;
            case 48000:
              CheckRadioButton(hwndDlg,IDC_FREQ1,IDC_FREQC,IDC_FREQ4);
              break;
            default:
              CheckRadioButton(hwndDlg,IDC_FREQ1,IDC_FREQC,IDC_FREQC);
              SetDlgItemInt(hwndDlg,IDC_FREQC_VALUE,cfg.nextreplayfreq,FALSE);
          }

          // set "resolution"
          if (cfg.nextuse16bit)
            CheckRadioButton(hwndDlg,IDC_QUALITY8,IDC_QUALITY16,IDC_QUALITY16);
          else
            CheckRadioButton(hwndDlg,IDC_QUALITY8,IDC_QUALITY16,IDC_QUALITY8);

          // set "channels"
          if (cfg.nextstereo)
            CheckRadioButton(hwndDlg,IDC_MONO,IDC_STEREO,IDC_STEREO);
          else
            CheckRadioButton(hwndDlg,IDC_MONO,IDC_STEREO,IDC_MONO);

          // set "port"
          SetDlgItemText(hwndDlg,IDC_ADLIBPORT,_itoa(cfg.nextadlibport,bufstr,16));

          // set "options"
          if (!cfg.nexttestopl2)
            CheckDlgButton(hwndDlg,IDC_NOTEST,BST_CHECKED);

          // set "directory"
          SetDlgItemText(hwndDlg,IDC_DIRECTORY,cfg.nextdiskdir);

          return TRUE;

        case PSN_KILLACTIVE:

          // check "frequency"
          if (IsDlgButtonChecked(hwndDlg,IDC_FREQ1) == BST_CHECKED)
            cfg.nextreplayfreq = 11025;
          if (IsDlgButtonChecked(hwndDlg,IDC_FREQ2) == BST_CHECKED)
            cfg.nextreplayfreq = 22050;
          if (IsDlgButtonChecked(hwndDlg,IDC_FREQ3) == BST_CHECKED)
            cfg.nextreplayfreq = 44100;
          if (IsDlgButtonChecked(hwndDlg,IDC_FREQ4) == BST_CHECKED)
            cfg.nextreplayfreq = 48000;
          if (IsDlgButtonChecked(hwndDlg,IDC_FREQC) == BST_CHECKED)
            cfg.nextreplayfreq = GetDlgItemInt(hwndDlg,IDC_FREQC_VALUE,NULL,FALSE);

          // check "resolution"
          if (IsDlgButtonChecked(hwndDlg,IDC_QUALITY16) == BST_CHECKED)
            cfg.nextuse16bit = true;
          if (IsDlgButtonChecked(hwndDlg,IDC_QUALITY8) == BST_CHECKED)
            cfg.nextuse16bit = false;

          // check " channels"
          if (IsDlgButtonChecked(hwndDlg,IDC_STEREO) == BST_CHECKED)
            cfg.nextstereo = true;
          if (IsDlgButtonChecked(hwndDlg,IDC_MONO) == BST_CHECKED)
            cfg.nextstereo = false;

          // check "output"
          if (IsDlgButtonChecked(hwndDlg,IDC_OUTTS) == BST_CHECKED)
            cfg.nextuseoutput = emuts;
          if (IsDlgButtonChecked(hwndDlg,IDC_OUTKS) == BST_CHECKED)
            cfg.nextuseoutput = emuks;
          if (IsDlgButtonChecked(hwndDlg,IDC_OUTOPL2) == BST_CHECKED)
            cfg.nextuseoutput = opl2;
          if (IsDlgButtonChecked(hwndDlg,IDC_OUTDISK) == BST_CHECKED)
            cfg.nextuseoutput = disk;

          // check "port"
          GetDlgItemText(hwndDlg,IDC_ADLIBPORT,bufstr,5);
          sscanf(bufstr,"%x",&buf);
          cfg.nextadlibport = (unsigned short)buf;

          // check "options"
          cfg.nexttestopl2 = !(IsDlgButtonChecked(hwndDlg,IDC_NOTEST) == BST_CHECKED);

          // check "directory"
          GetDlgItemText(hwndDlg,IDC_DIRECTORY,cfg.nextdiskdir,_MAX_PATH);

          return TRUE;

        case PSN_APPLY:
          return TRUE;
      }


    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDC_DIRECTORY:

          // display folder selection dialog
          char shd[_MAX_PATH];

          BROWSEINFO bi;

          bi.hwndOwner = hwndDlg;
          bi.pidlRoot = NULL;
          bi.pszDisplayName = shd;
          bi.lpszTitle = "Select output path for Disk Writer:";
          bi.ulFlags = BIF_RETURNONLYFSDIRS;
          bi.lpfn = NULL;
          bi.lParam = 0;
          bi.iImage = 0;

          if (SHGetPathFromIDList(SHBrowseForFolder(&bi),shd))
            SetDlgItemText(hwndDlg,IDC_DIRECTORY,shd);

          return TRUE;
      }
  }

  return FALSE;
}

BOOL APIENTRY ConfigPlaybackDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
      return TRUE;


    case WM_NOTIFY:
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_SETACTIVE:

          // set "options"
          if (cfg.nexttestloop)
            CheckDlgButton(hwndDlg,IDC_AUTOEND,BST_CHECKED);
          if (cfg.nextfastseek)
            CheckDlgButton(hwndDlg,IDC_FASTSEEK,BST_CHECKED);
          if (cfg.nextstdtimer)
            CheckDlgButton(hwndDlg,IDC_STDTIMER,BST_CHECKED);

          // set "priority"
          SendDlgItemMessage(hwndDlg,IDC_PRIORITY,TBM_SETRANGE,(WPARAM)FALSE,(LPARAM)MAKELONG(1,7));
          SendDlgItemMessage(hwndDlg,IDC_PRIORITY,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)(LONG)cfg.nextpriority);

          return TRUE;

        case PSN_KILLACTIVE:

          // check "options"
          cfg.nexttestloop = (IsDlgButtonChecked(hwndDlg,IDC_AUTOEND) == BST_CHECKED);
          cfg.nextfastseek = (IsDlgButtonChecked(hwndDlg,IDC_FASTSEEK) == BST_CHECKED);
          cfg.nextstdtimer = (IsDlgButtonChecked(hwndDlg,IDC_STDTIMER) == BST_CHECKED);

          // check "priority"
          cfg.nextpriority = (int)SendDlgItemMessage(hwndDlg,IDC_PRIORITY,TBM_GETPOS,0,0);

          return TRUE;

        case PSN_APPLY:
          return TRUE;
      }
  }

  return FALSE;
}

BOOL APIENTRY ConfigFormatsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  int i;

  switch (message)
  {
    case WM_INITDIALOG:
      return TRUE;


    case WM_NOTIFY:
      switch (((NMHDR *) lParam)->code)
      {
        case PSN_SETACTIVE:

          // set dialog controls
          for (i=0;i<FTELEMCOUNT;i++)
            SendDlgItemMessage(hwndDlg,IDC_FORMATLIST,LB_SETSEL,(BOOL)!filetypes[i].ignore,
              SendDlgItemMessage(hwndDlg,IDC_FORMATLIST,LB_ADDSTRING,0,(LPARAM)filetypes[i].description));

          return TRUE;

        case PSN_KILLACTIVE:

          // get dialog controls
          for (i=0;i<FTELEMCOUNT;i++)
            filetypes[i].ignore = (SendDlgItemMessage(hwndDlg,IDC_FORMATLIST,LB_GETSEL,i,0)) ? false : true;

          // clear listbox
          SendDlgItemMessage(hwndDlg,IDC_FORMATLIST,LB_RESETCONTENT,0,0);
          return TRUE;

        case PSN_APPLY:
          return TRUE;
      }


    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDC_FTSELALL:

          // select all
          SendDlgItemMessage(hwndDlg,IDC_FORMATLIST,LB_SETSEL,TRUE,-1);
          return TRUE;

        case IDC_FTDESELALL:

          // deselect all
          SendDlgItemMessage(hwndDlg,IDC_FORMATLIST,LB_SETSEL,FALSE,-1);
          return TRUE;
      }
  }

  return FALSE;
}

BOOL APIENTRY FileInfoDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  int i,spos = 0;
  char bufstr[10];
  std::string str;

  switch (message)
  {
	case WM_INITDIALOG:
    case WM_AP_UPDATE_ALL:

      // set "title"/"author"/"type"
      SetDlgItemText(hwndDlg,IDC_TITLE,FileInfoPlayer->gettitle().insert(0," ").c_str());
      SetDlgItemText(hwndDlg,IDC_AUTHOR,FileInfoPlayer->getauthor().insert(0," ").c_str());
      SetDlgItemText(hwndDlg,IDC_FORMAT,FileInfoPlayer->gettype().insert(0," ").c_str());

      // set "instruments"
      str.erase();
	  for (i=0;i<FileInfoPlayer->getinstruments();i++)
	  {
		if (i < 9)
	      sprintf(bufstr,"0%u - ",i+1);
		else
		  sprintf(bufstr,"%u - ",i+1);

        str += bufstr + FileInfoPlayer->getinstrument(i);

		if (i < FileInfoPlayer->getinstruments() - 1)
          str += "\r\n";
	  }
	  SetDlgItemText(hwndDlg,IDC_INSTLIST,str.c_str());

  	  // set "description" (ANSI "\n" to Windows "\r\n")
      str = FileInfoPlayer->getdesc();
      while ((spos = str.find('\n',spos)) != str.npos)
	  {
	    str.insert(spos,"\r");
		spos += 2;
	  }
	  SetDlgItemText(hwndDlg,IDC_DESCRIPTION,str.c_str());

	  // set "subsong" slider
      if (FileInfoPlayer == player)
      {
 	    SendDlgItemMessage(hwndDlg,IDC_SUBSONGSLIDER,TBM_SETRANGE,(WPARAM)FALSE,(LPARAM)MAKELONG(1,plr.maxsubsong));
	    SendDlgItemMessage(hwndDlg,IDC_SUBSONGSLIDER,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)(plr.subsong + 1));
      }
      else
	  {
 	    SendDlgItemMessage(hwndDlg,IDC_SUBSONGSLIDER,TBM_SETRANGE,(WPARAM)FALSE,(LPARAM)MAKELONG(1,FileInfoPlayer->getsubsongs()));
	    SendDlgItemMessage(hwndDlg,IDC_SUBSONGSLIDER,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)(DFL_SUBSONG + 1));
      }


    case WM_AP_UPDATE:

	  // update "subsong" info and slider
      SetDlgItemInt(hwndDlg,IDC_SUBSONGMIN,1,FALSE);
      if (FileInfoPlayer == player)
      {
        SetDlgItemInt(hwndDlg,IDC_SUBSONG,plr.subsong + 1,FALSE);
	    SetDlgItemInt(hwndDlg,IDC_SUBSONGMAX,plr.maxsubsong,FALSE);
      }
      else
	  {
	    SetDlgItemText(hwndDlg,IDC_SUBSONG,"--");
	    SetDlgItemInt(hwndDlg,IDC_SUBSONGMAX,FileInfoPlayer->getsubsongs(),FALSE);
      }

      // update "info"
	  SetDlgItemInt(hwndDlg,IDC_ORDER,FileInfoPlayer->getorder(),FALSE);
	  SetDlgItemInt(hwndDlg,IDC_ORDERS,FileInfoPlayer->getorders(),FALSE);
	  SetDlgItemInt(hwndDlg,IDC_PATTERN,FileInfoPlayer->getpattern(),FALSE);
	  SetDlgItemInt(hwndDlg,IDC_PATTERNS,FileInfoPlayer->getpatterns(),FALSE);
	  SetDlgItemInt(hwndDlg,IDC_ROW,FileInfoPlayer->getrow(),FALSE);
	  SetDlgItemInt(hwndDlg,IDC_SPEED,FileInfoPlayer->getspeed(),FALSE);
	  sprintf(bufstr,"%.2f hz",FileInfoPlayer->getrefresh());
	  SetDlgItemText(hwndDlg,IDC_TIMER,bufstr);

	  return TRUE;


    case WM_COMMAND:
      switch(LOWORD(wParam))
	  {
		case IDCANCEL:

          // close window
          if (FileInfoPlayer != player)
            delete FileInfoPlayer;

          DestroyWindow(hwndDlg);

          FileInfoWnd = NULL;

          return TRUE;
	  }


	case WM_HSCROLL:
      switch(GetDlgCtrlID((HWND)lParam))
	  {
		case IDC_SUBSONGSLIDER:

          // subsong changing
          if (FileInfoPlayer == player)
		  {
            int newsubsong = SendDlgItemMessage(hwndDlg,IDC_SUBSONGSLIDER,TBM_GETPOS,0,0);

            if ((newsubsong - 1) != plr.subsong)
			{
              plr.subsong = newsubsong - 1;

              SendMessage(mod.hMainWindow,WM_COMMAND,WINAMP_BUTTON2,0);

              SetDlgItemInt(hwndDlg,IDC_SUBSONG,newsubsong,FALSE);
			}
		  }

          return TRUE;
	  }


  } // switch (message)

  return FALSE;
}

/* -------- playing threads ------------------------------- */

DWORD WINAPI thread_emuts(void *status)
{
  long toadd = 0;
  bool stopped = false;

  // allocate sound buffer (double size for dsp)
  int sampsize = 1;
  if (cfg.use16bit)
    sampsize *= 2;
  if (cfg.stereo)
    sampsize *= 2;
  char *sndbuf = (char *)malloc(SNDBUFSIZE*sampsize*2);

/* ! */

  while (*(int *)status)
  {
    // seek requested ?
    if (plr.seek != -1)
    {
      // backward seek ?
      if (plr.seek < plr.outtime)
      {
        player->rewind(plr.subsong);
        plr.outtime = 0.0f;
      }

      // seek to needed position
      while ((plr.outtime < plr.seek) && player->update())
        plr.outtime += 1000/player->getrefresh();

      mod.outMod->Flush((int)plr.outtime);

      plr.seek = -1;
    }

    // update replayer
    if (stopped && cfg.testloop)
    {
      if (!mod.outMod->IsPlaying())
      {
        PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
        break;
      }
   
      Sleep(10);
      continue;
    }

    // fill sound buffer
    long towrite = SNDBUFSIZE;
    char *sndbufpos = sndbuf;
    while (towrite > 0)
    {
      while (toadd < 0)
      {
        toadd += cfg.replayfreq;
        stopped = !player->update();
        plr.outtime += 1000/player->getrefresh();
      }
      long i = min(towrite,(long)(toadd/player->getrefresh()+4)&~3);
      out.emu->update((short *)sndbufpos,i);
      sndbufpos += i * sampsize;
      towrite -= i;
      toadd -= i * player->getrefresh();
    }

    // update dsp
    towrite = mod.dsp_dosamples((short *)sndbuf,SNDBUFSIZE,(cfg.use16bit ? 16 : 8),(cfg.stereo ? 2 : 1),cfg.replayfreq);
    towrite *= sampsize;

    // wait for output plugin
    while (mod.outMod->CanWrite() < towrite)
      Sleep(10);

    // write sound buffer
    mod.outMod->Write(sndbuf,towrite);

    // vis
    mod.SAAddPCMData(sndbuf,(cfg.stereo ? 2 : 1),(cfg.use16bit ? 16 : 8),mod.outMod->GetWrittenTime());
    mod.VSAAddPCMData(sndbuf,(cfg.stereo ? 2 : 1),(cfg.use16bit ? 16 : 8),mod.outMod->GetWrittenTime());

    // update FileInfo, if needed
    if (FileInfoWnd && (FileInfoPlayer == player))
      PostMessage(FileInfoWnd,WM_AP_UPDATE,0,0);
  }

  free(sndbuf);

  return 0;
}

void CALLBACK thread_opl2(UINT wTimerID,UINT msg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
  static float refresh = (!plr.outtime) ? 0 : refresh; // :)

  // paused ?
  if (plr.paused)
    return;

  // seek requested ?
  if (plr.seek != -1)
  {
    // backward seek ?
    if (plr.seek < plr.outtime)
    {
      player->rewind(plr.subsong);
      plr.outtime = 0.0f;
    }

    // seek to needed position
    out.real->setquiet();
    if (cfg.fastseek)
      out.real->setnowrite();
    while ((plr.outtime < plr.seek) && player->update())
      plr.outtime += 1000/player->getrefresh();
    if (cfg.fastseek)
      out.real->setnowrite(false);
    out.real->setquiet(false);

    plr.seek = -1;
  }

  // update replayer
  if (!player->update() && cfg.testloop)
  {
    PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
    return;
  }

  // refresh rate changed ?
  if (refresh != player->getrefresh())
  {
    refresh = player->getrefresh();

    timeKillEvent(plr.thread.opl2);
    plr.thread.opl2 = timeSetEvent(UINT(1000/player->getrefresh()),0,thread_opl2,0,TIME_PERIODIC);
  }

  // update FileInfo, if needed
  if (FileInfoWnd && (FileInfoPlayer == player))
    PostMessage(FileInfoWnd,WM_AP_UPDATE,0,0);

  plr.outtime += 1000/player->getrefresh();
}

DWORD WINAPI thread_disk(void *status)
{
  while (*(int *)status)
    if (!plr.paused)
    {
      // seek requested ?
      if (plr.seek != -1)
      {
        // backward seek ?
        if (plr.seek < plr.outtime)
        {
          player->rewind(plr.subsong);
          plr.outtime = 0.0f;
        }

        // seek to needed position
        out.disk->setnowrite();
        while ((plr.outtime < plr.seek) && player->update())
          plr.outtime += 1000/player->getrefresh();
        out.disk->setnowrite(false);

        plr.seek = -1;
      }

      // update disk writer
      out.disk->update(player);

      // update replayer
      if (!player->update() && cfg.testloop)
      {
        PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
        break;
      }

      plr.outtime += 1000/player->getrefresh();

      // delay, if normal timing
      if (cfg.stdtimer)
      {
        // update FileInfo, if needed
        if (FileInfoWnd && (FileInfoPlayer == player))
          PostMessage(FileInfoWnd,WM_AP_UPDATE,0,0);

        Sleep((DWORD)(1000/player->getrefresh()));
      }
  }

  return 0;
}

/* -------- winamp2 plugin functions ---------------------- */

void wa2_Config(HWND hwndParent)
{
  PROPSHEETPAGE   psp[3];
  PROPSHEETHEADER psh;

  // "Output" page
  psp[0].dwSize = sizeof(PROPSHEETPAGE);
  psp[0].dwFlags = PSP_DEFAULT;
  psp[0].hInstance = mod.hDllInstance;
  psp[0].pszTemplate = MAKEINTRESOURCE(IDD_CFG_OUTPUT);
  psp[0].pfnDlgProc = (DLGPROC)ConfigOutputDlgProc;

  // "Playback" page
  psp[1].dwSize = sizeof(PROPSHEETPAGE);
  psp[1].dwFlags = PSP_DEFAULT;
  psp[1].hInstance = mod.hDllInstance;
  psp[1].pszTemplate = MAKEINTRESOURCE(IDD_CFG_PLAYBACK);
  psp[1].pfnDlgProc = (DLGPROC)ConfigPlaybackDlgProc;

  // "Formats" page
  psp[2].dwSize = sizeof(PROPSHEETPAGE);
  psp[2].dwFlags = PSP_DEFAULT;
  psp[2].hInstance = mod.hDllInstance;
  psp[2].pszTemplate = MAKEINTRESOURCE(IDD_CFG_FORMATS);
  psp[2].pfnDlgProc = (DLGPROC)ConfigFormatsDlgProc;

  // header
  psh.dwSize = sizeof(PROPSHEETHEADER);
  psh.dwFlags = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
  psh.hwndParent = hwndParent;
  psh.pszCaption = (LPSTR)"AdPlug :: Configuration";
  psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
  psh.nStartPage = 0;
  psh.ppsp = psp;

  // display sheet
  PropertySheet(&psh);

  // test config
  config_test();
}

void wa2_About(HWND hwndParent)
{
  DialogBox(mod.hDllInstance,MAKEINTRESOURCE(IDD_ABOUT),hwndParent,(DLGPROC)AboutDlgProc);
}

void wa2_GetFileInfo(char *file, char *title, int *length_in_ms)
{
  // info for current file ?
  if ((!file) || (!*file))
    file = plr.file;

  // set default info
  if (title)
    strcpy(title,strrchr(file,'\\')+1);
  if (length_in_ms)
    *length_in_ms = 0;

  // try to get real info
  CPlayer *p = CAdPlug::factory(file,out.silent);
  if (p)
  {
    if (title)
      if (!p->gettitle().empty())
        strcpy(title,p->gettitle().c_str());
    if (length_in_ms)
      *length_in_ms = CAdPlug::songlength(p,((file) ? plr.subsong : DFL_SUBSONG));
    delete p;
  }
}

int wa2_InfoBox(char *file, HWND hwndParent)
{
  // already displayed ?
  if (FileInfoWnd)
  {
    // same file ?
    if (!strcmp(FileInfoFile,file))
      return 0;

    // destroy player, if needed
	if (FileInfoPlayer != player)
      delete FileInfoPlayer;
  }

  // use new player
  if (plr.playing && !strcmp(file,plr.file))
	FileInfoPlayer = player;
  else
	FileInfoPlayer = CAdPlug::factory(file,out.silent);

  // return, if error
  if (!FileInfoPlayer)
  {
    if (FileInfoWnd)
    {
      DestroyWindow(FileInfoWnd);
      FileInfoWnd = NULL;
	}
	return 0;
  }

  // create or refresh ?
  if (FileInfoWnd)
    PostMessage(FileInfoWnd,WM_AP_UPDATE_ALL,0,0);
  else
    FileInfoWnd = CreateDialogParam(mod.hDllInstance,MAKEINTRESOURCE(IDD_FILEINFO),hwndParent,(DLGPROC)FileInfoDlgProc,NULL);

  // save file name
  strcpy(FileInfoFile,file);

  return 0;
}

int wa2_IsOurFile(char *fn)
{
  // is that filetype ignored ?
  if (test_filetype(fn))
    return 0;

  // try to init player
  CPlayer *p = CAdPlug::factory(fn,out.silent);
  if (p)
  {
    delete p;
    return 1;
  }

  return 0;
}

int wa2_Play(char *fn)
{
  int maxlatency;
  DWORD tmpd;

  // apply config
  config_apply(true);

  // new file ?
  if (strcmp(fn,plr.file))
  {
    plr.subsong = DFL_SUBSONG;

    strcpy(plr.file,fn);
  }

  // init MIDI, if opl2 used
  midiout = NULL;
  if (cfg.useoutput == opl2)
    for (int i=0;i<midiOutGetNumDevs();i++)
      if (midiOutGetDevCaps(i,&mc,sizeof(MIDIOUTCAPS)) == MMSYSERR_NOERROR)
        if (mc.wTechnology == MOD_FMSYNTH)
		  if(cfg.testopl2)
            if (midiOutOpen(&midiout,i,0,0,CALLBACK_NULL) != MMSYSERR_NOERROR)
			{
              MessageBox(mod.hMainWindow,"The OPL2 chip is already in use by the MIDI sequencer!\n"
                             "\n"
                             "Please quit all running MIDI applications before going on.","AdPlug :: Error",MB_OK | MB_ICONERROR);
            return 1;
          }
          else
            break;

  // init opl & player
  player = CAdPlug::factory(plr.file,opl_init());
  if (!player)
  {
    opl_done();
    return 1;
  }

  // init player data
  plr.paused = 0;
  plr.maxsubsong = player->getsubsongs();
  plr.outtime = 0.0f;
  plr.fulltime = get_song_length(plr.file,plr.subsong);
  plr.seek = -1;
  plr.playing = 1;


  maxlatency = 0;

  // init output
  switch (cfg.useoutput)
  {
    case emuts:
//  case emuks:
      maxlatency = mod.outMod->Open(cfg.replayfreq,(cfg.stereo ? 2 : 1),(cfg.use16bit ? 16 : 8),-1,-1);
      if (maxlatency < 0)
      {
        delete player;
        opl_done();
        return 1;
      }
      mod.outMod->SetVolume(-666);
      mod.SAVSAInit(maxlatency,cfg.replayfreq);
      mod.VSASetInfo((cfg.stereo ? 2 : 1),cfg.replayfreq);
      break;
    case opl2:
      out.real->setvolume(plr.volume);
      break;
  }

  // send to winamp some info
  mod.SetInfo(120000,(int)(cfg.replayfreq/1000),(cfg.stereo ? 2 : 1),1);

  // rewind song
  player->rewind(plr.subsong);

  // init playing thread
  switch (cfg.useoutput)
  {
    case emuts:
      plr.thread.emuts = (HANDLE)CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)thread_emuts,(void *)&plr.playing,0,&tmpd);
      SetThreadPriority(plr.thread.emuts,thread_priority[cfg.priority]);
      break;
//  case emuks:
    case opl2:
      timeGetDevCaps(&tc,sizeof(TIMECAPS));
      timeBeginPeriod(tc.wPeriodMin);
      plr.thread.opl2 = timeSetEvent((UINT)(1000/player->getrefresh()),0,thread_opl2,0,TIME_PERIODIC);
      break;
    case disk:
      plr.thread.disk = (HANDLE)CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)thread_disk,(void *)&plr.playing,0,&tmpd);
      SetThreadPriority(plr.thread.disk,thread_priority[cfg.priority]);
      break;
  }

  // update FileInfoPlayer, if needed
  if (FileInfoWnd && !strcmp(FileInfoFile,plr.file))
  {
    delete FileInfoPlayer;
    FileInfoPlayer = player;
  }

  return 0;
}

void wa2_Pause()
{
  plr.paused = 1;
}

void wa2_UnPause()
{
  plr.paused = 0;
}

int wa2_IsPaused()
{
  return plr.paused;
}

void wa2_Stop()
{
  if (!plr.playing)
    return;

  plr.playing = 0;

  // free playing thread
  switch (cfg.useoutput)
  {
    case emuts:
      if (WaitForSingleObject(plr.thread.emuts,(DWORD)(7*1000/player->getrefresh())) == WAIT_TIMEOUT)
        TerminateThread(plr.thread.emuts,0);
      CloseHandle(plr.thread.emuts);
      break;
//  case emuks:
    case opl2:
      timeKillEvent(plr.thread.opl2);
      timeEndPeriod(tc.wPeriodMin);
      break;
    case disk:
      if (WaitForSingleObject(plr.thread.disk,(DWORD)(7*1000/player->getrefresh())) == WAIT_TIMEOUT)
        TerminateThread(plr.thread.disk,0);
      CloseHandle(plr.thread.disk);
      break;
  }

  // update FileInfoPlayer, if needed
  if (FileInfoWnd && (FileInfoPlayer == player))
  {
	FileInfoPlayer = CAdPlug::factory(plr.file,out.silent);
    if (!FileInfoPlayer)
    {
      DestroyWindow(FileInfoWnd);
      FileInfoWnd = NULL;
    }
    else
      PostMessage(FileInfoWnd,WM_AP_UPDATE,0,0);
  }

  // free output
  switch (cfg.useoutput)
  {
    case emuts:
//  case emuks:
      mod.SAVSADeInit();
      mod.outMod->Close();
      break;
    case opl2:
      out.real->setvolume(63);
      break;
  }

  // free player
  delete player;

  // free opl
  opl_done();

  // free MIDI, if was used
  if (midiout)
    midiOutClose(midiout);
}

int wa2_GetLength()
{
  return plr.fulltime;
}

int wa2_GetOutputTime()
{
  int outtime = 0;

  switch (cfg.useoutput)
  {
    case emuts:
//  case emuks:
      outtime = mod.outMod->GetOutputTime();
      break;
    case opl2:
    case disk:
      outtime = plr.outtime;
      break;
  }

  return outtime;
}

void wa2_SetOutputTime(int time_in_ms)
{
  plr.seek = time_in_ms;
}

void wa2_SetVolume(int volume)
{
  plr.volume = (int)(63 - volume/(255/63));

  switch (cfg.useoutput)
  {
    case emuts:
//  case emuks:
      mod.outMod->SetVolume(volume);
      break;
    case opl2:
      if (plr.playing)
        out.real->setvolume(plr.volume);
      break;
  }
}

void wa2_SetPan(int pan)
{
  switch (cfg.useoutput)
  {
    case emuts:
//  case emuks:
      mod.outMod->SetPan(pan);
      break;
  }
}

void wa2_EQSet(int on, char data[10], int preamp)
{
}

void wa2_Init()
{
  InitCommonControls();

  LoadLibrary("riched20.dll");

  // init opls
  out.silent = new CSilentopl;

  // clear player data
  memset(&plr,0,sizeof(plr));

  // test & apply config
  config_test();
  config_apply(false);
}

void wa2_Quit()
{
  // free diskfile
  if (diskfile)
    free(diskfile);

  // free opls
  delete out.silent;

  // save configuration
  char bufstr[11];

  WritePrivateProfileString("in_adlib","Frequency",_itoa(cfg.nextreplayfreq,bufstr,10),cfgfile);
  WritePrivateProfileString("in_adlib","Resolution",_itoa(cfg.nextuse16bit,bufstr,10),cfgfile);
  WritePrivateProfileString("in_adlib","Stereo",_itoa(cfg.nextstereo,bufstr,10),cfgfile);
  WritePrivateProfileString("in_adlib","Output",_itoa(cfg.nextuseoutput,bufstr,10),cfgfile);
  WritePrivateProfileString("in_adlib","Port",_itoa(cfg.nextadlibport,bufstr,16),cfgfile);
  WritePrivateProfileString("in_adlib","Test",_itoa(cfg.nexttestopl2,bufstr,10),cfgfile);
  WritePrivateProfileString("in_adlib","Directory",cfg.nextdiskdir,cfgfile);
  WritePrivateProfileString("in_adlib","AutoEnd",_itoa(cfg.nexttestloop,bufstr,10),cfgfile);
  WritePrivateProfileString("in_adlib","FastSeek",_itoa(cfg.nextfastseek,bufstr,10),cfgfile);
  WritePrivateProfileString("in_adlib","Timer",_itoa(cfg.nextstdtimer,bufstr,10),cfgfile);
  WritePrivateProfileString("in_adlib","Priority",_itoa(cfg.nextpriority,bufstr,10),cfgfile);

  // save list of ignored filetypes
  char *bufptr = (char *)malloc(FTSIZE);

  memset(bufptr,0,FTSIZE);

  strcat(bufptr,";");

  for (int i=0;i<FTELEMCOUNT;i++)
    if (filetypes[i].ignore)
    {
      strcat(bufptr,filetypes[i].extension);
      strcat(bufptr,";");
    }

  WritePrivateProfileString("in_adlib","Ignore",bufptr,cfgfile);

  // free exported list of filetypes
  free(mod.FileExtensions);

  // free diskdir
  free(cfg.nextdiskdir);
}

extern "C" In_Module mod =
{
        IN_VER,
        PLUGINVER,
        0,
        0,
        NULL,
        1,
        1,
        wa2_Config,
        wa2_About,
        wa2_Init,
        wa2_Quit,
        wa2_GetFileInfo,
        wa2_InfoBox,
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
        wa2_EQSet,
        NULL,
        0
};

extern "C" __declspec(dllexport) In_Module *winampGetInModule2()
{
  int i;

#ifdef _DEBUG
  debug_init();
#endif

  // retrieve full path to config file
  GetModuleFileName(GetModuleHandle("in_adlib"),cfgfile,MAX_PATH);

  memcpy(strrchr(cfgfile,'\\')+1,"plugin.ini\0",11);

  // load configuration
  char bufstr[11];
  int  bufvalue;

  bufvalue = GetPrivateProfileInt("in_adlib","Frequency",DFL_REPLAYFREQ,cfgfile);
  if (bufvalue != -1)
    cfg.nextreplayfreq = bufvalue;

  bufvalue = GetPrivateProfileInt("in_adlib","Resolution",DFL_USE16BIT,cfgfile);
  if (bufvalue != -1)
    cfg.nextuse16bit = (bufvalue) ? true : false;

  bufvalue = GetPrivateProfileInt("in_adlib","Stereo",DFL_STEREO,cfgfile);
  if (bufvalue != -1)
    cfg.nextstereo = (bufvalue) ? true : false;

  bufvalue = GetPrivateProfileInt("in_adlib","Output",DFL_USEOUTPUT,cfgfile);
  if (bufvalue != -1)
    cfg.nextuseoutput = (enum output)bufvalue;

  GetPrivateProfileString("in_adlib","Port","0",bufstr,5,cfgfile);
  if (strcmp(bufstr,"0"))
    sscanf(bufstr,"%x",&cfg.nextadlibport);
  else
    cfg.nextadlibport = DFL_ADLIBPORT;

  bufvalue = GetPrivateProfileInt("in_adlib","Test",DFL_TESTOPL2,cfgfile);
  if (bufvalue != -1)
    cfg.nexttestopl2 = (bufvalue) ? true : false;

  cfg.nextdiskdir = (char *)malloc(_MAX_PATH);
  GetPrivateProfileString("in_adlib","Directory",DFL_DISKDIR,cfg.nextdiskdir,_MAX_PATH,cfgfile);
  if (!cfg.nextdiskdir[0] || !SetCurrentDirectory(cfg.nextdiskdir))
    strcpy(cfg.nextdiskdir,DFL_DISKDIR);

  bufvalue = GetPrivateProfileInt("in_adlib","AutoEnd",DFL_AUTOEND,cfgfile);
  if (bufvalue != -1)
	  cfg.nexttestloop = (bufvalue) ? true : false;

  bufvalue = GetPrivateProfileInt("in_adlib","FastSeek",DFL_FASTSEEK,cfgfile);
  if (bufvalue != -1)
    cfg.nextfastseek = (bufvalue) ? true : false;

  bufvalue = GetPrivateProfileInt("in_adlib","Timer",DFL_STDTIMER,cfgfile);
  if (bufvalue != -1)
    cfg.nextstdtimer = (bufvalue) ? true : false;

  bufvalue = GetPrivateProfileInt("in_adlib","Priority",DFL_PRIORITY,cfgfile);
  if (bufvalue != -1)
    cfg.nextpriority = bufvalue;

  // process list of ignored filetypes
  char *bufptr = (char *)malloc(FTSIZE);

  GetPrivateProfileString("in_adlib","Ignore",FTIGNORE,bufptr,FTSIZE,cfgfile);

  bufptr = lowstring(bufptr);

  for (i=0;i<FTELEMCOUNT;i++)
  {
    char chkext[sizeof(filetypes[0].extension)+2];

    strcpy(chkext,";");
    strcat(chkext,filetypes[i].extension);
    strcat(chkext,";");

    if (strstr(bufptr,chkext))
      filetypes[i].ignore = true;
  }

  // build exported list of filetypes
  unsigned int bufcpylen;

  memset(bufptr,0,FTSIZE);

  mod.FileExtensions = bufptr;

  for (i=0;i<FTELEMCOUNT;i++)
    if (!filetypes[i].ignore)
    {
      bufcpylen = strlen(filetypes[i].extension) + 1;
      memcpy(bufptr,filetypes[i].extension,bufcpylen);
      bufptr += bufcpylen;

      bufcpylen = strlen(filetypes[i].description) + 1;
      memcpy(bufptr,filetypes[i].description,bufcpylen);
      bufptr += bufcpylen;
    }

  return &mod;
}
