#include "util.h"
#include "stdump_emit.h"
#include "stdump_dlg.h"
#include "resource.h"
#include "struct_serialize.h"
#include <Shellapi.h>

emit_data_defs_lang g_emit_struct_defs[EMITLANG_MAX] =
{
  emit_data_defs_c,
  emit_data_defs_delphi
};

emit_enum_defs_lang g_emit_enums_defs[EMITLANG_MAX] =
{
  emit_enum_defs_c,
  emit_enum_defs_delphi,
};

void CStructDialog::GetSetOutputText(LPTSTR szText, bool bGet)
{
  if (bGet)
  {
    ::GetWindowText(_hwnd_edtOutput, szText, MAX_PATH);
  }
  else
  {
    ::SetWindowText(_hwnd_edtOutput, szText);
  }
}

void CStructDialog::PopulateDataList()
{
  ClearDataList();

  if (_data_lst.size() > 0)
  {
    AddItemToDataList("-------", ITEMDATA_STRUCT_LABEL);
    AddItemToDataList("Structs", ITEMDATA_STRUCT_LABEL);
    AddItemToDataList("-------", ITEMDATA_STRUCT_LABEL);
  }

  int cnt = 0;
  for (data_type_def_list_t::iterator it = _data_lst.begin();
    it != _data_lst.end();
    ++it)
  {
    data_type_def &def = *it;
    AddItemToDataList(def.name.c_str(), ITEMDATA_STRUCT_ITEM + cnt);
    cnt++;
  }

  cnt = 0;

  if (_enum_lst.size() > 0)
  {
    AddItemToDataList("-------", ITEMDATA_ENUM_LABEL);
    AddItemToDataList("Enums", ITEMDATA_ENUM_LABEL);
    AddItemToDataList("-------", ITEMDATA_ENUM_LABEL);
  }
  for (enum_type_def_list_t::iterator it = _enum_lst.begin();
    it != _enum_lst.end();
    ++it)
  {
    enum_type_def_t &def = *it;
    AddItemToDataList(def.name.c_str(), ITEMDATA_ENUM_ITEM + cnt);
    cnt++;
  }
}

void CStructDialog::OnBtnClearClick()
{
  ClearDataList();
  _data_lst.clear();
  _enum_lst.clear();
}

void CStructDialog::OnBtnAddUserTypesClick()
{
  if (isShiftDown())
  {
  }
  else
  {
    size_t qty = get_struc_qty();
    for (size_t i=0;i<qty;i++)
    {
      tid_t id = get_struc_by_idx(i);
      struc_t *st = get_struc(id);
      if (st == 0 || st->from_til())
        continue;
      enum_structs(st, _data_lst);
    }
  }
  PopulateDataList();
}

void CStructDialog::OnBtnAddClick()
{
  char str[MAXSTR];

  // Add enums
  if (isShiftDown())
  {
    for (;;)
    {
      qsnprintf(str, MAXSTR, "Select enums to add (%d so far)", _enum_lst.size());
      enum_t e = choose_enum("Select enums to add", BADNODE);
      if (e == BADNODE)
        break;

      enum_type_def_t def;
      enum_enum(e, def);

      _enum_lst.push_back(def);
    }
  }
  // Add structs
  else
  {
    struc_t *st;
    for (;;)
    {
      qsnprintf(str, MAXSTR, "Select structures to add (%d so far)", _data_lst.size());
      st = choose_struc(str);
      if (st == 0)
        break;
      int dum = 0;
      enum_structs(st, _data_lst);
    }
  //display_structs(_data_lst);
  }
  PopulateDataList();
}

void CStructDialog::OnBtnMakeTable()
{
  ea_t addr = get_screen_ea();

  if (!askaddr(&addr, "Enter address"))
    return;

  static sval_t size1 = 1, count = 1;

  if (!asklong(&size1, "What's the size of the element?"))
    return;

  if (!asklong(&count, "What's the count of elements?"))
    return;

  void *mem = qalloc(size1 * count);
  if (mem == NULL)
  {
    msg("Not enough memory!");
    return;
  }

  if (get_many_bytes(addr, mem, size1*count))
  {
    _lang = GetSetLanguage(true);

    emit_options_t opt;
    opt.read(get_cfg_filename(), _lang);

    std::string result;
    if (!emit_table(mem, _lang, count, size1, opt, result))
      msg("Failed to create table!");
    else
    {
      copy_data_to_clipboard(get_ida_hwnd(), (void *)result.c_str());
      msg("Table created and copied to clipboard!");
    }
  }

  qfree(mem);
}

