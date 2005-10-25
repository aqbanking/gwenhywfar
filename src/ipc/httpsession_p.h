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

  GWEN_HTTPSESSION_ASKFOLLOW_FN askFollowFn;
  GWEN_HTTPSESSION_GETAUTH_FN getAuthFn;

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

static GWEN_NETCONNECTION*
GWEN_HttpSession__GetConnection(GWEN_HTTP_SESSION *sess,
                                const char *proto,
                                const char *server,
                                int port);

static int GWEN_HttpSession__Request(GWEN_HTTP_SESSION *sess,
                                     const GWEN_HTTP_URL *url,
                                     const char *command,
                                     const char *arg,
                                     const char *body,
                                     unsigned int size,
                                     GWEN_DB_NODE *dbResultHeader,
                                     GWEN_BUFFER *result);

static int GWEN_HttpSession_AskFollow(GWEN_HTTP_SESSION *sess,
                                      const char *oldLocation,
                                      const char *newLocation);

static int GWEN_HttpSession__Handle_AuthReq(GWEN_HTTP_SESSION *sess,
                                            const GWEN_HTTP_URL *url,
                                            const char *arg,
                                            GWEN_DB_NODE *dbResultHeader,
                                            int forceAsk);


#endif /*GWEN_HTTP_SESSION_P_H */
