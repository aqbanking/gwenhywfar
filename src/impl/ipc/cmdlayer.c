/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri Sep 19 2003
    copyright   : (C) 2003 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "cmdlayer_p.h"
#include <gwenhyfwar/msglayer.h>
#include <gwenhyfwar/ipc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/misc.h>




/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSGLAYERCMDDATA *GWEN_MsgLayerCmdData_new(GWEN_MSGENGINE *msgEngine){
  GWEN_IPCMSGLAYERCMDDATA *mcd;

  assert(msgEngine);
  GWEN_NEW_OBJECT(GWEN_IPCMSGLAYERCMDDATA, mcd);
  mcd->msgEngine=msgEngine;
  return mcd;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_MsgLayerCmdData_free(GWEN_IPCMSGLAYERCMDDATA *mcd){
  if (mcd) {
    free(mcd);
  }
}






/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYERCMDDATA *
GWEN_ConnectionLayerCmdData_new(GWEN_MSGENGINE *msgEngine){
  GWEN_IPCCONNLAYERCMDDATA *ccd;

  assert(msgEngine);
  GWEN_NEW_OBJECT(GWEN_IPCCONNLAYERCMDDATA, ccd);
  ccd->msgEngine=msgEngine;
  return ccd;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayerCmdData_free(GWEN_IPCCONNLAYERCMDDATA *ccd){
  if (ccd) {
    free(ccd);
  }
}




/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSGLAYER *GWEN_MsgLayerCmd_new(GWEN_MSGENGINE *msgEngine,
                                       GWEN_IPCTRANSPORTLAYER *tl,
                                       GWEN_IPCMSGLAYER_STATE st){
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCMSGLAYERCMDDATA *mcd;

  ml=GWEN_MsgLayer_new(tl, st);
  mcd=GWEN_MsgLayerCmdData_new(msgEngine);
  GWEN_MsgLayer_SetData(ml, mcd);
  GWEN_MsgLayer_SetFreeDataFn(ml, GWEN_MsgLayerCmd_free);
  GWEN_MsgLayer_SetWorkFn(ml, GWEN_MsgLayerCmd_Work);
  GWEN_MsgLayer_SetAcceptFn(ml, GWEN_MsgLayerCmd_Accept);

  return ml;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_MsgLayerCmd_free(GWEN_IPCMSGLAYER *ml){
  GWEN_IPCMSGLAYERCMDDATA *mcd;

  assert(ml);
  mcd=(GWEN_IPCMSGLAYERCMDDATA*)GWEN_MsgLayer_GetData(ml);
  assert(mcd);
  GWEN_MsgLayerCmdData_free(mcd);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_MsgLayerCmd_Work(GWEN_IPCMSGLAYER *ml,
                                     int rd){
  GWEN_IPCMSGLAYERCMDDATA *mcd;
  GWEN_IPCMSGLAYER_STATE st;
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_ERRORCODE err;

  assert(ml);
  mcd=(GWEN_IPCMSGLAYERCMDDATA*)GWEN_MsgLayer_GetData(ml);
  assert(mcd);

  tl=GWEN_MsgLayer_GetTransportLayer(ml);
  assert(tl);
  st=GWEN_MsgLayer_GetState(ml);
  switch(st) {
  case GWEN_IPCMsglayerStateIdle:
  case GWEN_IPCMsglayerStateReading:
    if (rd) {
      /* in read mode */
    }
    break;

  case GWEN_IPCMsglayerStateWriting:
    if (!rd) {
      /* in write mode */
      unsigned int bytes;
      GWEN_BUFFER *buffer;

      assert(mcd->currentMsg);
      buffer=GWEN_Msg_GetBuffer(mcd->currentMsg);
      bytes=GWEN_Buffer_BytesLeft(buffer);
      DBG_DEBUG(0, "Writing %d bytes", bytes);
      err=GWEN_IPCTransportLayer_Write(tl,
                                       GWEN_Buffer_GetPosPointer(buffer),
                                       &bytes);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO(0, "called from here");
        return err;
      }
      if (bytes==GWEN_Buffer_BytesLeft(buffer)) {
        /* message completed */
        DBG_INFO(0, "Message sending completed");
        GWEN_Msg_free(mcd->currentMsg);
        mcd->currentMsg=0;
        GWEN_MsgLayer_SetState(ml, GWEN_IPCMsglayerStateIdle);
      }
      else {
        /* message not finished, advance pointer */
        err=GWEN_Buffer_IncrementPos(buffer, bytes);
        if (!GWEN_Error_IsOk(err)) {
          DBG_INFO(0, "called from here");
          GWEN_Msg_free(mcd->currentMsg);
          mcd->currentMsg=0;
          GWEN_MsgLayer_SetState(ml, GWEN_IPCMsglayerStateIdle);
          return err;
        }
      }
    }
    break;

  case GWEN_IPCMsglayerStateConnecting:
    if (!rd) {
      /* in write mode */
    }
    break;

  case GWEN_IPCMsglayerStateWaiting:
  case GWEN_IPCMsglayerStateClosed:
  default:
    break;
  } /* switch */

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_MsgLayerCmd_Accept(GWEN_IPCMSGLAYER *ml,
                                       GWEN_IPCTRANSPORTLAYER *tl,
                                       GWEN_IPCMSGLAYER **m){
  GWEN_IPCMSGLAYER *newml;
  GWEN_IPCMSGLAYERCMDDATA *mcd;

  assert(ml);
  assert(tl);
  mcd=(GWEN_IPCMSGLAYERCMDDATA*)GWEN_MsgLayer_GetData(ml);
  assert(mcd);
  newml=GWEN_MsgLayerCmd_new(mcd->msgEngine, tl, GWEN_IPCMsglayerStateClosed);
  *m=newml;
  return 0;
}








/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYER *GWEN_ConnectionLayerCmd_new(GWEN_MSGENGINE *msgEngine,
                                               GWEN_IPCMSGLAYER *ml,
                                               GWEN_IPCCONNLAYER_STATE st){
  GWEN_IPCCONNLAYER *cl;
  GWEN_IPCCONNLAYERCMDDATA *ccd;

  cl=GWEN_ConnectionLayer_new(ml, st);
  ccd=GWEN_ConnectionLayerCmdData_new(msgEngine);
  GWEN_ConnectionLayer_SetData(cl, ccd);
  GWEN_ConnectionLayer_SetFreeDataFn(cl, GWEN_ConnectionLayerCmd_free);
  GWEN_ConnectionLayer_SetWorkFn(cl, GWEN_ConnectionLayerCmd_Work);
  GWEN_ConnectionLayer_SetAcceptFn(cl, GWEN_ConnectionLayerCmd_Accept);
  GWEN_ConnectionLayer_SetOpenFn(cl, GWEN_ConnectionLayerCmd_Open);
  GWEN_ConnectionLayer_SetCloseFn(cl, GWEN_ConnectionLayerCmd_Close);

  return cl;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayerCmd_free(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCCONNLAYERCMDDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  GWEN_ConnectionLayerCmdData_free(ccd);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Work(GWEN_IPCCONNLAYER *cl, int rd){
  GWEN_IPCCONNLAYERCMDDATA *ccd;
  GWEN_IPCCONNLAYER_STATE cst;
  GWEN_IPCMSGLAYER_STATE mst;
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_ERRORCODE err;

  assert(cl);
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  assert(ml);
  tl=GWEN_MsgLayer_GetTransportLayer(ml);
  assert(tl);
  cst=GWEN_ConnectionLayer_GetState(cl);
  mst=GWEN_MsgLayer_GetState(ml);
  switch(cst) {

  case GWEN_IPCConnectionLayerStateOpening:
    switch(ccd->securityState) {
    case GWEN_IPCCONNLAYERCMD_SECSTATE_CLOSED:
      err=GWEN_IPCTransportLayer_FinishConnect(tl);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO(0, "called from here");
        return err;
      }
      /* goto next state */
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTED;
      /* no break, we simply continue here to save work-calls */
    case GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTED:
      if (!(ccd->flags & GWEN_IPCCONNLAYERCMD_FLAGS_MUST_CRYPT)) {
        /* no encryption needed, so go directly to connected state */
        GWEN_ConnectionLayer_SetState(cl, GWEN_IPCConnectionLayerStateOpen);
        ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTED;
        break;
      }
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTED;
      /* no break, simple fall-through to save work-calls */
    case GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTING:
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_PUBLIC_KEY_EXCHG;
      /* no break, simple fall-through to save work-calls */
    case GWEN_IPCCONNLAYERCMD_SECSTATE_PUBLIC_KEY_EXCHG:
      DBG_WARN(0, "No public key exchange, encryption not implemented.");
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_SESSION_KEY_EXCHG;
      /* no break, simple fall-through to save work-calls */
    case GWEN_IPCCONNLAYERCMD_SECSTATE_SESSION_KEY_EXCHG:
      DBG_WARN(0, "No session key exchange, encryption not implemented.");
      ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_ESTABLISHED;
      /* no break, simple fall-through to save work-calls */
    case GWEN_IPCCONNLAYERCMD_SECSTATE_ESTABLISHED:
      GWEN_ConnectionLayer_SetState(cl, GWEN_IPCConnectionLayerStateOpen);
      DBG_NOTICE(0, "Connection established");
      break;

    case GWEN_IPCCONNLAYERCMD_SECSTATE_CLOSING:
      DBG_ERROR(0, "Unexpected state \"CLOSING\"");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_STATE);
    default:
      DBG_ERROR(0, "Unexpected state %d", ccd->securityState);
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_STATE);
    } /* switch securityState */
    break;

  case GWEN_IPCConnectionLayerStateOpen:
    /* connection is open, so simply let the underlying layers work */
    err=GWEN_MsgLayer_Work(ml, rd);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO(0, "called from here");
      return err;
    }
    break;

  case GWEN_IPCConnectionLayerStateClosing:
    /* TODO: implement handshaking */
    break;

  case GWEN_IPCConnectionLayerStateListening:
    if (rd) {
      /* readable on listening socket, so a new connection is available */
      GWEN_IPCCONNLAYER *newcl;

      err=GWEN_ConnectionLayer_Accept(cl, &newcl);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO(0, "called from here");
        return err;
      }
      GWEN_ConnectionLayer_Append(cl, newcl);
    }
    break;

  case GWEN_IPCConnectionLayerStateClosed:
  default:
    break;
  } /* switch */

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Accept(GWEN_IPCCONNLAYER *cl,
                                              GWEN_IPCMSGLAYER *ml,
                                              GWEN_IPCCONNLAYER **c){
  /* TODO: Copy userMark */

}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Open(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCCONNLAYERCMDDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCCONNLAYERCMDDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  if (GWEN_ConnectionLayer_GetState(cl)!=GWEN_IPCConnectionLayerStateClosed){
    DBG_ERROR(0, "Connection is not closed");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_STATE);
  }
  GWEN_ConnectionLayer_SetState(cl, GWEN_IPCConnectionLayerStateOpening);
  ccd->securityState=GWEN_IPCCONNLAYERCMD_SECSTATE_CONNECTING;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayerCmd_Close(GWEN_IPCCONNLAYER *cl,
                                             int force){
}




