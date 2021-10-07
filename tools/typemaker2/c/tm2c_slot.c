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


#include "tm2c_slot.h"
#include "tm2c_misc.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



int TM2C_BuildSlotFunctions(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_SLOT_LIST *slotList;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  slotList=Typemaker2_Type_GetSlots(ty);
  if (slotList) {
    TYPEMAKER2_SLOT *slot;

    /* private prototypes */
    GWEN_Buffer_AppendString(tbuf, "/* slot functions */\n");
    slot=Typemaker2_Slot_List_First(slotList);
    while (slot) {

      /* callback function */
      GWEN_Buffer_AppendString(tbuf, "static int GWENHYWFAR_CB ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_SlotCallback_");
      s=Typemaker2_Slot_GetName(slot);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "(GWEN_SLOT *slot, void *userData, void *p1, void *p2, int p3, int p4);\n");

      slot=Typemaker2_Slot_List_Next(slot);
    }

    Typemaker2_Builder_AddPrivateDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);

    /* public prototypes */
    GWEN_Buffer_AppendString(tbuf, "/* slot functions */\n");
    slot=Typemaker2_Slot_List_First(slotList);
    while (slot) {
      /* GetSlot */
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      if (s) {
        GWEN_Buffer_AppendString(tbuf, s);
        GWEN_Buffer_AppendString(tbuf, " ");
      }
      GWEN_Buffer_AppendString(tbuf, "GWEN_SLOT *");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_GetSlot_");
      s=Typemaker2_Slot_GetName(slot);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "(const ");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " *p_struct);\n");


      slot=Typemaker2_Slot_List_Next(slot);
    }
    Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);

    /* implementation */
    GWEN_Buffer_AppendString(tbuf, "/* slot functions */\n");
    slot=Typemaker2_Slot_List_First(slotList);
    while (slot) {

      /* callback function */
      GWEN_Buffer_AppendString(tbuf, "static int GWENHYWFAR_CB ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_SlotCallback_");
      s=Typemaker2_Slot_GetName(slot);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "(GWEN_SLOT *slot, void *userData, void *p1, void *p2, int p3, int p4) {\n");

      GWEN_Buffer_AppendString(tbuf, "  ");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " *p_struct;\n");

      GWEN_Buffer_AppendString(tbuf, "\n");
      GWEN_Buffer_AppendString(tbuf, "  p_struct=(");
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "*) userData;\n");

      GWEN_Buffer_AppendString(tbuf, "  return ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_");
      s=Typemaker2_Slot_GetName(slot);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "(p_struct");

      s=Typemaker2_Slot_GetParamType1(slot); /* param 1 */
      if (s && *s && strcasecmp(s, "none")) {
        GWEN_Buffer_AppendString(tbuf, ", (");
        GWEN_Buffer_AppendString(tbuf, s);
        GWEN_Buffer_AppendString(tbuf, ") p1");
      }
      s=Typemaker2_Slot_GetParamType2(slot); /* param 2 */
      if (s && *s && strcasecmp(s, "none")) {
        GWEN_Buffer_AppendString(tbuf, ", (");
        GWEN_Buffer_AppendString(tbuf, s);
        GWEN_Buffer_AppendString(tbuf, ") p2");
      }
      if (Typemaker2_Slot_GetFlags(slot) & TYPEMAKER2_SLOT_FLAGS_USE_PARAM3)
        GWEN_Buffer_AppendString(tbuf, ", p3");
      if (Typemaker2_Slot_GetFlags(slot) & TYPEMAKER2_SLOT_FLAGS_USE_PARAM4)
        GWEN_Buffer_AppendString(tbuf, ", p4");

      GWEN_Buffer_AppendString(tbuf, ");\n");

      GWEN_Buffer_AppendString(tbuf, "}\n\n");


      /* GetSlot */
      GWEN_Buffer_AppendString(tbuf, "GWEN_SLOT *");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_GetSlot_");
      s=Typemaker2_Slot_GetName(slot);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "(const ");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " *p_struct) {\n");

      GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
      GWEN_Buffer_AppendString(tbuf, "  return GWEN_SignalObject_FindSlot(p_struct->_signalObject, ");
      s=Typemaker2_Slot_GetName(slot);       /* name */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "\", ");
      s=Typemaker2_Slot_GetParamType1(slot); /* param 1 */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s?s:"none");
      GWEN_Buffer_AppendString(tbuf, "\", ");
      s=Typemaker2_Slot_GetParamType2(slot); /* param 2 */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s?s:"none");
      GWEN_Buffer_AppendString(tbuf, "\");\n");

      GWEN_Buffer_AppendString(tbuf, "}\n");
      GWEN_Buffer_AppendString(tbuf, "\n");

      slot=Typemaker2_Slot_List_Next(slot);
    }
    Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);

  }
  GWEN_Buffer_free(tbuf);

  return 0;
}



