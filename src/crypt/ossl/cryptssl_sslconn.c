/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sat Jan 17 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "cryptssl_sslconn_p.h"
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <openssl/err.h>
#include <gwenhywfar/waitcallback.h>



int GWEN_SSLConn_IsAvailable(){
  return -1;
}


GWEN_SSL_CONNECTION *GWEN_SSLConn_new(const char *cafile,
                                      const char *capath){
  GWEN_SSL_CONNECTION *conn;

  GWEN_NEW_OBJECT(GWEN_SSL_CONNECTION, conn);
  if (cafile)
    conn->CAfile=strdup(cafile);
  if (capath)
    conn->CAdir=strdup(capath);
  conn->socket=GWEN_Socket_new();
  return conn;
}



void GWEN_SSLConn_free(GWEN_SSL_CONNECTION *conn){
  if (conn) {
    free(conn->CAfile);
    free(conn->CAdir);
    GWEN_Socket_free(conn->socket);
    free(conn);
  }
}



GWEN_ERRORCODE GWEN_SSLConn_Connect(GWEN_SSL_CONNECTION *conn,
                                    const GWEN_INETADDRESS *addr,
                                    int secure,
                                    int timeout){
  GWEN_ERRORCODE err;
  int rv;
  int fd;
  time_t startt;
  int distance;
  int count;
  X509 *cert;

  conn->isSecure=0;
  startt=time(0);

  /* create socket */
  err=GWEN_Socket_Open(conn->socket, GWEN_SocketTypeTCP);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }

  /* connect to server */
  err=GWEN_Socket_Connect_Wait(conn->socket,
                               addr,
                               timeout);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }

  /* set nonblocking again, so that we get a foot into to door of SSL */
  err=GWEN_Socket_SetBlocking(conn->socket, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }

  /* get socket handle (I know, it's ugly, but the function below is
   * not exported to the outside) */
  fd=GWEN_Socket_GetSocketInt(conn->socket);
  if (fd==-1) {
    DBG_ERROR(0, "No socket handle, cannot use this socket with SSL");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_UNSUPPORTED);
  }

  /* socket ready now setup SSL */
  conn->ssl_ctx=SSL_CTX_new(SSLv23_client_method());
  conn->ssl=SSL_new(conn->ssl_ctx);

  /* setup locations of certificates */
  rv=SSL_CTX_load_verify_locations(conn->ssl_ctx,
                                   conn->CAfile,
                                   conn->CAdir);
  if (rv==0) {
    int sslerr;

    sslerr=SSL_get_error(conn->ssl, rv);
    DBG_ERROR(0, "SSL error: %s (%d)",
              GWEN_SSLConn_ErrorString(sslerr),
              sslerr);
    GWEN_Socket_Close(conn->socket);
    SSL_free(conn->ssl);
    conn->ssl=0;
    SSL_CTX_free(conn->ssl_ctx);
    conn->ssl_ctx=0;
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_SSL);
  }

  /* set verification mode */
  if (secure) {
    SSL_CTX_set_verify(conn->ssl_ctx,
                       SSL_VERIFY_PEER,
                       0);
  }
  else {
    SSL_CTX_set_verify(conn->ssl_ctx,
                       SSL_VERIFY_NONE,
                       0);
  }

  /* tell SSL to use our socket */
  rv=SSL_set_fd(conn->ssl, fd);
  if (rv==0) {
    int sslerr;

    sslerr=SSL_get_error(conn->ssl, rv);
    DBG_ERROR(0, "SSL error: %s (%d)",
              GWEN_SSLConn_ErrorString(sslerr),
              sslerr);
    GWEN_Socket_Close(conn->socket);
    SSL_free(conn->ssl);
    conn->ssl=0;
    SSL_CTX_free(conn->ssl_ctx);
    conn->ssl_ctx=0;
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_SSL);
  }

  /* all setup, try to initiate a connection */
  if (timeout==GWEN_SSLCONN_TIMEOUT_NONE)
    distance=GWEN_SSLCONN_TIMEOUT_NONE;
  else if (timeout==-1)
    distance=-1;
  else {
    timeout-=(difftime(time(0), startt));
    if (timeout<0)
      timeout=1;
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance/1000)>timeout)
        distance=timeout/1000;
    if (!distance)
      distance=750;
  }

  for (count=0;;) {
    if (GWEN_WaitCallback(count)==GWEN_WaitCallbackResult_Abort) {
      DBG_ERROR(0, "User aborted via waitcallback");
      GWEN_Socket_Close(conn->socket);
      SSL_free(conn->ssl);
      conn->ssl=0;
      SSL_CTX_free(conn->ssl_ctx);
      conn->ssl_ctx=0;
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                            GWEN_SOCKET_ERROR_ABORTED);
    }
    rv=SSL_connect(conn->ssl);
    if (rv<1) {
      int sslerr;

      sslerr=SSL_get_error(conn->ssl, rv);
      if (sslerr==SSL_ERROR_WANT_READ) {
        err=GWEN_Socket_WaitForRead(conn->socket, distance);

      }
      else if (sslerr==SSL_ERROR_WANT_WRITE) {
        err=GWEN_Socket_WaitForWrite(conn->socket, distance);
      }
      else {
        DBG_ERROR(0, "SSL error: %s (%d)",
                  GWEN_SSLConn_ErrorString(sslerr),
                  sslerr);
        GWEN_Socket_Close(conn->socket);
        SSL_free(conn->ssl);
        conn->ssl=0;
        SSL_CTX_free(conn->ssl_ctx);
        conn->ssl_ctx=0;
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                              GWEN_CRYPT_ERROR_SSL);
      } /* if unhandled error */

      /* check for socket error */
      if (!GWEN_Error_IsOk(err)) {
        if (GWEN_Error_GetType(err)==GWEN_Error_FindType("Socket")) {
          if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
              GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
            DBG_ERROR_ERR(0, err);
            GWEN_Socket_Close(conn->socket);
            SSL_free(conn->ssl);
            conn->ssl=0;
            SSL_CTX_free(conn->ssl_ctx);
            conn->ssl_ctx=0;
            return err;
          }
        } /* if socket error */
        else {
          DBG_ERROR_ERR(0, err);
          GWEN_Socket_Close(conn->socket);
          SSL_free(conn->ssl);
          conn->ssl=0;
          SSL_CTX_free(conn->ssl_ctx);
          conn->ssl_ctx=0;
          return err;
        }
      }
    } /* if SSL error */
    else {
      break;
    }
    if (timeout!=GWEN_SSLCONN_TIMEOUT_FOREVER) {
      if (timeout==GWEN_SSLCONN_TIMEOUT_NONE ||
          difftime(time(0), startt)>timeout) {
        DBG_INFO_ERR(0, err);
        DBG_ERROR(0, "Could not connect within %d seconds, aborting",
                  timeout);
        GWEN_Socket_Close(conn->socket);
        SSL_free(conn->ssl);
        conn->ssl=0;
        SSL_CTX_free(conn->ssl_ctx);
        conn->ssl_ctx=0;
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                              GWEN_SOCKET_ERROR_TIMEOUT);
      }
    }
  } /* for */

  cert=SSL_get_peer_certificate(conn->ssl);
  if (!cert) {
    if (secure) {
      DBG_ERROR(0, "Peer did not send a certificate, abort");
      GWEN_Socket_Close(conn->socket);
      SSL_free(conn->ssl);
      conn->ssl=0;
      SSL_CTX_free(conn->ssl_ctx);
      conn->ssl_ctx=0;
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                            GWEN_CRYPT_ERROR_SSL);
    }
    else {
      DBG_WARN(0, "Peer did not send a certificate");
    }
  }
  else {
    char *certbuf;

    certbuf=X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);

    DBG_NOTICE(0, "Got a certificate: %s",
               certbuf);
    free(certbuf);
    if (SSL_get_verify_result(conn->ssl)!=X509_V_OK) {
      if (secure) {
        DBG_ERROR(0, "Invalid peer certificate, aborting");
        GWEN_Socket_Close(conn->socket);
        SSL_free(conn->ssl);
        conn->ssl=0;
        SSL_CTX_free(conn->ssl_ctx);
        conn->ssl_ctx=0;
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                              GWEN_CRYPT_ERROR_SSL);
      }
      else {
        DBG_WARN(0, "Invalid peer certificate, ignoring");
      }
    }
    else {
      DBG_NOTICE(0, "Peer certificate is valid");
      conn->isSecure=1;
    }
    X509_free(cert);
  }

  DBG_NOTICE(0, "SSL connection established (%s)",
             (conn->isSecure)?"verified":"not verified");
  return 0;
}



