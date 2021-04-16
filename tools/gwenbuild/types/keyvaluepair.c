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


#include "gwenbuild/types/keyvaluepair_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/memory.h>

#include <stdlib.h>



GWEN_LIST_FUNCTIONS(GWB_KEYVALUEPAIR, GWB_KeyValuePair)




static GWB_KEYVALUEPAIR *_getByKey(GWB_KEYVALUEPAIR *kvp, const char *key);




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



const char *GWB_KeyValuePair_List_GetValue(const GWB_KEYVALUEPAIR_LIST *kvpList, const char *key)
{
  GWB_KEYVALUEPAIR *kvp;

  kvp=GWB_KeyValuePair_List_GetFirstByKey(kvpList, key);
  if (kvp==NULL)
    return NULL;
  return kvp->value;
}



GWB_KEYVALUEPAIR *GWB_KeyValuePair_List_GetFirstByKey(const GWB_KEYVALUEPAIR_LIST *kvpList, const char *key)
{
  return _getByKey(GWB_KeyValuePair_List_First(kvpList), key);
}



GWB_KEYVALUEPAIR *GWB_KeyValuePair_List_GetNextByKey(const GWB_KEYVALUEPAIR *kvp, const char *key)
{
  if (kvp)
    return _getByKey(GWB_KeyValuePair_List_Next(kvp), key);
  return NULL;
}



GWB_KEYVALUEPAIR *_getByKey(GWB_KEYVALUEPAIR *kvp, const char *key)
{
  while(kvp) {
    if (kvp->key && strcmp(kvp->key, key)==0)
      return kvp;
    kvp=GWB_KeyValuePair_List_Next(kvp);
  }

  return NULL;
}



int GWB_KeyValuePair_List_SampleValuesByKey(const GWB_KEYVALUEPAIR_LIST *kvpList,
                                            const char *key,
                                            const char *prefix,
                                            const char *delim,
                                            GWEN_BUFFER *destBuf)
{
  const GWB_KEYVALUEPAIR *kvp;
  int entriesAdded=0;

  kvp=GWB_KeyValuePair_List_GetFirstByKey(kvpList, key);
  while(kvp) {
    if (kvp->value && *(kvp->value)) {
      if (entriesAdded && delim)
        GWEN_Buffer_AppendString(destBuf, delim);
      if (prefix)
        GWEN_Buffer_AppendString(destBuf, prefix);
      GWEN_Buffer_AppendString(destBuf, kvp->value);
      entriesAdded++;
    }
    kvp=GWB_KeyValuePair_List_GetNextByKey(kvp, key);
  }
  return entriesAdded;
}



int GWB_KeyValuePair_List_WriteAllPairsToBuffer(const GWB_KEYVALUEPAIR_LIST *kvpList,
                                                const char *prefix,
                                                const char *assignmentString,
                                                const char *delim,
                                                GWEN_BUFFER *destBuf)
{
  const GWB_KEYVALUEPAIR *kvp;
  int entriesAdded=0;

  kvp=GWB_KeyValuePair_List_First(kvpList);
  while(kvp) {
    if (kvp->key && *(kvp->key)) {
      if (entriesAdded && delim)
        GWEN_Buffer_AppendString(destBuf, delim);
      if (prefix)
        GWEN_Buffer_AppendString(destBuf, prefix);
      GWEN_Buffer_AppendString(destBuf, kvp->key);
      if (kvp->value && *(kvp->value)) {
        if (assignmentString)
          GWEN_Buffer_AppendString(destBuf, assignmentString);
        GWEN_Buffer_AppendString(destBuf, kvp->value);
      }
      entriesAdded++;
    }
    kvp=GWB_KeyValuePair_List_Next(kvp);
  }
  return entriesAdded;
}



