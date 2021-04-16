/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "gwenbuild/parser/p_options.h"
#include "gwenbuild/parser/parser.h"
#include "gwenbuild/option.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>

#include <ctype.h>



int _checkAgainstGivenOption(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWB_OPTION *option);
int _checkStringOption(GWB_OPTION *option, GWB_CONTEXT *currentContext, const char *givenValue);
int _checkStringListOption(GWB_PROJECT *project, GWB_OPTION *option, GWB_CONTEXT *currentContext, const char *givenValue);






int GWB_ParseOption(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  const char *sId;
  int otype;
  GWB_OPTION *option;
  const char *s;
  GWEN_XMLNODE *n;
  int rv;

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  sId=GWEN_XMLNode_GetProperty(xmlNode, "id", NULL);
  if (!(sId && *sId)) {
    DBG_ERROR(NULL, "Option has no id");
    GWEN_XMLNode_Dump(xmlNode, 2);
    return GWEN_ERROR_GENERIC;
  }

  otype=GWB_OptionType_fromString(GWEN_XMLNode_GetProperty(xmlNode, "type", "string"));
  if (!(otype>GWB_OptionType_None)) {
    DBG_ERROR(NULL, "Invalid option type (id=%s)", sId);
    GWEN_XMLNode_Dump(xmlNode, 2);
    return GWEN_ERROR_GENERIC;
  }

  option=GWB_Option_new(sId);
  GWB_Option_SetOptionType(option, otype);

  s=GWEN_XMLNode_GetCharValue(xmlNode, "default", NULL);
  if (s)
    GWB_Option_SetDefaultValue(option, s);

  s=GWEN_XMLNode_GetProperty(xmlNode, "definePrefix", NULL);
  if (s)
    GWB_Option_SetDefinePrefix(option, s);

  s=GWEN_XMLNode_GetCharValue(xmlNode, "choices", NULL);
  if (s) {
    GWEN_STRINGLIST *sl;

    sl=GWEN_StringList_fromString2(s, " ", 1, GWEN_TEXT_FLAGS_DEL_QUOTES | GWEN_TEXT_FLAGS_CHECK_BACKSLASH);
    if (sl) {
      GWEN_StringList_AppendStringList(GWB_Option_GetChoiceList(option), sl, 1);
      GWEN_StringList_free(sl);
    }
  }

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "alias", NULL, NULL);
  while(n) {
    const char *sName;
    GWEN_BUFFER *valueBuffer;

    sName=GWEN_XMLNode_GetProperty(n, "name", NULL);
    valueBuffer=GWB_Parser_ReadXmlDataIntoBufferAndExpand(GWB_Context_GetVars(currentContext), n);
    if (valueBuffer) {
      GWB_Option_AddAlias(option, sName, GWEN_Buffer_GetStart(valueBuffer));
      GWEN_Buffer_free(valueBuffer);
    }
    n=GWEN_XMLNode_FindNextTag(n, "alias", NULL, NULL);
  }

  _checkAgainstGivenOption(project, currentContext, option);

  GWB_Project_AddOption(project, option);
  return 0;
}



int _checkAgainstGivenOption(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWB_OPTION *option)
{
  const char *optionId;
  GWB_KEYVALUEPAIR *kvp;
  const char *givenValue=NULL;

  optionId=GWB_Option_GetId(option);
  kvp=GWB_KeyValuePair_List_GetFirstByKey(GWB_Project_GetGivenOptionList(project), optionId);
  if (kvp)
    givenValue=GWB_KeyValuePair_GetValue(kvp);
  if (givenValue==NULL)
    givenValue=GWB_Option_GetDefaultValue(option);
  if (givenValue) {
    int rv=GWEN_ERROR_GENERIC;

    switch(GWB_Option_GetOptionType(option)) {
    case GWB_OptionType_Unknown:
    case GWB_OptionType_None:
      DBG_ERROR(NULL, "Bad option type in option %s", optionId);
      rv=GWEN_ERROR_GENERIC;
      break;
    case GWB_OptionType_String:
      rv=_checkStringOption(option, currentContext, givenValue);
      break;
    case GWB_OptionType_StringList:
      rv=_checkStringListOption(project, option, currentContext, givenValue);
      break;
    }
    if (rv<0)
      return rv;
  }

  if (kvp) {
    GWB_KeyValuePair_List_Del(kvp);
    GWB_KeyValuePair_free(kvp);
  }

  return 0;
}



