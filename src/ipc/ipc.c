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

#include "ipc_p.h"
#include <gwenhywfar/net.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/nettransportsock.h>
#include <gwenhywfar/nettransportssl.h>


GWEN_LIST_FUNCTIONS(GWEN_IPCNODE, GWEN_IPCNode);
GWEN_LIST_FUNCTIONS(GWEN_IPCMSG, GWEN_IPCMsg);
GWEN_LIST_FUNCTIONS(GWEN_IPCREQUEST, GWEN_IPCRequest);


static GWEN_TYPE_UINT32 gwen_ipc__lastid=0;



/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Node
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


/* -------------------------------------------------------------- FUNCTION */
GWEN_IPCNODE *GWEN_IPCNode_new(){
  GWEN_IPCNODE *n;

  GWEN_NEW_OBJECT(GWEN_IPCNODE, n);
  GWEN_LIST_INIT(GWEN_IPCNODE, n);
  n->id=++gwen_ipc__lastid;
  n->usage=1;
  return n;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IPCNode_free(GWEN_IPCNODE *n){
  if (n) {
    assert(n->usage);
    if (--(n->usage)==0) {
      GWEN_NetConnection_free(n->connection);
      free(n->baseAuth);
      GWEN_LIST_FINI(GWEN_IPCNODE, n);
      GWEN_FREE_OBJECT(n);
    }
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IPCNode_Attach(GWEN_IPCNODE *n){
  assert(n);
  n->usage++;
}










/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Message
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

/* -------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_IPCMsg_new(GWEN_IPCNODE *n){
  GWEN_IPCMSG *m;

  assert(n);
  GWEN_NEW_OBJECT(GWEN_IPCMSG, m);
  GWEN_LIST_INIT(GWEN_IPCMSG, m);
  m->node=n;
  GWEN_IPCNode_Attach(m->node);
  m->id=++gwen_ipc__lastid;
  return m;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IPCMsg_free(GWEN_IPCMSG *m){
  if (m) {
    GWEN_IPCNode_free(m->node);
    GWEN_DB_Group_free(m->db);
    GWEN_LIST_FINI(GWEN_IPCMSG, m);
    GWEN_FREE_OBJECT(m);
  }
}







/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Request
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


/* -------------------------------------------------------------- FUNCTION */
GWEN_IPCREQUEST *GWEN_IPCRequest_new(){
  GWEN_IPCREQUEST *r;

  GWEN_NEW_OBJECT(GWEN_IPCREQUEST, r);
  GWEN_LIST_INIT(GWEN_IPCREQUEST, r);

  r->requestMsgs=GWEN_IPCMsg_List_new();
  r->responseMsgs=GWEN_IPCMsg_List_new();
  r->id=++gwen_ipc__lastid;
  return r;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IPCRequest_free(GWEN_IPCREQUEST *r){
  if (r) {
    GWEN_IPCMsg_List_free(r->responseMsgs);
    GWEN_IPCMsg_List_free(r->requestMsgs);
    GWEN_LIST_FINI(GWEN_IPCREQUEST, r);
    GWEN_FREE_OBJECT(r);
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IPCRequest_AddRequestMsg(GWEN_IPCREQUEST *r, GWEN_IPCMSG *m){
  assert(r);
  assert(m);
  GWEN_IPCMsg_List_Add(m, r->requestMsgs);
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IPCRequest_AddResponseMsg(GWEN_IPCREQUEST *r, GWEN_IPCMSG *m){
  assert(r);
  assert(m);
  GWEN_IPCMsg_List_Add(m, r->responseMsgs);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_IPCRequest_HasRequestMsg(GWEN_IPCREQUEST *r, GWEN_TYPE_UINT32 id){
  GWEN_IPCMSG *m;

  assert(r);
  assert(id);

  m=GWEN_IPCMsg_List_First(r->requestMsgs);
  while(m) {
    if (m->refId==id)
      return 1;
    m=GWEN_IPCMsg_List_Next(m);
  } /* while */

  return 0;
}









/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Manager
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




/* -------------------------------------------------------------- FUNCTION */
GWEN_IPCMANAGER *GWEN_IPCManager_new(){
  GWEN_IPCMANAGER *mgr;

  GWEN_NEW_OBJECT(GWEN_IPCMANAGER, mgr);
  mgr->libId=GWEN_Net_GetLibraryId();
  mgr->nodes=GWEN_IPCNode_List_new();
  mgr->outRequests=GWEN_IPCRequest_List_new();
  mgr->newInRequests=GWEN_IPCRequest_List_new();
  mgr->oldInRequests=GWEN_IPCRequest_List_new();

  return mgr;
}




/* -------------------------------------------------------------- FUNCTION */
void GWEN_IPCManager_free(GWEN_IPCMANAGER *mgr){
  if (mgr) {
    GWEN_IPCRequest_List_free(mgr->oldInRequests);
    GWEN_IPCRequest_List_free(mgr->newInRequests);
    GWEN_IPCRequest_List_free(mgr->outRequests);
    GWEN_IPCNode_List_free(mgr->nodes);

    GWEN_FREE_OBJECT(mgr);
  }
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_IPCManager_AddServer(GWEN_IPCMANAGER *mgr,
                                           GWEN_NETTRANSPORT *tr,
                                           GWEN_TYPE_UINT32 mark){
  GWEN_NETCONNECTION *conn;
  GWEN_IPCNODE *n;

  /* create connection layer */
  conn=GWEN_NetConnectionHTTP_new(tr,
                                  1,              /* take */
                                  mgr->libId,     /* libId */
                                  1,0);           /* protocol version */
  if (GWEN_NetConnection_StartListen(conn)) {
    DBG_ERROR(0, "Could not start server");
    return 0;
  }

  n=GWEN_IPCNode_new();
  n->connection=conn;
  n->mark=mark;
  GWEN_IPCNode_List_Add(n, mgr->nodes);
  return n->id;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_IPCManager_AddClient(GWEN_IPCMANAGER *mgr,
                                           GWEN_NETTRANSPORT *tr,
                                           const char *userName,
                                           const char *passwd,
                                           GWEN_TYPE_UINT32 mark){
  GWEN_NETCONNECTION *conn;
  GWEN_IPCNODE *n;

  n=GWEN_IPCNode_new();
  if (userName) {
    GWEN_BUFFER *abuf;
    GWEN_BUFFER *ebuf;

    abuf=GWEN_Buffer_new(0, strlen(userName)+strlen(passwd)+2, 0, 1);
    GWEN_Buffer_AppendString(abuf, userName);
    GWEN_Buffer_AppendByte(abuf, ':');
    GWEN_Buffer_AppendString(abuf, passwd);
    ebuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(abuf)*4/3+16, 0, 1);
    GWEN_Buffer_AppendString(ebuf, "BASIC ");
    if (GWEN_Base64_Encode(GWEN_Buffer_GetStart(abuf),
                           GWEN_Buffer_GetUsedBytes(abuf),
                           ebuf, 0)) {
      DBG_ERROR(0, "Could not encode authorization data");
      GWEN_Buffer_free(ebuf);
      GWEN_Buffer_free(abuf);
      GWEN_IPCNode_free(n);
      return 0;
    }
    n->baseAuth=strdup(GWEN_Buffer_GetStart(ebuf));
    GWEN_Buffer_free(ebuf);
    GWEN_Buffer_free(abuf);
  }
  /* create connection layer */
  conn=GWEN_NetConnectionHTTP_new(tr,
                                  1,              /* take */
                                  mgr->libId,     /* libId */
                                  1,0);           /* protocol version */
  n->connection=conn;
  n->mark=mark;
  GWEN_IPCNode_List_Add(n, mgr->nodes);
  return n->id;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_IPCREQUEST *GWEN_IPCManager__FindRequest(GWEN_IPCMANAGER *mgr,
                                              GWEN_TYPE_UINT32 rid,
                                              GWEN_IPCREQUEST *r) {
  assert(mgr);
  assert(r);

  while(r) {
    if (r->id==rid)
      return r;
    r=GWEN_IPCRequest_List_Next(r);
  }
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_IPCManager__SendMsg(GWEN_IPCMANAGER *mgr,
                             GWEN_IPCMSG *m) {
  GWEN_BUFFER *buf;
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbCmd;
  GWEN_DB_NODE *dbVars;
  GWEN_DB_NODE *dbHeader;
  char numbuf[16];

  buf=GWEN_Buffer_new(0, 512, 0, 1);
  bio=GWEN_BufferedIO_Buffer2_new(buf, 0); /* don't relinquish buffer */

  /* encode db */
  GWEN_BufferedIO_SetWriteBuffer(bio, 0, 128);
  if (GWEN_DB_WriteToStream(m->db, bio, GWEN_DB_FLAGS_COMPACT)) {
    DBG_ERROR(0, "Could not encode db");
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    GWEN_Buffer_free(buf);
    return -1;
  }

  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_BufferedIO_free(bio);
    GWEN_Buffer_free(buf);
    return -1;
  }
  GWEN_BufferedIO_free(bio);

  dbReq=GWEN_DB_Group_new("request");
  /* prepare command */
  dbCmd=GWEN_DB_GetGroup(dbReq, GWEN_DB_FLAGS_DEFAULT, "command");
  assert(dbCmd);

  GWEN_DB_SetCharValue(dbCmd, GWEN_DB_FLAGS_DEFAULT,
                       "cmd", "put");
  GWEN_DB_SetCharValue(dbCmd, GWEN_DB_FLAGS_DEFAULT,
                       "url", mgr->application);

  /* prepare vars */
  dbVars=GWEN_DB_GetGroup(dbCmd, GWEN_DB_FLAGS_DEFAULT, "vars");
  assert(dbVars);
  snprintf(numbuf, sizeof(numbuf), "%d", m->id);
  GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_DEFAULT,
                       "id", numbuf);
  if (m->refId) {
    snprintf(numbuf, sizeof(numbuf), "%d", m->id);
    GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_DEFAULT,
                         "refid", numbuf);
  }

  /* prepare header */
  dbHeader=GWEN_DB_GetGroup(dbReq, GWEN_DB_FLAGS_DEFAULT, "header");
  assert(dbHeader);
  GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_DEFAULT,
                       "connection", "keep-alive");
  GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_DEFAULT,
                       "cache-control", "no-store");
  GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_DEFAULT,
                       "user-agent", "Gwenhywfar v"
                       GWENHYWFAR_VERSION_FULL_STRING);
  if (m->node->baseAuth)
    GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_DEFAULT,
                         "authorization", m->node->baseAuth);

  GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_DEFAULT,
                       "content-type",
                       "application/gwenipc");

  /* send message */
  if (GWEN_NetConnectionHTTP_AddRequest(m->node->connection,
                                        dbReq,
                                        buf,
                                        0)) {
    DBG_ERROR(0, "Could not send message");
    GWEN_DB_Group_free(dbReq);
  }

  DBG_INFO(0, "Message is on its way");
  m->sendTime=time(0);
  GWEN_DB_Group_free(dbReq);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_IPCManager_SendRequest(GWEN_IPCMANAGER *mgr,
                                             GWEN_TYPE_UINT32 nid,
                                             GWEN_DB_NODE *req){
  GWEN_IPCNODE *n;
  GWEN_IPCREQUEST *r;
  GWEN_IPCMSG *m;

  n=GWEN_IPCNode_List_First(mgr->nodes);
  while(n) {
    if (n->id==nid)
      break;
    n=GWEN_IPCNode_List_Next(n);
  } /* while */
  if (!n) {
    DBG_ERROR(0, "Node %08x not found", nid);
    return 0;
  }

  m=GWEN_IPCMsg_new(n);
  m->db=req;

  if (GWEN_IPCManager__SendMsg(mgr, m)) {
    DBG_ERROR(0, "Could not send request");
    GWEN_IPCMsg_free(m);
    return 0;
  }

  r=GWEN_IPCRequest_new();
  GWEN_IPCRequest_AddRequestMsg(r, m);
  GWEN_IPCRequest_List_Add(r, mgr->outRequests);
  return r->id;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_IPCManager_SendMultiRequest(GWEN_IPCMANAGER *mgr,
                                                  GWEN_TYPE_UINT32 mark,
                                                  GWEN_DB_NODE *req){
  GWEN_IPCNODE *n;
  GWEN_IPCREQUEST *r;

  r=0;
  n=GWEN_IPCNode_List_First(mgr->nodes);
  while(n) {
    if (mark==0 || mark==n->mark) {
      GWEN_NETTRANSPORT_STATUS st;

      st=GWEN_NetConnection_GetStatus(n->connection);
      if (st!=GWEN_NetTransportStatusListening &&
          st!=GWEN_NetTransportStatusDisabled) {
        GWEN_IPCMSG *m;

        m=GWEN_IPCMsg_new(n);
        m->db=GWEN_DB_Group_dup(req);

        if (GWEN_IPCManager__SendMsg(mgr, m)) {
          DBG_ERROR(0, "Could not send request to node %08x", n->id);
          GWEN_IPCMsg_free(m);
        }
        else {
          if (r==0) {
            r=GWEN_IPCRequest_new();
            GWEN_IPCRequest_List_Add(r, mgr->outRequests);
          }
          GWEN_IPCRequest_AddRequestMsg(r, m);
        }
      }
    } /* if mark matches */
    n=GWEN_IPCNode_List_Next(n);
  } /* while */

  if (r) {
    return r->id;
  }
  DBG_ERROR(0, "Could not send any request");
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_IPCManager_SendResponse(GWEN_IPCMANAGER *mgr,
                                 GWEN_TYPE_UINT32 rid,
                                 GWEN_DB_NODE *rsp){
  GWEN_IPCREQUEST *r;
  GWEN_IPCMSG *m;
  GWEN_IPCMSG *om;

  r=GWEN_IPCRequest_List_First(mgr->oldInRequests);
  while(r) {
    if (r->id==rid)
      break;
  } /* while */
  if (!r) {
    DBG_ERROR(0, "Request %08x not found", rid);
    return -1;
  }

  om=GWEN_IPCMsg_List_First(r->requestMsgs);
  assert(om);

  m=GWEN_IPCMsg_new(om->node);
  m->refId=om->id;
  m->db=rsp;

  if (GWEN_IPCManager__SendMsg(mgr, m)) {
    DBG_ERROR(0, "Could not send response");
    GWEN_IPCMsg_free(m);
    return -1;
  }

  GWEN_IPCRequest_AddResponseMsg(r, m);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_IPCManager_RemoveRequest(GWEN_IPCMANAGER *mgr,
                                  GWEN_TYPE_UINT32 rid,
                                  int inOrOut){
  GWEN_IPCREQUEST *r;

  if (inOrOut==0)
    r=GWEN_IPCRequest_List_First(mgr->oldInRequests);
  else
    r=GWEN_IPCRequest_List_First(mgr->outRequests);
  while(r) {
    if (r->id==rid)
      break;
  } /* while */
  if (!r) {
    DBG_ERROR(0, "Request %08x not found", rid);
    return -1;
  }
  GWEN_IPCRequest_List_Del(r);
  GWEN_IPCRequest_free(r);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_IPCManager_GetNextInRequest(GWEN_IPCMANAGER *mgr,
                                                  GWEN_TYPE_UINT32 mark){
  GWEN_IPCREQUEST *r;

  r=GWEN_IPCRequest_List_First(mgr->newInRequests);
  if (r) {
    GWEN_IPCRequest_List_Del(r);
    GWEN_IPCRequest_List_Add(r, mgr->oldInRequests);
    return r->id;
  }
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_DB_NODE *GWEN_IPCManager_GetInRequestData(GWEN_IPCMANAGER *mgr,
                                               GWEN_TYPE_UINT32 rid){
  GWEN_IPCREQUEST *r;
  GWEN_IPCMSG *om;

  r=GWEN_IPCRequest_List_First(mgr->oldInRequests);
  while(r) {
    if (r->id==rid)
      break;
  } /* while */
  if (!r) {
    DBG_ERROR(0, "Request %08x not found", rid);
    return 0;
  }

  om=GWEN_IPCMsg_List_First(r->requestMsgs);
  assert(om);

  return om->db;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_DB_NODE *GWEN_IPCManager_GetResponseData(GWEN_IPCMANAGER *mgr,
                                              GWEN_TYPE_UINT32 rid){
  GWEN_IPCREQUEST *r;
  GWEN_IPCMSG *m;
  GWEN_DB_NODE *db;

  r=GWEN_IPCRequest_List_First(mgr->outRequests);
  while(r) {
    if (r->id==rid)
      break;
  } /* while */
  if (!r) {
    DBG_ERROR(0, "Request %08x not found", rid);
    return 0;
  }

  m=GWEN_IPCMsg_List_First(r->responseMsgs);
  assert(m);

  db=m->db;
  assert(m->node);
  assert(m->node->id);
  m->db=0;
  GWEN_IPCMsg_List_Del(m);
  GWEN_IPCMsg_free(m);
  return db;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_IPCManager__CollectMsgs(GWEN_IPCMANAGER *mgr, int maxMsg) {
  GWEN_IPCNODE *n;
  int msgs;

  msgs=0;
  n=GWEN_IPCNode_List_First(mgr->nodes);
  while(n && (maxMsg==0 || msgs<maxMsg)) {
    GWEN_NETMSG *nm;

    nm=GWEN_NetConnection_GetInMsg(n->connection);
    if (nm) {
      GWEN_DB_NODE *dbCmd;
      GWEN_TYPE_UINT32 msgId;
      GWEN_TYPE_UINT32 refId;
      GWEN_BUFFER *mbuf;
      GWEN_TYPE_UINT32 pos;

      dbCmd=GWEN_DB_GetGroup(GWEN_NetMsg_GetDB(nm),
                             GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                             "command");
      assert(dbCmd);
      msgId=GWEN_DB_GetIntValue(dbCmd, "vars/id", 0, 0);
      refId=GWEN_DB_GetIntValue(dbCmd, "vars/refId", 0, 0);

      mbuf=GWEN_NetMsg_GetBuffer(nm);
      assert(mbuf);
      pos=GWEN_Buffer_GetBookmark(mbuf, 1);
      if (!pos || pos>=GWEN_Buffer_GetUsedBytes(mbuf)) {
        DBG_WARN(0, "Bad message received (no body)");
      }
      else {
        GWEN_BUFFEREDIO *bio;
        GWEN_DB_NODE *dbBody;

        GWEN_Buffer_SetPos(mbuf, pos);
        bio=GWEN_BufferedIO_Buffer2_new(mbuf, 1);
        GWEN_BufferedIO_SetReadBuffer(bio, 0, 128);
        dbBody=GWEN_DB_Group_new("body");

        if (GWEN_DB_ReadFromStream(dbBody, bio,
                                   GWEN_DB_FLAGS_DEFAULT|
                                   GWEN_PATH_FLAGS_CREATE_GROUP)){
          DBG_WARN(0, "Bad message received (invalid body)");
          GWEN_DB_Group_free(dbBody);
        }
        else {
          GWEN_DB_SetIntValue(dbBody, GWEN_DB_FLAGS_OVERWRITE_VARS,
                              "ipc/nodeId", n->id);
          GWEN_DB_SetIntValue(dbBody, GWEN_DB_FLAGS_OVERWRITE_VARS,
                              "ipc/msgId", msgId);
          /* found a valid body */
          if (refId) {
            GWEN_IPCREQUEST *r;

            /* it pretends to be a response, check it */
            GWEN_DB_SetIntValue(dbBody, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                "ipc/refId", refId);
            r=GWEN_IPCRequest_List_First(mgr->outRequests);
            while(r) {
              if (GWEN_IPCRequest_HasRequestMsg(r, refId))
                break;
              r=GWEN_IPCRequest_List_Next(r);
            } /* while r */
            if (!r) {
              DBG_WARN(0, "Got a response for invalid request (%08x)", refId);
              GWEN_DB_Group_free(dbBody);
            }
            else {
              GWEN_IPCMSG *m;

              DBG_INFO(0, "Got a response for request %08x", r->id);
              m=GWEN_IPCMsg_new(n);
              m->db=dbBody;
              m->id=msgId;
              m->refId=refId;
              m->receivedTime=time(0);
              GWEN_IPCRequest_AddResponseMsg(r, m);
            }
          }
          else {
            GWEN_IPCREQUEST *r;
            GWEN_IPCMSG *m;

            /* this is a new incoming request */
            DBG_INFO(0, "Got an incoming request");
            m=GWEN_IPCMsg_new(n);
            m->db=dbBody;
            m->id=msgId;
            m->refId=refId;
            m->receivedTime=time(0);
            r=GWEN_IPCRequest_new();
            GWEN_IPCRequest_AddRequestMsg(r, m);
          }
        } /* if body is valid */
      } /* if there was a body */
      GWEN_NetMsg_free(nm);
      msgs++;
    } /* if there was a netmessage */
    n=GWEN_IPCNode_List_Next(n);
  } /* while */

  return msgs;
}



















