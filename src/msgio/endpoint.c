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

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif


#define GWEN_MSG_ENDPOINT_BUFFERSIZE 1024



GWEN_INHERIT_FUNCTIONS(GWEN_MSG_ENDPOINT)
GWEN_LIST_FUNCTIONS(GWEN_MSG_ENDPOINT, GWEN_MsgEndpoint)




GWEN_MSG_ENDPOINT *GWEN_MsgEndpoint_new(const char *name, int groupId)
{
  GWEN_MSG_ENDPOINT *ep;

  GWEN_NEW_OBJECT(GWEN_MSG_ENDPOINT, ep);
  GWEN_INHERIT_INIT(GWEN_MSG_ENDPOINT, ep);
  GWEN_LIST_INIT(GWEN_MSG_ENDPOINT, ep);
  ep->fd=-1;
  ep->receivedMessageList=GWEN_Msg_List_new();
  ep->sendMessageList=GWEN_Msg_List_new();
  ep->name=name?strdup(name):"<unnamed>";
  ep->groupId=groupId;

  return ep;
}



void GWEN_MsgEndpoint_free(GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    GWEN_LIST_FINI(GWEN_MSG_ENDPOINT, ep);
    GWEN_INHERIT_FINI(GWEN_MSG_ENDPOINT, ep);
    if (ep->fd>=0)
      close(ep->fd);
    GWEN_Msg_free(ep->currentlyReceivedMsg);
    GWEN_Msg_List_free(ep->receivedMessageList);
    GWEN_Msg_List_free(ep->sendMessageList);
    GWEN_FREE_OBJECT(ep);
  }
}



int GWEN_MsgEndpoint_GetFd(const GWEN_MSG_ENDPOINT *ep)
{
  return ep->fd;
}



void GWEN_MsgEndpoint_SetFd(GWEN_MSG_ENDPOINT *ep, int fd)
{
  ep->fd=fd;
}



int GWEN_MsgEndpoint_GetGroupId(const GWEN_MSG_ENDPOINT *ep)
{
  return ep->groupId;
}



const char *GWEN_MsgEndpoint_GetName(const GWEN_MSG_ENDPOINT *ep)
{
  return ep->name;
}



uint32_t GWEN_MsgEndpoint_GetFlags(const GWEN_MSG_ENDPOINT *ep)
{
  return ep->flags;
}



void GWEN_MsgEndpoint_SetFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f)
{
  ep->flags=f;
}



void GWEN_MsgEndpoint_AddFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f)
{
  ep->flags|=f;
}



void GWEN_MsgEndpoint_DelFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f)
{
  ep->flags&=~f;
}




GWEN_MSG_LIST *GWEN_MsgEndpoint_GetReceivedMessageList(const GWEN_MSG_ENDPOINT *ep)
{
  return ep->receivedMessageList;
}



GWEN_MSG_LIST *GWEN_MsgEndpoint_GetSendMessageList(const GWEN_MSG_ENDPOINT *ep)
{
  return ep->sendMessageList;
}



void GWEN_MsgEndpoint_AddReceivedMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m)
{
  GWEN_Msg_List_Add(m, ep->receivedMessageList);
}



GWEN_MSG *GWEN_MsgEndpoint_TakeFirstReceivedMessage(GWEN_MSG_ENDPOINT *ep)
{
  GWEN_MSG *msg;

  msg=GWEN_Msg_List_First(ep->receivedMessageList);
  if (msg)
    GWEN_Msg_List_Del(msg);
  return msg;
}



void GWEN_MsgEndpoint_AddSendMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m)
{
  GWEN_Msg_List_Add(m, ep->sendMessageList);
}



int GWEN_MsgEndpoint_HaveMessageToSend(const GWEN_MSG_ENDPOINT *ep)
{
  return (GWEN_Msg_List_GetCount(ep->sendMessageList)>0)?1:0;
}



GWEN_MSG *GWEN_MsgEndpoint_GetFirstSendMessage(const GWEN_MSG_ENDPOINT *ep)
{
  return GWEN_Msg_List_First(ep->sendMessageList);
}



GWEN_MSG *GWEN_MsgEndpoint_GetCurrentlyReceivedMsg(const GWEN_MSG_ENDPOINT *ep)
{
  return ep->currentlyReceivedMsg;
}



void GWEN_MsgEndpoint_SetCurrentlyReceivedMsg(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m)
{
  GWEN_Msg_free(ep->currentlyReceivedMsg);
  ep->currentlyReceivedMsg=m;
}



int GWEN_MsgEndpoint_GetReadFd(GWEN_MSG_ENDPOINT *ep)
{
  return (ep->getReadFdFn)?(ep->getReadFdFn(ep)):(ep->fd);
}



int GWEN_MsgEndpoint_GetWriteFd(GWEN_MSG_ENDPOINT *ep)
{
  if (ep->getWriteFdFn)
    return ep->getWriteFdFn(ep);
  else {
    int somethingToWrite;

    somethingToWrite=(GWEN_Msg_List_First(ep->sendMessageList)!=NULL)?1:0;
    if (somethingToWrite)
      return ep->fd;
  }
  return GWEN_ERROR_NO_DATA;
}



