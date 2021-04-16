/***************************************************************************
    begin       : Fri Apr 02 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "gwenbuild/option_p.h"

#include "gwenbuild/keyvaluepair.h"
#include "gwenbuild/gwenbuild.h"

#include <gwenhywfar/misc.h>




GWEN_LIST_FUNCTIONS(GWB_OPTION, GWB_Option)




GWB_OPTION *GWB_Option_new(const char *id)
{
  GWB_OPTION *option;

  GWEN_NEW_OBJECT(GWB_OPTION, option);
  GWEN_LIST_INIT(GWB_OPTION, option);

  option->id=id?strdup(id):NULL;
  option->aliasList=GWB_KeyValuePair_List_new();
  option->choiceList=GWEN_StringList_new();

  return option;
}



void GWB_Option_free(GWB_OPTION *option)
{
  if (option) {
    GWEN_LIST_FINI(GWB_OPTION, option);
    free(option->id);
    free(option->defaultValue);
    free(option->definePrefix);

    GWEN_StringList_free(option->choiceList);
    GWB_KeyValuePair_List_free(option->aliasList);

    GWEN_FREE_OBJECT(option);
  }
}



const char *GWB_Option_GetId(const GWB_OPTION *option)
{
  return option->id;
}



int GWB_Option_GetOptionType(const GWB_OPTION *option)
{
  return option->optionType;
}



void GWB_Option_SetOptionType(GWB_OPTION *option, int i)
{
  option->optionType=i;
}



const char *GWB_Option_GetDefaultValue(const GWB_OPTION *option)
{
  return option->defaultValue;
}



void GWB_Option_SetDefaultValue(GWB_OPTION *option, const char *s)
{
  free(option->defaultValue);
  option->defaultValue=s?strdup(s):NULL;
}



GWB_KEYVALUEPAIR_LIST *GWB_Option_GetAliasList(const GWB_OPTION *option)
{
  return option->aliasList;
}



void GWB_Option_AddAlias(GWB_OPTION *option, const char *name, const char *value)
{
  GWB_KEYVALUEPAIR *kvp;

  kvp=GWB_KeyValuePair_new(name, value);
  GWB_KeyValuePair_List_Add(kvp, option->aliasList);
}



const char *GWB_Option_GetAlias(const GWB_OPTION *option, const char *name)
{
  return GWB_KeyValuePair_List_GetValue(option->aliasList, name);
}



GWEN_STRINGLIST *GWB_Option_GetChoiceList(const GWB_OPTION *option)
{
  return option->choiceList;
}



int GWB_Option_HasChoices(const GWB_OPTION *option)
{
  return (GWEN_StringList_Count(option->choiceList)>0)?1:0;
}



int GWB_Option_IsValidChoice(const GWB_OPTION *option, const char *s)
{
  if (GWEN_StringList_Count(option->choiceList)==0)
    return 1;
  if (GWEN_StringList_HasString(option->choiceList, s))
    return 1;

  return 0;
}



void GWB_Option_AddChoice(GWB_OPTION *option, const char *s)
{
  GWEN_StringList_AppendString(option->choiceList, s, 0, 1);
}



const char *GWB_Option_GetDefinePrefix(const GWB_OPTION *option)
{
  return option->definePrefix;
}



void GWB_Option_SetDefinePrefix(GWB_OPTION *option, const char *s)
{
  free(option->definePrefix);
  option->definePrefix=s?strdup(s):NULL;
}




void GWB_Option_Dump(const GWB_OPTION *option, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "Option:\n");

  GWBUILD_Debug_PrintValue(     "id..........", option->id, indent+2);
  GWBUILD_Debug_PrintValue(     "defaultValue", option->defaultValue, indent+2);
  GWBUILD_Debug_PrintValue(     "definePrefix", option->definePrefix, indent+2);
  GWBUILD_Debug_PrintKvpList(   "aliasList...", option->aliasList, indent+2);
  GWBUILD_Debug_PrintStringList("choiceList..", option->choiceList, indent+2);
}





const char *GWB_OptionType_toString(int t)
{
  switch(t) {
  case GWB_OptionType_Unknown:     return "unknown";
  case GWB_OptionType_None:        return "none";
  case GWB_OptionType_String:      return "string";
  case GWB_OptionType_StringList:  return "stringlist";
  }

  return "unknown";
}



int GWB_OptionType_fromString(const char *s)
{
  if (s) {
    if (strcasecmp(s, "unknown")==0)
      return GWB_OptionType_Unknown;
    else if (strcasecmp(s, "none")==0)
      return GWB_OptionType_None;
    else if (strcasecmp(s, "string")==0)
      return GWB_OptionType_String;
    else if (strcasecmp(s, "stringlist")==0)
      return GWB_OptionType_StringList;
  }
  return GWB_OptionType_Unknown;
}






