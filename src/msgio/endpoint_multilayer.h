/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_MULTILAYER_H
#define GWEN_MSG_ENDPOINT_MULTILAYER_H


#include <gwenhywfar/endpoint.h>



#ifdef __cplusplus
extern "C" {
#endif

typedef int (*GWEN_ENDPOINT_MULTILAYER_STARTCONNECT_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT *epChild);

/**
 * Called from @ref GWEN_MsgEndpoint_CheckSockets when status is GWEN_MSG_ENDPOINT_STATE_CONNECTING.
 */
typedef void (*GWEN_ENDPOINT_MULTILAYER_CHECKSOCKETS_FN)(GWEN_MSG_ENDPOINT *ep,
                                                         GWEN_MSG_ENDPOINT *epChild,
                                                         GWEN_SOCKETSET *readSet,
                                                         GWEN_SOCKETSET *writeSet,
                                                         GWEN_SOCKETSET *xSet);



GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_MultilayerEndpoint_new(const char *name, int groupId);
GWENHYWFAR_API int GWEN_MultilayerEndpoint_StartConnect(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API int GWEN_MultilayerEndpoint_GetReconnectTimeInSeconds(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MultilayerEndpoint_SetReconnectTimeInSeconds(GWEN_MSG_ENDPOINT *ep, int i);

GWENHYWFAR_API int GWEN_MultilayerEndpoint_GetConnectTimeoutInSeconds(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MultilayerEndpoint_SetConnectTimeoutInSeconds(GWEN_MSG_ENDPOINT *ep, int i);

GWENHYWFAR_API int GWEN_MultilayerEndpoint_GetStage(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MultilayerEndpoint_SetStage(GWEN_MSG_ENDPOINT *ep, int i);


GWENHYWFAR_API
GWEN_ENDPOINT_MULTILAYER_STARTCONNECT_FN GWEN_MultilayerEndpoint_SetStartConnectFn(GWEN_MSG_ENDPOINT *ep,
                                                                                   GWEN_ENDPOINT_MULTILAYER_STARTCONNECT_FN fn);

GWENHYWFAR_API
GWEN_ENDPOINT_MULTILAYER_CHECKSOCKETS_FN GWEN_MultilayerEndpoint_SetCheckSocketsFn(GWEN_MSG_ENDPOINT *ep,
                                                                                   GWEN_ENDPOINT_MULTILAYER_CHECKSOCKETS_FN fn);


#ifdef __cplusplus
}
#endif


#endif


