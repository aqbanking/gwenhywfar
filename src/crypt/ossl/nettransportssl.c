/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Wed May 05 2004
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


#include "nettransportssl_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inetsocket_l.h>
#include <string.h>
#include <errno.h>


GWEN_INHERIT(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL);


/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORTSSL *GWEN_NetTransportSSLData_new(){
  GWEN_NETTRANSPORTSSL *skd;

  GWEN_NEW_OBJECT(GWEN_NETTRANSPORTSSL, skd);
  return skd;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetTransportSSLData_free(GWEN_NETTRANSPORTSSL *skd){
  if (skd) {
    if (skd->ownSocket)
      GWEN_Socket_free(skd->socket);
    free(skd->CAfile);
    free(skd->CAdir);

    free(skd);
  }
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT *GWEN_NetTransportSSL_new(GWEN_SOCKET *sk,
                                            const char *cafile,
                                            const char *capath,
                                            int secure,
                                            int relinquish){
  GWEN_NETTRANSPORT *tr;
  GWEN_NETTRANSPORTSSL *skd;

  tr=GWEN_NetTransport_new();
  skd=GWEN_NetTransportSSLData_new();
  GWEN_INHERIT_SETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL,
                       tr, skd, GWEN_NetTransportSSL_FreeData);

  skd->socket=sk;
  skd->ownSocket=relinquish;
  if (cafile)
    skd->CAfile=strdup(cafile);
  if (capath)
    skd->CAdir=strdup(capath);
  skd->secure=secure;

  GWEN_NetTransport_SetStartConnectFn(tr,
                                      GWEN_NetTransportSSL_StartConnect);
  GWEN_NetTransport_SetStartAcceptFn(tr,
                                     GWEN_NetTransportSSL_StartAccept);
  GWEN_NetTransport_SetStartDisconnectFn(tr,
                                         GWEN_NetTransportSSL_StartDisconnect);
  GWEN_NetTransport_SetReadFn(tr,
                              GWEN_NetTransportSSL_Read);
  GWEN_NetTransport_SetWriteFn(tr,
                               GWEN_NetTransportSSL_Write);
  GWEN_NetTransport_SetAddSocketsFn(tr,
                                    GWEN_NetTransportSSL_AddSockets);
  GWEN_NetTransport_SetWorkFn(tr,
                              GWEN_NetTransportSSL_Work);
  return tr;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetTransportSSL_FreeData(void *bp, void *p){
  GWEN_NetTransportSSLData_free((GWEN_NETTRANSPORTSSL*)p);
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT_RESULT
GWEN_NetTransportSSL_StartConnect(GWEN_NETTRANSPORT *tr){
  GWEN_NETTRANSPORTSSL *skd;
  GWEN_ERRORCODE err;
  char addrBuffer[128];

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);

  GWEN_InetAddr_GetAddress(GWEN_NetTransport_GetPeerAddr(tr),
                           addrBuffer, sizeof(addrBuffer));

  if (GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusUnconnected &&
      GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusPDisconnected){
    DBG_ERROR(0, "Socket is not unconnected (status \"%s\")",
              GWEN_NetTransport_StatusName(GWEN_NetTransport_GetStatus(tr)));
    return GWEN_NetTransportResultError;
  }

  DBG_INFO(0, "Starting to connect to %s (port %d)",
           addrBuffer,
           GWEN_InetAddr_GetPort(GWEN_NetTransport_GetPeerAddr(tr)));

  /* arm socket code */
  err=GWEN_Socket_Open(skd->socket);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* set nonblocking */
  err=GWEN_Socket_SetBlocking(skd->socket, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* actually start to connect */
  skd->active=1;
  err=GWEN_Socket_Connect(skd->socket, GWEN_NetTransport_GetPeerAddr(tr));
  /* not yet finished or real error ? */
  if (!GWEN_Error_IsOk(err)) {
    if (GWEN_Error_GetType(err)!=GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
        GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_IN_PROGRESS) {
      /* real error, so return that error */
      DBG_ERROR_ERR(0, err);
      return GWEN_NetTransportResultError;
    }

    /* adjust status (physically connecting) */
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPConnecting);
  }
  else {
    /* connection succeeded */
    DBG_INFO(0, "Connection established with %s (port %d)",
             addrBuffer,
             GWEN_InetAddr_GetPort(GWEN_NetTransport_GetPeerAddr(tr)));
    /* adjust status (logically connected) */
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPConnected);
  }
  GWEN_NetTransport_MarkActivity(tr);
  return GWEN_NetTransportResultOk;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT_RESULT
GWEN_NetTransportSSL_StartAccept(GWEN_NETTRANSPORT *tr){
  GWEN_NETTRANSPORTSSL *skd;
  GWEN_ERRORCODE err;
  char addrBuffer[128];

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);
  if (GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusUnconnected) {
    DBG_ERROR(0, "Socket is not unconnected (%d)",
              GWEN_NetTransport_GetStatus(tr));
    return GWEN_NetTransportResultError;
  }

  GWEN_InetAddr_GetAddress(GWEN_NetTransport_GetLocalAddr(tr),
                           addrBuffer, sizeof(addrBuffer));

  DBG_INFO(0, "Starting to listen on %s (port %d)",
           addrBuffer,
           GWEN_InetAddr_GetPort(GWEN_NetTransport_GetLocalAddr(tr)));

  /* arm socket code */
  err=GWEN_Socket_Open(skd->socket);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* set nonblocking */
  err=GWEN_Socket_SetBlocking(skd->socket, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* reuse address */
  err=GWEN_Socket_SetReuseAddress(skd->socket, 1);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* bind socket to local address */
  skd->active=0;
  err=GWEN_Socket_Bind(skd->socket, GWEN_NetTransport_GetLocalAddr(tr));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* start listening */
  err=GWEN_Socket_Listen(skd->socket, 10);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* adjust status (listening) */
  GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusListening);
  GWEN_NetTransport_MarkActivity(tr);

  return GWEN_NetTransportResultOk;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT_RESULT
GWEN_NetTransportSSL_StartDisconnect(GWEN_NETTRANSPORT *tr){
  GWEN_NETTRANSPORTSSL *skd;
  GWEN_NETTRANSPORT_STATUS st;
  int rv;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);

  /* check status */
  st=GWEN_NetTransport_GetStatus(tr);
  if (st==GWEN_NetTransportStatusUnconnected ||
      st==GWEN_NetTransportStatusPDisconnected ||
      st==GWEN_NetTransportStatusDisabled) {
    DBG_ERROR(0,
              "Socket is inactive: %s (%d)",
              GWEN_NetTransport_StatusName(GWEN_NetTransport_GetStatus(tr)),
              GWEN_NetTransport_GetStatus(tr));
    return GWEN_NetTransportResultError;
  }

  rv=SSL_shutdown(skd->ssl);
  if (rv==1 || rv==-1) {
    /* connection closed */
    DBG_NOTICE(0, "Connection closed");
    GWEN_Socket_Close(skd->socket);
    SSL_free(skd->ssl);
    skd->ssl=0;
    SSL_CTX_free(skd->ssl_ctx);
    skd->ssl_ctx=0;
    GWEN_NetTransport_MarkActivity(tr);
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPDisconnected);
    return GWEN_NetTransportResultOk;
  }

  /* adjust status (LDisconnecting) */
  GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusLDisconnecting);
  GWEN_NetTransport_MarkActivity(tr);

  return GWEN_NetTransportResultOk;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT_RESULT
