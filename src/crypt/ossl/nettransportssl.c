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

#define DEBUG_SSL_LOG

#include "nettransportssl_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/directory.h>
#include "inetsocket_l.h"
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/dbio.h>

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/dh.h>
#include <openssl/rand.h>

#include <openssl/conf.h>
#include <openssl/x509v3.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#endif /* HAVE_SYS_SOCKET_H */

#ifdef ENABLE_NLS
# include <libintl.h>
# include <locale.h>
# define I18N(text) dgettext("gwenhywfar", text)
#else
# define I18N(text) text
#endif


GWEN_INHERIT(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL);


static GWEN_NETTRANSPORTSSL_GETPASSWD_FN
  gwen_netransportssl_getPasswordFn=0;

static GWEN_NETTRANSPORTSSL_ASKADDCERT_FN
  gwen_netransportssl_askAddCertFn=0;


static GWEN_NETTRANSPORTSSL_ASKADDCERT_FN2
  gwen_netransportssl_askAddCertFn2=0;

static void *gwen_netransportssl_askAddCertUserData=0;



int GWEN_NetTransportSSL__GetPublicCaFile(GWEN_BUFFER *pbuf) {
#ifdef OS_WIN32
  HKEY hkey;
  TCHAR nbuffer[MAX_PATH];
  BYTE vbuffer[MAX_PATH];
  DWORD nsize;
  DWORD vsize;
  DWORD typ;
  int i;

  snprintf(nbuffer, sizeof(nbuffer), "Software\\Gwenhywfar\\Paths");

  /* open the key */
  if (RegOpenKey(HKEY_CURRENT_USER, nbuffer, &hkey)){
    DBG_ERROR(GWEN_LOGDOMAIN,
              "RegOpenKey failed, returning compile-time value");
    GWEN_Directory_OsifyPath(PUBLIC_CA_FILE,
                             pbuf,
                             1);
    return 1;
  }

  /* find the key for dbio-plugins */
  for (i=0;; i++) {
    nsize=sizeof(nbuffer);
    vsize=sizeof(vbuffer);
    if (ERROR_SUCCESS!=RegEnumValue(hkey,
                                    i,    /* index */
                                    nbuffer,
                                    &nsize,
                                    0,       /* reserved */
                                    &typ,
                                    vbuffer,
                                    &vsize))
      break;
    if (strcasecmp(nbuffer, "public-ca-file")==0 &&
        typ==REG_SZ) {
      /* variable found */
      RegCloseKey(hkey);
      GWEN_Buffer_AppendBytes(pbuf, (char*)vbuffer, vsize-1);
      return 0;
    }
  } /* for */

  RegCloseKey(hkey);
  DBG_INFO(GWEN_LOGDOMAIN,
           "RegKey does not exist, returning compile-time value");
  GWEN_Directory_OsifyPath(PUBLIC_CA_FILE,
			   pbuf,
			   1);
  return 1;
#else
  GWEN_Directory_OsifyPath(PUBLIC_CA_FILE,
			   pbuf,
			   0);
  return 0;
#endif
}


