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


#include "tm2c_copy.h"
#include "tm2c_misc.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



static void _addPrototype(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
static int _addImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
static int _addCopyMemberCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf);
static int _addAssignValueCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf);
static int _addDupValueCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf);





int TM2C_BuildCopy(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
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
                         "%s *%s_copy(const %s *p_struct);\n",
                         Typemaker2_Type_GetIdentifier(ty),
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
                         "%s *%s_copy(const %s *p_struct) {\n",
                         Typemaker2_Type_GetIdentifier(ty),
                         Typemaker2_Type_GetPrefix(ty),
                         Typemaker2_Type_GetIdentifier(ty));


  GWEN_Buffer_AppendString(tbuf, "  assert(p_src);\n");
  if (Typemaker2_Type_GetUsePrivateConstructor(ty))
    GWEN_Buffer_AppendArgs(tbuf, "  p_struct=%s__new();\n", Typemaker2_Type_GetPrefix(ty));
  else
    GWEN_Buffer_AppendArgs(tbuf, "  p_struct=%s_new();\n", Typemaker2_Type_GetPrefix(ty));


  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while (tm) {
      rv=_addCopyMemberCode(tb, ty, tm, tbuf);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
      GWEN_Buffer_AppendString(tbuf, "\n");
      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendString(tbuf, "  return p_struct;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



int _addCopyMemberCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_TYPE *mty;
  const char *sMemberName;
  int rv;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);
  sMemberName=Typemaker2_Member_GetName(tm);

  if (!(Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_NOCOPY)) {
    GWEN_Buffer_AppendArgs(tbuf, "  /* member \"%s\" */\n", sMemberName);

    /* TODO: only do this if there is a preset value and it is !=NULL for pointers! */
    if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_OWN) {
      rv=TM2C_AddReleaseOldValueCode(tb, ty, tm, tbuf);
      if (rv<0) {
        DBG_INFO(NULL, "here (%d)", rv);
        GWEN_Buffer_free(tbuf);
        return rv;
      }
    }

    if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer) {
      if (Typemaker2_Member_GetCopyFlags(tm) & TYPEMAKER2_FLAGS_ASSIGN) {
        rv=_addAssignValueCode(tb, ty, tm, tbuf);
        if (rv<0) {
          DBG_INFO(NULL, "here (%d)", rv);
          return rv;
        }
      } /* assign */
      else {
        GWEN_Buffer_AppendArgs(tbuf, "  if (p_src->%s) {\n", sMemberName);
        rv=_addDupValueCode(tb, ty, tm, tbuf);
        if (rv<0) {
          DBG_INFO(NULL, "here (%d)", rv);
          return rv;
        }
        GWEN_Buffer_AppendString(tbuf, "  }\n");
      }
    } /* if pointer */
    else {
      rv=_addDupValueCode(tb, ty, tm, tbuf);
      if (rv<0) {
        DBG_INFO(NULL, "here (%d)", rv);
        return rv;
      }
    }
  } /* if !nocopy */
  else {
    GWEN_Buffer_AppendString(tbuf, "  /* member \"");
    GWEN_Buffer_AppendString(tbuf, sMemberName);
    GWEN_Buffer_AppendString(tbuf, "\" has nocopy flag, not copying */\n");
  }

  GWEN_Buffer_AppendString(tbuf, "\n");
  return 0;
}



int _addAssignValueCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  GWEN_BUFFER *dstbuf;
  GWEN_BUFFER *srcbuf;
  int rv;
  const char *sMemberName;

  sMemberName=Typemaker2_Member_GetName(tm);
 
  dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendArgs(dstbuf, "p_struct->%s", sMemberName);

  srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendArgs(srcbuf, "p_src->%s", sMemberName);

  GWEN_Buffer_AppendString(tbuf, "    ");
  rv=Typemaker2_Builder_Invoke_AssignFn(tb, ty, tm,
                                        GWEN_Buffer_GetStart(srcbuf),
                                        GWEN_Buffer_GetStart(dstbuf),
                                        tbuf);
  GWEN_Buffer_free(srcbuf);
  GWEN_Buffer_free(dstbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  GWEN_Buffer_AppendString(tbuf, "\n");

  return 0;
}



int _addDupValueCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  GWEN_BUFFER *dstbuf;
  GWEN_BUFFER *srcbuf;
  int rv;
  const char *sMemberName;

  sMemberName=Typemaker2_Member_GetName(tm);
 
  dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendArgs(dstbuf, "p_struct->%s", sMemberName);

  srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendArgs(srcbuf, "p_src->%s", sMemberName);

  GWEN_Buffer_AppendString(tbuf, "    ");
  rv=Typemaker2_Builder_Invoke_DupFn(tb, ty, tm,
                                     GWEN_Buffer_GetStart(srcbuf),
                                     GWEN_Buffer_GetStart(dstbuf),
                                     tbuf);
  GWEN_Buffer_free(srcbuf);
  GWEN_Buffer_free(dstbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  GWEN_Buffer_AppendString(tbuf, "\n");

  return 0;
}