int TM2C_AddVirtualFnsFromSlots(TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_VIRTUALFN_LIST *fnList;
  TYPEMAKER2_SLOT_LIST *slotList;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  fnList=Typemaker2_Type_GetVirtualFns(ty);

  slotList=Typemaker2_Type_GetSlots(ty);
  if (slotList) {
    TYPEMAKER2_SLOT *slot;

    slot=Typemaker2_Slot_List_First(slotList);
    while (slot) {
      TYPEMAKER2_VIRTUALFN *fn;
      TYPEMAKER2_VIRTUALFN_PARAM_LIST *plist;

      /* callback function */
      s=Typemaker2_Slot_GetName(slot);
      GWEN_Buffer_AppendByte(tbuf, tolower(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);

      fn=Typemaker2_VirtualFn_new();
      plist=Typemaker2_VirtualFn_GetParamTypeList(fn);

      /* name */
      Typemaker2_VirtualFn_SetName(fn, GWEN_Buffer_GetStart(tbuf));

      s=Typemaker2_Slot_GetParamType1(slot); /* param 1 */
      if (s && *s && strcasecmp(s, "none")) {
        TYPEMAKER2_VIRTUALFN_PARAM *prm;

        prm=Typemaker2_VirtualFn_Param_new();
        Typemaker2_VirtualFn_Param_SetType(prm, s);
        Typemaker2_VirtualFn_Param_List_Add(prm, plist);
      }
      s=Typemaker2_Slot_GetParamType2(slot); /* param 2 */
      if (s && *s && strcasecmp(s, "none")) {
        TYPEMAKER2_VIRTUALFN_PARAM *prm;

        prm=Typemaker2_VirtualFn_Param_new();
        Typemaker2_VirtualFn_Param_SetType(prm, s);
        Typemaker2_VirtualFn_Param_List_Add(prm, plist);
      }
      if (Typemaker2_Slot_GetFlags(slot) & TYPEMAKER2_SLOT_FLAGS_USE_PARAM3) {
        TYPEMAKER2_VIRTUALFN_PARAM *prm;

        prm=Typemaker2_VirtualFn_Param_new();
        Typemaker2_VirtualFn_Param_SetType(prm, "int");
        Typemaker2_VirtualFn_Param_List_Add(prm, plist);
      }
      if (Typemaker2_Slot_GetFlags(slot) & TYPEMAKER2_SLOT_FLAGS_USE_PARAM4) {
        TYPEMAKER2_VIRTUALFN_PARAM *prm;

        prm=Typemaker2_VirtualFn_Param_new();
        Typemaker2_VirtualFn_Param_SetType(prm, "int");
        Typemaker2_VirtualFn_Param_List_Add(prm, plist);
      }

      Typemaker2_VirtualFn_SetAccess(fn, TypeMaker2_Access_Public);
      Typemaker2_VirtualFn_SetLocation(fn, "post");
      Typemaker2_VirtualFn_SetReturnType(fn, "int");
      Typemaker2_VirtualFn_SetDefaultReturnValue(fn, "GWEN_ERROR_NOT_IMPLEMENTED");

      Typemaker2_VirtualFn_AddFlags(fn, TYPEMAKER2_VIRTUALFN_FLAGS_FROMSLOT);

      Typemaker2_VirtualFn_List_Add(fn, fnList);

      slot=Typemaker2_Slot_List_Next(slot);
    }
  }
  GWEN_Buffer_free(tbuf);

  return 0;
}



void TM2C_DelVirtualFnsFromSlots(TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_VIRTUALFN_LIST *fnList;

  fnList=Typemaker2_Type_GetVirtualFns(ty);
  if (fnList) {
    TYPEMAKER2_VIRTUALFN *fn;

    fn=Typemaker2_VirtualFn_List_First(fnList);
    while (fn) {
      TYPEMAKER2_VIRTUALFN *fnNext;

      fnNext=Typemaker2_VirtualFn_List_Next(fn);
      if (Typemaker2_VirtualFn_GetFlags(fn) & TYPEMAKER2_VIRTUALFN_FLAGS_FROMSLOT) {
        Typemaker2_VirtualFn_List_Del(fn);
        Typemaker2_VirtualFn_free(fn);
      }
      fn=fnNext;
    }
  }
}
