#include "struct_serialize.h"
#include "XmlNodeWrapper.h"
#include "util.h"

static LPCSTR STR_XML_ROOT       = "IDAPlus";
static LPCSTR STR_XML_STRUCT     = "Struct";
static LPCSTR STR_XML_ENUM       = "Enum";
static LPCSTR STR_XML_MEMBERS    = "Members";
static LPCSTR STR_XML_MEMBER     = "Member";
static LPCSTR STR_XML_COMMENT    = "Comment";
static LPCSTR STR_XML_RCOMMENT   = "RepeatableComment";
static LPCSTR STR_XML_NAME       = "Name";
static LPCSTR STR_XML_TYPE       = "Type";
static LPCSTR STR_XML_FLAG       = "Flag";
static LPCSTR STR_XML_SERIAL     = "Serial";
static LPCSTR STR_XML_VALUE      = "Value";
static LPCSTR STR_XML_MASK       = "Mask";
static LPCSTR STR_XML_PROPS      = "Props";
static LPCSTR STR_XML_SIZE       = "Size";
static LPCSTR STR_XML_ISBF       = "IsBitField";
static LPCSTR STR_XML_ISPTR      = "IsPointer";
static LPCSTR STR_XML_ARSIZE     = "ArraySize";
static LPCSTR STR_XML_DEPTH      = "Depth";
static LPCSTR STR_XML_CUSTOMTYPE = "CustomType";

/*!
\brief Given an XML document, this function deserializes the XML into data_type_def_list record
*/
void DeSerializeStructs(CXmlNodeWrapper &root, data_type_def_list_t &def_list)
{
  data_type_def def;
  data_member_def_t mdef;

  CXmlNodeWrapper node;

  long nStructs = root.NumNodes();

  for (long iStructs=0;iStructs<nStructs;iStructs++)
  {
    node = root.GetNode(iStructs);
    if (node.Name() != STR_XML_STRUCT)
      continue;

    def.clear();

    def.name  = node.GetValue(STR_XML_NAME);
    def.props = parse_ulong(node.GetValue(STR_XML_PROPS));
    def.depth = parse_ulong(node.GetValue(STR_XML_DEPTH));
    def.type  = struct_util_consts_str_to_num((char *)node.GetValue(STR_XML_TYPE).operator LPCSTR());

    CXmlNodeWrapper comment;

    comment = node.GetNode(STR_XML_COMMENT);
    if (comment.IsValid())
      def.non_repeat_comment = comment.GetText();

    comment = node.GetNode(STR_XML_RCOMMENT);
    if (comment.IsValid())
      def.repeat_comment = comment.GetText();

    CXmlNodeWrapper members;
    members = node.GetNode(STR_XML_MEMBERS);

    long nMembers = members.NumNodes();
    for (int iMembers=0;iMembers<nMembers;iMembers++)
    {
      node = members.GetNode(iMembers);
      if (!node.IsValid() || node.Name() != STR_XML_MEMBER)
        continue;

      mdef.clear();
      mdef.name        = node.GetValue(STR_XML_NAME);
      mdef.custom_type = node.GetValue(STR_XML_CUSTOMTYPE);
      mdef.props       = parse_ulong(node.GetValue(STR_XML_PROPS));
      mdef.flag        = parse_ulong(node.GetValue(STR_XML_FLAG));
      mdef.array_size  = parse_ulong(node.GetValue(STR_XML_ARSIZE));
      mdef.size        = parse_ulong(node.GetValue(STR_XML_SIZE));
      mdef.data_type   = struct_util_consts_str_to_num((char *)node.GetValue(STR_XML_TYPE).operator LPCSTR());
      mdef.is_pointer  = parse_ulong(node.GetValue(STR_XML_ISPTR)) != 0;

      CXmlNodeWrapper comment;

      comment = node.GetNode(STR_XML_COMMENT);
      if (comment.IsValid())
        mdef.non_repeat_comment = comment.GetText();

      comment = node.GetNode(STR_XML_RCOMMENT);
      if (comment.IsValid())
        mdef.repeat_comment = comment.GetText();

      def.members.push_back(mdef);
    }
    def_list.push_back(def);
  }
}

