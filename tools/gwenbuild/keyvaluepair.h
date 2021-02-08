/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_KEYVALUEPAIR_H
#define GWBUILD_KEYVALUEPAIR_H


#include <gwenhywfar/list1.h>


typedef struct GWB_KEYVALUEPAIR GWB_KEYVALUEPAIR;
GWEN_LIST_FUNCTION_DEFS(GWB_KEYVALUEPAIR, GWB_KeyValuePair)

GWB_KEYVALUEPAIR *GWB_KeyValuePair_new(const char *key, const char *value);
GWB_KEYVALUEPAIR *GWB_KeyValuePair_dup(const GWB_KEYVALUEPAIR *originalKvp);
void GWB_KeyValuePair_free(GWB_KEYVALUEPAIR *kvp);

const char *GWB_KeyValuePair_GetKey(const GWB_KEYVALUEPAIR *kvp);
void GWB_KeyValuePair_SetKey(GWB_KEYVALUEPAIR *kvp, const char *s);

const char *GWB_KeyValuePair_GetValue(const GWB_KEYVALUEPAIR *kvp);
void GWB_KeyValuePair_SetValue(GWB_KEYVALUEPAIR *kvp, const char *s);


GWB_KEYVALUEPAIR_LIST *GWB_KeyValuePair_List_dup(const GWB_KEYVALUEPAIR_LIST *kvpList);

GWB_KEYVALUEPAIR *GWB_KeyValuePair_List_GetByKey(const GWB_KEYVALUEPAIR_LIST *kvpList, const char *key);

const char *GWB_KeyValuePair_List_GetValue(const GWB_KEYVALUEPAIR_LIST *kvpList, const char *key);

#endif
