/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 15 2003
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

#include "ipcxml_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/ipcxmlmsglayer.h>
#include <gwenhywfar/ipcxmlconnlayer.h>
#include <gwenhywfar/ipc.h>
#include <gwenhywfar/hbcimsg.h>
#include <gwenhywfar/hbcidialog.h>
#include <gwenhywfar/ipcxmlcmd.h>
#include <time.h>


static unsigned int gwen_ipcxmlrequest_lastid=0;
static unsigned int gwen_ipcxmlsession_lastid=0;




GWEN_IPCXMLREQUEST *GWEN_IPCXMLRequest_new(){
  GWEN_IPCXMLREQUEST *r;

  GWEN_NEW_OBJECT(GWEN_IPCXMLREQUEST, r);
  r->id=++gwen_ipcxmlrequest_lastid;
  r->db=GWEN_DB_Group_new("requestdata");
  return r;
}



void GWEN_IPCXMLRequest_free(GWEN_IPCXMLREQUEST *r){
  if (r) {
    GWEN_DB_Group_free(r->db);
    free(r);
  }
}



unsigned int GWEN_IPCXMLRequest_GetId(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->id;
}



unsigned int GWEN_IPCXMLRequest_GetMsgLayerId(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->msgLayerId;
}



void GWEN_IPCXMLRequest_SetMsgLayerId(GWEN_IPCXMLREQUEST *r,
                                      unsigned int i){
  assert(r);
  r->msgLayerId=i;
}


unsigned int GWEN_IPCXMLRequest_GetDialogId(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->dialogId;
}



void GWEN_IPCXMLRequest_SetDialogId(GWEN_IPCXMLREQUEST *r,
                                    unsigned int i){
  assert(r);
  r->dialogId=i;
}



unsigned int GWEN_IPCXMLRequest_GetMessageNumber(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->messageNumber;
}



void GWEN_IPCXMLRequest_SetMessageNumber(GWEN_IPCXMLREQUEST *r,
                                         unsigned int i){
  assert(r);
  r->messageNumber=i;
}



unsigned int GWEN_IPCXMLRequest_GetSegmentNumber(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->segmentNumber;
}



void GWEN_IPCXMLRequest_SetSegmentNumber(GWEN_IPCXMLREQUEST *r,
                                         unsigned int i){
  assert(r);
  r->segmentNumber=i;
}



GWEN_DB_NODE *GWEN_IPCXMLRequest_GetDb(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->db;
}



void GWEN_IPCXMLRequest_SetDb(GWEN_IPCXMLREQUEST *r,
                              GWEN_DB_NODE *db){
  assert(r);
  GWEN_DB_Group_free(r->db);
  r->db=db;
}






GWEN_IPCXMLSERVICE *GWEN_IPCXMLService_new(GWEN_MSGENGINE *msgEngine,
                                           GWEN_SECCTX_MANAGER *scm) {
  GWEN_IPCXMLSERVICE *xs;

  GWEN_NEW_OBJECT(GWEN_IPCXMLSERVICE, xs);
  xs->msgEngine=msgEngine;
  xs->securityManager=scm;
  xs->serviceLayer=GWEN_ServiceLayer_new();
  /* store pointer to this service in service layer
   * do not setup a freeDataFn for this, because the service layer
   * will not own this ! */
  GWEN_ServiceLayer_SetData(xs->serviceLayer, xs);

  return xs;
}



void GWEN_IPCXMLService_free(GWEN_IPCXMLSERVICE *xs) {
  if (xs) {
    GWEN_IPCXMLREQUEST *rq;

    rq=xs->outgoingRequests;
    while(rq) {
      GWEN_IPCXMLREQUEST *nrq;

      nrq=rq->next;
      GWEN_IPCXMLRequest_free(rq);
      rq=nrq;
    } /* while */
    rq=xs->incomingRequests;
    while(rq) {
      GWEN_IPCXMLREQUEST *nrq;

      nrq=rq->next;
      GWEN_IPCXMLRequest_free(rq);
      rq=nrq;
    } /* while */
    GWEN_ServiceLayer_free(xs->serviceLayer);
    free(xs);
  }
}




GWEN_IPCCONNLAYER *GWEN_IPCXMLService_CreateCL(GWEN_IPCXMLSERVICE *xs,
                                               GWEN_IPCXMLSERVICE_TYPE st,
                                               const char *localContext,
                                               unsigned int userMark,
                                               const char *addr,
                                               unsigned int port,
                                               unsigned int flags){
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;
  unsigned int cid;

  switch(st) {
  case GWEN_IPCXMLServiceTypeTCP:
    tl=GWEN_IPCTransportLayerTCP_new();
    GWEN_IPCTransportLayer_SetPort(tl, port);
    break;
  case GWEN_IPCXMLServiceTypeUnix:
    tl=GWEN_IPCTransportLayerUnix_new();
    break;
  default:
    DBG_ERROR(0, "Unknown service type (%d)", st);
    return 0;
  } /* switch */
  GWEN_IPCTransportLayer_SetAddress(tl, addr);

  /* start listening */
  if (flags & GWEN_IPCCONNLAYER_FLAGS_PASSIVE) {
    err=GWEN_IPCTransportLayer_Listen(tl);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO(0, "called from here");
      GWEN_IPCTransportLayer_free(tl);
      return 0;
    }
  }

  /* create higher layers */
  if (flags & GWEN_IPCCONNLAYER_FLAGS_PASSIVE) {
    ml=GWEN_IPCXMLMsgLayer_new(xs->msgEngine, tl,
                               GWEN_IPCMsglayerStateListening);
  }
  else {
    ml=GWEN_IPCXMLMsgLayer_new(xs->msgEngine, tl,
                               GWEN_IPCMsglayerStateUnconnected);
  }
  cl=GWEN_IPCXMLConnLayer_new(xs->serviceLayer,
                              xs->msgEngine,
                              localContext,
                              xs->securityManager,
                              ml,
                              !(flags & GWEN_IPCCONNLAYER_FLAGS_PASSIVE));

  cid=GWEN_ConnectionLayer_GetId(cl);
  GWEN_ConnectionLayer_SetUserMark(cl, userMark);
  GWEN_ConnectionLayer_SetFlags(cl, flags);
  GWEN_IPCXMLConnLayer_SetConnectedFn(cl, GWEN_IPCXMLService_ConnectionUp);
  GWEN_IPCXMLConnLayer_SetDisconnectedFn(cl,
                                         GWEN_IPCXMLService_ConnectionDown);
  return cl;
}



unsigned int GWEN_IPCXMLService_AddServer(GWEN_IPCXMLSERVICE *xs,
                                          GWEN_IPCXMLSERVICE_TYPE st,
                                          const char *localContext,
                                          unsigned int userMark,
                                          const char *addr,
                                          unsigned int port,
                                          unsigned int flags){
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;

  cl=GWEN_IPCXMLService_CreateCL(xs, st, localContext, userMark, addr, port,
                                 flags | GWEN_IPCCONNLAYER_FLAGS_PASSIVE);
  err=GWEN_ServiceLayer_AddConnection(xs->serviceLayer, cl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_ConnectionLayer_free(cl);
    return 0;
  }

  return GWEN_ConnectionLayer_GetId(cl);
}



