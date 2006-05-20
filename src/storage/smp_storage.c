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

#include "smp_storage_p.h"
#include "smp_find_l.h"
#include "smp_type_be.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gwentime.h>


GWEN_INHERIT(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE)



GWEN_STO_STORAGE *GWEN_SmpStoStorage_new(const char *typeName,
                                         const char *address) {
  GWEN_STO_STORAGE *st;
  GWEN_SMPSTO_STORAGE *xst;

  st=GWEN_StoStorage_new(typeName, address);
  assert(st);

  GWEN_NEW_OBJECT(GWEN_SMPSTO_STORAGE, xst);
  GWEN_INHERIT_SETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE,
                       st, xst, GWEN_SmpStoStorage_FreeData);

  GWEN_StoStorage_SetCreateFn(st, GWEN_SmpStoStorage_Create);
  GWEN_StoStorage_SetOpenFn(st, GWEN_SmpStoStorage_Open);
  GWEN_StoStorage_SetCloseFn(st, GWEN_SmpStoStorage_Close);
  GWEN_StoStorage_SetRegisterClientFn(st, GWEN_SmpStoStorage_RegisterClient);
  GWEN_StoStorage_SetUnregisterClientFn(st,
                                        GWEN_SmpStoStorage_UnregisterClient);
  GWEN_StoStorage_SetBeginEditFn(st, GWEN_SmpStoStorage_BeginEdit);
  GWEN_StoStorage_SetEndEditFn(st, GWEN_SmpStoStorage_EndEdit);
  GWEN_StoStorage_SetCreateTypeFn(st, GWEN_SmpStoStorage_CreateType);
  GWEN_StoStorage_SetOpenTypeFn(st, GWEN_SmpStoStorage_OpenType);
  GWEN_StoStorage_SetCloseTypeFn(st, GWEN_SmpStoStorage_CloseType);
  GWEN_StoStorage_SetFindFirstObjectFn(st,
                                       GWEN_SmpStoStorage_FindFirstObject);
  GWEN_StoStorage_SetFindNextObjectFn(st,
                                      GWEN_SmpStoStorage_FindNextObject);
  GWEN_StoStorage_SetCloseFindFn(st, GWEN_SmpStoStorage_CloseFind);
  GWEN_StoStorage_SetCreateObjectFn(st, GWEN_SmpStoStorage_CreateObject);
  GWEN_StoStorage_SetOpenObjectFn(st, GWEN_SmpStoStorage_OpenObject);
  GWEN_StoStorage_SetCloseObjectFn(st, GWEN_SmpStoStorage_CloseObject);
  GWEN_StoStorage_SetDeleteObjectFn(st, GWEN_SmpStoStorage_DeleteObject);
  GWEN_StoStorage_SetLockObjectFn(st, GWEN_SmpStoStorage_LockObject);

  return st;
}



void GWEN_SmpStoStorage_FreeData(void *bp, void *p) {
  GWEN_SMPSTO_STORAGE *xst;

  xst=(GWEN_SMPSTO_STORAGE*) p;
  GWEN_FREE_OBJECT(xst);
}




