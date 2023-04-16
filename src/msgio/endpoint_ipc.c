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




static int _isMsgComplete(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *msg);






GWEN_MSG_ENDPOINT *GWEN_IpcEndpoint_new(const char *name, int groupId)
{
  GWEN_MSG_ENDPOINT *ep;

  ep=GWEN_MsgEndpoint_new(name?name:GWEN_MSG_ENDPOINT_IPC_NAME, groupId);
  GWEN_IpcEndpoint_Extend(ep);
  return ep;
}



void GWEN_IpcEndpoint_Extend(GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    GWEN_MsgEndpoint_SetDefaultBufferSize(ep, GWEN_ENDPOINT_IPC_BUFFERSIZE);
    GWEN_MsgEndpoint_SetIsMsgCompleteFn(ep, _isMsgComplete);
  }
}



int _isMsgComplete(GWEN_UNUSED GWEN_MSG_ENDPOINT *ep, GWEN_MSG *msg)
{
  return GWEN_IpcMsg_IsMsgComplete(msg);
}



