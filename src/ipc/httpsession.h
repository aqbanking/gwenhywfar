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

#ifndef GWEN_HTTP_SESSION_H
#define GWEN_HTTP_SESSION_H


#define GWEN_HTTP_SESSION_FLAGS_ALLOW_RECONNECT     0x00000001
#define GWEN_HTTP_SESSION_FLAGS_KEEP_ALIVE          0x00000002
#define GWEN_HTTP_SESSION_FLAGS_ALLOW_REDIRECT      0x00000004
#define GWEN_HTTP_SESSION_FLAGS_USE_HTTPS           0x00000008
#define GWEN_HTTP_SESSION_FLAGS_CLEAR_COOKIES_RECON 0x00000010
#define GWEN_HTTP_SESSION_FLAGS_CLEAR_COOKIES_REDIR 0x00000020


#include <gwenhywfar/httpcookie.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/netconnectionhttp.h>
#include <gwenhywfar/httpurl.h>


typedef struct GWEN_HTTP_SESSION GWEN_HTTP_SESSION;

GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_HTTP_SESSION, GWENHYWFAR_API)
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_HTTP_SESSION, GWEN_HttpSession,
                            GWENHYWFAR_API)


typedef int (*GWEN_HTTPSESSION_ASKFOLLOW_FN)(GWEN_HTTP_SESSION *sess,
                                             const char *oldLocation,
                                             const char *newLocation);

typedef int (*GWEN_HTTPSESSION_GETAUTH_FN)(GWEN_HTTP_SESSION *sess,
                                           const GWEN_HTTP_URL *url,
                                           const char *authScheme,
                                           const char *realm,
                                           char *buffer,
                                           unsigned int size,
                                           int forceAsk);



GWEN_HTTP_SESSION *GWEN_HttpSession_new(const char *addr,
					int port,
					GWEN_TYPE_UINT32 flags,
					int pmajor,
					int pminor);

void GWEN_HttpSession_free(GWEN_HTTP_SESSION *sess);


void GWEN_HttpSession_SetAskFollowFn(GWEN_HTTP_SESSION *sess,
                                     GWEN_HTTPSESSION_ASKFOLLOW_FN fn);

void GWEN_HttpSession_SetGetAuthFn(GWEN_HTTP_SESSION *sess,
                                   GWEN_HTTPSESSION_GETAUTH_FN fn);

int GWEN_HttpSession_Request(GWEN_HTTP_SESSION *sess,
			     const char *command,
			     const char *arg,
			     const char *body,
			     unsigned int size,
                             GWEN_DB_NODE *dbResultHeader,
			     GWEN_BUFFER *result);

void GWEN_HttpSession_Close(GWEN_HTTP_SESSION *sess);

const char *GWEN_HttpSession_GetLastStatusMsg(const GWEN_HTTP_SESSION *sess);
int GWEN_HttpSession_GetLastStatusCode(const GWEN_HTTP_SESSION *sess);


GWEN_TYPE_UINT32 GWEN_HttpSession_GetFlags(const GWEN_HTTP_SESSION *sess);
void GWEN_HttpSession_SetFlags(GWEN_HTTP_SESSION *sess,
			       GWEN_TYPE_UINT32 flags);
void GWEN_HttpSession_AddFlags(GWEN_HTTP_SESSION *sess,
			       GWEN_TYPE_UINT32 flags);
void GWEN_HttpSession_SubFlags(GWEN_HTTP_SESSION *sess,
			       GWEN_TYPE_UINT32 flags);

GWEN_DB_NODE *GWEN_HttpSession_GetHeaders(const GWEN_HTTP_SESSION *sess);
void GWEN_HttpSession_SetHeaders(GWEN_HTTP_SESSION *sess,
				 GWEN_DB_NODE *db);

#endif /*GWEN_HTTP_SESSION_H */