unsigned int GWEN_IPCXMLService_AddClient(GWEN_IPCXMLSERVICE *xs,
                                          GWEN_IPCXMLSERVICE_TYPE st,
                                          const char *localName,
                                          const char *remoteName,
                                          unsigned int userMark,
                                          const char *addr,
                                          unsigned int port,
                                          unsigned int flags){
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;

  cl=GWEN_IPCXMLService_CreateCL(xs, st, localName, userMark, addr, port,
                                 flags & ~GWEN_IPCCONNLAYER_FLAGS_PASSIVE);
  err=GWEN_ServiceLayer_AddConnection(xs->serviceLayer, cl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_ConnectionLayer_free(cl);
    return 0;
  }
  GWEN_IPCXMLConnLayer_SetRemoteName(cl, remoteName);
  return GWEN_ConnectionLayer_GetId(cl);
}


unsigned int GWEN_IPCXMLService_GetSessionId(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int clid){
  GWEN_IPCCONNLAYER *cl;

  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return 0;
  }

  return GWEN_IPCXMLConnLayer_GetSessionId(cl);
}



void GWEN_IPCXMLService_SetSessionId(GWEN_IPCXMLSERVICE *xs,
                                     unsigned int clid,
                                     unsigned int sid){
  GWEN_IPCCONNLAYER *cl;

  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return;
  }

  GWEN_IPCXMLConnLayer_SetSessionId(cl, sid);
}



unsigned int GWEN_IPCXMLService_AddRequest(GWEN_IPCXMLSERVICE *xs,
                                           unsigned int clid,
                                           const char *requestName,
                                           unsigned int requestVersion,
                                           GWEN_DB_NODE *db,
                                           unsigned int flags){
  GWEN_IPCCONNLAYER *cl;
  GWEN_XMLNODE *node;
  GWEN_IPCXMLREQUEST *rq;

  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return 0;
  }

  node=GWEN_MsgEngine_FindNodeByProperty(xs->msgEngine,
                                         "SEG",
                                         "id",
                                         requestVersion,
                                         requestName);
  if (!node) {
    DBG_ERROR(0, "Request \"%s\" (version %d) not found",
              requestName, requestVersion);
    return 0;
  }

  rq=GWEN_IPCXMLConnLayer_AddRequest(cl, node, db, flags);
  if (!rq) {
    DBG_ERROR(0, "Could not add request \"%s\" (version %d)",
              requestName, requestVersion);
    return 0;
  }

  GWEN_LIST_ADD(GWEN_IPCXMLREQUEST, rq, &(xs->outgoingRequests));
  return rq->id;
}



GWEN_ERRORCODE GWEN_IPCXMLService_SetSecurityFlags(GWEN_IPCXMLSERVICE *xs,
                                                   unsigned int clid,
                                                   unsigned int flags) {
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;

  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_CONNECTION_NOT_FOUND);
  }

  err=GWEN_IPCXMLConnLayer_SetSecurityFlags(cl, flags);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }
  return 0;
}



unsigned int GWEN_IPCXMLService_GetSecurityFlags(GWEN_IPCXMLSERVICE *xs,
                                                 unsigned int clid){
  GWEN_IPCCONNLAYER *cl;

  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_CONNECTION_NOT_FOUND);
  }

  return GWEN_IPCXMLConnLayer_GetSecurityFlags(cl);
}



GWEN_ERRORCODE GWEN_IPCXMLService_Work(GWEN_IPCXMLSERVICE *xs,
                                       int timeout){
  GWEN_ERRORCODE err;

  assert(xs);
  assert(xs->serviceLayer);
  err=GWEN_ServiceLayer_Work(xs->serviceLayer, timeout);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }

  return 0;
}



GWEN_ERRORCODE GWEN_IPCXMLService_HandleMsgs(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int userMark,
                                             unsigned int maxmsgs){
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;
  GWEN_IPCMSG *imsg;
  GWEN_HBCIMSG *hmsg;
  unsigned int clid;
  unsigned int msgs;
  unsigned int errors;
  unsigned int ok;

  assert(xs);
  msgs=0;
  errors=0;
  ok=0;
  while(msgs<maxmsgs || msgs==0) {
    DBG_INFO(0, "Handling message %d", msgs);
    imsg=GWEN_ServiceLayer_NextIncomingMsg(xs->serviceLayer,
                                           userMark);
    if (!imsg) {
      if (msgs) {
        DBG_INFO(0, "No more messages");
        break;
      }
      DBG_INFO(0, "No message");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_WARN,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_INQUEUE_EMPTY);
    }
    clid=GWEN_Msg_GetMsgLayerId(imsg);
    cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
    if (!cl) {
      DBG_ERROR(0, "Connection not found (%d), internal error", clid);
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_CONNECTION_NOT_FOUND);
    }

    DBG_INFO(0, "Got a message for connection %d",
             GWEN_ConnectionLayer_GetId(cl));

    hmsg=GWEN_IPCXMLConnLayer_IPC2HBCI(cl, imsg);
    GWEN_Msg_free(imsg);
    imsg=0;
    if (!hmsg) {
      /* TODO: Requeue message */
      DBG_ERROR(0, "Message lost");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_MSG);
    }
    err=GWEN_IPCXMLService_HandleMsg(xs, hmsg);
    GWEN_HBCIMsg_free(hmsg);
    hmsg=0;
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(0, err);
      DBG_WARN(0, "Message lost");
      errors++;
    }
    else {
      ok++;
    }

    DBG_INFO(0, "Message handled successfully");
    msgs++;
  } /* while */

  if (errors) {
    if (ok)
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_SOME_FAILED);
    DBG_ERROR(0, "All messages failed");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  return 0;
}



int GWEN_IPCXMLService_AddSecurityGroup(GWEN_HBCIMSG *hmsg,
                                        GWEN_DB_NODE *n) {
  const GWEN_KEYSPEC *ks;
  GWEN_DB_NODE *ngr;
  const char *p;

  assert(hmsg);
  assert(n);

  /* add security group */
  ngr=GWEN_DB_GetGroup(n, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                       "security");
  assert(ngr);

  /* store crypter */
  ks=GWEN_HBCIMsg_GetCrypter(hmsg);
  if (ks) {
    p=GWEN_KeySpec_GetOwner(ks);
    if (!p) {
      DBG_ERROR(0, "Crypter has no name, aborting.");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_MSG);
    } /* if owner */
    GWEN_DB_SetCharValue(ngr,
                         GWEN_DB_FLAGS_DEFAULT,
                         "crypter",
                         p);
  }

  /* store signers */
  ks=GWEN_HBCIMsg_GetSigners(hmsg);
  while (ks) {
    p=GWEN_KeySpec_GetOwner(ks);
    if (!p) {
      DBG_ERROR(0, "Signer has no name, aborting.");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_BAD_MSG);
    } /* if owner */
    GWEN_DB_SetCharValue(ngr,
                         GWEN_DB_FLAGS_DEFAULT,
                         "signers",
                         p);
    ks=GWEN_KeySpec_ConstNext(ks);
  } /* while */

  GWEN_DB_SetIntValue(ngr,
                      GWEN_DB_FLAGS_DEFAULT,
                      "dialognumber",
                      GWEN_HBCIMsg_GetDialogNumber(hmsg));

  GWEN_DB_SetIntValue(ngr,
                      GWEN_DB_FLAGS_DEFAULT,
                      "msglayerid",
                      GWEN_HBCIMsg_GetMsgLayerId(hmsg));

  GWEN_DB_SetCharValue(ngr,
                       GWEN_DB_FLAGS_DEFAULT,
                       "dialogid",
                       GWEN_HBCIMsg_GetDialogId(hmsg));

  return 0;
}