GWEN_NetTransportSSL_Read(GWEN_NETTRANSPORT *tr,
                             char *buffer,
                             int *bsize){
  GWEN_NETTRANSPORTSSL *skd;
  int rv;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);

  /* check status */
  if (GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusLConnected) {
    DBG_ERROR(0, "Socket is not connected (%d)",
              GWEN_NetTransport_GetStatus(tr));
    return GWEN_NetTransportResultError;
  }

  /* try to read */
  rv=SSL_read(skd->ssl, buffer, *bsize);
  if (rv<1) {
    int sslerr;

    sslerr=SSL_get_error(skd->ssl, rv);
    if (sslerr==SSL_ERROR_WANT_READ)
      return GWEN_NetTransportResultWantRead;
    else if (sslerr==SSL_ERROR_WANT_WRITE)
      return GWEN_NetTransportResultWantWrite;
    else {
      DBG_ERROR(0, "SSL error: %s (%d)",
                GWEN_NetTransportSSL_ErrorString(sslerr),
                sslerr);
      GWEN_Socket_Close(skd->socket);
      SSL_free(skd->ssl);
      skd->ssl=0;
      SSL_CTX_free(skd->ssl_ctx);
      skd->ssl_ctx=0;
      GWEN_NetTransport_MarkActivity(tr);
      return GWEN_NetTransportResultError;
    }
  }

  DBG_NOTICE(0, "Read %d bytes", rv);
  *bsize=rv;
  GWEN_NetTransport_MarkActivity(tr);
  return GWEN_NetTransportResultOk;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT_RESULT
