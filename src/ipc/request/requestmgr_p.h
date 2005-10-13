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


#ifndef GWEN_IPC_REQUESTMGR_P_H
#define GWEN_IPC_REQUESTMGR_P_H

#include "requestmgr.h"


struct GWEN_IPC_REQUEST_MANAGER {
  GWEN_IPCMANAGER *ipcManager;
  GWEN_IPC_REQUEST_LIST *requests;
};

int GWEN_IpcRequestManager__Work(GWEN_IPC_REQUEST_LIST *rql);


#endif /* GWEN_IPC_REQUESTMGR_P_H */

