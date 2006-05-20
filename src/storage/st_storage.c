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

#include "st_storage_p.h"
#include "st_plugin_be.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/plugin.h>

#define GWEN_STORAGE_FOLDER "storage"

#ifdef OS_WIN32
# define DIRSEP "\\"
# include <windows.h>
#else
# define DIRSEP "/"
#endif


GWEN_INHERIT_FUNCTIONS(GWEN_STO_STORAGE)




GWEN_STO_STORAGE *GWEN_StoStorage_new(const char *typeName,
                                      const char *address) {
  GWEN_STO_STORAGE *st;

  assert(typeName);
  assert(address);
  GWEN_NEW_OBJECT(GWEN_STO_STORAGE, st);
  GWEN_INHERIT_INIT(GWEN_STO_STORAGE, st);

  st->typeName=strdup(typeName);
  st->address=strdup(address);

  st->clientList=GWEN_StoClient_List_new();
  st->typeList=GWEN_StoType_List_new();
  st->objectList=GWEN_StoObject_List_new();

  return st;
}



void GWEN_StoStorage_free(GWEN_STO_STORAGE *st) {
  if (st) {
    GWEN_INHERIT_FINI(GWEN_STO_STORAGE, st);
    GWEN_StoClient_List_free(st->clientList);
    GWEN_StoType_List_free(st->typeList);
    GWEN_StoObject_List_free(st->objectList);
    free(st->address);
    free(st->typeName);
    GWEN_FREE_OBJECT(st);
  }
}



const char *GWEN_StoStorage_GetTypeName(const GWEN_STO_STORAGE *st) {
  assert(st);
  return st->typeName;
}



const char *GWEN_StoStorage_GetAddress(const GWEN_STO_STORAGE *st) {
  assert(st);
  return st->address;
}



GWEN_STO_CLIENT_LIST*
GWEN_StoStorage_GetClientList(const GWEN_STO_STORAGE *st) {
  assert(st);
  return st->clientList;
}



GWEN_STO_CLIENT *GWEN_StoStorage_FindClient(const GWEN_STO_STORAGE *st,
                                            GWEN_TYPE_UINT32 id) {
  GWEN_STO_CLIENT *cl;

  assert(st);
  cl=GWEN_StoClient_List_First(st->clientList);
  while(cl) {
    if (GWEN_StoClient_GetId(cl)==id)
      break;
    cl=GWEN_StoClient_List_Next(cl);
  }

  return cl;
}



GWEN_STO_TYPE_LIST*
GWEN_StoStorage_GetTypeList(const GWEN_STO_STORAGE *st) {
  assert(st);
  return st->typeList;
}



GWEN_STO_TYPE *GWEN_StoStorage_FindTypeById(const GWEN_STO_STORAGE *st,
                                            GWEN_TYPE_UINT32 id) {
  GWEN_STO_TYPE *ty;

  assert(st);
  ty=GWEN_StoType_List_First(st->typeList);
  while(ty) {
    if (GWEN_StoType_GetId(ty)==id)
      break;
    ty=GWEN_StoType_List_Next(ty);
  }

  return ty;
}



