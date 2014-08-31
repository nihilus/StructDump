#ifndef __STDUMP__EMIT_08212006__
#define __STDUMP__EMIT_08212006__

#include "struct_util.h"
#include "stdump.h"

/*!
\brief This structure is responsible for parsing the structdump config file
*/
struct emit_options_t
{
  //! Show the size of a structure as comment
  bool cmt_sizeof;
  
  //! ECS_XXX variable
  int cmt_style;
  
  //! the pointer size
  int ptr_size;
  
  //! items per line when making tables
  int items_per_line;

  //! show structs offset as comments
  bool cmt_showoffs;

  //! show enums as constants or as enum type?
  bool enum_as_const;

  bool overwrite_structs;
  bool copy_to_clipboard_as_xml;

  std::string s_dt_byte;
  std::string s_dt_word;
  std::string s_dt_dword;
  std::string s_dt_float;
  std::string s_dt_double;
  std::string s_dt_tbyte;
  std::string s_dt_packedreal;
  std::string s_dt_qword;
  std::string s_dt_byte16;
  std::string s_dt_char;
  std::string s_dt_widechar;
  std::string s_struct;
  std::string s_union;
  std::string s_enum;
  std::string s_void_ptr;
  std::string s_num_prefix;
  bool read(char *cfgfile, int lang);
};

typedef bool (*emit_data_defs_lang)(data_type_def_list_t &def_lst, std::string &fn, bool bToFile, emit_options_t &opt);
typedef bool (*emit_enum_defs_lang)(enum_type_def_list_t &def_lst, std::string &fn, bool bToFile, emit_options_t &opt);

bool emit_data_defs_c(data_type_def_list_t &def_lst, std::string &fn, bool bToFile, emit_options_t &opt);
bool emit_data_defs_delphi(data_type_def_list_t &def_lst, std::string &fn, bool bToFile, emit_options_t &opt);
bool emit_struct_def_to_ida(data_type_def_list_t &lst, emit_options_t &opt);
bool emit_enum_defs_c(enum_type_def_list_t &def_lst, std::string &fn, bool bToFile, emit_options_t &opt);
bool emit_enum_defs_delphi(enum_type_def_list_t &def_lst, std::string &fn, bool bToFile, emit_options_t &opt);

int emit_to_clipboard(data_type_def_list_t &lst, enum_type_def_list_t &elist, emit_options_t &opt);
bool emit_enum_def_to_ida(enum_type_def_list_t &def_lst, emit_options_t &opt);

bool emit_table(void *data, int lang, size_t count, size_t size1, emit_options_t &opt, std::string &out);

#endif