GWEN_ERRORCODE GWEN_IPCXMLService_HandleMsg(GWEN_IPCXMLSERVICE *xs,
                                            GWEN_HBCIMSG *hmsg) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *gr;

  assert(xs);
  assert(hmsg);

  db=GWEN_DB_Group_new("hbcimsg");
  if (GWEN_HBCIMsg_DecodeMsg(hmsg, db, GWEN_MSGENGINE_READ_FLAGS_DEFAULT)) {
    DBG_INFO(0, "here");
    GWEN_DB_Group_free(db);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_MSG);
  }

  gr=GWEN_DB_GetFirstGroup(db);
  while (gr) {
    if (strcasecmp(GWEN_DB_GroupName(gr), "MsgHead")==0) {
      DBG_INFO(0, "Ignoring message head");
    }
    else if (strcasecmp(GWEN_DB_GroupName(gr), "MsgTail")==0) {
      DBG_INFO(0, "Ignoring message tail");
    }
    else if (strcasecmp(GWEN_DB_GroupName(gr), "SigHead")==0) {
      DBG_INFO(0, "Ignoring signature head");
    }
    else if (strcasecmp(GWEN_DB_GroupName(gr), "SigTail")==0) {
      DBG_INFO(0, "Ignoring signature tail");
    }
    else {
      unsigned msgLayerId;
      unsigned int dialogId;
      unsigned int messageNumber;
      unsigned int segmentNumber;
      GWEN_IPCXMLREQUEST *rq;

      msgLayerId=GWEN_HBCIMsg_GetMsgLayerId(hmsg);
      dialogId=GWEN_HBCIMsg_GetDialogNumber(hmsg);
      messageNumber=GWEN_HBCIMsg_GetMsgRef(hmsg);
      segmentNumber=GWEN_DB_GetIntValue(gr, "head/ref", 0, 0);

      if (messageNumber) {
        unsigned int matches;

        DBG_INFO(0, "Got a response to message %d", messageNumber);
        matches=0;
        rq=xs->outgoingRequests;
        while(rq) {
          DBG_INFO(0,
                   "Checking this:"
                   " msgLayer id  : %d / %d\n"
                   " dialog id    : %d / %d\n"
                   " MsgNumber: %d / %d"
                   " segmentNumber: %d / %d",
                   msgLayerId, rq->msgLayerId,
                   dialogId, rq->dialogId,
                   messageNumber, rq->messageNumber,
                   segmentNumber, rq->segmentNumber);

          if ((msgLayerId==rq->msgLayerId) &&
              (dialogId==rq->dialogId) &&
              (messageNumber==rq->messageNumber)) {
            if ((segmentNumber==rq->segmentNumber) ||
                segmentNumber==0) {
              GWEN_DB_NODE *dgr;

              /* found a matching request*/
              DBG_NOTICE(0, "Found a request for group \"%s\"",
                         GWEN_DB_GroupName(gr));
              matches++;

              dgr=GWEN_DB_Group_new(GWEN_DB_GroupName(gr));
              /* add security group */
              if (GWEN_IPCXMLService_AddSecurityGroup(hmsg, dgr)) {
                DBG_INFO(0, "here");
                GWEN_DB_Group_free(dgr);
                return GWEN_Error_new(0,
                                      GWEN_ERROR_SEVERITY_ERR,
                                      GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                                      GWEN_IPC_ERROR_BAD_MSG);
              }
              GWEN_DB_SetIntValue(dgr,
                                  GWEN_DB_FLAGS_DEFAULT,
                                  "security/segmentnumber",
                                  GWEN_DB_GetIntValue(gr, "head/seq", 0, 0));
              GWEN_DB_SetIntValue(dgr,
                                  GWEN_DB_FLAGS_DEFAULT,
                                  "security/msgnum",
                                  messageNumber);

              /* copy response */
              GWEN_DB_AddGroup(dgr, GWEN_DB_Group_dup(gr));

              /* append data */
              GWEN_DB_AddGroup(rq->db, dgr);

              /*DBG_INFO(0, "Request data so far:");
               GWEN_DB_Dump(rq->db, stderr, 2); */
            } /* if segment number matches, too */
          } /* if request matches so far */

          rq=rq->next;
        } /* while */
        if (!matches) {
          DBG_ERROR(0, "Got an unrequested response, dismissing.");
          return GWEN_Error_new(0,
                                GWEN_ERROR_SEVERITY_ERR,
                                GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                                GWEN_IPC_ERROR_INVALID);
        }
      } /* if messageNumber */
      else {
        GWEN_IPCXMLREQUEST *rq;
        GWEN_DB_NODE *dgr;

        /* create request, fill it */
        DBG_INFO(0, "Got an incoming request");
        rq=GWEN_IPCXMLRequest_new();
        GWEN_IPCXMLRequest_SetMsgLayerId(rq,
                                         GWEN_HBCIMsg_GetMsgLayerId(hmsg));

        GWEN_IPCXMLRequest_SetDialogId(rq,
                                       GWEN_HBCIMsg_GetDialogNumber(hmsg));
        GWEN_IPCXMLRequest_SetMessageNumber(rq,
                                            GWEN_HBCIMsg_GetMsgNumber(hmsg));
        GWEN_IPCXMLRequest_SetSegmentNumber(rq,
                                            GWEN_DB_GetIntValue(gr,
                                                                "head/seq",
                                                                0, 0));
        /* add data to it */
        dgr=GWEN_DB_Group_new(GWEN_DB_GroupName(gr));

        /* add security group */
        if (GWEN_IPCXMLService_AddSecurityGroup(hmsg, dgr)) {
          DBG_INFO(0, "here");
          GWEN_DB_Group_free(dgr);
          GWEN_IPCXMLRequest_free(rq);
          return GWEN_Error_new(0,
                                GWEN_ERROR_SEVERITY_ERR,
                                GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                                GWEN_IPC_ERROR_BAD_MSG);
        }
        GWEN_DB_SetIntValue(dgr,
                            GWEN_DB_FLAGS_DEFAULT,
                            "security/segmentnumber",
                            GWEN_DB_GetIntValue(gr, "head/seq", 0, 0));

        /* copy response */
        GWEN_DB_AddGroup(dgr, GWEN_DB_Group_dup(gr));

        /* append data */
        GWEN_DB_AddGroup(rq->db, dgr);

        /*DBG_INFO(0, "Request data so far:");
         GWEN_DB_Dump(rq->db, stderr, 2);*/

        /* enqueue incoming request */
        GWEN_LIST_ADD(GWEN_IPCXMLREQUEST, rq,
                      &(xs->incomingRequests));

      }
    } /* if not a special group */
    gr=GWEN_DB_GetNextGroup(gr);
  } /* while */

  GWEN_DB_Group_free(db);
  return 0;
}



const char *GWEN_IPCXMLService_GetLocalName(GWEN_IPCXMLSERVICE *xs,
                                            unsigned int clid){
  GWEN_IPCCONNLAYER *cl;

  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return 0;
  }

  return GWEN_IPCXMLConnLayer_GetLocalName(cl);
}



