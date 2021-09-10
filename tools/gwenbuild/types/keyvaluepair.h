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
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/xml.h>


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

const char *GWB_KeyValuePair_List_GetValue(const GWB_KEYVALUEPAIR_LIST *kvpList, const char *key);


GWB_KEYVALUEPAIR *GWB_KeyValuePair_List_GetFirstByKey(const GWB_KEYVALUEPAIR_LIST *kvpList, const char *key);
GWB_KEYVALUEPAIR *GWB_KeyValuePair_List_GetNextByKey(const GWB_KEYVALUEPAIR *kvp, const char *key);


int GWB_KeyValuePair_List_SampleValuesByKey(const GWB_KEYVALUEPAIR_LIST *kvpList,
                                            const char *key,
                                            const char *prefix,
                                            const char *delim,
                                            GWEN_BUFFER *destBuf);

/**
 * Adds all key-value-pairs to the given buffer.
 *
 * If you want to use the given key-value-pairs as DEFINEs for a compiler input you can set the following values
 * to the arguments: prefix="-D" assignmentString="=" delim=" " (space), resulting in "-DKEY1=VALUE1 -DKEY2=VALUE2".
 *
 * @return number of entries added to the buffer
 * @param kvpList list of key-value-pairs to add
 * @param prefix string to prepend before adding each key-value-pair
 * @param assignmentString assignment chars (e.g. "=", ":=" etc)
 * @param delim delimiter between each pair
 * @param destBuf buffer to write to
 */
int GWB_KeyValuePair_List_WriteAllPairsToBuffer(const GWB_KEYVALUEPAIR_LIST *kvpList,
                                                const char *prefix,
                                                const char *assignmentString,
                                                const char *delim,
                                                GWEN_BUFFER *destBuf);

void GWB_KeyValuePair_toXml(const GWB_KEYVALUEPAIR *kvp, GWEN_XMLNODE *xmlNode);

void GWB_KeyValuePair_List_WriteXml(const GWB_KEYVALUEPAIR_LIST *kvpList, GWEN_XMLNODE *xmlNode, const char *groupName);


#endif
