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


#include "msgio/endpoint_ipc.h"
#include "msgio/msg_ipc.h"

#include <gwenhywfar/debug.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>




#define GWEN_MSG_ENDPOINT_IPC_NAME   "ipc"
#define GWEN_ENDPOINT_IPC_BUFFERSIZE 256




static int _getReadFd(GWEN_MSG_ENDPOINT *ep);
static int _getWriteFd(GWEN_MSG_ENDPOINT *ep);
static int _handleReadable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
static int _handleWritable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);






GWEN_MSG_ENDPOINT *GWEN_MsgEndpointIpc_new(int groupId)
{
  GWEN_MSG_ENDPOINT *ep;

  ep=GWEN_MsgEndpoint_new(GWEN_MSG_ENDPOINT_IPC_NAME, groupId);
  GWEN_MsgEndpoint_SetHandleReadableFn(ep, _handleReadable);
  GWEN_MsgEndpoint_SetHandleWritableFn(ep, _handleWritable);
  GWEN_MsgEndpoint_SetGetReadFdFn(ep, _getReadFd);
  GWEN_MsgEndpoint_SetGetWriteFdFn(ep, _getWriteFd);

  return ep;
}




int _getReadFd(GWEN_MSG_ENDPOINT *ep)
{
  return GWEN_MsgEndpoint_GetFd(ep);
}



int _getWriteFd(GWEN_MSG_ENDPOINT *ep)
{
  return GWEN_MsgEndpoint_HaveMessageToSend(ep)?GWEN_MsgEndpoint_GetFd(ep):GWEN_ERROR_NO_DATA;
}



int _handleReadable(GWEN_MSG_ENDPOINT *ep, GWEN_UNUSED GWEN_MSG_ENDPOINT_MGR *emgr)
{
  int rv;
  uint8_t buffer[GWEN_ENDPOINT_IPC_BUFFERSIZE];
  int len;
  int i;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Reading from endpoint %s", GWEN_MsgEndpoint_GetName(ep));
  do {
    rv=read(GWEN_MsgEndpoint_GetFd(ep), buffer, sizeof(buffer));
  } while( (rv<0) && errno==EINTR);
  if (rv<0) {
    if (errno==EAGAIN || errno==EWOULDBLOCK)
      return GWEN_ERROR_TRY_AGAIN;
    DBG_ERROR(GWEN_LOGDOMAIN, "Error on read(): %s (%d)", strerror(errno), errno);
    return GWEN_ERROR_IO;
  }
  else if (rv==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "EOF met on read()");
    return GWEN_ERROR_IO;
  }
  len=rv;

  for (i=0; i<len; i++) {
    GWEN_MSG *msg;

    msg=GWEN_MsgEndpoint_GetCurrentlyReceivedMsg(ep);
    if (msg==NULL) {
      msg=GWEN_Msg_new(GWEN_ENDPOINT_IPC_BUFFERSIZE);
      GWEN_MsgEndpoint_SetCurrentlyReceivedMsg(ep, msg);
    }
    rv=GWEN_Msg_AddByte(msg, buffer[i]);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    rv=GWEN_MsgIpc_IsMsgComplete(msg);
    if (rv<0) {
      /* invalid message */
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid message, discarding");
      GWEN_MsgEndpoint_SetCurrentlyReceivedMsg(ep, NULL);
      rv=GWEN_MsgEndpoint_DiscardInput(ep);
      if (rv<0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
    else if (rv>0) {
      /* complete msg received, add to list */
      GWEN_Msg_Attach(msg);
      GWEN_MsgEndpoint_SetCurrentlyReceivedMsg(ep, NULL);
      GWEN_MsgEndpoint_AddReceivedMessage(ep, msg);
    }
  } /* for */

  return 0;
}



int _handleWritable(GWEN_MSG_ENDPOINT *ep, GWEN_UNUSED GWEN_MSG_ENDPOINT_MGR *emgr)
{
  GWEN_MSG *msg;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Writing to endpoint %s", GWEN_MsgEndpoint_GetName(ep));
  msg=GWEN_MsgEndpoint_GetFirstSendMessage(ep);
  if (msg) {
    uint8_t pos;
    int remaining;
    int rv;

    pos=GWEN_Msg_GetCurrentPos(msg);
    remaining=GWEN_Msg_GetRemainingBytes(msg);
    if (remaining>0) {
      const uint8_t *buf;
      int fd;

      fd=GWEN_MsgEndpoint_GetFd(ep);
      /* start new message */
      buf=GWEN_Msg_GetBuffer(msg)+pos;
      do {
        rv=write(fd, buf, remaining);
      } while(rv<0 && errno==EINTR);
      if (rv<0) {
        if (errno==EAGAIN || errno==EWOULDBLOCK)
          return GWEN_ERROR_TRY_AGAIN;
        DBG_ERROR(GWEN_LOGDOMAIN, "Error on write(): %s (%d)", strerror(errno), errno);
        return GWEN_ERROR_IO;
      }
      GWEN_Msg_IncCurrentPos(msg, rv);
      if (rv==remaining) {
        /* end current message */
        GWEN_Msg_List_Del(msg);
	GWEN_Msg_free(msg);
      }
    }
  }
  return 0;
}



