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
#include <gwenhywfar/net.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/nettransportsock.h>
#include <gwenhywfar/nettransportssl.h>
#include <gwenhywfar/text.h>

#include <ctype.h>


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
  sess->dbHeader=GWEN_DB_Group_new("header");

  return sess;
}



void GWEN_HttpSession_free(GWEN_HTTP_SESSION *sess) {
  if (sess) {
    GWEN_LIST_FINI(GWEN_HTTP_SESSION, sess);
    GWEN_INHERIT_FINI(GWEN_HTTP_SESSION, sess);

    GWEN_DB_Group_free(sess->dbHeader);
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
  /* add connection to pool for GWEN_Net_HeartBeat() */
  GWEN_Net_AddConnectionToPool(conn);
  return conn;
}



GWEN_NETCONNECTION*
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
                              const GWEN_HTTP_URL *url,
                              const char *command,
                              const char *arg,
                              const char *body,
                              unsigned int size,
                              GWEN_DB_NODE *dbResultHeader,
                              GWEN_BUFFER *result) {
  GWEN_NETCONNECTION *conn;

  conn=GWEN_HttpSession__GetConnection(sess,
				       GWEN_HttpUrl_GetProtocol(url),
				       GWEN_HttpUrl_GetServer(url),
				       GWEN_HttpUrl_GetPort(url));
  if (!conn) {
    DBG_ERROR(0, "No connection.");
    return -1;
  }
  else {
    int rv;
    GWEN_BUFFER *cmdBuf;

    GWEN_NetConnectionHTTP_SetHeaders(conn,
                                      GWEN_DB_Group_dup(sess->dbHeader));
    cmdBuf=GWEN_Buffer_new(0, 64, 0, 1);
    GWEN_HttpUrl_toCommandString(url, cmdBuf);
    DBG_ERROR(0, "Command: %s %s",
              command, GWEN_Buffer_GetStart(cmdBuf));
    GWEN_DB_Dump(GWEN_NetConnectionHTTP_GetHeaders(conn), stderr, 2);
    rv=GWEN_NetConnectionHTTP_Request(conn,
                                      command,
                                      GWEN_Buffer_GetStart(cmdBuf),
                                      body, size,
                                      dbResultHeader,
                                      result);
    if (rv) {
      GWEN_HttpSession__SetResult(sess, rv,
                                  "Error executing request");
      return rv;
    }
    else {
      DBG_ERROR(0, "Result:");
      GWEN_DB_Dump(dbResultHeader, stderr, 2);
      return 0;
    }
  }
  return 0;
}



int GWEN_HttpSession_AskFollow(GWEN_HTTP_SESSION *sess,
                               const char *oldLocation,
                               const char *newLocation) {
  assert(sess);
  if (sess->askFollowFn)
    return sess->askFollowFn(sess, oldLocation, newLocation);
  return -1;
}



void GWEN_HttpSession_SetAskFollowFn(GWEN_HTTP_SESSION *sess,
                                     GWEN_HTTPSESSION_ASKFOLLOW_FN fn) {
  assert(sess);
  sess->askFollowFn=fn;
}



void GWEN_HttpSession_SetGetAuthFn(GWEN_HTTP_SESSION *sess,
                                   GWEN_HTTPSESSION_GETAUTH_FN fn) {
  assert(sess);
  sess->getAuthFn=fn;
}