/*__________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                      Setters For Virtual Functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



GWEN_SMPSTO_NEXTUNIQUEID_FN
GWEN_SmpSto_SetNextUniqueIdFn(GWEN_STO_STORAGE *st,
                              GWEN_SMPSTO_NEXTUNIQUEID_FN f) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_SMPSTO_NEXTUNIQUEID_FN f0;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  f0=xst->nextUniqueIdFn;
  xst->nextUniqueIdFn=f;
  return f0;
}



GWEN_SMPSTO_CREATEDB_FN
GWEN_SmpSto_SetCreateDbFn(GWEN_STO_STORAGE *st,
                          GWEN_SMPSTO_CREATEDB_FN f) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_SMPSTO_CREATEDB_FN f0;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  f0=xst->createDbFn;
  xst->createDbFn=f;
  return f0;
}



GWEN_SMPSTO_LOADDB_FN
GWEN_SmpSto_SetLoadDbFn(GWEN_STO_STORAGE *st,
                        GWEN_SMPSTO_LOADDB_FN f) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_SMPSTO_LOADDB_FN f0;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  f0=xst->loadDbFn;
  xst->loadDbFn=f;
  return f0;
}



GWEN_SMPSTO_WRITETYPE_FN
GWEN_SmpSto_SetWriteTypeFn(GWEN_STO_STORAGE *st,
                           GWEN_SMPSTO_WRITETYPE_FN f) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_SMPSTO_WRITETYPE_FN f0;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  f0=xst->writeTypeFn;
  xst->writeTypeFn=f;
  return f0;
}



GWEN_SMPSTO_WRITEOBJECT_FN
GWEN_SmpSto_SetWriteObjectFn(GWEN_STO_STORAGE *st,
                             GWEN_SMPSTO_WRITEOBJECT_FN f) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_SMPSTO_WRITEOBJECT_FN f0;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  f0=xst->writeObjectFn;
  xst->writeObjectFn=f;
  return f0;
}



GWEN_SMPSTO_CREATETYPE_FN
GWEN_SmpSto_SetCreateTypeFn(GWEN_STO_STORAGE *st,
                            GWEN_SMPSTO_CREATETYPE_FN f) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_SMPSTO_CREATETYPE_FN f0;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  f0=xst->createTypeFn;
  xst->createTypeFn=f;
  return f0;
}



GWEN_SMPSTO_DUPTYPE_FN
GWEN_SmpSto_SetDupTypeFn(GWEN_STO_STORAGE *st,
                         GWEN_SMPSTO_DUPTYPE_FN f) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_SMPSTO_DUPTYPE_FN f0;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  f0=xst->dupTypeFn;
  xst->dupTypeFn=f;
  return f0;
}



GWEN_SMPSTO_CREATEOBJECT_FN
GWEN_SmpSto_SetCreateObjectFn(GWEN_STO_STORAGE *st,
                              GWEN_SMPSTO_CREATEOBJECT_FN f) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_SMPSTO_CREATEOBJECT_FN f0;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  f0=xst->createObjectFn;
  xst->createObjectFn=f;
  return f0;
}



GWEN_SMPSTO_DELETEOBJECT_FN
GWEN_SmpSto_SetDeleteObjectFn(GWEN_STO_STORAGE *st,
                              GWEN_SMPSTO_DELETEOBJECT_FN f) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_SMPSTO_DELETEOBJECT_FN f0;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  f0=xst->deleteObjectFn;
  xst->deleteObjectFn=f;
  return f0;
}



GWEN_SMPSTO_READOBJECT_FN
GWEN_SmpSto_SetReadObjectFn(GWEN_STO_STORAGE *st,
                            GWEN_SMPSTO_READOBJECT_FN f) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_SMPSTO_READOBJECT_FN f0;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  f0=xst->readObjectFn;
  xst->readObjectFn=f;
  return f0;
}



GWEN_SMPSTO_ADDLOG_FN
GWEN_SmpSto_SetAddLogFn(GWEN_STO_STORAGE *st, GWEN_SMPSTO_ADDLOG_FN f) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_SMPSTO_ADDLOG_FN f0;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  f0=xst->addLogFn;
  xst->addLogFn=f;
  return f0;
}





/*__________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                         Virtual Functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



int GWEN_SmpSto_NextUniqueId(GWEN_STO_STORAGE *st,
                             GWEN_TYPE_UINT32 *pid) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  assert(pid);
  if (xst->nextUniqueIdFn)
    return xst->nextUniqueIdFn(st, pid);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_SmpSto_CreateDb(GWEN_STO_STORAGE *st) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  if (xst->createDbFn)
    return xst->createDbFn(st);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_SmpSto_LoadDb(GWEN_STO_STORAGE *st) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  if (xst->loadDbFn)
    return xst->loadDbFn(st);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_SmpSto_WriteType(GWEN_STO_STORAGE *st,
                          GWEN_STO_TYPE *ty) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  assert(ty);
  if (xst->writeTypeFn)
    return xst->writeTypeFn(st, ty);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_SmpSto_DupType(GWEN_STO_STORAGE *st,
                        const GWEN_STO_TYPE *ty,
                        GWEN_STO_TYPE **pty) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  assert(ty);
  if (xst->dupTypeFn)
    return xst->dupTypeFn(st, ty, pty);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_SmpSto_ReadObject(GWEN_STO_STORAGE *st,
                           GWEN_STO_TYPE *ty,
                           GWEN_TYPE_UINT32 id,
                           GWEN_STO_OBJECT **po) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  assert(po);
  if (xst->readObjectFn)
    return xst->readObjectFn(st, ty, id, po);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_SmpSto_WriteObject(GWEN_STO_STORAGE *st,
                            GWEN_STO_OBJECT *o) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  assert(o);
  if (xst->writeObjectFn)
    return xst->writeObjectFn(st, o);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_SmpSto_CreateType(GWEN_STO_STORAGE *st,
                           const char *typeName,
                           const char *name,
                           GWEN_STO_TYPE **pty) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  assert(pty);
  if (xst->createTypeFn)
    return xst->createTypeFn(st, typeName, name, pty);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_SmpSto_CreateObject(GWEN_STO_STORAGE *st,
                             GWEN_STO_TYPE *ty,
                             GWEN_STO_OBJECT **po) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  assert(po);
  if (xst->createObjectFn)
    return xst->createObjectFn(st, ty, po);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_SmpSto_DeleteObject(GWEN_STO_STORAGE *st,
                             GWEN_STO_OBJECT *o) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  assert(o);
  if (xst->deleteObjectFn)
    return xst->deleteObjectFn(st, o);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_SmpSto_AddLog(GWEN_STO_STORAGE *st, const GWEN_STO_LOG *log) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  assert(log);
  if (xst->addLogFn)
    return xst->addLogFn(st, log);
  return GWEN_ERROR_UNSUPPORTED;
}






/*__________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                      Reimplemented Functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



int GWEN_SmpStoStorage_Create(GWEN_STO_STORAGE *st,
                              GWEN_STO_CLIENT *cl) {
  GWEN_SMPSTO_STORAGE *xst;
  int rv;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  rv=GWEN_SmpSto_CreateDb(st);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_SmpStoStorage_Open(GWEN_STO_STORAGE *st,
                            GWEN_STO_CLIENT *cl,
                            GWEN_TYPE_UINT32 openFlags) {
  GWEN_SMPSTO_STORAGE *xst;
  int rv;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  rv=GWEN_SmpSto_LoadDb(st);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_SmpStoStorage_Close(GWEN_STO_STORAGE *st,
                             GWEN_STO_CLIENT *cl) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  /* nothing to be done here because data is written on
   * GWEN_StoStorage_CloseType and GWEN_StoStorage_CloseObject,
   * not here
   */

  return 0;
}



