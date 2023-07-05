/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT2_TCPD_P_H
#define GWEN_MSG_ENDPOINT2_TCPD_P_H

#include <gwenhywfar/endpoint2_tcpd.h>


typedef struct GWEN_ENDPOINT2_TCPD GWEN_ENDPOINT2_TCPD;
struct GWEN_ENDPOINT2_TCPD {
  char *host;
  int port;
  GWEN_ENDPOINT2_TCPD_ACCEPT_FN acceptFn;
  void *acceptData;
};




#endif


