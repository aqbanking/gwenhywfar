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


#include "gwenbuild/keyvaluepair_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/memory.h>

#include <stdlib.h>



GWEN_LIST_FUNCTIONS(GWB_KEYVALUEPAIR, GWB_KeyValuePair)




GWB_KEYVALUEPAIR *GWB_KeyValuePair_new(const char *key, const char *value)
{
  GWB_KEYVALUEPAIR *kvp;

  GWEN_NEW_OBJECT(GWB_KEYVALUEPAIR, kvp);
  GWEN_LIST_INIT(GWB_KEYVALUEPAIR, kvp);

  if (key)
    GWB_KeyValuePair_SetKey(kvp, key);
  if (value)
    GWB_KeyValuePair_SetValue(kvp, value);

  return kvp;
}



GWB_KEYVALUEPAIR *GWB_KeyValuePair_dup(const GWB_KEYVALUEPAIR *originalKvp)
{
  GWB_KEYVALUEPAIR *kvp;

  GWEN_NEW_OBJECT(GWB_KEYVALUEPAIR, kvp);
  GWEN_LIST_INIT(GWB_KEYVALUEPAIR, kvp);

  if (originalKvp->key)
    GWB_KeyValuePair_SetKey(kvp, originalKvp->key);
  if (originalKvp->value)
    GWB_KeyValuePair_SetValue(kvp, originalKvp->value);

  return kvp;
}



void GWB_KeyValuePair_free(GWB_KEYVALUEPAIR *kvp)
{
  if (kvp) {
    free(kvp->value);
    free(kvp->key);
    GWEN_FREE_OBJECT(kvp);
  }
}



const char *GWB_KeyValuePair_GetKey(const GWB_KEYVALUEPAIR *kvp)
{
  return kvp->key;
}



void GWB_KeyValuePair_SetKey(GWB_KEYVALUEPAIR *kvp, const char *s)
{
  free(kvp->key);
  if (s)
    kvp->key=strdup(s);
  else
    kvp->key=NULL;
}



const char *GWB_KeyValuePair_GetValue(const GWB_KEYVALUEPAIR *kvp)
{
  return kvp->value;
}



void GWB_KeyValuePair_SetValue(GWB_KEYVALUEPAIR *kvp, const char *s)
{
  free(kvp->value);
  if (s)
    kvp->value=strdup(s);
  else
    kvp->value=NULL;
}



GWB_KEYVALUEPAIR_LIST *GWB_KeyValuePair_List_dup(const GWB_KEYVALUEPAIR_LIST *oldKvpList)
{
  GWB_KEYVALUEPAIR_LIST *kvpList;
  GWB_KEYVALUEPAIR *kvp;

  kvpList=GWB_KeyValuePair_List_new();
  kvp=GWB_KeyValuePair_List_First(oldKvpList);
  while(kvp) {
    GWB_KeyValuePair_List_Add(GWB_KeyValuePair_dup(kvp), kvpList);
    kvp=GWB_KeyValuePair_List_Next(kvp);
  }

  return kvpList;
}



GWB_KEYVALUEPAIR *GWB_KeyValuePair_List_GetByKey(const GWB_KEYVALUEPAIR_LIST *kvpList, const char *key)
{
  GWB_KEYVALUEPAIR *kvp;

  kvp=GWB_KeyValuePair_List_First(kvpList);
  while(kvp) {
    const char *s;

    s=GWB_KeyValuePair_GetKey(kvp);
    if (s && strcasecmp(s, key)==0)
      return kvp;
    kvp=GWB_KeyValuePair_List_Next(kvp);
  }

  return NULL;
}



const char *GWB_KeyValuePair_List_GetValue(const GWB_KEYVALUEPAIR_LIST *kvpList, const char *key)
{
  GWB_KEYVALUEPAIR *kvp;

  kvp=GWB_KeyValuePair_List_GetByKey(kvpList, key);
  if (kvp==NULL)
    return NULL;
  return kvp->value;
}






