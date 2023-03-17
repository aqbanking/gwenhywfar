/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_MGR_H
#define GWEN_MSG_ENDPOINT_MGR_H


#include <gwenhywfar/error.h>
#include <gwenhywfar/inherit.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_MSG_ENDPOINT_MGR GWEN_MSG_ENDPOINT_MGR;
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_MSG_ENDPOINT_MGR, GWENHYWFAR_API)


#ifdef __cplusplus
}
#endif


#include <gwenhywfar/endpoint.h>


#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API GWEN_MSG_ENDPOINT_MGR *GWEN_MsgEndpointMgr_new(void);
GWENHYWFAR_API void GWEN_MsgEndpointMgr_free(GWEN_MSG_ENDPOINT_MGR *emgr);

GWENHYWFAR_API GWEN_MSG_ENDPOINT_LIST *GWEN_MsgEndpointMgr_GetEndpointList(const GWEN_MSG_ENDPOINT_MGR *emgr);

GWENHYWFAR_API void GWEN_MsgEndpointMgr_AddEndpoint(GWEN_MSG_ENDPOINT_MGR *emgr, GWEN_MSG_ENDPOINT *ep);
GWENHYWFAR_API void GWEN_MsgEndpointMgr_DelEndpoint(GWEN_MSG_ENDPOINT_MGR *emgr, GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API int GWEN_MsgEndpointMgr_IoLoopOnce(GWEN_MSG_ENDPOINT_MGR *emgr);
GWENHYWFAR_API void GWEN_MsgEndpointMgr_RunAllEndpoints(GWEN_MSG_ENDPOINT_MGR *emgr);



#ifdef __cplusplus
}
#endif


#endif


