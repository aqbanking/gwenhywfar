/***************************************************************************
    begin       : Fri Apr 02 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_OPTION_H
#define GWBUILD_OPTION_H

#include <gwenhywfar/list1.h>
#include <gwenhywfar/stringlist.h>


typedef struct GWB_OPTION GWB_OPTION;
GWEN_LIST_FUNCTION_DEFS(GWB_OPTION, GWB_Option)


enum {
  GWB_OptionType_Unknown=-1,
  GWB_OptionType_None=0,
  GWB_OptionType_String,
  GWB_OptionType_StringList
};


#include "gwenbuild/keyvaluepair.h"



GWB_OPTION *GWB_Option_new(const char *id);
void GWB_Option_free(GWB_OPTION *option);

const char *GWB_Option_GetId(const GWB_OPTION *option);

int GWB_Option_GetOptionType(const GWB_OPTION *option);
void GWB_Option_SetOptionType(GWB_OPTION *option, int i);

const char *GWB_Option_GetDefaultValue(const GWB_OPTION *option);
void GWB_Option_SetDefaultValue(GWB_OPTION *option, const char *s);

GWB_KEYVALUEPAIR_LIST *GWB_Option_GetAliasList(const GWB_OPTION *option);
void GWB_Option_AddAlias(GWB_OPTION *option, const char *name, const char *value);
const char *GWB_Option_GetAlias(const GWB_OPTION *option, const char *name);

GWEN_STRINGLIST *GWB_Option_GetChoiceList(const GWB_OPTION *option);
int GWB_Option_HasChoices(const GWB_OPTION *option);
int GWB_Option_IsValidChoice(const GWB_OPTION *option, const char *s);
void GWB_Option_AddChoice(GWB_OPTION *option, const char *s);


void GWB_Option_Dump(const GWB_OPTION *option, int indent);


const char *GWB_OptionType_toString(int t);
int GWB_OptionType_fromString(const char *s);


#endif