/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORTSSL *GWEN_NetTransportSSLData_new(){
  GWEN_NETTRANSPORTSSL *skd;
  GWEN_BUFFER *tbuf;
  int rv;

  GWEN_NEW_OBJECT(GWEN_NETTRANSPORTSSL, skd);
  DBG_MEM_INC("GWEN_NETTRANSPORTSSL", 0);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_NetTransportSSL__GetPublicCaFile(tbuf);
  if (rv<0) {
    DBG_ERROR(0, "Could not get the path and name of the public cert file");
  }
  else {
    skd->CAfile=strdup(GWEN_Buffer_GetStart(tbuf));
  }
  GWEN_Buffer_free(tbuf);

  return skd;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetTransportSSLData_free(GWEN_NETTRANSPORTSSL *skd){
  if (skd) {
    DBG_MEM_DEC("GWEN_NETTRANSPORTSSL");
    if (skd->ownSocket)
      GWEN_Socket_free(skd->socket);
    free(skd->CAfile);
    free(skd->CAdir);
    free(skd->newCAdir);
    free(skd->ownCertFile);
    free(skd->dhfile);
    free(skd->cipherList);
    free(skd->peerCertificate);
    free(skd);
  }
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT *GWEN_NetTransportSSL_new(GWEN_SOCKET *sk,
                                            const char *capath,
                                            const char *newcapath,
                                            const char *ownCertFile,
					    const char *dhfile,
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
  if (capath)
    skd->CAdir=strdup(capath);
  if (newcapath)
    skd->newCAdir=strdup(newcapath);
  if (ownCertFile)
    skd->ownCertFile=strdup(ownCertFile);
  if (dhfile)
    skd->dhfile=strdup(dhfile);

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
GWEN_DB_NODE*
GWEN_NetTransportSSL_GetPeerCertificate(const GWEN_NETTRANSPORT *tr) {
  GWEN_NETTRANSPORTSSL *skd;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);
  assert(skd);

  return skd->peerCertificate;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSSL_IsOfType(GWEN_NETTRANSPORT *tr) {
  return GWEN_INHERIT_ISOFTYPE(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSSL_IsSecure(const GWEN_NETTRANSPORT *tr){
  GWEN_NETTRANSPORTSSL *skd;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);
  assert(skd);

  return skd->isSecure;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT_RESULT
GWEN_NetTransportSSL_StartConnect(GWEN_NETTRANSPORT *tr){
  GWEN_NETTRANSPORTSSL *skd;
  GWEN_ERRORCODE err;
  char addrBuffer[128];

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);
  assert(skd);

  GWEN_InetAddr_GetAddress(GWEN_NetTransport_GetPeerAddr(tr),
                           addrBuffer, sizeof(addrBuffer));

  if (GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusUnconnected &&
      GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusPDisconnected){
    DBG_ERROR(GWEN_LOGDOMAIN, "Socket is not unconnected (status \"%s\")",
              GWEN_NetTransport_StatusName(GWEN_NetTransport_GetStatus(tr)));
    return GWEN_NetTransportResultError;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Starting to connect to %s (port %d)",
           addrBuffer,
           GWEN_InetAddr_GetPort(GWEN_NetTransport_GetPeerAddr(tr)));

  /* arm socket code */
  err=GWEN_Socket_Open(skd->socket);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_NetTransportResultError;
  }

  /* set nonblocking */
  err=GWEN_Socket_SetBlocking(skd->socket, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
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
      DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
      return GWEN_NetTransportResultError;
    }

    /* adjust status (physically connecting) */
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPConnecting);
    GWEN_NetTransport_MarkActivity(tr);
    return GWEN_NetTransportResultWantRead;
  }
  else {
    /* connection succeeded */
    DBG_INFO(GWEN_LOGDOMAIN, "Connection established with %s (port %d)",
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
    DBG_ERROR(GWEN_LOGDOMAIN, "Socket is not unconnected (%d)",
              GWEN_NetTransport_GetStatus(tr));
    return GWEN_NetTransportResultError;
  }

  GWEN_InetAddr_GetAddress(GWEN_NetTransport_GetLocalAddr(tr),
                           addrBuffer, sizeof(addrBuffer));

  DBG_INFO(GWEN_LOGDOMAIN, "Starting to listen on %s (port %d)",
           addrBuffer,
           GWEN_InetAddr_GetPort(GWEN_NetTransport_GetLocalAddr(tr)));

  /* arm socket code */
  err=GWEN_Socket_Open(skd->socket);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_NetTransportResultError;
  }

  /* set nonblocking */
  err=GWEN_Socket_SetBlocking(skd->socket, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_NetTransportResultError;
  }

  /* reuse address */
  err=GWEN_Socket_SetReuseAddress(skd->socket, 1);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_NetTransportResultError;
  }

  /* bind socket to local address */
  skd->active=0;
  err=GWEN_Socket_Bind(skd->socket, GWEN_NetTransport_GetLocalAddr(tr));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_NetTransportResultError;
  }

  /* start listening */
  err=GWEN_Socket_Listen(skd->socket, 10);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
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
    DBG_INFO(GWEN_LOGDOMAIN,
	     "Socket is inactive: %s (%d)",
	     GWEN_NetTransport_StatusName(GWEN_NetTransport_GetStatus(tr)),
	     GWEN_NetTransport_GetStatus(tr));
    return GWEN_NetTransportResultError;
  }

  if (!skd->ssl) {
    /* connection closed */
    DBG_INFO(GWEN_LOGDOMAIN, "Connection closed");
    GWEN_Socket_Close(skd->socket);
    GWEN_NetTransport_MarkActivity(tr);
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPDisconnected);
    return GWEN_NetTransportResultOk;
  }
  rv=SSL_shutdown(skd->ssl);
  if (!rv) {
    /* send a TCP_FIN to trigger the other side's close_notify */
    shutdown(GWEN_Socket_GetSocketInt(skd->socket), 1);
    rv=SSL_shutdown(skd->ssl);
  }

  if (rv==1 || rv==-1) {
    /* connection closed */
    DBG_INFO(GWEN_LOGDOMAIN, "Connection closed");
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
    DBG_ERROR(GWEN_LOGDOMAIN, "Socket is not connected (%d)",
              GWEN_NetTransport_GetStatus(tr));
    return GWEN_NetTransportResultError;
  }

  /* try to read */
  DBG_DEBUG(GWEN_LOGDOMAIN, "Reading up to %d bytes while status \"%s\"",
            *bsize,
            SSL_state_string_long(skd->ssl));

  ERR_clear_error();
  rv=SSL_read(skd->ssl, buffer, *bsize);
  if (rv<1) {
    int sslerr;

    sslerr=SSL_get_error(skd->ssl, rv);
    if (sslerr==SSL_ERROR_WANT_READ)
      return GWEN_NetTransportResultWantRead;
    else if (sslerr==SSL_ERROR_WANT_WRITE)
      return GWEN_NetTransportResultWantWrite;
    else {
      /* FIXME: Simplify this */
      if (sslerr==SSL_ERROR_SYSCALL && errno==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Connection just went down (%d: %s)",
                  sslerr,
                  GWEN_NetTransportSSL_ErrorString(sslerr));
        GWEN_Socket_Close(skd->socket);
        SSL_free(skd->ssl);
        skd->ssl=0;
        SSL_CTX_free(skd->ssl_ctx);
        skd->ssl_ctx=0;
        /* connection closed, no real error */
        GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPDisconnected);
        *bsize=0;
        GWEN_NetTransport_MarkActivity(tr);
        return GWEN_NetTransportResultOk;
      }
      else {
        if (sslerr==SSL_ERROR_ZERO_RETURN) {
          DBG_INFO(GWEN_LOGDOMAIN, "Connection closed");
          GWEN_Socket_Close(skd->socket);
          SSL_free(skd->ssl);
          skd->ssl=0;
          SSL_CTX_free(skd->ssl_ctx);
          skd->ssl_ctx=0;
          /* connection closed, no real error */
          GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPDisconnected);
          *bsize=0;
          GWEN_NetTransport_MarkActivity(tr);
          return GWEN_NetTransportResultOk;
        }
        else {
          DBG_ERROR(GWEN_LOGDOMAIN, "List of pending SSL errors:");
          ERR_print_errors_fp(stderr); /* DEBUG */
          DBG_ERROR(GWEN_LOGDOMAIN, "SSL error: %s (%d)",
                    GWEN_NetTransportSSL_ErrorString(sslerr),
                    sslerr);
        }
      }

      GWEN_Socket_Close(skd->socket);
      SSL_free(skd->ssl);
      skd->ssl=0;
      SSL_CTX_free(skd->ssl_ctx);
      skd->ssl_ctx=0;

      if (sslerr==SSL_ERROR_ZERO_RETURN) {
        /* connection closed, no real error */
        GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPDisconnected);
        *bsize=0;
        GWEN_NetTransport_MarkActivity(tr);
        return GWEN_NetTransportResultOk;
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "List of pending SSL errors:");
        ERR_print_errors_fp(stderr); /* DEBUG */
        DBG_ERROR(GWEN_LOGDOMAIN, "SSL error: %s (%d)",
                  GWEN_NetTransportSSL_ErrorString(sslerr),
                  sslerr);
        DBG_ERROR(GWEN_LOGDOMAIN, "Disabling connection");
        GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
        GWEN_NetTransport_MarkActivity(tr);
        return GWEN_NetTransportResultError;
      }
    }
  }

  DBG_VERBOUS(GWEN_LOGDOMAIN, "Read %d bytes:", rv);
  GWEN_Text_LogString(buffer, rv, 0, GWEN_LoggerLevelVerbous);

  if (getenv("GWEN_SSL_DEBUG")) {
    FILE *f;

    DBG_NOTICE(GWEN_LOGDOMAIN, "Saving...");
    f=fopen("/tmp/read.bin", "a+");
    if (!f) {
      DBG_ERROR(GWEN_LOGDOMAIN, "fopen: %s", strerror(errno));
    }
    else {
      if (fwrite(buffer, rv, 1, f)!=1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "fwrite: %s", strerror(errno));
      }
      if (fclose(f)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "fclose: %s", strerror(errno));
      }
    }
  }

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
    DBG_ERROR(GWEN_LOGDOMAIN, "Socket is not connected (%d)",
              GWEN_NetTransport_GetStatus(tr));
    return GWEN_NetTransportResultError;
  }

  /* try to read */
  ERR_clear_error();
  rv=SSL_write(skd->ssl, buffer, *bsize);
  if (rv<1) {
    int sslerr;

    sslerr=SSL_get_error(skd->ssl, rv);
    if (sslerr==SSL_ERROR_WANT_READ)
      return GWEN_NetTransportResultWantRead;
    else if (sslerr==SSL_ERROR_WANT_WRITE)
      return GWEN_NetTransportResultWantWrite;
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "SSL error: %s (%d)",
                GWEN_NetTransportSSL_ErrorString(sslerr),
                sslerr);
      GWEN_Socket_Close(skd->socket);
      SSL_free(skd->ssl);
      skd->ssl=0;
      SSL_CTX_free(skd->ssl_ctx);
      skd->ssl_ctx=0;
      GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusUnconnected);
      GWEN_NetTransport_MarkActivity(tr);
      return GWEN_NetTransportResultError;
    }
  }
  else if (rv==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Broken pipe");
    GWEN_Socket_Close(skd->socket);
    SSL_free(skd->ssl);
    skd->ssl=0;
    SSL_CTX_free(skd->ssl_ctx);
    skd->ssl_ctx=0;
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusUnconnected);
    GWEN_NetTransport_MarkActivity(tr);
    return GWEN_NetTransportResultError;
  }

  DBG_DEBUG(GWEN_LOGDOMAIN, "Written %d bytes:", rv);
  GWEN_Text_LogString(buffer, rv, 0, GWEN_LoggerLevelVerbous); 

  if (getenv("GWEN_SSL_DEBUG")) {
    FILE *f;

    DBG_NOTICE(GWEN_LOGDOMAIN, "Saving...");
    f=fopen("/tmp/written.bin", "a+");
    if (!f) {
      DBG_ERROR(GWEN_LOGDOMAIN, "fopen: %s", strerror(errno));
    }
    else {
      if (fwrite(buffer, rv, 1, f)!=1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "fwrite: %s", strerror(errno));
      }
      if (fclose(f)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "fclose: %s", strerror(errno));
      }
    }
  }

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
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    return -1;
  }
  return 0;
}


