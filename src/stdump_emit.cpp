#include "util.h"
#include "stdump_emit.h" // Emit code
#include <sstream>
#include "struct_serialize.h"

/*!
\brief Reads the configuration value from the INI file
*/
bool emit_options_t::read(char *cfgfile, int lang)
{
  char key[MAX_PATH] = "general";

  // General config
  overwrite_structs = ::GetPrivateProfileInt(key, "overwrite_structs", 0, cfgfile) == 1 ? true : false;

  copy_to_clipboard_as_xml = ::GetPrivateProfileInt(key, "copy_to_clipboard_as_xml", 0, cfgfile) == 1 ? true : false;

  //
  // Parse Language specific settings
  //
  qsnprintf(key, sizeof(key), "lang%d", lang);
  const char *values[] =
  {
    // key, default value, store offset in this struct
    "dt_byte", "char", (const char *)&s_dt_byte,
    "dt_word", "unsigned short", (const char *)&s_dt_word,
    "dt_dword", "long", (const char *)&s_dt_dword,
    "dt_float", "float", (const char *)&s_dt_float,
    "dt_double", "double", (const char *)&s_dt_double,
    "dt_char", "char", (const char *)&s_dt_char,
    "dt_widechar", "wchar_t", (const char *)&s_dt_widechar,
    "dt_tbyte", "char10", (const char *)&s_dt_tbyte,
    "dt_packedreal", "char10", (const char *)&s_dt_packedreal,
    "dt_qword", "double", (const char *)&s_dt_qword,
    "dt_byte16", "char16", (const char *)&s_dt_byte16,
    "struct", "struct", (const char *)&s_struct,
    // If set, then all pointers will be expressed using this value
    "void_ptr", "", (const char *)&s_void_ptr, 
    "union", "union", (const char *)&s_union,
    "enum", "enum", (const char *)&s_enum,
    "num_prefix", "", (const char *)&s_num_prefix
  };

  char str[MAX_PATH];

  for (int i=0;i<sizeof(values)/sizeof(values[0]);i+=3)
  {
    str[0] = 0;
    ::GetPrivateProfileString(key, values[i], values[i+1], str, sizeof(str), cfgfile);
    std::string &s = *(std::string *)values[i+2];
    s = str;
  }

  ptr_size  = ::GetPrivateProfileInt(key, "ptrsize", inf.cc.size_i, cfgfile);
  items_per_line  = ::GetPrivateProfileInt(key, "items_per_line", 10, cfgfile);
  cmt_style = ::GetPrivateProfileInt(key, "cmt_style", ECS_NONE, cfgfile);
  cmt_sizeof = ::GetPrivateProfileInt(key, "cmt_sizeof", 0, cfgfile) == 1 ? true : false;
  cmt_showoffs = ::GetPrivateProfileInt(key, "cmt_showoffs", 0, cfgfile) == 1 ? true : false;
  enum_as_const = ::GetPrivateProfileInt(key, "enum_as_const", 0, cfgfile) == 1 ? true : false;

  return true;
}

/*!
\brief Emits delphi definitions from enums
*/
bool emit_enum_defs_delphi(enum_type_def_list_t &def_lst, std::string &fn, bool bToFile, emit_options_t &opt)
{
  std::ostringstream os;

  size_t nmembers = 0;

  for (enum_type_def_list_t::iterator e_it=def_lst.begin();e_it != def_lst.end();++e_it)
  {
    enum_type_def_t &def = *e_it;
    if (def.get_comment().size())
    {
      if (opt.cmt_style == ECS_PASCAL_MULTI)
        os << "{ " << def.get_comment() << "}\n";
      else if (opt.cmt_style == ECS_DELPHI_ONE)
        os << "// " << def.get_comment() << "\n";
    }
    else
    {
      if (opt.cmt_style == ECS_PASCAL_MULTI)
        os << "{ " << def.name << " }\n";
      else if (opt.cmt_style == ECS_DELPHI_ONE)
        os << "//" << def.name << "\n";
    }

    for (enum_member_def_list_t::iterator m_it=def.members.begin();m_it!=def.members.end();++m_it)
    {
      nmembers++;

      enum_member_def_t &mdef = *m_it;
      os << "\t" << mdef.name << "=" << mdef.val << ";";

      if (mdef.get_comment().size())
      {
        if (opt.cmt_style == ECS_PASCAL_MULTI)
          os << "\t{ " << mdef.get_comment() << "}\n";
        else if (opt.cmt_style == ECS_DELPHI_ONE)
          os << "\t// " << mdef.get_comment() << "\n";
      }
      else
      {
        os << "\n";
      }
    }
  }

  bool bRet = false;
  std::string s;

  if (nmembers > 0)
    s = "const\n";

  s += os.str();

  if (bToFile)
  {
    bRet = str_to_file(fn.c_str(), s);
  }
  else
  {
    fn = s;
    bRet = true;
  }
  if (::GetAsyncKeyState(VK_LMENU) < 0)
  {
    msg(os.str().c_str());
  }
  return bRet;
}