void SerializeStructs(CXmlDocumentWrapper &doc, CXmlNodeWrapper &root, data_type_def_list_t &def)
{
  CXmlNodeWrapper nodeStruct, nodeMembers, nodeMember, nodeComment;
  MSXML2::IXMLDOMNodePtr raw_node;

  long nStructs = 0;

  for (data_type_def_list_t::iterator it_def=def.begin();it_def!=def.end();++it_def)
  {
    data_type_def &def = *it_def;

    nodeStruct = root.InsertNode(nStructs++, STR_XML_STRUCT);

    // Attributes
    nodeStruct.SetValue(STR_XML_DEPTH, def.depth);
    nodeStruct.SetValue(STR_XML_PROPS, (int)def.props);
    nodeStruct.SetValue(STR_XML_NAME, def.name.c_str());
    nodeStruct.SetValue(STR_XML_TYPE, num_to_struct_util_consts_str(def.type));

    // Repeatable comment
    nodeComment = nodeStruct.InsertNode(0, STR_XML_RCOMMENT);
    raw_node = doc.getDoc()->createCDATASection(_bstr_t(def.repeat_comment.c_str()));
    nodeComment.InsertNode(0, raw_node);

    // Comment
    nodeComment = nodeStruct.InsertNode(1, STR_XML_COMMENT);
    raw_node = doc.getDoc()->createCDATASection(_bstr_t(def.non_repeat_comment.c_str()));
    nodeComment.InsertNode(0, raw_node);

    // Members
    nodeMembers = nodeStruct.InsertNode(2, STR_XML_MEMBERS);

    long nMembers = 0;

    for (member_def_list_t::iterator it_mem=def.members.begin();it_mem != def.members.end();++it_mem)
    {
      data_member_def_t &mdef = *it_mem;

      nodeMember = nodeMembers.InsertNode(nMembers++, STR_XML_MEMBER);

      nodeMember.SetValue(STR_XML_NAME, mdef.name.c_str());
      nodeMember.SetValue(STR_XML_CUSTOMTYPE, mdef.custom_type.c_str());
      nodeMember.SetValue(STR_XML_PROPS, (int)mdef.props);
      nodeMember.SetValue(STR_XML_FLAG, (int)mdef.flag);
      nodeMember.SetValue(STR_XML_ARSIZE, mdef.array_size);
      nodeMember.SetValue(STR_XML_SIZE, mdef.size);
      nodeMember.SetValue(STR_XML_TYPE, num_to_struct_util_consts_str(mdef.data_type));
      nodeMember.SetValue(STR_XML_ISPTR, (int)mdef.is_pointer);

      // Repeatable comment
      nodeComment = nodeMember.InsertNode(0, STR_XML_RCOMMENT);
      raw_node = doc.getDoc()->createCDATASection(_bstr_t(mdef.repeat_comment.c_str()));
      nodeComment.InsertNode(0, raw_node);

      // Comment
      nodeComment = nodeMember.InsertNode(0, STR_XML_COMMENT);
      raw_node = doc.getDoc()->createCDATASection(_bstr_t(mdef.non_repeat_comment.c_str()));
      nodeComment.InsertNode(0, raw_node);
    }
  }
}

void SerializeEnums(CXmlDocumentWrapper &doc, CXmlNodeWrapper &root, enum_type_def_list_t &def_list)
{
  CXmlNodeWrapper nodeEnum, nodeMembers, nodeMember, nodeComment;
  MSXML2::IXMLDOMNodePtr raw_node;
  char buf[MAXSTR];

  long nEnums = 0;

  for (enum_type_def_list_t::iterator it_def=def_list.begin();it_def!=def_list.end();++it_def)
  {
    enum_type_def_t &def = *it_def;
    nodeEnum = root.InsertNode(nEnums++, STR_XML_ENUM);
    
    // Attributes
    nodeEnum.SetValue(STR_XML_FLAG, int(def.flags));
    nodeEnum.SetValue(STR_XML_NAME, def.name.c_str());
    nodeEnum.SetValue(STR_XML_ISBF, int(def.is_bitfield ? 1 : 0));

    // Repeatable comment
    nodeComment = nodeEnum.InsertNode(0, STR_XML_RCOMMENT);
    raw_node = doc.getDoc()->createCDATASection(_bstr_t(def.repeatable_comment.c_str()));
    nodeComment.InsertNode(0, raw_node);

    // Comment
    nodeComment = nodeEnum.InsertNode(1, STR_XML_COMMENT);
    raw_node = doc.getDoc()->createCDATASection(_bstr_t(def.non_repeatable_comment.c_str()));
    nodeComment.InsertNode(0, raw_node);

    // Members
    nodeMembers = nodeEnum.InsertNode(2, STR_XML_MEMBERS);

    long nMembers = 0;

    for (enum_member_def_list_t::iterator it_mem=def.members.begin();it_mem != def.members.end();++it_mem)
    {
      enum_member_def_t &mdef = *it_mem;

      nodeMember = nodeMembers.InsertNode(nMembers++, STR_XML_MEMBER);
      nodeMember.SetValue(STR_XML_NAME, mdef.name.c_str());

      nodeMember.SetValue(STR_XML_SERIAL, mdef.serial);

      qsnprintf(buf, MAXSTR, "0x%a", mdef.mask);
      nodeMember.SetValue(STR_XML_MASK, buf);

      qsnprintf(buf, MAXSTR, "0x%a", mdef.val);
      nodeMember.SetValue(STR_XML_VALUE, buf);

      // Repeatable comment
      nodeComment = nodeMember.InsertNode(0, STR_XML_RCOMMENT);
      raw_node = doc.getDoc()->createCDATASection(_bstr_t(mdef.repeatable_comment.c_str()));
      nodeComment.InsertNode(0, raw_node);

      // Comment
      nodeComment = nodeMember.InsertNode(0, STR_XML_COMMENT);
      raw_node = doc.getDoc()->createCDATASection(_bstr_t(mdef.non_repeatable_comment.c_str()));
      nodeComment.InsertNode(0, raw_node);
    }
  }
}

