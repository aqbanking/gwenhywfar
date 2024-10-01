/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*#define DISABLE_DEBUGLOG*/


#include "msgio/endpoint_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>


#define GWEN_MSG_ENDPOINT_DEFAULT_MSGSIZE 1024



GWEN_INHERIT_FUNCTIONS(GWEN_MSG_ENDPOINT)
GWEN_TREE2_FUNCTIONS(GWEN_MSG_ENDPOINT, GWEN_MsgEndpoint)



GWEN_MSG_ENDPOINT *GWEN_MsgEndpoint_new(const char *name, int groupId)
{
  GWEN_MSG_ENDPOINT *ep;

  GWEN_NEW_OBJECT(GWEN_MSG_ENDPOINT, ep);
  ep->refCount=1;
  GWEN_INHERIT_INIT(GWEN_MSG_ENDPOINT, ep);
  GWEN_TREE2_INIT(GWEN_MSG_ENDPOINT, ep, GWEN_MsgEndpoint);

  ep->name=strdup(name?name:"<unnamed>");
  ep->groupId=groupId;

  ep->receivedMessageList=GWEN_Msg_List_new();
  ep->sendMessageList=GWEN_Msg_List_new();

  ep->defaultMessageSize=GWEN_MSG_ENDPOINT_DEFAULT_MSGSIZE;

  return ep;
}



void GWEN_MsgEndpoint_Attach(GWEN_MSG_ENDPOINT *ep)
{
  if (ep && ep->refCount)
    ep->refCount++;
}



void GWEN_MsgEndpoint_free(GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    if (ep->refCount>1)
      ep->refCount--;
    else {
      DBG_INFO(GWEN_LOGDOMAIN,
               "Deleting endpoint \"%s\" (%d msgs in recv list, %d msgs in send list)",
               (ep->name)?(ep->name):"<unnamed>",
               GWEN_Msg_List_GetCount(ep->receivedMessageList),
               GWEN_Msg_List_GetCount(ep->sendMessageList));
      GWEN_TREE2_FINI(GWEN_MSG_ENDPOINT, ep, GWEN_MsgEndpoint);
      GWEN_INHERIT_FINI(GWEN_MSG_ENDPOINT, ep);
      if (ep->socket) {
        GWEN_Socket_Close(ep->socket);
        GWEN_Socket_free(ep->socket);
      }
      GWEN_Msg_free(ep->currentlyReceivedMsg);
      GWEN_Msg_List_free(ep->receivedMessageList);
      GWEN_Msg_List_free(ep->sendMessageList);
      free(ep->name);
      ep->refCount=0;
      GWEN_FREE_OBJECT(ep);
    }
  }
}



const char *GWEN_MsgEndpoint_GetName(const GWEN_MSG_ENDPOINT *ep)
{
  return (ep?ep->name:NULL);
}



int GWEN_MsgEndpoint_GetGroupId(const GWEN_MSG_ENDPOINT *ep)
{
  return (ep?ep->groupId:0);
}



GWEN_SOCKET *GWEN_MsgEndpoint_GetSocket(const GWEN_MSG_ENDPOINT *ep)
{
  return (ep?ep->socket:NULL);
}



void GWEN_MsgEndpoint_SetSocket(GWEN_MSG_ENDPOINT *ep, GWEN_SOCKET *sk)
{
  if (ep) {
    if (ep->socket) {
      GWEN_Socket_Close(ep->socket);
      GWEN_Socket_free(ep->socket);
    }
    ep->socket=sk;
  }
}



int GWEN_MsgEndpoint_GetState(const GWEN_MSG_ENDPOINT *ep)
{
  return (ep?ep->state:GWEN_MSG_ENDPOINT_STATE_UNCONNECTED);
}



void GWEN_MsgEndpoint_SetState(GWEN_MSG_ENDPOINT *ep, int m)
{
  if (ep) {
    if (ep->state!=m) {
      ep->timeOfLastStateChange=time(NULL);
      DBG_INFO(GWEN_LOGDOMAIN, "Changing status of endpoint %s to %d", GWEN_MsgEndpoint_GetName(ep), m);
      ep->state=m;
    }
  }
}



