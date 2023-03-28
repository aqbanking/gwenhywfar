/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_TCPC_P_H
#define GWEN_MSG_ENDPOINT_TCPC_P_H


#include <gwenhywfar/endpoint_tcpc.h>


typedef struct GWEN_ENDPOINT_TCPC GWEN_ENDPOINT_TCPC;
struct GWEN_ENDPOINT_TCPC {
  char *host;
  int port;
  int state;

  GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN handleWritableFn;
  GWEN_MSG_ENDPOINT_GET_WRITEFD_FN getWriteFdFn;
  GWEN_MSG_ENDPOINT_GET_READFD_FN getReadFdFn;
};



#endif


