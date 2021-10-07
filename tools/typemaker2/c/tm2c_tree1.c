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


#include "tm2c_tree1.h"
#include "tm2c_misc.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>


static void _addGetByMemberProtoType(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);
static int _addGetByMemberImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);
static void _addGetByMemberDeclaration(TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf);




int TM2C_BuildTreeGetByMember(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  int rv;

  _addGetByMemberProtoType(tb, ty, tm);
  rv=_addGetByMemberImplementation(tb, ty, tm);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  return 0;
}



void _addGetByMemberProtoType(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_TYPE *mty;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s)
    GWEN_Buffer_AppendArgs(tbuf, "%s ", s);
  _addGetByMemberDeclaration(ty, tm, tbuf);
  GWEN_Buffer_AppendString(tbuf, ";\n");

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



int _addGetByMemberImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_TYPE *mty;
  const char *sTypeId;
  const char *sTypePrefix;
  const char *sMemberName;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);
  sMemberName=Typemaker2_Member_GetName(tm);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  _addGetByMemberDeclaration(ty, tm, tbuf);
  GWEN_Buffer_AppendString(tbuf, "{\n");

  GWEN_Buffer_AppendArgs(tbuf,   "  %s *p_struct;\n\n", sTypeId);
  GWEN_Buffer_AppendString(tbuf, "  assert(p_tree);\n");
  GWEN_Buffer_AppendArgs(tbuf,   "  p_struct = %s_Tree_GetFirst(p_tree);\n", sTypePrefix);
  GWEN_Buffer_AppendString(tbuf, "  while(p_struct) {\n");
  GWEN_Buffer_AppendString(tbuf, "    int p_rv;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "    ");
  if (1) {
    GWEN_BUFFER *dstbuf;
    GWEN_BUFFER *srcbuf;
    int rv;

    srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(srcbuf, "p_cmp");

    dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendArgs(dstbuf, "p_struct->%s", sMemberName);

    rv=Typemaker2_Builder_Invoke_CompareFn(tb, ty, tm,
                                           GWEN_Buffer_GetStart(srcbuf),
                                           GWEN_Buffer_GetStart(dstbuf),
                                           tbuf);
    GWEN_Buffer_free(srcbuf);
    GWEN_Buffer_free(dstbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
    GWEN_Buffer_AppendString(tbuf, "\n");
  }


  GWEN_Buffer_AppendString(tbuf, "    if (p_rv == 0)\n");
  GWEN_Buffer_AppendString(tbuf, "      return p_struct;\n");
  GWEN_Buffer_AppendArgs(tbuf,   "    p_struct = %s_Tree_GetBelow(p_struct);\n", sTypePrefix);
  GWEN_Buffer_AppendString(tbuf, "  }\n");

  GWEN_Buffer_AppendString(tbuf, "  return NULL;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



void _addGetByMemberDeclaration(TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_TYPE *mty;
  const char *sTypeId;
  const char *sTypePrefix;
  const char *sMemberName;
  const char *sMemberTypeId;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);
  sMemberName=Typemaker2_Member_GetName(tm);
  sMemberTypeId=Typemaker2_Type_GetIdentifier(mty);

  GWEN_Buffer_AppendArgs(tbuf,
                         "%s *%s_Tree_GetBy%c%s(const %s_TREE *p_tree, ",
                         sTypeId,
                         sTypePrefix,
                         toupper(*sMemberName),
                         sMemberName+1,
                         sTypeId);
  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
      Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array)
    GWEN_Buffer_AppendArgs(tbuf, " const %s *", sMemberTypeId);
  else
    GWEN_Buffer_AppendArgs(tbuf, "%s ", sMemberTypeId);
  GWEN_Buffer_AppendString(tbuf, "p_cmp)");
}



