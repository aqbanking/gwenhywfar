/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri May 07 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_IPC_H
#define GWEN_IPC_H


#include <gwenhywfar/types.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/nettransport.h>

#include <time.h>



typedef struct GWEN_IPCMANAGER GWEN_IPCMANAGER;


GWEN_IPCMANAGER *GWEN_IPCManager_new();
void GWEN_IPCManager_free(GWEN_IPCMANAGER *mgr);


GWEN_TYPE_UINT32 GWEN_IPCManager_AddServer(GWEN_IPCMANAGER *mgr,
                                           GWEN_NETTRANSPORT *tr,
                                           GWEN_TYPE_UINT32 mark);

GWEN_TYPE_UINT32 GWEN_IPCManager_AddClient(GWEN_IPCMANAGER *mgr,
                                           GWEN_NETTRANSPORT *tr,
                                           const char *userName,
                                           const char *passwd,
                                           GWEN_TYPE_UINT32 mark);


GWEN_TYPE_UINT32 GWEN_IPCManager_SendRequest(GWEN_IPCMANAGER *mgr,
                                             GWEN_TYPE_UINT32 nid,
                                             GWEN_DB_NODE *req);

GWEN_TYPE_UINT32 GWEN_IPCManager_SendMultiRequest(GWEN_IPCMANAGER *mgr,
                                                  GWEN_TYPE_UINT32 mark,
                                                  GWEN_DB_NODE *req);

int GWEN_IPCManager_SendResponse(GWEN_IPCMANAGER *mgr,
                                 GWEN_TYPE_UINT32 rid,
                                 GWEN_DB_NODE *rsp);

int GWEN_IPCManager_RemoveRequest(GWEN_IPCMANAGER *mgr,
                                  GWEN_TYPE_UINT32 rid,
                                  int inOrOut);

GWEN_TYPE_UINT32 GWEN_IPCManager_GetNextInRequest(GWEN_IPCMANAGER *mgr,
                                                  GWEN_TYPE_UINT32 mark);

GWEN_DB_NODE *GWEN_IPCManager_GetInRequestData(GWEN_IPCMANAGER *mgr,
                                               GWEN_TYPE_UINT32 rid);

GWEN_DB_NODE *GWEN_IPCManager_GetResponseData(GWEN_IPCMANAGER *mgr,
                                              GWEN_TYPE_UINT32 rid);


#endif /* GWEN_IPC_H */