/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSSL__Check_Cert(GWEN_NETTRANSPORTSSL *skd,
                                     const char *name){
  X509 *peer;
  char cn[256];

  if (SSL_get_verify_result(skd->ssl)!=X509_V_OK) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid certificate");
    return -1;
  }
  /* check common name */
  peer=SSL_get_peer_certificate(skd->ssl);
  X509_NAME_get_text_by_NID(X509_get_subject_name(peer),
                            NID_commonName, cn, sizeof(cn));
  if(strcasecmp(cn, name)) {
    DBG_WARN(GWEN_LOGDOMAIN, "Common name does not match (\"%s\" != \"%s\")",
             cn, name);
    return -1;
  }
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSSL_GetPassword(GWEN_NETTRANSPORT *tr,
                                     char *buffer, int num, int rwflag){
  if (gwen_netransportssl_getPasswordFn)
    return gwen_netransportssl_getPasswordFn(tr, buffer, num, rwflag);
  else {
    DBG_WARN(GWEN_LOGDOMAIN, "No getPasswordFn set");
    return 0;
  }
}



/* -------------------------------------------------------------- FUNCTION */
void
GWEN_NetTransportSSL_SetGetPasswordFn(GWEN_NETTRANSPORTSSL_GETPASSWD_FN fn){
  gwen_netransportssl_getPasswordFn=fn;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORTSSL_GETPASSWD_FN
GWEN_NetTransportSSL_GetGetPasswordFn(){
  return gwen_netransportssl_getPasswordFn;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSSL_PasswordCB(char *buffer, int num,
                                    int rwflag, void *userdata){
  return GWEN_NetTransportSSL_GetPassword((GWEN_NETTRANSPORT*)userdata,
                                          buffer, num, rwflag);
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORTSSL_ASKADDCERT_RESULT
GWEN_NetTransportSSL__AskAddCert(GWEN_NETTRANSPORT *tr,
				 GWEN_DB_NODE *cert){
  DBG_INFO(GWEN_LOGDOMAIN, "Would ask user about this:");
  if (GWEN_Logger_GetLevel(GWEN_LOGDOMAIN)>=GWEN_LoggerLevelInfo)
    GWEN_DB_Dump(cert, stderr, 2);

  if (gwen_netransportssl_askAddCertFn2)
    return gwen_netransportssl_askAddCertFn2(tr,
					     cert,
					     gwen_netransportssl_askAddCertUserData);
  else if (gwen_netransportssl_askAddCertFn)
    return gwen_netransportssl_askAddCertFn(tr, cert);
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No askAddCert function set");
    return GWEN_NetTransportSSL_AskAddCertResultNo;
  }
}



/* -------------------------------------------------------------- FUNCTION */
void
GWEN_NetTransportSSL_SetAskAddCertFn(GWEN_NETTRANSPORTSSL_ASKADDCERT_FN fn){
  gwen_netransportssl_askAddCertFn=fn;
  gwen_netransportssl_askAddCertFn2=0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORTSSL_ASKADDCERT_FN GWEN_NetTransportSSL_GetAskAddCertFn(){
  return gwen_netransportssl_askAddCertFn;
}



/* -------------------------------------------------------------- FUNCTION */
void
GWEN_NetTransportSSL_SetAskAddCertFn2(GWEN_NETTRANSPORTSSL_ASKADDCERT_FN2 fn,
				      void *user_data){
  gwen_netransportssl_askAddCertFn2=fn;
  gwen_netransportssl_askAddCertFn=0;
  gwen_netransportssl_askAddCertUserData=user_data;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetTransportSSL__InfoCallBack(SSL *s, int where, int ret){
  const char *str;
  int w;

  w=where& ~SSL_ST_MASK;
  if (w & SSL_ST_CONNECT)
    str="SSL_connect";
  else if (w & SSL_ST_ACCEPT)
    str="SSL_accept";
  else
    str="undefined";

  if (where & SSL_CB_LOOP){
    DBG_INFO(GWEN_LOGDOMAIN,"%s: %s",str,SSL_state_string_long(s));
  }
  else if (where & SSL_CB_ALERT){
    str=(where & SSL_CB_READ)?"read":"write";
    DBG_INFO(GWEN_LOGDOMAIN, "SSL3 alert %s: %s: %s",
             str,
             SSL_alert_type_string_long(ret),
             SSL_alert_desc_string_long(ret));
  }
  else if (where & SSL_CB_EXIT){
    if (ret==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "%s: failed in \"%s\"",
               str,
               SSL_state_string_long(s));
    }
    else if (ret<0){
      DBG_DEBUG(GWEN_LOGDOMAIN, "%s: error in \"%s\"",
                str,
                SSL_state_string_long(s));
    }
  }
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSSL__SaveCert(GWEN_NETTRANSPORT *tr,
                                   X509 *cert,
                                   const char *dir,
                                   int overwrite) {
  FILE *f;
  const char *fmode = "";
  char cn[256];
  GWEN_NETTRANSPORTSSL *skd;
  X509_NAME *nm;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);
  assert(skd);

  nm=X509_get_subject_name(cert);
  X509_NAME_get_text_by_NID(nm,
                            NID_commonName, cn, sizeof(cn));

  if (!dir)
    dir=skd->CAdir;
  /*if (skd->CAdir) { */
  if (dir) {
    GWEN_BUFFER *nbuf;
    unsigned long hash;
    char numbuf[32];
    int i;
    GWEN_TYPE_UINT32 pos;

    hash=X509_NAME_hash(nm);
    snprintf(numbuf, sizeof(numbuf), "%08lx", hash);
    nbuf=GWEN_Buffer_new(0, 128, 0, 1);
    GWEN_Buffer_AppendString(nbuf, dir);
    /* check path, create it if necessary */
    if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(nbuf),
                               GWEN_PATH_FLAGS_CHECKROOT)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid path (\"%s\")", GWEN_Buffer_GetStart(nbuf));
      GWEN_Buffer_free(nbuf);
      return -1;
    }
    GWEN_Buffer_AppendByte(nbuf, '/');
    GWEN_Buffer_AppendString(nbuf, numbuf);
    pos=GWEN_Buffer_GetPos(nbuf);
    for (i=0; i<GWEN_NETTRANSPORTSSL_MAXCOLL; i++) {
      snprintf(numbuf, sizeof(numbuf), "%d", i);
      GWEN_Buffer_Crop(nbuf, 0, pos);
      GWEN_Buffer_SetPos(nbuf, pos);
      GWEN_Buffer_AppendByte(nbuf, '.');
      GWEN_Buffer_AppendString(nbuf, numbuf);
      if (overwrite)
        /* overwrite older incoming certificates */
        break;
      if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(nbuf),
                                 GWEN_PATH_FLAGS_NAMEMUSTEXIST|
                                 GWEN_PATH_FLAGS_VARIABLE|
                                 GWEN_PATH_FLAGS_CHECKROOT)) {
        break;
      }
    }
    if (i>=GWEN_NETTRANSPORTSSL_MAXCOLL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Maximum number of hash collisions reached!");
      GWEN_Buffer_free(nbuf);
      return -1;
    }

    DBG_DEBUG(GWEN_LOGDOMAIN, "Saving file as \"%s\"", GWEN_Buffer_GetStart(nbuf));
    f=fopen(GWEN_Buffer_GetStart(nbuf), "w+");
    if (!f) {
      DBG_ERROR(GWEN_LOGDOMAIN, "fopen(\"%s\", \"%s\"): %s",
                GWEN_Buffer_GetStart(nbuf), fmode, strerror(errno));
      GWEN_Buffer_free(nbuf);
      return -1;
    }
    GWEN_Buffer_free(nbuf);
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Don't know where to save the file...");
    return -1;
  }

  if (!PEM_write_X509(f, cert)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not save certificate of \"%s\"", cn);
    return 0;
  }

  if (fclose(f)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fclose: %s", strerror(errno));
    return -1;
  }
  DBG_INFO(GWEN_LOGDOMAIN, "Certificate of \"%s\" added", cn);

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSSL__VerifyCallBack(int preverify_ok,
                                         X509_STORE_CTX *ctx) {
  int err;

  err=X509_STORE_CTX_get_error(ctx);
  if (!preverify_ok) {
    DBG_INFO(GWEN_LOGDOMAIN, "Verify error %d: \"%s\"",
             err, X509_verify_cert_error_string(err));
    if (err==X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY ||
        err==X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT ||
        err==X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN) {
      DBG_INFO(GWEN_LOGDOMAIN, "Unknown certificate, will not abort yet");
      return 1;
    }
  }
  return preverify_ok;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSSL__SetupSSL(GWEN_NETTRANSPORT *tr, int fd){
  int rv = 0;
  GWEN_NETTRANSPORTSSL *skd;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);
  assert(skd);

  /* enable all workarounds for bugs in other SSL implementation for
   * maximum compatibility */
  SSL_CTX_set_options(skd->ssl_ctx, SSL_OP_ALL);

  /* set default password handler and data */
  SSL_CTX_set_default_passwd_cb(skd->ssl_ctx,
                                GWEN_NetTransportSSL_PasswordCB);
  SSL_CTX_set_default_passwd_cb_userdata(skd->ssl_ctx,
                                         tr);

  if (skd->ownCertFile) {
    /* load own certificate file */
    DBG_DEBUG(GWEN_LOGDOMAIN, "Loading certificate and keys");
    if (!(SSL_CTX_use_certificate_chain_file(skd->ssl_ctx,
					     skd->ownCertFile))){
      int sslerr;

      sslerr=SSL_get_error(skd->ssl, rv);
      DBG_ERROR(GWEN_LOGDOMAIN, "SSL error reading certfile: %s (%d)",
                GWEN_NetTransportSSL_ErrorString(sslerr),
                sslerr);
      return -1;
    }
    /* load keys */
    if(!(SSL_CTX_use_PrivateKey_file(skd->ssl_ctx,
                                     skd->ownCertFile,
                                     SSL_FILETYPE_PEM))) {
      int sslerr;

      sslerr=SSL_get_error(skd->ssl, rv);
      DBG_ERROR(GWEN_LOGDOMAIN, "SSL error reading keyfile: %s (%d)",
                GWEN_NetTransportSSL_ErrorString(sslerr),
                sslerr);
      return -1;
    }
    if (!SSL_CTX_check_private_key(skd->ssl_ctx)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Private key does not match the certificate public key");
      return -1;
    }

  }

  /* setup locations of certificates */
  if (skd->CAdir || skd->CAfile) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Loading certificate locations");
    rv=SSL_CTX_load_verify_locations(skd->ssl_ctx,
				     skd->CAfile,
				     skd->CAdir);
    if (rv==0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"SSL: Could not load certificate location "
		"(was: \"%s\" and \"%s\")",
		skd->CAfile,
		skd->CAdir);
      return -1;
    }
  }

  //SSL_CTX_set_verify_depth(skd->ssl_ctx, 1);

  /* setup server stuff */
  if (!skd->active) {
    if (GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusListening) {
      FILE *f;

      DBG_DEBUG(GWEN_LOGDOMAIN, "Loading DH params");

      f=fopen(skd->dhfile, "r");
      if (!f) {
        DBG_ERROR(GWEN_LOGDOMAIN, "SSL: fopen(%s): %s", skd->dhfile, strerror(errno));
        return -1;
      }
      else {
        DH *dh_tmp;
        int codes;

        dh_tmp=PEM_read_DHparams(f, NULL, NULL, NULL);
        fclose(f);
        if (dh_tmp==0) {
          DBG_ERROR(GWEN_LOGDOMAIN, "SSL: Error reading DH params");
          return -1;
        }

        /* check for usability */
        if (!DH_check(dh_tmp, &codes)){
          int sslerr;

          sslerr=SSL_get_error(skd->ssl, rv);
          DBG_ERROR(GWEN_LOGDOMAIN, "SSL DH_check error: %s (%d)",
                    GWEN_NetTransportSSL_ErrorString(sslerr),
                    sslerr);
          DH_free(dh_tmp);
          return -1;
        }

        if (codes & DH_CHECK_P_NOT_PRIME){
          DBG_ERROR(GWEN_LOGDOMAIN, "SSL DH error: p is not prime");
          DH_free(dh_tmp);
          return -1;
        }
        if ((codes & DH_NOT_SUITABLE_GENERATOR) &&
            (codes & DH_CHECK_P_NOT_SAFE_PRIME)){
          DBG_ERROR(GWEN_LOGDOMAIN, "SSL DH error : "
                    "neither suitable generator or safe prime");
          DH_free(dh_tmp);
          return -1;
        }

        /* DH params seem to be ok */
        if (SSL_CTX_set_tmp_dh(skd->ssl_ctx, dh_tmp)<0) {
          DBG_ERROR(GWEN_LOGDOMAIN, "SSL: Could not set DH params");
          DH_free(dh_tmp);
          return -1;
        }

        /* always expect peer certificate */
        if (skd->secure)
          SSL_CTX_set_verify(skd->ssl_ctx,
                             SSL_VERIFY_PEER |
                             SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                             GWEN_NetTransportSSL__VerifyCallBack);
        else
          SSL_CTX_set_verify(skd->ssl_ctx,
                             SSL_VERIFY_PEER,
                             GWEN_NetTransportSSL__VerifyCallBack);
      }
    }
  }
  else {
    /* active connection */
    if (skd->secure)
      SSL_CTX_set_verify(skd->ssl_ctx,
                         SSL_VERIFY_PEER,
                         GWEN_NetTransportSSL__VerifyCallBack);
    else
      SSL_CTX_set_verify(skd->ssl_ctx,
                         SSL_VERIFY_NONE,
                         GWEN_NetTransportSSL__VerifyCallBack);
  }

  skd->ssl=SSL_new(skd->ssl_ctx);
  if (skd->active)
    SSL_set_connect_state(skd->ssl);
  else
    SSL_set_accept_state(skd->ssl);

  /* set info callback */
  SSL_set_info_callback(skd->ssl,
                        (void (*)()) GWEN_NetTransportSSL__InfoCallBack);

  /* tell SSL to use our socket */
  rv=SSL_set_fd(skd->ssl, fd);
  if (rv==0) {
    int sslerr;

    sslerr=SSL_get_error(skd->ssl, rv);
    DBG_ERROR(GWEN_LOGDOMAIN, "SSL error setting socket: %s (%d)",
	      GWEN_NetTransportSSL_ErrorString(sslerr),
              sslerr);
    return -1;
  }

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetTransportSSL__CertEntries2Db(X509_NAME *nm,
                                          GWEN_DB_NODE *dbCert,
                                          int nid,
                                          const char *name) {
  X509_NAME_ENTRY *e;
  const unsigned char *p;
  int len;
  int lastpos;
  char *cpy;

  lastpos=-1;
  for (;;) {
    lastpos=X509_NAME_get_index_by_NID(nm, nid, lastpos);
    if (lastpos==-1)
      break;
    e=X509_NAME_get_entry(nm, lastpos);
    assert(e);
    p=e->value->data;
    len=e->value->length;
    if (p) {
      cpy=(char*)malloc(len+1);
      memmove(cpy, p, len);
      cpy[len]=0;
      GWEN_DB_SetCharValue(dbCert, GWEN_DB_FLAGS_DEFAULT,
                           name, cpy);
      free(cpy);
    }
  } /* for */
}



