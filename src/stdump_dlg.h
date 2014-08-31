#ifndef __STDUM_DLG__08212006__
#define __STDUM_DLG__08212006__

#include "struct_util.h"
#include <Windows.h>

class CStructDialog
{
private:
  static LRESULT CALLBACK DlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
  HINSTANCE _hInstance;
  HICON _hIcon;
  TCHAR _sz_outputfile[MAX_PATH];

  HWND _hwnd_lbData;
  HWND _hwnd_edtOutput;
  HWND _hwndDlg;

  static bool isShiftDown();
  void OnInitDialog();
  void OnBtnBrowseClick();
  void OnBtnMakeTable();
  void OnBtnClearClick();
  void OnBtnAddClick();
  void OnClose();
  void OnBtnOutputClick();
  void OnBtnEditConfigClick();
  void OnBtnAddUserTypesClick();
  void PopulateDataList();
public:
  enum consts_e
  {
    ITEMDATA_NOTHING = 0,
    ITEMDATA_STRUCT_LABEL = 1,
    ITEMDATA_ENUM_LABEL = 2,
    ITEMDATA_STRUCT_ITEM = 10,
    ITEMDATA_ENUM_ITEM = 10000
  };
  data_type_def_list_t _data_lst;
  enum_type_def_list_t _enum_lst;

  int _lang;

  CStructDialog();
  UINT DoModal(HINSTANCE hInst, HWND hwndParent = 0);

  int GetSetLanguage(bool bGet, int lang = 0);
  void ClearDataList();
  bool AddItemToDataList(LPCTSTR szItem, long itemData = ITEMDATA_NOTHING);
  void GetSetOutputText(LPTSTR szText, bool bGet = true);

  bool OnBtnPasteClick();
  bool OnBtnDeleteDefs();
  void OnBtnCopyClick();
};

#endif