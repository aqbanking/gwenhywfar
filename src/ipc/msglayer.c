/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Sep 16 2003
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

#include "msglayer_p.h"
#include <gwenhyfwar/transportlayer.h>
#include <gwenhyfwar/ipc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/misc.h>


static unsigned int GWEN_IPCMsgLayer_LastMsgId=0;
static unsigned int GWEN_IPCMsgLayer_LastMsgLayerId=0;



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_Msg_new(){
  GWEN_IPCMSG *m;

  GWEN_NEW_OBJECT(GWEN_IPCMSG, m);
  m->msgId=++GWEN_IPCMsgLayer_LastMsgId;
  return m;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_Msg_free(GWEN_IPCMSG *m){
  if (m) {
    if (m->freeDataFn)
      m->freeDataFn(m);
    GWEN_Buffer_free(m->buffer);
    free(m);
  }
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_Msg_SetMsgLayerId(GWEN_IPCMSG *m, unsigned int id){
  assert(m);
  m->msgLayerId=id;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_BUFFER *GWEN_Msg_GetBuffer(GWEN_IPCMSG *m){
  assert(m);
  return m->buffer;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_Msg_SetBuffer(GWEN_IPCMSG *m, GWEN_BUFFER *buf){
  assert(m);
  free(m->buffer);
  m->buffer=buf;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_Msg_GetMsgLayerId(GWEN_IPCMSG *m){
  assert(m);
  return m->msgLayerId;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_Msg_GetMsgId(GWEN_IPCMSG *m){
  assert(m);
  return m->msgId;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_Msg_GetReferenceId(GWEN_IPCMSG *m){
  assert(m);
  return m->refId;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_Msg_SetReferenceId(GWEN_IPCMSG *m, unsigned int i){
  assert(m);
  m->refId=i;
}



/* --------------------------------------------------------------- FUNCTION */
void *GWEN_Msg_GetData(GWEN_IPCMSG *m){
  assert(m);
  return m->data;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_Msg_SetData(GWEN_IPCMSG *m, void *d){
  assert(m);
  if (m->freeDataFn)
    m->freeDataFn(m);
  m->data=d;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_Msg_SetFreeFn(GWEN_IPCMSG *m, GWEN_IPCMSG_FREE f){
  assert(m);
  m->freeDataFn=f;
}




/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSGLAYER *GWEN_MsgLayer_new(GWEN_IPCTRANSPORTLAYER *tl,
                                    GWEN_IPCMSGLAYER_STATE st) {
  GWEN_IPCMSGLAYER *ml;

  assert(tl);
  GWEN_NEW_OBJECT(GWEN_IPCMSGLAYER, ml);
  ml->id=++GWEN_IPCMsgLayer_LastMsgLayerId;
  ml->maxIncomingMsgs=GWEN_IPCMSGLAYER_MAXINCOMING_MSGS;
  ml->maxOutgoingMsgs=GWEN_IPCMSGLAYER_MAXOUTGOING_MSGS;
  ml->state=st;
  ml->transportLayer=tl;
  return ml;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_MsgLayer_free(GWEN_IPCMSGLAYER *ml){
  if (ml) {
    GWEN_IPCMSG *msg;

    /* free all incoming msgs */
    msg=ml->incomingMsgs;
    while(msg) {
      GWEN_IPCMSG *nextmsg;

      nextmsg=msg->next;
      GWEN_Msg_free(msg);
      msg=nextmsg;
    }

    /* free all outgoing msgs */
    msg=ml->outgoingMsgs;
    while(msg) {
      GWEN_IPCMSG *nextmsg;

      nextmsg=msg->next;
      GWEN_Msg_free(msg);
      msg=nextmsg;
    }

    /* free the rest */
    if (ml->freeDataFn)
      ml->freeDataFn(ml);
    GWEN_IPCTransportLayer_free(ml->transportLayer);
    free(ml);
  }
}



/* --------------------------------------------------------------- FUNCTION */
void *GWEN_MsgLayer_GetData(GWEN_IPCMSGLAYER *ml){
  assert(ml);
  return ml->data;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_MsgLayer_SetData(GWEN_IPCMSGLAYER *ml, void *d){
  assert(ml);
  if (ml->freeDataFn)
    ml->freeDataFn(ml);
  ml->data=d;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_MsgLayer_Work(GWEN_IPCMSGLAYER *ml, int rd){
  assert(ml);
  if (ml->workFn)
    return ml->workFn(ml, rd);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_MsgLayer_GetIncomingMsg(GWEN_IPCMSGLAYER *ml){
  GWEN_IPCMSG *msg;

  assert(ml);
  if (ml->nIncomingMsgs) {
    msg=ml->incomingMsgs;
    assert(msg);
    GWEN_LIST_DEL(GWEN_IPCMSG, msg, &(ml->incomingMsgs));
    ml->nIncomingMsgs--;
    return msg;
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_MsgLayer_AddOutgoingMsg(GWEN_IPCMSGLAYER *ml,
                                            GWEN_IPCMSG *msg){
  assert(ml);
  assert(msg);
  if (ml->nOutgoingMsgs<ml->maxOutgoingMsgs) {
    GWEN_LIST_ADD(GWEN_IPCMSG, msg, &(ml->outgoingMsgs));
    ml->nOutgoingMsgs++;
  }
  else {
    DBG_INFO(0, "Outgoing queue full (%d msgs)", ml->nOutgoingMsgs);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_OUTQUEUE_FULL);
  }

  DBG_INFO(0, "Added outgoing msg (now %d msgs)", ml->nOutgoingMsgs);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_MsgLayer_CheckAddOutgoingMsg(GWEN_IPCMSGLAYER *ml){
  assert(ml);
  return ((ml->maxOutgoingMsgs-ml->nOutgoingMsgs)>0);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_MsgLayer_GetOutgoingMsg(GWEN_IPCMSGLAYER *ml){
  GWEN_IPCMSG *msg;

  assert(ml);
  if (ml->nOutgoingMsgs) {
    msg=ml->outgoingMsgs;
    assert(msg);
    GWEN_LIST_DEL(GWEN_IPCMSG, msg, &(ml->outgoingMsgs));
    ml->nOutgoingMsgs--;
    return msg;
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_MsgLayer_AddIncomingMsg(GWEN_IPCMSGLAYER *ml,
                                            GWEN_IPCMSG *msg){
  assert(ml);
  assert(msg);
  if (ml->nIncomingMsgs<ml->maxIncomingMsgs) {
    GWEN_LIST_ADD(GWEN_IPCMSG, msg, &(ml->incomingMsgs));
    ml->nIncomingMsgs++;
  }
  else {
    DBG_INFO(0, "Incoming queue full (%d msgs)", ml->nIncomingMsgs);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_INQUEUE_FULL);
  }

  DBG_INFO(0, "Added incoming msg (now %d msgs)", ml->nIncomingMsgs);
  return 0;

}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSGLAYER_STATE GWEN_MsgLayer_GetState(GWEN_IPCMSGLAYER *ml){
  assert(ml);
  return ml->state;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_MsgLayer_SetState(GWEN_IPCMSGLAYER *ml,
                            GWEN_IPCMSGLAYER_STATE st){
  assert(ml);
  DBG_INFO(0, "Changing state from %d to %d", ml->state, st);
  ml->state=st;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCTRANSPORTLAYER *GWEN_MsgLayer_GetTransportLayer(GWEN_IPCMSGLAYER *ml){
  assert(ml);
  return ml->transportLayer;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_MsgLayer_GetId(GWEN_IPCMSGLAYER *ml){
  assert(ml);
  return ml->id;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_MsgLayer_SetFreeDataFn(GWEN_IPCMSGLAYER *ml,
                                 GWEN_IPCMSGLAYER_FREE f){
  assert(ml);
  ml->freeDataFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_MsgLayer_SetWorkFn(GWEN_IPCMSGLAYER *ml, GWEN_IPCMSGLAYER_WORK f){
  assert(ml);
  ml->workFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_MsgLayer_SetAcceptFn(GWEN_IPCMSGLAYER *ml,
                               GWEN_IPCMSGLAYER_ACCEPT f){
  assert(ml);
  ml->acceptFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_MsgLayer_Connect(GWEN_IPCMSGLAYER *ml) {
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_ERRORCODE err;

  assert(ml);
  if (ml->state!=GWEN_IPCMsglayerStateClosed) {
    DBG_ERROR(0, "MsgLayer is not closed (state is %d)", ml->state);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_STATE);
  }
  assert(ml->transportLayer);
  tl=ml->transportLayer;
  err=GWEN_IPCTransportLayer_StartConnect(tl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_DEBUG(0, "called from here");
    return err;
  }
  ml->state=GWEN_IPCMsglayerStateConnecting;
  return err;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_MsgLayer_Disconnect(GWEN_IPCMSGLAYER *ml) {
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_ERRORCODE err;

  assert(ml);
  if (ml->state==GWEN_IPCMsglayerStateListening) {
    DBG_ERROR(0, "MsgLayer is listening, not connected");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_STATE);
  }
  assert(ml->transportLayer);
  tl=ml->transportLayer;
  ml->state=GWEN_IPCMsglayerStateClosed;
  err=GWEN_IPCTransportLayer_Disconnect(tl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_DEBUG(0, "called from here");
  }
  return err;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_MsgLayer_Accept(GWEN_IPCMSGLAYER *ml,
                                    GWEN_IPCMSGLAYER **m){
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_IPCTRANSPORTLAYER *newtl;
  GWEN_IPCMSGLAYER *newml;
  GWEN_ERRORCODE err;

  assert(ml);
  if (ml->state!=GWEN_IPCMsglayerStateListening) {
    DBG_ERROR(0, "MsgLayer is not listening");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_STATE);
  }

  /* let the transport layer accept the new connection */
  assert(ml->transportLayer);
  tl=ml->transportLayer;
  err=GWEN_IPCTransportLayer_Accept(tl, &newtl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_DEBUG(0, "called from here");
    return err;
  }

  /* let the msg layer accept the new connection */
  assert(ml->acceptFn);
  DBG_INFO(0, "Calling msgLayer->acceptFn");
  err=ml->acceptFn(ml, newtl, &newml);
  if (!GWEN_Error_IsOk(err)) {
    DBG_DEBUG(0, "called from here");
    return err;
  }
  *m=newml;

  DBG_INFO(0, "Connection accepted");
  return err;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_SOCKET *GWEN_IPCMsgLayer_GetReadSocket(GWEN_IPCMSGLAYER *ml){
  assert(ml);
  assert(ml->transportLayer);
  return GWEN_IPCTransportLayer_GetReadSocket(ml->transportLayer);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_SOCKET *GWEN_IPCMsgLayer_GetWriteSocket(GWEN_IPCMSGLAYER *ml){
  assert(ml);
  assert(ml->transportLayer);
  return GWEN_IPCTransportLayer_GetWriteSocket(ml->transportLayer);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_Msg_GetNext(GWEN_IPCMSG *m){
  assert(m);
  return m->next;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_MsgLayer_OutgoingMsgs(GWEN_IPCMSGLAYER *ml){
  assert(ml);
  return ml->nOutgoingMsgs;
}







