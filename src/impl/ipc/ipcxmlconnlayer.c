/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 08 2003
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


#include "ipcxmlconnlayer_p.h"
#include <gwenhyfwar/ipc.h>
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/ipcxmlsecctx.h>
#include <gwenhyfwar/ipcxml.h>



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCXMLCONNLAYERDATA *GWEN_IPCXMLConnLayerData_new(){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  GWEN_NEW_OBJECT(GWEN_IPCXMLCONNLAYERDATA, ccd);
  return ccd;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayerData_free(GWEN_IPCXMLCONNLAYERDATA *ccd){
  if (ccd) {
    GWEN_CryptKey_free(ccd->localKey);
    GWEN_HBCIMsg_free(ccd->currentMsg);

    free(ccd);
  }
}





/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYER *GWEN_IPCXMLConnLayer_new(GWEN_MSGENGINE *msgEngine,
                                            GWEN_SECCTX_MANAGER *scm,
                                            GWEN_IPCMSGLAYER *ml,
                                            int active){
  GWEN_IPCCONNLAYER *cl;
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  cl=GWEN_ConnectionLayer_new(ml);
  ccd=GWEN_IPCXMLConnLayerData_new();
  ccd->msgEngine=msgEngine;
  ccd->securityManager=scm;
  ccd->dialog=GWEN_HBCIDialog_new(msgEngine, scm);
  if (active)
    GWEN_HBCIDialog_SetFlags(ccd->dialog, GWEN_HBCIDIALOG_FLAGS_INITIATOR);

  GWEN_ConnectionLayer_SetType(cl, GWEN_IPCXMLCONNLAYER_TYPE);
  GWEN_ConnectionLayer_SetData(cl, ccd);
  GWEN_ConnectionLayer_SetFreeDataFn(cl, GWEN_IPCXMLConnLayer_free);
  GWEN_ConnectionLayer_SetWorkFn(cl, GWEN_IPCXMLConnLayer_Work);
  GWEN_ConnectionLayer_SetAcceptFn(cl, GWEN_IPCXMLConnLayer_Accept);
  GWEN_ConnectionLayer_SetOpenFn(cl, GWEN_IPCXMLConnLayer_Open);
  GWEN_ConnectionLayer_SetCloseFn(cl, GWEN_IPCXMLConnLayer_Close);
  GWEN_ConnectionLayer_SetDownFn(cl, GWEN_IPCXMLConnLayer_Down);

  return cl;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_free(GWEN_IPCCONNLAYER *cl) {
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);
  GWEN_IPCXMLConnLayerData_free(ccd);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_MSGENGINE *GWEN_IPCXMLConnLayer_GetMsgEngine(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  return ccd->msgEngine;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_IPCXMLConnLayer_GetFlags(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  return ccd->flags;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_SetFlags(GWEN_IPCCONNLAYER *cl,
                                   unsigned int f){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  ccd->flags=f;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_CRYPTKEY *GWEN_IPCXMLConnLayer_GetLocalKey(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  return ccd->localKey;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_SetLocalKey(GWEN_IPCCONNLAYER *cl,
                                      GWEN_CRYPTKEY *k){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  GWEN_CryptKey_free(ccd->localKey);
  ccd->localKey=k;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Accept(GWEN_IPCCONNLAYER *cl,
                                           GWEN_IPCMSGLAYER *ml,
                                           GWEN_IPCCONNLAYER **c){
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_IPCXMLCONNLAYERDATA *newccd;
  GWEN_IPCCONNLAYER *newcl;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  newcl=GWEN_IPCXMLConnLayer_new(ccd->msgEngine, ccd->securityManager, ml, 0);
  DBG_INFO(0, "Created new GWEN_IPCCONNLAYER for incoming connection");
  newccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(newcl);
  assert(newccd);

  /* copy important data */
  newccd->flags=ccd->flags;
  if (ccd->localKey)
    newccd->localKey=GWEN_CryptKey_dup(ccd->localKey);

  GWEN_IPCXMLConnLayer_Up(newcl);

  *c=newcl;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Open(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_IPCMSGLAYER *ml;
  GWEN_ERRORCODE err;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  assert(ml);

  DBG_INFO(0, "Starting to connect %d",
           GWEN_ConnectionLayer_GetId(cl));
  err=GWEN_MsgLayer_Connect(ml);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    return err;
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Close(GWEN_IPCCONNLAYER *cl,
                                          int force){
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_IPCMSGLAYER *ml;
  GWEN_ERRORCODE err;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  assert(ml);
  err=GWEN_MsgLayer_Disconnect(ml);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    return err;
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Up(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_IPCMSGLAYER *ml;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  ccd->connected=1;
  ccd->dialogId++;
  GWEN_HBCIDialog_Reset(ccd->dialog);

  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  assert(ml);
  GWEN_MsgLayer_SetState(ml, GWEN_IPCMsglayerStateIdle);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_Down(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  if (ccd->connected) {
    ccd->connected=0;
    GWEN_HBCIDialog_Reset(ccd->dialog);
  }
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Work(GWEN_IPCCONNLAYER *cl, int rd){
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_IPCMSGLAYER_STATE mst;
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_ERRORCODE err;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
  assert(ml);
  tl=GWEN_MsgLayer_GetTransportLayer(ml);
  assert(tl);

  /* let the underlying layers work */
  err=GWEN_MsgLayer_Work(ml, rd);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    return err;
  }

  mst=GWEN_MsgLayer_GetState(ml);
  if (mst==GWEN_IPCMsglayerStateConnecting) {
    err=GWEN_IPCTransportLayer_FinishConnect(tl);
    if (!GWEN_Error_IsOk(err)) {
      if (GWEN_Error_GetType(err)!=
          GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
          GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_IN_PROGRESS) {
        /* real error, so return that error */
        DBG_INFO(0, "called from here");
        return err;
      }
      /* otherwise connecting in progress, return */
      /* TODO: timeout */
      return 0;
    }
    /* goto next state */
    DBG_INFO(0, "Physically connected");
    GWEN_IPCXMLConnLayer_Up(cl);
  }



  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_HBCIMSG *GWEN_IPCXMLConnLayer_IPC2HBCI(GWEN_IPCCONNLAYER *cl,
                                            GWEN_IPCMSG *msg){
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_HBCIMSG *hmsg;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  hmsg=GWEN_HBCIMsg_new(ccd->dialog);
  /* copy data from IPC message to HBCI message */
  GWEN_HBCIMsg_SetBuffer(hmsg, GWEN_Msg_TakeBuffer(msg));
  GWEN_HBCIMsg_SetMsgLayerId(hmsg, GWEN_ConnectionLayer_GetId(cl));

  return hmsg;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_IPCXMLConnLayer_HBCI2IPC(GWEN_IPCCONNLAYER *cl,
                                           GWEN_HBCIMSG *hmsg){
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_IPCMSG *msg;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  msg=GWEN_Msg_new();
  /* copy data from IPC message to HBCI message */
  GWEN_Msg_SetBuffer(msg, GWEN_HBCIMsg_TakeBuffer(hmsg));
  GWEN_Msg_SetMsgLayerId(msg, GWEN_HBCIMsg_GetMsgLayerId(hmsg));

  return msg;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_HBCIMSG *GWEN_IPCXMLConnLayer_MsgFactory(GWEN_IPCCONNLAYER *cl) {
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_HBCIMSG *hmsg;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  hmsg=GWEN_HBCIMsg_new(ccd->dialog);
  GWEN_HBCIMsg_SetMsgLayerId(hmsg, GWEN_ConnectionLayer_GetId(cl));

  return hmsg;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCXMLConnLayer_SetSecurityEnv(GWEN_IPCCONNLAYER *cl,
                                    const GWEN_KEYSPEC *signer,
                                    const GWEN_KEYSPEC *crypter){
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_ERRORCODE err;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  GWEN_KeySpec_free(ccd->signer);
  if (signer)
    ccd->signer=GWEN_KeySpec_dup(signer);
  else
    ccd->signer=0;

  GWEN_KeySpec_free(ccd->crypter);
  if (crypter)
    ccd->crypter=GWEN_KeySpec_dup(crypter);
  else
    ccd->crypter=0;

  err=GWEN_IPCXMLConnLayer_Flush(cl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_Flush(GWEN_IPCCONNLAYER *cl) {
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_ERRORCODE err;

  DBG_DEBUG(0, "Function called");
  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  if (ccd->currentMsg) {
    if (GWEN_HBCIMsg_GetNodes(ccd->currentMsg)) {
      GWEN_IPCMSG *msg;
      GWEN_IPCMSGLAYER *ml;

      ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
      assert(ml);

      if (!GWEN_MsgLayer_CheckAddOutgoingMsg(ml)) {
        DBG_WARN(0, "Can not relay message to messageLayer");
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                              GWEN_IPC_ERROR_OUTQUEUE_FULL);
      }

      DBG_INFO(0, "Encoding message");
      err=GWEN_HBCIMsg_EncodeMsg(ccd->currentMsg);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO_ERR(0, err);
        return err;
      }
      DBG_INFO(0, "Transforming message for message layer");
      msg=GWEN_IPCXMLConnLayer_HBCI2IPC(cl, ccd->currentMsg);
      assert(msg);
      DBG_INFO(0, "Enqueing message to message layer");
      err=GWEN_MsgLayer_AddOutgoingMsg(ml, msg);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO_ERR(0, err);
        GWEN_HBCIMsg_free(ccd->currentMsg);
        ccd->currentMsg=0;
        return err;
      }
      DBG_NOTICE(0, "Message successfully enqueued");
    } /* if message is not empty */
    else {
      DBG_INFO(0, "Message is empty, nothing to flush");
    }
    GWEN_HBCIMsg_free(ccd->currentMsg);
    ccd->currentMsg=0;
  } /* if there is a current message */
  else {
    DBG_INFO(0, "No message, nothing to flush");
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCXMLREQUEST *GWEN_IPCXMLConnLayer_AddRequest(GWEN_IPCCONNLAYER *cl,
						    GWEN_XMLNODE *node,
						    GWEN_DB_NODE *db,
						    int flush) {
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_ERRORCODE err;
  GWEN_IPCXMLREQUEST *rq;
  unsigned int segnum;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  /* check whether we have an open connection */
  if (!ccd->connected) {
    GWEN_IPCMSGLAYER *ml;

    ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
    assert(ml);
    if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateUnconnected) {
      /* msglayer is unconnected, shall we try to auto-connect ? */
      if (!(GWEN_ConnectionLayer_GetFlags(cl) &
	    GWEN_IPCCONNLAYER_FLAGS_PASSIVE)){
	/* active connection, so try it */
	DBG_NOTICE(0, "Attempting auto-connect to server");
	err=GWEN_ConnectionLayer_Open(cl);
	if (!GWEN_Error_IsOk(err)) {
	  DBG_INFO_ERR(0, err);
	  return 0;
	}
	/* open succeeded, auto-connect now in progress */
	DBG_INFO(0, "Auto-connect now in progress");
      } /* if !passive */
      else {
	/* passive connection, we can not auto-connect */
	DBG_ERROR(0, "Connection not open");
	return 0;
      }
    } /* if unconnected */
    else {
      /* some other status... */
      if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateConnecting) {
	DBG_INFO(0, "Auto-connect already in progress");
      }
      else {
	/* bad status */
	DBG_ERROR(0,
		  "Status of MsgLayer does not allow sending (%s)",
		  GWEN_MsgLayer_GetStateString(GWEN_MsgLayer_GetState(ml)));
	return 0;
      }
    }
  } /* if !connected */

  /* create message if there is none */
  if (!ccd->currentMsg) {
    ccd->currentMsg=GWEN_IPCXMLConnLayer_MsgFactory(cl);
    if (!ccd->currentMsg) {
      DBG_ERROR(0, "Could not create message");
      return 0;
    }
  }

  /* add node to message */
  segnum=GWEN_HBCIMsg_AddNode(ccd->currentMsg, node, db);
  if (segnum==0) {
    DBG_INFO(0, "Could not add node");
    return 0;
  }

  if (flush) {
    /* directly encode and enqueue the message if requested */
    err=GWEN_IPCXMLConnLayer_Flush(cl);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(0, err);
      return 0;
    }
  }

  /* create request, fill it */
  rq=GWEN_IPCXMLRequest_new();
  GWEN_IPCXMLRequest_SetMsgLayerId(rq, GWEN_ConnectionLayer_GetId(cl));
  GWEN_IPCXMLRequest_SetDialogId(rq, ccd->dialogId);
  GWEN_IPCXMLRequest_SetMessageNumber(rq,
    GWEN_HBCIMsg_GetMsgNumber(ccd->currentMsg));
  GWEN_IPCXMLRequest_SetSegmentNumber(rq, segnum);

  /* finished */
  return rq;
}






