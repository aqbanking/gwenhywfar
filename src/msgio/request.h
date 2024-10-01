/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2024 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_REQUEST_H
#define GWEN_MSG_REQUEST_H


#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/msg.h>
#include <gwenhywfar/endpoint.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/tree2.h>
#include <gwenhywfar/timestamp.h>


#define GWEN_MSG_REQUEST_RESULT_NOT_HANDLED  0
#define GWEN_MSG_REQUEST_RESULT_HANDLED      1

#define GWEN_MSG_REQUEST_REASON_DONE         0
#define GWEN_MSG_REQUEST_REASON_ABORTED      1

#define GWEN_MSG_REQUEST_STATE_OPEN          0
#define GWEN_MSG_REQUEST_STATE_DONE          1



typedef struct GWEN_MSG_REQUEST GWEN_MSG_REQUEST;

GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_MSG_REQUEST, GWENHYWFAR_API)
GWEN_TREE2_FUNCTION_LIB_DEFS(GWEN_MSG_REQUEST, GWEN_MsgRequest, GWENHYWFAR_API)

typedef int (*GWEN_MSG_REQUEST_HANDLERESPONSE_FN)(GWEN_MSG_REQUEST *rq, GWEN_MSG *msg);
typedef void (*GWEN_MSG_REQUEST_SUBREQUESTFINISHED_FN)(GWEN_MSG_REQUEST *rq, GWEN_MSG_REQUEST *subRq, int reason);
typedef void (*GWEN_MSG_REQUEST_ABORT_FN)(GWEN_MSG_REQUEST *rq);


GWENHYWFAR_API GWEN_MSG_REQUEST *GWEN_MsgRequest_new();
GWENHYWFAR_API void GWEN_MsgRequest_free(GWEN_MSG_REQUEST *rq);

GWENHYWFAR_API int GWEN_MsgRequest_GetRequestType(const GWEN_MSG_REQUEST *rq);
GWENHYWFAR_API void GWEN_MsgRequest_SetRequestType(GWEN_MSG_REQUEST *rq, int t);

GWENHYWFAR_API GWEN_MSG *GWEN_MsgRequest_GetRequestMsg(const GWEN_MSG_REQUEST *rq);
GWENHYWFAR_API void GWEN_MsgRequest_SetRequestMsg(GWEN_MSG_REQUEST *rq, GWEN_MSG *msg);

GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_MsgRequest_GetEndpoint(const GWEN_MSG_REQUEST *rq);
GWENHYWFAR_API void GWEN_MsgRequest_SetEndpoint(GWEN_MSG_REQUEST *rq, GWEN_MSG_ENDPOINT *ep);

GWENHYWFAR_API uint32_t GWEN_MsgRequest_GetRequestMsgId(const GWEN_MSG_REQUEST *rq);
GWENHYWFAR_API void GWEN_MsgRequest_SetRequestMsgId(GWEN_MSG_REQUEST *rq, uint32_t id);

GWENHYWFAR_API GWEN_MSG_LIST *GWEN_MsgRequest_GetMsgList(const GWEN_MSG_REQUEST *rq);
GWENHYWFAR_API GWEN_MSG *GWEN_MsgRequest_GetFirstMsgFromList(const GWEN_MSG_REQUEST *rq);
GWENHYWFAR_API void GWEN_MsgRequest_AddMsgToList(GWEN_MSG_REQUEST *rq, GWEN_MSG *msg);

GWENHYWFAR_API const GWEN_TIMESTAMP *GWEN_MsgRequest_GetCreatedAt(const GWEN_MSG_REQUEST *rq);
GWENHYWFAR_API void GWEN_MsgRequest_SetCreatedAt(GWEN_MSG_REQUEST *rq, GWEN_TIMESTAMP *ts);

GWENHYWFAR_API const GWEN_TIMESTAMP *GWEN_MsgRequest_GetExpiresAt(const GWEN_MSG_REQUEST *rq);
GWENHYWFAR_API void GWEN_MsgRequest_SetExpiresAt(GWEN_MSG_REQUEST *rq, GWEN_TIMESTAMP *ts);

GWENHYWFAR_API void GWEN_MsgRequest_SetTimestamps(GWEN_MSG_REQUEST *rq, int expiresInSecs);

GWENHYWFAR_API int GWEN_MsgRequest_GetResult(const GWEN_MSG_REQUEST *rq);
GWENHYWFAR_API void GWEN_MsgRequest_SetResult(GWEN_MSG_REQUEST *rq, int result);

GWENHYWFAR_API int GWEN_MsgRequest_GetState(const GWEN_MSG_REQUEST *rq);
GWENHYWFAR_API void GWEN_MsgRequest_SetState(GWEN_MSG_REQUEST *rq, int i);


GWENHYWFAR_API GWEN_MSG_REQUEST *GWEN_MsgRequest_Tree2_FindByEndpointAndMsgId(GWEN_MSG_REQUEST *rootRq,
                                                                              GWEN_MSG_ENDPOINT *ep,
                                                                              uint32_t refMsgId);


GWENHYWFAR_API int GWEN_MsgRequest_HandleResponse(GWEN_MSG_REQUEST *rq, GWEN_MSG *msg);
GWENHYWFAR_API void GWEN_MsgRequest_SubRequestFinished(GWEN_MSG_REQUEST *rq, GWEN_MSG_REQUEST *subRq, int reason);
GWENHYWFAR_API void GWEN_MsgRequest_Abort(GWEN_MSG_REQUEST *rq);



GWENHYWFAR_API void *GWEN_MsgRequest_GetPrivateData(const GWEN_MSG_REQUEST *rq);
GWENHYWFAR_API void GWEN_MsgRequest_SetPrivateData(GWEN_MSG_REQUEST *rq, void *p);

GWENHYWFAR_API GWEN_MSG_REQUEST_HANDLERESPONSE_FN GWEN_MsgRequest_SetHandleResponseFn(GWEN_MSG_REQUEST *rq,
                                                                                      GWEN_MSG_REQUEST_HANDLERESPONSE_FN fn);

GWENHYWFAR_API GWEN_MSG_REQUEST_SUBREQUESTFINISHED_FN GWEN_MsgRequest_SetSubRequestFinishedFn(GWEN_MSG_REQUEST *rq,
                                                                                              GWEN_MSG_REQUEST_SUBREQUESTFINISHED_FN f);
GWENHYWFAR_API GWEN_MSG_REQUEST_ABORT_FN GWEN_MsgRequest_SetAbortFn(GWEN_MSG_REQUEST *rq, GWEN_MSG_REQUEST_ABORT_FN f);



#endif


