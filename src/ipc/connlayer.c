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

#include "connlayer_p.h"
#include "msglayer_p.h"
#include <gwenhyfwar/msglayer.h>
#include <gwenhyfwar/ipc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/misc.h>



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYER *GWEN_ConnectionLayer_new(GWEN_IPCMSGLAYER *ml,
                                            GWEN_IPCCONNLAYER_STATE st) {
  GWEN_IPCCONNLAYER *cl;

  assert(ml);
  GWEN_NEW_OBJECT(GWEN_IPCCONNLAYER, cl);
  cl->maxIncomingMsgs=GWEN_IPCCONNLAYER_MAXINCOMING_MSGS;
  cl->maxOutgoingMsgs=GWEN_IPCCONNLAYER_MAXOUTGOING_MSGS;
  cl->state=st;
  cl->msgLayer=ml;
  return cl;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_free(GWEN_IPCCONNLAYER *cl){
  if (cl) {
    GWEN_IPCMSG *msg;

    /* free all incoming msgs */
    msg=cl->incomingMsgs;
    while(msg) {
      GWEN_IPCMSG *nextmsg;

      nextmsg=msg->next;
      GWEN_Msg_free(msg);
      msg=nextmsg;
    }

    /* free all outgoing msgs */
    msg=cl->outgoingMsgs;
    while(msg) {
      GWEN_IPCMSG *nextmsg;

      nextmsg=msg->next;
      GWEN_Msg_free(msg);
      msg=nextmsg;
    }

    /* free the rest */
    if (cl->freeDataFn)
      cl->freeDataFn(cl);
    GWEN_MsgLayer_free(cl->msgLayer);
    free(cl->info);
    free(cl);
  }
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_ConnectionLayer_GetInfo(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->info;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetInfo(GWEN_IPCCONNLAYER *cl,
                                  const char *s){
  assert(cl);
  assert(s);
  free(cl->info);
  cl->info=strdup(s);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_SOCKET *GWEN_ConnectionLayer_GetReadSocket(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  assert(cl->msgLayer);
  return GWEN_IPCMsgLayer_GetReadSocket(cl->msgLayer);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_SOCKET *GWEN_ConnectionLayer_GetWriteSocket(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  assert(cl->msgLayer);
  return GWEN_IPCMsgLayer_GetWriteSocket(cl->msgLayer);
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetFreeDataFn(GWEN_IPCCONNLAYER *cl,
                                        GWEN_IPCCONNLAYER_FREE f){
  assert(cl);
  cl->freeDataFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetWorkFn(GWEN_IPCCONNLAYER *cl,
                                    GWEN_IPCCONNLAYER_WORK f){
  assert(cl);
  cl->workFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetAcceptFn(GWEN_IPCCONNLAYER *cl,
                                      GWEN_IPCCONNLAYER_ACCEPT f){
  assert(cl);
  cl->acceptFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetOpenFn(GWEN_IPCCONNLAYER *cl,
                                    GWEN_IPCCONNLAYER_OPEN f){
  assert(cl);
  cl->openFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetCloseFn(GWEN_IPCCONNLAYER *cl,
                                     GWEN_IPCCONNLAYER_CLOSE f){
  assert(cl);
  cl->closeFn=f;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayer_Accept(GWEN_IPCCONNLAYER *cl,
                                           GWEN_IPCCONNLAYER **c){
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCMSGLAYER *newml;
  GWEN_IPCCONNLAYER *newcl;
  GWEN_ERRORCODE err;

  assert(cl);
  if (cl->state!=GWEN_IPCConnectionLayerStateListening) {
    DBG_ERROR(0, "ConnectionLayer is not listening");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_STATE);
  }

  /* let the message layer accept the new connection */
  assert(cl->msgLayer);
  ml=cl->msgLayer;
  err=GWEN_MsgLayer_Accept(ml, &newml);
  if (!GWEN_Error_IsOk(err)) {
    DBG_DEBUG(0, "called from here");
    return err;
  }

  /* let the connection layer accept the new connection */
  assert(cl->acceptFn);
  DBG_INFO(0, "Calling connectionLayer->acceptFn");
  err=cl->acceptFn(cl, newml, &newcl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_DEBUG(0, "called from here");
    return err;
  }
  newcl->libMark=cl->libMark;
  newcl->userMark=cl->userMark;
  newcl->flags=cl->flags | GWEN_IPCCONNLAYER_FLAGS_PASSIVE;
  *c=newcl;

  DBG_INFO(0, "Connection accepted as %d",
           GWEN_ConnectionLayer_GetId(newcl));

  return err;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_ConnectionLayer_GetIncomingMsg(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCMSG *msg;

  assert(cl);
  if (cl->nIncomingMsgs) {
    msg=cl->incomingMsgs;
    assert(msg);
    GWEN_LIST_DEL(GWEN_IPCMSG, msg, &(cl->incomingMsgs));
    cl->nIncomingMsgs--;
    DBG_INFO(0, "Returning incoming message");
    return msg;
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_ConnectionLayer_GetOutgoingMsg(GWEN_IPCCONNLAYER *cl){
  GWEN_IPCMSG *msg;

  assert(cl);
  if (cl->nOutgoingMsgs) {
    msg=cl->outgoingMsgs;
    assert(msg);
    GWEN_LIST_DEL(GWEN_IPCMSG, msg, &(cl->outgoingMsgs));
    cl->nOutgoingMsgs--;
    DBG_INFO(0, "Returning outgoing message");
    return msg;
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_ConnectionLayer_FindMsgReply(GWEN_IPCCONNLAYER *cl,
                                               unsigned int refId){
  GWEN_IPCMSG *msg;

  assert(cl);
  msg=cl->incomingMsgs;
  if (!msg) {
    DBG_INFO(0, "No incoming messages on %d.",
             GWEN_ConnectionLayer_GetId(cl));
  }
  while(msg) {
    DBG_INFO(0, "Checking msg (refid=%d)", GWEN_Msg_GetReferenceId(msg));
    if (GWEN_Msg_GetReferenceId(msg)==refId) {
      DBG_DEBUG(0, "Found message for ID %d", refId);
      GWEN_LIST_DEL(GWEN_IPCMSG, msg, &(cl->incomingMsgs));
      cl->nIncomingMsgs--;
      return msg;
    }
    msg=GWEN_Msg_GetNext(msg);
  }
  DBG_DEBUG(0, "No message for ID %d found", refId);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayer_AddOutgoingMsg(GWEN_IPCCONNLAYER *cl,
                                                   GWEN_IPCMSG *msg){
  assert(cl);
  assert(msg);
  if (cl->nOutgoingMsgs<cl->maxOutgoingMsgs) {
    GWEN_LIST_ADD(GWEN_IPCMSG, msg, &(cl->outgoingMsgs));
    cl->nOutgoingMsgs++;
  }
  else {
    DBG_INFO(0, "Outgoing queue full (%d msgs)", cl->nOutgoingMsgs);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_OUTQUEUE_FULL);
  }

  DBG_INFO(0, "Added outgoing msg (now %d msgs)", cl->nOutgoingMsgs);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayer_AddIncomingMsg(GWEN_IPCCONNLAYER *cl,
                                                   GWEN_IPCMSG *msg){
  assert(cl);
  assert(msg);
  if (cl->nIncomingMsgs<cl->maxIncomingMsgs) {
    GWEN_LIST_ADD(GWEN_IPCMSG, msg, &(cl->incomingMsgs));
    cl->nIncomingMsgs++;
    DBG_INFO(0, "Added message to incoming queue (%d)",
             cl->nIncomingMsgs);
    DBG_INFO(0, "First incoming message is %08x",
             (unsigned int)cl->incomingMsgs);
  }
  else {
    DBG_INFO(0, "Incoming queue full (%d msgs)", cl->nIncomingMsgs);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_INQUEUE_FULL);
  }

  DBG_INFO(0, "Added incoming msg to %d (now %d msgs)",
           GWEN_ConnectionLayer_GetId(cl),
           cl->nIncomingMsgs);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
void *GWEN_ConnectionLayer_GetData(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->data;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetData(GWEN_IPCCONNLAYER *cl, void *d){
  assert(cl);
  if (cl->freeDataFn)
    cl->freeDataFn(cl);
  cl->data=d;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayer_Work(GWEN_IPCCONNLAYER *cl,
                                         int rd){
  assert(cl);
  if (cl->workFn)
    return cl->workFn(cl, rd);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSGLAYER *GWEN_ConnectionLayer_GetMsgLayer(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->msgLayer;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayer_Open(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  if (cl->openFn)
    return cl->openFn(cl);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ConnectionLayer_Close(GWEN_IPCCONNLAYER *cl,
                                          int force){
  assert(cl);
  if (cl->closeFn)
    return cl->closeFn(cl, force);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_ConnectionLayer_GetId(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  assert(cl->msgLayer);
  return GWEN_MsgLayer_GetId(cl->msgLayer);
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_ConnectionLayer_GetUserMark(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->userMark;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetUserMark(GWEN_IPCCONNLAYER *cl,
                                      unsigned int i){
  assert(cl);
  cl->userMark=i;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_ConnectionLayer_GetLibMark(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->libMark;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetLibMark(GWEN_IPCCONNLAYER *cl,
                                     unsigned int i){
  assert(cl);
  cl->libMark=i;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYER_STATE
GWEN_ConnectionLayer_GetState(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->state;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetState(GWEN_IPCCONNLAYER *cl,
                                   GWEN_IPCCONNLAYER_STATE st){
  assert(cl);
  DBG_INFO(0, "Changing state on %d from \"%s\" (%d) to \"%s\" (%d)",
           GWEN_ConnectionLayer_GetId(cl),
           GWEN_ConnectionLayer_GetStateString(cl->state),
           cl->state,
           GWEN_ConnectionLayer_GetStateString(st),
           st);
  cl->state=st;
}



/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_ConnectionLayer_GetStateString(GWEN_IPCCONNLAYER_STATE st){
  switch(st) {
  case GWEN_IPCConnectionLayerStateUnconnected: return "Unconnected";
  case GWEN_IPCConnectionLayerStateOpening:     return "Opening";
  case GWEN_IPCConnectionLayerStateOpen:        return "Open";
  case GWEN_IPCConnectionLayerStateClosing:     return "Closing";
  case GWEN_IPCConnectionLayerStateListening:   return "Listening";
  case GWEN_IPCConnectionLayerStateClosed:      return "Closed";
  default: return "Unknown";
  }
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_ConnectionLayer_GetFlags(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->flags;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetFlags(GWEN_IPCCONNLAYER *cl,
                                   unsigned int flags){
  assert(cl);
  cl->flags=flags;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYER *GWEN_ConnectionLayer_GetNext(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->next;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_ConnectionLayer_HasOutgoingMsg(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return (cl->nOutgoingMsgs!=0);
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_Append(GWEN_IPCCONNLAYER *cl,
                                 GWEN_IPCCONNLAYER *newcl){
  assert(cl);
  assert(newcl);
  GWEN_LIST_ADD(GWEN_IPCCONNLAYER, newcl, &cl);
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int GWEN_ConnectionLayer_GetType(GWEN_IPCCONNLAYER *cl){
  assert(cl);
  return cl->typ;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ConnectionLayer_SetType(GWEN_IPCCONNLAYER *cl,
                                  unsigned int i){
  assert(cl);
  cl->typ=i;
}




