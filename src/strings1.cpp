#define USE_DANGEROUS_FUNCTIONS
#define _CRT_SECURE_NO_DEPRECATE

#include "Win32InputBox.h"
#include <ida.hpp>
#include <idp.hpp>
#include <expr.hpp>
#include <bytes.hpp>
#include <loader.hpp>
#include <kernwin.hpp>
#include <struct.hpp>
#include <enum.hpp>

extern plugin_t PLUGIN;

void enum_members1(struc_t *st)
{
  char buf[MAXSTR];
  for (size_t i=0;i<st->memqty;i++)
  {
    member_t &mem = st->members[i];
    get_member_name(mem.id, buf, sizeof(buf));
    msg("member[%d], name=%s; %a-%a ; id=%d; flags=%x\n", i, buf, mem.soff, mem.eoff, mem.id, mem.flag);
  }
}

void enum_members3(struc_t *st)
{
  asize_t ofs = get_struc_first_offset(st);
  ssize_t m_idx;
  char buf[MAXSTR];

  while ((m_idx = get_next_member_idx(st, ofs)) != BADADDR)
  {
    member_t *mem = get_member(st, ofs);
    get_member_name(mem->id, buf, sizeof(buf));
    msg("name=%s idx=%a ; ofs=%a\n", buf, m_idx, ofs);
    ofs += mem->eoff;
  }
}

void enum_members2(struc_t *st)
{
  char buf[MAXSTR];
  type_t type[MAXSTR] = {0};
  int gap_cnt = 1;

  asize_t ofs = 0, gapend = BADADDR, gapstart = BADADDR;

  for (size_t i=0;i<st->memqty;i++)
  {
    member_t &mem = st->members[i];

    // unexpected beginning of member?
    if (mem.soff != ofs)
    {
//      msg("gap detected @ %a!\n", ofs);
      gapstart = ofs;
    }

    if (gapstart != BADADDR)
    {
      gapend = mem.soff;
      msg("char pad%d[%d]\n", gap_cnt, gapend - gapstart);
      //msg("gap size=%a\n", gapend - gapstart);
      gapend = gapstart = BADADDR;
      gap_cnt++;
    }

    typeinfo_t mem_ti;
    retrieve_member_info(&mem, &mem_ti);

    // data type size of member
    asize_t dt_mem_size = get_data_type_size(mem.flag, &mem_ti);

    // member size
    asize_t mem_size = get_member_size(&mem);

    // get the member's name
    get_member_name(mem.id, buf, sizeof(buf));

    char dtype[MAXSTR];
    char arraystr[MAXSTR];
    char typemod[10];

    arraystr[0] = 0;
    typemod[0] = 0;

    if (isWord(mem.flag))
      strcpy(dtype, "unsigned short");
    else if (isDwrd(mem.flag))
      strcpy(dtype, "unsigned long");
    else if (isByte(mem.flag))
      strcpy(dtype, "char");
    else if (isStruct(mem.flag))
    {
      struc_t *nested_st = get_sptr(&mem);

      get_struc_name(nested_st->id, dtype, MAXSTR);
    }
    else 
      strcpy(dtype, "user_type");

    if (isOff0(mem.flag))
    {
      strcpy(typemod, "*");
    }

    if (isEnum0(mem.flag))
    {
      get_enum_name(mem_ti.ec.tid, dtype, sizeof(dtype));
    }

    asize_t ar_size = mem_size / dt_mem_size;

    // an array?
    if (ar_size > 1)
    {
      sprintf(arraystr, "[%d]", ar_size);
    }

    char out[100];
    sprintf(out, "%s " /* type */

                 "%s" /* typemodif */ "%s" /* varname */ "%s" /*array*/ ";", dtype, typemod, buf, arraystr);
    msg("%s\n", out);


/*
inline bool idaapi isByte  (flags_t F)   { return isData(F) && (F & DT_TYPE) == FF_BYTE; }
inline bool idaapi isWord  (flags_t F)   { return isData(F) && (F & DT_TYPE) == FF_WORD; }
inline bool idaapi isDwrd  (flags_t F)   { return isData(F) && (F & DT_TYPE) == FF_DWRD; }
inline bool idaapi isQwrd  (flags_t F)   { return isData(F) && (F & DT_TYPE) == FF_QWRD; }
inline bool idaapi isOwrd  (flags_t F)   { return isData(F) && (F & DT_TYPE) == FF_OWRD; }
inline bool idaapi isTbyt  (flags_t F)   { return isData(F) && (F & DT_TYPE) == FF_TBYT; }
inline bool idaapi isFloat (flags_t F)   { return isData(F) && (F & DT_TYPE) == FF_FLOAT; }
inline bool idaapi isDouble(flags_t F)   { return isData(F) && (F & DT_TYPE) == FF_DOUBLE; }
inline bool idaapi isPackReal(flags_t F) { return isData(F) && (F & DT_TYPE) == FF_PACKREAL; }
inline bool idaapi isASCII (flags_t F)   { return isData(F) && (F & DT_TYPE) == FF_ASCI; }
inline bool idaapi isStruct(flags_t F)   { return isData(F) && (F & DT_TYPE) == FF_STRU; }
inline bool idaapi is3byte (flags_t F)   { return isData(F) && (F & DT_TYPE) == FF_3BYTE; }

*/
    /*
    msg("member[%d], name=%s; %a-%a ; id=%d; flags=%x type=%s dt_mem_size=%a mem_size=%a\n", 
      i, 
      buf, 
      mem.soff, 
      mem.eoff, 
      mem.id, 
      mem.flag,
      type,
      dt_mem_size,
      mem_size);
    */
    // we expect next member to begin @ end of last member
    ofs = mem.eoff;
  }
}

