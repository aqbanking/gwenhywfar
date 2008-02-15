/***************************************************************************
 $RCSfile$
                             -------------------
    begin       : Fri Feb 15 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_HTTP_SESSION_P_H
#define GWEN_HTTP_SESSION_P_H

#include <gwenhywfar/httpsession.h>


struct GWEN_HTTP_SESSION {
  GWEN_INHERIT_ELEMENT(GWEN_HTTP_SESSION);

  GWEN_URL *url;
  GWEN_IO_LAYER *ioLayer;
  uint32_t guiid;

  uint32_t flags;

  int httpVMajor;
  int httpVMinor;
  char *httpUserAgent;
  char *httpContentType;

  uint32_t usage;
};


static int GWEN_HttpSession__SetAddress(GWEN_HTTP_SESSION *sess,
					GWEN_INETADDRESS *addr,
					const char *peerAddr);

static int GWEN_HttpSession__RecvPacket(GWEN_HTTP_SESSION *sess,
					GWEN_BUFFER *buf, int timeout);



#endif

