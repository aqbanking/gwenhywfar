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


#include "tm2c_destructor.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



static void _addPrototypes(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
static void _addFreeHookPrototype(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
static int _addImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
static void _addTypeFinis(TYPEMAKER2_TYPE *ty, uint32_t flags, GWEN_BUFFER *tbuf);
static int _addMemberFinis(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf);





int TM2C_BuildDestructor(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  int rv;

  _addPrototypes(tb, ty);
  _addFreeHookPrototype(tb, ty);

  rv=_addImplementation(tb, ty);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  return 0;
}



void _addPrototypes(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* prototype */
  GWEN_Buffer_AppendString(tbuf, "/** Destructor. */\n");
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_free(");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



void _addFreeHookPrototype(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  const char *s;

  /* insert freeHook prototype, if needed */
  s=Typemaker2_Type_GetFreeHook(ty);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, "static void ");
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, "(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " *p_struct);\n");
    Typemaker2_Builder_AddPrivateDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
}



int _addImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  const char *sTypeId;
  const char *sTypePrefix;
  int rv;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);
  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);

  /* implementation */
  GWEN_Buffer_AppendArgs(tbuf, "void %s_free(%s *p_struct) {\n", sTypePrefix, sTypeId);

  GWEN_Buffer_AppendString(tbuf, "  if (p_struct) {\n");

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_REFCOUNT) {
    GWEN_Buffer_AppendString(tbuf, "  assert(p_struct->_refCount);\n");
    GWEN_Buffer_AppendString(tbuf, "  if (p_struct->_refCount==1) {\n");
  }

  if ((flags & TYPEMAKER2_TYPEFLAGS_WITH_SIGNALS) || (flags & TYPEMAKER2_TYPEFLAGS_WITH_SLOTS))
    GWEN_Buffer_AppendString(tbuf, "    GWEN_SignalObject_free(p_struct->_signalObject);\n");

  /* insert freeHook, if any */
  s=Typemaker2_Type_GetFreeHook(ty);
  if (s && *s)
    GWEN_Buffer_AppendArgs(tbuf, "    %s(p_struct);\n", s);

  _addTypeFinis(ty, flags, tbuf);

  rv=_addMemberFinis(tb, ty, tbuf);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_REFCOUNT) {
    GWEN_Buffer_AppendString(tbuf, "    p_struct->_refCount=0;\n");
  }

  GWEN_Buffer_AppendString(tbuf, "    GWEN_FREE_OBJECT(p_struct);\n");
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_REFCOUNT) {
    GWEN_Buffer_AppendString(tbuf, "  }\n");
    GWEN_Buffer_AppendString(tbuf, "  else\n");
    GWEN_Buffer_AppendString(tbuf, "    p_struct->_refCount--;\n");
  }
  GWEN_Buffer_AppendString(tbuf, "  }\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



void _addTypeFinis(TYPEMAKER2_TYPE *ty, uint32_t flags, GWEN_BUFFER *tbuf)
{
  const char *sTypeId;
  const char *sTypePrefix;

  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_INHERIT)
    GWEN_Buffer_AppendArgs(tbuf, "    GWEN_INHERIT_FINI(%s, p_struct)\n", sTypeId);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_LIST1)
    GWEN_Buffer_AppendArgs(tbuf, "    GWEN_LIST_FINI(%s, p_struct)\n", sTypeId);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE)
    GWEN_Buffer_AppendArgs(tbuf, "    GWEN_TREE_FINI(%s, p_struct)\n", sTypeId);


  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE2)
    GWEN_Buffer_AppendArgs(tbuf, "    GWEN_TREE2_FINI(%s, p_struct, %s)\n", sTypeId, sTypePrefix);
}



int _addMemberFinis(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_MEMBER_LIST *tml;

  GWEN_Buffer_AppendString(tbuf, "  /* members */\n");
  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while (tm) {
      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_OWN) {
        /* destruct, but only if the member is owned */
        GWEN_BUFFER *dstbuf;
        int rv;
        const char *s;

        dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
        GWEN_Buffer_AppendString(dstbuf, "p_struct->");
        s=Typemaker2_Member_GetName(tm);
        GWEN_Buffer_AppendString(dstbuf, s);

        GWEN_Buffer_AppendString(tbuf, "    ");
        rv=Typemaker2_Builder_Invoke_DestructFn(tb, ty, tm,
                                                GWEN_Buffer_GetStart(dstbuf),
                                                NULL, /* no dest */
                                                tbuf);
        if (rv<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          GWEN_Buffer_free(dstbuf);
          return rv;
        }
        GWEN_Buffer_free(dstbuf);
        GWEN_Buffer_AppendString(tbuf, "\n");
      }

      tm=Typemaker2_Member_List_Next(tm);
    }
  }
  return 0;
}



