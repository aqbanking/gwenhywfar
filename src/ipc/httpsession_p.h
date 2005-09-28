/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWEN_HTTP_SESSION_P_H
#define GWEN_HTTP_SESSION_P_H


#include <gwenhywfar/httpsession.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>


struct GWEN_HTTP_SESSION {
  GWEN_INHERIT_ELEMENT(GWEN_HTTP_SESSION)
  GWEN_LIST_ELEMENT(GWEN_HTTP_SESSION)

  char *server;
  int port;

  int pmajor;
  int pminor;

  char *trustedCertDir;
  char *newTrustedCertDir;
  char *certFile;

  int connectTimeout;

  GWEN_DB_NODE *dbHeader;

  GWEN_TYPE_UINT32 flags;
  unsigned int maxSize;

  int lastStatusCode;
  char *lastStatusMsg;

  GWEN_NETCONNECTION_LIST2 *connections;
};



static void GWEN_HttpSession__SetResult(GWEN_HTTP_SESSION *sess,
                                        int code, const char *txt);


static GWEN_NETCONNECTION*
GWEN_HttpSession__FindConnection(GWEN_HTTP_SESSION *sess,
                                 const char *server,
                                 int port);
static GWEN_NETCONNECTION*
GWEN_HttpSession__CreateConnection(GWEN_HTTP_SESSION *sess,
                                   const char *proto,
                                   const char *server,
				   int port);



#endif /*GWEN_HTTP_SESSION_P_H */
