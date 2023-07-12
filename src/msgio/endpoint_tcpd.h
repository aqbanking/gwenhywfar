/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_TCPD_H
#define GWEN_MSG_ENDPOINT_TCPD_H


#include <gwenhywfar/endpoint.h>

#include <time.h>



#ifdef __cplusplus
extern "C" {
#endif


typedef GWEN_MSG_ENDPOINT* (*GWEN_ENDPOINT_TCPD_ACCEPT_FN)(GWEN_MSG_ENDPOINT *ep,
                                                           GWEN_SOCKET *sk,
                                                           const GWEN_INETADDRESS *addr,
                                                           void *data);



GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_TcpdEndpoint_new(const char *host, int port, const char *name, int groupId);


GWENHYWFAR_API void GWEN_TcpdEndpoint_SetAcceptFn(GWEN_MSG_ENDPOINT *ep, GWEN_ENDPOINT_TCPD_ACCEPT_FN f, void *data);


#ifdef __cplusplus
}
#endif


#endif