void GWEN_IPCXMLService_SetLocalName(GWEN_IPCXMLSERVICE *xs,
                                     unsigned int clid,
                                     const char *n){
  GWEN_IPCCONNLAYER *cl;

  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return;
  }

  GWEN_IPCXMLConnLayer_SetLocalName(cl, n);
}



const char *GWEN_IPCXMLService_GetRemoteName(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int clid){
  GWEN_IPCCONNLAYER *cl;

  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return 0;
  }

  return GWEN_IPCXMLConnLayer_GetRemoteName(cl);
}



void GWEN_IPCXMLService_SetRemoteName(GWEN_IPCXMLSERVICE *xs,
                                      unsigned int clid,
                                      const char *s){
  GWEN_IPCCONNLAYER *cl;

  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return;
  }

  GWEN_IPCXMLConnLayer_SetRemoteName(cl, s);
}



GWEN_IPCXMLREQUEST *GWEN_IPCXMLService_GetInRequest(GWEN_IPCXMLSERVICE *xs,
                                                    unsigned int id) {
  GWEN_IPCXMLREQUEST *rq;

  rq=xs->incomingRequests;
  while(rq) {
    if (rq->id==id)
      return rq;
    rq=rq->next;
  } /* while */
  return 0;
}



GWEN_IPCXMLREQUEST *GWEN_IPCXMLService_GetOutRequest(GWEN_IPCXMLSERVICE *xs,
                                                     unsigned int id) {
  GWEN_IPCXMLREQUEST *rq;

  rq=xs->outgoingRequests;
  while(rq) {
    if (rq->id==id)
      return rq;
    rq=rq->next;
  } /* while */
  return 0;
}



GWEN_ERRORCODE GWEN_IPCXMLService_AddResponse(GWEN_IPCXMLSERVICE *xs,
                                              unsigned int requestId,
                                              const char *requestName,
                                              unsigned int requestVersion,
                                              GWEN_DB_NODE *db,
                                              unsigned int flags){
  GWEN_IPCCONNLAYER *cl;
  GWEN_IPCXMLREQUEST *rq;
  GWEN_ERRORCODE err;
  GWEN_XMLNODE *node;

  assert(xs);

  rq=GWEN_IPCXMLService_GetInRequest(xs, requestId);
  if (!rq) {
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_REQUEST_NOT_FOUND);
  }

  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer,
                                      rq->msgLayerId, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", rq->msgLayerId);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_CONNECTION_NOT_FOUND);
  }

  node=GWEN_MsgEngine_FindNodeByProperty(xs->msgEngine,
                                         "SEG",
                                         "id",
                                         requestVersion,
                                         requestName);
  if (!node) {
    DBG_ERROR(0, "Request \"%s\" (version %d) not found",
              requestName, requestVersion);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_UNKNOWN_MSG);
  }

  err=GWEN_IPCXMLConnLayer_AddResponse(cl,
                                       rq,
                                       node,
                                       db,
                                       flags);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }
  return 0;
}



GWEN_ERRORCODE GWEN_IPCXMLService_Flush(GWEN_IPCXMLSERVICE *xs,
                                        unsigned int clid){
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;

  assert(xs);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_CONNECTION_NOT_FOUND);
  }

  err=GWEN_IPCXMLConnLayer_Flush(cl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }
  return 0;
}



void GWEN_IPCXMLService_Close(GWEN_IPCXMLSERVICE *xs,
                              unsigned int id,
                              unsigned int userMark,
                              int force){
  assert(xs);
  assert(xs->serviceLayer);
  GWEN_ServiceLayer_Close(xs->serviceLayer, id, userMark, force);
}



unsigned int GWEN_IPCXMLService_GetNextRequest(GWEN_IPCXMLSERVICE *xs){
  GWEN_IPCXMLREQUEST *rq;

  rq=xs->incomingRequests;
  if (!rq) {
    DBG_DEBUG(0, "No incoming request");
    return 0;
  }

  DBG_INFO(0, "Returning incoming request %d", rq->id);
  return rq->id;
}



GWEN_ERRORCODE GWEN_IPCXMLService_DeleteRequest(GWEN_IPCXMLSERVICE *xs,
                                                unsigned int requestId){
  GWEN_IPCXMLREQUEST *rq;

  rq=GWEN_IPCXMLService_GetInRequest(xs, requestId);
  if (rq) {
    GWEN_LIST_DEL(GWEN_IPCXMLREQUEST, rq, &(xs->incomingRequests));
  }
  else {
    rq=GWEN_IPCXMLService_GetOutRequest(xs, requestId);
    if (!rq) {
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_REQUEST_NOT_FOUND);
    }
    GWEN_LIST_DEL(GWEN_IPCXMLREQUEST, rq, &(xs->outgoingRequests));
  }
  return 0;
}



GWEN_DB_NODE *GWEN_IPCXMLService_GetRequestData(GWEN_IPCXMLSERVICE *xs,
                                                unsigned int requestId){
  GWEN_IPCXMLREQUEST *rq;
  GWEN_DB_NODE *gr;

  rq=GWEN_IPCXMLService_GetInRequest(xs, requestId);
  if (!rq) {
    DBG_ERROR(0, "Request %d not found", requestId);
    return 0;
  }

  assert(rq->db);
  gr=GWEN_DB_GetFirstGroup(rq->db);
  if (!gr) {
    DBG_INFO(0, "No data for request %d", requestId);
    return 0;
  }
  GWEN_DB_UnlinkGroup(gr);
  return gr;
}



GWEN_DB_NODE *GWEN_IPCXMLService_PeekResponseData(GWEN_IPCXMLSERVICE *xs,
                                                  unsigned int requestId){
  GWEN_IPCXMLREQUEST *rq;
  GWEN_DB_NODE *gr;

  rq=GWEN_IPCXMLService_GetOutRequest(xs, requestId);
  if (!rq) {
    DBG_ERROR(0, "Request %d not found", requestId);
    return 0;
  }

  assert(rq->db);
  gr=GWEN_DB_GetFirstGroup(rq->db);
  if (!gr) {
    DBG_INFO(0, "No data for request %d", requestId);
    return 0;
  }
  return gr;
}



GWEN_DB_NODE *GWEN_IPCXMLService_GetResponseData(GWEN_IPCXMLSERVICE *xs,
                                                 unsigned int requestId){
  GWEN_IPCXMLREQUEST *rq;
  GWEN_DB_NODE *gr;

  rq=GWEN_IPCXMLService_GetOutRequest(xs, requestId);
  if (!rq) {
    DBG_ERROR(0, "Request %d not found", requestId);
    return 0;
  }

  assert(rq->db);
  gr=GWEN_DB_GetFirstGroup(rq->db);
  if (!gr) {
    DBG_INFO(0, "No data for request %d", requestId);
    return 0;
  }
  GWEN_DB_UnlinkGroup(gr);
  return gr;
}



unsigned int GWEN_IPCXMLService_GetRequestConnection(GWEN_IPCXMLSERVICE *xs,
                                                     unsigned int rqid){
  GWEN_IPCXMLREQUEST *rq;

  rq=GWEN_IPCXMLService_GetInRequest(xs, rqid);
  if (!rq)
    rq==GWEN_IPCXMLService_GetOutRequest(xs, rqid);
  if (!rq) {
    DBG_ERROR(0, "Request %d not found", rqid);
    return 0;
  }
  return rq->msgLayerId;
}



