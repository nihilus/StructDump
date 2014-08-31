#ifndef __STDUMP_08212006__
#define __STDUMP_08212006__

#include <loader.hpp>
#include <idp.hpp>

enum consts_e
{
  ECS_C_DOXY = 1, // C; emit comment style = doxygen
  ECS_C_NORMAL = 2, // C; normal comments
  ECS_CPP_NORMAL = 3, // C++ single line comment

  ECS_PASCAL_MULTI = 1, // Pascal multiline comments
  ECS_DELPHI_ONE = 2, // Delphi C style comments
  ECS_NONE = 0, // no comments

  // Emit language
  EMITLANG_CPP = 0,
  EMITLANG_DELPHI = 1,
  EMITLANG_MAX = 2,

  // Error codes
  STDUMP_ERR_NONE = 0,
  STDUMP_ERR_STRUCT_ALREADY_EXISTS = 1,
  STDUMP_ERR_NO_CLIP_MEM = 2,
  STDUMP_ERR_CLIPBOARD = 3,
  STDUMP_ERR_SERIALIZATION = 4,
};

int idaapi init(void);
void idaapi term(void);
void idaapi run(int arg);

//--------------------------------------------------------------------------
//
//      PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------
extern plugin_t PLUGIN;

char *get_cfg_filename();
void display_structs(data_type_def_list_t &def_lst);
HWND get_ida_hwnd();
extern UINT g_CF_IDA_XML;

#endif