time_t GWEN_MsgEndpoint_GetTimeOfLastStateChange(const GWEN_MSG_ENDPOINT *ep)
{
  return (ep?ep->timeOfLastStateChange:0);
}






uint32_t GWEN_MsgEndpoint_GetFlags(const GWEN_MSG_ENDPOINT *ep)
{
  return (ep?ep->flags:0);
}



void GWEN_MsgEndpoint_SetFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f)
{
  if (ep)
    ep->flags=f;
}



void GWEN_MsgEndpoint_AddFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f)
{
  if (ep)
    ep->flags|=f;
}



void GWEN_MsgEndpoint_DelFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f)
{
  if (ep)
    ep->flags&=~f;
}



int GWEN_MsgEndpoint_GetDefaultMessageSize(const GWEN_MSG_ENDPOINT *ep)
{
  return ep?ep->defaultMessageSize:0;
}



void GWEN_MsgEndpoint_SetDefaultMessageSize(GWEN_MSG_ENDPOINT *ep, int i)
{
  if (ep)
    ep->defaultMessageSize=i;
}



uint32_t GWEN_MsgEndpoint_GetNextMessageId(GWEN_MSG_ENDPOINT *ep)
{
  return ep?(++(ep->lastMsgId)):0;
}



GWEN_MSG_LIST *GWEN_MsgEndpoint_GetReceivedMessageList(const GWEN_MSG_ENDPOINT *ep)
{
  return (ep?ep->receivedMessageList:NULL);
}



GWEN_MSG_LIST *GWEN_MsgEndpoint_GetSendMessageList(const GWEN_MSG_ENDPOINT *ep)
{
  return (ep?ep->sendMessageList:NULL);
}



void GWEN_MsgEndpoint_AddReceivedMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m)
{
  if (ep && m) {
    GWEN_Msg_RewindCurrentPos(m);
    GWEN_Msg_List_Add(m, ep->receivedMessageList);
  }
}



GWEN_MSG *GWEN_MsgEndpoint_GetFirstReceivedMessage(const GWEN_MSG_ENDPOINT *ep)
{
  return ep?GWEN_Msg_List_First(ep->receivedMessageList):NULL;
}



GWEN_MSG *GWEN_MsgEndpoint_TakeFirstReceivedMessage(GWEN_MSG_ENDPOINT *ep)
{
  GWEN_MSG *msg;

  msg=GWEN_MsgEndpoint_GetFirstReceivedMessage(ep);
  if (msg)
    GWEN_Msg_List_Del(msg);
  return msg;
}



void GWEN_MsgEndpoint_AddSendMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m)
{
  if (ep && m) {
    GWEN_Msg_RewindCurrentPos(m);
    GWEN_Msg_List_Add(m, ep->sendMessageList);
  }
}



GWEN_MSG *GWEN_MsgEndpoint_GetFirstSendMessage(const GWEN_MSG_ENDPOINT *ep)
{
  return (ep?GWEN_Msg_List_First(ep->sendMessageList):NULL);
}



int GWEN_MsgEndpoint_HaveMessageToSend(const GWEN_MSG_ENDPOINT *ep)
{
  return (ep && GWEN_Msg_List_GetCount(ep->sendMessageList)>0)?1:0;
}



GWEN_MSG *GWEN_MsgEndpoint_GetCurrentlyReceivedMsg(const GWEN_MSG_ENDPOINT *ep)
{
  return (ep?ep->currentlyReceivedMsg:NULL);
}



void GWEN_MsgEndpoint_SetCurrentlyReceivedMsg(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m)
{
  if (ep)
    ep->currentlyReceivedMsg=m;
}



void GWEN_MsgEndpoint_AddSockets(GWEN_MSG_ENDPOINT *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet)
{
  if (ep && ep->addSocketsFn)
    ep->addSocketsFn(ep, readSet, writeSet, xSet);
}



void GWEN_MsgEndpoint_CheckSockets(GWEN_MSG_ENDPOINT *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet)
{
  if (ep && ep->checkSocketsFn)
    ep->checkSocketsFn(ep, readSet, writeSet, xSet);
}



