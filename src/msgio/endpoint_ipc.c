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


#include "./endpoint_ipc.h"
#include "./endpoint_msgio.h"
#include "./endpoint_tcpc.h"
#include "./msg_ipc.h"

#include <gwenhywfar/debug.h>


#define GWEN_MSG_ENDPOINT_IPC_NAME       "ipc"



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _getBytesNeededForMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *msg);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

void GWEN_IpcEndpoint_Extend(GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    GWEN_MsgIoEndpoint_SetGetNeededBytesFn(ep, _getBytesNeededForMessage);
  }
}



int _getBytesNeededForMessage(GWEN_UNUSED GWEN_MSG_ENDPOINT *ep, GWEN_MSG *msg)
{
  uint32_t bytesInMsg;

  bytesInMsg=GWEN_Msg_GetBytesInBuffer(msg);
  if (bytesInMsg<GWEN_MSGIPC_OFFS_PAYLOAD) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Header not yet complete");
    return (int) (GWEN_MSGIPC_OFFS_PAYLOAD-bytesInMsg);
  }
  else {
    uint32_t msgSize;

    msgSize=GWEN_IpcMsg_GetMsgSize(msg); /* IPC length field contains ALL bytes including header */
    return (int) (msgSize-bytesInMsg);
  }
}



GWEN_MSG_ENDPOINT *GWEN_IpcEndpoint_CreateIpcTcpClient(const char *host, int port, const char *name, int groupId)
{
  GWEN_MSG_ENDPOINT *ep;

  ep=GWEN_TcpcEndpoint_new(host, port, name?name:GWEN_MSG_ENDPOINT_IPC_NAME, groupId);
  GWEN_MsgIoEndpoint_Extend(ep);
  GWEN_IpcEndpoint_Extend(ep);
  return ep;
}



GWEN_MSG_ENDPOINT *GWEN_IpcEndpoint_CreateIpcTcpServiceForSocket(GWEN_SOCKET *sk, const char *name, int groupId)
{
  GWEN_MSG_ENDPOINT *ep;

  ep=GWEN_MsgEndpoint_new(name?name:GWEN_MSG_ENDPOINT_IPC_NAME, groupId);
  GWEN_MsgEndpoint_SetSocket(ep, sk);
  GWEN_MsgIoEndpoint_Extend(ep);
  GWEN_IpcEndpoint_Extend(ep);
  return ep;
}



