/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "tm2c_struct.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>


static void _addFlagspecificElements(TYPEMAKER2_TYPE *ty, uint32_t flags, GWEN_BUFFER *tbuf);
static int _addMembers(TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf);
static int _addMember(TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf);
static int _addVirtualFunctions(TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf);




int TM2C_BuildStruct(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  uint32_t flags;
  const char *sTypeId;
  int rv;
  int pack;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  sTypeId=Typemaker2_Type_GetIdentifier(ty);

  pack=Typemaker2_Type_GetPack(ty);

  if (pack)
    GWEN_Buffer_AppendArgs(tbuf, "#pragma pack(push, %d)\n", pack);

  GWEN_Buffer_AppendArgs(tbuf, "struct %s {\n", sTypeId);

  _addFlagspecificElements(ty, flags, tbuf);

  if ((flags & TYPEMAKER2_TYPEFLAGS_WITH_SIGNALS) || (flags & TYPEMAKER2_TYPEFLAGS_WITH_SLOTS))
    GWEN_Buffer_AppendString(tbuf, "  GWEN_SIGNALOBJECT *_signalObject;\n");

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_REFCOUNT)
    GWEN_Buffer_AppendString(tbuf, "  int _refCount;\n");

  rv=_addMembers(ty, tbuf);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  rv=_addVirtualFunctions(ty, tbuf);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }


  GWEN_Buffer_AppendString(tbuf, "};\n");

  if (pack)
    GWEN_Buffer_AppendArgs(tbuf, "#pragma pack(pop)\n");

  /* add to declaration according to structAccess (defaults to "private") */
  switch(Typemaker2_Type_GetStructAccess(ty)) {
  case TypeMaker2_Access_Public:
    Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
    break;
  case TypeMaker2_Access_Library:
    Typemaker2_Builder_AddLibraryDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
    break;
  case TypeMaker2_Access_Protected:
    Typemaker2_Builder_AddProtectedDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
    break;
  case TypeMaker2_Access_Private:
    Typemaker2_Builder_AddPrivateDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
    break;
  }
  GWEN_Buffer_free(tbuf);

  return 0;
}



void _addFlagspecificElements(TYPEMAKER2_TYPE *ty, uint32_t flags, GWEN_BUFFER *tbuf)
{
  const char *sTypeId;

  sTypeId=Typemaker2_Type_GetIdentifier(ty);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_INHERIT)
    GWEN_Buffer_AppendArgs(tbuf, "  GWEN_INHERIT_ELEMENT(%s)\n", sTypeId);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_LIST1)
    GWEN_Buffer_AppendArgs(tbuf, "  GWEN_LIST_ELEMENT(%s)\n", sTypeId);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE)
    GWEN_Buffer_AppendArgs(tbuf, "  GWEN_TREE_ELEMENT(%s)\n", sTypeId);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE2)
    GWEN_Buffer_AppendArgs(tbuf, "  GWEN_TREE2_ELEMENT(%s)\n", sTypeId);
}



int _addMembers(TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_MEMBER_LIST *tml;

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while (tm) {
      int rv;

      rv=_addMember(tm, tbuf);
      if (rv<0) {
        DBG_INFO(NULL, "here (%d)", rv);
        return rv;
      }

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  return 0;
}



int _addMember(TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_TYPE *mty;
  int typeIsWritten=0;
  const char *s;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  GWEN_Buffer_AppendString(tbuf, "  ");

  if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_ENUM) {
    TYPEMAKER2_ENUM *te=Typemaker2_Member_GetEnumPtr(tm);
    const char *s;

    if (te==NULL) {
      DBG_ERROR(0, "No enum name set in definition of member \"%s\"", Typemaker2_Member_GetName(tm));
      return GWEN_ERROR_BAD_DATA;
    }
    s=Typemaker2_Enum_GetType(te);
    if (s && *s) {
      GWEN_Buffer_AppendArgs(tbuf, "%s ", s);
      typeIsWritten=1;
    }
  }

  if (!typeIsWritten) {
    if ((Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer) &&
        (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_CONST))
      GWEN_Buffer_AppendString(tbuf, "const ");

    s=Typemaker2_Type_GetIdentifier(mty);
    if (!(s && *s)) {
      const char *xx;

      xx=Typemaker2_Member_GetName(tm);
      DBG_ERROR(0, "Member [%s] has no identifier", xx?xx:"(unnamed)");
      return GWEN_ERROR_BAD_DATA;
    }
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");

    if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer)
      GWEN_Buffer_AppendString(tbuf, "*");
  }

  s=Typemaker2_Member_GetName(tm);
  GWEN_Buffer_AppendString(tbuf, s);
  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array)
    GWEN_Buffer_AppendArgs(tbuf, "[%d]", Typemaker2_Member_GetMaxLen(tm));

  GWEN_Buffer_AppendString(tbuf, ";");
  if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE)
    GWEN_Buffer_AppendString(tbuf, " /* volatile */");
  GWEN_Buffer_AppendString(tbuf, "\n");

  return 0;
}



int _addVirtualFunctions(TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_VIRTUALFN_LIST *fns;

  /* add virtual functions */
  fns=Typemaker2_Type_GetVirtualFns(ty);
  assert(fns);
  if (Typemaker2_VirtualFn_List_GetCount(fns)) {
    const char *s;
    TYPEMAKER2_VIRTUALFN *vf;

    vf=Typemaker2_VirtualFn_List_First(fns);
    while (vf) {
      GWEN_Buffer_AppendString(tbuf, "  ");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        DBG_ERROR(0, "Virtual function has no name");
        return GWEN_ERROR_BAD_DATA;
      }
      while (*s)
        GWEN_Buffer_AppendByte(tbuf, toupper(*(s++)));
      GWEN_Buffer_AppendString(tbuf, "_FN ");

      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        DBG_ERROR(0, "Virtual function has no name");
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendByte(tbuf, tolower(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "Fn;\n");

      vf=Typemaker2_VirtualFn_List_Next(vf);
    }
  }

  return 0;
}