void GWEN_IPCXMLService_SetConnectionUpFn(GWEN_IPCXMLSERVICE *xs,
                                          GWEN_IPCXMLSERVICE_CONNUP_FN fn){
  assert(xs);
  xs->connUpFn=fn;
}



void
GWEN_IPCXMLService_SetConnectionDownFn(GWEN_IPCXMLSERVICE *xs,
                                       GWEN_IPCXMLSERVICE_CONNDOWN_FN fn){
  assert(xs);
  xs->connDownFn=fn;
}



void GWEN_IPCXMLService_ConnectionUp(GWEN_SERVICELAYER *sl,
                                     GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLSERVICE *xs;

  DBG_INFO(0, "Up");
  assert(sl);
  assert(cl);
  xs=GWEN_ServiceLayer_GetData(sl);
  assert(xs);
  if (xs->connUpFn)
    xs->connUpFn(xs, GWEN_ConnectionLayer_GetId(cl));
  DBG_DEBUG(0, "Up: done");
}



void GWEN_IPCXMLService_ConnectionDown(GWEN_SERVICELAYER *sl,
                                       GWEN_IPCCONNLAYER *cl){
  GWEN_IPCXMLSERVICE *xs;

  DBG_INFO(0, "Down");

  assert(sl);
  assert(cl);
  xs=GWEN_ServiceLayer_GetData(sl);
  assert(xs);
  if (xs->connDownFn)
    xs->connDownFn(xs, GWEN_ConnectionLayer_GetId(cl));

  if (GWEN_ConnectionLayer_GetFlags(cl) & GWEN_IPCCONNLAYER_FLAGS_PASSIVE) {
    /* passive connection, try to remove temporary remote context */
    const char *p;

    p=GWEN_IPCXMLConnLayer_GetRemoteName(cl);
    if (!p) {
      DBG_WARN(0, "Unknown peer disconnected");
    }
    else {
      GWEN_SECCTX *ctx;
      unsigned int sid;

      ctx=GWEN_SecContextMgr_GetContext(xs->securityManager,
                                        GWEN_IPCXMLConnLayer_GetLocalName(cl),
                                        p);
      if (!ctx) {
        DBG_WARN(0, "Unknown context disconnected");
      }
      else {
        if (GWEN_SecContext_GetFlags(ctx) & GWEN_SECCTX_FLAGS_TEMP) {
          DBG_INFO(0, "Removing temporary context");
          if (GWEN_SecContextMgr_DelContext(xs->securityManager, ctx)) {
            DBG_WARN(0, "Could not remove temporary context");
          }
        } /* if temp */

        sid=GWEN_IPCXMLService_GetSessionId(xs,
                                            GWEN_ConnectionLayer_GetId(cl));
        if (sid) {
          GWEN_IPCXMLService_SetSessionId(xs,
                                          GWEN_ConnectionLayer_GetId(cl),
                                          0);
          if (GWEN_IPCXMLService_DelSession(xs, sid)) {
            DBG_WARN(0, "Could not delete session");
          }
          else {
            DBG_NOTICE(0, "Removed session for \"%s\":\"%s\"",
                       GWEN_IPCXMLConnLayer_GetLocalName(cl),
                       GWEN_IPCXMLConnLayer_GetRemoteName(cl));
          }
        }
      } /* if context found */
    } /* if remote name */

  }
  DBG_DEBUG(0, "Down: done");
}



const char *GWEN_IPCXMLService_GetServiceCode(GWEN_IPCXMLSERVICE *xs,
                                              unsigned int clid){
  GWEN_IPCCONNLAYER *cl;

  assert(xs);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return 0;
  }
  return GWEN_IPCXMLConnLayer_GetServiceCode(cl);
}



const GWEN_CRYPTKEY *GWEN_IPCXMLService_GetSignKey(GWEN_IPCXMLSERVICE *xs,
                                                   unsigned int clid){
  GWEN_IPCCONNLAYER *cl;

  assert(xs);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return 0;
  }

  return GWEN_IPCXMLConnLayer_GetSignKey(cl);
}



void GWEN_IPCXMLService_SetSignKey(GWEN_IPCXMLSERVICE *xs,
                                   unsigned int clid,
                                   const GWEN_CRYPTKEY *key){
  GWEN_IPCCONNLAYER *cl;

  assert(xs);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return;
  }

  GWEN_IPCXMLConnLayer_SetSignKey(cl, key);
}



const GWEN_CRYPTKEY *GWEN_IPCXMLService_GetCryptKey(GWEN_IPCXMLSERVICE *xs,
                                                    unsigned int clid){
  GWEN_IPCCONNLAYER *cl;

  assert(xs);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return 0;
  }

  return GWEN_IPCXMLConnLayer_GetCryptKey(cl);
}



void GWEN_IPCXMLService_SetCryptKey(GWEN_IPCXMLSERVICE *xs,
                                    unsigned int clid,
                                    const GWEN_CRYPTKEY *key){
  GWEN_IPCCONNLAYER *cl;

  assert(xs);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return;
  }

  GWEN_IPCXMLConnLayer_SetCryptKey(cl, key);
}



unsigned int GWEN_IPCXMLService_GetConnectionFlags(GWEN_IPCXMLSERVICE *xs,
                                                   unsigned int clid){
  GWEN_IPCCONNLAYER *cl;

  assert(xs);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return 0;
  }

  return GWEN_ConnectionLayer_GetFlags(cl);
}



void GWEN_IPCXMLService_SetConnectionFlags(GWEN_IPCXMLSERVICE *xs,
                                           unsigned int clid,
                                           unsigned int flags){
  GWEN_IPCCONNLAYER *cl;

  assert(xs);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return;
  }

  GWEN_ConnectionLayer_SetFlags(cl, flags);
}



GWEN_ERRORCODE GWEN_IPCXMLService_GetContext(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int clid,
                                             const char *rname,
                                             GWEN_SECCTX **pctx){
  GWEN_IPCCONNLAYER *cl;
  GWEN_SECCTX *ctx;

  assert(xs);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_CONNECTION_NOT_FOUND);
  }

  DBG_INFO(0, "Looking for context for connection %s",
           GWEN_IPCXMLConnLayer_GetLocalName(cl));
  ctx=GWEN_SecContextMgr_GetContext(xs->securityManager,
                                    GWEN_IPCXMLConnLayer_GetLocalName(cl),
                                    rname);
  if (!ctx) {
    DBG_INFO(0, "Context not found");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_INVALID);
  }

  *pctx=ctx;
  return 0;
}



GWEN_ERRORCODE GWEN_IPCXMLService_ReleaseContext(GWEN_IPCXMLSERVICE *xs,
                                                 unsigned int clid,
                                                 GWEN_SECCTX *ctx,
                                                 int aban){
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;

  assert(xs);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_CONNECTION_NOT_FOUND);
  }

  err=GWEN_SecContextMgr_ReleaseContext(xs->securityManager, ctx, aban);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }

  return 0;
}



GWEN_ERRORCODE GWEN_IPCXMLService_AddContext(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int clid,
                                             GWEN_SECCTX *ctx){
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;

  assert(xs);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_CONNECTION_NOT_FOUND);
  }

  err=GWEN_SecContextMgr_AddContext(xs->securityManager, ctx);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }

  return 0;
}



