/***************************************************************************
 $RCSfile: account.h,v $
 -------------------
 cvs         : $Id: account.h,v 1.16 2006/02/22 19:38:54 aquamaniac Exp $
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "st_client_p.h"
#include "st_storage_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>


GWEN_INHERIT_FUNCTIONS(GWEN_STO_CLIENT)
GWEN_LIST_FUNCTIONS(GWEN_STO_CLIENT, GWEN_StoClient)
GWEN_LIST2_FUNCTIONS(GWEN_STO_CLIENT, GWEN_StoClient)


GWEN_STO_CLIENT *GWEN_StoClient_new(GWEN_STO_STORAGE *st,
                                    GWEN_TYPE_UINT32 id,
                                    const char *userName) {
  GWEN_STO_CLIENT *cl;

  GWEN_NEW_OBJECT(GWEN_STO_CLIENT, cl);
  GWEN_INHERIT_INIT(GWEN_STO_CLIENT, cl);
  GWEN_LIST_INIT(GWEN_STO_CLIENT, cl);

  cl->storage=st;
  cl->id=id;
  cl->objectList=GWEN_StoObject_List_new();
  cl->typeList=GWEN_StoType_List_new();
  if (userName)
    cl->userName=strdup(userName);
  cl->openObjectIdList=GWEN_IdList_new();
  cl->openTypeIdList=GWEN_IdList_new();
  cl->logList=GWEN_StoLog_List_new();

  return cl;
}



void GWEN_StoClient_free(GWEN_STO_CLIENT *cl) {
  if (cl) {
    GWEN_LIST_FINI(GWEN_STO_CLIENT, cl);
    GWEN_INHERIT_FINI(GWEN_STO_CLIENT, cl);
    GWEN_StoLog_List_free(cl->logList);
    GWEN_IdList_free(cl->openObjectIdList);
    GWEN_StoObject_List_free(cl->objectList);
    GWEN_StoType_List_free(cl->typeList);
    free(cl->userName);
    GWEN_FREE_OBJECT(cl);
  }
}



GWEN_STO_TYPE_LIST *GWEN_StoClient_GetTypeList(const GWEN_STO_CLIENT *cl){
  assert(cl);
  return cl->typeList;
}



GWEN_STO_TYPE *GWEN_StoClient_FindTypeByName(const GWEN_STO_CLIENT *cl,
                                             const char *typeName,
                                             const char *name) {
  GWEN_STO_TYPE *ty;

  assert(cl);
  assert(typeName);
  if (name==0)
    name="";

  ty=GWEN_StoType_List_First(cl->typeList);
  while(ty) {
    const char *n;

    n=GWEN_StoType_GetName(ty);
    if (n==0)
      n="";
    if ((strcasecmp(typeName, GWEN_StoType_GetTypeName(ty))==0) &&
        (strcasecmp(name, n)==0))
      break;
    ty=GWEN_StoType_List_Next(ty);
  }

  return ty;
}



void GWEN_StoClient_AddType(GWEN_STO_CLIENT *cl, GWEN_STO_TYPE *ty) {
  assert(cl);
  GWEN_StoType_List_Add(ty, cl->typeList);
}



GWEN_STO_OBJECT_LIST *GWEN_StoClient_GetObjectList(const GWEN_STO_CLIENT *cl){
  assert(cl);
  return cl->objectList;
}



GWEN_STO_OBJECT *GWEN_StoClient_FindObject(const GWEN_STO_CLIENT *cl,
                                           GWEN_TYPE_UINT32 id) {
  GWEN_STO_OBJECT *o;

  assert(cl);
  o=GWEN_StoObject_List_First(cl->objectList);
  while(o) {
    if (GWEN_StoObject_GetId(o)==id)
      break;
    o=GWEN_StoObject_List_Next(o);
  }

  return o;
}



void GWEN_StoClient_AddObject(GWEN_STO_CLIENT *cl, GWEN_STO_OBJECT *o) {
  assert(cl);
  GWEN_StoObject_List_Add(o, cl->objectList);
}



GWEN_IDLIST *GWEN_StoClient_GetOpenObjectIdList(const GWEN_STO_CLIENT *cl) {
  assert(cl);
  return cl->openObjectIdList;
}



void GWEN_StoClient_AddOpenObjectId(GWEN_STO_CLIENT *cl,
                                    GWEN_TYPE_UINT32 oid) {
  assert(cl);
  GWEN_IdList_AddId(cl->openObjectIdList, oid);
}



void GWEN_StoClient_DelOpenObjectId(GWEN_STO_CLIENT *cl,
                                    GWEN_TYPE_UINT32 oid) {
  assert(cl);
  GWEN_IdList_DelId(cl->openObjectIdList, oid);
}



GWEN_IDLIST *GWEN_StoClient_GetOpenTypeIdList(const GWEN_STO_CLIENT *cl) {
  assert(cl);
  return cl->openTypeIdList;
}



void GWEN_StoClient_AddOpenTypeId(GWEN_STO_CLIENT *cl,
                                    GWEN_TYPE_UINT32 oid) {
  assert(cl);
  GWEN_IdList_AddId(cl->openTypeIdList, oid);
}



void GWEN_StoClient_DelOpenTypeId(GWEN_STO_CLIENT *cl,
                                    GWEN_TYPE_UINT32 oid) {
  assert(cl);
  GWEN_IdList_DelId(cl->openTypeIdList, oid);
}



GWEN_TYPE_UINT32 GWEN_StoClient_GetId(const GWEN_STO_CLIENT *cl) {
  assert(cl);
  return cl->id;
}



const char *GWEN_StoClient_GetUserName(const GWEN_STO_CLIENT *cl) {
  assert(cl);
  return cl->userName;
}



GWEN_STO_LOG_LIST *GWEN_StoClient_GetLogList(const GWEN_STO_CLIENT *cl) {
  assert(cl);
  return cl->logList;
}



void GWEN_StoClient_AddLog(GWEN_STO_CLIENT *cl, GWEN_STO_LOG *log) {
  assert(cl);
  assert(log);
  GWEN_StoLog_List_Add(log, cl->logList);
}








int GWEN_StoClient_Create(GWEN_STO_CLIENT *cl) {
  assert(cl);
  return GWEN_StoStorage_Create(cl->storage, cl);
}



int GWEN_StoClient_Open(GWEN_STO_CLIENT *cl,
                        GWEN_TYPE_UINT32 openFlags) {
  assert(cl);
  return GWEN_StoStorage_Open(cl->storage, cl, openFlags);
}



int GWEN_StoClient_Close(GWEN_STO_CLIENT *cl) {
  assert(cl);
  return GWEN_StoStorage_Close(cl->storage, cl);
}


int GWEN_StoClient_BeginEdit(GWEN_STO_CLIENT *cl) {
  assert(cl);
  return GWEN_StoStorage_BeginEdit(cl->storage, cl);
}



int GWEN_StoClient_EndEdit(GWEN_STO_CLIENT *cl,
                           GWEN_STO_CLOSEMODE cm) {
  assert(cl);
  return GWEN_StoStorage_EndEdit(cl->storage, cl, cm);
}



int GWEN_StoClient_CreateType(GWEN_STO_CLIENT *cl,
                              const char *typeName,
                              const char *name,
                              GWEN_STO_TYPE **pts) {
  assert(cl);
  return GWEN_StoStorage_CreateType(cl->storage, cl, typeName, name, pts);
}



int GWEN_StoClient_OpenType(GWEN_STO_CLIENT *cl,
                            const char *typeName,
                            const char *name,
                            GWEN_TYPE_UINT32 openFlags,
                            GWEN_STO_TYPE **pts) {
  assert(cl);
  return GWEN_StoStorage_OpenType(cl->storage, cl, typeName, name,
                                  openFlags, pts);
}



int GWEN_StoClient_CloseType(GWEN_STO_CLIENT *cl,
                             GWEN_STO_TYPE *ts,
                             GWEN_STO_CLOSEMODE cm) {
  assert(cl);
  return GWEN_StoStorage_CloseType(cl->storage, cl, ts, cm);
}



int GWEN_StoClient_FindFirstObject(GWEN_STO_CLIENT *cl,
                                   GWEN_STO_TYPE *ts,
                                   GWEN_STO_FIND **pfnd,
                                   GWEN_TYPE_UINT32 *id) {
  assert(cl);
  return GWEN_StoStorage_FindFirstObject(cl->storage, cl, ts, pfnd, id);
}



int GWEN_StoClient_FindNextObject(GWEN_STO_CLIENT *cl,
                                  GWEN_STO_TYPE *ts,
                                  GWEN_STO_FIND *fnd,
                                  GWEN_TYPE_UINT32 *id) {
  assert(cl);
  return GWEN_StoStorage_FindNextObject(cl->storage, cl, ts, fnd, id);
}



int GWEN_StoClient_CloseFind(GWEN_STO_CLIENT *cl,
                             GWEN_STO_TYPE *ts,
                             GWEN_STO_FIND *fnd) {
  assert(cl);
  return GWEN_StoStorage_CloseFind(cl->storage, cl, ts, fnd);
}



int GWEN_StoClient_CreateObject(GWEN_STO_CLIENT *cl,
                                GWEN_STO_TYPE *ts,
                                GWEN_STO_OBJECT **po) {
  assert(cl);
  return GWEN_StoStorage_CreateObject(cl->storage, cl, ts, po);
}



int GWEN_StoClient_OpenObject(GWEN_STO_CLIENT *cl,
                              GWEN_STO_TYPE *ts,
                              GWEN_TYPE_UINT32 id,
                              GWEN_TYPE_UINT32 openFlags,
                              GWEN_STO_OBJECT **po) {
  assert(cl);
  return GWEN_StoStorage_OpenObject(cl->storage, cl, ts, id, openFlags, po);
}



int GWEN_StoClient_CloseObject(GWEN_STO_CLIENT *cl,
                               GWEN_STO_TYPE *ts,
                               GWEN_STO_OBJECT *o,
                               GWEN_STO_CLOSEMODE cm) {
  assert(cl);
  return GWEN_StoStorage_CloseObject(cl->storage, cl, ts, o, cm);
}



int GWEN_StoClient_LockObject(GWEN_STO_CLIENT *cl,
                              GWEN_STO_TYPE *ts,
                              GWEN_TYPE_UINT32 id,
                              GWEN_STO_LOCKMODE lm) {
  assert(cl);
  return GWEN_StoStorage_LockObject(cl->storage, cl, ts, id, lm);
}













