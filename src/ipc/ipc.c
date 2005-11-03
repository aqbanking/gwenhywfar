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

#include <strings.h>

#include "ipc_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/nl_packets.h>
#include <gwenhywfar/net2.h>


GWEN_LIST_FUNCTIONS(GWEN_IPCNODE, GWEN_IpcNode)
GWEN_LIST_FUNCTIONS(GWEN_IPCMSG, GWEN_IpcMsg)
GWEN_LIST_FUNCTIONS(GWEN_IPC__REQUEST, GWEN_Ipc__Request)


static GWEN_TYPE_UINT32 gwen_ipc__lastid=0;


/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Node
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


/* -------------------------------------------------------------- FUNCTION */
GWEN_IPCNODE *GWEN_IpcNode_new(){
  GWEN_IPCNODE *n;

  GWEN_NEW_OBJECT(GWEN_IPCNODE, n);
  DBG_MEM_INC("GWEN_IPCNODE", 0);
  GWEN_LIST_INIT(GWEN_IPCNODE, n);

  if (gwen_ipc__lastid==0)
    gwen_ipc__lastid=time(0);
  n->id=++gwen_ipc__lastid;
  n->usage=1;
  return n;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IpcNode_free(GWEN_IPCNODE *n){
  if (n) {
    DBG_MEM_DEC("GWEN_IPCNODE");
    assert(n->usage);
    if (--(n->usage)==0) {
      GWEN_NetLayer_free(n->netLayer);
      GWEN_LIST_FINI(GWEN_IPCNODE, n);
      GWEN_FREE_OBJECT(n);
    }
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IpcNode_Attach(GWEN_IPCNODE *n){
  assert(n);
  DBG_MEM_INC("GWEN_IPCNODE", 1);
  n->usage++;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IpcNode_Dump(GWEN_IPCNODE *n, FILE *f, int indent){
  int i;

  assert(n);
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "---------------------------------------\n");
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "IPC Node:\n");
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Id               : %08x\n", n->id);
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Mark             : %d\n", n->mark);
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Usage            : %d\n", n->usage);
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Is Server        : ");
  if (n->isServer)
    fprintf(f, "yes\n");
  else
    fprintf(f, "no\n");
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Is Passive Client: ");
  if (n->isPassiveClient)
    fprintf(f, "yes\n");
  else
    fprintf(f, "no\n");
}







/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Message
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

/* -------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_IpcMsg_new(GWEN_IPCNODE *n){
  GWEN_IPCMSG *m;

  assert(n);
  GWEN_NEW_OBJECT(GWEN_IPCMSG, m);
  DBG_MEM_INC("GWEN_IPCMSG", 0);
  GWEN_LIST_INIT(GWEN_IPCMSG, m);
  m->node=n;
  GWEN_IpcNode_Attach(m->node);
  return m;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IpcMsg_free(GWEN_IPCMSG *m){
  if (m) {
    DBG_MEM_DEC("GWEN_IPCMSG");
    GWEN_NL_Packet_free(m->packet);
    GWEN_IpcNode_free(m->node);
    GWEN_DB_Group_free(m->db);
    GWEN_LIST_FINI(GWEN_IPCMSG, m);
    GWEN_FREE_OBJECT(m);
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IpcMsg_Dump(GWEN_IPCMSG *m, FILE *f, int indent){
  int i;

  assert(m);
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "---------------------------------------\n");
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "IPC Message:\n");
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Id               : %08x\n", m->id);
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "RefId            : %08x\n", m->refId);
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Node Id          : %08x\n", m->node->id);
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "DB:\n");
  GWEN_DB_Dump(m->db, f, indent+4);
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Send Time        : %s",
	  (m->sendTime)?ctime(&(m->sendTime)):"never\n");
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Receiption Time  : %s",
	  (m->receivedTime)?ctime(&(m->receivedTime)):"never\n");
}






/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Request
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


/* -------------------------------------------------------------- FUNCTION */
GWEN_IPC__REQUEST *GWEN_Ipc__Request_new(){
  GWEN_IPC__REQUEST *r;

  GWEN_NEW_OBJECT(GWEN_IPC__REQUEST, r);
  DBG_MEM_INC("GWEN_IPC__REQUEST", 0);
  GWEN_LIST_INIT(GWEN_IPC__REQUEST, r);

  r->requestMsgs=GWEN_IpcMsg_List_new();
  r->responseMsgs=GWEN_IpcMsg_List_new();
  r->usage=1;
  return r;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_Ipc__Request_free(GWEN_IPC__REQUEST *r){
  if (r) {
    assert(r->usage);
    if (--(r->usage)==0) {
      DBG_MEM_DEC("GWEN_IPC__REQUEST");
      GWEN_IpcMsg_List_free(r->responseMsgs);
      GWEN_IpcMsg_List_free(r->requestMsgs);
      GWEN_LIST_FINI(GWEN_IPC__REQUEST, r);
      GWEN_FREE_OBJECT(r);
    }
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_Ipc__Request_Attach(GWEN_IPC__REQUEST *r){
  assert(r);
  assert(r->usage);
  r->usage++;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_Ipc__Request_AddRequestMsg(GWEN_IPC__REQUEST *r, GWEN_IPCMSG *m){
  assert(r);
  assert(m);
  GWEN_IpcMsg_List_Add(m, r->requestMsgs);
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG_LIST *GWEN_Ipc__Request_GetRequestMsgList(const GWEN_IPC__REQUEST *r){
  assert(r);
  return r->requestMsgs;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_Ipc__Request_AddResponseMsg(GWEN_IPC__REQUEST *r, GWEN_IPCMSG *m){
  assert(r);
  assert(m);
  GWEN_IpcMsg_List_Add(m, r->responseMsgs);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_Ipc__Request_HasRequestMsg(GWEN_IPC__REQUEST *r,
				    GWEN_TYPE_UINT32 nid,
				    GWEN_TYPE_UINT32 id){
  GWEN_IPCMSG *m;

  assert(r);
  assert(id);

  m=GWEN_IpcMsg_List_First(r->requestMsgs);
  while(m) {
    if (m->node->id==nid &&
	m->id==id)
      return 1;
    m=GWEN_IpcMsg_List_Next(m);
  } /* while */

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_Ipc__Request_Dump(GWEN_IPC__REQUEST *r, FILE *f, int indent){
  int i;
  GWEN_IPCMSG *m;

  assert(r);
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "---------------------------------------\n");
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "IPC Request:\n");
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Id               : %08x\n", r->id);
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Request Message(s)\n");
  m=GWEN_IpcMsg_List_First(r->requestMsgs);
  if (!m) {
    for (i=0; i<indent+4; i++)
      fprintf(f, " ");
    fprintf(f, "none\n");
  }
  while(m) {
    GWEN_IpcMsg_Dump(m, f, indent+4);
    m=GWEN_IpcMsg_List_Next(m);
  }
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Response Message(s)\n");
  m=GWEN_IpcMsg_List_First(r->responseMsgs);
  if (!m) {
    for (i=0; i<indent+4; i++)
      fprintf(f, " ");
    fprintf(f, "none\n");
  }
  while(m) {
    GWEN_IpcMsg_Dump(m, f, indent+4);
    m=GWEN_IpcMsg_List_Next(m);
  }
}







/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Manager
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




/* -------------------------------------------------------------- FUNCTION */
GWEN_IPCMANAGER *GWEN_IpcManager_new(){
  GWEN_IPCMANAGER *mgr;

  GWEN_NEW_OBJECT(GWEN_IPCMANAGER, mgr);
  DBG_MEM_INC("GWEN_IPCMANAGER", 0);
  mgr->nodes=GWEN_IpcNode_List_new();
  mgr->outRequests=GWEN_Ipc__Request_List_new();
  mgr->newInRequests=GWEN_Ipc__Request_List_new();
  mgr->oldInRequests=GWEN_Ipc__Request_List_new();

  mgr->usage=1;
  return mgr;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IpcManager_Attach(GWEN_IPCMANAGER *mgr){
  assert(mgr);
  DBG_MEM_INC("GWEN_IPCMANAGER", 1);
  mgr->usage++;
}




/* -------------------------------------------------------------- FUNCTION */
void GWEN_IpcManager_free(GWEN_IPCMANAGER *mgr){
  if (mgr) {
    DBG_MEM_DEC("GWEN_IPCMANAGER");
    assert(mgr->usage);
    if (--(mgr->usage)==0) {
      free(mgr->application);
      GWEN_Ipc__Request_List_free(mgr->oldInRequests);
      GWEN_Ipc__Request_List_free(mgr->newInRequests);
      GWEN_Ipc__Request_List_free(mgr->outRequests);
      GWEN_IpcNode_List_free(mgr->nodes);

      GWEN_FREE_OBJECT(mgr);
    }
  }
}



/* -------------------------------------------------------------- FUNCTION */
const char *GWEN_IpcManager_GetApplicationName(const GWEN_IPCMANAGER *mgr) {
  assert(mgr);
  return mgr->application;
}


/* -------------------------------------------------------------- FUNCTION */
void GWEN_IpcManager_SetApplicationName(GWEN_IPCMANAGER *mgr,
                                        const char *s){
  assert(mgr);
  if (s) mgr->application=strdup(s);
  else mgr->application=0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_IpcManager_AddServer(GWEN_IPCMANAGER *mgr,
					   GWEN_NETLAYER *nlBase,
                                           GWEN_TYPE_UINT32 mark){
  GWEN_IPCNODE *n;
  GWEN_NETLAYER *nl;
  int rv;

  /* create connection layer */
  nl=GWEN_NetLayerPackets_new(nlBase);
  GWEN_Net_AddConnectionToPool(nl);
  rv=GWEN_NetLayer_Listen(nl);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not start server (%d)", rv);
    GWEN_NetLayer_free(nl);
    return 0;
  }

  n=GWEN_IpcNode_new();
  n->netLayer=nl;
  n->mark=mark;
  n->isServer=1;
  GWEN_IpcNode_List_Add(n, mgr->nodes);
  return n->id;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_IpcManager_AddClient(GWEN_IPCMANAGER *mgr,
					   GWEN_NETLAYER *nlBase,
					   GWEN_TYPE_UINT32 mark){
  GWEN_IPCNODE *n;
  GWEN_NETLAYER *nl;

  n=GWEN_IpcNode_new();

  nl=GWEN_NetLayerPackets_new(nlBase);
  GWEN_Net_AddConnectionToPool(nl);

  n->netLayer=nl;
  n->mark=mark;
  n->isServer=0;
  GWEN_IpcNode_List_Add(n, mgr->nodes);
  return n->id;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IpcManager__RemoveNodeRequestMessages(GWEN_IPCMANAGER *mgr,
						GWEN_IPCNODE *n,
                                                GWEN_IPC__REQUEST_LIST *rl,
                                                const char *msgType) {
  GWEN_IPC__REQUEST *r;

  /* remove all messages for/of this client from request */
  r=GWEN_Ipc__Request_List_First(rl);
  while(r) {
    GWEN_IPC__REQUEST *rnext;
    GWEN_IPCMSG *msg;

    rnext=GWEN_Ipc__Request_List_Next(r);
    msg=GWEN_IpcMsg_List_First(r->requestMsgs);
    while(msg) {
      GWEN_IPCMSG *nextmsg;

      nextmsg=GWEN_IpcMsg_List_Next(msg);
      assert(msg->node);
      if (msg->node==n) {
        /* same node, remove msg */
	DBG_INFO(GWEN_LOGDOMAIN, "Removing %s message for/from node %08x",
		 msgType, n->id);
	if (GWEN_Logger_GetLevel(GWEN_LOGDOMAIN)>=GWEN_LoggerLevel_Info) {
	  if (msg->db) {
	    GWEN_DB_Dump(msg->db, stderr, 2);
	  }
	}
	GWEN_IpcMsg_List_Del(msg);
	GWEN_IpcMsg_free(msg);
      }
      msg=nextmsg;
    } /* while msg */

    /* check whether the request is empty */
    if (GWEN_IpcMsg_List_First(r->requestMsgs)==0) {
      /* it is, remove the request */
      DBG_INFO(GWEN_LOGDOMAIN, "Removing %s request %08x for/from node %08x",
	       msgType, r->id, n->id);
      GWEN_Ipc__Request_List_Del(r);
      GWEN_Ipc__Request_free(r);
    }
    r=rnext;
  } /* while r */
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_IpcManager_RemoveClient(GWEN_IPCMANAGER *mgr,
				 GWEN_TYPE_UINT32 nid) {
  GWEN_IPCNODE *n;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Removing client %08x", nid);
  assert(mgr);

  /* get client node */
  n=GWEN_IpcNode_List_First(mgr->nodes);
  while(n) {
    if (n->id==nid)
      break;
    n=GWEN_IpcNode_List_Next(n);
  } /* while */
  if (!n) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node %08x not found", nid);
    return -1;
  }

  /* remove all messages of this client in any request */
  GWEN_IpcManager__RemoveNodeRequestMessages(mgr, n, mgr->outRequests,
                                             "outRequest");
  GWEN_IpcManager__RemoveNodeRequestMessages(mgr, n, mgr->newInRequests,
                                             "newInRequest");
  GWEN_IpcManager__RemoveNodeRequestMessages(mgr, n, mgr->oldInRequests,
                                             "newOutRequest");
  /* remove node */
  DBG_NOTICE(GWEN_LOGDOMAIN, "Disconnecting netLayer");
  GWEN_NetLayer_Disconnect(n->netLayer);
  GWEN_Net_DelConnectionFromPool(n->netLayer);
  DBG_NOTICE(GWEN_LOGDOMAIN, "Removing client %08x", n->id);
  GWEN_IpcNode_List_Del(n);
  GWEN_IpcNode_free(n);

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_IPC__REQUEST *GWEN_IpcManager__FindRequest(GWEN_IPCMANAGER *mgr,
						GWEN_TYPE_UINT32 rid,
						GWEN_IPC__REQUEST *r) {
  assert(mgr);
  assert(r);

  while(r) {
    if (r->id==rid)
      return r;
    r=GWEN_Ipc__Request_List_Next(r);
  }
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_IpcManager_SendRequest(GWEN_IPCMANAGER *mgr,
                                             GWEN_TYPE_UINT32 nid,
                                             GWEN_DB_NODE *req){
  GWEN_IPCNODE *n;
  GWEN_IPC__REQUEST *r;
  GWEN_IPCMSG *m;

  n=GWEN_IpcNode_List_First(mgr->nodes);
  while(n) {
    if (n->id==nid)
      break;
    n=GWEN_IpcNode_List_Next(n);
  } /* while */
  if (!n) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node %08x not found", nid);
    return 0;
  }

  m=GWEN_IpcMsg_new(n);
  m->db=req;
  m->id=++(n->nextMsgId);

  if (GWEN_IpcManager__SendMsg(mgr, m)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not send request");
    GWEN_IpcMsg_free(m);
    return 0;
  }

  r=GWEN_Ipc__Request_new();
  r->id=++gwen_ipc__lastid;
  GWEN_Ipc__Request_AddRequestMsg(r, m);
  GWEN_Ipc__Request_List_Add(r, mgr->outRequests);
  return r->id;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_IpcManager_SendMultiRequest(GWEN_IPCMANAGER *mgr,
                                                  GWEN_TYPE_UINT32 mark,
                                                  GWEN_DB_NODE *req){
  GWEN_IPCNODE *n;
  GWEN_IPC__REQUEST *r;

  r=0;
  n=GWEN_IpcNode_List_First(mgr->nodes);
  while(n) {
    if (mark==0 || mark==n->mark) {
      GWEN_NETLAYER_STATUS st;

      st=GWEN_NetLayer_GetStatus(n->netLayer);
      if (st!=GWEN_NetLayerStatus_Listening &&
	  st!=GWEN_NetLayerStatus_Disabled) {
	GWEN_IPCMSG *m;

        m=GWEN_IpcMsg_new(n);
        m->db=GWEN_DB_Group_dup(req);
        m->id=--(n->nextMsgId);

        if (GWEN_IpcManager__SendMsg(mgr, m)) {
	  DBG_ERROR(GWEN_LOGDOMAIN,
		    "Could not send request to node %08x",
		    n->id);
          GWEN_IpcMsg_free(m);
        }
        else {
          if (r==0) {
            r=GWEN_Ipc__Request_new();
	    r->id=++gwen_ipc__lastid;
            GWEN_Ipc__Request_List_Add(r, mgr->outRequests);
          }
          GWEN_Ipc__Request_AddRequestMsg(r, m);
        }
      }
    } /* if mark matches */
    n=GWEN_IpcNode_List_Next(n);
  } /* while */

  if (r) {
    return r->id;
  }
  DBG_ERROR(GWEN_LOGDOMAIN, "Could not send any request");
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_IpcManager_SendResponse(GWEN_IPCMANAGER *mgr,
                                 GWEN_TYPE_UINT32 rid,
                                 GWEN_DB_NODE *rsp){
  GWEN_IPC__REQUEST *r;
  GWEN_IPCMSG *m;
  GWEN_IPCMSG *om;

  r=GWEN_Ipc__Request_List_First(mgr->oldInRequests);
  while(r) {
    if (r->id==rid)
      break;
    r=GWEN_Ipc__Request_List_Next(r);
  } /* while */
  if (!r) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Request %08x not found", rid);
    return -1;
  }

  om=GWEN_IpcMsg_List_First(r->requestMsgs);
  assert(om);

  m=GWEN_IpcMsg_new(om->node);
  m->refId=om->id;
  m->db=rsp;
  m->id=++(om->node->nextMsgId);

  DBG_DEBUG(GWEN_LOGDOMAIN, "Sending response %08x for request %08x",
	    m->id, m->refId);

  if (GWEN_IpcManager__SendMsg(mgr, m)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not send response");
    GWEN_IpcMsg_free(m);
    return -1;
  }

  GWEN_Ipc__Request_AddResponseMsg(r, m);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_IpcManager_RemoveRequest(GWEN_IPCMANAGER *mgr,
                                  GWEN_TYPE_UINT32 rid,
                                  int inOrOut){
  GWEN_IPC__REQUEST *r;

  if (inOrOut==0)
    r=GWEN_Ipc__Request_List_First(mgr->oldInRequests);
  else
    r=GWEN_Ipc__Request_List_First(mgr->outRequests);
  while(r) {
    if (r->id==rid)
      break;
    r=GWEN_Ipc__Request_List_Next(r);
  } /* while */
  if (!r) {
    DBG_ERROR(GWEN_LOGDOMAIN, "%s request %08x not found",
	      inOrOut?"Outgoing":"Incoming", rid);
    return -1;
  }
  GWEN_Ipc__Request_List_Del(r);
  GWEN_Ipc__Request_free(r);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_IpcManager_GetNextInRequest(GWEN_IPCMANAGER *mgr,
                                                  GWEN_TYPE_UINT32 mark){
  GWEN_IPC__REQUEST *r;

  r=GWEN_Ipc__Request_List_First(mgr->newInRequests);
  while(r) {
    GWEN_IPCMSG *om;

    if (mark==0)
      break;

    om=GWEN_IpcMsg_List_First(r->requestMsgs);
    assert(om);
    assert(om->node);
    if (om->node->mark==mark)
      break;
    r=GWEN_Ipc__Request_List_Next(r);
  }

  if (r) {
    GWEN_Ipc__Request_List_Del(r);
    GWEN_Ipc__Request_List_Add(r, mgr->oldInRequests);
    return r->id;
  }
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_DB_NODE *GWEN_IpcManager_GetInRequestData(GWEN_IPCMANAGER *mgr,
                                               GWEN_TYPE_UINT32 rid){
  GWEN_IPC__REQUEST *r;
  GWEN_IPCMSG *om;

  r=GWEN_Ipc__Request_List_First(mgr->oldInRequests);
  while(r) {
    if (r->id==rid)
      break;
    r=GWEN_Ipc__Request_List_Next(r);
  } /* while */
  if (!r) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Request %08x not found", rid);
    return 0;
  }

  om=GWEN_IpcMsg_List_First(r->requestMsgs);
  assert(om);

  return om->db;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER *GWEN_IpcManager_GetNetLayer(GWEN_IPCMANAGER *mgr,
					   GWEN_TYPE_UINT32 nid){
  GWEN_IPCNODE *n;

  n=GWEN_IpcNode_List_First(mgr->nodes);
  while(n) {
    if (n->id==nid)
      break;
    n=GWEN_IpcNode_List_Next(n);
  } /* while */
  if (!n) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node %08x not found", nid);
    return 0;
  }

  return n->netLayer;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32
GWEN_IpcManager_GetClientForNetLayer(const GWEN_IPCMANAGER *mgr,
				     const GWEN_NETLAYER *nl) {
  GWEN_IPCNODE *n;

  n=GWEN_IpcNode_List_First(mgr->nodes);
  while(n) {
    if (n->netLayer==nl)
      break;
    n=GWEN_IpcNode_List_Next(n);
  } /* while */
  if (!n) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No node found for connection");
    return 0;
  }
  return n->id;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_DB_NODE *GWEN_IpcManager_PeekResponseData(GWEN_IPCMANAGER *mgr,
                                               GWEN_TYPE_UINT32 rid){
  GWEN_IPC__REQUEST *r;
  GWEN_IPCMSG *m;
  GWEN_DB_NODE *db;

  r=GWEN_Ipc__Request_List_First(mgr->outRequests);
  while(r) {
    if (r->id==rid)
      break;
    r=GWEN_Ipc__Request_List_Next(r);
  } /* while */
  if (!r) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Request %08x not found", rid);
    return 0;
  }

  m=GWEN_IpcMsg_List_First(r->responseMsgs);
  if (!m) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "No response yet");
    return 0;
  }
  db=m->db;
  assert(m->node);
  assert(m->node->id);
  return db;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_DB_NODE *GWEN_IpcManager_GetResponseData(GWEN_IPCMANAGER *mgr,
                                              GWEN_TYPE_UINT32 rid){
  GWEN_IPC__REQUEST *r;
  GWEN_IPCMSG *m;
  GWEN_DB_NODE *db;

  r=GWEN_Ipc__Request_List_First(mgr->outRequests);
  while(r) {
    if (r->id==rid)
      break;
    r=GWEN_Ipc__Request_List_Next(r);
  } /* while */
  if (!r) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Request %08x not found", rid);
    return 0;
  }

  m=GWEN_IpcMsg_List_First(r->responseMsgs);
  if (!m) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "No response yet");
    return 0;
  }

  db=m->db;
  assert(m->node);
  assert(m->node->id);
  m->db=0;
  GWEN_IpcMsg_List_Del(m);
  GWEN_IpcMsg_free(m);
  return db;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_IpcManager__MakeErrorResponse(GWEN_IPCMANAGER *mgr,
                                                GWEN_IPCMSG *m,
                                                int code,
                                                const char *txt) {
  GWEN_DB_NODE *db;
  GWEN_IPCMSG *newm;

  db=GWEN_DB_Group_new("Error");
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
                      "code", code);
  if (txt)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
                         "text", txt);

  newm=GWEN_IpcMsg_new(m->node);
  newm->db=db;
  return newm;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_IpcManager__CheckRequests(GWEN_IPCMANAGER *mgr) {
  GWEN_IPC__REQUEST *r;

  r=GWEN_Ipc__Request_List_First(mgr->outRequests);
  while(r) {
    GWEN_IPCMSG *m;
    GWEN_IPC__REQUEST *nextr;

    nextr=GWEN_Ipc__Request_List_Next(r);
    m=GWEN_IpcMsg_List_First(r->requestMsgs);
    while(m) {
      GWEN_IPCMSG *nextm;
      int removeIt;

      nextm=GWEN_IpcMsg_List_Next(m);
      removeIt=0;
      assert(m->node);

      /* check connection status */
      assert(m->node->netLayer);
      if (GWEN_NetLayer_GetStatus(m->node->netLayer)==
	  GWEN_NetLayerStatus_Disabled) {
	GWEN_IPCMSG *errm;

	/* connection broken, remove msg */
        DBG_INFO(GWEN_LOGDOMAIN, "Connection broken");
        errm=GWEN_IpcManager__MakeErrorResponse(mgr,
                                                m,
						GWEN_IPC_ERROR_CONNERR,
                                                "Connection down");
        GWEN_IpcMsg_List_Add(errm, r->responseMsgs);
        removeIt=1;
      }

      /* check timeout */
      if (m->sendTime && mgr->sendTimeOut) {
        if (difftime(time(0), m->sendTime)>mgr->sendTimeOut) {
          GWEN_IPCMSG *errm;

          DBG_INFO(GWEN_LOGDOMAIN, "Message timed out");
          errm=GWEN_IpcManager__MakeErrorResponse(mgr,
                                                  m,
                                                  GWEN_IPC_ERROR_TIMEOUT,
                                                  "Message timed out");
          GWEN_IpcMsg_List_Add(errm, r->responseMsgs);
          removeIt=1;
        }
      }

      if (removeIt) {
        GWEN_IpcMsg_List_Del(m);
        GWEN_IpcMsg_free(m);
      }
      m=nextm;
    } /* while */

    r=nextr;
  } /* while */

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_IpcManager_Work(GWEN_IPCMANAGER *mgr) {
  int done=0;

  assert(mgr);

  if (GWEN_IpcManager__Work(mgr)==0)
    done++;

  if (GWEN_IpcManager__CheckRequests(mgr)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error checking requests");
  }

  return done?0:1;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_IpcManager_Disconnect(GWEN_IPCMANAGER *mgr, GWEN_TYPE_UINT32 nid){
  GWEN_IPCNODE *n;
  int rv;

  n=GWEN_IpcNode_List_First(mgr->nodes);
  while(n) {
    if (n->id==nid)
      break;
    n=GWEN_IpcNode_List_Next(n);
  } /* while */
  if (!n) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node %08x not found", nid);
    return -1;
  }

  n->nextMsgId=0;
  n->lastMsgId=0;

  /* remove all messages of this client in any request */
  GWEN_IpcManager__RemoveNodeRequestMessages(mgr, n, mgr->outRequests,
                                             "outRequest");
  GWEN_IpcManager__RemoveNodeRequestMessages(mgr, n, mgr->newInRequests,
                                             "newInRequest");
  GWEN_IpcManager__RemoveNodeRequestMessages(mgr, n, mgr->oldInRequests,
                                             "newOutRequest");

  rv=GWEN_NetLayer_Disconnect(n->netLayer);
  return rv;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_IpcManager_Dump(GWEN_IPCMANAGER *mgr, FILE *f, int indent){
  int i;
  GWEN_IPCNODE *n;
  GWEN_IPC__REQUEST *r;

  assert(mgr);
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "=======================================\n");
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "IPC Manager:\n");
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Active Nodes     : %ld\n",
	  GWEN_MemoryDebug_GetObjectCount("GWEN_IPCNODE"));
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Active Messages  : %ld\n",
	  GWEN_MemoryDebug_GetObjectCount("GWEN_IPCMSG"));
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Active Requests  : %ld\n",
          GWEN_MemoryDebug_GetObjectCount("GWEN_IPC__REQUEST"));
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Application      : %s\n", mgr->application);
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Nodes(s)\n");
  n=GWEN_IpcNode_List_First(mgr->nodes);
  if (!n) {
    for (i=0; i<indent+4; i++)
      fprintf(f, " ");
    fprintf(f, "none\n");
  }
  while(n) {
    GWEN_IpcNode_Dump(n, f, indent+4);
    n=GWEN_IpcNode_List_Next(n);
  }
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Outgoing Request(s)\n");
  r=GWEN_Ipc__Request_List_First(mgr->outRequests);
  if (!r) {
    for (i=0; i<indent+4; i++)
      fprintf(f, " ");
    fprintf(f, "none\n");
  }
  while(r) {
    GWEN_Ipc__Request_Dump(r, f, indent+4);
    r=GWEN_Ipc__Request_List_Next(r);
  }
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Unhandled Incoming Request(s)\n");
  r=GWEN_Ipc__Request_List_First(mgr->newInRequests);
  if (!r) {
    for (i=0; i<indent+4; i++)
      fprintf(f, " ");
    fprintf(f, "none\n");
  }
  while(r) {
    GWEN_Ipc__Request_Dump(r, f, indent+4);
    r=GWEN_Ipc__Request_List_Next(r);
  }
  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Incoming Request(s) in Work\n");
  r=GWEN_Ipc__Request_List_First(mgr->oldInRequests);
  if (!r) {
    for (i=0; i<indent+4; i++)
      fprintf(f, " ");
    fprintf(f, "none\n");
  }
  while(r) {
    GWEN_Ipc__Request_Dump(r, f, indent+4);
    r=GWEN_Ipc__Request_List_Next(r);
  }
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_IpcManager__SendMsg(GWEN_IPCMANAGER *mgr,
                             GWEN_IPCMSG *m) {
  GWEN_BUFFER *buf;
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbIpc;
  GWEN_DB_NODE *dbData;
  char numbuf[16];
  GWEN_NETLAYER_STATUS nst;
  int rv;
  GWEN_NL_PACKET *pk;

  nst=GWEN_NetLayer_GetStatus(m->node->netLayer);
  if (nst==GWEN_NetLayerStatus_Disabled) {
    DBG_ERROR(GWEN_LOGDOMAIN, "NetLayer is disabled");
    return -1;
  }

  /* check for connection state, connect if necessary */
  if (nst==GWEN_NetLayerStatus_Unconnected) {
    if (m->node->isPassiveClient) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Passive IPC client \"%08x\" is down, "
		"not sending message", m->node->id);
      return -1;
    }
    rv=GWEN_NetLayer_Connect(m->node->netLayer);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not connect");
      return rv;
    }
  }

  dbReq=GWEN_DB_Group_new("request");
  /* prepare IPC group */
  dbIpc=GWEN_DB_GetGroup(dbReq, GWEN_DB_FLAGS_DEFAULT, "ipc");
  snprintf(numbuf, sizeof(numbuf), "%d", m->id);
  GWEN_DB_SetCharValue(dbIpc, GWEN_DB_FLAGS_DEFAULT,
		       "id", numbuf);
  if (m->refId) {
    snprintf(numbuf, sizeof(numbuf), "%d", m->refId);
    GWEN_DB_SetCharValue(dbIpc, GWEN_DB_FLAGS_DEFAULT,
			 "refid", numbuf);
  }
  GWEN_DB_SetCharValue(dbIpc, GWEN_DB_FLAGS_DEFAULT,
		       "cmd", GWEN_DB_GroupName(m->db));

  dbData=GWEN_DB_GetGroup(dbReq, GWEN_DB_FLAGS_DEFAULT, "data");
  GWEN_DB_AddGroupChildren(dbData, m->db);

  buf=GWEN_Buffer_new(0, 512, 0, 1);
  bio=GWEN_BufferedIO_Buffer2_new(buf, 0); /* don't relinquish buffer */

  /* encode db */
  GWEN_BufferedIO_SetWriteBuffer(bio, 0, 128);
  if (GWEN_DB_WriteToStream(dbReq, bio, GWEN_DB_FLAGS_COMPACT)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode db");
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    GWEN_Buffer_free(buf);
    GWEN_DB_Group_free(dbReq);
    return -1;
  }
  GWEN_DB_Group_free(dbReq);

  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_BufferedIO_free(bio);
    GWEN_Buffer_free(buf);
    return -1;
  }
  GWEN_BufferedIO_free(bio);

  /* create packet */
  pk=GWEN_NL_Packet_new();
  GWEN_Buffer_Rewind(buf);
  GWEN_NL_Packet_SetBuffer(pk, buf);

  /* send message */
  rv=GWEN_NetLayerPackets_SendPacket(m->node->netLayer, pk);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not send package");
    GWEN_NL_Packet_free(pk);
    return rv;
  }

  DBG_DEBUG(GWEN_LOGDOMAIN, "Message is on its way");
  m->sendTime=time(0);
  return 0;
}




/* -------------------------------------------------------------- FUNCTION */
int GWEN_IpcManager__HandlePacket(GWEN_IPCMANAGER *mgr,
				  GWEN_IPCNODE *n,
				  GWEN_NL_PACKET *pk) {
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbIpc;
  GWEN_TYPE_UINT32 msgId;
  GWEN_TYPE_UINT32 refId;
  GWEN_BUFFER *buf;
  GWEN_BUFFEREDIO *bio;
  int rv;

  /* read and decode message */
  buf=GWEN_NL_Packet_GetBuffer(pk);
  assert(buf);
  DBG_DEBUG(GWEN_LOGDOMAIN, "Got an incoming message");
  if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
    GWEN_Buffer_Dump(buf, stderr, 2);
  GWEN_Buffer_Rewind(buf);
  bio=GWEN_BufferedIO_Buffer2_new(buf, 0); /* don't tak over */
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 128);
  dbReq=GWEN_DB_Group_new("request");
  rv=GWEN_DB_ReadFromStream(dbReq, bio,
			    GWEN_DB_FLAGS_DEFAULT |
			    GWEN_DB_FLAGS_STOP_ON_EMPTY_LINE);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid incoming request");
    GWEN_Buffer_Dump(buf, stderr, 2);
    GWEN_BufferedIO_Abandon(bio);
    GWEN_DB_Group_free(dbReq);
    GWEN_BufferedIO_free(bio);
    return rv;
  }
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);

  /* parse decoded message */
  dbIpc=GWEN_DB_GetGroup(dbReq,
			 GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			 "ipc");
  if (dbIpc==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid incoming request");
    GWEN_DB_Dump(dbReq, stderr, 2);
    GWEN_DB_Group_free(dbReq);
    return -1;
  }
  msgId=GWEN_DB_GetIntValue(dbIpc, "id", 0, 0);
  refId=GWEN_DB_GetIntValue(dbIpc, "refId", 0, 0);

  if (msgId<=n->lastMsgId) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad message id (%d<=%d)",
	      msgId, n->lastMsgId);
    GWEN_DB_Group_free(dbReq);
    return -1;
  }

  n->lastMsgId=msgId;
  GWEN_DB_SetIntValue(dbIpc, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "nodeId", n->id);
  /* got a valid message */
  if (refId) {
    GWEN_IPC__REQUEST *r;
    GWEN_IPCMSG *m;

    /* it pretends to be a response, check it */
    r=GWEN_Ipc__Request_List_First(mgr->outRequests);
    while(r) {
      if (GWEN_Ipc__Request_HasRequestMsg(r, n->id, refId))
	break;
      r=GWEN_Ipc__Request_List_Next(r);
    } /* while r */
    if (!r) {
      DBG_WARN(GWEN_LOGDOMAIN,
	       "Got a response for invalid request (%08x)",
	       refId);
      GWEN_DB_Group_free(dbReq);
      return -1;
    }

    DBG_DEBUG(GWEN_LOGDOMAIN,
	      "Got a response for request %08x",
	      r->id);
    m=GWEN_IpcMsg_new(n);
    m->db=dbReq;
    m->id=msgId;
    m->refId=refId;
    m->receivedTime=time(0);
    GWEN_Ipc__Request_AddResponseMsg(r, m);
  }
  else {
    GWEN_IPC__REQUEST *r;
    GWEN_IPCMSG *m;

    /* this is a new incoming request */
    DBG_DEBUG(GWEN_LOGDOMAIN, "Got an incoming request (%08x)",
              msgId);
    if (GWEN_Logger_GetLevel(GWEN_LOGDOMAIN)>=GWEN_LoggerLevel_Debug) {
      GWEN_DB_Dump(dbReq, stderr, 2);
    }
    m=GWEN_IpcMsg_new(n);
    m->db=dbReq;
    m->id=msgId;
    m->refId=refId;
    m->receivedTime=time(0);
    r=GWEN_Ipc__Request_new();
    r->id=++gwen_ipc__lastid;
    GWEN_Ipc__Request_AddRequestMsg(r, m);
    GWEN_Ipc__Request_List_Add(r, mgr->newInRequests);
  } /* if refId found */

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_IpcManager__Work(GWEN_IPCMANAGER *mgr) {
  GWEN_IPCNODE *n;
  int done=0;

  n=GWEN_IpcNode_List_First(mgr->nodes);
  while(n) {
    GWEN_IPCNODE *next;
#if 0
    GWEN_NETLAYER_STATUS st;
#endif

    next=GWEN_IpcNode_List_Next(n);
    DBG_DEBUG(GWEN_LOGDOMAIN, "Checking node");

#if 0
    st=GWEN_NetLayer_GetStatus(n->netLayer);
    if (st==GWEN_NetLayerStatus_Disabled) {
      DBG_INFO(GWEN_LOGDOMAIN,
	       "NetLayer for passive IPC client \"%0x8\" "
	       "is disabled, removing it", n->id);
      GWEN_IpcManager_RemoveClient(mgr, n->id);
      return 0;
    }

    if (st==GWEN_NetLayerStatus_Disconnected) {
      if (n->isPassiveClient) {
	if (!GWEN_NetLayerPackets_HasNextPacket(n->netLayer)) {
	  DBG_INFO(GWEN_LOGDOMAIN,
		   "NetLayer for passive IPC client \"%0x8\" "
		   "is disconnected, removing it", n->id);
	  GWEN_IpcManager_RemoveClient(mgr, n->id);
	  return 0;
	}
      }
    }
#endif

    if (n->isServer) {
      GWEN_NETLAYER *nl;

      DBG_DEBUG(GWEN_LOGDOMAIN, "Node is a server");
      /* collect connections */
      nl=GWEN_NetLayer_GetIncomingLayer(n->netLayer);
      if (nl) {
        GWEN_IPCNODE *newnode;

        DBG_INFO(GWEN_LOGDOMAIN, "Got an incoming connection");
	newnode=GWEN_IpcNode_new();
	newnode->netLayer=nl;
	newnode->mark=n->mark;
	newnode->isPassiveClient=1;
	GWEN_IpcNode_List_Add(newnode, mgr->nodes);
	GWEN_Net_AddConnectionToPool(nl);
        done++;
      }
      else {
        DBG_DEBUG(GWEN_LOGDOMAIN, "No incoming connection");
      }
    } /* if isServer */
    else {
      GWEN_NL_PACKET *pk;
      int rv;

      DBG_DEBUG(GWEN_LOGDOMAIN, "Node is NOT a server");
      pk=GWEN_NetLayerPackets_GetNextPacket(n->netLayer);
      if (pk) {
        rv=GWEN_IpcManager__HandlePacket(mgr, n, pk);
	GWEN_NL_Packet_free(pk);
	if (rv) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	}
	done++;
      }
      else {
	DBG_DEBUG(GWEN_LOGDOMAIN, "No message");
      }
    } /* if client */
    n=next;
  } /* while */

  if (done)
    return 0;
  return 1;
}













