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


#include "tm2c_getter.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



static void _addSignalObjects(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
static int _addMemberProtoTypes(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);
static void _addMemberImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);
static void _addFunctionDeclaration(TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf);





int TM2C_BuildGetter(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;

  flags=Typemaker2_Type_GetFlags(ty);

  /* probably add getter for signalObject */
  if ((flags & TYPEMAKER2_TYPEFLAGS_WITH_SIGNALS) || (flags & TYPEMAKER2_TYPEFLAGS_WITH_SLOTS))
    _addSignalObjects(tb, ty);


  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while (tm) {
      if (!(Typemaker2_Member_GetGetFlags(tm) & TYPEMAKER2_FLAGS_OMIT)) {
        int rv;

        rv=_addMemberProtoTypes(tb, ty, tm);
        if (rv<0) {
          DBG_INFO(NULL, "here (%d)", rv);
          return rv;
        }

        _addMemberImplementation(tb, ty, tm);
      }

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  return 0;
}



void _addSignalObjects(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_TYPEMANAGER *tym;
  const char *sTypeId;
  const char *sTypePrefix;
  const char *sApi;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);
  sApi=Typemaker2_TypeManager_GetApiDeclaration(tym);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* prototype */
  if (sApi && *sApi)
    GWEN_Buffer_AppendArgs(tbuf, "%s ", sApi);

  GWEN_Buffer_AppendArgs(tbuf,
                         "GWEN_SIGNALOBJECT *%s_GetSignalObject(const %s *p_struct);\n",
                         sTypePrefix, sTypeId);

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendArgs(tbuf,
                         "GWEN_SIGNALOBJECT *%s_GetSignalObject(const %s *p_struct) {\n",
                         sTypePrefix, sTypeId);

  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
  GWEN_Buffer_AppendString(tbuf, "  return p_struct->_signalObject;\n");

  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



int _addMemberProtoTypes(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_TYPE *mty;
  GWEN_BUFFER *tbuf;
  const char *sApi;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  sApi=Typemaker2_TypeManager_GetApiDeclaration(tym);

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* apidoc */
  GWEN_Buffer_AppendArgs(tbuf,
                         "/** Getter.\n"
                         " * Use this function to get the member \"%s\" (see @ref %s_%s)\n*/\n",
                         Typemaker2_Member_GetName(tm),
                         Typemaker2_Type_GetIdentifier(ty),
                         Typemaker2_Member_GetName(tm));

  /* prototype */
  if (sApi && Typemaker2_Member_GetAccess(tm)<=TypeMaker2_Access_Protected)
    GWEN_Buffer_AppendArgs(tbuf, "%s ", sApi);

  _addFunctionDeclaration(ty, tm, tbuf);

  GWEN_Buffer_AppendString(tbuf, ";\n");
  
  switch (Typemaker2_Member_GetAccess(tm)) {
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
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid access type");
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_BAD_DATA;
  }
  GWEN_Buffer_free(tbuf);

  return 0;
}



void _addMemberImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  TYPEMAKER2_TYPE *mty;
  GWEN_BUFFER *tbuf;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  _addFunctionDeclaration(ty, tm, tbuf);

  GWEN_Buffer_AppendString(tbuf, "{\n");

  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
  GWEN_Buffer_AppendArgs(tbuf, "  return p_struct->%s;\n", Typemaker2_Member_GetName(tm));

  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));

  GWEN_Buffer_free(tbuf);
}



void _addFunctionDeclaration(TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_TYPE *mty;
  const char *sTypeId;
  const char *sTypePrefix;
  const char *sMemberName;
  const char *s;

  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);
  sMemberName=Typemaker2_Member_GetName(tm);

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);


  /* determine and write return type */
  /* const return value? */
  if ((Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
       Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array) &&
      (Typemaker2_Member_GetGetFlags(tm) & TYPEMAKER2_FLAGS_CONST))
    GWEN_Buffer_AppendString(tbuf, "const ");

  /* enum return value? */
  if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_ENUM) {
    TYPEMAKER2_ENUM *te;
    const char *s;

    te=Typemaker2_Member_GetEnumPtr(tm);
    assert(te);
    s=Typemaker2_Enum_GetType(te);
    if (s && *s)
      GWEN_Buffer_AppendString(tbuf, s);
    else {
      s=Typemaker2_Type_GetIdentifier(mty);
      GWEN_Buffer_AppendString(tbuf, s);
    }
  }
  else {
    s=Typemaker2_Type_GetIdentifier(mty);
    GWEN_Buffer_AppendString(tbuf, s);
  }

  GWEN_Buffer_AppendString(tbuf, " ");

  /* pointer return value? */
  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
      Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array)
    GWEN_Buffer_AppendString(tbuf, "*");

  /* function name */
  GWEN_Buffer_AppendArgs(tbuf, "%s_Get%c%s(", sTypePrefix, toupper(*sMemberName), sMemberName+1);

  if (!(Typemaker2_Member_GetGetFlags(tm) & TYPEMAKER2_FLAGS_NOCONSTOBJECT))
    GWEN_Buffer_AppendString(tbuf, "const ");

  GWEN_Buffer_AppendArgs(tbuf, "%s *p_struct)", sTypeId);
}





