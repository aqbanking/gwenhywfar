/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2024 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_REQUEST_P_H
#define GWEN_MSG_REQUEST_P_H

#include "msgio/request.h"


struct GWEN_MSG_REQUEST {
  GWEN_INHERIT_ELEMENT(GWEN_MSG_REQUEST)
  GWEN_TREE2_ELEMENT(GWEN_MSG_REQUEST)

  int requestType;
  GWEN_MSG *requestMsg;         /* msg this request is based on */
  GWEN_MSG_ENDPOINT *endpoint;  /* source/dest endpoint for this request */
  uint32_t requestMsgId;

  GWEN_MSG_LIST *msgList;

  GWEN_TIMESTAMP *createdAt;
  GWEN_TIMESTAMP *expiresAt;

  GWEN_MSG_REQUEST_HANDLERESPONSE_FN handleResponseFn;
  GWEN_MSG_REQUEST_SUBREQUESTFINISHED_FN subRequestFinishedFn;
  GWEN_MSG_REQUEST_ABORT_FN abortFn;

  void *privateData;

  int state;
  int result;
};



#endif


