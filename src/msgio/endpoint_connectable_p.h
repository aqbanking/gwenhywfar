/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_CONNECTABLE_P_H
#define GWEN_MSG_ENDPOINT_CONNECTABLE_P_H


#include <gwenhywfar/endpoint_connectable.h>



typedef struct GWEN_CONN_ENDPOINT GWEN_CONN_ENDPOINT;
struct GWEN_CONN_ENDPOINT {
  int state;
  time_t timeOfLastStateChange;
  int reconnectWaitTime;
  GWEN_CONN_ENDPOINT_CONNECT_FN connectFn;
  GWEN_CONN_ENDPOINT_DISCONNECT_FN disconnectFn;

  GWEN_MSG_ENDPOINT_GET_READFD_FN getReadFdFn;
  GWEN_MSG_ENDPOINT_GET_WRITEFD_FN getWriteFdFn;
  GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN handleReadableFn;
  GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN handleWritableFn;
  GWEN_MSG_ENDPOINT_RUN_FN runFn;
};



#endif