GWEN_ERRORCODE GWEN_IPCXMLService_DelContext(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int clid,
                                             GWEN_SECCTX *ctx){
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;

  assert(xs);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_CONNECTION_NOT_FOUND);
  }

  err=GWEN_SecContextMgr_DelContext(xs->securityManager, ctx);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }

  return 0;
}



GWEN_ERRORCODE GWEN_IPCXMLService_SecureOpen(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int clid,
                                             unsigned int oflags,
                                             int timeout) {
  GWEN_IPCCONNLAYER *cl;
  const GWEN_CRYPTKEY *key;
  unsigned int rqid;
  time_t startTime;
  int timeleft;
  GWEN_ERRORCODE err;
  unsigned int flags;
  unsigned int newflags;
  unsigned int rc;
  char nbuffer[128];
  const char *lp;

  assert(xs);

  startTime=time(0);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return 0;
  }

  /* disable signing, because we are about to send our sign key */
  flags=GWEN_IPCXMLService_GetSecurityFlags(xs, clid);
  newflags=flags & ~GWEN_HBCIMSG_FLAGS_SIGN;
  newflags|=GWEN_HBCIMSG_FLAGS_CRYPT;
  err=GWEN_IPCXMLService_SetSecurityFlags(xs, clid,
                                          newflags);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }

  /* -------------------------------------------------------- open session */
  rqid=GWEN_IPCXMLCmd_Request_OpenSession(xs, clid, 0,
                                          GWEN_IPCXML_REQUESTFLAGS_FLUSH);
  if (!rqid) {
    DBG_INFO(0, "here");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  if (timeout>0) {
    time_t currTime;

    currTime=time(0);
    timeleft=timeout-difftime(currTime, startTime);
  }
  else
    timeleft=timeout;
  err=GWEN_IPCXMLService_WaitForResponse(xs, rqid, timeleft);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return err;
  }
  err=GWEN_IPCXMLCmd_Result_OpenSession(xs, rqid,
                                        nbuffer, sizeof(nbuffer),
                                        0, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return err;
  }
  lp=GWEN_IPCXMLService_GetRemoteName(xs, clid);
  if (!lp) {
    DBG_WARN(0, "No remote id");
    GWEN_IPCXMLService_SetRemoteName(xs, clid, nbuffer);
  }
  if (strcasecmp(nbuffer,
                 GWEN_IPCXMLService_GetRemoteName(xs, clid))!=0) {
    DBG_ERROR(0, "Unexpected server id (%s)", nbuffer);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  GWEN_IPCXMLService_DeleteRequest(xs, rqid);

  /* ------------------------------------------------------- send sign key */
  if (oflags & GWEN_IPCXMLSERVICE_OPENFLAG_SENDSIGNKEY) {
    DBG_INFO(0, "Sending sign key");
    key=GWEN_IPCXMLService_GetSignKey(xs, clid);
    if (!key) {
      DBG_INFO(0, "No local sign key");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_INVALID);
    }

    /*DBG_INFO(0, "Sending key:");
    GWEN_KeySpec_Dump(GWEN_CryptKey_GetKeySpec(key), stderr, 2);*/

    rqid=GWEN_IPCXMLCmd_Request_SendPubKey(xs, clid,
                                           GWEN_IPCXML_REQUESTFLAGS_FLUSH,
                                           key);
    if (!rqid) {
      DBG_INFO(0, "here");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_GENERIC);
    }
    if (timeout>0) {
      time_t currTime;
  
      currTime=time(0);
      timeleft=timeout-difftime(currTime, startTime);
    }
    else
      timeleft=timeout;
    err=GWEN_IPCXMLService_WaitForResponse(xs, rqid, timeleft);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(0, err);
      GWEN_IPCXMLService_DeleteRequest(xs, rqid);
      return err;
    }
    DBG_INFO(0, "Checking for response to SendPubKey (sign)");
    err=GWEN_IPCXMLCmd_Result_SendPubKey(xs, rqid, &rc);
    if (!GWEN_Error_IsOk(err)) {
      GWEN_IPCXMLService_DeleteRequest(xs, rqid);
      DBG_INFO_ERR(0, err);
      return err;
    }
    else {
      if (GWEN_IPCXML_RESULT_IS_OK(rc)) {
        DBG_INFO(0, "Server accepted sign key");
      }
      else {
        DBG_ERROR(0, "Server did not accept sign key: %d", rc);
        GWEN_IPCXMLService_DeleteRequest(xs, rqid);
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                              GWEN_IPC_ERROR_GENERIC);
      }
      GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    }
  }

  /* ------------------------------------------------------ send crypt key */
  if (oflags & GWEN_IPCXMLSERVICE_OPENFLAG_SENDCRYPTKEY) {
    DBG_INFO(0, "Sending crypt key");
    key=GWEN_IPCXMLService_GetCryptKey(xs, clid);
    if (!key) {
      DBG_INFO(0, "No local crypt key");
      GWEN_IPCXMLService_DeleteRequest(xs, rqid);
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_INVALID);
    }

    /*DBG_INFO(0, "Sending key:");
     GWEN_KeySpec_Dump(GWEN_CryptKey_GetKeySpec(key), stderr, 2);*/

    rqid=GWEN_IPCXMLCmd_Request_SendPubKey(xs, clid,
                                           GWEN_IPCXML_REQUESTFLAGS_FLUSH,
                                           key);
    if (!rqid) {
      DBG_INFO(0, "here");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_GENERIC);
    }
    if (timeout>0) {
      time_t currTime;

      currTime=time(0);
      timeleft=timeout-difftime(currTime, startTime);
    }
    else
      timeleft=timeout;
    err=GWEN_IPCXMLService_WaitForResponse(xs, rqid, timeleft);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(0, err);
      GWEN_IPCXMLService_DeleteRequest(xs, rqid);
      return err;
    }
    err=GWEN_IPCXMLCmd_Result_SendPubKey(xs, rqid, &rc);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(0, err);
      GWEN_IPCXMLService_DeleteRequest(xs, rqid);
      return err;
    }
    else {
      if (GWEN_IPCXML_RESULT_IS_OK(rc)) {
        DBG_INFO(0, "Server accepted crypt key");
      }
      else {
        DBG_ERROR(0, "Server did not accept crypt key: %d", rc);
        GWEN_IPCXMLService_DeleteRequest(xs, rqid);
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                              GWEN_IPC_ERROR_GENERIC);
      }
      GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    }
  }

  /* ----------------------------------------------- switch to secure mode */
  /* enable signing */
  newflags|=GWEN_HBCIMSG_FLAGS_SIGN;
  err=GWEN_IPCXMLService_SetSecurityFlags(xs, clid,
                                          newflags);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }

  rqid=GWEN_IPCXMLCmd_Request_Secure(xs, clid,
                                     GWEN_IPCXML_REQUESTFLAGS_FLUSH);
  if (!rqid) {
    DBG_INFO(0, "here");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  if (timeout>0) {
    time_t currTime;

    currTime=time(0);
    timeleft=timeout-difftime(currTime, startTime);
  }
  else
    timeleft=timeout;
  err=GWEN_IPCXMLService_WaitForResponse(xs, rqid, timeleft);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return err;
  }

  DBG_INFO(0, "Checking for response to Secure");
  err=GWEN_IPCXMLCmd_Result_Secure(xs, rqid, &rc);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return err;
  }
  if (GWEN_IPCXML_RESULT_IS_OK(rc)) {
    DBG_INFO(0, "Session secured");
  }
  else {
    DBG_ERROR(0, "Session not secured: %d", rc);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  GWEN_IPCXMLService_DeleteRequest(xs, rqid);

  DBG_NOTICE(0, "Connection open and secured");
  return 0;
}