GWEN_STO_TYPE *GWEN_StoStorage_FindTypeByName(const GWEN_STO_STORAGE *st,
                                              const char *typeName,
                                              const char *name) {
  GWEN_STO_TYPE *ty;

  assert(st);
  assert(typeName);
  if (name==0)
    name="";

  ty=GWEN_StoType_List_First(st->typeList);
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



void GWEN_StoStorage_AddType(GWEN_STO_STORAGE *st, GWEN_STO_TYPE *ty) {
  assert(st);
  assert(ty);

  GWEN_StoType_List_Add(ty, st->typeList);
}



GWEN_STO_OBJECT_LIST*
GWEN_StoStorage_GetObjectList(const GWEN_STO_STORAGE *st) {
  assert(st);
  return st->objectList;
}



GWEN_STO_OBJECT *GWEN_StoStorage_FindObject(const GWEN_STO_STORAGE *st,
                                            GWEN_TYPE_UINT32 id) {
  GWEN_STO_OBJECT *o;

  assert(st);
  o=GWEN_StoObject_List_First(st->objectList);
  while(o) {
    if (GWEN_StoObject_GetId(o)==id)
      break;
    o=GWEN_StoObject_List_Next(o);
  }

  return o;
}



void GWEN_StoStorage_AddObject(GWEN_STO_STORAGE *st, GWEN_STO_OBJECT *o) {
  assert(st);
  GWEN_StoObject_List_Add(o, st->objectList);
}



GWEN_STO_CLOSEMODE GWEN_StoCloseMode_fromString(const char *s) {
  assert(s);
  if (strcasecmp(s, "normal")==0)
    return GWEN_StoCloseMode_Normal;
  else if (strcasecmp(s, "noupdate")==0)
    return GWEN_StoCloseMode_NoUpdate;
  else if (strcasecmp(s, "delete")==0)
    return GWEN_StoCloseMode_Delete;
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid GWEN_StoCloseMode \"%s\"", s);
    return GWEN_StoCloseMode_Unknown;
  }
}


const char *GWEN_StoCloseMode_toString(GWEN_STO_CLOSEMODE cm) {
  switch(cm) {
  case GWEN_StoCloseMode_Unknown:  return "unknown";
  case GWEN_StoCloseMode_Normal:   return "normal";
  case GWEN_StoCloseMode_NoUpdate: return "noupdate";
  case GWEN_StoCloseMode_Delete:   return "delete";
  }
  return "unknown";
}



GWEN_STO_LOCKMODE GWEN_StoLockMode_fromString(const char *s) {
  assert(s);
  if (strcasecmp(s, "lock")==0)
    return GWEN_StoLockMode_Lock;
  else if (strcasecmp(s, "unlock")==0)
    return GWEN_StoLockMode_Unlock;
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid GWEN_StoLockMode \"%s\"", s);
    return GWEN_StoLockMode_Unknown;
  }
}



const char *GWEN_StoLockMode_toString(GWEN_STO_LOCKMODE lm) {
  switch(lm) {
  case GWEN_StoLockMode_Unknown:  return "unknown";
  case GWEN_StoLockMode_Lock:     return "lock";
  case GWEN_StoLockMode_Unlock:   return "unlock";
  }
  return "unknown";
}