GWEN_ERRORCODE GWEN_SSLConn_Disconnect(GWEN_SSL_CONNECTION *conn,
                                       int timeout){
  GWEN_ERRORCODE err;
  int rv;
  time_t startt;
  int distance;
  int count;

  /* try to close connection */
  startt=time(0);
  if (timeout==0)
    distance=0;
  else if (timeout==-1)
    distance=-1;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance/1000)>timeout)
        distance=timeout/1000;
    if (!distance)
      distance=750;
  }

  for (count=0;;) {
    if (GWEN_WaitCallback(count)==GWEN_WaitCallbackResult_Abort) {
      DBG_ERROR(0, "User aborted via waitcallback");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                            GWEN_SOCKET_ERROR_ABORTED);
    }
    rv=SSL_shutdown(conn->ssl);

    if (rv==0) {
      /* connection closed */
      DBG_NOTICE(0, "Connection closed");
      GWEN_Socket_Close(conn->socket);
      SSL_free(conn->ssl);
      conn->ssl=0;
      SSL_CTX_free(conn->ssl_ctx);
      conn->ssl_ctx=0;
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                            GWEN_SOCKET_ERROR_BROKEN_PIPE);
    }
    else if (rv<0) {
      int sslerr;

      sslerr=SSL_get_error(conn->ssl, rv);
      if (sslerr==SSL_ERROR_WANT_READ) {
        err=GWEN_Socket_WaitForRead(conn->socket, distance);
      }
      else if (sslerr==SSL_ERROR_WANT_WRITE) {
        err=GWEN_Socket_WaitForWrite(conn->socket, distance);
      }
      else {
        DBG_ERROR(0, "SSL error: %s (%d)",
                  GWEN_SSLConn_ErrorString(sslerr),
                  sslerr);
        GWEN_Socket_Close(conn->socket);
        SSL_free(conn->ssl);
        conn->ssl=0;
        SSL_CTX_free(conn->ssl_ctx);
        conn->ssl_ctx=0;
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                              GWEN_CRYPT_ERROR_SSL);
      } /* if unhandled error */

      /* check for socket error */
      if (!GWEN_Error_IsOk(err)) {
        if (GWEN_Error_GetType(err)==GWEN_Error_FindType("Socket")) {
          if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
              GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
            DBG_ERROR_ERR(0, err);
            GWEN_Socket_Close(conn->socket);
            SSL_free(conn->ssl);
            conn->ssl=0;
            SSL_CTX_free(conn->ssl_ctx);
            conn->ssl_ctx=0;
            return err;
          }
        } /* if socket error */
        else {
          DBG_ERROR_ERR(0, err);
          GWEN_Socket_Close(conn->socket);
          SSL_free(conn->ssl);
          conn->ssl=0;
          SSL_CTX_free(conn->ssl_ctx);
          conn->ssl_ctx=0;
          return err;
        }
      }
    } /* if SSL error */
    else {
      break;
    }
    if (timeout!=GWEN_SSLCONN_TIMEOUT_FOREVER) {
      if (timeout==GWEN_SSLCONN_TIMEOUT_NONE ||
          difftime(time(0), startt)>timeout) {
        DBG_INFO_ERR(0, err);
        DBG_ERROR(0, "Could not disconnect within %d seconds, aborting",
                  timeout);
        GWEN_Socket_Close(conn->socket);
        SSL_free(conn->ssl);
        conn->ssl=0;
        SSL_CTX_free(conn->ssl_ctx);
        conn->ssl_ctx=0;
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                              GWEN_SOCKET_ERROR_TIMEOUT);
      }
    }
  } /* for */

  GWEN_Socket_Close(conn->socket);
  SSL_free(conn->ssl);
  conn->ssl=0;
  SSL_CTX_free(conn->ssl_ctx);
  conn->ssl_ctx=0;
  return 0;
}