int GWEN_SmpStoStorage_RegisterClient(GWEN_STO_STORAGE *st,
                                      const char *userName,
                                      GWEN_STO_CLIENT **pcl) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_STO_CLIENT *cl;
  GWEN_TYPE_UINT32 clientId;
  GWEN_STO_LOG *log;
  GWEN_TIME *ti;
  GWEN_BUFFER *tbuf;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  clientId=++(xst->lastRuntimeId);
  cl=GWEN_StoClient_new(st, clientId, userName);
  *pcl=cl;

  DBG_NOTICE(GWEN_LOGDOMAIN, "Registered client [%s] (%x)",
             GWEN_StoClient_GetUserName(cl),
             GWEN_StoClient_GetId(cl));

  /* generate log message */
  log=GWEN_StoLog_new();
  GWEN_StoLog_SetUserName(log, GWEN_StoClient_GetUserName(cl));
  GWEN_StoLog_SetLogAction(log, GWEN_StoLog_ActionBeginSession);
  ti=GWEN_CurrentTime();
  assert(ti);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (GWEN_Time_toUtcString(ti, "YYYY/MM/DD-hh:mm:ss", tbuf)) {
    GWEN_Buffer_free(tbuf);
    GWEN_Time_free(ti);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_StoLog_SetParam1(log, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  GWEN_Time_free(ti);
  GWEN_StoClient_AddLog(cl, log);

  return 0;
}



int GWEN_SmpStoStorage_UnregisterClient(GWEN_STO_STORAGE *st,
                                        GWEN_STO_CLIENT *cl,
                                        const char *reason) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_STO_OBJECT *o;
  GWEN_TYPE_UINT32 oid;
  GWEN_IDLIST *idl;
  GWEN_STO_LOG *log;
  GWEN_TIME *ti;
  GWEN_BUFFER *tbuf;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  /* check for editlock */
  if (xst->lockHolder==cl) {
    DBG_NOTICE(GWEN_LOGDOMAIN,
               "Removing EditLock held by user [%s] (%x)",
               GWEN_StoClient_GetUserName(cl),
               GWEN_StoClient_GetId(cl));
    xst->lockHolder=0;
  }

  /* check for object locks */
  o=GWEN_StoObject_List_First(GWEN_StoClient_GetObjectList(cl));
  while(o) {
    if (GWEN_StoObject_GetLockHolder(o)==cl) {
      DBG_NOTICE(GWEN_LOGDOMAIN,
                 "Removing ObjectLock held by user [%s] (%x) on %x",
                 GWEN_StoClient_GetUserName(cl),
                 GWEN_StoClient_GetId(cl),
                 GWEN_StoObject_GetId(o));
      GWEN_StoObject_SetLockHolder(o, 0);
    }
    o=GWEN_StoObject_List_Next(o);
  }

  /* close objects which have been opened by this client */
  idl=GWEN_StoClient_GetOpenObjectIdList(cl);
  oid=GWEN_IdList_GetFirstId(idl);
  while(oid) {
    o=GWEN_StoStorage_FindObject(st, oid);
    if (o) {
      DBG_NOTICE(GWEN_LOGDOMAIN,
                 "Closing object held by user [%s] (%x) on %x",
                 GWEN_StoClient_GetUserName(cl),
                 GWEN_StoClient_GetId(cl),
                 oid);
      if (GWEN_SmpStoStorage__DecObjectOpenCount(st, o)) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Could not decrement open counter for object %x", oid);
      }
    }
    oid=GWEN_IdList_GetNextId(idl);
  }
  GWEN_IdList_Clear(idl);

  DBG_NOTICE(GWEN_LOGDOMAIN, "Unregistered client [%s] (%x)",
             GWEN_StoClient_GetUserName(cl),
             GWEN_StoClient_GetId(cl));

  /* generate log message */
  log=GWEN_StoLog_new();
  GWEN_StoLog_SetUserName(log, GWEN_StoClient_GetUserName(cl));
  GWEN_StoLog_SetLogAction(log, GWEN_StoLog_ActionEndSession);
  ti=GWEN_CurrentTime();
  assert(ti);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (GWEN_Time_toUtcString(ti, "YYYY/MM/DD-hh:mm:ss", tbuf)) {
    GWEN_Buffer_free(tbuf);
    GWEN_Time_free(ti);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_StoLog_SetParam1(log, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  GWEN_Time_free(ti);
  GWEN_StoLog_SetParam2(log, reason);
  GWEN_StoClient_AddLog(cl, log);

  GWEN_StoClient_free(cl);
  return 0;
}



int GWEN_SmpStoStorage_BeginEdit(GWEN_STO_STORAGE *st,
                                 GWEN_STO_CLIENT *cl) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_STO_LOG *log;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  if (xst->lockHolder!=0) {
    if (xst->lockHolder==cl) {
      DBG_ERROR(GWEN_LOGDOMAIN, "User [%s] (%x) already has EditLock",
                GWEN_StoClient_GetUserName(cl),
                GWEN_StoClient_GetId(cl));
      return GWEN_ERROR_INVALID;
    }
    DBG_DEBUG(GWEN_LOGDOMAIN, "EditLock in use");
    return GWEN_ERROR_TRY_AGAIN;
  }

  xst->lockHolder=cl;
  DBG_DEBUG(GWEN_LOGDOMAIN, "EditLock taken by client [%s] (%x)",
            GWEN_StoClient_GetUserName(cl),
            GWEN_StoClient_GetId(cl));

  /* generate log message */
  log=GWEN_StoLog_new();
  GWEN_StoLog_SetUserName(log, GWEN_StoClient_GetUserName(cl));
  GWEN_StoLog_SetLogAction(log, GWEN_StoLog_ActionBeginEdit);
  GWEN_StoClient_AddLog(cl, log);

  return 0;
}



