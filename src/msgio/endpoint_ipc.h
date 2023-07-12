/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_IPC_H
#define GWEN_MSG_ENDPOINT_IPC_H


#include <gwenhywfar/endpoint.h>

#include <time.h>



#ifdef __cplusplus
extern "C" {
#endif


/**
 * Create a IPC client which works over a TCP connection.
 */
GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_IpcEndpoint_CreateIpcTcpClient(const char *host, int port, const char *name, int groupId);

/**
 * Create a IPC service for the given socket which works over a TCP connection.
 * The given socket is expected to be created by accepting an incoming connection (e.g. from the callback
 * function @ref GWEN_ENDPOINT2_TCPD_ACCEPT_FN).
 */
GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_IpcEndpoint_CreateIpcTcpServiceForSocket(GWEN_SOCKET *sk, const char *name, int groupId);


/**
 * Extends the given endpoint to support GWEN IPC messages. It expects the function GWEN_MsgIoEndpoint2_Extend() to have been called
 * beforehand.
 */
GWENHYWFAR_API void GWEN_IpcEndpoint_Extend(GWEN_MSG_ENDPOINT *ep);





#ifdef __cplusplus
}
#endif


#endif


