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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "httpsession_p.h"
#include <gwenhywfar/httpcookie.h>
#include <gwenhywfar/httpurl.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/nettransportsock.h>
#include <gwenhywfar/nettransportssl.h>


GWEN_INHERIT_FUNCTIONS(GWEN_HTTP_SESSION)
GWEN_LIST_FUNCTIONS(GWEN_HTTP_SESSION, GWEN_HttpSession)



GWEN_HTTP_SESSION *GWEN_HttpSession_new(const char *addr,
					int port,
					GWEN_TYPE_UINT32 flags,
					int pmajor,
					int pminor) {
  GWEN_HTTP_SESSION *sess;

  assert(addr);
  GWEN_NEW_OBJECT(GWEN_HTTP_SESSION, sess);
  GWEN_INHERIT_INIT(GWEN_HTTP_SESSION, sess);
  GWEN_LIST_INIT(GWEN_HTTP_SESSION, sess);

  sess->server=strdup(addr);
  sess->port=port;
  sess->pmajor=pmajor;
  sess->pminor=pminor;
  sess->flags=flags;

  sess->connections=GWEN_NetConnection_List2_new();

  return sess;
}



void GWEN_HttpSession_free(GWEN_HTTP_SESSION *sess) {
  if (sess) {
    GWEN_LIST_FINI(GWEN_HTTP_SESSION, sess);
    GWEN_INHERIT_FINI(GWEN_HTTP_SESSION, sess);

    GWEN_NetConnection_List2_free(sess->connections);
    free(sess->lastStatusMsg);
    free(sess->server);
    GWEN_FREE_OBJECT(sess);
  }
}



int GWEN_HttpSession__Request(GWEN_HTTP_SESSION *sess,
                              const char *command,
                              const char *arg,
                              const char *body,
                              unsigned int size,
                              GWEN_BUFFER *result) {

}



int GWEN_HttpSession_Request(GWEN_HTTP_SESSION *sess,
			     const char *command,
			     const char *arg,
			     const char *body,
			     unsigned int size,
			     GWEN_BUFFER *result) {
}



const char *GWEN_HttpSession_GetLastStatusMsg(const GWEN_HTTP_SESSION *sess){
  assert(sess);
  return sess->lastStatusMsg;
}



int GWEN_HttpSession_GetLastStatusCode(const GWEN_HTTP_SESSION *sess) {
  assert(sess);
  return sess->lastStatusCode;
}




GWEN_TYPE_UINT32 GWEN_HttpSession_GetFlags(const GWEN_HTTP_SESSION *sess) {
  assert(sess);
  return sess->flags;
}



void GWEN_HttpSession_SetFlags(GWEN_HTTP_SESSION *sess,
			       GWEN_TYPE_UINT32 flags) {
  assert(sess);
  sess->flags=flags;
}



void GWEN_HttpSession_AddFlags(GWEN_HTTP_SESSION *sess,
			       GWEN_TYPE_UINT32 flags) {
  assert(sess);
  sess->flags|=flags;
}



void GWEN_HttpSession_SubFlags(GWEN_HTTP_SESSION *sess,
			       GWEN_TYPE_UINT32 flags) {
  assert(sess);
  sess->flags&=~flags;
}



GWEN_DB_NODE *GWEN_HttpSession_GetHeaders(const GWEN_HTTP_SESSION *sess) {
  assert(sess);
  return sess->dbHeader;
}



void GWEN_HttpSession_SetHeaders(GWEN_HTTP_SESSION *sess,
				 GWEN_DB_NODE *db) {
  assert(sess);
  GWEN_DB_Group_free(sess->dbHeader);
  sess->dbHeader=GWEN_DB_Group_dup(db);
}









