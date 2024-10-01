/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2024 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*#define DISABLE_DEBUGLOG*/


#include "msgio/request_p.h"

#include <gwenhywfar/error.h>
#include <gwenhywfar/misc.h>



GWEN_INHERIT_FUNCTIONS(GWEN_MSG_REQUEST)
GWEN_TREE2_FUNCTIONS(GWEN_MSG_REQUEST, GWEN_MsgRequest)




GWEN_MSG_REQUEST *GWEN_MsgRequest_new()
{
  GWEN_MSG_REQUEST *rq;

  GWEN_NEW_OBJECT(GWEN_MSG_REQUEST, rq);
  GWEN_INHERIT_INIT(GWEN_MSG_REQUEST, rq);
  GWEN_TREE2_INIT(GWEN_MSG_REQUEST, rq, GWEN_MsgRequest);

  return rq;
}



void GWEN_MsgRequest_free(GWEN_MSG_REQUEST *rq)
{
  if (rq) {
    GWEN_TREE2_FINI(GWEN_MSG_REQUEST, rq, GWEN_MsgRequest);
    GWEN_INHERIT_FINI(GWEN_MSG_REQUEST, rq);

    GWEN_Timestamp_free(rq->expiresAt);
    GWEN_Timestamp_free(rq->createdAt);
    GWEN_Msg_free(rq->requestMsg);
    GWEN_Msg_List_free(rq->msgList);
    GWEN_MsgEndpoint_free(rq->endpoint);

    GWEN_FREE_OBJECT(rq);
  }
}



int GWEN_MsgRequest_GetRequestType(const GWEN_MSG_REQUEST *rq)
{
  return rq?rq->requestType:0;
}



void GWEN_MsgRequest_SetRequestType(GWEN_MSG_REQUEST *rq, int t)
{
  if (rq)
    rq->requestType=t;
}



GWEN_MSG *GWEN_MsgRequest_GetRequestMsg(const GWEN_MSG_REQUEST *rq)
{
  return rq?rq->requestMsg:NULL;
}



void GWEN_MsgRequest_SetRequestMsg(GWEN_MSG_REQUEST *rq, GWEN_MSG *msg)
{
  if (rq) {
    GWEN_Msg_free(rq->requestMsg);
    rq->requestMsg=msg;
  }
}



GWEN_MSG_ENDPOINT *GWEN_MsgRequest_GetEndpoint(const GWEN_MSG_REQUEST *rq)
{
  return rq?rq->endpoint:NULL;
}



void GWEN_MsgRequest_SetEndpoint(GWEN_MSG_REQUEST *rq, GWEN_MSG_ENDPOINT *ep)
{
  if (rq) {
    if (ep)
      GWEN_MsgEndpoint_Attach(ep);
    if (rq->endpoint)
      GWEN_MsgEndpoint_free(ep);
    rq->endpoint=ep;
  }
}



uint32_t GWEN_MsgRequest_GetRequestMsgId(const GWEN_MSG_REQUEST *rq)
{
  return rq?rq->requestMsgId:0;
}



void GWEN_MsgRequest_SetRequestMsgId(GWEN_MSG_REQUEST *rq, uint32_t id)
{
  if (rq)
    rq->requestMsgId=id;
}



GWEN_MSG_LIST *GWEN_MsgRequest_GetMsgList(const GWEN_MSG_REQUEST *rq)
{
  return rq?rq->msgList:NULL;
}



GWEN_MSG *GWEN_MsgRequest_GetFirstMsgFromList(const GWEN_MSG_REQUEST *rq)
{
  return (rq && rq->msgList)?GWEN_Msg_List_First(rq->msgList):NULL;
}



void GWEN_MsgRequest_AddMsgToList(GWEN_MSG_REQUEST *rq, GWEN_MSG *msg)
{
  if (rq && msg) {
    if (rq->msgList==NULL)
      rq->msgList=GWEN_Msg_List_new();
    GWEN_Msg_List_Add(msg, rq->msgList);
  }
}



const GWEN_TIMESTAMP *GWEN_MsgRequest_GetCreatedAt(const GWEN_MSG_REQUEST *rq)
{
  return rq?rq->createdAt:NULL;
}



void GWEN_MsgRequest_SetCreatedAt(GWEN_MSG_REQUEST *rq, GWEN_TIMESTAMP *ts)
{
  if (rq) {
    GWEN_Timestamp_free(rq->createdAt);
    rq->createdAt=ts;
  }
}



const GWEN_TIMESTAMP *GWEN_MsgRequest_GetExpiresAt(const GWEN_MSG_REQUEST *rq)
{
  return rq?rq->expiresAt:NULL;
}



void GWEN_MsgRequest_SetExpiresAt(GWEN_MSG_REQUEST *rq, GWEN_TIMESTAMP *ts)
{
  if (rq) {
    GWEN_Timestamp_free(rq->expiresAt);
    rq->expiresAt=ts;
  }
}