void GWEN_MsgEndpoint_ChildrenAddSockets(GWEN_MSG_ENDPOINT *ep,
                                          GWEN_SOCKETSET *readSet,
                                          GWEN_SOCKETSET *writeSet,
                                          GWEN_SOCKETSET *xSet)
{
  GWEN_MSG_ENDPOINT *epChild;

  epChild=GWEN_MsgEndpoint_Tree2_GetFirstChild(ep);
  while(epChild) {
    GWEN_MsgEndpoint_AddSockets(epChild, readSet, writeSet, xSet);
    epChild=GWEN_MsgEndpoint_Tree2_GetNext(epChild);
  }
}



void GWEN_MsgEndpoint_ChildrenCheckSockets(GWEN_MSG_ENDPOINT *ep,
                                            GWEN_SOCKETSET *readSet,
                                            GWEN_SOCKETSET *writeSet,
                                            GWEN_SOCKETSET *xSet)
{
  GWEN_MSG_ENDPOINT *epChild;

  epChild=GWEN_MsgEndpoint_Tree2_GetFirstChild(ep);
  while(epChild) {
    GWEN_MsgEndpoint_CheckSockets(epChild, readSet, writeSet, xSet);
    epChild=GWEN_MsgEndpoint_Tree2_GetNext(epChild);
  }
}



void GWEN_MsgEndpoint_RemoveUnconnectedAndEmptyChildren(GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    GWEN_MSG_ENDPOINT *epChild;

    epChild=GWEN_MsgEndpoint_Tree2_GetFirstChild(ep);
    while(epChild) {
      GWEN_MSG_ENDPOINT *epNext;

      epNext=GWEN_MsgEndpoint_Tree2_GetNext(epChild);
      if ((GWEN_MsgEndpoint_GetState(epChild)==GWEN_MSG_ENDPOINT_STATE_UNCONNECTED) &&
          (GWEN_MsgEndpoint_GetFirstReceivedMessage(epChild)==NULL)) {
        DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Disconnected and empty, removing", GWEN_MsgEndpoint_GetName(epChild));
        GWEN_MsgEndpoint_Tree2_Unlink(epChild);
        GWEN_MsgEndpoint_free(epChild);
      }
      epChild=epNext;
    }
  }
}



void GWEN_MsgEndpoint_IoLoop(GWEN_MSG_ENDPOINT *ep, int timeout)
{
  GWEN_SOCKETSET *readSet;
  GWEN_SOCKETSET *writeSet;
  GWEN_SOCKETSET *xSet;
  int rv;

  readSet=GWEN_SocketSet_new();
  writeSet=GWEN_SocketSet_new();
  xSet=GWEN_SocketSet_new();
  GWEN_MsgEndpoint_AddSockets(ep, readSet, writeSet, xSet);

  do {
    rv=GWEN_Socket_Select(GWEN_SocketSet_GetSocketCount(readSet)?readSet:NULL,
                          GWEN_SocketSet_GetSocketCount(writeSet)?writeSet:NULL,
                          GWEN_SocketSet_GetSocketCount(xSet)?xSet:NULL,
                          timeout);
  } while(rv==GWEN_ERROR_INTERRUPTED);
  if (rv<0) {
    if (rv!=GWEN_ERROR_TIMEOUT) {
      DBG_INFO(GWEN_LOGDOMAIN, "Error on GWEN_Socket_Select: %d", rv);
    }
  }
  else
    GWEN_MsgEndpoint_CheckSockets(ep, readSet, writeSet, xSet);
  GWEN_SocketSet_free(xSet);
  GWEN_SocketSet_free(writeSet);
  GWEN_SocketSet_free(readSet);
}



