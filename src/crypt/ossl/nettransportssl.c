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
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/inetsocket_l.h>

#include <openssl/pem.h>
#include <openssl/err.h>

#include <string.h>
#include <errno.h>
#include <sys/socket.h>



GWEN_INHERIT(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL);


static GWEN_NETTRANSPORTSSL_GETPASSWD_FN
  gwen_netransportssl_getPasswordFn=0;

static GWEN_NETTRANSPORTSSL_ASKADDCERT_FN
  gwen_netransportssl_askAddCertFn=0;



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
    free(skd->CAdir);
    free(skd->ownCertFile);
    free(skd->dhfile);
    free(skd->cipherList);
    free(skd);
  }
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT *GWEN_NetTransportSSL_new(GWEN_SOCKET *sk,
                                            const char *capath,
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
  if (!rv) {
    /* send a TCP_FIN to trigger the other side's close_notify */
    shutdown(GWEN_Socket_GetSocketInt(skd->socket), 1);
    rv=SSL_shutdown(skd->ssl);
  }
  if (rv==1 || rv==-1) {
    /* connection closed */
    DBG_INFO(0, "Connection closed");
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
      GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
      GWEN_NetTransport_MarkActivity(tr);
      return GWEN_NetTransportResultError;
    }
  }

  DBG_DEBUG(0, "Read %d bytes:", rv);
  GWEN_Text_LogString(buffer, rv, 0, GWEN_LoggerLevelVerbous);
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
      GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
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
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
    GWEN_NetTransport_MarkActivity(tr);
    return GWEN_NetTransportResultError;
  }

  DBG_DEBUG(0, "Written %d bytes:", rv);
  GWEN_Text_LogString(buffer, rv, 0, GWEN_LoggerLevelVerbous);
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
int GWEN_NetTransportSSL__Check_Cert(GWEN_NETTRANSPORTSSL *skd,
                                     const char *name){
  X509 *peer;
  char cn[256];

  if (SSL_get_verify_result(skd->ssl)!=X509_V_OK) {
    DBG_ERROR(0, "Invalid certificate");
    return -1;
  }
  /* check common name */
  peer=SSL_get_peer_certificate(skd->ssl);
  X509_NAME_get_text_by_NID(X509_get_subject_name(peer),
                            NID_commonName, cn, sizeof(cn));
  if(strcasecmp(cn, name)) {
    DBG_WARN(0, "Common name does not match (\"%s\" != \"%s\")",
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
    DBG_WARN(0, "No getPasswordFn set");
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
  DBG_NOTICE(0, "Would ask user about this:");
  GWEN_DB_Dump(cert, stderr, 2);
  return GWEN_NetTransportSSL_AskAddCertResultPerm;

  if (gwen_netransportssl_askAddCertFn)
    return gwen_netransportssl_askAddCertFn(tr, cert);
  else {
    DBG_WARN(0, "No askAddCert function set");
    return GWEN_NetTransportSSL_AskAddCertResultNo;
  }
}



/* -------------------------------------------------------------- FUNCTION */
void
GWEN_NetTransportSSL_SetAskAddCertFn(GWEN_NETTRANSPORTSSL_ASKADDCERT_FN fn){
  gwen_netransportssl_askAddCertFn=fn;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORTSSL_ASKADDCERT_FN GWEN_NetTransportSSL_GetAskAddCertFn(){
  return gwen_netransportssl_askAddCertFn;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSSL__SaveCert(GWEN_NETTRANSPORT *tr,
                                   X509 *cert) {
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

  if (skd->CAdir) {
    GWEN_BUFFER *nbuf;
    unsigned long hash;
    char numbuf[32];
    int i;
    GWEN_TYPE_UINT32 pos;

    hash=X509_NAME_hash(nm);
    snprintf(numbuf, sizeof(numbuf), "%08lx", hash);
    nbuf=GWEN_Buffer_new(0, 128, 0, 1);
    GWEN_Buffer_AppendString(nbuf, skd->CAdir);
    /* check path, create it if necessary */
    if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(nbuf),
                               GWEN_PATH_FLAGS_CHECKROOT)) {
      DBG_ERROR(0, "Invalid path (\"%s\")", GWEN_Buffer_GetStart(nbuf));
      GWEN_Buffer_free(nbuf);
      return -1;
    }
    GWEN_Buffer_AppendByte(nbuf, '/');
    GWEN_Buffer_AppendString(nbuf, numbuf);
    pos=GWEN_Buffer_GetPos(nbuf);
    for (i=0; i<GWEN_NETTRANSPORTSSL_MAXCOLL; i++) {
      snprintf(numbuf, sizeof(numbuf), "%d", i);
      GWEN_Buffer_SetUsedBytes(nbuf, pos);
      GWEN_Buffer_SetPos(nbuf, pos);
      GWEN_Buffer_AppendByte(nbuf, '.');
      GWEN_Buffer_AppendString(nbuf, numbuf);
      if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(nbuf),
                                 GWEN_PATH_FLAGS_NAMEMUSTEXIST|
                                 GWEN_PATH_FLAGS_VARIABLE|
                                 GWEN_PATH_FLAGS_CHECKROOT)) {
        break;
      }
    }
    if (i>=GWEN_NETTRANSPORTSSL_MAXCOLL) {
      DBG_ERROR(0, "Maximum number of hash collisions reached!");
      GWEN_Buffer_free(nbuf);
      return -1;
    }

    DBG_DEBUG(0, "Saving file as \"%s\"", GWEN_Buffer_GetStart(nbuf));
    f=fopen(GWEN_Buffer_GetStart(nbuf), "w+");
    if (!f) {
      DBG_ERROR(0, "fopen(\"%s\", \"%s\"): %s",
                GWEN_Buffer_GetStart(nbuf), fmode, strerror(errno));
      GWEN_Buffer_free(nbuf);
      return -1;
    }
    GWEN_Buffer_free(nbuf);
  }
  else {
    DBG_ERROR(0, "Don't know where to save the file...");
    return -1;
  }

  if (!PEM_write_X509(f, cert)) {
    DBG_ERROR(0, "Could not save certificate of \"%s\"", cn);
    return 0;
  }

  DBG_INFO(0, "Certificate of \"%s\" added", cn);

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSSL__SetupSSL(GWEN_NETTRANSPORT *tr, int fd){
  int rv = 0;
  GWEN_NETTRANSPORTSSL *skd;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSSL, tr);
  assert(skd);

  /* set default password handler and data */
  SSL_CTX_set_default_passwd_cb(skd->ssl_ctx,
                                GWEN_NetTransportSSL_PasswordCB);
  SSL_CTX_set_default_passwd_cb_userdata(skd->ssl_ctx,
                                         tr);

  if (skd->ownCertFile) {
    /* load own certificate file */
    DBG_NOTICE(0, "Loading certificate and keys");
    if (!(SSL_CTX_use_certificate_chain_file(skd->ssl_ctx,
                                             skd->ownCertFile))){
      int sslerr;

      sslerr=SSL_get_error(skd->ssl, rv);
      DBG_ERROR(0, "SSL error reading certfile: %s (%d)",
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
      DBG_ERROR(0, "SSL error reading keyfile: %s (%d)",
                GWEN_NetTransportSSL_ErrorString(sslerr),
                sslerr);
      return -1;
    }
    if (!SSL_CTX_check_private_key(skd->ssl_ctx)) {
      DBG_ERROR(0, "Private key does not match the certificate public key");
      return -1;
    }

  }

  /* setup locations of certificates */
  if (skd->CAdir) {
    DBG_NOTICE(0, "Loading certificate locations");
    rv=SSL_CTX_load_verify_locations(skd->ssl_ctx, 0, skd->CAdir);
    if (rv==0) {
      DBG_ERROR(0, "SSL: Could not load certificate location");
      return -1;
    }
  }


  //SSL_CTX_set_verify_depth(skd->ssl_ctx, 1);

  /* setup server stuff */
  if (!skd->active &&
      GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusListening) {
    FILE *f;

    DBG_NOTICE(0, "Loading DH params");

    f=fopen(skd->dhfile, "r");
    if (!f) {
      DBG_ERROR(0, "SSL: fopen(%s): %s", skd->dhfile, strerror(errno));
      return -1;
    }
    else {
      DH *dh_tmp;

      dh_tmp=PEM_read_DHparams(f, NULL, NULL, NULL);
      fclose(f);
      if (dh_tmp==0) {
	DBG_ERROR(0, "SSL: Error reading DH params");
        return -1;
      }
      if (SSL_CTX_set_tmp_dh(skd->ssl_ctx, dh_tmp)<0) {
	DBG_ERROR(0, "SSL: Could not set DH params");
	DH_free(dh_tmp);
	return -1;
      }

      /* always expect peer certificate */
      if (skd->secure)
        SSL_CTX_set_verify(skd->ssl_ctx,
                           SSL_VERIFY_PEER |
                           SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                           0);
      else
        SSL_CTX_set_verify(skd->ssl_ctx,
                           SSL_VERIFY_PEER,
                           0);
    }
  }
  else {
    if (skd->secure)
      SSL_CTX_set_verify(skd->ssl_ctx,
                         SSL_VERIFY_PEER,
                         0);
    else
      SSL_CTX_set_verify(skd->ssl_ctx,
                         SSL_VERIFY_NONE,
                         0);
  }

  skd->ssl=SSL_new(skd->ssl_ctx);

  /* tell SSL to use our socket */
  rv=SSL_set_fd(skd->ssl, fd);
  if (rv==0) {
    int sslerr;

    sslerr=SSL_get_error(skd->ssl, rv);
    DBG_ERROR(0, "SSL error setting socket: %s (%d)",
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
  const char *p;
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



/* -------------------------------------------------------------- FUNCTION */
GWEN_DB_NODE *GWEN_NetTransportSSL__Cert2Db(X509 *cert) {
  GWEN_DB_NODE *dbCert;
  X509_NAME *nm;

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

    /* get socket handle (I know, it's ugly, but the function below is
     * not exported to the outside) */
    fd=GWEN_Socket_GetSocketInt(skd->socket);
    if (fd==-1) {
      DBG_ERROR(0, "No socket handle, cannot use this socket with SSL");
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
      DBG_VERBOUS(0, "Calling connect");
      rv=SSL_connect(skd->ssl);
    }
    else {
      DBG_VERBOUS(0, "Calling accept");
      rv=SSL_accept(skd->ssl);
    }

    if (rv<1) {
      int sslerr;

      sslerr=SSL_get_error(skd->ssl, rv);
      if (sslerr!=SSL_ERROR_WANT_READ &&
	  sslerr!=SSL_ERROR_WANT_WRITE) {
	if (sslerr==SSL_ERROR_SYSCALL && errno==0) {
          DBG_NOTICE(0, "Accept postponed");
	  return GWEN_NetTransportWorkResult_NoChange;
	}
	DBG_ERROR(0, "SSL error: %s (%d)",
		  GWEN_NetTransportSSL_ErrorString(sslerr),
		  sslerr);
	ERR_print_errors_fp(stderr);

	GWEN_Socket_Close(skd->socket);
	SSL_free(skd->ssl);
	skd->ssl=0;
	SSL_CTX_free(skd->ssl_ctx);
	skd->ssl_ctx=0;
	GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
	return GWEN_NetTransportWorkResult_Error;
      }
      else {
          return GWEN_NetTransportWorkResult_NoChange;
      }
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
        GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
        return GWEN_NetTransportWorkResult_Error;
      }
      else {
        DBG_WARN(0, "Peer did not send a certificate");
      }
    }
    else {
      char *certbuf;
      long vr;

      certbuf=X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);

      DBG_INFO(0, "Got a certificate: %s",
               certbuf);
      vr=SSL_get_verify_result(skd->ssl);
      if (vr==X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY ||
          vr==X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT ||
	  vr==X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN) {
	GWEN_NETTRANSPORTSSL_ASKADDCERT_RESULT res;
	int isErr;
	GWEN_DB_NODE *dbCert;

	/* setup certificate */
        dbCert=GWEN_NetTransportSSL__Cert2Db(cert);

        /* ask user */
        isErr=0;
        DBG_INFO(0, "Unknown certificate \"%s\", asking user", certbuf);
        res=GWEN_NetTransportSSL__AskAddCert(tr, dbCert);
        switch(res) {
        case GWEN_NetTransportSSL_AskAddCertResultError:
          DBG_ERROR(0, "Error asking user");
          isErr=1;
          break;
        case GWEN_NetTransportSSL_AskAddCertResultNo:
          if (skd->secure) {
            DBG_ERROR(0, "User doesn't trust the certificate");
            isErr=1;
          }
          break;
        case GWEN_NetTransportSSL_AskAddCertResultTmp:
          DBG_INFO(0, "Temporarily trusting certificate");
          break;
        case GWEN_NetTransportSSL_AskAddCertResultPerm:
          DBG_NOTICE(0, "Adding certificate to trusted certs");
          if (GWEN_NetTransportSSL__SaveCert(tr, cert)) {
            DBG_ERROR(0, "Error saving certificate");
            isErr=1;
          }
          break;
        default:
          DBG_ERROR(0, "Unexpected result");
          break;
        } /* switch */

        if (isErr) {
          GWEN_Socket_Close(skd->socket);
          SSL_free(skd->ssl);
          skd->ssl=0;
          SSL_CTX_free(skd->ssl_ctx);
          skd->ssl_ctx=0;
          GWEN_DB_Group_free(dbCert);
          free(certbuf);
          X509_free(cert);
          GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
          return GWEN_NetTransportWorkResult_Error;
        }

        GWEN_DB_Group_free(dbCert);
      }
      else if (vr!=X509_V_OK) {
        if (skd->secure) {
          DBG_ERROR(0, "Invalid peer certificate, aborting");
          GWEN_Socket_Close(skd->socket);
          SSL_free(skd->ssl);
          skd->ssl=0;
          SSL_CTX_free(skd->ssl_ctx);
          skd->ssl_ctx=0;
          free(certbuf);
          X509_free(cert);
          GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusDisabled);
          return GWEN_NetTransportWorkResult_Error;
        }
        else {
          DBG_WARN(0, "Invalid peer certificate, ignoring");
        }
      }
      else {
        DBG_NOTICE(0, "Peer certificate is valid");
        /* TODO: check_cert? */
        skd->isSecure=1;
      }
      free(certbuf);
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
                                     skd->CAdir,
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
    DBG_WARN(0, "No ciphers");
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return 0;
  }
}