void CStructDialog::OnBtnOutputClick()
{
  char fn[MAX_PATH] = {0};

  _lang = GetSetLanguage(true);

  GetSetOutputText(fn, true);

  bool b;

  std::string s_enum, s_struct;

  // read configuration file
  emit_options_t opt;
  opt.read(get_cfg_filename(), _lang);

  g_emit_struct_defs[_lang](_data_lst, s_struct, false, opt);
  g_emit_enums_defs[_lang](_enum_lst, s_enum, false, opt);

  s_struct += "\n" + s_enum;
  
  b = str_to_file(fn, s_struct);
                                         
  if (b)
  {
    if (MessageBox(_hwndDlg, _T("Success!\nDo you want to view it?"), _T("Info"), MB_ICONINFORMATION | MB_YESNO) == IDYES)
    {
      ::ShellExecute(_hwndDlg, _T("open"), _T("wordpad"), fn, 0, SW_SHOW);
    }
  }
}

void CStructDialog::OnBtnEditConfigClick()
{
  ::ShellExecute(_hwndDlg, _T("open"), get_cfg_filename(), 0, 0, SW_SHOW);
}

void CStructDialog::OnBtnBrowseClick()
{
  char *fn;

  _lang = GetSetLanguage(true);

  if (_lang == EMITLANG_CPP)
    fn = askfile_c(1, "c:\\file.h", "Enter file name");
  else if (_lang == EMITLANG_DELPHI)
    fn = askfile_c(1, "c:\\file.pas", "Enter file name");

  if (fn)
    GetSetOutputText(fn, false);
}

bool CStructDialog::AddItemToDataList(LPCTSTR szItem, long itemData)
{
  int pos = ::SendMessage(_hwnd_lbData, LB_ADDSTRING, 0, (WPARAM) szItem);
  if (pos == LB_ERR)
    return false;
  ::SendMessage(_hwnd_lbData, LB_SETITEMDATA, (WPARAM) pos, (LPARAM)itemData);
  return true;
}

void CStructDialog::ClearDataList()
{
  ::SendMessage(_hwnd_lbData, LB_RESETCONTENT, 0, 0);
}

int CStructDialog::GetSetLanguage(bool bGet, int lang)
{
  static UINT langs[] = { IDC_RAD_CPP, IDC_RAD_DELPHI };

  if (bGet)
  {
    for (int i=0;i<sizeof(langs)/sizeof(langs[0]);i++)
    {
      HWND h = GetDlgItem(_hwndDlg, langs[i]);
      if (SendMessage(h, BM_GETCHECK, 0, 0) == BST_CHECKED)
        return i;
    }
    return -1;
  }
  else
  {
    int t = sizeof(langs)/sizeof(langs[0]);
    if (lang < 0 || lang > t)
      lang = 0;

    for (int i=0;i<t;i++)
    {
      HWND h = GetDlgItem(_hwndDlg, langs[i]);
      SendMessage(h, BM_SETCHECK, (i == lang) ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    return lang;
  }
}

CStructDialog::CStructDialog()
{
  _hwndDlg = _hwnd_lbData = _hwnd_edtOutput = 0;
  _hInstance = 0;
  _lang = EMITLANG_CPP;
  qstrncpy(_sz_outputfile, "c:\\file.h", sizeof(_sz_outputfile));
}

void CStructDialog::OnInitDialog()
{
  _hwnd_lbData = ::GetDlgItem(_hwndDlg, IDC_LB_STRUCT);
  _hwnd_edtOutput = ::GetDlgItem(_hwndDlg, IDC_EDT_OUTPUT);

  _hIcon = ::LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_ICON1));

  ::SendMessage(_hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)_hIcon);

  GetSetOutputText(_sz_outputfile, false);

  GetSetLanguage(false, _lang);

  PopulateDataList();
}

void CStructDialog::OnClose()
{
  // Update default language
  _lang = GetSetLanguage(true);

  GetSetOutputText(_sz_outputfile, true);
}

