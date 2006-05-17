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

#include "st_type_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

GWEN_INHERIT_FUNCTIONS(GWEN_STO_TYPE)
GWEN_LIST_FUNCTIONS(GWEN_STO_TYPE, GWEN_StoType)
GWEN_LIST2_FUNCTIONS(GWEN_STO_TYPE, GWEN_StoType)




GWEN_STO_TYPE *GWEN_StoType_new(GWEN_STO_STORAGE *storage,
                                GWEN_TYPE_UINT32 id,
                                const char *typeName,
                                const char *name) {
  GWEN_STO_TYPE *ty;

  GWEN_NEW_OBJECT(GWEN_STO_TYPE, ty);
  GWEN_INHERIT_INIT(GWEN_STO_TYPE, ty);
  GWEN_LIST_INIT(GWEN_STO_TYPE, ty);

  ty->storage=storage;
  ty->id=id;
  ty->typeName=strdup(typeName);
  if (name)
    ty->name=strdup(name);
  ty->varList=GWEN_StoVarDef_List_new();

  return ty;
}



GWEN_STO_TYPE *GWEN_StoType_dup(const GWEN_STO_TYPE *oty) {
  GWEN_STO_TYPE *ty;

  ty=GWEN_StoType_new(oty->storage,
                      oty->id,
                      oty->typeName,
                      oty->name);
  GWEN_StoVarDef_List_free(ty->varList);
  ty->varList=GWEN_StoVarDef_List_dup(oty->varList);

  return ty;
}



void GWEN_StoType_free(GWEN_STO_TYPE *ty) {
  if (ty) {
    GWEN_LIST_FINI(GWEN_STO_TYPE, ty);
    GWEN_INHERIT_FINI(GWEN_STO_TYPE, ty);
    GWEN_StoVarDef_List_free(ty->varList);
    free(ty->name);
    free(ty->typeName);
    GWEN_FREE_OBJECT(ty);
  }
}



GWEN_STO_STORAGE *GWEN_StoType_GetStorage(const GWEN_STO_TYPE *ty) {
  assert(ty);
  return ty->storage;
}



GWEN_TYPE_UINT32 GWEN_StoType_GetId(const GWEN_STO_TYPE *ty) {
  assert(ty);
  return ty->id;
}



const char *GWEN_StoType_GetTypeName(const GWEN_STO_TYPE *ty) {
  assert(ty);
  return ty->typeName;
}



const char *GWEN_StoType_GetName(const GWEN_STO_TYPE *ty) {
  assert(ty);
  return ty->name;
}



int GWEN_StoType_GetOpenCount(const GWEN_STO_TYPE *ty) {
  assert(ty);
  return ty->openCount;
}



int GWEN_StoType_IncOpenCount(GWEN_STO_TYPE *ty) {
  assert(ty);
  ty->openCount++;
  return 0;
}



int GWEN_StoType_DecOpenCount(GWEN_STO_TYPE *ty) {
  assert(ty);
  if (ty->openCount<1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Open counter of type [%s] is <1 (%d)",
              ty->typeName, ty->openCount);
    return GWEN_ERROR_INVALID;
  }
  ty->openCount--;
  return 0;
}



void GWEN_StoType_SetOpenCount(GWEN_STO_TYPE *ty, int i) {
  assert(ty);
  ty->openCount=i;
}



int GWEN_StoType_IsModified(const GWEN_STO_TYPE *ty) {
  assert(ty);
  return ty->modified;
}



void GWEN_StoType_SetModified(GWEN_STO_TYPE *ty, int i) {
  assert(ty);
  ty->modified=i;
}



const GWEN_STO_VARDEF_LIST *GWEN_StoType_GetVarList(const GWEN_STO_TYPE *ty) {
  assert(ty);
  return ty->varList;
}



int GWEN_StoType_AddVarNoLog(GWEN_STO_TYPE *ty, GWEN_STO_VARDEF *vdef) {
  assert(ty);
  assert(vdef);
  GWEN_StoVarDef_List_Add(vdef, ty->varList);

  if (ty->owner) {
    GWEN_STO_LOG *log;
    char numbuf[32];

    /* generate log message */
    log=GWEN_StoLog_new();
    GWEN_StoLog_SetUserName(log, GWEN_StoClient_GetUserName(ty->owner));
    GWEN_StoLog_SetLogAction(log, GWEN_StoLog_ActionTypeAddVar);
    GWEN_StoLog_SetTypeBaseName(log, GWEN_StoType_GetTypeName(ty));
    GWEN_StoLog_SetTypeName(log, GWEN_StoType_GetName(ty));
    GWEN_StoLog_SetParam1(log, GWEN_StoVarDef_GetName(vdef));
    snprintf(numbuf, sizeof(numbuf), "%d", GWEN_StoVarDef_GetMinNum(vdef));
    GWEN_StoLog_SetParam2(log, numbuf);
    snprintf(numbuf, sizeof(numbuf), "%d", GWEN_StoVarDef_GetMaxNum(vdef));
    GWEN_StoLog_SetParam3(log, numbuf);

    GWEN_StoClient_AddLog(ty->owner, log);
  }

  return 0;
}



int GWEN_StoType_AddVar(GWEN_STO_TYPE *ty, const GWEN_STO_VARDEF *vdef){
  int rv;

  rv=GWEN_StoType_AddVarNoLog(ty, GWEN_StoVarDef_dup(vdef));
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here(%d)", rv);
    return rv;
  }

  /* TODO: Log */
  return 0;
}



void GWEN_StoType_SetVarList(GWEN_STO_TYPE *ty,
                             const GWEN_STO_VARDEF_LIST *vl) {
  assert(ty);
  assert(vl);
  GWEN_StoVarDef_List_free(ty->varList);
  ty->varList=GWEN_StoVarDef_List_dup(vl);
}



GWEN_STO_TYPE *GWEN_StoType_GetOriginalType(const GWEN_STO_TYPE *ty) {
  assert(ty);
  return ty->origType;
}



void GWEN_StoType_SetOriginalType(GWEN_STO_TYPE *ty, GWEN_STO_TYPE *oty) {
  assert(ty);
  ty->origType=oty;
}



GWEN_STO_CLIENT *GWEN_StoType_GetOwner(const GWEN_STO_TYPE *ty) {
  assert(ty);
  return ty->owner;
}



void GWEN_StoType_SetOwner(GWEN_STO_TYPE *ty, GWEN_STO_CLIENT *cl) {
  assert(ty);
  ty->owner=cl;
}














