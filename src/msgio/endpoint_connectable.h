/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_CONNECTABLE_H
#define GWEN_MSG_ENDPOINT_CONNECTABLE_H

#include <gwenhywfar/endpoint.h>

#include <time.h>



#define GWEN_MSG_ENDPOINT_CONN_STATE_DISCONNECTED -1
#define GWEN_MSG_ENDPOINT_CONN_STATE_UNCONNECTED   0
#define GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTING    1
#define GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTED     2
#define GWEN_MSG_ENDPOINT_CONN_STATE_NEXTFREE      3


#ifdef __cplusplus
extern "C" {
#endif


typedef int (*GWEN_CONN_ENDPOINT_CONNECT_FN)(GWEN_MSG_ENDPOINT *ep);
typedef void (*GWEN_CONN_ENDPOINT_DISCONNECT_FN)(GWEN_MSG_ENDPOINT *ep);


GWENHYWFAR_API void GWEN_ConnectableMsgEndpoint_Extend(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API int GWEN_ConnectableMsgEndpoint_GetReconnectWaitTime(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_ConnectableMsgEndpoint_SetReconnectWaitTime(GWEN_MSG_ENDPOINT *ep, int t);


GWENHYWFAR_API int GWEN_ConnectableMsgEndpoint_GetState(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_ConnectableMsgEndpoint_SetState(GWEN_MSG_ENDPOINT *ep, int m);

GWENHYWFAR_API time_t GWEN_ConnectableMsgEndpoint_GetTimeOfLastStateChange(const GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API int GWEN_ConnectableMsgEndpoint_Connect(GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_ConnectableMsgEndpoint_Disconnect(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API
GWEN_CONN_ENDPOINT_CONNECT_FN GWEN_ConnectableMsgEndpoint_SetConnectFn(GWEN_MSG_ENDPOINT *ep,
                                                                       GWEN_CONN_ENDPOINT_CONNECT_FN fn);

GWENHYWFAR_API
GWEN_CONN_ENDPOINT_DISCONNECT_FN GWEN_ConnectableMsgEndpoint_SetDisconnectFn(GWEN_MSG_ENDPOINT *ep,
                                                                             GWEN_CONN_ENDPOINT_DISCONNECT_FN fn);


#ifdef __cplusplus
}
#endif


#endif


