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
#include <gwenhywfar/nl_packets.h>



/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Node
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

typedef struct GWEN_IPCNODE GWEN_IPCNODE;
GWEN_LIST_FUNCTION_DEFS(GWEN_IPCNODE, GWEN_IpcNode)
struct GWEN_IPCNODE {
  GWEN_LIST_ELEMENT(GWEN_IPCNODE)
  GWEN_NETLAYER *netLayer;
  int isServer;
  int isPassiveClient;
  GWEN_TYPE_UINT32 id;
  GWEN_TYPE_UINT32 mark;
  GWEN_TYPE_UINT32 usage;
  GWEN_TYPE_UINT32 flags;

  GWEN_TYPE_UINT32 nextMsgId;
  GWEN_TYPE_UINT32 lastMsgId;
};
GWEN_IPCNODE *GWEN_IpcNode_new();
void GWEN_IpcNode_free(GWEN_IPCNODE *n);
void GWEN_IpcNode_Attach(GWEN_IPCNODE *n);



/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Message
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

typedef struct GWEN_IPCMSG GWEN_IPCMSG;
GWEN_LIST_FUNCTION_DEFS(GWEN_IPCMSG, GWEN_IpcMsg)
struct GWEN_IPCMSG {
  GWEN_LIST_ELEMENT(GWEN_IPCMSG)
  GWEN_IPCNODE *node;

  GWEN_TYPE_UINT32 id;
  GWEN_TYPE_UINT32 refId;
  GWEN_DB_NODE *db;
  time_t sendTime;
  time_t receivedTime;
  GWEN_NL_PACKET *packet;
};
GWEN_IPCMSG *GWEN_IpcMsg_new(GWEN_IPCNODE *n);
void GWEN_IpcMsg_free(GWEN_IPCMSG *m);



/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Request
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

typedef struct GWEN_IPC__REQUEST GWEN_IPC__REQUEST;
GWEN_LIST_FUNCTION_DEFS(GWEN_IPC__REQUEST, GWEN_Ipc__Request)
struct GWEN_IPC__REQUEST {
  GWEN_LIST_ELEMENT(GWEN_IPC__REQUEST)
  GWEN_INHERIT_ELEMENT(GWEN_IPC__REQUEST)

  GWEN_TYPE_UINT32 id;
  GWEN_IPCMSG_LIST *requestMsgs;
  GWEN_IPCMSG_LIST *responseMsgs;

  int usage;
};
GWEN_IPC__REQUEST *GWEN_Ipc__Request_new();
void GWEN_Ipc__Request_Attach(GWEN_IPC__REQUEST *r);
void GWEN_Ipc__Request_free(GWEN_IPC__REQUEST *r);
void GWEN_Ipc__Request_AddRequestMsg(GWEN_IPC__REQUEST *r, GWEN_IPCMSG *m);
void GWEN_Ipc__Request_AddResponseMsg(GWEN_IPC__REQUEST *r, GWEN_IPCMSG *m);
int GWEN_Ipc__Request_HasRequestMsg(GWEN_IPC__REQUEST *r,
                                  GWEN_TYPE_UINT32 nid,
                                  GWEN_TYPE_UINT32 id);
GWEN_IPCMSG_LIST *GWEN_Ipc__Request_GetRequestMsgList(const GWEN_IPC__REQUEST *r);





/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                IPC Manager
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

struct GWEN_IPCMANAGER {
  char *application;
  GWEN_IPCNODE_LIST *nodes;
  GWEN_IPC__REQUEST_LIST *outRequests;
  GWEN_IPC__REQUEST_LIST *newInRequests;
  GWEN_IPC__REQUEST_LIST *oldInRequests;
  unsigned int sendTimeOut;
  GWEN_TYPE_UINT32 usage;
};

GWEN_IPC__REQUEST *GWEN_IpcManager__FindRequest(GWEN_IPCMANAGER *mgr,
                                              GWEN_TYPE_UINT32 rid,
                                              GWEN_IPC__REQUEST *r);
int GWEN_IpcManager__SendMsg(GWEN_IPCMANAGER *mgr,
                             GWEN_IPCMSG *m);
GWEN_IPCMSG *GWEN_IpcManager__MakeErrorResponse(GWEN_IPCMANAGER *mgr,
                                                GWEN_IPCMSG *m,
                                                int code,
                                                const char *txt);
int GWEN_IpcManager__CheckRequests(GWEN_IPCMANAGER *mgr);

void GWEN_IpcManager__RemoveNodeRequestMessages(GWEN_IPCMANAGER *mgr,
                                                GWEN_IPCNODE *n,
                                                GWEN_IPC__REQUEST_LIST *rl,
                                                const char *msgType);
int GWEN_IpcManager__HandlePacket(GWEN_IPCMANAGER *mgr,
                                  GWEN_IPCNODE *n,
                                  GWEN_NL_PACKET *pk);

int GWEN_IpcManager__Work(GWEN_IPCMANAGER *mgr);

void GWEN_IpcNode_Dump(GWEN_IPCNODE *n, FILE *f, int indent);
void GWEN_IpcMsg_Dump(GWEN_IPCMSG *m, FILE *f, int indent);
void GWEN_Ipc__Request_Dump(GWEN_IPC__REQUEST *r, FILE *f, int indent);


#endif /* GWEN_IPC_P_H */




