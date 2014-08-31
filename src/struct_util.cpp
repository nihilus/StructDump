#include "struct_util.h"

std::string &data_member_def_t::get_comment()
{
  if (repeat_comment.size())
    return repeat_comment;
  else
    return non_repeat_comment;
}

void data_member_def_t::clear()
{
  data_type = 0;
  array_size = 0;
  size = 0;
  is_pointer = false;
  name = "";
  repeat_comment = non_repeat_comment = "";
}

data_member_def_t::data_member_def_t()
{
  clear();
}

data_member_def_t::data_member_def_t(const data_member_def_t &rhs)
{
  assign(rhs);
}
data_member_def_t &data_member_def_t::operator=(data_member_def_t &rhs)
{
  assign(rhs);
  return *this;
}

void data_member_def_t::assign(const data_member_def_t &rhs)
{
  name = rhs.name;
  data_type = rhs.data_type;
  array_size = rhs.array_size;
  is_pointer = rhs.is_pointer;
  repeat_comment = rhs.repeat_comment;
  non_repeat_comment = rhs.non_repeat_comment;
  flag = rhs.flag;
  props = rhs.props;
  size = rhs.size;
  custom_type = rhs.custom_type;
  offs = rhs.offs;
}

data_type_def &data_type_def::operator=(const data_type_def &rhs)
{
  assign(rhs);
  return *this;
}

data_type_def::data_type_def(const data_type_def &rhs)
{
  assign(rhs);
}

bool data_type_def::operator <(const data_type_def &rhs)
{
  return depth < rhs.depth;
}

bool data_type_def::operator==(const data_type_def &rhs)
{
  return name == rhs.name;
}

std::string &data_type_def::get_comment()
{
  if (repeat_comment.size())
    return repeat_comment;
  else
    return non_repeat_comment;
}

void data_type_def::clear()
{
  type = 0;
  props = 0;
  size = 0;
  depth = 0;
  id = 0;
  members.clear();
  repeat_comment = non_repeat_comment = "";
}

data_type_def::data_type_def()
{
  clear();
}

void data_type_def::assign(const data_type_def &rhs)
{
  type = rhs.type;
  name = rhs.name;
  props = rhs.props;
  members.assign(rhs.members.begin(), rhs.members.end());
  repeat_comment = rhs.repeat_comment;
  non_repeat_comment = rhs.non_repeat_comment;
  size = rhs.size;
  depth = rhs.depth;
  id = rhs.id;
}

// ==================================================================

void enum_type_def_t::clear()
{
  id = 0;
  name = repeatable_comment = non_repeatable_comment = "";
  is_bitfield = false;
  flags = 0;
  members.clear();
}

enum_type_def_t::enum_type_def_t()
{
  clear();
}

enum_type_def_t::enum_type_def_t(const enum_type_def_t &rhs)
{
  assign(rhs);
}

void enum_type_def_t::assign(const enum_type_def_t &rhs)
{
  id = rhs.id;
  name = rhs.name;
  non_repeatable_comment = rhs.non_repeatable_comment;
  repeatable_comment = rhs.repeatable_comment;
  is_bitfield = rhs.is_bitfield;
  flags = rhs.flags;
  members.assign(rhs.members.begin(), rhs.members.end());
}

std::string &enum_type_def_t::get_comment()
{
  if (repeatable_comment.size())
    return repeatable_comment;
  else
    return non_repeatable_comment;
}

// ------------------------------------------------

std::string &enum_member_def_t::get_comment()
{
  if (repeatable_comment.size())
    return repeatable_comment;
  else
    return non_repeatable_comment;
}

void enum_member_def_t::assign(const enum_member_def_t &rhs)
{
  id = rhs.id;
  name = rhs.name;
  non_repeatable_comment = rhs.non_repeatable_comment;
  repeatable_comment = rhs.repeatable_comment;
  mask = rhs.mask;
  val = rhs.val;
  serial = rhs.serial;
}

void enum_member_def_t::clear()
{
  id   = 0;
  name = repeatable_comment = non_repeatable_comment = "";
  mask = DEFMASK;
  val  = 0;
  serial = 0;
}

enum_member_def_t::enum_member_def_t()
{
  clear();
}

enum_member_def_t::enum_member_def_t(const enum_member_def_t &rhs)
{
  assign(rhs);
}

