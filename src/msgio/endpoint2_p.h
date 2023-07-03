/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT2_P_H
#define GWEN_MSG_ENDPOINT2_P_H


#include <gwenhywfar/endpoint2.h>


struct GWEN_MSG_ENDPOINT2 {
  GWEN_INHERIT_ELEMENT(GWEN_MSG_ENDPOINT2)
  GWEN_LIST_ELEMENT(GWEN_MSG_ENDPOINT2)

  char *name;
  int groupId;
  uint32_t flags;
  int state;
  time_t timeOfLastStateChange;

  GWEN_MSG_ENDPOINT_MGR2 *manager;
  GWEN_SOCKET *socket;

  GWEN_MSG_LIST *receivedMessageList;
  GWEN_MSG_LIST *sendMessageList;
  GWEN_MSG *currentlyReceivedMsg;

  GWEN_MSG_ENDPOINT2_ADDSOCKETS_FN addSocketsFn;
  GWEN_MSG_ENDPOINT2_CHECKSOCKETS_FN checkSocketsFn;
  GWEN_MSG_ENDPOINT2_RUN_FN runFn;
};


#endif