void DeSerializeEnums(CXmlNodeWrapper &root, enum_type_def_list_t &def_list)
{
  enum_type_def_t def;
  enum_member_def_t mdef;

  CXmlNodeWrapper node;

  long nEnums = root.NumNodes();

  for (long iEnums=0;iEnums<nEnums;iEnums++)
  {
    node = root.GetNode(iEnums);
    if (node.Name() != STR_XML_ENUM)
      continue;

    def.clear();

    def.name  = node.GetValue(STR_XML_NAME);
    def.flags  = parse_ulong(node.GetValue(STR_XML_FLAG));
    def.is_bitfield = parse_ulong(node.GetValue(STR_XML_ISBF)) == 0 ? false : true;

    CXmlNodeWrapper comment;

    comment = node.GetNode(STR_XML_COMMENT);
    if (comment.IsValid())
      def.non_repeatable_comment = comment.GetText();

    comment = node.GetNode(STR_XML_RCOMMENT);
    if (comment.IsValid())
      def.repeatable_comment = comment.GetText();

    CXmlNodeWrapper members;
    members = node.GetNode(STR_XML_MEMBERS);

    long nMembers = members.NumNodes();
    for (int iMembers=0;iMembers<nMembers;iMembers++)
    {
      node = members.GetNode(iMembers);
      if (!node.IsValid() || node.Name() != STR_XML_MEMBER)
        continue;

      mdef.clear();
      mdef.name        = node.GetValue(STR_XML_NAME);
      mdef.serial      = int(parse_ulong(node.GetValue(STR_XML_SERIAL)));
      mdef.mask        = parse_ulong(node.GetValue(STR_XML_MASK));
      mdef.val         = parse_ulong(node.GetValue(STR_XML_VALUE));

      CXmlNodeWrapper comment;

      comment = node.GetNode(STR_XML_COMMENT);
      if (comment.IsValid())
        mdef.non_repeatable_comment = comment.GetText();

      comment = node.GetNode(STR_XML_RCOMMENT);
      if (comment.IsValid())
        mdef.repeatable_comment = comment.GetText();

      def.members.push_back(mdef);
    }
    def_list.push_back(def);
  }
}

bool DeSerialize(SerializePack_t &sp, LPCSTR szSrc, int nSrcType)
{
  data_type_def_list_t &def_list = *sp.st;
  enum_type_def_list_t &edef_list = *sp.en;

  CXmlDocumentWrapper doc;

  CXmlNodeWrapper root, node;

  if (nSrcType == SERIALIZE_SRC_URL)
  {
    HRESULT hr;
    bool bOk = false;
    MSXML2::IXMLDOMDocument2 * pXMLDoc2 = NULL;

    try
    {
      do 
      {
        hr = CoCreateInstance(MSXML2::CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, MSXML2::IID_IXMLDOMDocument2, (void**)&pXMLDoc2);
        if (!SUCCEEDED(hr))
          return false;

        // We want asynchronous request
        if (!SUCCEEDED(pXMLDoc2->put_async(FALSE)))
          break;

        // We need WinInet not WinHTTP?
        if (!SUCCEEDED(pXMLDoc2->setProperty(_T("ServerHTTPRequest"), _variant_t(false))))
          break;

        // Load XML remotely
        hr = pXMLDoc2->load(szSrc);

        MSXML2::IXMLDOMDocument *pXMLDoc1 = doc.getDoc();

        hr = pXMLDoc1->loadXML(pXMLDoc2->xml);

        // release DOC2 interface
        pXMLDoc2->Release();
        pXMLDoc2 = 0;

        bOk = true;
      } while (0);
    }
    catch (...)
    {
    }

    if (pXMLDoc2)
      pXMLDoc2->Release();

    if (!bOk)
      return false;
  }
  else if (nSrcType == SERIALIZE_SRC_STR)
  {
    if (!doc.LoadXML(szSrc))
      return false;
  }
  else
  {
    if (!doc.Load(szSrc))
      return false;
  }

  root = doc.AsNode();
  if (!root.IsValid())
    return false;

  DeSerializeStructs(root, def_list);
  DeSerializeEnums(root, edef_list);

  return true;
}

bool Serialize(SerializePack_t &sp, std::string &out)
{
  data_type_def_list_t &def = *sp.st;
  enum_type_def_list_t &edef = *sp.en;
                                
  CXmlDocumentWrapper doc;

  CXmlNodeWrapper root;

  // Create the root node
  {
    CString s;
    s.Format("<%s />", STR_XML_ROOT);
    doc.LoadXML(s);
    root = doc.AsNode();
  }

  doc.getDoc()->preserveWhiteSpace = TRUE;

  SerializeStructs(doc, root, def);
  SerializeEnums(doc, root, edef);

  out = root.GetXML();

  //doc.Save("c:\\x.xml");
  return true;
}