/*!
\brief Emits C or C++ definitions from enums
*/
bool emit_enum_defs_c(enum_type_def_list_t &def_lst, std::string &fn, bool bToFile, emit_options_t &opt)
{
  std::ostringstream os;

  if (opt.enum_as_const)
  {
    for (enum_type_def_list_t::iterator e_it=def_lst.begin();e_it != def_lst.end();++e_it)
    {
      enum_type_def_t &def = *e_it;
      if (def.get_comment().size())
      {
        if (opt.cmt_style == ECS_C_NORMAL)
          os << "/* " << def.get_comment() << "*/\n";
        else if (opt.cmt_style == ECS_C_DOXY)
          os << "/*!\n  \\brief " << def.get_comment() << "\n*/\n";
      }
      else
      {
        if (opt.cmt_style == ECS_C_NORMAL)
          os << "/*" << def.name << "*/\n";
        else if (opt.cmt_style == ECS_CPP_NORMAL)
        os << "//" << def.name << "\n";
      }

      for (enum_member_def_list_t::iterator m_it=def.members.begin();m_it!=def.members.end();++m_it)
      {
        enum_member_def_t &mdef = *m_it;
        os << "#define " << mdef.name << "\t" << mdef.val;
        if (opt.cmt_style == ECS_C_NORMAL)
          os << "\t/* " << mdef.get_comment() << "*/\n";
        else if (opt.cmt_style == ECS_CPP_NORMAL)
          os << "\t// " << mdef.get_comment() << "\n";
        else
          os << "\n";
      }
    }
  }
  else
  {
    for (enum_type_def_list_t::iterator e_it=def_lst.begin();e_it != def_lst.end();++e_it)
    {
      enum_type_def_t &def = *e_it;
      if (def.get_comment().size())
      {
        if (opt.cmt_style == ECS_C_NORMAL)
          os << "/* " << def.get_comment() << "*/\n";
        else if (opt.cmt_style == ECS_C_DOXY)
          os << "/*!\n  \\brief " << def.get_comment() << "\n*/\n";
      }
      else
      {
        if (opt.cmt_style == ECS_C_NORMAL)
          os << "/*" << def.name << "*/\n";
        else if (opt.cmt_style == ECS_CPP_NORMAL)
          os << "//" << def.name << "\n";
      }

      os << "typedef enum tag" << def.name << "\n{\n";

      size_t i=0, sz=def.members.size();
      for (enum_member_def_list_t::iterator m_it=def.members.begin();m_it!=def.members.end();++m_it)
      {
        enum_member_def_t &mdef = *m_it;
        os << "\t" << mdef.name << " = " << mdef.val;
        if (++i != sz)
          os << ",";
        if (mdef.get_comment().size())
        {
        if (opt.cmt_style == ECS_C_NORMAL)
          os << "\t/* " << mdef.get_comment() << "*/\n";
        else if (opt.cmt_style == ECS_CPP_NORMAL)
          os << "\t// " << mdef.get_comment() << "\n";
        }
        else
        {
          os << "\n";
        }
      }
      os << "} " << def.name << ";\n";
    }
  }

  bool bRet = false;

  if (bToFile)
  {
    bRet = str_to_file(fn.c_str(), os.str());
  }
  else
  {
    fn = os.str();
    bRet = true;
  }
  if (::GetAsyncKeyState(VK_LMENU) < 0)
  {
    msg(os.str().c_str());
  }
  return bRet;
}