/*__________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                     Setters for Virtual Functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


GWEN_STO_STORAGE_CREATE_FN
GWEN_StoStorage_SetCreateFn(GWEN_STO_STORAGE *st,
                            GWEN_STO_STORAGE_CREATE_FN f) {
  GWEN_STO_STORAGE_CREATE_FN f0;

  assert(st);
  f0=st->createFn;
  st->createFn=f;
  return f0;
}



GWEN_STO_STORAGE_OPEN_FN
GWEN_StoStorage_SetOpenFn(GWEN_STO_STORAGE *st,
                          GWEN_STO_STORAGE_OPEN_FN f) {
  GWEN_STO_STORAGE_OPEN_FN f0;

  assert(st);
  f0=st->openFn;
  st->openFn=f;
  return f0;
}



GWEN_STO_STORAGE_CLOSE_FN
GWEN_StoStorage_SetCloseFn(GWEN_STO_STORAGE *st,
                           GWEN_STO_STORAGE_CLOSE_FN f) {
  GWEN_STO_STORAGE_CLOSE_FN f0;

  assert(st);
  f0=st->closeFn;
  st->closeFn=f;
  return f0;
}



GWEN_STO_STORAGE_REGISTERCLIENT_FN
GWEN_StoStorage_SetRegisterClientFn(GWEN_STO_STORAGE *st,
                                    GWEN_STO_STORAGE_REGISTERCLIENT_FN f) {
  GWEN_STO_STORAGE_REGISTERCLIENT_FN f0;

  assert(st);
  f0=st->registerClientFn;
  st->registerClientFn=f;
  return f0;
}



GWEN_STO_STORAGE_UNREGISTERCLIENT_FN
GWEN_StoStorage_SetUnregisterClientFn(GWEN_STO_STORAGE *st,
                                      GWEN_STO_STORAGE_UNREGISTERCLIENT_FN f){
  GWEN_STO_STORAGE_UNREGISTERCLIENT_FN f0;

  assert(st);
  f0=st->unregisterClientFn;
  st->unregisterClientFn=f;
  return f0;
}



GWEN_STO_STORAGE_BEGINEDIT_FN
GWEN_StoStorage_SetBeginEditFn(GWEN_STO_STORAGE *st,
                               GWEN_STO_STORAGE_BEGINEDIT_FN f) {
  GWEN_STO_STORAGE_BEGINEDIT_FN f0;

  assert(st);
  f0=st->beginEditFn;
  st->beginEditFn=f;
  return f0;
}



GWEN_STO_STORAGE_ENDEDIT_FN
GWEN_StoStorage_SetEndEditFn(GWEN_STO_STORAGE *st,
                             GWEN_STO_STORAGE_ENDEDIT_FN f) {
  GWEN_STO_STORAGE_ENDEDIT_FN f0;

  assert(st);
  f0=st->endEditFn;
  st->endEditFn=f;
  return f0;
}



GWEN_STO_STORAGE_CREATETYPE_FN
GWEN_StoStorage_SetCreateTypeFn(GWEN_STO_STORAGE *st,
                                GWEN_STO_STORAGE_CREATETYPE_FN f) {
  GWEN_STO_STORAGE_CREATETYPE_FN f0;

  assert(st);
  f0=st->createTypeFn;
  st->createTypeFn=f;
  return f0;
}



GWEN_STO_STORAGE_OPENTYPE_FN
GWEN_StoStorage_SetOpenTypeFn(GWEN_STO_STORAGE *st,
                              GWEN_STO_STORAGE_OPENTYPE_FN f) {
  GWEN_STO_STORAGE_OPENTYPE_FN f0;

  assert(st);
  f0=st->openTypeFn;
  st->openTypeFn=f;
  return f0;
}



GWEN_STO_STORAGE_CLOSETYPE_FN
GWEN_StoStorage_SetCloseTypeFn(GWEN_STO_STORAGE *st,
                               GWEN_STO_STORAGE_CLOSETYPE_FN f) {
  GWEN_STO_STORAGE_CLOSETYPE_FN f0;

  assert(st);
  f0=st->closeTypeFn;
  st->closeTypeFn=f;
  return f0;
}



GWEN_STO_STORAGE_FINDFIRSTOBJECT_FN
GWEN_StoStorage_SetFindFirstObjectFn(GWEN_STO_STORAGE *st,
                                     GWEN_STO_STORAGE_FINDFIRSTOBJECT_FN f) {
  GWEN_STO_STORAGE_FINDFIRSTOBJECT_FN f0;

  assert(st);
  f0=st->findFirstObjectFn;
  st->findFirstObjectFn=f;
  return f0;
}



GWEN_STO_STORAGE_FINDNEXTOBJECT_FN
GWEN_StoStorage_SetFindNextObjectFn(GWEN_STO_STORAGE *st,
                                    GWEN_STO_STORAGE_FINDNEXTOBJECT_FN f) {
  GWEN_STO_STORAGE_FINDNEXTOBJECT_FN f0;

  assert(st);
  f0=st->findNextObjectFn;
  st->findNextObjectFn=f;
  return f0;
}



GWEN_STO_STORAGE_CLOSEFIND_FN
GWEN_StoStorage_SetCloseFindFn(GWEN_STO_STORAGE *st,
                               GWEN_STO_STORAGE_CLOSEFIND_FN f) {
  GWEN_STO_STORAGE_CLOSEFIND_FN f0;

  assert(st);
  f0=st->closeFindFn;
  st->closeFindFn=f;
  return f0;
}



GWEN_STO_STORAGE_CREATEOBJECT_FN
GWEN_StoStorage_SetCreateObjectFn(GWEN_STO_STORAGE *st,
                                  GWEN_STO_STORAGE_CREATEOBJECT_FN f) {
  GWEN_STO_STORAGE_CREATEOBJECT_FN f0;

  assert(st);
  f0=st->createObjectFn;
  st->createObjectFn=f;
  return f0;
}



GWEN_STO_STORAGE_OPENOBJECT_FN
GWEN_StoStorage_SetOpenObjectFn(GWEN_STO_STORAGE *st,
                                GWEN_STO_STORAGE_OPENOBJECT_FN f) {
  GWEN_STO_STORAGE_OPENOBJECT_FN f0;

  assert(st);
  f0=st->openObjectFn;
  st->openObjectFn=f;
  return f0;
}



GWEN_STO_STORAGE_CLOSEOBJECT_FN
GWEN_StoStorage_SetCloseObjectFn(GWEN_STO_STORAGE *st,
                                 GWEN_STO_STORAGE_CLOSEOBJECT_FN f) {
  GWEN_STO_STORAGE_CLOSEOBJECT_FN f0;

  assert(st);
  f0=st->closeObjectFn;
  st->closeObjectFn=f;
  return f0;
}



GWEN_STO_STORAGE_DELETEOBJECT_FN
GWEN_StoStorage_SetDeleteObjectFn(GWEN_STO_STORAGE *st,
                                  GWEN_STO_STORAGE_DELETEOBJECT_FN f) {
  GWEN_STO_STORAGE_DELETEOBJECT_FN f0;

  assert(st);
  f0=st->deleteObjectFn;
  st->deleteObjectFn=f;
  return f0;
}



GWEN_STO_STORAGE_LOCKOBJECT_FN
GWEN_StoStorage_SetLockObjectFn(GWEN_STO_STORAGE *st,
                                GWEN_STO_STORAGE_LOCKOBJECT_FN f) {
  GWEN_STO_STORAGE_LOCKOBJECT_FN f0;

  assert(st);
  f0=st->lockObjectFn;
  st->lockObjectFn=f;
  return f0;
}



GWEN_STO_STORAGE_LOG_CB
GWEN_StoStorage_SetLog_Cb(GWEN_STO_STORAGE *st,
                          GWEN_STO_STORAGE_LOG_CB cb) {
  GWEN_STO_STORAGE_LOG_CB f0;

  assert(st);
  f0=st->logCb;
  st->logCb=cb;
  return f0;
}








/*__________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                         Virtual Functions
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



int GWEN_StoStorage_Create(GWEN_STO_STORAGE *st,
                           GWEN_STO_CLIENT *cl) {
  assert(st);
  assert(cl);
  if (st->createFn)
    return st->createFn(st, cl);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_Open(GWEN_STO_STORAGE *st,
                         GWEN_STO_CLIENT *cl,
                         GWEN_TYPE_UINT32 openFlags) {
  assert(st);
  assert(cl);
  if (st->openFn)
    return st->openFn(st, cl, openFlags);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_Close(GWEN_STO_STORAGE *st,
                          GWEN_STO_CLIENT *cl) {
  assert(st);
  assert(cl);
  if (st->closeFn)
    return st->closeFn(st, cl);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_RegisterClient(GWEN_STO_STORAGE *st,
                                   const char *userName,
                                   GWEN_STO_CLIENT **pcl) {
  assert(st);
  assert(pcl);
  if (st->registerClientFn)
    return st->registerClientFn(st, userName, pcl);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_UnregisterClient(GWEN_STO_STORAGE *st,
                                     GWEN_STO_CLIENT *cl,
                                     const char *reason) {
  assert(st);
  assert(cl);
  if (st->unregisterClientFn)
    return st->unregisterClientFn(st, cl, reason);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_BeginEdit(GWEN_STO_STORAGE *st,
                              GWEN_STO_CLIENT *cl) {
  assert(st);
  assert(cl);
  if (st->beginEditFn)
    return st->beginEditFn(st, cl);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_EndEdit(GWEN_STO_STORAGE *st,
                            GWEN_STO_CLIENT *cl,
                            GWEN_STO_CLOSEMODE cm) {
  assert(st);
  assert(cl);
  if (st->endEditFn)
    return st->endEditFn(st, cl, cm);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_CreateType(GWEN_STO_STORAGE *st,
                               GWEN_STO_CLIENT *cl,
                               const char *typeName,
                               const char *name,
                               GWEN_STO_TYPE **pts) {
  assert(st);
  assert(cl);
  assert(typeName);
  assert(pts);
  if (st->createTypeFn)
    return st->createTypeFn(st, cl, typeName, name, pts);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_OpenType(GWEN_STO_STORAGE *st,
                             GWEN_STO_CLIENT *cl,
                             const char *typeName,
                             const char *name,
                             GWEN_TYPE_UINT32 openFlags,
                             GWEN_STO_TYPE **pts) {
  assert(st);
  assert(cl);
  assert(typeName);
  assert(pts);
  if (st->openTypeFn)
    return st->openTypeFn(st, cl, typeName, name, openFlags, pts);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_CloseType(GWEN_STO_STORAGE *st,
                              GWEN_STO_CLIENT *cl,
                              GWEN_STO_TYPE *ts) {
  assert(st);
  assert(cl);
  assert(ts);
  if (st->closeTypeFn)
    return st->closeTypeFn(st, cl, ts);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_FindFirstObject(GWEN_STO_STORAGE *st,
                                    GWEN_STO_CLIENT *cl,
                                    GWEN_STO_TYPE *ts,
                                    GWEN_STO_FIND **pfnd,
                                    GWEN_TYPE_UINT32 *id) {
  assert(st);
  assert(cl);
  assert(ts);
  assert(pfnd);
  if (st->findFirstObjectFn)
    return st->findFirstObjectFn(st, cl, ts, pfnd, id);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_FindNextObject(GWEN_STO_STORAGE *st,
                                   GWEN_STO_CLIENT *cl,
                                   GWEN_STO_TYPE *ts,
                                   GWEN_STO_FIND *fnd,
                                   GWEN_TYPE_UINT32 *id) {
  assert(st);
  assert(cl);
  assert(ts);
  assert(fnd);
  assert(id);
  if (st->findNextObjectFn)
    return st->findNextObjectFn(st, cl, ts, fnd, id);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_CloseFind(GWEN_STO_STORAGE *st,
                              GWEN_STO_CLIENT *cl,
                              GWEN_STO_TYPE *ts,
                              GWEN_STO_FIND *fnd) {
  assert(st);
  assert(cl);
  assert(ts);
  assert(fnd);
  if (st->closeFindFn)
    return st->closeFindFn(st, cl, ts, fnd);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_CreateObject(GWEN_STO_STORAGE *st,
                                 GWEN_STO_CLIENT *cl,
                                 GWEN_STO_TYPE *ts,
                                 GWEN_STO_OBJECT **po) {
  assert(st);
  assert(cl);
  assert(ts);
  assert(po);
  if (st->createObjectFn)
    return st->createObjectFn(st, cl, ts, po);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_OpenObject(GWEN_STO_STORAGE *st,
                               GWEN_STO_CLIENT *cl,
                               GWEN_STO_TYPE *ts,
                               GWEN_TYPE_UINT32 id,
                               GWEN_TYPE_UINT32 openFlags,
                               GWEN_STO_OBJECT **po) {
  assert(st);
  assert(cl);
  assert(ts);
  assert(po);
  if (st->openObjectFn)
    return st->openObjectFn(st, cl, ts, id, openFlags, po);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_CloseObject(GWEN_STO_STORAGE *st,
                                GWEN_STO_CLIENT *cl,
                                GWEN_STO_TYPE *ts,
                                GWEN_STO_OBJECT *o) {
  assert(st);
  assert(cl);
  assert(ts);
  assert(o);
  if (st->closeObjectFn)
    return st->closeObjectFn(st, cl, ts, o);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_DeleteObject(GWEN_STO_STORAGE *st,
                                 GWEN_STO_CLIENT *cl,
                                 GWEN_STO_TYPE *ts,
                                 GWEN_STO_OBJECT *o) {
  assert(st);
  assert(cl);
  assert(ts);
  assert(o);
  if (st->deleteObjectFn)
    return st->deleteObjectFn(st, cl, ts, o);
  return GWEN_ERROR_UNSUPPORTED;
}



int GWEN_StoStorage_LockObject(GWEN_STO_STORAGE *st,
                               GWEN_STO_CLIENT *cl,
                               GWEN_STO_TYPE *ts,
                               GWEN_TYPE_UINT32 id,
                               GWEN_STO_LOCKMODE lm) {
  assert(st);
  assert(cl);
  assert(ts);
  if (st->lockObjectFn)
    return st->lockObjectFn(st, cl, ts, id, lm);
  return GWEN_ERROR_UNSUPPORTED;
}



void GWEN_StoStorage_LogCallback(GWEN_STO_STORAGE *st,
                                 const GWEN_STO_LOG *log){
  assert(st);
  assert(log);
  if (st->logCb)
    st->logCb(st, log);
}







GWEN_STO_STORAGE *GWEN_StoStorage_Factory(const char *modname,
                                          const char *addr){
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_STO_STORAGE *st;

  pm=GWEN_PluginManager_FindPluginManager("storage");
  if (!pm) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No plugin manager for \"storage\" found");
    return 0;
  }

  pl=GWEN_PluginManager_GetPlugin(pm, modname);
  if (!pl) {
    DBG_INFO(GWEN_LOGDOMAIN, "Storage-Plugin \"%s\" not found", modname);
    return 0;
  }

  st=GWEN_StoPlugin_Factory(pl, addr);
  if (!st) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Plugin did not create a storage");
  }
  return st;
}



GWEN_ERRORCODE GWEN_StoStorage_ModuleInit(){
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_BUFFER *pbuf;

  pm=GWEN_PluginManager_new("storage");
  if (GWEN_PluginManager_Register(pm)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not register storage plugin manager");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_ERROR_TYPE_ERROR,
                          GWEN_ERROR_UNSPECIFIED);
  }
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);

  if (GWEN_GetPluginPath(pbuf)) {
    GWEN_Buffer_free(pbuf);
    GWEN_PluginManager_free(pm);
    return GWEN_Error_new(0,
			  GWEN_ERROR_SEVERITY_ERR,
			  GWEN_ERROR_TYPE_ERROR,
			  GWEN_ERROR_UNSPECIFIED);
  }

  GWEN_Buffer_AppendString(pbuf, DIRSEP GWEN_STORAGE_FOLDER);
  GWEN_PluginManager_AddPath(pm, GWEN_Buffer_GetStart(pbuf));
  GWEN_Buffer_free(pbuf);

  return 0;
}



GWEN_ERRORCODE GWEN_StoStorage_ModuleFini(){
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_FindPluginManager("storage");
  if (pm) {
    int rv;

    rv=GWEN_PluginManager_Unregister(pm);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not unregister storage plugin manager (%d)", rv);
    }
    else
      GWEN_PluginManager_free(pm);
  }

  return 0;
}