int GWEN_HttpSession__Handle_AuthReq(GWEN_HTTP_SESSION *sess,
                                     const GWEN_HTTP_URL *url,
                                     const char *arg,
                                     GWEN_DB_NODE *dbResultHeader,
                                     int forceAsk) {
  const char *s;
  char *p;
  char scheme[32];
  char tmp[64];
  char realm[64];
  char authbuffer[256];
  int i;
  int rv;

  if (!sess->getAuthFn) {
    DBG_ERROR(GWEN_LOGDOMAIN, "o getAuth function set");
    abort();
  }

  s=GWEN_DB_GetCharValue(dbResultHeader, "WWW-Authenticate", 0, 0);
  if (!s || !*s) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad server reply (no \"WWW-Authenticate\")");
    GWEN_HttpSession__SetResult(sess,
                                GWEN_ERROR_INVALID,
                                "Bad server reply");
    return GWEN_ERROR_INVALID;
  }

  i=0;
  while(*s && !isspace(*s)) {
    if (i>=sizeof(scheme)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Buffer overflow");
      GWEN_HttpSession__SetResult(sess,
                                  GWEN_ERROR_BUFFER_OVERFLOW,
                                  "Buffer overflow");
      return GWEN_ERROR_BUFFER_OVERFLOW;
    }
    scheme[i++]=*(s++);
  }
  s++;
  scheme[i]=0;

  p=GWEN_Text_GetWord(s, "=", tmp, sizeof(tmp)-1,
                      GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS |
                      GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS |
                      GWEN_TEXT_FLAGS_NEED_DELIMITER |
                      GWEN_TEXT_FLAGS_DEL_QUOTES,
                      &s);
  if (!p) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No realm given in server response");
    GWEN_HttpSession__SetResult(sess,
                                GWEN_ERROR_INVALID,
                                "Bad server reply");
    return GWEN_ERROR_INVALID;
  }
  s++;

  p=GWEN_Text_GetWord(s, "", realm, sizeof(realm)-1,
                      GWEN_TEXT_FLAGS_NULL_IS_DELIMITER |
                      GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS |
                      GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS |
                      GWEN_TEXT_FLAGS_NEED_DELIMITER |
                      GWEN_TEXT_FLAGS_DEL_QUOTES,
                      &s);
  if (!p) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No realm given in server response");
    GWEN_HttpSession__SetResult(sess,
                                GWEN_ERROR_INVALID,
                                "Bad server reply");
    return GWEN_ERROR_INVALID;
  }

  memset(authbuffer, 0, sizeof(authbuffer));

  rv=sess->getAuthFn(sess, url, scheme, realm,
                     authbuffer, sizeof(authbuffer), forceAsk);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, scheme);
    GWEN_Buffer_AppendString(tbuf, " ");
    GWEN_Buffer_AppendString(tbuf, authbuffer);
    GWEN_DB_SetCharValue(sess->dbHeader,
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "Authorization",
                         GWEN_Buffer_GetStart(tbuf));
    memset(authbuffer, 0, sizeof(authbuffer));
    GWEN_Buffer_free(tbuf);
    return 0;
  }
}



