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


#include "tm2c_readxml.h"
#include "tm2c_misc.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



static void _addPrototype(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
static int _addImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
static int _addReadMemberCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf);




int TM2C_BuildReadXml(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
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
                         "void %s_ReadXml(%s *p_struct, GWEN_XMLNODE *p_db);\n",
                         Typemaker2_Type_GetPrefix(ty),
                         Typemaker2_Type_GetIdentifier(ty));
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



int _addImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_MEMBER_LIST *tml;
  int rv;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendArgs(tbuf,
                         "void %s_ReadXml(%s *p_struct, GWEN_XMLNODE *p_db){\n",
                         Typemaker2_Type_GetPrefix(ty),
                         Typemaker2_Type_GetIdentifier(ty));
  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while (tm) {

      rv=_addReadMemberCode(tb, ty, tm, tbuf);
      if (rv<0) {
        DBG_INFO(NULL, "here (%d)", rv);
        GWEN_Buffer_free(tbuf);
        return rv;
      }

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}




int _addReadMemberCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_TYPE *mty;
  int rv;
  const char *sMemberName;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);
  sMemberName=Typemaker2_Member_GetName(tm);

  GWEN_Buffer_AppendArgs(tbuf, "  /* member \"%s\" */\n", sMemberName);
  if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_OWN) {
    rv=TM2C_AddReleaseOldValueCode(tb, ty, tm, tbuf);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
  }

  /* read from object */
  if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE) {
    GWEN_BUFFER *dstbuf;

    GWEN_Buffer_AppendArgs(tbuf, "  /* member \"%s\" is volatile, just presetting it */\n", sMemberName);
    dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendArgs(dstbuf, "p_struct->%s", sMemberName);
    GWEN_Buffer_AppendString(tbuf, "  ");
    rv=Typemaker2_Builder_Invoke_ConstructFn(tb, ty, tm, NULL, GWEN_Buffer_GetStart(dstbuf), tbuf);
    GWEN_Buffer_free(dstbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    GWEN_Buffer_AppendString(tbuf, "\n");
  }
  else {
    GWEN_BUFFER *dstbuf;

    /* not volatile */
    dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendArgs(dstbuf, "p_struct->%s", sMemberName);
    GWEN_Buffer_AppendString(tbuf, "  ");
    rv=Typemaker2_Builder_Invoke_FromXmlFn(tb, ty, tm, NULL, GWEN_Buffer_GetStart(dstbuf), tbuf);
    GWEN_Buffer_free(dstbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    GWEN_Buffer_AppendString(tbuf, "\n");
  }

  return 0;
}



int TM2C_BuildFromXml(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  /* uint32_t flags; */
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* flags=Typemaker2_Type_GetFlags(ty); */

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_fromXml(GWEN_XMLNODE *p_db);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_fromXml(GWEN_XMLNODE *p_db) {\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct;\n");

  GWEN_Buffer_AppendString(tbuf, "  p_struct=");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  if (Typemaker2_Type_GetUsePrivateConstructor(ty)) {
    GWEN_Buffer_AppendByte(tbuf, '_');
  }
  GWEN_Buffer_AppendString(tbuf, "_new();\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_ReadXml(p_struct, p_db);\n");

  GWEN_Buffer_AppendString(tbuf, "  return p_struct;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}

