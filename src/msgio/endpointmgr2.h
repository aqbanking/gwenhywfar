/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_MGR2_H
#define GWEN_MSG_ENDPOINT_MGR2_H


#include <gwenhywfar/error.h>
#include <gwenhywfar/inherit.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_MSG_ENDPOINT_MGR2 GWEN_MSG_ENDPOINT_MGR2;
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_MSG_ENDPOINT_MGR2, GWENHYWFAR_API)


#ifdef __cplusplus
}
#endif


#include <gwenhywfar/endpoint.h>


#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API GWEN_MSG_ENDPOINT_MGR2 *GWEN_MsgEndpointMgr2_new(void);
GWENHYWFAR_API void GWEN_MsgEndpointMgr2_free(GWEN_MSG_ENDPOINT_MGR2 *emgr);

GWENHYWFAR_API GWEN_MSG_ENDPOINT2_LIST *GWEN_MsgEndpointMgr2_GetEndpointList(const GWEN_MSG_ENDPOINT_MGR2 *emgr);

GWENHYWFAR_API void GWEN_MsgEndpointMgr2_AddEndpoint(GWEN_MSG_ENDPOINT_MGR2 *emgr, GWEN_MSG_ENDPOINT2 *ep);
GWENHYWFAR_API void GWEN_MsgEndpointMgr2_DelEndpoint(GWEN_MSG_ENDPOINT_MGR2 *emgr, GWEN_MSG_ENDPOINT2 *ep);

GWENHYWFAR_API int GWEN_MsgEndpointMgr2_IoLoopOnce(GWEN_MSG_ENDPOINT_MGR2 *emgr);
GWENHYWFAR_API void GWEN_MsgEndpointMgr2_RunAllEndpoints(GWEN_MSG_ENDPOINT_MGR2 *emgr);



#ifdef __cplusplus
}
#endif


#endif