int _checkStringOption(GWB_OPTION *option, GWB_CONTEXT *currentContext, const char *givenValue)
{
  const char *optionId;

  optionId=GWB_Option_GetId(option);
  fprintf(stdout, " option %s: ", optionId);
  if (givenValue) {
    const char *s;
    GWEN_BUFFER *nameBuffer;

    s=GWB_Option_GetAlias(option, givenValue);
    if (s==NULL)
      s=givenValue;

    if (!GWB_Option_IsValidChoice(option, s)) {
      DBG_ERROR(NULL,
                "Value \"%s\" (given value \"%s\") is not a valid choice for option \"%s\"",
                s, givenValue, optionId);
      return GWEN_ERROR_INVALID;
    }

    nameBuffer=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(nameBuffer, "option_");
    GWEN_Buffer_AppendString(nameBuffer, optionId);
    GWEN_DB_SetCharValue(GWB_Context_GetVars(currentContext),
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         GWEN_Buffer_GetStart(nameBuffer), s);
    GWEN_Buffer_free(nameBuffer);
    fprintf(stdout, "%s\n", s);
  }

  return 0;
}



int _checkStringListOption(GWB_PROJECT *project, GWB_OPTION *option, GWB_CONTEXT *currentContext, const char *givenValue)
{
  const char *optionId;
  const char *definePrefix;

  optionId=GWB_Option_GetId(option);
  definePrefix=GWB_Option_GetDefinePrefix(option);
  fprintf(stdout, " option %s: ", optionId);
  if (givenValue) {
    const char *s;
    GWEN_STRINGLIST *sl;

    s=GWB_Option_GetAlias(option, givenValue);
    if (s==NULL)
      s=givenValue;

    sl=GWEN_StringList_fromString2(s, " ", 1, GWEN_TEXT_FLAGS_DEL_QUOTES | GWEN_TEXT_FLAGS_CHECK_BACKSLASH);
    if (sl) {
      GWEN_STRINGLISTENTRY *se;
      GWEN_BUFFER *nameBuffer;

      nameBuffer=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Buffer_AppendString(nameBuffer, "option_");
      GWEN_Buffer_AppendString(nameBuffer, optionId);

      se=GWEN_StringList_FirstEntry(sl);
      while(se) {
        const char *sCurrentGivenValue;

        sCurrentGivenValue=GWEN_StringListEntry_Data(se);
        if (sCurrentGivenValue) {
          if (!GWB_Option_IsValidChoice(option, sCurrentGivenValue)) {
            DBG_ERROR(NULL,
                      "Value \"%s\" is not a valid choice for option \"%s\"",
                      sCurrentGivenValue, optionId);
            GWEN_Buffer_free(nameBuffer);
            GWEN_StringList_free(sl);
            return GWEN_ERROR_INVALID;
          }
          GWEN_DB_SetCharValue(GWB_Context_GetVars(currentContext),
                               0,
                               GWEN_Buffer_GetStart(nameBuffer), sCurrentGivenValue);
          fprintf(stdout, "%s ", sCurrentGivenValue);
          if (definePrefix) {
            GWEN_BUFFER *dbuf;

            dbuf=GWEN_Buffer_new(0, 64, 0, 1);
            GWEN_Buffer_AppendString(dbuf, definePrefix);
            s=sCurrentGivenValue;
            while(*s)
	      GWEN_Buffer_AppendByte(dbuf, toupper(*(s++)));
            GWB_Project_SetDefine(project, GWEN_Buffer_GetStart(dbuf), "1");
            GWEN_Buffer_free(dbuf);
          }
        }

        se=GWEN_StringListEntry_Next(se);
      } /* while */
      GWEN_Buffer_free(nameBuffer);
      GWEN_StringList_free(sl);
    } /* if sl */

  } /* if givenValue */
  fprintf(stdout, "\n");
  return 0;
}






