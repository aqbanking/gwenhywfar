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
#include <gwenhywfar/ipc.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/ipcxmlsecctx.h>
#include <gwenhywfar/ipcxml.h>



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCXMLCONNLAYERDATA *GWEN_IPCXMLConnLayerData_new(){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  GWEN_NEW_OBJECT(GWEN_IPCXMLCONNLAYERDATA, ccd);
  return ccd;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayerData_free(GWEN_IPCXMLCONNLAYERDATA *ccd){
  if (ccd) {
    GWEN_HBCIMsg_free(ccd->currentMsg);
    free(ccd);
  }
}





/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYER *GWEN_IPCXMLConnLayer_new(GWEN_SERVICELAYER *sl,
                                            GWEN_MSGENGINE *msgEngine,
                                            const char *localName,
                                            GWEN_SECCTX_MANAGER *scm,
                                            GWEN_IPCMSGLAYER *ml,
                                            int active){
  GWEN_IPCCONNLAYER *cl;
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(sl);
  assert(msgEngine);
  assert(localName);
  assert(scm);
  assert(ml);

  cl=GWEN_ConnectionLayer_new(ml);
  ccd=GWEN_IPCXMLConnLayerData_new();
  ccd->serviceLayer=sl;
  ccd->msgEngine=msgEngine;
  ccd->securityManager=scm;
  ccd->dialog=GWEN_HBCIDialog_new(msgEngine, scm);
  ccd->dialogId=1;
  GWEN_HBCIDialog_SetLocalName(ccd->dialog, localName);
  if (active)
    GWEN_HBCIDialog_SetFlags(ccd->dialog, GWEN_HBCIDIALOG_FLAGS_INITIATOR);

  GWEN_ConnectionLayer_SetType(cl, GWEN_IPCXMLCONNLAYER_TYPE);
  GWEN_ConnectionLayer_SetData(cl, ccd);
  GWEN_ConnectionLayer_SetFreeDataFn(cl, GWEN_IPCXMLConnLayer_free);
  GWEN_ConnectionLayer_SetWorkFn(cl, GWEN_IPCXMLConnLayer_Work);
  GWEN_ConnectionLayer_SetAcceptFn(cl, GWEN_IPCXMLConnLayer_Accept);
  GWEN_ConnectionLayer_SetOpenFn(cl, GWEN_IPCXMLConnLayer_Open);
  GWEN_ConnectionLayer_SetCloseFn(cl, GWEN_IPCXMLConnLayer_Close);
  GWEN_ConnectionLayer_SetUpFn(cl, GWEN_IPCXMLConnLayer_Up);
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

  newcl=GWEN_IPCXMLConnLayer_new(ccd->serviceLayer,
                                 ccd->msgEngine,
                                 GWEN_HBCIDialog_GetLocalName(ccd->dialog),
                                 ccd->securityManager, ml, 0);
  DBG_INFO(0, "Created new GWEN_IPCCONNLAYER for incoming connection");
  newccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(newcl);
  assert(newccd);

  /* copy important data */
  newccd->flags=ccd->flags;
  newccd->connectedFn=ccd->connectedFn;
  newccd->disconnectedFn=ccd->disconnectedFn;

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
void GWEN_IPCXMLConnLayer_Up(GWEN_IPCCONNLAYER *cl){
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
  /* remove all incoming messages on this connection, because the security
   * context might change now */
  GWEN_MsgLayer_ClearIncomingMsg(ml);
  GWEN_MsgLayer_SetState(ml, GWEN_IPCMsglayerStateIdle);
  /* notify application */
  GWEN_IPCXMLConnLayer_Connected(cl);
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_Down(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  if (ccd->connected) {
    /* notify application */
    GWEN_IPCXMLConnLayer_Disconnected(cl);
    ccd->connected=0;
    /*GWEN_HBCIDialog_Reset(ccd->dialog);*/
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
        /* TODO: try again to connect after a certain timeout */
        GWEN_MsgLayer_SetState(ml, GWEN_IPCMsglayerStateUnconnected);
        return err;
      }
      /* otherwise connecting in progress, return */
      /* TODO: timeout */
      return 0;
    }
    /* goto next state */
    DBG_INFO(0, "Physically connected");
    GWEN_ConnectionLayer_Up(cl);
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
  GWEN_HBCIMsg_SetDialogNumber(hmsg, ccd->dialogId);
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

  GWEN_HBCIMsg_SetFlags(hmsg, ccd->msgFlags);

  if (ccd->msgFlags & GWEN_HBCIMSG_FLAGS_SIGN) {
    /* add signer */
    GWEN_KEYSPEC *ks;

    ks=GWEN_KeySpec_new();
    GWEN_KeySpec_SetOwner(ks, GWEN_HBCIDialog_GetLocalName(ccd->dialog));
    GWEN_HBCIMsg_AddSigner(hmsg, ks);
    GWEN_KeySpec_free(ks);
  }

  if (ccd->msgFlags & GWEN_HBCIMSG_FLAGS_CRYPT) {
    /* add crypt */
    const char *p;

    p=GWEN_HBCIDialog_GetRemoteName(ccd->dialog);
    if (p) {
      GWEN_KEYSPEC *ks;

      ks=GWEN_KeySpec_new();
      GWEN_KeySpec_SetOwner(ks, p);
      GWEN_HBCIMsg_SetCrypter(hmsg, ks);
      GWEN_KeySpec_free(ks);
    }
    else {
      DBG_ERROR(0, "Encryption requested but no remote name");
      GWEN_HBCIMsg_free(hmsg);
      return 0;
    }
  }

  return hmsg;
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
                                                    unsigned int flags) {
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

  DBG_INFO(0, "Connecting should be handled now");
  /* create message if there is none */
  if (!ccd->currentMsg) {
    DBG_INFO(0, "Creating new message");
    ccd->currentMsg=GWEN_IPCXMLConnLayer_MsgFactory(cl);
    if (!ccd->currentMsg) {
      DBG_ERROR(0, "Could not create message");
      return 0;
    }
  }

  /* add node to message */
  DBG_INFO(0, "Adding node");
  segnum=GWEN_HBCIMsg_AddNode(ccd->currentMsg, node, db);
  if (segnum==0) {
    DBG_INFO(0, "Could not add node");
    return 0;
  }
  DBG_INFO(0, "Node added as segment %d", segnum);
  GWEN_Buffer_Dump(GWEN_HBCIMsg_GetBuffer(ccd->currentMsg), stderr, 2);

  /* create request, fill it */
  DBG_INFO(0, "Creating new request");
  rq=GWEN_IPCXMLRequest_new();
  GWEN_IPCXMLRequest_SetMsgLayerId(rq, GWEN_ConnectionLayer_GetId(cl));
  GWEN_IPCXMLRequest_SetDialogId(rq, ccd->dialogId);
  GWEN_IPCXMLRequest_SetMessageNumber(rq,
     GWEN_HBCIMsg_GetMsgNumber(ccd->currentMsg));
  GWEN_IPCXMLRequest_SetSegmentNumber(rq, segnum);

  if (flags & GWEN_IPCXML_REQUESTFLAGS_FLUSH) {
    DBG_INFO(0, "Flushing message");
    /* directly encode and enqueue the message if requested */
    err=GWEN_IPCXMLConnLayer_Flush(cl);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(0, err);
      GWEN_IPCXMLRequest_free(rq);
      return 0;
    }
  }

  /* finished */
  return rq;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_AddResponse(GWEN_IPCCONNLAYER *cl,
                                                GWEN_IPCXMLREQUEST *rq,
                                                GWEN_XMLNODE *node,
                                                GWEN_DB_NODE *db,
                                                unsigned int flags) {
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_ERRORCODE err;
  unsigned int segnum;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  /* check whether we have an open connection */
  if (!ccd->connected) {
    DBG_ERROR(0, "Connection not open, no response possible");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_STATE);
  } /* if !connected */

  if (ccd->currentMsg) {
    /* there already is a message, check whether it belongs to the same
     * request */
    if ((GWEN_HBCIMsg_GetMsgRef(ccd->currentMsg)!=
         GWEN_IPCXMLRequest_GetMessageNumber(rq)) ||
        (GWEN_HBCIMsg_GetDialogNumber(ccd->currentMsg)!=
         GWEN_IPCXMLRequest_GetDialogId(rq))) {
      DBG_INFO(0, "Different message, flushing");
      /* directly encode and enqueue the message if requested */
      err=GWEN_IPCXMLConnLayer_Flush(cl);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO_ERR(0, err);
        return err;
      }
    }
  }

  /* create message if there is none */
  if (!ccd->currentMsg) {
    DBG_INFO(0, "Creating new message");
    ccd->currentMsg=GWEN_IPCXMLConnLayer_MsgFactory(cl);
    if (!ccd->currentMsg) {
      DBG_ERROR(0, "Could not create message");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_INTERNAL);
    }
    /* store message reference number */
    GWEN_HBCIMsg_SetMsgNumber(ccd->currentMsg,
                              GWEN_IPCXMLRequest_GetMessageNumber(rq));
  }

  /* store reference segment number */
  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "head/ref",
                      GWEN_IPCXMLRequest_GetSegmentNumber(rq));

  /* add node to message */
  DBG_INFO(0, "Adding node");
  segnum=GWEN_HBCIMsg_AddNode(ccd->currentMsg, node, db);
  if (segnum==0) {
    DBG_INFO(0, "Could not add node");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_INVALID);
  }

  DBG_INFO(0, "Node added as segment %d", segnum);
  GWEN_Buffer_Dump(GWEN_HBCIMsg_GetBuffer(ccd->currentMsg), stderr, 2);

  if (flags & GWEN_IPCXML_REQUESTFLAGS_FLUSH) {
    DBG_INFO(0, "Flushing message");
    /* directly encode and enqueue the message if requested */
    err=GWEN_IPCXMLConnLayer_Flush(cl);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(0, err);
      GWEN_IPCXMLRequest_free(rq);
      return err;
    }
  }

  /* finished */
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_IPCXMLConnLayer_GetLocalName(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  return GWEN_HBCIDialog_GetLocalName(ccd->dialog);
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_IPCXMLConnLayer_GetRemoteName(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  return GWEN_HBCIDialog_GetRemoteName(ccd->dialog);
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_SetRemoteName(GWEN_IPCCONNLAYER *cl,
                                        const char *s){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  GWEN_HBCIDialog_SetRemoteName(ccd->dialog, s);
}




/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCXMLConnLayer_SetSecurityFlags(GWEN_IPCCONNLAYER *cl,
                                                     unsigned int flags){
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_ERRORCODE err;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  err=GWEN_IPCXMLConnLayer_Flush(cl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }

  ccd->msgFlags=flags;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_Connected(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  DBG_INFO(0, "Up");
  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);
  if (ccd->connectedFn)
    ccd->connectedFn(ccd->serviceLayer, cl);
  DBG_DEBUG(0, "Up: done");
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_Disconnected(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  DBG_INFO(0, "Down");
  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);
  if (ccd->disconnectedFn)
    ccd->disconnectedFn(ccd->serviceLayer, cl);
  DBG_DEBUG(0, "Down: Done");
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_SetConnectedFn(GWEN_IPCCONNLAYER *cl,
                                         GWEN_IPCXMLCONNLAYER_CONNECTED_FN f){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);
  ccd->connectedFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_SetDisconnectedFn(GWEN_IPCCONNLAYER *cl,
                                            GWEN_IPCXMLCONNLAYER_DISCONNECTED_FN f){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);
  ccd->disconnectedFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_IPCXMLConnLayer_GetServiceCode(GWEN_IPCCONNLAYER *cl) {
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  assert(ccd->securityManager);
  return GWEN_SecContextMgr_GetServiceCode(ccd->securityManager);
}



/* --------------------------------------------------------------- FUNCTION */
const GWEN_CRYPTKEY*
GWEN_IPCXMLConnLayer_GetSignKey(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  assert(ccd->securityManager);
  return GWEN_IPCXMLSecCtxMgr_GetLocalSignKey(ccd->securityManager);
}



/* --------------------------------------------------------------- FUNCTION */
const GWEN_CRYPTKEY*
GWEN_IPCXMLConnLayer_GetCryptKey(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  assert(ccd->securityManager);
  return GWEN_IPCXMLSecCtxMgr_GetLocalSignKey(ccd->securityManager);
}







