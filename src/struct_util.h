#ifndef __01092007__STRUCT_UTIL__
#define __01092007__STRUCT_UTIL__

#include <string>
#include <ida.hpp>
#include <list>
#include <struct.hpp>
#include <algorithm>
#include <enum.hpp>
#include <windows.h>

enum struct_util_consts
{
  DT_UNASSIGNED = 0x0,
  DT_BYTE = 0x01,
  DT_WORD = 0x02,
  DT_DWORD = 0x04,
  DT_FLOAT = 0x08,
  DT_DOUBLE = 0x10,
  DT_TBYTE = 0x20,
  DT_PACKREAL = 0x40,
  DT_QWORD = 0x80,
  DT_BYTE16 = 0x100,
  DT_STRUCT = 0x200,
  DT_ENUM = 0x400,
  DT_UNION = 0x800,
  DT_CUSTOM = 0x1000,
  DT_FLAG_PTR_TO = 0x2000,
  DT_CHAR = 0x4000,
  DT_WIDECHAR = 0x8000
};

struct data_member_def_t
{
private:
  void assign(const data_member_def_t &rhs);

public:
  unsigned long offs;
  std::string name;
  std::string repeat_comment;
  std::string non_repeat_comment;

  std::string custom_type;

  flags_t flag, props;

  int data_type;
  int array_size; // < 1 = not an array, > 1 = array
  bool is_pointer;
  int size;

  void clear();
  data_member_def_t();
  data_member_def_t(const data_member_def_t &rhs);
  data_member_def_t &operator=(data_member_def_t &rhs);
  std::string &get_comment();
};

typedef std::list<data_member_def_t> member_def_list_t;

/*!
\brief Structure holding the definition of an IDA structure
*/
struct data_type_def
{
  void assign(const data_type_def &rhs);

  //! In IDA struct ID
  tid_t id;
  //! 
  int depth;

  //! struct or union, enum (check DT_XXXX values)
  int type;

  //! Structure name
  std::string name;

  //! Repeatable comment
  std::string repeat_comment;

  //! The non-repeatable comment
  std::string non_repeat_comment;

  ulong props;

  member_def_list_t members;

  asize_t size;

  void clear();

  data_type_def();

  data_type_def &operator=(const data_type_def &rhs);
  data_type_def(const data_type_def &rhs);
  bool operator <(const data_type_def &rhs);
  bool operator==(const data_type_def &rhs);
  std::string &get_comment();
};

typedef std::list<data_type_def> data_type_def_list_t;

struct enum_member_def_t
{
  const_t id;

  std::string 
    repeatable_comment, 
    non_repeatable_comment,
    name;

  bmask_t mask;
  uval_t val;
  int serial;
public:
  void assign(const enum_member_def_t &rhs);
  void clear();
  enum_member_def_t();
  enum_member_def_t(const enum_member_def_t &rhs);
  std::string &get_comment();
};

typedef std::list<enum_member_def_t> enum_member_def_list_t;

struct enum_type_def_t
{
  enum_t id;
  std::string 
    repeatable_comment, 
    non_repeatable_comment,
    name;

  bool is_bitfield;
  flags_t flags;

  enum_member_def_list_t members;

public:
  void clear();
  enum_type_def_t();
  enum_type_def_t(const enum_type_def_t &rhs);
  void assign(const enum_type_def_t &rhs);
  std::string &get_comment();
};

typedef std::list<enum_type_def_t> enum_type_def_list_t;

int enum_structs(struc_t *st, data_type_def_list_t &def_list);
unsigned long struct_util_consts_str_to_num(char *str);
const char *num_to_struct_util_consts_str(unsigned long val);
bool enum_enum(enum_t e, enum_type_def_t &def);

#endif