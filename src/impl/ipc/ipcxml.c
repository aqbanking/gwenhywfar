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
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/ipcxmlmsglayer.h>
#include <gwenhyfwar/ipcxmlconnlayer.h>
#include <gwenhyfwar/ipc.h>


static unsigned int GWEN_IPCXMLRequest_LastId=0;


GWEN_IPCXMLREQUEST *GWEN_IPCXMLRequest_new(){
  GWEN_IPCXMLREQUEST *r;

  GWEN_NEW_OBJECT(GWEN_IPCXMLREQUEST, r);
  r->id=++GWEN_IPCXMLRequest_LastId;
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
  cl=GWEN_IPCXMLConnLayer_new(xs->msgEngine,
                              localContext,
                              xs->securityManager,
                              ml,
                              !(flags & GWEN_IPCCONNLAYER_FLAGS_PASSIVE));

  cid=GWEN_ConnectionLayer_GetId(cl);
  GWEN_ConnectionLayer_SetUserMark(cl, userMark);
  GWEN_ConnectionLayer_SetFlags(cl, flags);

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
                                          const char *localContext,
                                          unsigned int userMark,
                                          const char *addr,
                                          unsigned int port,
                                          unsigned int flags){
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;

  cl=GWEN_IPCXMLService_CreateCL(xs, st, localContext, userMark, addr, port,
                                 flags & ~GWEN_IPCCONNLAYER_FLAGS_PASSIVE);
  err=GWEN_ServiceLayer_AddConnection(xs->serviceLayer, cl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_ConnectionLayer_free(cl);
    return 0;
  }

  return GWEN_ConnectionLayer_GetId(cl);
}





unsigned int GWEN_IPCXMLService_AddRequest(GWEN_IPCXMLSERVICE *xs,
                                           unsigned int clid,
                                           const char *requestName,
                                           unsigned int requestVersion,
                                           GWEN_DB_NODE *db,
                                           int flush){
  GWEN_IPCCONNLAYER *cl;
  GWEN_XMLNODE *node;
  GWEN_IPCXMLREQUEST *rq;

  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
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

  rq=GWEN_IPCXMLConnLayer_AddRequest(cl, node, db, flush);
  if (!rq) {
    DBG_ERROR(0, "Could not add request \"%s\" (version %d)",
              requestName, requestVersion);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }

  GWEN_LIST_ADD(GWEN_IPCXMLREQUEST, rq, &(xs->outgoingRequests));
  return rq->id;
}



GWEN_ERRORCODE GWEN_IPCXMLService_SetSecurityEnv(GWEN_IPCXMLSERVICE *xs,
                                                 unsigned int clid,
                                                 const GWEN_KEYSPEC *signer,
                                                 const GWEN_KEYSPEC *crypter){
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

  err=GWEN_IPCXMLConnLayer_SetSecurityEnv(cl, signer, crypter);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }
  return 0;
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
                                             int maxmsgs){
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
    DBG_NOTICE(0, "Handling message %d", msgs);
    imsg=GWEN_ServiceLayer_NextIncomingMsg(xs->serviceLayer,
                                           userMark);
    if (!imsg) {
      if (msgs) {
        DBG_INFO(0, "No more messages");
        break;
      }
      DBG_INFO(0, "No message");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
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

        matches=0;
        rq=xs->incomingRequests;
        while(rq) {
          if ((rq->msgLayerId==rq->msgLayerId) &&
              (dialogId==rq->dialogId) &&
              (messageNumber==rq->messageNumber)) {
            if ((segmentNumber==rq->segmentNumber) ||
                segmentNumber==0) {
              GWEN_DB_NODE *dgr;

              /* found a matching request*/
              DBG_NOTICE(0, "Found a request for group \"%s\"",
                         GWEN_DB_GroupName(gr));
              matches++;

              dgr=GWEN_DB_Group_new("data");
              /* add security group */
              if (GWEN_IPCXMLService_AddSecurityGroup(hmsg, dgr)) {
                DBG_INFO(0, "here");
                GWEN_DB_Group_free(dgr);
                return -1;
              }

              /* copy response */
              GWEN_DB_AddGroup(dgr, GWEN_DB_Group_dup(gr));

              /* append data */
              GWEN_DB_AddGroup(rq->db, dgr);

              DBG_INFO(0, "Request data so far:");
              GWEN_DB_Dump(rq->db, stderr, 2);
            } /* if segment number matches, too */
          } /* if request matches so far */

          rq=rq->next;
        } /* while */
        if (!matches) {
          DBG_ERROR(0, "Got an unrequested response, dismissing.");
          return -1;
        }
      } /* if messageNumber */
      else {
        GWEN_IPCXMLREQUEST *rq;
        GWEN_DB_NODE *dgr;

        /* create request, fill it */
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
        dgr=GWEN_DB_Group_new("data");

        /* add security group */
        if (GWEN_IPCXMLService_AddSecurityGroup(hmsg, dgr)) {
          DBG_INFO(0, "here");
          GWEN_DB_Group_free(dgr);
          GWEN_IPCXMLRequest_free(rq);
          return -1;
        }

        /* copy response */
        GWEN_DB_AddGroup(dgr, GWEN_DB_Group_dup(gr));

        /* append data */
        GWEN_DB_AddGroup(rq->db, dgr);

        DBG_INFO(0, "Request data so far:");
        GWEN_DB_Dump(rq->db, stderr, 2);

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