/*!
\brief Table creator
*/
bool emit_table(void *data, int lang, size_t count, size_t size1, emit_options_t &opt, std::string &out)
{
  unsigned short int *w = (unsigned short *)data;
  unsigned long *l = (unsigned long *)data;
  unsigned char *u = (unsigned char *)data;

  std::ostringstream os;

  char buf[MAXSTR];

  const char *sz_prefix = opt.s_num_prefix.c_str();
  const size_t maxcols = opt.items_per_line, prefix_len = strlen(sz_prefix);
  std::string str;
  bool bLangKnown = true;

  switch(size1)
  {
  case 1:
    str = opt.s_dt_byte;
    break;
  case 2:
    str = opt.s_dt_word;
    break;
  case 4:
    str = opt.s_dt_dword;
    break;
  default:
    return false;
  }

  if (lang == 0)
  {
    qsnprintf(buf, MAXSTR, "const %s data[%d] = \r\n{\r\n", str.c_str(), count);
    str = "};\r\n";
  }
  else if (lang == 1)
  {
    qsnprintf(buf, MAXSTR, "const\r\n\tdata: array[0..%d] of %s = \r\n(", count, str.c_str());
    str = ");\r\n";
  }
  else
    bLangKnown = false;

  if (bLangKnown)
    os.write(buf, strlen(buf));

  size_t col=1;

  for (size_t i=0;i<count;i++)
  {
    switch (size1)
    {
    case 1:
      qsnprintf(buf, MAXSTR, "%02x", *u & 0xFF);
      u++;
      break;
    case 2:
      qsnprintf(buf, MAXSTR,  "%04x", *w & 0xFFFF);
      w++;
      break;
    case 4:
      qsnprintf(buf, MAXSTR,  "%08X", *l & 0xFFFFFFFF);
      l++;
      break;
    }

    os.write(sz_prefix, prefix_len);
    os.write(buf, size1*2);
    if (i != count-1)
      os.write(",", 1);

    if (col % maxcols == 0)
    {
      col = 0;
      os.write("\r\n", 2);
    }
    col++;
  }

  if (col != 1)
    os.write("\r\n", 2);

  if (bLangKnown)
    os.write(str.c_str(), str.size());

  out = os.str().c_str();

  return true;
}

