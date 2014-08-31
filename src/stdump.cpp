/* 
* StDump.cpp 
*
*	Copyright (C) lallous <lallousx86@yahoo.com>
*
*  StructDump is an IDA plugin, allowing you to export IDA types into
*  high-level language definitions.
*	
*
*  StructDump is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*   
*  StructDump is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*   
*  You should have received a copy of the GNU General Public License
*  along with GNU Make; see the file COPYING.  If not, write to
*  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
*
*/


/*
ToDo
---------
- Fix multiline comments generation in C
- Option to generate C assert code, so to check if generate structs are equivalent (in size terms) regarding the compiler's alignment config.
- General options section; thus allow replacing of ":->" and other characters by user defined character
- allow general options to select default language
- option: overwrite struct

History
---------

* 08/21/2006
- Initial version release
- Dumping to C++ functionality

* 08/22/2006
- Adding support for ASCII types

* 08/23/2006
- Now data_defs map has TID as key instead of std::string

* 08/25/2006
rev1:
- Now data_defs is a list instead of a map to sort elements in order of dependance
- Now no need to emit forwad declaration, since the output should be in a correct order
- Now allow user to dump all structures not from til
- Delphi emit support
rev2:
- fixed a nasty bug, which was introduced by wrongly defining a static member instead of leaving it get inititalized every time
- save dialog's settings upon closing it

* 01/25/2007
- allow option to copy/paste as XML only or as XML/language TEXT

* 01/26/2007
- making way for enumeration dumping

* 01/31/2007
- emit enums in C/delphi works

* 02/27/2007
- visited code and completed it

* 04/05/2007
- visited code and preparing it for release

* 01/30/2008
- Added "make table" functionality to dump data into C or Delphi arrays

*/

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <tchar.h>
#include <ida.hpp>
#include <idp.hpp>
#include <expr.hpp>
#include <bytes.hpp>
#include <loader.hpp>
#include <struct.hpp>
#include <enum.hpp>
#include <map>
#include <string>
#include <list>
#include <sstream>
#include <stdlib.h>
#include <algorithm>


#include "struct_util.h"
#include "struct_serialize.h"
#include "stdump.h"
#include "util.h"

#include "stdump_dlg.h" // The dialog code

CStructDialog g_dlg;

#include "stdump_emit.h" // Emit code

extern plugin_t PLUGIN;
HINSTANCE g_hInstance = 0;

UINT g_CF_IDA_XML;

HWND get_ida_hwnd()
{
  return (HWND)callui(ui_get_hwnd).vptr;
}

char *get_cfg_filename()
{
  static char inifile[MAX_PATH];
  char drive[10];
  char dir[MAX_PATH];
  char fn[MAX_PATH];
  union { char ext[MAX_PATH]; char str[MAX_PATH]; };

  ::GetModuleFileName((HMODULE) g_hInstance, inifile, sizeof(inifile));

  _splitpath(inifile, drive, dir, fn, ext);

  qstrncpy(inifile, drive, sizeof(drive));
  qstrncat(inifile, dir, sizeof(inifile));
  qstrncat(inifile, fn, sizeof(inifile));
  qstrncat(inifile, ".ini", sizeof(inifile));

  return inifile;
}

void display_structs(data_type_def_list_t &def_lst)
{
  static int times = 1;
  data_type_def_list_t::iterator it;

  // Sort so that least dependant go first
  def_lst.sort();

  for (it = def_lst.begin();
    it != def_lst.end();
    ++it)
  {
    data_type_def &def = *it;
    msg("-%d> %s d:%d\n", times, def.name.c_str(), def.depth);
  }
  ++times;
}

void idaapi run(int arg)
{
  if (arg < 0)
    return;

  msg("StDump v0.4 " __DATE__ " " __TIME__ " <lallousx86@yahoo.com>\n");

  switch (arg)
  {
  case 0:
    g_dlg.DoModal(g_hInstance, get_ida_hwnd());
    break;
  case 1:
    g_dlg.OnBtnCopyClick();
    break;
  case 2:
    g_dlg.OnBtnPasteClick();
    break;
  }
  
  return;
}

int idaapi init(void)
{
  ::CoInitialize(NULL);

  g_CF_IDA_XML = ::RegisterClipboardFormat("CF_IDAPLUS_XML_STRUCT");

  return PLUGIN_KEEP;
}

void idaapi term(void)
{
  ::CoUninitialize();
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  switch(ul_reason_for_call) 
  {
  case DLL_PROCESS_ATTACH:
    DisableThreadLibraryCalls((HMODULE)hModule);
    g_hInstance = (HINSTANCE) hModule;
    break;

  case DLL_THREAD_ATTACH:
    break;

  case DLL_THREAD_DETACH:
    break;

  case DLL_PROCESS_DETACH:

    break;
  }
  return TRUE;
}

plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  0,
  init,
  term,
  run,
  // Comment
  "StructDump plugin"
  ,

  // Help
  "Struct to dump plugin\n"
  "\n"
  "This plugin dumps IDA types into high-level source code format\n"
  "\n"
  "Currently, emitting to C++ is supported, but support for other language\n"
  "is simple.\n"
  ,
  "Struct dump",          // the preferred short name of the plugin
  "Alt-F5"         // the preferred hotkey to run the plugin
};