int GWEN_HttpSession_Request(GWEN_HTTP_SESSION *sess,
			     const char *command,
			     const char *arg,
			     const char *body,
			     unsigned int size,
                             GWEN_DB_NODE *dbResultHeader,
                             GWEN_BUFFER *result) {
  int loop;
  int didAuth=0;
  GWEN_BUFFER *bufArg;

  bufArg=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(bufArg, arg);

  /* always delete authorization header, no caching here! */
  GWEN_DB_DeleteVar(sess->dbHeader, "Authorization");

  for (loop=0;loop<10; loop++) {
    GWEN_DB_NODE *ldbResultHeader;
    GWEN_HTTP_URL *url;
    int rv;

    url=GWEN_HttpUrl_fromString(arg);
    if (url==0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad URL");
      GWEN_HttpSession__SetResult(sess, GWEN_ERROR_GENERIC,
                                  "Bad URL");
      GWEN_Buffer_free(bufArg);
      return -1;
    }

    ldbResultHeader=GWEN_DB_Group_new("result");
    DBG_ERROR(0, "Requesting this: %s", GWEN_Buffer_GetStart(bufArg));
    GWEN_Buffer_Reset(result);
    rv=GWEN_HttpSession__Request(sess, url, command,
                                 GWEN_Buffer_GetStart(bufArg),
                                 body, size,
                                 ldbResultHeader,
                                 result);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_HttpUrl_free(url);
      GWEN_DB_Group_free(ldbResultHeader);
      GWEN_Buffer_free(bufArg);
      return rv;
    }
  
    if (rv>=200 && rv<300) {
      GWEN_DB_DeleteVar(sess->dbHeader, "Authorization");
      GWEN_DB_AddGroupChildren(dbResultHeader, ldbResultHeader);
      GWEN_HttpUrl_free(url);
      GWEN_DB_Group_free(ldbResultHeader);
      GWEN_Buffer_free(bufArg);
      return rv;
    }
  
    if (rv==302 || rv==307 || rv==303) { /* moved */
      const char *newLocation;
      int ures;
  
      newLocation=GWEN_DB_GetCharValue(ldbResultHeader, "location", 0, 0);
      if (newLocation==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Document moved according to server, "
                  "but no location given");
        GWEN_HttpSession__SetResult(sess, GWEN_ERROR_GENERIC,
                                    "No new location sent by server");
        GWEN_HttpUrl_free(url);
        GWEN_DB_Group_free(ldbResultHeader);
        GWEN_Buffer_free(bufArg);
        return GWEN_ERROR_GENERIC;
      }
  
      ures=GWEN_HttpSession_AskFollow(sess, arg, newLocation);
      if (ures!=1) {
        /* don't follow */
        DBG_INFO(GWEN_LOGDOMAIN, "User doesn't accept new location");
        GWEN_HttpSession__SetResult(sess,
                                    GWEN_ERROR_USER_ABORTED,
                                    "User doesn't accept new location");
        GWEN_HttpUrl_free(url);
        GWEN_DB_Group_free(ldbResultHeader);
        GWEN_Buffer_free(bufArg);
        return GWEN_ERROR_USER_ABORTED;
      }
      GWEN_Buffer_Reset(result);
      GWEN_Buffer_Reset(bufArg);
      GWEN_Buffer_AppendString(bufArg, newLocation);
    }
    else if (rv==401) { /* authorization required */
      int res;

      GWEN_DB_DeleteVar(sess->dbHeader, "Authorization");
      res=GWEN_HttpSession__Handle_AuthReq(sess, url,
                                           GWEN_Buffer_GetStart(bufArg),
                                           ldbResultHeader,
                                           didAuth);
      if (res) {
        GWEN_HttpUrl_free(url);
        GWEN_DB_Group_free(ldbResultHeader);
        GWEN_Buffer_free(bufArg);
        return res;
      }
      didAuth=1;
    }
    else {
      GWEN_HttpUrl_free(url);
      GWEN_DB_Group_free(ldbResultHeader);
      GWEN_Buffer_free(bufArg);
      return rv;
    }
    GWEN_HttpUrl_free(url);
    GWEN_DB_Group_free(ldbResultHeader);
  } /* for */

  DBG_ERROR(GWEN_LOGDOMAIN, "Too many loops");
  GWEN_Buffer_free(bufArg);
  return GWEN_ERROR_ABORTED;
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



void GWEN_HttpSession_Close(GWEN_HTTP_SESSION *sess) {
  GWEN_NETCONNECTION_LIST2_ITERATOR *cit;

  cit=GWEN_NetConnection_List2_First(sess->connections);
  if (cit) {
    GWEN_NETCONNECTION *conn;

    conn=GWEN_NetConnection_List2Iterator_Data(cit);
    assert(conn);
    while(conn) {
      GWEN_NETTRANSPORT_STATUS st;

      st=GWEN_NetConnection_GetStatus(conn);
      if (st!=GWEN_NetTransportStatusPDisconnected &&
          st!=GWEN_NetTransportStatusDisabled) {
        int rv;

        rv=GWEN_NetConnection_Disconnect_Wait(conn, 2);
        if (rv) {
          DBG_WARN(GWEN_LOGDOMAIN, "Error disconnecting");
        }
      }
      GWEN_NetConnection_free(conn);
      conn=GWEN_NetConnection_List2Iterator_Next(cit);
    }
    GWEN_NetConnection_List2Iterator_free(cit);
  }
  GWEN_NetConnection_List2_Clear(sess->connections);
}









