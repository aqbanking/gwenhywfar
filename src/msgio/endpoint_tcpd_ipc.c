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


#include "msgio/endpoint_tcpd_ipc.h"
#include "msgio/endpoint_ipc.h"
#include "msgio/endpoint_tcpd.h"
#include "msgio/msg_ipc.h"

#include <gwenhywfar/debug.h>


#define GWEN_MSG_ENDPOINT_TCPD_IPC_NAME   "tcpd_ipc"



static GWEN_MSG_ENDPOINT *_createChild(GWEN_MSG_ENDPOINT *ep);





GWEN_MSG_ENDPOINT *GWEN_IpcTcpdEndpoint_new(const char *host, int port, const char *name, int groupId)
{
  GWEN_MSG_ENDPOINT *ep;

  ep=GWEN_TcpdEndpoint_new(host, port, name?name:GWEN_MSG_ENDPOINT_TCPD_IPC_NAME, groupId);
  GWEN_MsgEndpoint_SetCreateChildFn(ep, _createChild);
  return ep;
}



GWEN_MSG_ENDPOINT *_createChild(GWEN_MSG_ENDPOINT *ep)
{
  return GWEN_IpcEndpoint_new("TCP Client", GWEN_MsgEndpoint_GetGroupId(ep));
}