int GWEN_SmpStoStorage_EndEdit(GWEN_STO_STORAGE *st,
                               GWEN_STO_CLIENT *cl,
                               GWEN_STO_CLOSEMODE cm) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  if (xst->lockHolder==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No user has the EditLock");
    return GWEN_ERROR_INVALID;
  }

  if (xst->lockHolder!=cl) {
    DBG_ERROR(GWEN_LOGDOMAIN, "User [%s] (%x) does not have the EditLock",
              GWEN_StoClient_GetUserName(cl),
              GWEN_StoClient_GetId(cl));
    return GWEN_ERROR_INVALID;
  }

  xst->lockHolder=0;
  DBG_DEBUG(GWEN_LOGDOMAIN, "EditLock released by client [%s] (%x)",
            GWEN_StoClient_GetUserName(cl),
            GWEN_StoClient_GetId(cl));

  if (cm==GWEN_StoCloseMode_Normal) {
    GWEN_STO_TYPE *ty;
    GWEN_STO_OBJECT *o;
    GWEN_STO_LOG *log;
    int rv;

    /* write all types from client's list */
    ty=GWEN_StoType_List_First(GWEN_StoClient_GetTypeList(cl));
    while(ty) {
      GWEN_STO_TYPE *oty;

      rv=GWEN_SmpSto_WriteType(st, ty);
      if (rv) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Could not write type [%s/%s] (%x), "
                  "database might be corrupted",
                  GWEN_StoType_GetTypeName(ty),
                  GWEN_StoType_GetName(ty),
                  GWEN_StoType_GetId(ty));
        return GWEN_ERROR_BAD_DATA;
      }
      oty=GWEN_StoType_GetOriginalType(ty);
      if (oty) {
        /* update original type */
        GWEN_StoType_SetVarList(oty, GWEN_StoType_GetVarList(ty));
      }
      ty=GWEN_StoType_List_Next(ty);
    }

    /* write all objects from client's list */
    o=GWEN_StoObject_List_First(GWEN_StoClient_GetObjectList(cl));
    while(o) {
      GWEN_STO_OBJECT *oo;

      rv=GWEN_SmpSto_WriteObject(st, o);
      if (rv) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Could not write object %x, database might be corrupted",
                  GWEN_StoObject_GetId(o));
        return GWEN_ERROR_BAD_DATA;
      }

      oo=GWEN_StoStorage_FindObject(st, GWEN_StoObject_GetId(o));
      if (oo)
        /* update cached object */
        GWEN_StoObject_SetDbDataNode(oo, GWEN_StoObject_GetDbDataNode(o));

      o=GWEN_StoObject_List_Next(o);
    }

    /* log all changes of this session */
    log=GWEN_StoLog_List_First(GWEN_StoClient_GetLogList(cl));
    while(log) {
      rv=GWEN_SmpSto_AddLog(st, log);
      if (rv) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Could not write log, database might be corrupted");
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_StoStorage_LogCallback(st, log);
      log=GWEN_StoLog_List_Next(log);
    }

    /* generate and add final log message of this edit session */
    log=GWEN_StoLog_new();
    GWEN_StoLog_SetUserName(log, GWEN_StoClient_GetUserName(cl));
    GWEN_StoLog_SetLogAction(log, GWEN_StoLog_ActionEndEdit);
    rv=GWEN_SmpSto_AddLog(st, log);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not write final log, database might be corrupted");
      return GWEN_ERROR_BAD_DATA;
    }
    GWEN_StoStorage_LogCallback(st, log);

  }
  else if (cm!=GWEN_StoCloseMode_NoUpdate) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid closeMode %d", cm);
    return GWEN_ERROR_INVALID;
  }

  GWEN_StoObject_List_Clear(GWEN_StoClient_GetObjectList(cl));
  GWEN_StoType_List_Clear(GWEN_StoClient_GetTypeList(cl));
  GWEN_StoLog_List_Clear(GWEN_StoClient_GetLogList(cl));

  return 0;
}