void GWEN_MsgRequest_SetTimestamps(GWEN_MSG_REQUEST *rq, int expiresInSecs)
{
  if (rq) {
    GWEN_TIMESTAMP *ts;

    ts=GWEN_Timestamp_NowInLocalTime();
    GWEN_Timestamp_free(rq->createdAt);
    rq->createdAt=GWEN_Timestamp_dup(ts);
    GWEN_Timestamp_AddSeconds(ts, expiresInSecs);
    GWEN_Timestamp_free(rq->expiresAt);
    rq->expiresAt=ts;
  }
}



int GWEN_MsgRequest_HandleResponse(GWEN_MSG_REQUEST *rq, GWEN_MSG *msg)
{
  if (rq && rq->handleResponseFn)
    return (rq->handleResponseFn)(rq, msg);
  return GWEN_MSG_REQUEST_RESULT_NOT_HANDLED;
}



void GWEN_MsgRequest_SubRequestFinished(GWEN_MSG_REQUEST *rq, GWEN_MSG_REQUEST *subRq, int reason)
{
  if (rq && rq->subRequestFinishedFn)
    rq->subRequestFinishedFn(rq, subRq, reason);
}



void GWEN_MsgRequest_Abort(GWEN_MSG_REQUEST *rq)
{
  if (rq && rq->abortFn) {
    rq->abortFn(rq);
    GWEN_MsgRequest_SetState(rq, GWEN_MSG_REQUEST_STATE_DONE);
  }
  else {
    GWEN_MSG_REQUEST *rqParent;

    GWEN_MsgRequest_SetState(rq, GWEN_MSG_REQUEST_STATE_DONE);
    rqParent=GWEN_MsgRequest_Tree2_GetParent(rq);
    if (rqParent)
      GWEN_MsgRequest_SubRequestFinished(rqParent, rq, GWEN_MSG_REQUEST_REASON_ABORTED);
  }
}



GWEN_MSG_REQUEST_HANDLERESPONSE_FN GWEN_MsgRequest_SetHandleResponseFn(GWEN_MSG_REQUEST *rq,
								       GWEN_MSG_REQUEST_HANDLERESPONSE_FN f)
{
  if (rq) {
    GWEN_MSG_REQUEST_HANDLERESPONSE_FN oldFn;

    oldFn=rq->handleResponseFn;
    rq->handleResponseFn=f;
    return oldFn;
  }
  return NULL;
}



GWEN_MSG_REQUEST_SUBREQUESTFINISHED_FN GWEN_MsgRequest_SetSubRequestFinishedFn(GWEN_MSG_REQUEST *rq,
									       GWEN_MSG_REQUEST_SUBREQUESTFINISHED_FN f)
{
  if (rq) {
    GWEN_MSG_REQUEST_SUBREQUESTFINISHED_FN oldFn;

    oldFn=rq->subRequestFinishedFn;
    rq->subRequestFinishedFn=f;
    return oldFn;
  }

  return NULL;
}



GWEN_MSG_REQUEST_ABORT_FN GWEN_MsgRequest_SetAbortFn(GWEN_MSG_REQUEST *rq, GWEN_MSG_REQUEST_ABORT_FN f)
{
  if (rq) {
    GWEN_MSG_REQUEST_ABORT_FN oldFn;

    oldFn=rq->abortFn;
    rq->abortFn=f;
    return oldFn;
  }
  return NULL;
}






void *GWEN_MsgRequest_GetPrivateData(const GWEN_MSG_REQUEST *rq)
{
  return rq?rq->privateData:NULL;
}



void GWEN_MsgRequest_SetPrivateData(GWEN_MSG_REQUEST *rq, void *p)
{
  if (rq)
    rq->privateData=p;
}



int GWEN_MsgRequest_GetResult(const GWEN_MSG_REQUEST *rq)
{
  return rq?rq->result:0;
}



void GWEN_MsgRequest_SetResult(GWEN_MSG_REQUEST *rq, int result)
{
  if (rq)
    rq->result=result;
}



int GWEN_MsgRequest_GetState(const GWEN_MSG_REQUEST *rq)
{
  return rq?rq->state:0;
}



void GWEN_MsgRequest_SetState(GWEN_MSG_REQUEST *rq, int i)
{
  if (rq)
    rq->state=i;
}








GWEN_MSG_REQUEST *GWEN_MsgRequest_Tree2_FindByEndpointAndMsgId(GWEN_MSG_REQUEST *rootRq, GWEN_MSG_ENDPOINT *ep, uint32_t refMsgId)
{
  if (rootRq) {
    GWEN_MSG_REQUEST *rq;

    rq=GWEN_MsgRequest_Tree2_GetFirstChild(rootRq);
    while(rq) {
      if (rq->endpoint==ep && rq->requestMsgId==refMsgId)
	return rq;
      rq=GWEN_MsgRequest_Tree2_GetBelow(rq);
    } /* while */
  }

  return NULL;
}




