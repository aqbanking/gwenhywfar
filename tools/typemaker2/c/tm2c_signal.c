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


#include "tm2c_signal.h"
#include "tm2c_misc.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>




int TM2C_BuildSignalFunctions(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_SIGNAL_LIST *sigList;
  const char *sTypeId;
  const char *sTypePrefix;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);


  sigList=Typemaker2_Type_GetSignals(ty);
  if (sigList) {
    TYPEMAKER2_SIGNAL *sig;

    /* prototype */
    GWEN_Buffer_AppendString(tbuf, "/* signal functions */\n");
    sig=Typemaker2_Signal_List_First(sigList);
    while (sig) {
      const char *sSignalName;

      sSignalName=Typemaker2_Signal_GetName(sig);

      /* SignalConnect */
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      if (s && *s)
	GWEN_Buffer_AppendArgs(tbuf, "%s ", s);
      GWEN_Buffer_AppendArgs(tbuf,
			     "int %s_SignalConnect_%c%s(%s *p_struct, GWEN_SLOT *slot);\n",
			     sTypePrefix,
			     toupper(*sSignalName),
			     sSignalName+1,
			     sTypeId);


      /* SignalDisconnect */
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      if (s && *s)
	GWEN_Buffer_AppendArgs(tbuf, "%s ", s);
      GWEN_Buffer_AppendArgs(tbuf,
			     "int %s_SignalDisconnect_%c%s(%s *p_struct, GWEN_SLOT *slot);\n",
			     sTypePrefix,
			     toupper(*sSignalName),
			     sSignalName+1,
			     sTypeId);


      /* GetSignal */
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      if (s && *s)
	GWEN_Buffer_AppendArgs(tbuf, "%s ", s);
      GWEN_Buffer_AppendArgs(tbuf,
			     "GWEN_SIGNAL *%s_GetSignal_%c%s(const %s *p_struct);\n",
			     sTypePrefix,
			     toupper(*sSignalName),
			     sSignalName+1,
			     sTypeId);


      /* EmitSignal */
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      if (s && *s)
	GWEN_Buffer_AppendArgs(tbuf, "%s ", s);
      GWEN_Buffer_AppendArgs(tbuf,
			     "int %s_EmitSignal_%c%s(%s *p_struct",
			     sTypePrefix,
			     toupper(*sSignalName),
			     sSignalName+1,
			     sTypeId);

      s=Typemaker2_Signal_GetParamType1(sig);            /* param 1 */
      if (s && *s && strcasecmp(s, "none")!=0)
	GWEN_Buffer_AppendArgs(tbuf, ", %s param1", s);

      s=Typemaker2_Signal_GetParamType2(sig);            /* param 2 */
      if (s && *s && strcasecmp(s, "none")!=0)
	GWEN_Buffer_AppendArgs(tbuf, ", %s param2", s);
      if (Typemaker2_Signal_GetFlags(sig) & TYPEMAKER2_SIGNAL_FLAGS_USE_PARAM3)
	GWEN_Buffer_AppendString(tbuf, ", int param3");
      if (Typemaker2_Signal_GetFlags(sig) & TYPEMAKER2_SIGNAL_FLAGS_USE_PARAM4)
	GWEN_Buffer_AppendString(tbuf, ", int param4");
      GWEN_Buffer_AppendString(tbuf, ");\n");


      sig=Typemaker2_Signal_List_Next(sig);
    }
    Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);

    /* implementation */
    GWEN_Buffer_AppendString(tbuf, "/* signal functions */\n");
    sig=Typemaker2_Signal_List_First(sigList);
    while (sig) {

      /* SignalConnect */
      GWEN_Buffer_AppendString(tbuf, "int ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_SignalConnect_");
      s=Typemaker2_Signal_GetName(sig);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_SLOT *slot) {\n");

      GWEN_Buffer_AppendString(tbuf, "  GWEN_SIGNAL *sig;\n");
      GWEN_Buffer_AppendString(tbuf, "\n");
      GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
      GWEN_Buffer_AppendString(tbuf, "  sig=GWEN_SignalObject_FindSignal(p_struct->_signalObject, ");
      s=Typemaker2_Signal_GetName(sig);       /* name */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "\", ");
      s=Typemaker2_Signal_GetParamType1(sig); /* param 1 */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s?s:"none");
      GWEN_Buffer_AppendString(tbuf, "\", ");
      s=Typemaker2_Signal_GetParamType2(sig); /* param 2 */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s?s:"none");
      GWEN_Buffer_AppendString(tbuf, "\");\n");
      GWEN_Buffer_AppendString(tbuf, "  if (sig)\n");
      GWEN_Buffer_AppendString(tbuf, "    return GWEN_Signal_Connect(sig, slot);\n");
      GWEN_Buffer_AppendString(tbuf, "  else\n");
      GWEN_Buffer_AppendString(tbuf, "    return GWEN_ERROR_NOT_FOUND;\n");

      GWEN_Buffer_AppendString(tbuf, "}\n");
      GWEN_Buffer_AppendString(tbuf, "\n");

      /* SignalDisconnect */
      GWEN_Buffer_AppendString(tbuf, "int ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_SignalDisconnect_");
      s=Typemaker2_Signal_GetName(sig);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_SLOT *slot) {\n");

      GWEN_Buffer_AppendString(tbuf, "  GWEN_SIGNAL *sig;\n");
      GWEN_Buffer_AppendString(tbuf, "\n");
      GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
      GWEN_Buffer_AppendString(tbuf, "  sig=GWEN_SignalObject_FindSignal(p_struct->_signalObject, ");
      s=Typemaker2_Signal_GetName(sig);       /* name */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "\", ");
      s=Typemaker2_Signal_GetParamType1(sig); /* param 1 */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s?s:"none");
      GWEN_Buffer_AppendString(tbuf, "\", ");
      s=Typemaker2_Signal_GetParamType2(sig); /* param 2 */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s?s:"none");
      GWEN_Buffer_AppendString(tbuf, "\");\n");
      GWEN_Buffer_AppendString(tbuf, "  if (sig)\n");
      GWEN_Buffer_AppendString(tbuf, "    return GWEN_Signal_Disconnect(sig, slot);\n");
      GWEN_Buffer_AppendString(tbuf, "  else\n");
      GWEN_Buffer_AppendString(tbuf, "    return GWEN_ERROR_NOT_FOUND;\n");

      GWEN_Buffer_AppendString(tbuf, "}\n");
      GWEN_Buffer_AppendString(tbuf, "\n");


      /* GetSignal */
      GWEN_Buffer_AppendString(tbuf, "GWEN_SIGNAL *");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_GetSignal_");
      s=Typemaker2_Signal_GetName(sig);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "(const ");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " *p_struct) {\n");

      GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
      GWEN_Buffer_AppendString(tbuf, "  return GWEN_SignalObject_FindSignal(p_struct->_signalObject, ");
      s=Typemaker2_Signal_GetName(sig);       /* name */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "\", ");
      s=Typemaker2_Signal_GetParamType1(sig); /* param 1 */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s?s:"none");
      GWEN_Buffer_AppendString(tbuf, "\", ");
      s=Typemaker2_Signal_GetParamType2(sig); /* param 2 */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s?s:"none");
      GWEN_Buffer_AppendString(tbuf, "\");\n");

      GWEN_Buffer_AppendString(tbuf, "}\n");
      GWEN_Buffer_AppendString(tbuf, "\n");


      /* EmitSignal */
      GWEN_Buffer_AppendString(tbuf, "int ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_EmitSignal_");
      s=Typemaker2_Signal_GetName(sig);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " *p_struct");
      s=Typemaker2_Signal_GetParamType1(sig);            /* param 1 */
      if (s && *s && strcasecmp(s, "none")) {
        GWEN_Buffer_AppendString(tbuf, ", ");
        GWEN_Buffer_AppendString(tbuf, s);
        GWEN_Buffer_AppendString(tbuf, " param1");
      }
      s=Typemaker2_Signal_GetParamType2(sig);            /* param 2 */
      if (s && *s && strcasecmp(s, "none")) {
        GWEN_Buffer_AppendString(tbuf, ", ");
        GWEN_Buffer_AppendString(tbuf, s);
        GWEN_Buffer_AppendString(tbuf, " param2");
      }
      if (Typemaker2_Signal_GetFlags(sig) & TYPEMAKER2_SIGNAL_FLAGS_USE_PARAM3) {
        GWEN_Buffer_AppendString(tbuf, ", int param3");
      }
      if (Typemaker2_Signal_GetFlags(sig) & TYPEMAKER2_SIGNAL_FLAGS_USE_PARAM4) {
        GWEN_Buffer_AppendString(tbuf, ", int param4");
      }
      GWEN_Buffer_AppendString(tbuf, "){\n");

      GWEN_Buffer_AppendString(tbuf, "  GWEN_SIGNAL *sig;\n");
      GWEN_Buffer_AppendString(tbuf, "\n");
      GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
      GWEN_Buffer_AppendString(tbuf, "  sig=GWEN_SignalObject_FindSignal(p_struct->_signalObject, ");
      s=Typemaker2_Signal_GetName(sig);       /* name */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "\", ");

      s=Typemaker2_Signal_GetParamType1(sig); /* param 1 */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s?s:"none");
      GWEN_Buffer_AppendString(tbuf, "\", ");
      s=Typemaker2_Signal_GetParamType2(sig); /* param 2 */
      GWEN_Buffer_AppendString(tbuf, "\"");
      GWEN_Buffer_AppendString(tbuf, s?s:"none");
      GWEN_Buffer_AppendString(tbuf, "\");\n");

      GWEN_Buffer_AppendString(tbuf, "  if (sig) \n");
      GWEN_Buffer_AppendString(tbuf, "    return GWEN_Signal_Emit(sig, ");
      s=Typemaker2_Signal_GetParamType1(sig);            /* param 1 */
      if (s && *s && strcasecmp(s, "none")) {
        GWEN_Buffer_AppendString(tbuf, "(void*) ");
        GWEN_Buffer_AppendString(tbuf, "param1, ");
      }
      else
        GWEN_Buffer_AppendString(tbuf, "NULL, ");
      s=Typemaker2_Signal_GetParamType2(sig);            /* param 2 */
      if (s && *s && strcasecmp(s, "none")) {
        GWEN_Buffer_AppendString(tbuf, "(void*) ");
        GWEN_Buffer_AppendString(tbuf, " param2, ");
      }
      else
        GWEN_Buffer_AppendString(tbuf, "NULL, ");
      if (Typemaker2_Signal_GetFlags(sig) & TYPEMAKER2_SIGNAL_FLAGS_USE_PARAM3)
        GWEN_Buffer_AppendString(tbuf, "param3, ");
      else
        GWEN_Buffer_AppendString(tbuf, "0, ");
      if (Typemaker2_Signal_GetFlags(sig) & TYPEMAKER2_SIGNAL_FLAGS_USE_PARAM4)
        GWEN_Buffer_AppendString(tbuf, "param4");
      else
        GWEN_Buffer_AppendString(tbuf, "0");
      GWEN_Buffer_AppendString(tbuf, ");\n");


      GWEN_Buffer_AppendString(tbuf, "  else\n");
      GWEN_Buffer_AppendString(tbuf, "    return 0;\n");

      GWEN_Buffer_AppendString(tbuf, "}\n");
      GWEN_Buffer_AppendString(tbuf, "\n");


      sig=Typemaker2_Signal_List_Next(sig);
    }
    Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);

  }
  GWEN_Buffer_free(tbuf);

  return 0;
}
