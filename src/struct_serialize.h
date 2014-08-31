#ifndef __STRUCT_SERIALIZE__01112006__
#define __STRUCT_SERIALIZE__01112006__

#include "struct_util.h"
#include <windows.h>
#include <tchar.h>

#define SERIALIZE_SRC_FILE 0
#define SERIALIZE_SRC_URL  1
#define SERIALIZE_SRC_STR  2

struct SerializePack_t
{
  data_type_def_list_t *st;
  enum_type_def_list_t *en;
};

bool Serialize(SerializePack_t &sp, std::string &out);
bool DeSerialize(SerializePack_t &sp, LPCSTR szSrc, int nSrcType);

#endif