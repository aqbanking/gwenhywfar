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



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCXMLCONNLAYERDATA *GWEN_IPCXMLConnLayerData_new(){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  GWEN_NEW_OBJECT(GWEN_IPCXMLCONNLAYERDATA, ccd);
  return ccd;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayerData_free(GWEN_IPCXMLCONNLAYERDATA *ccd){
  if (ccd) {
    GWEN_HBCIMSG *m, *mn;

    GWEN_CryptKey_free(ccd->localKey);
    GWEN_CryptKey_free(ccd->remoteKey);
    GWEN_CryptKey_free(ccd->sessionKey);
    free(ccd->peerName);
    free(ccd->peerVersion);
    free(ccd->ownName);
    free(ccd->ownVersion);

    m=ccd->incomingMsgs;
    while(m) {
      mn=GWEN_HBCIMsg_Next(m);
      GWEN_HBCIMsg_Del(m, &(ccd->incomingMsgs));
      GWEN_HBCIMsg_free(m);
      m=mn;
    } /*while */

    m=ccd->outgoingMsgs;
    while(m) {
      mn=GWEN_HBCIMsg_Next(m);
      GWEN_HBCIMsg_Del(m, &(ccd->outgoingMsgs));
      GWEN_HBCIMsg_free(m);
      m=mn;
    } /*while */

    free(ccd);
  }
}





/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYER *GWEN_IPCXMLConnLayer_new(GWEN_MSGENGINE *msgEngine,
                                            GWEN_IPCMSGLAYER *ml,
                                            GWEN_IPCCONNLAYER_STATE st){
  GWEN_IPCCONNLAYER *cl;
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  cl=GWEN_ConnectionLayer_new(ml, st);
  ccd=GWEN_IPCXMLConnLayerData_new();
  ccd->msgEngine=msgEngine;
  GWEN_ConnectionLayer_SetType(cl, GWEN_IPCXMLCONNLAYER_TYPE);
  GWEN_ConnectionLayer_SetData(cl, ccd);
  GWEN_ConnectionLayer_SetFreeDataFn(cl, GWEN_IPCXMLConnLayer_free);
  GWEN_ConnectionLayer_SetWorkFn(cl, GWEN_IPCXMLConnLayer_Work);
  GWEN_ConnectionLayer_SetAcceptFn(cl, GWEN_IPCXMLConnLayer_Accept);
  GWEN_ConnectionLayer_SetOpenFn(cl, GWEN_IPCXMLConnLayer_Open);
  GWEN_ConnectionLayer_SetCloseFn(cl, GWEN_IPCXMLConnLayer_Close);

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
void GWEN_IPCXMLConnLayer_SetMsgEngine(GWEN_IPCCONNLAYER *cl,
                                       GWEN_MSGENGINE *e){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  ccd->msgEngine=e;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_IPCXMLConnLayer_GetSecurityState(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  return ccd->securityState;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_SetSecurityState(GWEN_IPCCONNLAYER *cl,
                                           unsigned int s){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  ccd->securityState=s;
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
GWEN_CRYPTKEY *GWEN_IPCXMLConnLayer_GetRemoteKey(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  return ccd->remoteKey;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_SetRemoteKey(GWEN_IPCCONNLAYER *cl,
                                       GWEN_CRYPTKEY *k){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  GWEN_CryptKey_free(ccd->remoteKey);
  ccd->remoteKey=k;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_CRYPTKEY *GWEN_IPCXMLConnLayer_GetSessionKey(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  GWEN_CryptKey_free(ccd->localKey);
  return ccd->sessionKey;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_SetSessionKey(GWEN_IPCCONNLAYER *cl,
                                        GWEN_CRYPTKEY *k){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  GWEN_CryptKey_free(ccd->sessionKey);
  ccd->sessionKey=k;
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_IPCXMLConnLayer_GetPeerName(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  return ccd->peerName;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_SetPeerName(GWEN_IPCCONNLAYER *cl,
                                      const char *s){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  assert(s);
  free(ccd->peerName);
  ccd->peerName=strdup(s);
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_IPCXMLConnLayer_GetPeerVersion(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  return ccd->peerVersion;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_SetPeerVersion(GWEN_IPCCONNLAYER *cl,
                                         const char *s){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  free(ccd->peerVersion);
  assert(s);
  ccd->peerVersion=strdup(s);
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_IPCXMLConnLayer_GetOwnName(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  return ccd->ownName;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_SetOwnName(GWEN_IPCCONNLAYER *cl,
                                     const char *s){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  free(ccd->ownName);
  assert(s);
  ccd->ownName=strdup(s);
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_IPCXMLConnLayer_GetOwnVersion(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  return ccd->ownVersion;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_SetOwnVersion(GWEN_IPCCONNLAYER *cl,
                                        const char *s){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  free(ccd->ownVersion);
  assert(s);
  ccd->ownVersion=strdup(s);
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

  newcl=GWEN_IPCXMLConnLayer_new(ccd->msgEngine, ml,
                                 GWEN_IPCConnectionLayerStateOpening);
  DBG_INFO(0, "Created new GWEN_IPCCONNLAYER for incoming connection");
  newccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(newcl);
  assert(newccd);

  /* copy important data */
  if (ccd->ownName)
    newccd->ownName=strdup(ccd->ownName);
  if (ccd->ownVersion)
    newccd->ownVersion=strdup(ccd->ownVersion);
  newccd->flags=ccd->flags;
  if (ccd->localKey)
    newccd->localKey=GWEN_CryptKey_dup(ccd->localKey);

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
  GWEN_CryptKey_free(ccd->sessionKey);
  ccd->sessionKey=0;
  GWEN_ConnectionLayer_SetState(cl, GWEN_IPCConnectionLayerStateOpening);
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
  GWEN_CryptKey_free(ccd->sessionKey);
  ccd->sessionKey=0;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_AddIncomingMsg(GWEN_IPCCONNLAYER *cl,
                                         GWEN_HBCIMSG *m){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  assert(m);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  GWEN_HBCIMsg_Add(m, &(ccd->incomingMsgs));
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_AddOutgoingMsg(GWEN_IPCCONNLAYER *cl,
                                         GWEN_HBCIMSG *m){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  assert(m);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  GWEN_HBCIMsg_Del(m, &(ccd->incomingMsgs));
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_HBCIMSG *GWEN_IPCXMLConnLayer_FindIncomingMsg(GWEN_IPCCONNLAYER *cl,
                                                      unsigned int refid){
  GWEN_IPCXMLCONNLAYERDATA *ccd;
  GWEN_HBCIMSG *m;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  m=ccd->incomingMsgs;
  while(m) {
    if (GWEN_HBCIMsg_GetMsgRef(m)==refid)
      return m;
    m=GWEN_HBCIMsg_Next(m);
  } /* while */
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_HBCIMSG *GWEN_IPCXMLConnLayer_NextIncomingMsg(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  return ccd->incomingMsgs;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCXMLConnLayer_UnlinkIncomingMsg(GWEN_IPCCONNLAYER *cl,
                                            GWEN_HBCIMSG *m){
  GWEN_IPCXMLCONNLAYERDATA *ccd;

  assert(cl);
  assert(m);
  ccd=(GWEN_IPCXMLCONNLAYERDATA*)GWEN_ConnectionLayer_GetData(cl);
  assert(ccd);
  assert(GWEN_ConnectionLayer_GetType(cl)==GWEN_IPCXMLCONNLAYER_TYPE);

  GWEN_HBCIMsg_Del(m, &(ccd->incomingMsgs));
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
  mst=GWEN_MsgLayer_GetState(ml);

  /* let the underlying layers work */
  err=GWEN_MsgLayer_Work(ml, rd);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    return err;
  }

  if (mst==GWEN_IPCMsglayerStateConnecting) {
    err=GWEN_IPCTransportLayer_FinishConnect(tl);
    if (!GWEN_Error_IsOk(err)) {
      if (GWEN_Error_GetType(err)!=
          GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
          GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_IN_PROGRESS) {
        // real error, so return that error
        DBG_INFO(0, "called from here");
        return err;
      }
      /* otherwise connecting in progress, return */
      /* TODO: timeout */
      return 0;
    }
    /* goto next state */
    DBG_INFO(0, "Physically connected");
    GWEN_MsgLayer_SetState(ml, GWEN_IPCMsglayerStateIdle);
  }



  return 0;
}