GWEN_ERRORCODE GWEN_SSLConn_Accept(GWEN_SSL_CONNECTION *conn,
                                   GWEN_INETADDRESS **addr,
                                   int secure,
                                   int timeout){
  DBG_ERROR(0, "Not yet supported");
  return GWEN_Error_new(0,
                        GWEN_ERROR_SEVERITY_ERR,
                        GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                        GWEN_SOCKET_ERROR_UNSUPPORTED);
}



GWEN_ERRORCODE GWEN_SSLConn__ReadOrWrite(GWEN_SSL_CONNECTION *conn,
                                         const char *outbuffer,
                                         char *inbuffer,
                                         int *bsize,
                                         int timeout,
                                         int reading){
  GWEN_ERRORCODE err;
  int rv;
  time_t startt;
  int distance;
  int count;

  /* all setup, try to initiate a connection */
  startt=time(0);
  if (timeout==0)
    distance=0;
  else if (timeout==-1)
    distance=-1;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance/1000)>timeout)
        distance=timeout/1000;
    if (!distance)
      distance=750;
  }

  for (count=0;;) {
    if (GWEN_WaitCallback(count)==GWEN_WaitCallbackResult_Abort) {
      DBG_ERROR(0, "User aborted via waitcallback");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                            GWEN_SOCKET_ERROR_ABORTED);
    }
    if (reading)
      rv=SSL_read(conn->ssl, inbuffer, *bsize);
    else
      rv=SSL_write(conn->ssl, outbuffer, *bsize);

    if (rv==0) {
      /* connection closed */
      DBG_NOTICE(0, "Connection closed");
      GWEN_Socket_Close(conn->socket);
      SSL_free(conn->ssl);
      conn->ssl=0;
      SSL_CTX_free(conn->ssl_ctx);
      conn->ssl_ctx=0;
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                            GWEN_SOCKET_ERROR_BROKEN_PIPE);
    }
    else if (rv<0) {
      int sslerr;

      sslerr=SSL_get_error(conn->ssl, rv);
      if (sslerr==SSL_ERROR_WANT_READ) {
        err=GWEN_Socket_WaitForRead(conn->socket, distance);
      }
      else if (sslerr==SSL_ERROR_WANT_WRITE) {
        err=GWEN_Socket_WaitForWrite(conn->socket, distance);
      }
      else {
        DBG_ERROR(0, "SSL error: %s (%d)",
                  GWEN_SSLConn_ErrorString(sslerr),
                  sslerr);
        GWEN_Socket_Close(conn->socket);
        SSL_free(conn->ssl);
        conn->ssl=0;
        SSL_CTX_free(conn->ssl_ctx);
        conn->ssl_ctx=0;
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                              GWEN_CRYPT_ERROR_SSL);
      } /* if unhandled error */

      /* check for socket error */
      if (!GWEN_Error_IsOk(err)) {
        if (GWEN_Error_GetType(err)==GWEN_Error_FindType("Socket")) {
          if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
              GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
            DBG_ERROR_ERR(0, err);
            GWEN_Socket_Close(conn->socket);
            SSL_free(conn->ssl);
            conn->ssl=0;
            SSL_CTX_free(conn->ssl_ctx);
            conn->ssl_ctx=0;
            return err;
          }
        } /* if socket error */
        else {
          DBG_ERROR_ERR(0, err);
          GWEN_Socket_Close(conn->socket);
          SSL_free(conn->ssl);
          conn->ssl=0;
          SSL_CTX_free(conn->ssl_ctx);
          conn->ssl_ctx=0;
          return err;
        }
      }
    } /* if SSL error */
    else {
      /* set number of bytes written */
      *bsize=rv;
      break;
    }
    if (timeout==GWEN_SSLCONN_TIMEOUT_TRYONLY) {
      DBG_INFO(0, "Could not %s within %d seconds, aborting",
               reading?"read":"write",
               timeout);
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_WARN,
                            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                            GWEN_SOCKET_ERROR_TIMEOUT);
    }
    else if (timeout!=GWEN_SSLCONN_TIMEOUT_FOREVER) {
      if (timeout==GWEN_SSLCONN_TIMEOUT_NONE ||
          difftime(time(0), startt)>timeout) {
        DBG_INFO_ERR(0, err);
        DBG_ERROR(0, "Could not %s within %d seconds, aborting",
                  reading?"read":"write",
                  timeout);
        GWEN_Socket_Close(conn->socket);
        SSL_free(conn->ssl);
        conn->ssl=0;
        SSL_CTX_free(conn->ssl_ctx);
        conn->ssl_ctx=0;
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                              GWEN_SOCKET_ERROR_TIMEOUT);
      }
    }
  } /* for */

  return 0;
}



