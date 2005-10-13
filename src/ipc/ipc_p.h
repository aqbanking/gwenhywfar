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


#ifndef GWEN_IPC_P_H
#define GWEN_IPC_P_H

#include <gwenhywfar/ipc.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/nettransportssl.h>
#include <gwenhywfar/netconnectionhttp.h>



/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Node
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

typedef struct GWEN_IPCNODE GWEN_IPCNODE;
GWEN_LIST_FUNCTION_DEFS(GWEN_IPCNODE, GWEN_IPCNode)
struct GWEN_IPCNODE {
  GWEN_LIST_ELEMENT(GWEN_IPCNODE)
  GWEN_NETCONNECTION *connection;
  int isServer;
  int isPassiveClient;
  GWEN_TYPE_UINT32 id;
  GWEN_TYPE_UINT32 mark;
  GWEN_TYPE_UINT32 usage;
  GWEN_TYPE_UINT32 flags;
  char *baseAuth;

  GWEN_TYPE_UINT32 nextMsgId;
  GWEN_TYPE_UINT32 lastMsgId;
};
GWEN_IPCNODE *GWEN_IPCNode_new();
void GWEN_IPCNode_free(GWEN_IPCNODE *n);
void GWEN_IPCNode_Attach(GWEN_IPCNODE *n);



/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Message
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

typedef struct GWEN_IPCMSG GWEN_IPCMSG;
GWEN_LIST_FUNCTION_DEFS(GWEN_IPCMSG, GWEN_IPCMsg)
struct GWEN_IPCMSG {
  GWEN_LIST_ELEMENT(GWEN_IPCMSG)
  GWEN_IPCNODE *node;

  GWEN_TYPE_UINT32 id;
  GWEN_TYPE_UINT32 refId;
  GWEN_DB_NODE *db;
  time_t sendTime;
  time_t receivedTime;
};
GWEN_IPCMSG *GWEN_IPCMsg_new(GWEN_IPCNODE *n);
void GWEN_IPCMsg_free(GWEN_IPCMSG *m);



/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Request
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

typedef struct GWEN_IPC__REQUEST GWEN_IPC__REQUEST;
GWEN_LIST_FUNCTION_DEFS(GWEN_IPC__REQUEST, GWEN_IPC__Request)
struct GWEN_IPC__REQUEST {
  GWEN_LIST_ELEMENT(GWEN_IPC__REQUEST)
  GWEN_INHERIT_ELEMENT(GWEN_IPC__REQUEST)

  GWEN_TYPE_UINT32 id;
  GWEN_IPCMSG_LIST *requestMsgs;
  GWEN_IPCMSG_LIST *responseMsgs;

  int usage;
};
GWEN_IPC__REQUEST *GWEN_IPC__Request_new();
void GWEN_IPC__Request_Attach(GWEN_IPC__REQUEST *r);
void GWEN_IPC__Request_free(GWEN_IPC__REQUEST *r);
void GWEN_IPC__Request_AddRequestMsg(GWEN_IPC__REQUEST *r, GWEN_IPCMSG *m);
void GWEN_IPC__Request_AddResponseMsg(GWEN_IPC__REQUEST *r, GWEN_IPCMSG *m);
int GWEN_IPC__Request_HasRequestMsg(GWEN_IPC__REQUEST *r,
                                  GWEN_TYPE_UINT32 nid,
                                  GWEN_TYPE_UINT32 id);
GWEN_IPCMSG_LIST *GWEN_IPC__Request_GetRequestMsgList(const GWEN_IPC__REQUEST *r);





/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Manager
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

struct GWEN_IPCMANAGER {
  char *application;
  GWEN_TYPE_UINT32 libId;
  GWEN_IPCNODE_LIST *nodes;
  GWEN_IPC__REQUEST_LIST *outRequests;
  GWEN_IPC__REQUEST_LIST *newInRequests;
  GWEN_IPC__REQUEST_LIST *oldInRequests;
  unsigned int sendTimeOut;
  GWEN_TYPE_UINT32 usage;
};

GWEN_IPC__REQUEST *GWEN_IPCManager__FindRequest(GWEN_IPCMANAGER *mgr,
                                              GWEN_TYPE_UINT32 rid,
                                              GWEN_IPC__REQUEST *r);
int GWEN_IPCManager__SendMsg(GWEN_IPCMANAGER *mgr,
                             GWEN_IPCMSG *m);
GWEN_TYPE_UINT32 GWEN_IPCManager__AddService(GWEN_IPCMANAGER *mgr,
                                             GWEN_NETTRANSPORT *tr,
                                             GWEN_TYPE_UINT32 mark,
                                             int isServer);
GWEN_IPCMSG *GWEN_IPCManager__MakeErrorResponse(GWEN_IPCMANAGER *mgr,
                                                GWEN_IPCMSG *m,
                                                int code,
                                                const char *txt);
int GWEN_IPCManager__Collect(GWEN_IPCMANAGER *mgr, int maxMsg);
int GWEN_IPCManager__CheckRequests(GWEN_IPCMANAGER *mgr);

void GWEN_IPCManager__Connection_Up(GWEN_NETCONNECTION *conn);
void GWEN_IPCManager__Connection_Down(GWEN_NETCONNECTION *conn);


void GWEN_IPCManager__RemoveNodeRequestMessages(GWEN_IPCMANAGER *mgr,
                                                GWEN_IPCNODE *n,
                                                GWEN_IPC__REQUEST_LIST *rl,
                                                const char *msgType);

void GWEN_IPCNode_Dump(GWEN_IPCNODE *n, FILE *f, int indent);
void GWEN_IPCMsg_Dump(GWEN_IPCMSG *m, FILE *f, int indent);
void GWEN_IPC__Request_Dump(GWEN_IPC__REQUEST *r, FILE *f, int indent);



#endif /* GWEN_IPC_P_H */




