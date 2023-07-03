/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT2_TCPC_H
#define GWEN_MSG_ENDPOINT2_TCPC_H


#include <gwenhywfar/endpoint2.h>

#include <time.h>



#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API GWEN_MSG_ENDPOINT2 *GWEN_TcpcEndpoint2_new(const char *host, int port,
                                                          GWEN_MSG_ENDPOINT_MGR2 *mgr,
                                                          const char *name, int groupId);



#ifdef __cplusplus
}
#endif


#endif


