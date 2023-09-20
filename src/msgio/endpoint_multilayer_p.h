/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_MULTILAYER_P_H
#define GWEN_MSG_ENDPOINT_MULTILAYER_P_H

#include <gwenhywfar/endpoint_multilayer.h>


typedef struct GWEN_ENDPOINT_MULTILAYER GWEN_ENDPOINT_MULTILAYER;
struct GWEN_ENDPOINT_MULTILAYER {
  GWEN_ENDPOINT_MULTILAYER_STARTCONNECT_FN startConnectFn;
  GWEN_ENDPOINT_MULTILAYER_CHECKSOCKETS_FN checkSocketsFn;

  int reconnectTimeInSeconds;
  int connectTimeoutInSeconds;

  int stage;
};




#endif