int GWEN_MsgEndpoint_HandleReadable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr)
{
  return (ep->handleReadableFn)?(ep->handleReadableFn(ep, emgr)):0;
}



int GWEN_MsgEndpoint_HandleWritable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr)
{
  return (ep->handleWritableFn)?(ep->handleWritableFn(ep, emgr)):0;
}



void GWEN_MsgEndpoint_Run(GWEN_MSG_ENDPOINT *ep)
{
  if (ep->runFn)
    ep->runFn(ep);
}



void GWEN_MsgEndpoint_ProcessOutMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m)
{
  if (ep->processOutMsgFn)
    ep->processOutMsgFn(ep, m);
  else
    GWEN_MsgEndpoint_AddSendMessage(ep, m);
}



int GWEN_MsgEndpoint_IsMsgComplete(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m)
{
  if (ep->isMsgCompleteFn)
    return ep->isMsgCompleteFn(ep, m);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



GWEN_MSG_ENDPOINT *GWEN_MsgEndpoint_CreateChild(GWEN_MSG_ENDPOINT *ep)
{
  if (ep->createChildFn)
    return ep->createChildFn(ep);
  return NULL;
}



int GWEN_MsgEndpoint_DiscardInput(GWEN_MSG_ENDPOINT *ep)
{
  int rv;
  uint8_t buffer[64];

  do {
    rv=read(ep->fd, buffer, sizeof(buffer));
  } while(rv>0 || (rv<0 && errno==EINTR));
  if (rv<0 && errno!=EAGAIN && errno!=EWOULDBLOCK) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error on read(): %s (%d)", strerror(errno), errno);
    return GWEN_ERROR_IO;
  }
  else if (rv==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "EOF met on read()");
#if 0
    return GWEN_ERROR_IO;
#endif
  }
  return 0;
}



GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN GWEN_MsgEndpoint_SetHandleReadableFn(GWEN_MSG_ENDPOINT *ep,
                                                                         GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN f)
{
  GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN oldFn;

  oldFn=ep->handleReadableFn;
  ep->handleReadableFn=f;
  return oldFn;
}



GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN GWEN_MsgEndpoint_SetHandleWritableFn(GWEN_MSG_ENDPOINT *ep,
                                                                         GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN f)
{
  GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN oldFn;

  oldFn=ep->handleWritableFn;
  ep->handleWritableFn=f;
  return oldFn;
}



GWEN_MSG_ENDPOINT_GET_READFD_FN GWEN_MsgEndpoint_SetGetReadFdFn(GWEN_MSG_ENDPOINT *ep,
                                                                GWEN_MSG_ENDPOINT_GET_READFD_FN f)
{
  GWEN_MSG_ENDPOINT_GET_READFD_FN oldFn;

  oldFn=ep->getReadFdFn;
  ep->getReadFdFn=f;
  return oldFn;
}



GWEN_MSG_ENDPOINT_GET_WRITEFD_FN GWEN_MsgEndpoint_SetGetWriteFdFn(GWEN_MSG_ENDPOINT *ep,
                                                                  GWEN_MSG_ENDPOINT_GET_WRITEFD_FN f)
{
  GWEN_MSG_ENDPOINT_GET_WRITEFD_FN oldFn;

  oldFn=ep->getWriteFdFn;
  ep->getWriteFdFn=f;
  return oldFn;
}



GWEN_MSG_ENDPOINT_RUN_FN GWEN_MsgEndpoint_SetRunFn(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_RUN_FN f)
{
  GWEN_MSG_ENDPOINT_RUN_FN oldFn;

  oldFn=ep->runFn;
  ep->runFn=f;
  return oldFn;
}



GWEN_MSG_ENDPOINT_PROC_OUTMSG_FN GWEN_MsgEndpoint_SetProcessOutMsgFn(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_PROC_OUTMSG_FN f)
{
  GWEN_MSG_ENDPOINT_PROC_OUTMSG_FN oldFn;

  oldFn=ep->processOutMsgFn;
  ep->processOutMsgFn=f;
  return oldFn;
}



GWEN_MSG_ENDPOINT_ISMSGCOMPLETE_FN GWEN_MsgEndpoint_SetIsMsgCompleteFn(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_ISMSGCOMPLETE_FN f)
{
  GWEN_MSG_ENDPOINT_ISMSGCOMPLETE_FN oldFn;

  oldFn=ep->isMsgCompleteFn;
  ep->isMsgCompleteFn=f;
  return oldFn;
}



GWEN_MSG_ENDPOINT_CREATECHILD_FN GWEN_MsgEndpoint_SetCreateChildFn(GWEN_MSG_ENDPOINT *ep,
								   GWEN_MSG_ENDPOINT_CREATECHILD_FN f)
{
  GWEN_MSG_ENDPOINT_CREATECHILD_FN oldFn;

  oldFn=ep->createChildFn;
  ep->createChildFn=f;
  return oldFn;
}