LRESULT CALLBACK CStructDialog::DlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  CStructDialog *_this = (CStructDialog *) ::GetWindowLong(hDlg, GWL_USERDATA);

  switch (Msg)
  {
  case WM_INITDIALOG:
    {
      // Store "this" in dialog handle's window data
      ::SetWindowLong(hDlg, GWL_USERDATA, (LONG) lParam);

      _this = (CStructDialog *) lParam;
      _this->_hwndDlg = hDlg;

      _this->OnInitDialog();
      break;
    }
  case WM_COMMAND:
    {
      UINT buttonId = LOWORD(wParam);

      switch (buttonId)
      {
      case IDCANCEL:
        _this->OnClose();
        ::EndDialog(hDlg, IDOK);
        break;
      case IDC_BTN_BROWSE:
        _this->OnBtnBrowseClick();
        break;
      case IDC_BTN_ADDUSER:
        _this->OnBtnAddUserTypesClick();
        break;
      case IDC_BTN_EDITCFG:
        _this->OnBtnEditConfigClick();
        break;
      case IDC_BTN_ADD:
        _this->OnBtnAddClick();
        break;
      case IDC_BTN_CLEAR:
        _this->OnBtnClearClick();
        break;
      case IDC_BTN_OUTPUT:
        _this->OnBtnOutputClick();
        break;
      case IDC_BTN_COPY:
        _this->OnBtnCopyClick();
        break;
      case IDC_BTN_PASTE:
        _this->OnBtnPasteClick();
        break;
      case IDC_BTN_DELETE_FROM_IDA_DB:
        _this->OnBtnDeleteDefs();
      case IDC_BTN_MAKETABLE:
        _this->OnBtnMakeTable();
        break;
      }
    }
  }
  return FALSE;
}

UINT CStructDialog::DoModal(HINSTANCE hInst, HWND hwndParent)
{
  _hInstance = hInst;

  HRSRC hrsrcDialog = ::FindResource(_hInstance, MAKEINTRESOURCE(IDD_SETUPDLG), RT_DIALOG);

  HGLOBAL hGlobal = LoadResource(_hInstance, hrsrcDialog);

  return DialogBoxIndirectParam(_hInstance, (LPCDLGTEMPLATE) hGlobal, hwndParent, (DLGPROC) DlgProc, (LPARAM)this);
}

bool CStructDialog::isShiftDown()
{
  return GetAsyncKeyState(VK_SHIFT) < 0;
}

void CStructDialog::OnBtnCopyClick()
{
  // read configuration file
  emit_options_t opt;
  opt.read(get_cfg_filename(), _lang);

  if (isShiftDown())
  {
    // force copy as XML
    opt.copy_to_clipboard_as_xml = true;
  }

  emit_to_clipboard(_data_lst, _enum_lst, opt);

  msg("Copied from IDA to clipboard\n");
}

bool CStructDialog::OnBtnPasteClick()
{
  UINT format = isShiftDown() ? CF_TEXT : g_CF_IDA_XML;

  if (!IsClipboardFormatAvailable(format))
  {
    msg("No copied data on the clipboard!\n");
    return false;
  }

  if (!::OpenClipboard(_hwndDlg))
    return false;

  HGLOBAL hMem = ::GetClipboardData(format);
  if (hMem == 0)
    return false;

  std::string str = (char *) ::GlobalLock(hMem);
  ::GlobalUnlock(hMem);
  ::CloseClipboard();

  data_type_def_list_t lst;
  enum_type_def_list_t elst;

  SerializePack_t sp = {&lst, &elst};

  if (!::DeSerialize(sp, str.c_str(), SERIALIZE_SRC_STR))
    return false;

  emit_options_t opt;
  opt.read(get_cfg_filename(), _lang);

  int err = 0;

  if (!emit_struct_def_to_ida(lst, opt))
  {
     msg("Failed while emitting structures!\n");
     err++;
  }

  if (!emit_enum_def_to_ida(elst, opt))
  {
    err++;
  }
  msg("Pasted into IDA (%d error(s))\n", err);

  return true;
}

bool CStructDialog::OnBtnDeleteDefs()
{
  int nSkip=0;

  if (!isShiftDown() && 
    MessageBox(_hwndDlg, "Are you sure you want to continue?", "Confirm", MB_YESNO | MB_ICONWARNING) == IDNO)
  {
    return false;
  }

  // Sort the structs, so that the least dependant are out first!
  _data_lst.sort();
  for (data_type_def_list_t::iterator it = _data_lst.begin();
    it != _data_lst.end();
    ++it)
  {
    data_type_def &def = *it;
    tid_t tid = get_struc_id(def.name.c_str());
    if (tid == BADNODE)
    {
      nSkip++;
      continue;
    }
    
    struc_t *sptr = get_struc(tid);
    if (sptr == 0)
    {
      nSkip++;
      continue;
    }
    del_struc(sptr);
  }

  _data_lst.clear();

  // Delete enum
  for (enum_type_def_list_t::iterator it = _enum_lst.begin();
    it != _enum_lst.end();
    ++it)
  {
    enum_type_def_t &def = *it;
    del_enum(def.id);
  }
  _enum_lst.clear();

  ClearDataList();

  return (nSkip == 0);
}