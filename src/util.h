#ifndef __01092007__UTILS__
#define __01092007__UTILS__

#include <ObjBase.h>
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <string>
#include <stdio.h>

class CCoInitialize
{
public:
  CCoInitialize()
  {
    ::CoInitialize(NULL);
  }
  ~CCoInitialize()
  {
    ::CoUninitialize();
  }
};

const unsigned long parse_ulong(LPCTSTR strNum);
bool str_to_file(const char *fn, std::string &s);

inline bool has_attr(unsigned long value, unsigned long mask)
{
  return (value & mask) == mask;
}

bool copy_data_to_clipboard(HWND hwnd, void *data, UINT size = 0, UINT cformat = CF_TEXT);

#endif