int GWEN_SmpStoStorage_CreateType(GWEN_STO_STORAGE *st,
                                  GWEN_STO_CLIENT *cl,
                                  const char *typeName,
                                  const char *name,
                                  GWEN_STO_TYPE **pty) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_STO_TYPE *ty;
  GWEN_STO_LOG *log;
  int rv;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  if (name==0)
    name="unnamed";

  if (xst->lockHolder!=cl) {
    DBG_ERROR(GWEN_LOGDOMAIN, "User [%s] (%x) does not have the EditLock",
              GWEN_StoClient_GetUserName(cl),
              GWEN_StoClient_GetId(cl));
    return GWEN_ERROR_INVALID;
  }

  if (GWEN_StoClient_FindTypeByName(cl, typeName, name) ||
      GWEN_StoStorage_FindTypeByName(st, typeName, name)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Type [%s/%s] already exists", typeName, name);
    return GWEN_ERROR_FOUND;
  }

  rv=GWEN_SmpSto_CreateType(st, typeName, name, &ty);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  assert(ty);
  GWEN_StoType_SetOwner(ty, cl);
  GWEN_StoType_IncOpenCount(ty);

  /* we only add the new type to the clients local list. It will finally be
   * written upon EndEdit.
   */
  GWEN_StoClient_AddType(cl, ty);
  *pty=ty;

  /* generate log message */
  log=GWEN_StoLog_new();
  GWEN_StoLog_SetUserName(log, GWEN_StoClient_GetUserName(cl));
  GWEN_StoLog_SetLogAction(log, GWEN_StoLog_ActionTypeCreate);
  GWEN_StoLog_SetTypeBaseName(log, GWEN_StoType_GetTypeName(ty));
  GWEN_StoLog_SetTypeName(log, GWEN_StoType_GetName(ty));
  GWEN_StoClient_AddLog(cl, log);

  return 0;
}