void idaapi run(int arg)
{
  char *st_name = "struc_2";
  char buf[MAXSTR];
  static char st_name_buf[100];

  if (arg == 0)
  {
    strcpy(buf, st_name);

    CWin32InputBox::InputBox("Struct name", "Enter struct name:", st_name_buf, sizeof(st_name_buf));
    st_name = st_name_buf;
  }
  
  size_t st_qty = get_struc_qty();

  msg("we have: %d structs\n", st_qty);

  // find the structure by name
  tid_t id = get_struc_id(st_name);

  // get the size of the structure
  asize_t st_sz = get_struc_size(id);

  msg("sizeof(%s) = %d\n", st_name, st_sz);

  struc_t *st = get_struc(id);

  enum_members2(st);
  return;
}

int idaapi init(void)
{
  run(-1);
  return PLUGIN_KEEP;
}


void idaapi term(void)
{
}

//--------------------------------------------------------------------------
char comment[] = "This is a sample plugin. It does nothing useful";

char help[] =
"A sample plugin module\n"
"\n"
"This module shows you how to create plugin modules.\n"
"\n"
"It does nothing useful - just prints a message that is was called\n"
"and shows the current address.\n";


//--------------------------------------------------------------------------
// This is the preferred name of the plugin module in the menu system
// The preferred name may be overriden in plugins.cfg file

char wanted_name[] = "Sample plugin";


// This is the preferred hotkey for the plugin module
// The preferred hotkey may be overriden in plugins.cfg file
// Note: IDA won't tell you if the hotkey is not correct
//       It will just disable the hotkey.

char wanted_hotkey[] = "Alt-z";


//--------------------------------------------------------------------------
//
//      PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  0,           // plugin flags
  init,                 // initialize

  term,                 // terminate. this pointer may be NULL.

  run,                  // invoke plugin

  comment,              // long comment about the plugin
  // it could appear in the status line
  // or as a hint

  help,                 // multiline help about the plugin

  wanted_name,          // the preferred short name of the plugin
  wanted_hotkey         // the preferred hotkey to run the plugin
};