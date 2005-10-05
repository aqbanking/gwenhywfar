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
    free(sess->trustedCertDir);
    free(sess->newTrustedCertDir);
    free(sess->certFile);
    free(sess->lastStatusMsg);
    free(sess->server);
    GWEN_FREE_OBJECT(sess);
  }
}



void GWEN_HttpSession__SetResult(GWEN_HTTP_SESSION *sess,
				 int code, const char *txt) {
  sess->lastStatusCode=code;
  free(sess->lastStatusMsg);
  if (txt) sess->lastStatusMsg=strdup(txt);
  else sess->lastStatusMsg=0;
}



GWEN_NETCONNECTION*
GWEN_HttpSession__FindConnection(GWEN_HTTP_SESSION *sess,
				 const char *server,
				 int port) {
  GWEN_NETCONNECTION_LIST2_ITERATOR *cit;

  cit=GWEN_NetConnection_List2_First(sess->connections);
  if (cit) {
    GWEN_NETCONNECTION *conn;

    conn=GWEN_NetConnection_List2Iterator_Data(cit);
    assert(conn);
    while(conn) {
      if (GWEN_NetConnectionHTTP_GetVirtualPort(conn)==port &&
	  strcasecmp(GWEN_NetConnectionHTTP_GetVirtualServer(conn),
		     server)==0) {
	GWEN_NetConnection_List2Iterator_free(cit);
        return conn;
      }
      conn=GWEN_NetConnection_List2Iterator_Next(cit);
    }
    GWEN_NetConnection_List2Iterator_free(cit);
  }

  return 0;
}



GWEN_NETCONNECTION*
GWEN_HttpSession__CreateConnection(GWEN_HTTP_SESSION *sess,
                                   const char *proto,
				   const char *server,
				   int port) {
  GWEN_NETTRANSPORT *tr;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETCONNECTION *conn;
  GWEN_DB_NODE *dbT;
  GWEN_ERRORCODE err;

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  if (strcasecmp(proto, "https")==0)
    tr=GWEN_NetTransportSSL_new(sk,
				sess->trustedCertDir,
				sess->newTrustedCertDir,
				sess->certFile,
				0, 1, 1);
  else
    tr=GWEN_NetTransportSocket_new(sk, 1);

  if (!tr) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Protocol \"%s\" not supported.", proto);
    GWEN_HttpSession__SetResult(sess,
				GWEN_ERROR_UNSUPPORTED,
				"Protocol not supported");
    return 0;
  }
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  err=GWEN_InetAddr_SetAddress(addr, server);
  if (!GWEN_Error_IsOk(err))
    err=GWEN_InetAddr_SetName(addr, server);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_NetTransport_free(tr);
    GWEN_HttpSession__SetResult(sess, GWEN_Error_GetSimpleCode(err),
				"Could not resolve hostname");
    return 0;
  }
  GWEN_InetAddr_SetPort(addr, port);
  GWEN_NetTransport_SetPeerAddr(tr, addr);
  GWEN_InetAddr_free(addr);

  /* create connection layer */
  conn=GWEN_NetConnectionHTTP_new(tr,
				  1,             /* take over tr */
				  0,             /* libId */
				  sess->pmajor,  /* protocol version */
				  sess->pminor); /* protocol version */

  GWEN_NetConnectionHTTP_SubMode(conn,
				 GWEN_NETCONN_MODE_WAITBEFOREREAD |
				 GWEN_NETCONN_MODE_WAITBEFOREBODYREAD);
  GWEN_NetConnectionHTTP_SetVirtualServer(conn, server);
  GWEN_NetConnectionHTTP_SetVirtualPort(conn, port);

  dbT=GWEN_NetConnectionHTTP_GetHeaders(conn);
  if (sess->flags & GWEN_HTTP_SESSION_FLAGS_KEEP_ALIVE)
    GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "connection",
			 "keep-alive");
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "Accept",
		       "*/*");
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "Host",
		       server);
  return conn;
}


/* FIXME: The "static" is commented out as long as this function
   is unused to prevent an annoying compiler warning. Can and
   should be commented back in again as soon as the function is
   used somewhere */
/* static */ GWEN_NETCONNECTION*
GWEN_HttpSession__GetConnection(GWEN_HTTP_SESSION *sess,
				const char *proto,
				const char *server,
				int port) {
  GWEN_NETCONNECTION *conn;

  conn=GWEN_HttpSession__FindConnection(sess, server, port);
  if (!conn) {
    int rv;

    DBG_ERROR(GWEN_LOGDOMAIN, "Creating connection to %s:%d",
              server, port);
    conn=GWEN_HttpSession__CreateConnection(sess, proto, server, port);
    if (!conn) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return 0;
    }
    rv=GWEN_NetConnection_Connect_Wait(conn, sess->connectTimeout);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_HttpSession__SetResult(sess, rv,
				  "Could not connect to peer");
      GWEN_NetConnection_free(conn);
      return 0;
    }
    GWEN_NetConnection_List2_PushBack(sess->connections, conn);
  }
  else {
    GWEN_NETTRANSPORT_STATUS st;

    /* check whether connection is still active or can be restarted */
    st=GWEN_NetConnection_GetStatus(conn);
    if (st!=GWEN_NetTransportStatusLConnected) {
      if (st==GWEN_NetTransportStatusDisabled) {
	st=GWEN_NetTransportStatusPDisconnected;
	GWEN_NetConnection_SetStatus(conn, st);
      }
      if (st==GWEN_NetTransportStatusPDisconnected) {
	if (sess->flags & GWEN_HTTP_SESSION_FLAGS_ALLOW_RECONNECT) {
	  int rv;

          /* reconnect allowed */
	  rv=GWEN_NetConnection_Connect_Wait(conn, sess->connectTimeout);
	  if (rv) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_HttpSession__SetResult(sess, rv,
					"Could not connect to peer");
            GWEN_NetConnection_List2_Remove(sess->connections, conn);
	    GWEN_NetConnection_free(conn);
	    return 0;
	  }
	}
	else {
	  /* reconnect not allowed */
          DBG_ERROR(GWEN_LOGDOMAIN, "Connection lost");
	  GWEN_HttpSession__SetResult(sess,
				      GWEN_ERROR_BROKEN_PIPE,
				      "Connection to peer lost");
	  return 0;
	}
      }
      else {
	DBG_ERROR(GWEN_LOGDOMAIN, "Connection error (%d)", st);
	GWEN_HttpSession__SetResult(sess,
				    GWEN_ERROR_BROKEN_PIPE,
				    "Bad connection status");
	return 0;
      }
    }
  }

  /* now we have a working connection */
  return conn;
}





int GWEN_HttpSession__Request(GWEN_HTTP_SESSION *sess,
                              const char *command,
                              const char *arg,
                              const char *body,
                              unsigned int size,
                              GWEN_BUFFER *result) {
  return 0; /* Added to surpress gcc4 -O2 warnings. Martin is still at work.*/
}



int GWEN_HttpSession_Request(GWEN_HTTP_SESSION *sess,
			     const char *command,
			     const char *arg,
			     const char *body,
			     unsigned int size,
			     GWEN_BUFFER *result) {
  return 0; /* Added to surpress gcc4 -O2 warnings. Martin is still at work.*/
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