int GWEN_SmpStoStorage_OpenType(GWEN_STO_STORAGE *st,
                                GWEN_STO_CLIENT *cl,
                                const char *typeName,
                                const char *name,
                                GWEN_TYPE_UINT32 openFlags,
                                GWEN_STO_TYPE **pty) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_STO_TYPE *ty;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  if (name==0)
    name="unnamed";

  if (openFlags & GWEN_STO_OPENFLAGS_WR) {
    if (xst->lockHolder!=cl) {
      DBG_ERROR(GWEN_LOGDOMAIN, "User [%s] (%x) does not have the EditLock",
                GWEN_StoClient_GetUserName(cl),
                GWEN_StoClient_GetId(cl));
      return GWEN_ERROR_INVALID;
    }
  }

  /* first try to find type in the clients's local list */
  ty=GWEN_StoClient_FindTypeByName(cl, typeName, name);
  if (ty) {
    GWEN_StoType_IncOpenCount(ty);
    *pty=ty;
    return 0;
  }

  /* then try to find in storage-wide list */
  ty=GWEN_StoStorage_FindTypeByName(st, typeName, name);
  if (!ty) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Type [%s/%s] not found", typeName, name);
    return GWEN_ERROR_NOT_FOUND;
  }

  if (openFlags & GWEN_STO_OPENFLAGS_WR) {
    GWEN_STO_TYPE *nty;
    int rv;

    /* duplicate the type found (see below) */
    rv=GWEN_SmpSto_DupType(st, ty, &nty);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    assert(nty);

    /* link to original type */
    GWEN_StoType_IncOpenCount(ty);
    GWEN_StoType_IncOpenCount(nty);
    GWEN_StoType_SetOwner(nty, cl);
    GWEN_StoType_SetOriginalType(nty, ty);
    GWEN_StoClient_AddOpenTypeId(cl, GWEN_StoType_GetId(ty));

    /* add a copy of the real type to the client's local list. This way
     * all other clients will continue to get the unchanged version until
     * EndEdit() is called (upon which the new type will be written and all
     * its changes come into effect).
     */
    GWEN_StoClient_AddType(cl, nty);
    *pty=nty;
  }
  else {
    GWEN_StoType_IncOpenCount(ty);
    GWEN_StoClient_AddOpenTypeId(cl, GWEN_StoType_GetId(ty));
    *pty=ty;
  }

  return 0;
}



int GWEN_SmpStoStorage_CloseType(GWEN_STO_STORAGE *st,
                                 GWEN_STO_CLIENT *cl,
                                 GWEN_STO_TYPE *ty) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_STO_TYPE *oty;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  assert(cl);
  assert(ty);

  /* we do nothing here with the type except decrementing the open
   * counters. The data is really written upon EndEdit().
   */
  if (GWEN_StoType_DecOpenCount(ty)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Type [%s/%s] (%x) is not open",
              GWEN_StoType_GetTypeName(ty),
              GWEN_StoType_GetName(ty),
              GWEN_StoType_GetId(ty));
    return GWEN_ERROR_INVALID;
  }

  oty=GWEN_StoType_GetOriginalType(ty);
  if (oty) {
    if (GWEN_StoType_DecOpenCount(oty)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Original type [%s/%s] (%x) is not open",
                GWEN_StoType_GetTypeName(oty),
                GWEN_StoType_GetName(oty),
                GWEN_StoType_GetId(oty));
      return GWEN_ERROR_INVALID;
    }
  }
  else {
    GWEN_StoClient_DelOpenTypeId(cl, GWEN_StoType_GetId(ty));
  }

  return 0;
}



int GWEN_SmpStoStorage_CreateObject(GWEN_STO_STORAGE *st,
                                    GWEN_STO_CLIENT *cl,
                                    GWEN_STO_TYPE *ty,
                                    GWEN_STO_OBJECT **po) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_STO_OBJECT *o;
  int rv;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  if (xst->lockHolder!=cl) {
    DBG_ERROR(GWEN_LOGDOMAIN, "User [%s] (%x) does not have the EditLock",
              GWEN_StoClient_GetUserName(cl),
              GWEN_StoClient_GetId(cl));
    return GWEN_ERROR_INVALID;
  }

  rv=GWEN_SmpSto_CreateObject(st, ty, &o);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  GWEN_StoObject_SetOwner(o, cl);

  GWEN_StoObject_IncOpenCount(o);
  GWEN_StoClient_AddObject(cl, o);
  *po=o;

  return 0;
}



