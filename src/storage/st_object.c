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

#include "st_object_p.h"
#include "st_log_l.h"
#include "st_client_be.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>



GWEN_INHERIT_FUNCTIONS(GWEN_STO_OBJECT)
GWEN_LIST_FUNCTIONS(GWEN_STO_OBJECT, GWEN_StoObject)
GWEN_LIST2_FUNCTIONS(GWEN_STO_OBJECT, GWEN_StoObject)


GWEN_STO_OBJECT *GWEN_StoObject_new(GWEN_STO_TYPE *ty,
                                    GWEN_TYPE_UINT32 id) {
  GWEN_STO_OBJECT *o;

  GWEN_NEW_OBJECT(GWEN_STO_OBJECT, o)
  GWEN_INHERIT_INIT(GWEN_STO_OBJECT, o)
  GWEN_LIST_INIT(GWEN_STO_OBJECT, o)

  assert(ty);

  o->typ=ty;
  o->id=id;
  o->dbDataNode=GWEN_DB_Group_new("data");

  return o;
}



void GWEN_StoObject_free(GWEN_STO_OBJECT *o) {
  if (o) {
    GWEN_LIST_FINI(GWEN_STO_OBJECT, o)
    GWEN_INHERIT_FINI(GWEN_STO_OBJECT, o)
    GWEN_FREE_OBJECT(o);
  }
}



GWEN_TYPE_UINT32 GWEN_StoObject_GetFlags(const GWEN_STO_OBJECT *o) {
  assert(o);
  return o->flags;
}



void GWEN_StoObject_SetFlags(GWEN_STO_OBJECT *o, GWEN_TYPE_UINT32 fl) {
  assert(o);
  o->flags=fl;
}



void GWEN_StoObject_AddFlags(GWEN_STO_OBJECT *o, GWEN_TYPE_UINT32 fl) {
  assert(o);
  o->flags|=fl;
}



void GWEN_StoObject_SubFlags(GWEN_STO_OBJECT *o, GWEN_TYPE_UINT32 fl) {
  assert(o);
  o->flags&=~fl;
}



GWEN_DB_NODE *GWEN_StoObject_GetDbDataNode(const GWEN_STO_OBJECT *o) {
  assert(o);
  return o->dbDataNode;
}



void GWEN_StoObject_SetDbDataNode(GWEN_STO_OBJECT *o, GWEN_DB_NODE *db) {
  assert(o);
  assert(db);
  GWEN_DB_Group_free(o->dbDataNode);
  o->dbDataNode=GWEN_DB_Group_dup(db);
}



void GWEN_StoObject_SetRefCount(GWEN_STO_OBJECT *o, int i) {
  assert(o);
  if (o->refCount!=i) {
    o->refCount=i;
    o->modified=1;
  }
}



int GWEN_StoObject_IsModified(const GWEN_STO_OBJECT *o) {
  assert(o);
  return o->modified;
}



void GWEN_StoObject_SetModified(GWEN_STO_OBJECT *o, int i) {
  assert(o);
  o->modified=i;
}



int GWEN_StoObject_GetOpenCount(const GWEN_STO_OBJECT *o) {
  assert(o);
  return o->openCount;
}



int GWEN_StoObject_IncOpenCount(GWEN_STO_OBJECT *o) {
  assert(o);
  o->openCount++;
  return 0;
}



int GWEN_StoObject_DecOpenCount(GWEN_STO_OBJECT *o) {
  assert(o);
  if (o->openCount<1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Open counter of object %x is <1 (%d)",
              o->id, o->openCount);
    return GWEN_ERROR_INVALID;
  }
  o->openCount--;
  return 0;
}



void GWEN_StoObject_SetOpenCount(GWEN_STO_OBJECT *o, int i) {
  assert(o);
  o->openCount=i;
}



GWEN_TYPE_UINT32 GWEN_StoObject_GetId(const GWEN_STO_OBJECT *o) {
  assert(o);
  return o->id;
}



GWEN_STO_TYPE *GWEN_StoObject_GetType(const GWEN_STO_OBJECT *o) {
  assert(o);
  return o->typ;
}



int GWEN_StoObject_GetRefCount(const GWEN_STO_OBJECT *o) {
  assert(o);
  return o->refCount;
}



