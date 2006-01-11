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


#ifndef GWEN_IPC_REQUESTMGR_H
#define GWEN_IPC_REQUESTMGR_H


typedef struct GWEN_IPC_REQUEST_MANAGER GWEN_IPC_REQUEST_MANAGER;

#include <gwenhywfar/request.h>
#include <gwenhywfar/ipc.h>

GWENHYWFAR_API 
GWEN_IPC_REQUEST_MANAGER *GWEN_IpcRequestManager_new(GWEN_IPCMANAGER *ipcMgr);

GWENHYWFAR_API 
void GWEN_IpcRequestManager_free(GWEN_IPC_REQUEST_MANAGER *rm);


GWENHYWFAR_API 
GWEN_IPCMANAGER*
  GWEN_IpcRequestManager_GetIpcManager(const GWEN_IPC_REQUEST_MANAGER *rm);
GWENHYWFAR_API 
GWEN_IPC_REQUEST_LIST*
  GWEN_IpcRequestManager_GetRequests(const GWEN_IPC_REQUEST_MANAGER *rm);


GWENHYWFAR_API 
void GWEN_IpcRequestManager_AddRequest(GWEN_IPC_REQUEST_MANAGER *rm,
                                       GWEN_IPC_REQUEST *rq);

GWENHYWFAR_API 
int GWEN_IpcRequestManager_Work(GWEN_IPC_REQUEST_MANAGER *rm);


#endif /* GWEN_IPC_REQUESTMGR_H */