/*!
\brief Given a struc_t ptr it enumerates all its members.
\return It returns the depth of the given structure
\note This method is recursive, thus if a structure has dependencies the dependant structs will also be enumarated
*/
int enum_structs(struc_t *st, data_type_def_list_t &def_list)
{
  char str[MAXSTR], str2[MAXSTR];
  int gap_cnt = 1;
  data_member_def_t mdef;
  data_type_def def;
  asize_t ofs = 0;
  int max_depth = 0, cur_depth = 0;

  // Retrieve name
  str[0] = 0;
  get_struc_name(st->id, str, sizeof(str));
  def.name = str;

  // Retrieve the ID
  def.id = st->id;

  // If struct already there, skip it!
  data_type_def_list_t::iterator it;

  it = std::find(def_list.begin(), def_list.end(), def);
  if (it != def_list.end())
  {
    // Return this data_type's depth
    return it->depth;
  }

  str[0] = 0;
  str2[0] = 0;

  // Retrieve comments (first repeatable)
  get_struc_cmt(st->id, true, str, sizeof(str));

  def.repeat_comment = str;

  // Get non-repeatable comment
  get_struc_cmt(st->id, false, str2, sizeof(str2));
  def.non_repeat_comment = str2;

  // No repeatable? get non repeatable
  if (strlen(str) == 0)
  {
    qstrncpy(str, str2, MAXSTR);
  }

  // Is it a union?
  def.type = st->is_union() ? DT_UNION : DT_STRUCT;

  // store the structure's size
  def.size = get_struc_size(st->id);

  // Enumerate all members
  for (size_t i=0;i<st->memqty;i++)
  {
    member_t &mem = st->members[i];

    // Assume unknown data type
    mdef.data_type = DT_UNASSIGNED;

    mdef.props = mem.props & ~(SF_TYPLIB | SF_NOLIST);
    mdef.flag  = mem.flag;

    // Store member's offset
    mdef.offs = st->is_union() ? 0 : mem.soff;

    // Unexpected beginning of member? (gap)
    if (mem.soff != ofs)
    {
      asize_t gap_size = mem.soff - ofs;

      mdef.clear();
      mdef.array_size = gap_size;
      mdef.is_pointer = false;
      mdef.data_type = DT_BYTE;
      mdef.size = 1;

      qsnprintf(str, MAXSTR, "rsvd%d", gap_cnt);
      mdef.name = str;

      // Insert element to fill the gap
      def.members.push_back(mdef);
      gap_cnt++;
    }

    // Retrieve member's type information
    typeinfo_t mem_ti;

    retrieve_member_info(&mem, &mem_ti);

    // Get data-type size of member
    asize_t dt_mem_size = get_data_type_size(mem.flag, &mem_ti);

    mdef.size = dt_mem_size;

    // Get member's size
    asize_t mem_size = get_member_size(&mem);

    // Get the member's name
    get_member_name(mem.id, str, sizeof(str));
    mdef.name = str;

    mdef.data_type = 0;

    // Get the non-repeatable comment; (for special markup)
    str[0] = 0;
    str2[0] = 0;
    get_member_cmt(mem.id, false, str, sizeof(str));

    mdef.non_repeat_comment = str;

    // Check markup existance
    if (strlen(str))
    {
      // Example: (ptr_to:MYSTRUCT)
      const char *user_ptr = "(ptr_to:";
      char *pStart;
      if ((pStart = strstr(str, user_ptr)) != 0)
      {
        char *pEnd = strchr(pStart, ')');
        size_t len = strlen(user_ptr);
        mdef.custom_type.assign(pStart+len, pEnd - pStart - len);
        mdef.data_type |= DT_FLAG_PTR_TO;
      }
    }

    // Get repeatable comments
    get_member_cmt(mem.id, true, str2, sizeof(str2));

    // No non-repeatable comments? get repeatable
    if (strlen(str) == 0)
      qstrncpy(str, str2, MAXSTR);

    // Is this member a structure?
    if (isStruct(mem.flag))
    {
      // Get a pointer to the nested struct
      struc_t *nested_st = get_sptr(&mem);

      // Retrieve struct's name
      get_struc_name(nested_st->id, str, MAXSTR);

      mdef.data_type   = DT_CUSTOM;
      mdef.custom_type = str;

      // Since IDA does not allow circular struct definition, we can safely recurse
      int cur_depth = enum_structs(nested_st, def_list) + 1;
      if (max_depth < cur_depth)
        max_depth  = cur_depth;
    }
    else if (isWord(mem.flag))
    {
      mdef.data_type |= DT_WORD;
    }
    if (isFloat(mem.flag)) // size = 4
    {
      mdef.data_type |= DT_FLOAT;
    }
    else if (isDwrd(mem.flag)) // size = 4
    {
      mdef.data_type |= DT_DWORD;
    }
    else if (isDouble(mem.flag)) // size = 8
    {
      mdef.data_type |= DT_DOUBLE;
    }
    else if (isQwrd(mem.flag))
    {
      mdef.data_type |= DT_QWORD;
    }
    else if (isPackReal(mem.flag)) // size = 10
    {
      mdef.data_type |= DT_PACKREAL;
    }
    else if (isTbyt(mem.flag)) // size = 10
    {
      mdef.data_type |= DT_TBYTE;
    }
    else if (isOwrd(mem.flag)) // size = 16
    {
      mdef.data_type |= DT_BYTE16;
    }
    else if (isASCII(mem.flag))
    {
      if (dt_mem_size == 1)
      {
        mdef.data_type = DT_CHAR;
      }
      else if (dt_mem_size == 2)
      {
        mdef.data_type = DT_WIDECHAR;
      }
    }
    else if (isByte(mem.flag))
    {
      mdef.data_type |= DT_BYTE;
    }

    // Is it an offset?
    mdef.is_pointer = isOff0(mem.flag);

    // Is is an enum?
    if (isEnum0(mem.flag))
    {
      // Get the name of the enum
      get_enum_name(mem_ti.ec.tid, str, sizeof(str));

      mdef.custom_type = str;
      mdef.data_type |= DT_ENUM;
    }

    // Divide the member's size over the member's data type size
    mdef.array_size = mem_size / dt_mem_size;

    // We expect next member to begin at the end of last member
    ofs = mem.eoff;

    // store item
    def.members.push_back(mdef);
  }

  // store the level
  def.depth = max_depth;

  // insert item into list
  def_list.push_back(def);

  return max_depth;
}