GWEN_ERRORCODE GWEN_IPCXMLService_AnonymousOpen(GWEN_IPCXMLSERVICE *xs,
                                                unsigned int clid,
                                                unsigned int oflags,
                                                int timeout) {
  GWEN_IPCCONNLAYER *cl;
  GWEN_CRYPTKEY *newsignkey;
  GWEN_CRYPTKEY *newcryptkey;
  unsigned int rqid;
  time_t startTime;
  int timeleft;
  GWEN_ERRORCODE err;
  unsigned int flags;
  unsigned int newflags;
  unsigned int rc;
  char snbuffer[64];
  char cnbuffer[64];
  const char *lp;
  GWEN_SECCTX *sc;

  assert(xs);

  startTime=time(0);
  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return 0;
  }

  /* disable signing, because we are about to send our sign key */
  flags=GWEN_IPCXMLService_GetSecurityFlags(xs, clid);
  newflags=flags;
  newflags&=~GWEN_HBCIMSG_FLAGS_SIGN;
  newflags&=~GWEN_HBCIMSG_FLAGS_CRYPT;
  err=GWEN_IPCXMLService_SetSecurityFlags(xs, clid,
                                          newflags);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }

  /* -------------------------------------------------------- open session */
  rqid=
    GWEN_IPCXMLCmd_Request_OpenSession(xs, clid,
                                       "anonymous",
                                       GWEN_IPCXML_REQUESTFLAGS_FLUSH);
  if (!rqid) {
    DBG_INFO(0, "here");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  if (timeout>0) {
    time_t currTime;

    currTime=time(0);
    timeleft=timeout-difftime(currTime, startTime);
  }
  else
    timeleft=timeout;
  err=GWEN_IPCXMLService_WaitForResponse(xs, rqid, timeleft);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return err;
  }

  err=GWEN_IPCXMLCmd_Result_OpenSession(xs, rqid,
                                        snbuffer, sizeof(snbuffer),
                                        cnbuffer, sizeof(cnbuffer));
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return err;
  }
  if (*snbuffer==0) {
    DBG_ERROR(0, "Peer did not send its id");
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_MSG);
  }

  lp=GWEN_IPCXMLService_GetRemoteName(xs, clid);
  if (!lp) {
    DBG_WARN(0, "No remote id");
    GWEN_IPCXMLService_SetRemoteName(xs, clid, snbuffer);
  }
  if (strcasecmp(snbuffer,
                 GWEN_IPCXMLService_GetRemoteName(xs, clid))!=0) {
    DBG_ERROR(0, "Unexpected server id (%s)", snbuffer);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  GWEN_IPCXMLService_DeleteRequest(xs, rqid);

  /* set new local id */
  if (*cnbuffer==0) {
    DBG_ERROR(0, "Peer did not send a new id");
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_BAD_MSG);
  }
  GWEN_IPCXMLService_SetLocalName(xs, clid, cnbuffer);

  /* create new temporary context */
  DBG_INFO(0,
           "Creating temporary context for id \"%s\":\"%s\"",
           cnbuffer, snbuffer);
  sc=GWEN_IPCXMLSecCtx_new(cnbuffer, snbuffer);
  assert(sc);
  if (GWEN_IPCXMLSecCtxMgr_LoadKeys(xs->securityManager, sc, 0,
                                    snbuffer)) {
    DBG_WARN(0, "We have no keys for peer \"%s\"", snbuffer);
    DBG_INFO(0, "Enter:");
    getchar();
  }
  GWEN_SecContext_SetFlags(sc, GWEN_SECCTX_FLAGS_TEMP);

  newsignkey=GWEN_CryptKey_Factory("RSA");
  if (!newsignkey) {
    DBG_ERROR(0, "Could not create RSA key");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  GWEN_CryptKey_SetOwner(newsignkey, cnbuffer);
  GWEN_CryptKey_SetKeyName(newsignkey, "S");
  err=GWEN_CryptKey_Generate(newsignkey, 768);
  if (!GWEN_Error_IsOk(err)) {
    GWEN_CryptKey_free(newsignkey);
    DBG_INFO_ERR(0, err);
    return err;
  }
  GWEN_IPCXMLSecCtx_SetLocalSignKey(sc, newsignkey);

  newcryptkey=GWEN_CryptKey_Factory("RSA");
  if (!newcryptkey) {
    DBG_ERROR(0, "Could not create RSA key");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  GWEN_CryptKey_SetOwner(newcryptkey, cnbuffer);
  GWEN_CryptKey_SetKeyName(newcryptkey, "V");
  err=GWEN_CryptKey_Generate(newcryptkey, 768);
  if (!GWEN_Error_IsOk(err)) {
    GWEN_CryptKey_free(newcryptkey);
    DBG_INFO_ERR(0, err);
    return err;
  }
  GWEN_IPCXMLSecCtx_SetLocalCryptKey(sc, newcryptkey);

  if (GWEN_SecContextMgr_AddContext(xs->securityManager, sc)) {
    DBG_INFO(0,
             "Could not add temporary context for id \"%s\":\"%s\"",
             cnbuffer, snbuffer);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    GWEN_SecContext_free(sc);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }


  /* -------------------------------------------------- turn on encryption */
  newflags|=GWEN_HBCIMSG_FLAGS_CRYPT;
  err=GWEN_IPCXMLService_SetSecurityFlags(xs, clid,
                                          newflags);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }

  /* --------------------------------------------- create and send sign key */
  DBG_INFO(0, "Creating and sending sign key");

  /*DBG_INFO(0, "Sending key:");
   GWEN_KeySpec_Dump(GWEN_CryptKey_GetKeySpec(newsignkey), stderr, 2);*/

  rqid=GWEN_IPCXMLCmd_Request_SendPubKey(xs, clid,
                                         GWEN_IPCXML_REQUESTFLAGS_FLUSH,
                                         newsignkey);
  GWEN_CryptKey_free(newsignkey);
  if (!rqid) {
    DBG_INFO(0, "here");
    GWEN_CryptKey_free(newcryptkey);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  if (timeout>0) {
    time_t currTime;

    currTime=time(0);
    timeleft=timeout-difftime(currTime, startTime);
  }
  else
    timeleft=timeout;
  err=GWEN_IPCXMLService_WaitForResponse(xs, rqid, timeleft);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    GWEN_CryptKey_free(newcryptkey);
    return err;
  }
  DBG_INFO(0, "Checking for response to SendPubKey (sign)");
  err=GWEN_IPCXMLCmd_Result_SendPubKey(xs, rqid, &rc);
  if (!GWEN_Error_IsOk(err)) {
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    DBG_INFO_ERR(0, err);
    GWEN_CryptKey_free(newcryptkey);
    return err;
  }
  else {
    if (GWEN_IPCXML_RESULT_IS_OK(rc)) {
      DBG_INFO(0, "Server accepted sign key");
    }
    else {
      DBG_ERROR(0, "Server did not accept sign key: %d", rc);
      GWEN_IPCXMLService_DeleteRequest(xs, rqid);
      GWEN_CryptKey_free(newcryptkey);
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_GENERIC);
    }
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
  }

  /* ------------------------------------------------------ send crypt key */
  DBG_INFO(0, "Creating and sending crypt key");
  /*DBG_INFO(0, "Sending key:");
   GWEN_KeySpec_Dump(GWEN_CryptKey_GetKeySpec(newcryptkey), stderr, 2);*/

  rqid=GWEN_IPCXMLCmd_Request_SendPubKey(xs, clid,
                                         GWEN_IPCXML_REQUESTFLAGS_FLUSH,
                                         newcryptkey);
  GWEN_CryptKey_free(newcryptkey);
  if (!rqid) {
    DBG_INFO(0, "here");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  if (timeout>0) {
    time_t currTime;

    currTime=time(0);
    timeleft=timeout-difftime(currTime, startTime);
  }
  else
    timeleft=timeout;
  err=GWEN_IPCXMLService_WaitForResponse(xs, rqid, timeleft);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return err;
  }
  err=GWEN_IPCXMLCmd_Result_SendPubKey(xs, rqid, &rc);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return err;
  }
  else {
    if (GWEN_IPCXML_RESULT_IS_OK(rc)) {
      DBG_INFO(0, "Server accepted crypt key");
    }
    else {
      DBG_ERROR(0, "Server did not accept crypt key: %d", rc);
      GWEN_IPCXMLService_DeleteRequest(xs, rqid);
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                            GWEN_IPC_ERROR_GENERIC);
    }
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
  }

  /* ----------------------------------------------- switch to secure mode */
  /* enable signing */
  newflags|=GWEN_HBCIMSG_FLAGS_SIGN;
  err=GWEN_IPCXMLService_SetSecurityFlags(xs, clid,
                                          newflags);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }

  rqid=GWEN_IPCXMLCmd_Request_Secure(xs, clid,
                                     GWEN_IPCXML_REQUESTFLAGS_FLUSH);
  if (!rqid) {
    DBG_INFO(0, "here");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  if (timeout>0) {
    time_t currTime;

    currTime=time(0);
    timeleft=timeout-difftime(currTime, startTime);
  }
  else
    timeleft=timeout;
  err=GWEN_IPCXMLService_WaitForResponse(xs, rqid, timeleft);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return err;
  }

  DBG_INFO(0, "Checking for response to Secure");
  err=GWEN_IPCXMLCmd_Result_Secure(xs, rqid, &rc);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return err;
  }
  if (GWEN_IPCXML_RESULT_IS_OK(rc)) {
    DBG_INFO(0, "Session secured");
  }
  else {
    DBG_ERROR(0, "Session not secured: %d", rc);
    GWEN_IPCXMLService_DeleteRequest(xs, rqid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }
  GWEN_IPCXMLService_DeleteRequest(xs, rqid);

  DBG_NOTICE(0, "Connection open and secured");
  return 0;
}