void GWEN_MsgEndpoint_ChildrenIoLoop(GWEN_MSG_ENDPOINT *ep, int timeout)
{
  GWEN_SOCKETSET *readSet;
  GWEN_SOCKETSET *writeSet;
  GWEN_SOCKETSET *xSet;
  int rv;

  readSet=GWEN_SocketSet_new();
  writeSet=GWEN_SocketSet_new();
  xSet=GWEN_SocketSet_new();
  GWEN_MsgEndpoint_ChildrenAddSockets(ep, readSet, writeSet, xSet);

  do {
    rv=GWEN_Socket_Select(GWEN_SocketSet_GetSocketCount(readSet)?readSet:NULL,
                          GWEN_SocketSet_GetSocketCount(writeSet)?writeSet:NULL,
                          GWEN_SocketSet_GetSocketCount(xSet)?xSet:NULL,
                          timeout);
  } while(rv==GWEN_ERROR_INTERRUPTED);
  if (rv<0) {
    if (rv!=GWEN_ERROR_TIMEOUT) {
      DBG_INFO(GWEN_LOGDOMAIN, "Error on GWEN_Socket_Select: %d", rv);
    }
  }
  else
    GWEN_MsgEndpoint_ChildrenCheckSockets(ep, readSet, writeSet, xSet);
  GWEN_SocketSet_free(xSet);
  GWEN_SocketSet_free(writeSet);
  GWEN_SocketSet_free(readSet);
}



int GWEN_MsgEndpoint_ReadFromSocket(GWEN_MSG_ENDPOINT *ep, uint8_t *bufferPtr, uint32_t bufferLen)
{
  int len;
  int rv;

  len=bufferLen;
  DBG_DEBUG(GWEN_LOGDOMAIN, "Endpoint %s: Reading from socket", GWEN_MsgEndpoint_GetName(ep));
  do {
    rv=GWEN_Socket_Read(ep->socket, (char*) bufferPtr, &len);
  } while(rv==GWEN_ERROR_INTERRUPTED);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: here (%d)", GWEN_MsgEndpoint_GetName(ep), rv);
    return rv;
  }
  DBG_DEBUG(GWEN_LOGDOMAIN, "Endpoint %s: Read %d bytes from socket", GWEN_MsgEndpoint_GetName(ep), len);
  return len;
}



int GWEN_MsgEndpoint_WriteToSocket(GWEN_MSG_ENDPOINT *ep, const uint8_t *bufferPtr, uint32_t bufferLen)
{
  int len;
  int rv;

  len=bufferLen;
  do {
    rv=GWEN_Socket_Write(ep->socket, (const char*) bufferPtr, &len);
  } while(rv==GWEN_ERROR_INTERRUPTED);
  if (rv<0)
    return rv;
  return len;
}



int GWEN_MsgEndpoint_DiscardInput(GWEN_MSG_ENDPOINT *ep)
{
  int rv;
  uint8_t buffer[64];

  do {
    rv=GWEN_MsgEndpoint_ReadFromSocket(ep, buffer, sizeof(buffer));
  } while(rv>0);
  if (rv<0 && rv!=GWEN_ERROR_TIMEOUT) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error on read(): %d", rv);
    return rv;
  }
  else if (rv==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "EOF met on read()");
#if 0
    return GWEN_ERROR_IO;
#endif
  }
  return 0;
}



void GWEN_MsgEndpoint_Disconnect(GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    DBG_INFO(GWEN_LOGDOMAIN, "Disconnecting endpoint");
    if (ep->socket) {
      DBG_INFO(GWEN_LOGDOMAIN, "Disconnecting socket");
      GWEN_Socket_Close(ep->socket);
      GWEN_Socket_free(ep->socket);
      ep->socket=NULL;
    }
    GWEN_MsgEndpoint_SetState(ep, GWEN_MSG_ENDPOINT_STATE_UNCONNECTED);
  }
}








GWEN_MSG_ENDPOINT_ADDSOCKETS_FN GWEN_MsgEndpoint_SetAddSocketsFn(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_ADDSOCKETS_FN fn)
{
  if (ep) {
    GWEN_MSG_ENDPOINT_ADDSOCKETS_FN o;

    o=ep->addSocketsFn;
    ep->addSocketsFn=fn;
    return o;
  }
  return NULL;
}



GWEN_MSG_ENDPOINT_CHECKSOCKETS_FN GWEN_MsgEndpoint_SetCheckSocketsFn(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_CHECKSOCKETS_FN fn)
{
  if (ep) {
    GWEN_MSG_ENDPOINT_CHECKSOCKETS_FN o;

    o=ep->checkSocketsFn;
    ep->checkSocketsFn=fn;
    return o;
  }
  return NULL;
}