int GWEN_SmpStoStorage_OpenObject(GWEN_STO_STORAGE *st,
                                  GWEN_STO_CLIENT *cl,
                                  GWEN_STO_TYPE *ty,
                                  GWEN_TYPE_UINT32 id,
                                  GWEN_TYPE_UINT32 openFlags,
                                  GWEN_STO_OBJECT **po) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_STO_OBJECT *o;
  int rv;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  o=GWEN_StoClient_FindObject(cl, id);
  if (o) {
    GWEN_StoObject_IncOpenCount(o);
    *po=o;
    return 0;
  }

  if (openFlags & GWEN_STO_OPENFLAGS_WR) {
    /* open for writing */
    if (xst->lockHolder!=cl) {
      DBG_ERROR(GWEN_LOGDOMAIN, "User [%s] (%x) does not have the EditLock",
                GWEN_StoClient_GetUserName(cl),
                GWEN_StoClient_GetId(cl));
      return GWEN_ERROR_INVALID;
    }

    /* read object and add it to the client's list */
    rv=GWEN_SmpSto_ReadObject(st, ty, id, &o);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    GWEN_StoObject_IncOpenCount(o);
    GWEN_StoClient_AddObject(cl, o);
    GWEN_StoObject_SetOwner(o, cl);
  }
  else {
    /* open for reading */
    o=GWEN_StoStorage_FindObject(st, id);
    if (o) {
      /* already in list, inc open counter and return object */
      GWEN_StoObject_IncOpenCount(o);
      GWEN_StoClient_AddOpenObjectId(cl, id);
    }
    else {
      rv=GWEN_SmpSto_ReadObject(st, ty, id, &o);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
      GWEN_StoStorage_AddObject(st, o);
      GWEN_StoObject_IncOpenCount(o);
      GWEN_StoClient_AddOpenObjectId(cl, id);
    }
  }

  *po=o;
  return 0;
}



int GWEN_SmpStoStorage__DecObjectOpenCount(GWEN_STO_STORAGE *st,
                                           GWEN_STO_OBJECT *o) {
  GWEN_SMPSTO_STORAGE *xst;
  int rv;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  /* decrement open counter, delete object if requested to do so */
  if (GWEN_StoObject_GetOpenCount(o)==1) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Object no longer in use");
    if (GWEN_StoObject_GetFlags(o) & GWEN_STO_OBJECT_FLAGS_TO_DELETE) {
      rv=GWEN_SmpSto_DeleteObject(st, o);
      if (rv) {
        DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
    GWEN_StoObject_List_Del(o);
    GWEN_StoObject_free(o);
  }
  else {
    if (GWEN_StoObject_DecOpenCount(o)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Object %x not in use",
                GWEN_StoObject_GetId(o));
      return GWEN_ERROR_INVALID;
    }
  }

  return 0;
}



int GWEN_SmpStoStorage_CloseObject(GWEN_STO_STORAGE *st,
                                   GWEN_STO_CLIENT *cl,
                                   GWEN_STO_TYPE *ty,
                                   GWEN_STO_OBJECT *o) {
  GWEN_SMPSTO_STORAGE *xst;
  int rv;
  GWEN_TYPE_UINT32 oid;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  oid=GWEN_StoObject_GetId(o);
  if (GWEN_StoClient_FindObject(cl, oid)) {
    /* object in client's local list, nothing to do. The object will be
     * written upon EndEdit. */
    if (GWEN_StoObject_DecOpenCount(o)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Object %x not in use",
                GWEN_StoObject_GetId(o));
      return GWEN_ERROR_INVALID;
    }
  }
  else {
    if (0==GWEN_StoStorage_FindObject(st, oid)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Object %x not enlisted",
                GWEN_StoObject_GetId(o));
      return GWEN_ERROR_INVALID;
    }
    rv=GWEN_SmpStoStorage__DecObjectOpenCount(st, o);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    GWEN_StoClient_DelOpenObjectId(cl, oid);
  }
  return 0;
}



int GWEN_SmpStoStorage_DeleteObject(GWEN_STO_STORAGE *st,
                                    GWEN_STO_CLIENT *cl,
                                    GWEN_STO_TYPE *ty,
                                    GWEN_STO_OBJECT *o) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_STO_LOG *log;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  if (xst->lockHolder!=cl) {
    DBG_ERROR(GWEN_LOGDOMAIN, "User [%s] (%x) does not have the EditLock",
              GWEN_StoClient_GetUserName(cl),
              GWEN_StoClient_GetId(cl));
    return GWEN_ERROR_INVALID;
  }

  if (0==GWEN_StoClient_FindObject(cl, GWEN_StoObject_GetId(o)) &&
      0==GWEN_StoStorage_FindObject(st, GWEN_StoObject_GetId(o))) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Object %x not enlisted",
              GWEN_StoObject_GetId(o));
    return GWEN_ERROR_INVALID;
  }

  GWEN_StoObject_AddFlags(o, GWEN_STO_OBJECT_FLAGS_TO_DELETE);

  /* generate log message */
  log=GWEN_StoLog_new();
  GWEN_StoLog_SetUserName(log, GWEN_StoClient_GetUserName(cl));
  GWEN_StoLog_SetLogAction(log, GWEN_StoLog_ActionObjectDelete);
  GWEN_StoLog_SetTypeBaseName(log, GWEN_StoType_GetTypeName(ty));
  GWEN_StoLog_SetTypeName(log, GWEN_StoType_GetName(ty));
  GWEN_StoLog_SetObjectId(log, GWEN_StoObject_GetId(o));
  GWEN_StoClient_AddLog(cl, log);

  return 0;
}