int GWEN_StoObject_IncRefCount(GWEN_STO_OBJECT *o) {
  assert(o);
  o->refCount++;
  o->modified=1;
  if (o->owner) {
    GWEN_STO_LOG *log;

    /* generate log message */
    log=GWEN_StoLog_new();
    GWEN_StoLog_SetUserName(log, GWEN_StoClient_GetUserName(o->owner));
    GWEN_StoLog_SetLogAction(log, GWEN_StoLog_ActionObjectIncRef);
    GWEN_StoLog_SetTypeBaseName(log, GWEN_StoType_GetTypeName(o->typ));
    GWEN_StoLog_SetTypeName(log, GWEN_StoType_GetName(o->typ));
    GWEN_StoLog_SetObjectId(log, GWEN_StoObject_GetId(o));
    GWEN_StoClient_AddLog(o->owner, log);
  }

  return 0;
}



int GWEN_StoObject_DecRefCount(GWEN_STO_OBJECT *o) {
  assert(o);
  if (o->refCount<1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Reference counter of object %x is <1 (%d)",
              o->id, o->refCount);
    return GWEN_ERROR_INVALID;
  }
  o->modified=1;
  o->refCount--;
  if (o->owner) {
    GWEN_STO_LOG *log;

    /* generate log message */
    log=GWEN_StoLog_new();
    GWEN_StoLog_SetUserName(log, GWEN_StoClient_GetUserName(o->owner));
    GWEN_StoLog_SetLogAction(log, GWEN_StoLog_ActionObjectDecRef);
    GWEN_StoLog_SetTypeBaseName(log, GWEN_StoType_GetTypeName(o->typ));
    GWEN_StoLog_SetTypeName(log, GWEN_StoType_GetName(o->typ));
    GWEN_StoLog_SetObjectId(log, GWEN_StoObject_GetId(o));
    GWEN_StoClient_AddLog(o->owner, log);
  }
  return 0;
}



GWEN_STO_CLIENT *GWEN_StoObject_GetLockHolder(const GWEN_STO_OBJECT *o) {
  assert(o);
  return o->lockHolder;
}



void GWEN_StoObject_SetLockHolder(GWEN_STO_OBJECT *o, GWEN_STO_CLIENT *cl) {
  assert(o);
  o->lockHolder=cl;
}



GWEN_STO_CLIENT *GWEN_StoObject_GetOwner(const GWEN_STO_OBJECT *o) {
  assert(o);
  return o->owner;
}



void GWEN_StoObject_SetOwner(GWEN_STO_OBJECT *o, GWEN_STO_CLIENT *cl) {
  assert(o);
  o->owner=cl;
}



GWEN_STO_OBJECT *GWEN_StoObject_GetOriginalObject(const GWEN_STO_OBJECT *o) {
  assert(o);
  return o->origObject;
}



void GWEN_StoObject_SetOriginalObject(GWEN_STO_OBJECT *o,
                                      GWEN_STO_OBJECT *oo) {
  assert(o);
  o->origObject=oo;
}



void GWEN_StoObject_SetCharValue(GWEN_STO_OBJECT *o,
                                 const char *varName,
                                 const char *value,
                                 int overwrite) {
  GWEN_TYPE_UINT32 flags;
  int rv;

  flags=GWEN_DB_FLAGS_DEFAULT;
  if (overwrite)
    flags|=GWEN_DB_FLAGS_OVERWRITE_VARS;
  if (value==0 && overwrite) {
    GWEN_DB_DeleteVar(o->dbDataNode, varName);
  }
  else {
    assert(value);
    rv=GWEN_DB_SetCharValue(o->dbDataNode, flags, varName, value);
    assert(rv==0);
  }
  if (o->owner) {
    GWEN_STO_LOG *log;

    /* generate log message */
    log=GWEN_StoLog_new();
    GWEN_StoLog_SetUserName(log, GWEN_StoClient_GetUserName(o->owner));
    GWEN_StoLog_SetLogAction(log, GWEN_StoLog_ActionObjectSetValue);
    GWEN_StoLog_SetTypeBaseName(log, GWEN_StoType_GetTypeName(o->typ));
    GWEN_StoLog_SetTypeName(log, GWEN_StoType_GetName(o->typ));
    GWEN_StoLog_SetObjectId(log, GWEN_StoObject_GetId(o));
    GWEN_StoLog_SetParam1(log, varName);
    GWEN_StoLog_SetParam2(log, value);
    if (overwrite)
      GWEN_StoLog_SetParam3(log, "1");
    else
      GWEN_StoLog_SetParam3(log, "0");
    GWEN_StoClient_AddLog(o->owner, log);
  }
}



const char *GWEN_StoObject_GetCharValue(const GWEN_STO_OBJECT *o,
                                        const char *varName,
                                        int idx,
                                        const char *defValue) {
  return GWEN_DB_GetCharValue(o->dbDataNode, varName, idx, defValue);
}

