GWEN_ERRORCODE GWEN_IPCXMLService_WaitForResponse(GWEN_IPCXMLSERVICE *xs,
                                                  unsigned int rqid,
                                                  int timeout) {
  time_t startTime;

  startTime=time(0);
  for (;;) {
    time_t currTime;
    GWEN_ERRORCODE err;

    /* work */
    err=GWEN_IPCXMLService_Work(xs, (timeout>-1)?timeout*1000:timeout);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(0, err);
      return err;
    }

    /* distribute messages, if any */
    err=GWEN_IPCXMLService_HandleMsgs(xs, 0, 2); /* no userMark, 2 msgs max */
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(0, err);
      return err;
    }

    if (GWEN_IPCXMLService_PeekResponseData(xs, rqid)) {
      DBG_INFO(0, "Got a response for request %d", rqid);
      return 0;
    }
    if (timeout>-1) {
      currTime=time(0);
      if ((int)difftime(currTime, startTime)>timeout) {
        DBG_ERROR(0, "Timeout while waiting for responses");
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                              GWEN_IPC_ERROR_INQUEUE_EMPTY);
      }
    } /* if timeout */
  } /* for */
  /* never reaches this point */
}
















GWEN_IPCXMLSESSION *GWEN_IPCXMLSession_new(const char *lname,
                                           const char *rname){
  GWEN_IPCXMLSESSION *s;

  GWEN_NEW_OBJECT(GWEN_IPCXMLSESSION, s);
  if (lname)
    s->localName=strdup(lname);
  if (rname)
    s->remoteName=strdup(rname);
  s->sessionId=++gwen_ipcxmlsession_lastid;
  return s;
}



void GWEN_IPCXMLSession_free(GWEN_IPCXMLSESSION *s){
  if (s) {
    free(s->localName);
    free(s->remoteName);
    free(s);
  }
}



const char *GWEN_IPCXMLSession_GetLocalName(GWEN_IPCXMLSESSION *s){
  assert(s);
  return s->localName;
}



void GWEN_IPCXMLSession_SetLocalName(GWEN_IPCXMLSESSION *s,
                                     const char *n){
  assert(s);
  assert(n);
  free(s->localName);
  s->localName=strdup(n);
}



void GWEN_IPCXMLSession_SetRemoteName(GWEN_IPCXMLSESSION *s,
                                      const char *n){
  assert(s);
  assert(n);
  free(s->remoteName);
  s->remoteName=strdup(n);
}


const char *GWEN_IPCXMLSession_GetRemoteName(GWEN_IPCXMLSESSION *s){
  assert(s);
  return s->remoteName;
}



GWEN_IPCXMLSESSION *GWEN_IPCXMLService_FindSession(GWEN_IPCXMLSERVICE *xs,
                                                   const char *lname,
                                                   const char *rname){
  GWEN_IPCXMLSESSION *curr;

  assert(xs);
  assert(lname);
  assert(rname);
  curr=xs->sessions;
  while(curr) {
    if (strcasecmp(curr->localName, lname)==0 &&
        strcasecmp(curr->remoteName, rname)==0) {
      return curr;
    }
    curr=curr->next;
  } /* while */
  return 0;
}



unsigned int GWEN_IPCXMLService_AddSession(GWEN_IPCXMLSERVICE *xs,
                                           const char *lname,
                                           const char *rname){
  GWEN_IPCXMLSESSION *s;

  assert(xs);
  assert(lname);
  assert(rname);

  s=GWEN_IPCXMLService_FindSession(xs, lname, rname);
  if (s) {
    DBG_INFO(0, "Session \"%s\":\"%s\" already exists",
             lname, rname);
    return 0;
  }
  s=GWEN_IPCXMLSession_new(lname, rname);
  GWEN_LIST_ADD(GWEN_IPCXMLSESSION, s, &(xs->sessions));
  return s->sessionId;
}



int GWEN_IPCXMLService_DelSession(GWEN_IPCXMLSERVICE *xs,
                                  unsigned int sid) {
  GWEN_IPCXMLSESSION *curr;

  assert(xs);
  curr=xs->sessions;
  while(curr) {
    if (curr->sessionId==sid) {
      GWEN_LIST_DEL(GWEN_IPCXMLSESSION, curr, &(xs->sessions));
      GWEN_IPCXMLSession_free(curr);
      return 0;
    }
    curr=curr->next;
  } /* while */
  return -1;
}



int GWEN_IPCXMLService_HasSession(GWEN_IPCXMLSERVICE *xs,
                                  const char *lname,
                                  const char *rname){
  return (GWEN_IPCXMLService_FindSession(xs, lname, rname)!=0);
}