GWEN_ERRORCODE GWEN_SSLConn_Read(GWEN_SSL_CONNECTION *conn,
                                 char *buffer,
                                 int *bsize,
                                 int timeout){
  GWEN_ERRORCODE err;

  err=GWEN_SSLConn__ReadOrWrite(conn, 0, buffer, bsize, timeout, 1);
  /* check for socket error */
  if (!GWEN_Error_IsOk(err)) {
    if (GWEN_Error_GetType(err)==GWEN_Error_FindType("Socket")) {
      if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
          GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
        DBG_INFO_ERR(0, err);
        return err;
      }
    } /* if socket error */
    else {
      DBG_ERROR_ERR(0, err);
      return err;
    }
  }

  return 0;
}



GWEN_ERRORCODE GWEN_SSLConn_Write(GWEN_SSL_CONNECTION *conn,
                                  const char *buffer,
                                  int *bsize,
                                  int timeout){
  GWEN_ERRORCODE err;

  err=GWEN_SSLConn__ReadOrWrite(conn, buffer, 0, bsize, timeout, 0);
  /* check for socket error */
  if (!GWEN_Error_IsOk(err)) {
    if (GWEN_Error_GetType(err)==GWEN_Error_FindType("Socket")) {
      if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
          GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
        DBG_INFO_ERR(0, err);
        return err;
      }
    } /* if socket error */
    else {
      DBG_ERROR_ERR(0, err);
      return err;
    }
  }

  return 0;
}




