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


#include "tm2c_constructor.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



static void _buildPrototypes(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
static int _buildImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
static void _addTypeInits(TYPEMAKER2_TYPE *ty, uint32_t flags, GWEN_BUFFER *tbuf);
static void _addSignalInits(TYPEMAKER2_TYPE *ty, uint32_t flags, GWEN_BUFFER *tbuf);
static void _addSlotInits(TYPEMAKER2_TYPE *ty, uint32_t flags, GWEN_BUFFER *tbuf);
static int _addMemberInits(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf);
static void _addVirtualFns(TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf);




int TM2C_BuildConstructor(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  int rv;

  _buildPrototypes(tb, ty);
  rv=_buildImplementation(tb, ty);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }
  return 0;
}



void _buildPrototypes(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* prototype */
  if (Typemaker2_Type_GetUsePrivateConstructor(ty)) {
    GWEN_Buffer_AppendString(tbuf, "/** Private constructor. */\n");
  }
  else {
    GWEN_Buffer_AppendString(tbuf, "/** Constructor. */\n");
    s=Typemaker2_TypeManager_GetApiDeclaration(tym);
    if (s) {
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " ");
    }
  }

  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  /* created structs are always pointers */
  GWEN_Buffer_AppendString(tbuf, " *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  if (Typemaker2_Type_GetUsePrivateConstructor(ty))
    GWEN_Buffer_AppendString(tbuf, "_");
  GWEN_Buffer_AppendString(tbuf, "_new(void)");
  GWEN_Buffer_AppendString(tbuf, ";\n");
  if (Typemaker2_Type_GetUsePrivateConstructor(ty))
    Typemaker2_Builder_AddPrivateDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  else
    Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



int _buildImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  int rv;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  if (Typemaker2_Type_GetUsePrivateConstructor(ty))
    GWEN_Buffer_AppendString(tbuf, "_");
  GWEN_Buffer_AppendString(tbuf, "_new(void)");
  GWEN_Buffer_AppendString(tbuf, " {\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  _addTypeInits(ty, flags, tbuf);
  _addSignalInits(ty, flags, tbuf);
  _addSlotInits(ty, flags, tbuf);

  rv=_addMemberInits(tb, ty, tbuf);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  _addVirtualFns(ty, tbuf);

  GWEN_Buffer_AppendString(tbuf, "\n");
  GWEN_Buffer_AppendString(tbuf, "  return p_struct;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



void _addTypeInits(TYPEMAKER2_TYPE *ty, uint32_t flags, GWEN_BUFFER *tbuf)
{
  const char *sTypeId;
  const char *sTypePrefix;

  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);

  GWEN_Buffer_AppendArgs(tbuf, "  GWEN_NEW_OBJECT(%s, p_struct)\n", sTypeId);
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_REFCOUNT)
    GWEN_Buffer_AppendString(tbuf, "  p_struct->_refCount=1;\n");
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_INHERIT)
    GWEN_Buffer_AppendArgs(tbuf, "  GWEN_INHERIT_INIT(%s, p_struct)\n", sTypeId);
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_LIST1)
    GWEN_Buffer_AppendArgs(tbuf, "  GWEN_LIST_INIT(%s, p_struct)\n", sTypeId);
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE)
    GWEN_Buffer_AppendArgs(tbuf, "  GWEN_TREE_INIT(%s, p_struct)\n", sTypeId);
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE2)
    GWEN_Buffer_AppendArgs(tbuf, "  GWEN_TREE2_INIT(%s, p_struct, %s)\n", sTypeId, sTypePrefix);
}



void _addSignalInits(TYPEMAKER2_TYPE *ty, uint32_t flags, GWEN_BUFFER *tbuf)
{
  if ((flags & TYPEMAKER2_TYPEFLAGS_WITH_SIGNALS) || (flags & TYPEMAKER2_TYPEFLAGS_WITH_SLOTS))
    GWEN_Buffer_AppendString(tbuf, "  p_struct->_signalObject=GWEN_SignalObject_new();\n");

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_SIGNALS) {
    TYPEMAKER2_SIGNAL_LIST *slist;

    GWEN_Buffer_AppendString(tbuf, "  /* generate pre-defined signals */\n");
    slist=Typemaker2_Type_GetSignals(ty);
    if (slist) {
      TYPEMAKER2_SIGNAL *sig;

      sig=Typemaker2_Signal_List_First(slist);
      while (sig) {
	const char *s;

	s=Typemaker2_Signal_GetName(sig);
	GWEN_Buffer_AppendArgs(tbuf, "  /* signal \"%s \" */\n", s);
        GWEN_Buffer_AppendString(tbuf, "  GWEN_Signal_new(p_struct->_signalObject, NULL, ");
	/* name */
	GWEN_Buffer_AppendArgs(tbuf, "\"%s\", ", s);

        /* param type 1 */
        s=Typemaker2_Signal_GetParamType1(sig);
	GWEN_Buffer_AppendArgs(tbuf, "\"%s\", ", s?s:"none");

        /* param type 2 */
        s=Typemaker2_Signal_GetParamType2(sig);
	GWEN_Buffer_AppendArgs(tbuf, "\"%s\");\n", s?s:"none");

        sig=Typemaker2_Signal_List_Next(sig);
      }
    }
  }
}



