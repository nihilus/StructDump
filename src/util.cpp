#include "util.h"

const unsigned long parse_ulong(LPCTSTR strNum)
{
  int base;
  TCHAR *dum;

  if ( (_tcsnicmp(strNum+_tcslen(strNum)-1, _T("h"), 1)==0) || (_tcsnicmp(strNum, _T("0x"), 2)==0))
    base = 16;
  else if (strNum[0] == _TXCHAR('0'))
    base = 8;
  else
    base = 10;
  return _tcstoul(strNum, &dum, base);
}

bool str_to_file(const char *fn, std::string &s)
{
  bool bRet = false;
  do
  {
    FILE *fp = fopen(fn, "wb");
    if (fp == NULL)
      break;

    fwrite(s.c_str(), 1, s.size(), fp);
    fclose(fp);
    bRet = true;
  } while (0);
  return bRet;
}

bool copy_data_to_clipboard(HWND hwnd, void *data, UINT size, UINT cformat)
{
  if (size == 0)
    size = strlen((const char *)data) + 1;

  HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, size);
  if (hMem == 0)
    return false;

  bool bCleanUp = true;

  do 
  {
    char *pMem = (char *) ::GlobalLock(hMem);
    if (pMem == 0)
      break;

    memcpy(pMem, data, size);
    ::GlobalUnlock(hMem);

    if (!::OpenClipboard(hwnd))
      break;

    ::EmptyClipboard();

    // Copy data
    if (!::SetClipboardData(cformat, hMem))
      break;

    bCleanUp = false;
  } while(0);

  if (bCleanUp)
    ::GlobalFree(hMem);

  ::CloseClipboard();
  return bCleanUp == false;
}
