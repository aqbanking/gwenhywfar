/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_MSGIO_P_H
#define GWEN_MSG_ENDPOINT_MSGIO_P_H

#include <gwenhywfar/endpoint_msgio.h>


typedef struct GWEN_ENDPOINT_MSGIO GWEN_ENDPOINT_MSGIO;
struct GWEN_ENDPOINT_MSGIO {
  GWEN_MSG_ENDPOINT_ADDSOCKETS_FN addSocketsFn;
  GWEN_MSG_ENDPOINT_CHECKSOCKETS_FN checkSocketsFn;

  GWEN_ENDPOINT_MSGIO_GETBYTESNEEDED_FN getBytesNeededFn;
  GWEN_ENDPOINT_MSGIO_SENDMSGSTART_FN sendMsgStartFn;
  GWEN_ENDPOINT_MSGIO_SENDMSGFINISH_FN sendMsgFinishFn;
};




#endif


