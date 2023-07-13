/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_P_H
#define GWEN_MSG_ENDPOINT_P_H


#include <gwenhywfar/endpoint.h>


struct GWEN_MSG_ENDPOINT {
  GWEN_INHERIT_ELEMENT(GWEN_MSG_ENDPOINT)
  GWEN_TREE2_ELEMENT(GWEN_MSG_ENDPOINT)

  char *name;
  int groupId;
  uint32_t flags;
  int state;
  time_t timeOfLastStateChange;

  GWEN_SOCKET *socket;

  GWEN_MSG_LIST *receivedMessageList;
  GWEN_MSG_LIST *sendMessageList;
  GWEN_MSG *currentlyReceivedMsg;

  GWEN_MSG_ENDPOINT_ADDSOCKETS_FN addSocketsFn;
  GWEN_MSG_ENDPOINT_CHECKSOCKETS_FN checkSocketsFn;

  int defaultMessageSize;
};


#endif