int GWEN_NetTransportSSL__ASN_UTC2Db(ASN1_TIME *d,
                                     GWEN_DB_NODE *db,
                                     const char *name) {
  if (d->data) {
    const char *s;
    unsigned int i;
    struct tm t;
    struct tm *lt;
    time_t currTime;
    int isUtc;

    s=(const char*)(d->data);
    i=strlen(s);
    if (i<10) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad time expression (%s)", s);
      return -1;
    }
    currTime=time(0);
    isUtc=(s[i-1]=='Z');
    if (isUtc)
      lt=gmtime(&currTime);
    else
      lt=localtime(&currTime);

    memmove(&t, lt, sizeof(t));
    t.tm_year=((s[0]-'0')*10)+(s[1]-'0')+100;
    t.tm_mon=(((s[2]-'0')*10)+(s[3]-'0'))-1;
    t.tm_mday=((s[4]-'0')*10)+(s[5]-'0');
    t.tm_hour=((s[6]-'0')*10)+(s[7]-'0');
    t.tm_min=((s[8]-'0')*10)+(s[9]-'0');
    if (i>11)
      t.tm_sec=((s[10]-'0')*10)+(s[11]-'0');
    else
      t.tm_sec=0;
    t.tm_wday=0;
    t.tm_yday=0;

    currTime=mktime(&t);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        name, (unsigned int)currTime);
    return 0;
  }
  else {
    return -1;
  }
}