void _addSlotInits(TYPEMAKER2_TYPE *ty, uint32_t flags, GWEN_BUFFER *tbuf)
{
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_SLOTS) {
    TYPEMAKER2_SLOT_LIST *slist;

    GWEN_Buffer_AppendString(tbuf, "  /* generate pre-defined slots */\n");
    slist=Typemaker2_Type_GetSlots(ty);
    if (slist) {
      TYPEMAKER2_SLOT *slot;

      slot=Typemaker2_Slot_List_First(slist);
      while (slot) {
	const char *s;

	s=Typemaker2_Slot_GetName(slot);
	GWEN_Buffer_AppendArgs(tbuf, "  /* slot \"%s\" */\n", s);
	GWEN_Buffer_AppendString(tbuf, "  GWEN_Slot_new(p_struct->_signalObject, NULL, ");
        /* name */
	GWEN_Buffer_AppendArgs(tbuf, "\"%s\", ", s);

        /* param type 1 */
        s=Typemaker2_Slot_GetParamType1(slot);
	GWEN_Buffer_AppendArgs(tbuf, "\"%s\", ", s?s:"none");

        /* param type 2 */
        s=Typemaker2_Slot_GetParamType2(slot);
	GWEN_Buffer_AppendArgs(tbuf, "\"%s\", ", s?s:"none");

        /* callback function */
	s=Typemaker2_Type_GetPrefix(ty);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "_SlotCallback_");

	s=Typemaker2_Slot_GetName(slot);
	GWEN_Buffer_AppendArgs(tbuf, "%c%s", toupper(*s), (s+1));
	GWEN_Buffer_AppendString(tbuf, ", (void*) p_struct);\n");

        slot=Typemaker2_Slot_List_Next(slot);
      }
    }
  }
}



int _addMemberInits(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_MEMBER_LIST *tml;

  GWEN_Buffer_AppendString(tbuf, "  /* members */\n");

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;
    int rv;

    tm=Typemaker2_Member_List_First(tml);
    while (tm) {
      TYPEMAKER2_TYPE *mty;
      GWEN_BUFFER *dstbuf;
      const char *s;

      dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Buffer_AppendString(dstbuf, "p_struct->");
      s=Typemaker2_Member_GetName(tm);
      GWEN_Buffer_AppendString(dstbuf, s);

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      GWEN_Buffer_AppendString(tbuf, "  ");

      rv=Typemaker2_Builder_Invoke_ConstructFn(tb, ty, tm,
                                               NULL, /* no source */
                                               GWEN_Buffer_GetStart(dstbuf),
                                               tbuf);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Buffer_free(dstbuf);
        return rv;
      }
      GWEN_Buffer_free(dstbuf);

      GWEN_Buffer_AppendString(tbuf, "\n");

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  return 0;
}



void _addVirtualFns(TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_VIRTUALFN_LIST *tvfl;

  tvfl=Typemaker2_Type_GetVirtualFns(ty);
  if (tvfl) {
    TYPEMAKER2_VIRTUALFN *tvf;

    GWEN_Buffer_AppendString(tbuf, "  /* virtual functions */\n");
    tvf=Typemaker2_VirtualFn_List_First(tvfl);
    while (tvf) {
      const char *fname;
      const char *fpreset;

      fname=Typemaker2_VirtualFn_GetName(tvf);
      fpreset=Typemaker2_VirtualFn_GetPreset(tvf);
      if (fname && *fname && fpreset && *fpreset)
	GWEN_Buffer_AppendArgs(tbuf, "  p_struct->%c%sFn=%s;\n", tolower(*fname), fname+1, fpreset);
      tvf=Typemaker2_VirtualFn_List_Next(tvf);
    }
  }
}





