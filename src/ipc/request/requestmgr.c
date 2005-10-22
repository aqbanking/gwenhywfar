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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <strings.h>

#include "requestmgr_p.h"
#include <gwenhywfar/misc.h>




GWEN_IPC_REQUEST_MANAGER *GWEN_IpcRequestManager_new(GWEN_IPCMANAGER *ipcMgr){
  GWEN_IPC_REQUEST_MANAGER *rm;

  GWEN_NEW_OBJECT(GWEN_IPC_REQUEST_MANAGER, rm);
  rm->ipcManager=ipcMgr;
  rm->requests=GWEN_IpcRequest_List_new();

  return rm;
}



void GWEN_IpcRequestManager_free(GWEN_IPC_REQUEST_MANAGER *rm){
  if (rm) {
    GWEN_IpcRequest_List_free(rm->requests);
    GWEN_FREE_OBJECT(rm);
  }
}



GWEN_IPCMANAGER*
GWEN_IpcRequestManager_GetIpcManager(const GWEN_IPC_REQUEST_MANAGER *rm){
  assert(rm);
  return rm->ipcManager;
}



GWEN_IPC_REQUEST_LIST*
GWEN_IpcRequestManager_GetRequests(const GWEN_IPC_REQUEST_MANAGER *rm){
  assert(rm);
  return rm->requests;
}



void GWEN_IpcRequestManager_AddRequest(GWEN_IPC_REQUEST_MANAGER *rm,
                                       GWEN_IPC_REQUEST *rq){
  assert(rm);
  GWEN_IpcRequest_List_Add(rq, rm->requests);
}



int GWEN_IpcRequestManager__Work(GWEN_IPC_REQUEST_LIST *rql){
  GWEN_IPC_REQUEST *rq;
  int done=0;

  rq=GWEN_IpcRequest_List_First(rql);
  while(rq) {
    GWEN_IPC_REQUEST_LIST *crql;
    int rv;

    crql=GWEN_IpcRequest_GetSubRequests(rq);
    if (crql) {
      rv=GWEN_IpcRequestManager__Work(crql);
      if (rv!=1)
        done++;
    }

    rv=GWEN_IpcRequest_WorkFn(rq);
    if (rv!=1)
      done++;

    rq=GWEN_IpcRequest_List_Next(rq);
  }

  if (done)
    return 0;
  return 1;
}



int GWEN_IpcRequestManager_Work(GWEN_IPC_REQUEST_MANAGER *rm){
  return GWEN_IpcRequestManager__Work(rm->requests);
}