/*!
\brief Data definitions into C/C++ format
*/
bool emit_data_defs_c(data_type_def_list_t &def_lst, std::string &fn, bool bToFile, emit_options_t &opt)
{
  std::ostringstream os;

  def_lst.sort();

  data_type_def_list_t::iterator it;

  // Forward declare only if more than an item
  if (def_lst.size() > 1)
  {
    // Iterate through all
    for (it = def_lst.begin();
      it != def_lst.end();
      ++it)
    {
      data_type_def &def = *it;
      os << (def.type == DT_UNION ? opt.s_union : opt.s_struct) << " " << def.name << ";\n";
    }
  }

  for (it = def_lst.begin();
    it != def_lst.end();
    ++it)
  {
    data_type_def &def = *it;
    if (def.get_comment().size())
    {
      if (opt.cmt_style == ECS_C_NORMAL)
        os << "/* " << def.get_comment() << "*/\n";
      else if (opt.cmt_style == ECS_C_DOXY)
        os << "/*!\n  \\brief " << def.get_comment() << "\n*/\n";
    }

    if (opt.cmt_sizeof)
      os << "// sizeof(" << def.name << ") = " << def.size << "\n";

    os << (def.type == DT_UNION ? opt.s_union : opt.s_struct) << " " << def.name << "\n"
      << "{\n";

    // Dump all members
    for (member_def_list_t::iterator m_it = def.members.begin();
      m_it != def.members.end();
      ++m_it)
    {
      data_member_def_t &mem = *m_it;

      if (mem.get_comment().size())
      {
        if (opt.cmt_style == ECS_C_NORMAL)
          os << "  /* " << mem.get_comment() << " */\n";
        else if (opt.cmt_style == ECS_C_DOXY)
          os << "  /*! " << mem.get_comment() << "*/\n";
      }

      // Ident
      os << "  ";

      if (opt.cmt_showoffs)
      {
        char str[MAXSTR];
        qsnprintf(str, MAXSTR, "/* 0x%a/%d */ ", mem.offs, mem.offs);
        os << str;
      }

      std::string tmp;

      if (has_attr(mem.data_type, DT_CUSTOM))
        tmp = mem.custom_type;
      else if (has_attr(mem.data_type, DT_CHAR))
        tmp = opt.s_dt_char;
      else if (has_attr(mem.data_type, DT_WIDECHAR))
        tmp = opt.s_dt_widechar;
      else if (has_attr(mem.data_type, DT_BYTE))
        tmp = opt.s_dt_byte;
      else if (has_attr(mem.data_type, DT_WORD))
        tmp = opt.s_dt_word;
      else if (has_attr(mem.data_type, DT_DWORD))
        tmp = opt.s_dt_dword;
      else if (has_attr(mem.data_type, DT_QWORD))
        tmp = opt.s_dt_qword;
      else if (has_attr(mem.data_type, DT_DOUBLE))
        tmp = opt.s_dt_double;
      else if (has_attr(mem.data_type, DT_FLOAT))
        tmp = opt.s_dt_float;
      else if (has_attr(mem.data_type, DT_BYTE16))
        tmp = opt.s_dt_byte16;
      else if (has_attr(mem.data_type, DT_TBYTE))
        tmp = opt.s_dt_tbyte;
      else if (has_attr(mem.data_type, DT_PACKREAL))
        tmp = opt.s_dt_packedreal;

      bool bPointer = (mem.size == opt.ptr_size) && mem.is_pointer;

      if (has_attr(mem.data_type, DT_ENUM) && (mem.size == opt.ptr_size))
        tmp = mem.custom_type;

      // Special pointer tag?
      if (bPointer && has_attr(mem.data_type, DT_FLAG_PTR_TO))
      {
        tmp = mem.custom_type;
      }
      // Generic pointer?
      else if (bPointer && opt.s_void_ptr.size())
      {
        tmp = opt.s_void_ptr;
      }

      os << tmp << " ";

      if (bPointer)
        os << "*";

      os << mem.name;

      if (mem.array_size > 1)
        os << "[" << mem.array_size << "]";

      os << ";\n";
    }

    os << "};\n";
  }

  bool bRet = false;

  if (bToFile)
  {
    while (fn.size())
    {
      FILE *fp = qfopen(fn.c_str(), "wb");
      if (fp == NULL)
        break;

      const std::string &s = os.str();
      qfwrite(fp, s.c_str(), s.size());
      qfclose(fp);
      bRet = true;
      break;
    }
  }
  else
  {
    fn = os.str();
  }
  if (::GetAsyncKeyState(VK_LMENU) < 0)
  {
    msg(os.str().c_str());
  }
  return bRet;
}

