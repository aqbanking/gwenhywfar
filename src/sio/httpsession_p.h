/***************************************************************************
    begin       : Fri Feb 15 2008
    copyright   : (C) 2008-2017 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_HTTP_SESSION_P_H
#define GWEN_HTTP_SESSION_P_H

#include <gwenhywfar/httpsession.h>


struct GWEN_HTTP_SESSION {
  GWEN_INHERIT_ELEMENT(GWEN_HTTP_SESSION);

  char *url;
  char *defaultProtocol;
  int defaultPort;

  GWEN_SYNCIO *syncIo;

  uint32_t flags;

  int httpVMajor;
  int httpVMinor;
  char *httpUserAgent;
  char *httpContentType;

  GWEN_HTTPSESSION_INITSYNCIO_FN initSyncIoFn;

  uint32_t usage;
};


static int GWEN_HttpSession__RecvPacket(GWEN_HTTP_SESSION *sess, GWEN_BUFFER *buf);
static int GWEN_HttpSession__RecvPacketToSio(GWEN_HTTP_SESSION *sess, GWEN_SYNCIO *sio);


static int GWEN_HttpSession_InitSyncIo(GWEN_HTTP_SESSION *sess, GWEN_SYNCIO *sio);



#endif