/*!
\brief Given an enum it enumerates all its members.
*/
bool enum_enum(enum_t e, enum_type_def_t &def)
{
  char buf[MAXSTR];

  def.clear();

  // Get the enum name
  if (get_enum_name(e, buf, MAXSTR) == -1)
    return false;

  // How many members?
  size_t mem_qty = get_enum_size(e);

  enum_member_def_t mdef;

  def.name           = buf;
  def.is_bitfield    = is_bf(e);
  def.id             = e;
  def.flags          = get_enum_flag(e);

  buf[0] = 0;
  get_enum_cmt(e, false, buf, MAXSTR);
  if (buf[0])
    def.non_repeatable_comment = buf;

  buf[0] = 0;
  get_enum_cmt(e, true, buf, MAXSTR);
  if (buf[0])
    def.repeatable_comment = buf;

  for (bmask_t mask = get_first_bmask(e);;mask = get_next_bmask(e, mask))
  {
    for (uval_t v = get_first_const(e, mask);;v = get_next_const(e, v, mask))
    {
      const_t main_cid;
      uchar serial = 0;

      for (const_t cid=main_cid=get_first_serial_const(e, v, &serial, mask);
        cid != BADNODE;cid = get_next_serial_const(main_cid, &serial) )
      {
        mdef.clear();

        buf[0] = 0;
        get_const_name(cid, buf, MAXSTR);

        mdef.name = buf;
        mdef.mask = mask;
        mdef.val  = v;
        mdef.id   = cid;
        mdef.serial = (int)serial;

        buf[0] = 0;
        get_const_cmt(cid, false, buf, MAXSTR);
        if (buf[0])
          mdef.non_repeatable_comment = buf;

        buf[0] = 0;
        get_const_cmt(cid, true, buf, MAXSTR);
        if (buf[0])
          mdef.repeatable_comment = buf;

        def.members.push_back(mdef);
      }
      if (v == BADNODE)
        break;
    }
    if (mask == BADNODE)
      break;
  }
  return true;
}

const char *num_to_struct_util_consts_str(unsigned long val)
{
  if (val == 0x01)
    return "DT_BYTE";
  else if (val == 0x02)
    return "DT_WORD";
  else if (val == 0x04)
    return "DT_DWORD";
  else if (val == 0x08)
    return "DT_FLOAT";
  else if (val == 0x10)
    return "DT_DOUBLE";
  else if (val == 0x20)
    return "DT_TBYTE";
  else if (val == 0x40)
    return "DT_PACKREAL";
  else if (val == 0x80)
    return "DT_QWORD";
  else if (val == 0x100)
    return "DT_BYTE16";
  else if (val == 0x200)
    return "DT_STRUCT";
  else if (val == 0x400)
    return "DT_ENUM";
  else if (val == 0x800)
    return "DT_UNION";
  else if (val == 0x1000)
    return "DT_CUSTOM";
  else if (val == 0x2000)
    return "DT_FLAG_PTR_TO";
  else if (val == 0x4000)
    return "DT_CHAR";
  else if (val == 0x8000)
    return "DT_WIDECHAR";
  else
    return "DT_UNASSIGNED";
}

unsigned long struct_util_consts_str_to_num(char *str)
{
  if (strcmp(str, "DT_BYTE") == 0)
    return 0x01;
  else if (strcmp(str, "DT_WORD") == 0)
    return 0x02;
  else if (strcmp(str, "DT_DWORD") == 0)
    return 0x04;
  else if (strcmp(str, "DT_FLOAT") == 0)
    return 0x08;
  else if (strcmp(str, "DT_DOUBLE") == 0)
    return 0x10;
  else if (strcmp(str, "DT_TBYTE") == 0)
    return 0x20;
  else if (strcmp(str, "DT_PACKREAL") == 0)
    return 0x40;
  else if (strcmp(str, "DT_QWORD") == 0)
    return 0x80;
  else if (strcmp(str, "DT_BYTE16") == 0)
    return 0x100;
  else if (strcmp(str, "DT_STRUCT") == 0)
    return 0x200;
  else if (strcmp(str, "DT_ENUM") == 0)
    return 0x400;
  else if (strcmp(str, "DT_UNION") == 0)
    return 0x800;
  else if (strcmp(str, "DT_CUSTOM") == 0)
    return 0x1000;
  else if (strcmp(str, "DT_FLAG_PTR_TO") == 0)
    return 0x2000;
  else if (strcmp(str, "DT_CHAR") == 0)
    return 0x4000;
  else if (strcmp(str, "DT_WIDECHAR") == 0)
    return 0x8000;
  else
    return 0x0;
}