bool emit_data_defs_delphi(data_type_def_list_t &def_lst, std::string &fn, bool bToFile, emit_options_t &opt)
{
  //emit_options_t opt;
  std::ostringstream os;

  if (def_lst.size() == 0)
    return false;

  // sort elements
  def_lst.sort();

  for (
    data_type_def_list_t::iterator it = def_lst.begin();
    it != def_lst.end();
  ++it)
  {
    data_type_def &def = *it;

    bool bUnion = (def.type == DT_UNION) ? true : false;

    // Display record's comment
    if (def.get_comment().size())
    {
      if (opt.cmt_style == ECS_PASCAL_MULTI)
        os << "{ " << def.get_comment() << "}\n";
      else if (opt.cmt_style == ECS_DELPHI_ONE)
        os << "// " << def.get_comment() << "\n";
    }

    // sizeof?
    if (opt.cmt_sizeof)
      os << "{ sizeof(" << def.name << ") = " << def.size << "}\n";

    // emit the type
    os << "type " << def.name << " = packed " << opt.s_struct << "\n";

    int nUnion = 1;

    if (bUnion && (def.members.size() > 0))
      os << "  case Integer of \n";

    // Dump all members
    for (
      member_def_list_t::iterator m_it = def.members.begin();
      m_it != def.members.end();
    ++m_it)
    {
      data_member_def_t &mem = *m_it;

      if (mem.get_comment().size())
      {
        // Display member's comment
        if (opt.cmt_style == ECS_PASCAL_MULTI)
          os << "  {" << mem.get_comment() << "}\n";
        else if (opt.cmt_style == ECS_DELPHI_ONE)
          os << "  // " << mem.get_comment() << "\n";
      }

      // Ident
      os << "  ";

      if (opt.cmt_showoffs)
      {
        char str[MAXSTR];
        qsnprintf(str, MAXSTR, "{ 0x%a/%d } ", mem.offs, mem.offs);
        os << str;
      }

      std::string tmp;

      if (has_attr(mem.data_type, DT_CUSTOM))
        tmp = mem.custom_type;
      else if (has_attr(mem.data_type, DT_CHAR))
        tmp = opt.s_dt_char;
      else if (has_attr(mem.data_type, DT_WIDECHAR))
        tmp = opt.s_dt_widechar;
      else if (has_attr(mem.data_type, DT_BYTE))
        tmp = opt.s_dt_byte;
      else if (has_attr(mem.data_type, DT_WORD))
        tmp = opt.s_dt_word;
      else if (has_attr(mem.data_type, DT_DWORD))
        tmp = opt.s_dt_dword;
      else if (has_attr(mem.data_type, DT_QWORD))
        tmp = opt.s_dt_qword;
      else if (has_attr(mem.data_type, DT_DOUBLE))
        tmp = opt.s_dt_double;
      else if (has_attr(mem.data_type, DT_FLOAT))
        tmp = opt.s_dt_float;
      else if (has_attr(mem.data_type, DT_BYTE16))
        tmp = opt.s_dt_byte16;
      else if (has_attr(mem.data_type, DT_TBYTE))
        tmp = opt.s_dt_tbyte;
      else if (has_attr(mem.data_type, DT_PACKREAL))
        tmp = opt.s_dt_packedreal;

      bool bPointer = (mem.size == opt.ptr_size) && mem.is_pointer;

      if (has_attr(mem.data_type, DT_ENUM) && (mem.size == opt.ptr_size))
        tmp = opt.s_dt_dword;

      // Special pointer tag?
      if (bPointer && has_attr(mem.data_type, DT_FLAG_PTR_TO))
      {
        tmp = mem.custom_type;
      }
      // Generic pointer?
      else if (bPointer && opt.s_void_ptr.size())
      {
        tmp = opt.s_void_ptr;
      }

      if (bUnion)
        os << nUnion << ": (";

      os << mem.name << ": ";

      if (mem.array_size > 1)
        os << "array[0.." << mem.array_size-1 << "] of ";

      if (bPointer)
        os << "^";

      os << tmp;

      if (bUnion)
        os << ");\n";
      else
        os << ";\n";

      nUnion++;
    }

    os << "end;\n";
  }

  bool bRet = false;
  if (bToFile)
  {
    while (fn.size())
    {
      FILE *fp = qfopen(fn.c_str(), "wb");
      if (fp == NULL)
        break;
      const std::string &s = os.str();
      qfwrite(fp, s.c_str(), s.size());
      qfclose(fp);
      bRet = true;
      break;
    }
  }
  else
  {
    fn = os.str();
  }
  if (::GetAsyncKeyState(VK_LMENU) < 0)
  {
    msg(os.str().c_str());
  }

  return bRet;
}

/*!
\brief Emit the definition into the clipboard
*/
int emit_to_clipboard(data_type_def_list_t &lst, enum_type_def_list_t &elist, emit_options_t &opt)
{
  int ret = STDUMP_ERR_CLIPBOARD;
  std::string str;

  // Serialize pack
  SerializePack_t sp = {&lst, &elist};

  // Serialize all the data
  if (!Serialize(sp, str))
    return STDUMP_ERR_SERIALIZATION;

  HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, str.size()+1);
  if (hMem == 0)
    return STDUMP_ERR_NO_CLIP_MEM;

  bool bCleanUp = true;

  do 
  {
    char *pMem = (char *) ::GlobalLock(hMem);
    if (pMem == 0)
      break;

    lstrcpy(pMem, str.c_str());
    ::GlobalUnlock(hMem);

    if (!::OpenClipboard(get_ida_hwnd()))
      break;

    ::EmptyClipboard();

    // copy XML data
    if (!::SetClipboardData(g_CF_IDA_XML, hMem))
      break;

    if (opt.copy_to_clipboard_as_xml == false)
    {
      // Now try to put TEXT data into clipboard
      emit_data_defs_c(lst, str, false, opt);
    }

    // Try to allocate memory
    hMem = ::GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, str.size() + 1);
    if (hMem == 0)
    {
      bCleanUp = false;
      ret = STDUMP_ERR_NO_CLIP_MEM;
      break;
    }

    pMem = (char *) ::GlobalLock(hMem);
    if (pMem == 0)
      break;

    lstrcpy(pMem, str.c_str());
    ::GlobalUnlock(hMem);

    if (!::SetClipboardData(CF_TEXT, hMem))
      break;

    bCleanUp = false;
  } while(0);

  if (bCleanUp)
    ::GlobalFree(hMem);

  ::CloseClipboard();

  return bCleanUp == false;
}