GWEN_NetTransportSSL_Write(GWEN_NETTRANSPORT *tr,
                              const char *buffer,
                              int *bsize){
  GWEN_NETTRANSPORTSSL *skd;
  int rv;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);

  /* check status */
  if (GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusLConnected) {
    DBG_ERROR(0, "Socket is not connected (%d)",
              GWEN_NetTransport_GetStatus(tr));
    return GWEN_NetTransportResultError;
  }

  /* try to read */
  rv=SSL_write(skd->ssl, buffer, *bsize);
  if (rv<1) {
    int sslerr;

    sslerr=SSL_get_error(skd->ssl, rv);
    if (sslerr==SSL_ERROR_WANT_READ)
      return GWEN_NetTransportResultWantRead;
    else if (sslerr==SSL_ERROR_WANT_WRITE)
      return GWEN_NetTransportResultWantWrite;
    else {
      DBG_ERROR(0, "SSL error: %s (%d)",
                GWEN_NetTransportSSL_ErrorString(sslerr),
                sslerr);
      GWEN_Socket_Close(skd->socket);
      SSL_free(skd->ssl);
      skd->ssl=0;
      SSL_CTX_free(skd->ssl_ctx);
      skd->ssl_ctx=0;
      GWEN_NetTransport_MarkActivity(tr);
      return GWEN_NetTransportResultError;
    }
  }
  else if (rv==0) {
    DBG_ERROR(0, "Broken pipe");
    GWEN_Socket_Close(skd->socket);
    SSL_free(skd->ssl);
    skd->ssl=0;
    SSL_CTX_free(skd->ssl_ctx);
    skd->ssl_ctx=0;
    GWEN_NetTransport_MarkActivity(tr);
    return GWEN_NetTransportResultError;
  }

  DBG_NOTICE(0, "Written %d bytes", rv);
  *bsize=rv;
  GWEN_NetTransport_MarkActivity(tr);
  return GWEN_NetTransportResultOk;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSSL_AddSockets(GWEN_NETTRANSPORT *tr,
                                    GWEN_SOCKETSET *sset,
                                    int forReading){
  GWEN_NETTRANSPORTSSL *skd;
  GWEN_ERRORCODE err;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);

  /* add socket */
  err=GWEN_SocketSet_AddSocket(sset, skd->socket);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return -1;
  }
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT_WORKRESULT
GWEN_NetTransportSSL_Work(GWEN_NETTRANSPORT *tr) {
  GWEN_NETTRANSPORTSSL *skd;
  GWEN_NETTRANSPORT_STATUS st;
  GWEN_ERRORCODE err;
  int rv;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);

  st=GWEN_NetTransport_GetStatus(tr);
  DBG_DEBUG(0, "Working with status \"%s\" (%d)",
            GWEN_NetTransport_StatusName(st),
            st);
  switch(st) {
  case GWEN_NetTransportStatusPConnecting: {
    char addrBuffer[128];
    DBG_VERBOUS(0, "Still connecting");

    /* get socket error to check whether the connect succeeded */
    err=GWEN_Socket_GetSocketError(skd->socket);
    if (!GWEN_Error_IsOk(err)) {
      if (GWEN_Error_GetType(err)!=
          GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
          (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
           GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
        DBG_INFO_ERR(0, err);
        return GWEN_NetTransportWorkResult_Error;
      }
      DBG_VERBOUS(0, "Still not connected");
      return GWEN_NetTransportWorkResult_NoChange;
    }
    /* log address */
    GWEN_InetAddr_GetAddress(GWEN_NetTransport_GetPeerAddr(tr),
                             addrBuffer, sizeof(addrBuffer));
    DBG_INFO(0, "Connection established with %s (port %d)",
             addrBuffer,
             GWEN_InetAddr_GetPort(GWEN_NetTransport_GetPeerAddr(tr)));
    /* set to "physically connected" */
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPConnected);
    DBG_INFO(0, "Physical connection established");
    GWEN_NetTransport_MarkActivity(tr);
    return GWEN_NetTransportWorkResult_Change;
  }

  case GWEN_NetTransportStatusPConnected: {
    /* establish SSL */
    int fd;

    DBG_NOTICE(0, "GWEN_NetTransportStatusPConnected");
    /* get socket handle (I know, it's ugly, but the function below is
     * not exported to the outside) */
    fd=GWEN_Socket_GetSocketInt(skd->socket);
    if (fd==-1) {
      DBG_ERROR(0, "No socket handle, cannot use this socket with SSL");
      GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
      return GWEN_NetTransportWorkResult_Error;
    }

    /* socket ready now setup SSL */
    skd->ssl_ctx=SSL_CTX_new(SSLv23_client_method());
    skd->ssl=SSL_new(skd->ssl_ctx);

    /* setup locations of certificates */
    rv=SSL_CTX_load_verify_locations(skd->ssl_ctx,
                                     skd->CAfile,
                                     skd->CAdir);
    if (rv==0) {
      int sslerr;

      sslerr=SSL_get_error(skd->ssl, rv);
      DBG_ERROR(0, "SSL error: %s (%d)",
                GWEN_NetTransportSSL_ErrorString(sslerr),
                sslerr);
      GWEN_Socket_Close(skd->socket);
      SSL_free(skd->ssl);
      skd->ssl=0;
      SSL_CTX_free(skd->ssl_ctx);
      skd->ssl_ctx=0;
      GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
      return GWEN_NetTransportWorkResult_Error;
    }

    if (skd->secure) {
      SSL_CTX_set_verify(skd->ssl_ctx,
                         SSL_VERIFY_PEER,
                         0);
    }
    else {
      SSL_CTX_set_verify(skd->ssl_ctx,
                         SSL_VERIFY_NONE,
                         0);
    }

    /* tell SSL to use our socket */
    rv=SSL_set_fd(skd->ssl, fd);
    if (rv==0) {
      int sslerr;

      sslerr=SSL_get_error(skd->ssl, rv);
      DBG_ERROR(0, "SSL error: %s (%d)",
                GWEN_NetTransportSSL_ErrorString(sslerr),
                sslerr);
      GWEN_Socket_Close(skd->socket);
      SSL_free(skd->ssl);
      skd->ssl=0;
      SSL_CTX_free(skd->ssl_ctx);
      skd->ssl_ctx=0;
      GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
      return GWEN_NetTransportWorkResult_Error;
    }

    /* set to "logically connecting" */
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusLConnecting);
    GWEN_NetTransport_MarkActivity(tr);
    return GWEN_NetTransportWorkResult_Change;
  }

  case GWEN_NetTransportStatusLConnecting: {
    X509 *cert;

    /* check for established SSL */
    if (skd->active) {
      rv=SSL_connect(skd->ssl);
      if (rv<1) {
        int sslerr;

        sslerr=SSL_get_error(skd->ssl, rv);
        if (sslerr!=SSL_ERROR_WANT_READ &&
            sslerr!=SSL_ERROR_WANT_WRITE) {
          DBG_ERROR(0, "SSL error: %s (%d)",
                    GWEN_NetTransportSSL_ErrorString(sslerr),
                    sslerr);
          GWEN_Socket_Close(skd->socket);
          SSL_free(skd->ssl);
          skd->ssl=0;
          SSL_CTX_free(skd->ssl_ctx);
          skd->ssl_ctx=0;
          return GWEN_NetTransportWorkResult_Error;
        }
        else {
          return GWEN_NetTransportWorkResult_NoChange;
        }
      }
    }
    else {
      /* passive */
      /* TODO */
      return GWEN_NetTransportWorkResult_NoChange;
    }

    /* now logically connected */
    cert=SSL_get_peer_certificate(skd->ssl);
    if (!cert) {
      if (skd->secure) {
        DBG_ERROR(0, "Peer did not send a certificate, abort");
        GWEN_Socket_Close(skd->socket);
        SSL_free(skd->ssl);
        skd->ssl=0;
        SSL_CTX_free(skd->ssl_ctx);
        skd->ssl_ctx=0;
        return GWEN_NetTransportWorkResult_Error;
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
      if (SSL_get_verify_result(skd->ssl)!=X509_V_OK) {
        if (skd->secure) {
          DBG_ERROR(0, "Invalid peer certificate, aborting");
          GWEN_Socket_Close(skd->socket);
          SSL_free(skd->ssl);
          skd->ssl=0;
          SSL_CTX_free(skd->ssl_ctx);
          skd->ssl_ctx=0;
          return GWEN_NetTransportWorkResult_Error;
        }
        else {
          DBG_WARN(0, "Invalid peer certificate, ignoring");
        }
      }
      else {
        DBG_NOTICE(0, "Peer certificate is valid");
        skd->isSecure=1;
      }
      X509_free(cert);
    }

    DBG_NOTICE(0, "SSL connection established (%s)",
               (skd->isSecure)?"verified":"not verified");

    GWEN_NetTransport_MarkActivity(tr);
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusLConnected);
    return GWEN_NetTransportWorkResult_Change;
  }

  case GWEN_NetTransportStatusListening: {
    GWEN_SOCKET *newS;
    GWEN_INETADDRESS *iaddr;
    GWEN_NETTRANSPORT *newTr;
    char addrBuffer[128];

    DBG_VERBOUS(0, "Listening");
    if (GWEN_NetTransport_GetIncomingCount(tr)+1<
        GWEN_NetTransport_GetBackLog(tr)) {
      newS=0;
      iaddr=0;
      /* accept new connection (if there is any) */
      err=GWEN_Socket_Accept(skd->socket, &iaddr, &newS);
      if (!GWEN_Error_IsOk(err)) {
        GWEN_InetAddr_free(iaddr);
        GWEN_Socket_free(newS);
        /* no new connection, due to an error ? */
        if (GWEN_Error_GetType(err)!=
            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
            (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
             GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
          /* jepp, there was an error */
          DBG_INFO_ERR(0, err);
          return GWEN_NetTransportWorkResult_Error;
        }
        /* otherwise there simply is no waiting connection */
        DBG_DEBUG(0, "No incoming connection");
        return GWEN_NetTransportWorkResult_NoChange;
      }

      /* we have an incoming connection */
      GWEN_InetAddr_GetAddress(iaddr, addrBuffer, sizeof(addrBuffer));
      DBG_INFO(0, "Incoming connection from %s (port %d)",
               addrBuffer, GWEN_InetAddr_GetPort(iaddr));

      /* set socket nonblocking */
      err=GWEN_Socket_SetBlocking(newS, 0);
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(0, err);
        GWEN_InetAddr_free(iaddr);
        GWEN_Socket_free(newS);
        return GWEN_NetTransportWorkResult_Error;
      }

      /* create new transport layer, let it take over the new socket */
      newTr=GWEN_NetTransportSSL_new(newS,
                                     skd->CAfile, skd->CAdir, skd->secure,
                                     1);
      GWEN_NetTransport_SetPeerAddr(newTr, iaddr);
      GWEN_InetAddr_free(iaddr);
      GWEN_NetTransport_SetLocalAddr(newTr, GWEN_NetTransport_GetLocalAddr(tr));
      /* set status (already fully connected) */
      GWEN_NetTransport_SetStatus(newTr, GWEN_NetTransportStatusPConnected);
      /* set flags: This connection is a passive one */
      GWEN_NetTransport_SetFlags(newTr,
                                 GWEN_NetTransport_GetFlags(newTr) |
                                 GWEN_NETTRANSPORT_FLAGS_PASSIVE);

      /* add it to queue of incoming connections */
      GWEN_NetTransport_AddNextIncoming(tr, newTr);
      return GWEN_NetTransportWorkResult_Change;
    }
    else {
      DBG_INFO(0, "Too many incoming connections waiting");
    }
    GWEN_NetTransport_MarkActivity(tr);
    break;
  }

  case GWEN_NetTransportStatusLConnected:
    DBG_DEBUG(0, "Active connection, nothing to do");
    /* TODO: check whether read/write are possible, return code
     * accordingly */
    break;

  case GWEN_NetTransportStatusLDisconnecting: {
    rv=SSL_shutdown(skd->ssl);
    if (rv==1 || rv==-1) {
      /* connection closed */
      DBG_NOTICE(0, "Connection closed");
      GWEN_Socket_Close(skd->socket);
      SSL_free(skd->ssl);
      skd->ssl=0;
      SSL_CTX_free(skd->ssl_ctx);
      skd->ssl_ctx=0;
      GWEN_NetTransport_MarkActivity(tr);
      GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPDisconnected);
      return GWEN_NetTransportWorkResult_Change;
    }
    break;
  }

  case GWEN_NetTransportStatusUnconnected:
  case GWEN_NetTransportStatusDisabled:
  case GWEN_NetTransportStatusPDisconnecting:
  case GWEN_NetTransportStatusPDisconnected:
    DBG_VERBOUS(0, "Inactive connection (status \"%s\")",
                GWEN_NetTransport_StatusName(st));
    break;

  default:
    DBG_WARN(0, "Hmm, status \"%s\" (%d) is unexpected...",
             GWEN_NetTransport_StatusName(st),
             st);
    break;
  } /* switch */

  return GWEN_NetTransportWorkResult_NoChange;
}



/* -------------------------------------------------------------- FUNCTION */
const char *GWEN_NetTransportSSL_ErrorString(unsigned int e) {
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




