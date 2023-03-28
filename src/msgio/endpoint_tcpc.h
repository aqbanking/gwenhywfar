/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_TCPC_H
#define GWEN_MSG_ENDPOINT_TCPC_H


#include <gwenhywfar/endpoint.h>



#ifdef __cplusplus
extern "C" {
#endif


#define GWEN_MSG_ENDPOINT_TCPC_STATE_UNCONNECTED 0
#define GWEN_MSG_ENDPOINT_TCPC_STATE_CONNECTING  1
#define GWEN_MSG_ENDPOINT_TCPC_STATE_CONNECTED   2
#define GWEN_MSG_ENDPOINT_TCPC_STATE_NEXTFREE    3



GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_TcpcEndpoint_new(const char *host, int port, const char *name, int groupId);

GWENHYWFAR_API int GWEN_TcpcEndpoint_StartConnect(GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API int GWEN_TcpcEndpoint_GetState(const GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_TcpcEndpoint_SetState(GWEN_MSG_ENDPOINT *ep, int m);




#ifdef __cplusplus
}
#endif


#endif