/* -------------------------------------------------------------- FUNCTION */
GWEN_DB_NODE *GWEN_NetTransportSSL__Cert2Db(X509 *cert) {
  GWEN_DB_NODE *dbCert;
  X509_NAME *nm;
  ASN1_TIME *d;
  EVP_PKEY *pktmp;
  unsigned int  md_size;
  unsigned char md[EVP_MAX_MD_SIZE];

  nm=X509_get_subject_name(cert);

  /* setup certificate db */
  dbCert=GWEN_DB_Group_new("cert");

  GWEN_NetTransportSSL__CertEntries2Db(nm, dbCert,
                                       NID_commonName,
                                       "commonName");
  GWEN_NetTransportSSL__CertEntries2Db(nm, dbCert,
                                       NID_organizationName,
                                       "organizationName");
  GWEN_NetTransportSSL__CertEntries2Db(nm, dbCert,
                                       NID_organizationalUnitName,
                                       "organizationalUnitName");
  GWEN_NetTransportSSL__CertEntries2Db(nm, dbCert,
                                       NID_countryName,
                                       "countryName");
  GWEN_NetTransportSSL__CertEntries2Db(nm, dbCert,
                                       NID_localityName,
                                       "localityName");
  GWEN_NetTransportSSL__CertEntries2Db(nm, dbCert,
                                       NID_stateOrProvinceName,
                                       "stateOrProvinceName");

  d=X509_get_notBefore(cert);
  if (d) {
    if (GWEN_NetTransportSSL__ASN_UTC2Db(d, dbCert, "notBefore")) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error in notBefore date");
    }
  }

  d=X509_get_notAfter(cert);
  if (d) {
    if (GWEN_NetTransportSSL__ASN_UTC2Db(d, dbCert, "notAfter")) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error in notBefore date");
    }
  }

  pktmp=X509_get_pubkey(cert);
  if (pktmp) {
    RSA *kd;

    kd=EVP_PKEY_get1_RSA(pktmp);
    if (kd) {
      char buffer[256];
      int l;
      GWEN_DB_NODE *dbKey;
      GWEN_DB_NODE *dbKeyData;

      dbKey=GWEN_DB_GetGroup(dbCert, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                             "pubKey");

      assert(dbKey);
      dbKeyData=GWEN_DB_GetGroup(dbKey, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                                 "data");
      GWEN_DB_SetCharValue(dbKey,
                           GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "type", "RSA");

      GWEN_DB_SetIntValue(dbKeyData,
                          GWEN_DB_FLAGS_DEFAULT |
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "public", 1);
      if (kd->n) {
        l=BN_bn2bin(kd->n, (unsigned char*) &buffer);
        GWEN_DB_SetBinValue(dbKeyData,
                            GWEN_DB_FLAGS_DEFAULT |
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "n", buffer, l);
      }

      if (kd->e) {
        l=BN_bn2bin(kd->e, (unsigned char*) &buffer);
        GWEN_DB_SetBinValue(dbKeyData,
                            GWEN_DB_FLAGS_DEFAULT |
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "e", buffer, l);
      }
      RSA_free(kd);
    } /* if there is data for the public key */
    EVP_PKEY_free(pktmp);
  } /* if there is a pubkey */

  if (!X509_digest(cert, EVP_md5(), md, &md_size)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Error building fingerprint of the certificate");
  }
  if (!md_size) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Empty fingerprint of the certificate");
  }
  else {
    GWEN_BUFFER *dbuf;

    GWEN_DB_SetBinValue(dbCert,
			GWEN_DB_FLAGS_DEFAULT |
			GWEN_DB_FLAGS_OVERWRITE_VARS,
			"fingerprint", md, md_size);

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Text_ToHexBuffer(/* GCC4 pointer-signedness fix: */ (char*)md,
			      md_size, dbuf, 2, ':', 0)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Could not convert fingerprint to hex");
    }
    else {
      GWEN_DB_SetCharValue(dbCert,
			   GWEN_DB_FLAGS_DEFAULT |
			   GWEN_DB_FLAGS_OVERWRITE_VARS,
			   "HexFingerprint", GWEN_Buffer_GetStart(dbuf));
    }
    GWEN_Buffer_free(dbuf);
  }

  return dbCert;
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
  DBG_DEBUG(GWEN_LOGDOMAIN, "Working with status \"%s\" (%d)",
            GWEN_NetTransport_StatusName(st),
            st);
  switch(st) {
  case GWEN_NetTransportStatusPConnecting: {
    char addrBuffer[128];
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Still connecting");

    /* get socket error to check whether the connect succeeded */
    err=GWEN_Socket_GetSocketError(skd->socket);
    if (!GWEN_Error_IsOk(err)) {
      if (GWEN_Error_GetType(err)!=
          GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
          (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
           GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
        DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
        return GWEN_NetTransportWorkResult_Error;
      }
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Still not connected");
      return GWEN_NetTransportWorkResult_NoChange;
    }
    /* log address */
    GWEN_InetAddr_GetAddress(GWEN_NetTransport_GetPeerAddr(tr),
                             addrBuffer, sizeof(addrBuffer));
    DBG_INFO(GWEN_LOGDOMAIN, "Connection established with %s (port %d)",
             addrBuffer,
             GWEN_InetAddr_GetPort(GWEN_NetTransport_GetPeerAddr(tr)));
    /* set to "physically connected" */
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPConnected);
    DBG_INFO(GWEN_LOGDOMAIN, "Physical connection established");
    GWEN_NetTransport_MarkActivity(tr);
    return GWEN_NetTransportWorkResult_Change;
  }

  case GWEN_NetTransportStatusPConnected: {
    /* establish SSL */
    int fd;

    DBG_INFO(GWEN_LOGDOMAIN, "Physically connected, connecting logically");
    /* reset security */
    GWEN_DB_Group_free(skd->peerCertificate);
    skd->peerCertificate=0;
    skd->isSecure=0;

    /* get socket handle (I know, it's ugly, but the function below is
     * not exported to the outside) */
    fd=GWEN_Socket_GetSocketInt(skd->socket);
    if (fd==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "No socket handle, cannot use this socket with SSL");
      DBG_ERROR(GWEN_LOGDOMAIN, "Disabling connection");
      GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
      return GWEN_NetTransportWorkResult_Error;
    }

    /* socket ready now setup SSL */
    if (skd->active)
      skd->ssl_ctx=SSL_CTX_new(SSLv23_client_method());
    else
      skd->ssl_ctx=SSL_CTX_new(SSLv23_server_method());

    if (GWEN_NetTransportSSL__SetupSSL(tr, fd)) {
      GWEN_Socket_Close(skd->socket);
      SSL_free(skd->ssl);
      skd->ssl=0;
      SSL_CTX_free(skd->ssl_ctx);
      skd->ssl_ctx=0;
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup SSL conntection");
      DBG_ERROR(GWEN_LOGDOMAIN, "Disabling connection");
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
    ERR_clear_error();
    if (skd->active) {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Calling connect");
      rv=SSL_connect(skd->ssl);
    }
    else {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Calling accept");
      rv=SSL_accept(skd->ssl);
    }

    if (rv!=1) {
      int sslerr;

      sslerr=SSL_get_error(skd->ssl, rv);
      if (sslerr!=SSL_ERROR_WANT_READ &&
	  sslerr!=SSL_ERROR_WANT_WRITE) {
	if (sslerr==SSL_ERROR_SYSCALL && errno==0) {
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "SSL: Syscall error flagged, but errno is 0...");
        }
        else {
          DBG_ERROR(GWEN_LOGDOMAIN, "SSL error: %s (%d)",
                    GWEN_NetTransportSSL_ErrorString(sslerr),
                    sslerr);
          ERR_print_errors_fp(stderr);
        }

	GWEN_Socket_Close(skd->socket);
	SSL_free(skd->ssl);
	skd->ssl=0;
	SSL_CTX_free(skd->ssl_ctx);
        skd->ssl_ctx=0;
        DBG_ERROR(GWEN_LOGDOMAIN, "Disabling connection");
	GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
	return GWEN_NetTransportWorkResult_Error;
      }
      else {
          return GWEN_NetTransportWorkResult_NoChange;
      }
    }

    /* now logically connected */
    DBG_INFO(GWEN_LOGDOMAIN, "Logically connected");
    GWEN_DB_Group_free(skd->peerCertificate);
    skd->peerCertificate=0;

    /* show info about used cipher */
    if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevelNotice) {
      SSL_CIPHER *ci;
      char buffer[256];
      const char *p;

      ci=SSL_get_current_cipher(skd->ssl);
      assert(ci);

      p=SSL_CIPHER_description(ci, buffer, sizeof(buffer));
      DBG_INFO(GWEN_LOGDOMAIN, "Connected using \"%s\"", p);
    }

    cert=SSL_get_peer_certificate(skd->ssl);
    if (!cert) {
      if (skd->secure) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Peer did not send a certificate, abort");
        GWEN_Socket_Close(skd->socket);
        SSL_free(skd->ssl);
        skd->ssl=0;
        SSL_CTX_free(skd->ssl_ctx);
        skd->ssl_ctx=0;
        DBG_ERROR(GWEN_LOGDOMAIN, "Disabling connection");
        GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
        return GWEN_NetTransportWorkResult_Error;
      }
      else {
        DBG_WARN(GWEN_LOGDOMAIN, "Peer did not send a certificate");
      }
    }
    else {
      char *certbuf;
      long vr;
      GWEN_DB_NODE *dbCert;
      const char *s;
      X509_NAME *nm;
      unsigned long hash;
      char numbuf[16];
      int isNew;
      int isError;
      int isWarning;
      GWEN_INETADDRESS *peerAddr;
      GWEN_ERRORCODE err;
      GWEN_NETTRANSPORTSSL_ASKADDCERT_RESULT res;
      int isErr;

      certbuf=X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);

      DBG_INFO(GWEN_LOGDOMAIN, "Got a certificate: %s",
               certbuf);

      /* setup certificate */
      isNew=0;
      isError=0;
      isWarning=0;
      skd->peerCertificate=GWEN_NetTransportSSL__Cert2Db(cert);
      dbCert=skd->peerCertificate;

      err=GWEN_Socket_GetPeerAddr(skd->socket, &peerAddr);
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
      }
      else {
        char addrBuffer[256];

        err=GWEN_InetAddr_GetAddress(peerAddr, addrBuffer,
                                     sizeof(addrBuffer)-1);
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
        }
        else {
          GWEN_DB_SetCharValue(dbCert, GWEN_DB_FLAGS_DEFAULT,
                               "ipaddr", addrBuffer);
        }
        GWEN_InetAddr_free(peerAddr);
      }

      /* setup statusText and statusCode */
      vr=SSL_get_verify_result(skd->ssl);
      switch(vr) {
      case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
        s=I18N("New certificate");
        isNew=1;
        break;
      case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
        s=I18N("Unable to get issuer certificate");
        isWarning=1;
        break;
      case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
        s=I18N("Unable to decrypt cert signature");
        isError=1;
        break;
      case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
        s=I18N("Unable to decode issuer public key");
        isError=1;
        break;
      case X509_V_ERR_CERT_SIGNATURE_FAILURE:
        s=I18N("Cert signature failure");
        isError=1;
        break;
      case X509_V_ERR_CERT_NOT_YET_VALID:
        s=I18N("Cert not yet valid");
        isError=1;
        break;
      case X509_V_ERR_CERT_HAS_EXPIRED:
        s=I18N("Cert has expired");
        isError=1;
        break;
      case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        s=I18N("Self-signed root cert");
        isWarning=1;
        break;
      case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
        s=I18N("Self-signed cert");
        isWarning=1;
        break;
      case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
        s=I18N("Unable to verify leaf signature");
        isError=1;
        break;
      case X509_V_ERR_CERT_CHAIN_TOO_LONG:
        s=I18N("Cert chain too long");
        isError=1;
        break;
      case X509_V_ERR_CERT_REVOKED:
        s=I18N("Cert revoked");
        isError=1;
        break;
      case X509_V_ERR_INVALID_CA:
        s=I18N("Invalid CA");
        isError=1;
        break;
      case X509_V_ERR_CERT_UNTRUSTED:
        s=I18N("Cert untrusted");
        isError=1;
        break;
      case X509_V_ERR_CERT_REJECTED:
        s=I18N("Cert rejected");
        isError=1;
        break;

      case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
      case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
      case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
      case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
      case X509_V_ERR_PATH_LENGTH_EXCEEDED:
      case X509_V_ERR_INVALID_PURPOSE:
        s=I18N("Formal error in certificate");
        isError=1;
        break;

      case X509_V_ERR_OUT_OF_MEM:
        s=I18N("Out of memory");
        isError=1;
        break;
      case X509_V_ERR_UNABLE_TO_GET_CRL:
        s=I18N("Unable to get CRL");
        isError=1;
        break;
      case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
        s=I18N("Unable to decrypt CRL signature");
        isError=1;
        break;
      case X509_V_ERR_CRL_SIGNATURE_FAILURE:
        s=I18N("CRL signature failure");
        isError=1;
        break;
      case X509_V_ERR_CRL_NOT_YET_VALID:
        s=I18N("CRL not yet valid");
        isError=1;
        break;
      case X509_V_ERR_CRL_HAS_EXPIRED:
        s=I18N("CRL has expired");
        isError=1;
        break;
      case X509_V_OK:
        s=I18N("Certificate is valid");
        break;
      default:
        s=I18N("Unknown SSL error");
        isError=1;
      }
      GWEN_DB_SetCharValue(dbCert, GWEN_DB_FLAGS_DEFAULT,
                           "statusText", s);
      nm=X509_get_subject_name(cert);
      if (nm) {
        hash=X509_NAME_hash(nm);
        snprintf(numbuf, sizeof(numbuf), "%08lx", hash);
        GWEN_DB_SetCharValue(dbCert, GWEN_DB_FLAGS_DEFAULT,
                             "hashValue", numbuf);
      }
      GWEN_DB_SetIntValue(dbCert, GWEN_DB_FLAGS_DEFAULT,
                          "isNew", isNew);
      GWEN_DB_SetIntValue(dbCert, GWEN_DB_FLAGS_DEFAULT,
                          "isError", isError);
      GWEN_DB_SetIntValue(dbCert, GWEN_DB_FLAGS_DEFAULT,
                          "isWarning", isWarning);


      if (vr==X509_V_OK && skd->secure) {
	DBG_ERROR(GWEN_LOGDOMAIN,
		  "Invalid peer certificate in secure mode, aborting");
	GWEN_Socket_Close(skd->socket);
	SSL_free(skd->ssl);
	skd->ssl=0;
	SSL_CTX_free(skd->ssl_ctx);
	skd->ssl_ctx=0;
	free(certbuf);
	X509_free(cert);
        DBG_ERROR(GWEN_LOGDOMAIN, "Disabling connection");
	GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
	return GWEN_NetTransportWorkResult_Error;
      }

      /* ask user */
      isErr=0;
      DBG_INFO(GWEN_LOGDOMAIN,
	       "Unknown certificate \"%s\", asking user",
	       certbuf);
      res=GWEN_NetTransportSSL__AskAddCert(tr, dbCert);
      switch(res) {
      case GWEN_NetTransportSSL_AskAddCertResultError:
	DBG_ERROR(GWEN_LOGDOMAIN, "Error asking user");
	isErr=1;
	break;
      case GWEN_NetTransportSSL_AskAddCertResultNo:
	DBG_ERROR(GWEN_LOGDOMAIN, "User doesn't trust the certificate");
	isErr=1;
	break;
      case GWEN_NetTransportSSL_AskAddCertResultTmp:
	DBG_INFO(GWEN_LOGDOMAIN, "Temporarily trusting certificate");
	break;
      case GWEN_NetTransportSSL_AskAddCertResultPerm:
	DBG_NOTICE(GWEN_LOGDOMAIN, "Adding certificate to trusted certs");
	if (GWEN_NetTransportSSL__SaveCert(tr, cert, skd->CAdir, !isNew)) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Error saving certificate");
	  isErr=1;
	}
	break;
      case GWEN_NetTransportSSL_AskAddCertResultIncoming:
	if (!skd->newCAdir) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "No dir for incoming connections given");
	  isErr=1;
	}
	else {
	  DBG_NOTICE(GWEN_LOGDOMAIN, "Adding certificate to incoming certs");
	  if (GWEN_NetTransportSSL__SaveCert(tr, cert, skd->newCAdir, 1)) {
	    DBG_ERROR(GWEN_LOGDOMAIN, "Error saving certificate");
	    isErr=1;
	  }
	}
	break;
      default:
	DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected result");
	break;
      } /* switch */

      if (isErr) {
	GWEN_Socket_Close(skd->socket);
	SSL_free(skd->ssl);
	skd->ssl=0;
	SSL_CTX_free(skd->ssl_ctx);
	skd->ssl_ctx=0;
	free(certbuf);
	X509_free(cert);

        DBG_ERROR(GWEN_LOGDOMAIN, "Disabling connection");
        GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
	return GWEN_NetTransportWorkResult_Error;
      }

      if (vr==X509_V_OK) {
	DBG_INFO(GWEN_LOGDOMAIN, "Secure connection with peer \"%s\"",
		 GWEN_DB_GetCharValue(skd->peerCertificate,
				      "commonName", 0,
				      "<nobody>"));
	skd->isSecure=1;
      }
      free(certbuf);
      X509_free(cert);
    }

    DBG_INFO(GWEN_LOGDOMAIN, "SSL connection established (%s)",
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

    DBG_VERBOUS(GWEN_LOGDOMAIN, "Listening");
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
          DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
          return GWEN_NetTransportWorkResult_Error;
        }
        /* otherwise there simply is no waiting connection */
        DBG_DEBUG(GWEN_LOGDOMAIN, "No incoming connection");
        return GWEN_NetTransportWorkResult_NoChange;
      }

      /* we have an incoming connection */
      GWEN_InetAddr_GetAddress(iaddr, addrBuffer, sizeof(addrBuffer));
      DBG_INFO(GWEN_LOGDOMAIN, "Incoming connection from %s (port %d)",
               addrBuffer, GWEN_InetAddr_GetPort(iaddr));

      /* set socket nonblocking */
      err=GWEN_Socket_SetBlocking(newS, 0);
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
        GWEN_InetAddr_free(iaddr);
        GWEN_Socket_free(newS);
        return GWEN_NetTransportWorkResult_Error;
      }

      /* create new transport layer, let it take over the new socket */
      newTr=GWEN_NetTransportSSL_new(newS,
                                     skd->CAdir,
                                     skd->newCAdir,
				     skd->ownCertFile,
                                     skd->dhfile,
                                     skd->secure,
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
      GWEN_NetTransport_MarkActivity(tr);
      GWEN_NetTransport_AddNextIncoming(tr, newTr);
      return GWEN_NetTransportWorkResult_Change;
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "Too many incoming connections waiting");
    }
    GWEN_NetTransport_MarkActivity(tr);
    break;
  }

  case GWEN_NetTransportStatusLConnected:
    DBG_DEBUG(GWEN_LOGDOMAIN, "Active connection, nothing to do");
    /* TODO: check whether read/write are possible, return code
     * accordingly */
    break;

  case GWEN_NetTransportStatusLDisconnecting: {
    rv=SSL_shutdown(skd->ssl);
    if (rv==1 || rv==-1) {
      /* connection closed */
      DBG_INFO(GWEN_LOGDOMAIN, "Connection closed");
      GWEN_Socket_Close(skd->socket);
      SSL_free(skd->ssl);
      skd->ssl=0;
      SSL_CTX_free(skd->ssl_ctx);
      skd->ssl_ctx=0;
      GWEN_NetTransport_MarkActivity(tr);
      GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPDisconnected);
      return GWEN_NetTransportWorkResult_Change;
    }
    else {
      int sslerr;

      /* check for timeout (maximum: 5 seconds) */
      if (GWEN_NetTransport_GetIdleTime(tr)>5) {
	DBG_NOTICE(GWEN_LOGDOMAIN, "Forcing connection close");
	GWEN_Socket_Close(skd->socket);
	SSL_free(skd->ssl);
	skd->ssl=0;
	SSL_CTX_free(skd->ssl_ctx);
	skd->ssl_ctx=0;
	GWEN_NetTransport_MarkActivity(tr);
	GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPDisconnected);
	return GWEN_NetTransportWorkResult_Change;
      }

      /* check for error and act accordingly */
      sslerr=SSL_get_error(skd->ssl, rv);
      if (sslerr==SSL_ERROR_WANT_READ)
	return GWEN_NetTransportResultWantRead;
      else if (sslerr==SSL_ERROR_WANT_WRITE)
	return GWEN_NetTransportResultWantWrite;
      else {
	if (sslerr==SSL_ERROR_SYSCALL && errno==0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "Connection just went down (%d: %s)",
		   sslerr,
		   GWEN_NetTransportSSL_ErrorString(sslerr));
	}
	else {
	  if (sslerr==SSL_ERROR_ZERO_RETURN) {
	    DBG_INFO(GWEN_LOGDOMAIN, "Connection closed");
	  }
	  else {
	    DBG_ERROR(GWEN_LOGDOMAIN, "List of pending SSL errors:");
	    ERR_print_errors_fp(stderr); /* DEBUG */
	    DBG_ERROR(GWEN_LOGDOMAIN, "SSL error: %s (%d)",
		      GWEN_NetTransportSSL_ErrorString(sslerr),
		      sslerr);
	  }
	}

	GWEN_Socket_Close(skd->socket);
	SSL_free(skd->ssl);
	skd->ssl=0;
	SSL_CTX_free(skd->ssl_ctx);
	skd->ssl_ctx=0;
  
	/* connection closed */
	GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPDisconnected);
	GWEN_NetTransport_MarkActivity(tr);
	return GWEN_NetTransportResultOk;
      }
    }
    break;
  }

  case GWEN_NetTransportStatusUnconnected:
  case GWEN_NetTransportStatusDisabled:
  case GWEN_NetTransportStatusPDisconnecting:
  case GWEN_NetTransportStatusPDisconnected:
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Inactive connection (status \"%s\")",
                GWEN_NetTransport_StatusName(st));
    break;

  default:
    DBG_WARN(GWEN_LOGDOMAIN, "Hmm, status \"%s\" (%d) is unexpected...",
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



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetTransportSSL_SetCipherList(GWEN_NETTRANSPORT *tr,
                                        const char *ciphers){
  GWEN_NETTRANSPORTSSL *skd;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);
  assert(skd);

  free(skd->cipherList);
  if (ciphers)
    skd->cipherList=strdup(ciphers);
  else
    skd->cipherList=0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_DB_NODE *GWEN_NetTransportSSL_GetCipherList(){
  STACK_OF(SSL_CIPHER) *ciphers;
  SSL *ssl;
  SSL_CTX *ctx;
  const char *p;

  ctx=SSL_CTX_new(SSLv23_client_method());
  ssl=SSL_new(ctx);

  ciphers=(STACK_OF(SSL_CIPHER)*)SSL_get_ciphers(ssl);
  if (ciphers) {
    GWEN_DB_NODE *dbCiphers;
    SSL_CIPHER *curr;
    int n;
    char buffer[256];

    dbCiphers=GWEN_DB_Group_new("ciphers");
    for (n=0;n<sk_SSL_CIPHER_num(ciphers) ; n++) {
      curr=sk_SSL_CIPHER_value(ciphers, n);
      p=SSL_CIPHER_get_name(curr);
      if (p) {
        GWEN_DB_NODE *dbCurr;

        dbCurr=GWEN_DB_GetGroup(dbCiphers, GWEN_PATH_FLAGS_CREATE_GROUP,
                                "cipher");
        GWEN_DB_SetCharValue(dbCurr, GWEN_DB_FLAGS_DEFAULT,
                             "name", p);
        GWEN_DB_SetIntValue(dbCurr, GWEN_DB_FLAGS_DEFAULT,
                            "bits", SSL_CIPHER_get_bits(curr, 0));
        p=SSL_CIPHER_get_version(curr);
        if (p)
          GWEN_DB_SetCharValue(dbCurr, GWEN_DB_FLAGS_DEFAULT,
                               "version", p);
        p=SSL_CIPHER_description(curr, buffer, sizeof(buffer));
        if (p)
          GWEN_DB_SetCharValue(dbCurr, GWEN_DB_FLAGS_DEFAULT,
                               "description", p);
      } /* if cipher name */
    } /* for */
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return dbCiphers;
  } /* if ciphers */
  else {
    DBG_WARN(GWEN_LOGDOMAIN, "No ciphers");
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return 0;
  }
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSSL_GenerateCertAndKeyFile(const char *fname,
                                                int bits,
                                                int serial,
                                                int days,
                                                GWEN_DB_NODE *db) {
  X509 *x;
  EVP_PKEY *pk;
  RSA *rsa;
  X509_NAME *name=NULL;
  X509_NAME_ENTRY *ne=NULL;
  FILE *f;
  const char *p;

  X509V3_add_standard_extensions();

  pk=EVP_PKEY_new();
  if (!pk){
    fprintf(stderr, "Could not create RSA key\n");
    EVP_PKEY_free(pk);
    X509V3_EXT_cleanup();
    return -1;
  }

  x=X509_new();
  if (!x) {
    fprintf(stderr, "Could not create certificate\n");
    EVP_PKEY_free(pk);
    X509V3_EXT_cleanup();
    return -1;
  }

  rsa=RSA_generate_key(bits, RSA_F4, NULL, NULL);
  if (!EVP_PKEY_assign_RSA(pk, rsa)){
    fprintf(stderr, "Could not assign RSA key\n");
    X509_free(x);
    EVP_PKEY_free(pk);
    RSA_free(rsa);
    X509V3_EXT_cleanup();
    return -1;
  }
  rsa=NULL;

  X509_set_version(x,3);
  ASN1_INTEGER_set(X509_get_serialNumber(x),serial);
  X509_gmtime_adj(X509_get_notBefore(x),0);
  if (!days)
    days=GWEN_NETTRANSPORTSSL_DEFAULT_CERT_DAYS;
  X509_gmtime_adj(X509_get_notAfter(x),(long)60*60*24*days);
  X509_set_pubkey(x, pk);

  name=X509_NAME_new();

  p=GWEN_DB_GetCharValue(db, "countryName", 0, "DE");
  ne=X509_NAME_ENTRY_create_by_NID(NULL,
                                   NID_countryName,
                                   V_ASN1_APP_CHOOSE,
                                   (unsigned char*)p,
                                   -1);
  X509_NAME_add_entry(name, ne, 0, 0);

  p=GWEN_DB_GetCharValue(db, "commonName", 0, 0);
  if (p) {
    X509_NAME_ENTRY_create_by_NID(&ne, NID_commonName,
                                  V_ASN1_APP_CHOOSE,
                                  (unsigned char*)p,
                                  -1);
    X509_NAME_add_entry(name, ne, -1, 0);
  }

  p=GWEN_DB_GetCharValue(db, "organizationName", 0, 0);
  if (p) {
    X509_NAME_ENTRY_create_by_NID(&ne, NID_organizationName,
                                  V_ASN1_APP_CHOOSE,
                                  (unsigned char*)p,
                                  -1);
    X509_NAME_add_entry(name, ne, -1, 0);
  }

  p=GWEN_DB_GetCharValue(db, "organizationalUnitName", 0, 0);
  if (p) {
    X509_NAME_ENTRY_create_by_NID(&ne, NID_organizationalUnitName,
                                  V_ASN1_APP_CHOOSE,
                                  (unsigned char*)p,
                                  -1);
    X509_NAME_add_entry(name, ne, -1, 0);
  }

  p=GWEN_DB_GetCharValue(db, "localityName", 0, 0);
  if (p) {
    X509_NAME_ENTRY_create_by_NID(&ne, NID_localityName,
                                  V_ASN1_APP_CHOOSE,
                                  (unsigned char*)p,
                                  -1);
    X509_NAME_add_entry(name, ne, -1, 0);
  }

  p=GWEN_DB_GetCharValue(db, "stateOrProvinceName", 0, 0);
  if (p) {
    X509_NAME_ENTRY_create_by_NID(&ne, NID_stateOrProvinceName,
                                  V_ASN1_APP_CHOOSE,
                                  (unsigned char*)p,
                                  -1);
    X509_NAME_add_entry(name, ne, -1, 0);
  }

  X509_set_subject_name(x, name);
  X509_set_issuer_name(x, name);

  /* finished with structure */
  X509_NAME_ENTRY_free(ne);
  X509_NAME_free(name);

  if (!X509_sign(x, pk, EVP_md5())) {
    fprintf(stderr, "Could not sign\n");
    X509_free(x);
    EVP_PKEY_free(pk);
    X509V3_EXT_cleanup();
    return -1;
  }

  /* save key */
  f=fopen(fname, "w+");
  if (!f) {
    fprintf(stderr, "Could not save private key\n");
    X509_free(x);
    EVP_PKEY_free(pk);
    X509V3_EXT_cleanup();
    return -1;
  }
  PEM_write_RSAPrivateKey(f,
                          pk->pkey.rsa,
                          NULL,
                          NULL,
                          0,
                          NULL,
                          NULL);
  /* save cert */
  PEM_write_X509(f, x);
  if (fclose(f)) {
    fprintf(stderr, "Could not close file\n");
    X509_free(x);
    EVP_PKEY_free(pk);
    X509V3_EXT_cleanup();
    return -1;
  }
  X509_free(x);
  EVP_PKEY_free(pk);
  X509V3_EXT_cleanup();

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetTransportSSL__GenerateDhFile_Callback(int i, int j, void *p) {
  GWEN_WAITCALLBACK_RESULT res;

  switch(i) {
  case 0:
    DBG_DEBUG(GWEN_LOGDOMAIN, "Generated %d. potential prime number", j);
    break;
  case 1:
    DBG_DEBUG(GWEN_LOGDOMAIN, "Testing %d. prime number", j);
    break;
  case 2:
    DBG_DEBUG(GWEN_LOGDOMAIN, "Prime found in %d. try", j);
    break;
  } /* switch */

  res=GWEN_WaitCallback();
  if (res!=GWEN_WaitCallbackResult_Continue) {
    DBG_WARN(GWEN_LOGDOMAIN, "User wants to abort, but this function can not be aborted");
  }
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSSL_GenerateDhFile(const char *fname, int bits) {
  DH *dh;
  FILE *f;

#ifdef GWEN_RANDOM_DEVICE
  if (!RAND_load_file(GWEN_RANDOM_DEVICE, 40)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not seed random (maybe \"%s\" is missing?)",
	      GWEN_RANDOM_DEVICE);
    return -1;
  }
#endif
  dh=DH_generate_parameters(bits,
			    2,
			    GWEN_NetTransportSSL__GenerateDhFile_Callback,
			    0);
  if (!dh) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not generate DH parameters");
    return -1;
  }

  f=fopen(fname, "w+");
  if (!f) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(%s): %s", fname, strerror(errno));
    DH_free(dh);
    return -1;
  }

  if (!PEM_write_DHparams(f, dh)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not write DH params");
    fclose(f);
    DH_free(dh);
    return -1;
  }

  if (fclose(f)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fclose(%s): %s", fname, strerror(errno));
    DH_free(dh);
    return -1;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "DH params generated and written");
  DH_free(dh);
  return 0;
}






















