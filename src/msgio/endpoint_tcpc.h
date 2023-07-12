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

#include <time.h>



#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_TcpcEndpoint_new(const char *host, int port, const char *name, int groupId);

GWENHYWFAR_API int GWEN_TcpcEndpoint_StartConnect(GWEN_MSG_ENDPOINT *ep);


#ifdef __cplusplus
}
#endif


#endif


