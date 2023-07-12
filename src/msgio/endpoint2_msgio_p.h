/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT2_MSGIO_P_H
#define GWEN_MSG_ENDPOINT2_MSGIO_P_H

#include <gwenhywfar/endpoint2_msgio.h>


typedef struct GWEN_ENDPOINT2_MSGIO GWEN_ENDPOINT2_MSGIO;
struct GWEN_ENDPOINT2_MSGIO {
  GWEN_MSG_ENDPOINT2_ADDSOCKETS_FN addSocketsFn;
  GWEN_MSG_ENDPOINT2_CHECKSOCKETS_FN checkSocketsFn;

  GWEN_ENDPOINT2_MSGIO_GETBYTESNEEDED_FN getBytesNeededFn;
  GWEN_ENDPOINT2_MSGIO_SENDMSGSTART_FN sendMsgStartFn;
  GWEN_ENDPOINT2_MSGIO_SENDMSGFINISH_FN sendMsgFinishFn;

  time_t timeOfLastIo;
};




#endif