/*!
\brief Emit a structure definition into IDA
*/
bool emit_enum_def_to_ida(enum_type_def_list_t &def_lst, emit_options_t &opt)
{
  for (enum_type_def_list_t::iterator e_it=def_lst.begin();e_it != def_lst.end();++e_it)
  {
    enum_type_def_t &def = *e_it;

    // Skip existing enums
    if (get_enum((def.name.c_str())) != BADNODE) 
    {
        msg("Enum %s already exists, skipping...\n", def.name.c_str());
        continue;
    }

    enum_t eid = add_enum(BADADDR, def.name.c_str(), def.flags);
    if (eid == BADNODE)
    {
      msg("Failed while creating enum member: %s\n", def.name.c_str());
      continue;
    }

    if (def.is_bitfield)
      set_enum_bf(eid, true);

    if (def.repeatable_comment.size())
      set_enum_cmt(eid, def.repeatable_comment.c_str(), true);
    
    if (def.non_repeatable_comment.size())
      set_enum_cmt(eid, def.non_repeatable_comment.c_str(), false);

    def.id = eid;

    size_t i=0, sz=def.members.size();
    for (enum_member_def_list_t::iterator m_it=def.members.begin();m_it!=def.members.end();++m_it)
    {
      enum_member_def_t &mdef = *m_it;
      if (add_const(eid, mdef.name.c_str(), mdef.val, mdef.mask) != 0)
      {
        msg("Failed while creating enum member: %s.%s\n", def.name.c_str(), mdef.name.c_str());
        del_enum(eid);
        break;
      }

      const_t cid = get_const(eid, mdef.val, mdef.serial, mdef.mask);
      if (mdef.repeatable_comment.size())
        set_const_cmt(cid, mdef.repeatable_comment.c_str(), true);

      if (mdef.non_repeatable_comment.size())
        set_const_cmt(cid, mdef.non_repeatable_comment.c_str(), false);
    }
  }
  return true;
}

/*!
\brief Emit a structure definition into IDA
*/
bool emit_struct_def_to_ida(data_type_def_list_t &lst, emit_options_t &opt)
{
  // Sort by depth
  lst.sort();

  for (data_type_def_list_t::iterator it=lst.begin();it!=lst.end();++it)
  {
    data_type_def &d = *it;

    struc_t *st;
    tid_t st_id;

    if ((st_id = get_struc_id(d.name.c_str())) == BADNODE)
    {
      // Add the structure
      st_id = add_struc(BADADDR, d.name.c_str(), has_attr(d.type, DT_UNION));
    }
    else
    {
      continue;
    }

    st = get_struc(st_id);

    set_struc_cmt(st_id, d.repeat_comment.c_str(), true);
    set_struc_cmt(st_id, d.non_repeat_comment.c_str(), false);

    for (member_def_list_t::iterator it_mem=d.members.begin();it_mem != d.members.end();++it_mem)
    {
      data_member_def_t &mem = *it_mem;
      bool bCustom = has_attr(mem.data_type, DT_CUSTOM);
      typeinfo_t mt = {0};

      tid_t st_id_custom;

      if (bCustom)
      {
        st_id_custom = get_struc_id(mem.custom_type.c_str());
        if (st_id_custom == 0)
          break;
        mt.tid = st_id_custom;
      }

      int nmem = add_struc_member(
        st, 
        mem.name.c_str(), 
        BADADDR, 
        mem.flag, 
        bCustom ? &mt : 0, 
        mem.array_size * mem.size);

      if (nmem != 0)
        break;

      member_t *mem_t = get_member_by_name(st, mem.name.c_str());
      if (mem_t == 0)
        break;
      set_member_cmt(mem_t, mem.repeat_comment.c_str(), true);
      set_member_cmt(mem_t, mem.non_repeat_comment.c_str(), false);
    }
  }
  return true;
}