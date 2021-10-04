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


#include "tm2c_writedb.h"
#include "tm2c_misc.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



static void _addPrototype(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
static int _addImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
static int _addWriteMemberCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf);





int TM2C_BuildWriteDb(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  int rv;

  _addPrototype(tb, ty);

  rv=_addImplementation(tb, ty);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  return 0;
}



void _addPrototype(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s)
    GWEN_Buffer_AppendArgs(tbuf, "%s ", s);
  GWEN_Buffer_AppendArgs(tbuf,
                         "int %s_WriteDb(const %s *p_struct, GWEN_DB_NODE *p_db);\n",
                         Typemaker2_Type_GetPrefix(ty),
                         Typemaker2_Type_GetIdentifier(ty));
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



int _addImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_MEMBER_LIST *tml;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendArgs(tbuf,
                         "int %s_WriteDb(const %s *p_struct, GWEN_DB_NODE *p_db) {\n",
                         Typemaker2_Type_GetPrefix(ty),
                         Typemaker2_Type_GetIdentifier(ty));


  if (Typemaker2_Type_GetNonVolatileMemberCount(ty)) {
    GWEN_Buffer_AppendString(tbuf, "  int p_rv;\n");
    GWEN_Buffer_AppendString(tbuf, "\n");
  }
  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while (tm) {
      int rv;

      rv=_addWriteMemberCode(tb, ty, tm, tbuf);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
      GWEN_Buffer_AppendString(tbuf, "\n");

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendString(tbuf, "  return 0;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



int _addWriteMemberCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_TYPE *mty;
  const char *sMemberName;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);
  sMemberName=Typemaker2_Member_GetName(tm);

  if (!(Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE)) {
    GWEN_BUFFER *srcbuf;
    int rv;

    GWEN_Buffer_AppendArgs(tbuf, "  /* member \"%s\" */\n", sMemberName);

    srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendArgs(srcbuf, "p_struct->%s", sMemberName);
    GWEN_Buffer_AppendString(tbuf, "  ");
    rv=Typemaker2_Builder_Invoke_ToDbFn(tb, ty, tm, GWEN_Buffer_GetStart(srcbuf), NULL, tbuf);
    GWEN_Buffer_free(srcbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    GWEN_Buffer_AppendString(tbuf, "\n");

    GWEN_Buffer_AppendString(tbuf, "  if (p_rv<0) {\n");
    GWEN_Buffer_AppendString(tbuf, "    DBG_INFO(GWEN_LOGDOMAIN, \"here (%d)\\n\", p_rv);\n");
    GWEN_Buffer_AppendString(tbuf, "    return p_rv;\n");
    GWEN_Buffer_AppendString(tbuf, "  }\n");
  }
  else
    GWEN_Buffer_AppendArgs(tbuf, "  /* member \"%s\" is volatile, not writing to db */\n", sMemberName);
  GWEN_Buffer_AppendString(tbuf, "\n");

  return 0;
}