int GWEN_SmpStoStorage_LockObject(GWEN_STO_STORAGE *st,
                                  GWEN_STO_CLIENT *cl,
                                  GWEN_STO_TYPE *ty,
                                  GWEN_TYPE_UINT32 id,
                                  GWEN_STO_LOCKMODE lm) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_STO_OBJECT *o;
  int rv;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  if (lm==GWEN_StoLockMode_Lock) {
    GWEN_STO_CLIENT *lockHolder;

    o=GWEN_StoStorage_FindObject(st, id);
    if (o==0) {
      /* read object and add it to the global list */
      rv=GWEN_SmpSto_ReadObject(st, ty, id, &o);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
      GWEN_StoStorage_AddObject(st, o);
    }
    assert(o);

    lockHolder=GWEN_StoObject_GetLockHolder(o);
    if (lockHolder) {
      if (lockHolder==cl) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "User [%s] (%x) already has the ObjectLock for %x",
                  GWEN_StoClient_GetUserName(cl),
                  GWEN_StoClient_GetId(cl),
                  GWEN_StoObject_GetId(o));
        return GWEN_ERROR_INVALID;
      }
      DBG_ERROR(GWEN_LOGDOMAIN, "Another user has the ObjectLock");
      return GWEN_ERROR_TRY_AGAIN;
    }
    GWEN_StoObject_SetLockHolder(o, cl);
    GWEN_StoObject_IncOpenCount(o);
    DBG_INFO(GWEN_LOGDOMAIN,
             "User [%s] (%x) acquired the ObjectLock for %x",
             GWEN_StoClient_GetUserName(cl),
             GWEN_StoClient_GetId(cl),
             GWEN_StoObject_GetId(o));
  } /* if lock */
  else if (lm==GWEN_StoLockMode_Unlock) {
    if (GWEN_StoObject_GetLockHolder(o)==cl) {
      GWEN_StoObject_SetLockHolder(o, 0);
      rv=GWEN_SmpStoStorage__DecObjectOpenCount(st, o);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_StoObject_SetLockHolder(o, cl);
        return rv;
      }
      return 0;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Another user has the ObjectLock");
      return GWEN_ERROR_INVALID;
    }
  } /* if unlock */
  else {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "User [%s] (%x): Unknown lockMode %d for object %x",
              GWEN_StoClient_GetUserName(cl),
              GWEN_StoClient_GetId(cl),
              lm,
              GWEN_StoObject_GetId(o));
    return GWEN_ERROR_INVALID;
  }

  return 0;
}



int GWEN_SmpStoStorage_FindFirstObject(GWEN_STO_STORAGE *st,
                                       GWEN_STO_CLIENT *cl,
                                       GWEN_STO_TYPE *ty,
                                       GWEN_STO_FIND **pfnd,
                                       GWEN_TYPE_UINT32 *pid) {
  GWEN_SMPSTO_STORAGE *xst;
  GWEN_STO_FIND *fnd;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  fnd=GWEN_SmpStoFind_new(0, GWEN_SmpStoType_GetObjectIdList(ty));
  *pfnd=fnd;
  *pid=GWEN_SmpStoFind_GetFirstId(fnd);
  if (*pid==0)
    return GWEN_ERROR_NOT_FOUND;
  return 0;
}



int GWEN_SmpStoStorage_FindNextObject(GWEN_STO_STORAGE *st,
                                      GWEN_STO_CLIENT *cl,
                                      GWEN_STO_TYPE *ty,
                                      GWEN_STO_FIND *fnd,
                                      GWEN_TYPE_UINT32 *pid) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  assert(fnd);
  *pid=GWEN_SmpStoFind_GetNextId(fnd);
  if (*pid==0)
    return GWEN_ERROR_NOT_FOUND;
  return 0;
}



int GWEN_SmpStoStorage_CloseFind(GWEN_STO_STORAGE *st,
                                 GWEN_STO_CLIENT *cl,
                                 GWEN_STO_TYPE *ty,
                                 GWEN_STO_FIND *fnd) {
  GWEN_SMPSTO_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, GWEN_SMPSTO_STORAGE, st);
  assert(st);

  assert(fnd);
  GWEN_StoFind_free(fnd);
  return 0;
}

