const char *GWEN_SSLConn_ErrorString(unsigned int e) {
  const char *s;

  switch(e) {
  case SSL_ERROR_NONE:             s="SSL: None"; break;
#ifdef SSL_ERROR_ZERO_RETURN
  case SSL_ERROR_ZERO_RETURN:      s="SSL: connection closed"; break;
#endif
  case SSL_ERROR_WANT_READ:        s="SSL: Want to read"; break;
  case SSL_ERROR_WANT_WRITE:       s="SSL: Want to write"; break;
#ifdef SSL_ERROR_WANT_CONNECT
  case SSL_ERROR_WANT_CONNECT:     s="SSL: Want to connect"; break;
#endif
#ifdef SSL_ERROR_WANT_ACCEPT
  case SSL_ERROR_WANT_ACCEPT:      s="SSL: Want to accept"; break;
#endif
#ifdef SSL_ERROR_WANT_X509_LOOKUP
  case SSL_ERROR_WANT_X509_LOOKUP: s="SSL: Want to lookup certificate"; break;
#endif
  case SSL_ERROR_SYSCALL:          s=strerror(errno); break;
  case SSL_ERROR_SSL:              s="SSL: protocol error"; break;
  default:                         s="SSL: Unknown error"; break;
  } /* switch */
  return s;
}


