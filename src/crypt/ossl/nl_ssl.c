/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Feb 09 2004
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


#include "nl_ssl_p.h"
#include "i18n_l.h"
#include "bio_netlayer_l.h"
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/text.h>

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/ssl_cipher.h>

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/dh.h>
#include <openssl/rand.h>

#include <openssl/conf.h>
#include <openssl/x509v3.h>

#include <errno.h>

#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif


GWEN_INHERIT(GWEN_NETLAYER, GWEN_NL_SSL)



static int gwen_netlayerssl_sslidx=-1;



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER *GWEN_NetLayerSsl_new(GWEN_NETLAYER *baseLayer,
                                    const char *capath,
                                    const char *newcapath,
                                    const char *ownCertFile,
                                    const char *dhFolder,
                                    int secure) {
  GWEN_NETLAYER *nl;
  GWEN_NL_SSL *nld;
  GWEN_BUFFER *tbuf;
  int rv;

  assert(baseLayer);
  nl=GWEN_NetLayer_new(GWEN_NL_SSL_NAME);
  GWEN_NEW_OBJECT(GWEN_NL_SSL, nld);

  GWEN_INHERIT_SETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl, nld,
                       GWEN_NetLayerSsl_FreeData);

  GWEN_NetLayer_SetBaseLayer(nl, baseLayer);
  GWEN_NetLayer_SetParentLayer(baseLayer, nl);

  GWEN_NetLayer_SetLocalAddr(nl, GWEN_NetLayer_GetLocalAddr(baseLayer));
  GWEN_NetLayer_SetPeerAddr(nl, GWEN_NetLayer_GetPeerAddr(baseLayer));

  /* set public ca file */
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_NetLayerSsl_GetPublicCaFile(tbuf);
  if (rv<0) {
    DBG_ERROR(0, "Could not get the path and name of the public cert file");
  }
  else {
    nld->CAfile=strdup(GWEN_Buffer_GetStart(tbuf));
  }
  GWEN_Buffer_free(tbuf);

  if (capath)
    nld->CAdir=strdup(capath);
  if (newcapath)
    nld->newCAdir=strdup(newcapath);
  if (ownCertFile)
    nld->ownCertFile=strdup(ownCertFile);
  if (dhFolder)
    nld->dhFolder=strdup(dhFolder);

  nld->secure=secure;

  GWEN_NetLayer_SetWorkFn(nl, GWEN_NetLayerSsl_Work);
  GWEN_NetLayer_SetReadFn(nl, GWEN_NetLayerSsl_Read);
  GWEN_NetLayer_SetWriteFn(nl, GWEN_NetLayerSsl_Write);
  GWEN_NetLayer_SetConnectFn(nl, GWEN_NetLayerSsl_Connect);
  GWEN_NetLayer_SetDisconnectFn(nl, GWEN_NetLayerSsl_Disconnect);
  GWEN_NetLayer_SetListenFn(nl, GWEN_NetLayerSsl_Listen);
  GWEN_NetLayer_SetAddSocketsFn(nl, GWEN_NetLayerSsl_AddSockets);

  if (gwen_netlayerssl_sslidx==-1) {
    gwen_netlayerssl_sslidx=SSL_get_ex_new_index(0, 0, 0, 0, 0);
    assert(gwen_netlayerssl_sslidx!=-1);
  }

  return nl;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayerSsl_FreeData(void *bp, void *p) {
  GWEN_NL_SSL *nld;

  nld=(GWEN_NL_SSL*) p;
  BIO_free(nld->bio);
  GWEN_FREE_OBJECT(nld);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSsl_GetPassword(char *buffer, int num, int rwflag,
                                 void *user_data) {
  return GWEN_NetLayer_GetPassword((GWEN_NETLAYER*)user_data,
                                   buffer, num, rwflag);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSsl_GetPublicCaFile(GWEN_BUFFER *pbuf) {
  GWEN_STRINGLIST *sl;
  int rv;

  sl=GWEN_PathManager_GetPaths(GWEN_PM_LIBNAME,
                               GWEN_PM_SYSCONFDIR);
  assert(sl);
  rv=GWEN_Directory_FindFileInPaths(sl,
                                    "gwen-public-ca.crt",
                                    pbuf);
  GWEN_StringList_free(sl);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayerSsl_SetMode(GWEN_NETLAYER *nl, GWEN_NETLAYER_SSL_MODE m) {
  GWEN_NL_SSL *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  nld->mode=m;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSsl_Setup(GWEN_NETLAYER *nl) {
  GWEN_NL_SSL *nld;
  int rv;
  GWEN_NETLAYER *baseLayer;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  /* enable all workarounds for bugs in other SSL implementation for
   * maximum compatibility */
  SSL_CTX_set_options(nld->ssl_ctx, SSL_OP_ALL);

  /* set password handler */
  SSL_CTX_set_default_passwd_cb(nld->ssl_ctx,
                                GWEN_NetLayerSsl_GetPassword);
  SSL_CTX_set_default_passwd_cb_userdata(nld->ssl_ctx, nl);

  if (nld->ownCertFile) {
    /* load own certificate file */
    DBG_DEBUG(GWEN_LOGDOMAIN, "Loading certificate and keys");
    rv=SSL_CTX_use_certificate_chain_file(nld->ssl_ctx,
                                          nld->ownCertFile);
    if (!rv) {
      int sslerr;

      sslerr=SSL_get_error(nld->ssl, rv);
      DBG_ERROR(GWEN_LOGDOMAIN, "SSL error reading certfile: %s (%d)",
                GWEN_NetLayerSsl_ErrorString(sslerr),
                sslerr);
      return -1;
    }

    /* load keys */
    rv=SSL_CTX_use_PrivateKey_file(nld->ssl_ctx,
                                   nld->ownCertFile,
                                   SSL_FILETYPE_PEM);
    if (!rv) {
      int sslerr;

      sslerr=SSL_get_error(nld->ssl, rv);
      DBG_ERROR(GWEN_LOGDOMAIN, "SSL error reading keyfile: %s (%d)",
                GWEN_NetLayerSsl_ErrorString(sslerr),
                sslerr);
      return -1;
    }
    if (!SSL_CTX_check_private_key(nld->ssl_ctx)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Private key does not match the certificate public key");
      return -1;
    }
  }

  /* setup locations of certificates */
  if (nld->CAdir || nld->CAfile) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Loading certificate locations");
    rv=SSL_CTX_load_verify_locations(nld->ssl_ctx,
				     nld->CAfile,
				     nld->CAdir);
    if (rv==0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"SSL: Could not load certificate location "
		"(was: \"%s\" and \"%s\")",
		nld->CAfile,
		nld->CAdir);
      return -1;
    }
  }

  //SSL_CTX_set_verify_depth(nld->ssl_ctx, 1);

  /* setup server stuff */
  if ((GWEN_NetLayer_GetFlags(nl) & GWEN_NETLAYER_FLAGS_PASSIVE)) {
    if (GWEN_NetLayer_GetStatus(nl)!=GWEN_NetLayerStatus_Listening) {
      /* always expect peer certificate */
      if (nld->secure)
        SSL_CTX_set_verify(nld->ssl_ctx,
                           SSL_VERIFY_PEER |
                           SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                           GWEN_NetLayerSsl_VerifyCallBack);
      else
        SSL_CTX_set_verify(nld->ssl_ctx,
                           SSL_VERIFY_PEER,
                           GWEN_NetLayerSsl_VerifyCallBack);
    }
  }
  else {
    /* active connection */
    if (nld->secure)
      SSL_CTX_set_verify(nld->ssl_ctx,
                         SSL_VERIFY_PEER,
                         GWEN_NetLayerSsl_VerifyCallBack);
    else
      SSL_CTX_set_verify(nld->ssl_ctx,
                         SSL_VERIFY_NONE,
                         GWEN_NetLayerSsl_VerifyCallBack);
  }

  nld->ssl=SSL_new(nld->ssl_ctx);
  SSL_set_ex_data(nld->ssl, gwen_netlayerssl_sslidx, nl);
  SSL_set_tmp_dh_callback(nld->ssl, GWEN_NetLayerSsl_tmp_dh_callback);

  if (nld->cipherList) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Setting cipher list to \"%s\"",
              nld->cipherList);
    SSL_set_cipher_list(nld->ssl, nld->cipherList);
  }

  if (GWEN_NetLayer_GetFlags(nl) & GWEN_NETLAYER_FLAGS_PASSIVE)
    SSL_set_accept_state(nld->ssl);
  else
    SSL_set_connect_state(nld->ssl);

  /* set info callback */
  SSL_set_info_callback(nld->ssl,
                        (void (*)()) GWEN_NetLayerSsl_InfoCallBack);

  /* tell SSL to use our BIO */
  nld->bio=BIO_netlayer_new(baseLayer);
  SSL_set_bio(nld->ssl, nld->bio, nld->bio);

  return 0;
}




/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSsl_Connect(GWEN_NETLAYER *nl) {
  GWEN_NL_SSL *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;
  GWEN_NETLAYER_STATUS st;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  st=GWEN_NetLayer_GetStatus(nl);
  if (st!=GWEN_NetLayerStatus_Unconnected &&
      st!=GWEN_NetLayerStatus_Disconnected){
    DBG_ERROR(GWEN_LOGDOMAIN, "Socket is not unconnected (status \"%s\")",
              GWEN_NetLayerStatus_toString(st));
    return GWEN_ERROR_INVALID;
  }

  nld->mode=GWEN_NetLayerSslMode_PConnecting;
  rv=GWEN_NetLayer_Connect(baseLayer);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Result of BaseLayer Connect: %d", rv);
  if (rv<0) {
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disabled);
    nld->mode=GWEN_NetLayerSslMode_Idle;
  }
  else {
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Connecting);
    GWEN_NetLayer_SubFlags(nl, GWEN_NETLAYER_FLAGS_PASSIVE);
  }
  return rv;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSsl_Listen(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;
  int rv;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);
  rv=GWEN_NetLayer_Listen(baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_PASSIVE);
  return rv;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSsl_Disconnect(GWEN_NETLAYER *nl) {
  GWEN_NL_SSL *nld;
  GWEN_NETLAYER_STATUS st;
  GWEN_NETLAYER *baseLayer;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  /* check status */
  st=GWEN_NetLayer_GetStatus(nl);
  if (st==GWEN_NetLayerStatus_Unconnected ||
      st==GWEN_NetLayerStatus_Disconnected ||
      st==GWEN_NetLayerStatus_Disabled) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Socket is inactive: %s (%d)",
              GWEN_NetLayerStatus_toString(st), st);
    return GWEN_ERROR_INVALID;
  }

  nld->mode=GWEN_NetLayerSslMode_PDisconnecting;
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnecting);
  rv=GWEN_NetLayer_Disconnect(baseLayer);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }

  SSL_free(nld->ssl);
  nld->ssl=0;
  SSL_CTX_free(nld->ssl_ctx);
  nld->ssl_ctx=0;
  nld->bio=0; /* FIXME: Does SSL_free free this ? */
  nld->mode=GWEN_NetLayerSslMode_Idle;
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);

  return rv;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSsl_Read(GWEN_NETLAYER *nl, char *buffer, int *bsize){
  GWEN_NL_SSL *nld;
  GWEN_NETLAYER_STATUS st;
  GWEN_NETLAYER *baseLayer;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  assert(*bsize>=0);

  /* check status */
  st=GWEN_NetLayer_GetStatus(nl);
  if (st!=GWEN_NetLayerStatus_Connected) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Socket is not connected: %s (%d)",
              GWEN_NetLayerStatus_toString(st), st);
    return GWEN_ERROR_INVALID;
  }

  /* check for EOF */
  if (GWEN_NetLayer_GetFlags(nl) & GWEN_NETLAYER_FLAGS_EOFMET) {
    DBG_INFO(GWEN_LOGDOMAIN, "EOF already met");
    return GWEN_ERROR_EOF;
  }

  assert(nld->ssl);

  DBG_DEBUG(GWEN_LOGDOMAIN,
            "Reading up to %d bytes while status \"%s\"",
            *bsize,
            SSL_state_string_long(nld->ssl));

  /* try to read */
  ERR_clear_error();
  rv=SSL_read(nld->ssl, buffer, *bsize);
  if (rv<1) {
    int sslerr;

    sslerr=SSL_get_error(nld->ssl, rv);
    if (sslerr==SSL_ERROR_WANT_READ ||
        sslerr==SSL_ERROR_WANT_WRITE)
      return 1; /* would block */
    else {
      if (sslerr==SSL_ERROR_SYSCALL && errno==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Connection just went down (%d: %s)",
                  sslerr,
                  GWEN_NetLayerSsl_ErrorString(sslerr));
        SSL_free(nld->ssl);
        nld->ssl=0;
        SSL_CTX_free(nld->ssl_ctx);
        nld->ssl_ctx=0;
        nld->bio=0; /* FIXME: Does SSL_free free this ? */

        /* connection closed, no real error */
        GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
        *bsize=0;
        return 0; /* changed */
      } /* if error is syscall and errno==0 */
      else {
        if (sslerr==SSL_ERROR_ZERO_RETURN) {
          DBG_INFO(GWEN_LOGDOMAIN, "Connection closed");
          SSL_free(nld->ssl);
          nld->ssl=0;
          SSL_CTX_free(nld->ssl_ctx);
          nld->ssl_ctx=0;
          nld->bio=0; /* FIXME: Does SSL_free free this ? */

          /* connection closed, no real error */
          GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
          *bsize=0;
          return 0; /* changed */
        }
        else {
          DBG_ERROR(GWEN_LOGDOMAIN, "List of pending SSL errors:");
          ERR_print_errors_fp(stderr); /* DEBUG */
          DBG_ERROR(GWEN_LOGDOMAIN, "SSL error: %s (%d)",
                    GWEN_NetLayerSsl_ErrorString(sslerr),
                    sslerr);
          return GWEN_ERROR_READ;
        }
      } /* if !(error is syscall and errno==0) */
    } /* if error is not wantRead or wantWrite */
  } /* if rv<1 */
  else if (rv==0) { /* EOF met */
    DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
    GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_EOFMET);
    SSL_free(nld->ssl);
    nld->ssl=0;
    SSL_CTX_free(nld->ssl_ctx);
    nld->ssl_ctx=0;
    nld->bio=0; /* FIXME: Does SSL_free free this ? */
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
  }

  DBG_DEBUG(GWEN_LOGDOMAIN, "Read %d bytes:", rv);
  GWEN_Text_LogString(buffer, rv, GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

  if (getenv("GWEN_SSL_DEBUG")) {
    FILE *f;

    DBG_DEBUG(GWEN_LOGDOMAIN, "Saving...");
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
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSsl_Write(GWEN_NETLAYER *nl, const char *buffer,int *bsize) {
  GWEN_NL_SSL *nld;
  GWEN_NETLAYER_STATUS st;
  GWEN_NETLAYER *baseLayer;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  /* check status */
  st=GWEN_NetLayer_GetStatus(nl);
  if (st!=GWEN_NetLayerStatus_Connected) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Socket is not connected: %s (%d)",
              GWEN_NetLayerStatus_toString(st), st);
    return GWEN_ERROR_INVALID;
  }

  assert(nld->ssl);

  DBG_DEBUG(GWEN_LOGDOMAIN,
            "Writing up to %d bytes while status \"%s\"",
            *bsize,
            SSL_state_string_long(nld->ssl));

  /* try to write */
  ERR_clear_error();
  rv=SSL_write(nld->ssl, buffer, *bsize);
  if (rv<1) {
    int sslerr;

    sslerr=SSL_get_error(nld->ssl, rv);
    if (sslerr==SSL_ERROR_WANT_READ ||
        sslerr==SSL_ERROR_WANT_WRITE)
      return 1; /* would block */
    else {
      if (sslerr==SSL_ERROR_SYSCALL && errno==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Connection just went down (%d: %s)",
                  sslerr,
                  GWEN_NetLayerSsl_ErrorString(sslerr));
        SSL_free(nld->ssl);
        nld->ssl=0;
        SSL_CTX_free(nld->ssl_ctx);
        nld->ssl_ctx=0;
        nld->bio=0; /* FIXME: Does SSL_free free this ? */

        /* connection closed, no real error */
        GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
        *bsize=0;
        return 0; /* changed */
      } /* if error is syscall and errno==0 */
      else {
        if (sslerr==SSL_ERROR_ZERO_RETURN) {
          DBG_INFO(GWEN_LOGDOMAIN, "Connection closed");
          SSL_free(nld->ssl);
          nld->ssl=0;
          SSL_CTX_free(nld->ssl_ctx);
          nld->ssl_ctx=0;
          nld->bio=0; /* FIXME: Does SSL_free free this ? */

          /* connection closed, no real error */
          GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
          *bsize=0;
          return 0; /* changed */
        }
        else {
          DBG_ERROR(GWEN_LOGDOMAIN, "List of pending SSL errors:");
          ERR_print_errors_fp(stderr); /* DEBUG */
          DBG_ERROR(GWEN_LOGDOMAIN, "SSL error: %s (%d)",
                    GWEN_NetLayerSsl_ErrorString(sslerr),
                    sslerr);
          return GWEN_ERROR_READ;
        }
      } /* if !(error is syscall and errno==0) */
    } /* if error is not wantRead or wantWrite */
  } /* if rv<1 */
  else if (rv==0) { /* EOF met */
    DBG_INFO(GWEN_LOGDOMAIN, "Broken pipe");
    GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_BROKENPIPE);
    SSL_free(nld->ssl);
    nld->ssl=0;
    SSL_CTX_free(nld->ssl_ctx);
    nld->ssl_ctx=0;
    nld->bio=0; /* FIXME: Does SSL_free free this ? */
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
  }

  DBG_DEBUG(GWEN_LOGDOMAIN, "Written %d bytes:", rv);
  GWEN_Text_LogString(buffer, rv, GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

  if (getenv("GWEN_SSL_DEBUG")) {
    FILE *f;

    DBG_DEBUG(GWEN_LOGDOMAIN, "Saving...");
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
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSsl_AddSockets(GWEN_NETLAYER *nl,
                                GWEN_SOCKETSET *readSet,
                                GWEN_SOCKETSET *writeSet,
                                GWEN_SOCKETSET *exSet) {
  GWEN_NETLAYER *baseLayer;
  int rv;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);
  rv=GWEN_NetLayer_AddSockets(baseLayer, readSet, writeSet, exSet);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }
  return rv;
}



/* -------------------------------------------------------------- FUNCTION */
const char *GWEN_NetLayerSsl_ErrorString(unsigned int e) {
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
void GWEN_NetLayerSsl_InfoCallBack(SSL *s, int where, int ret){
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
    DBG_DEBUG(GWEN_LOGDOMAIN,"%s: %s", str, SSL_state_string_long(s));
  }
  else if (where & SSL_CB_ALERT){
    str=(where & SSL_CB_READ)?"read":"write";
    DBG_DEBUG(GWEN_LOGDOMAIN, "SSL3 alert %s: %s: %s",
              str,
              SSL_alert_type_string_long(ret),
              SSL_alert_desc_string_long(ret));
  }
  else if (where & SSL_CB_EXIT){
    if (ret==0) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "%s: failed in \"%s\"",
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
int GWEN_NetLayerSsl_VerifyCallBack(int preverify_ok,
                                    X509_STORE_CTX *ctx) {
  int err;

  err=X509_STORE_CTX_get_error(ctx);
  if (!preverify_ok) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Verify error %d: \"%s\"",
              err, X509_verify_cert_error_string(err));
    if (err==X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY ||
        err==X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT ||
        err==X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Unknown certificate, will not abort yet");
      return 1;
    }
  }
  return preverify_ok;
}



/* -------------------------------------------------------------- FUNCTION */
char *GWEN_NetLayerSsl_GetCertEntry(X509_NAME *nm, int nid) {
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
      return cpy;
    }
  } /* for */

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TIME *GWEN_NetLayerSsl_ASN_GetTime(ASN1_TIME *d) {
  if (d->data) {
    const char *s;
    unsigned int i;
    GWEN_TIME *ti;
    int year, month, day;
    int hour, min, sec=0;
    int isUtc;

    s=(const char*)(d->data);
    i=strlen(s);
    if (i<10) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad time expression (%s)", s);
      return 0;
    }
    isUtc=(s[i-1]=='Z');
    year=((s[0]-'0')*10)+(s[1]-'0');
    if (year<100) {
      if (year<90)
        year+=2000;
      else
        year+=1900;
    }
    month=(((s[2]-'0')*10)+(s[3]-'0'))-1;
    day=((s[4]-'0')*10)+(s[5]-'0');
    hour=((s[6]-'0')*10)+(s[7]-'0');
    min=((s[8]-'0')*10)+(s[9]-'0');
    if (i>11)
      sec=((s[10]-'0')*10)+(s[11]-'0');
    ti=GWEN_Time_new(year, month, day, hour, min, sec, isUtc);
    return ti;
  }
  else {
    return 0;
  }
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_SSLCERTDESCR *GWEN_NetLayerSsl_Cert2Descr(X509 *cert) {
  X509_NAME *nm;
  ASN1_TIME *d;
  EVP_PKEY *pktmp;
  unsigned int  md_size;
  unsigned char md[EVP_MAX_MD_SIZE];
  char *s;
  GWEN_SSLCERTDESCR *cd;

  nm=X509_get_subject_name(cert);

  /* setup certificate descr */
  cd=GWEN_SslCertDescr_new();

  s=GWEN_NetLayerSsl_GetCertEntry(nm, NID_commonName);
  if (s) {
    GWEN_SslCertDescr_SetCommonName(cd, s);
    free(s);
  }

  s=GWEN_NetLayerSsl_GetCertEntry(nm, NID_organizationName);
  if (s) {
    GWEN_SslCertDescr_SetOrganizationName(cd, s);
    free(s);
  }

  s=GWEN_NetLayerSsl_GetCertEntry(nm, NID_organizationalUnitName);
  if (s) {
    GWEN_SslCertDescr_SetOrganizationalUnitName(cd, s);
    free(s);
  }

  s=GWEN_NetLayerSsl_GetCertEntry(nm, NID_countryName);
  if (s) {
    GWEN_SslCertDescr_SetCountryName(cd, s);
    free(s);
  }

  s=GWEN_NetLayerSsl_GetCertEntry(nm, NID_localityName);
  if (s) {
    GWEN_SslCertDescr_SetLocalityName(cd, s);
    free(s);
  }

  s=GWEN_NetLayerSsl_GetCertEntry(nm, NID_stateOrProvinceName);
  if (s) {
    GWEN_SslCertDescr_SetStateOrProvinceName(cd, s);
    free(s);
  }

  d=X509_get_notBefore(cert);
  if (d) {
    GWEN_TIME *ti;

    ti=GWEN_NetLayerSsl_ASN_GetTime(d);
    if (ti) {
      GWEN_SslCertDescr_SetNotBefore(cd, ti);
      GWEN_Time_free(ti);
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error in notBefore date");
    }
  }

  d=X509_get_notAfter(cert);
  if (d) {
    GWEN_TIME *ti;

    ti=GWEN_NetLayerSsl_ASN_GetTime(d);
    if (ti) {
      GWEN_SslCertDescr_SetNotAfter(cd, ti);
      GWEN_Time_free(ti);
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error in notAfter date");
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
      GWEN_CRYPTKEY *key;

      dbKey=GWEN_DB_Group_new("pubKey");

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
      key=GWEN_CryptKey_fromDb(dbKey);
      if (key) {
        GWEN_SslCertDescr_SetPublicKey(cd, key);
        GWEN_CryptKey_free(key);
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "No public key in cert");
      }
      GWEN_DB_Group_free(dbKey);
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

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Text_ToHexBuffer(/* GCC4 pointer-signedness fix: */ (char*)md,
			      md_size, dbuf, 2, ':', 0)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Could not convert fingerprint to hex");
    }
    else {
      GWEN_SslCertDescr_SetFingerPrint(cd, GWEN_Buffer_GetStart(dbuf));
    }
    GWEN_Buffer_free(dbuf);
  }

  return cd;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSsl_SaveCert(GWEN_NETLAYER *nl,
                              X509 *cert,
                              const char *dir,
                              int overwrite) {
  GWEN_NL_SSL *nld;
  FILE *f;
  char cn[256];
  X509_NAME *nm;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  nm=X509_get_subject_name(cert);
  X509_NAME_get_text_by_NID(nm,
                            NID_commonName, cn, sizeof(cn));

  if (!dir)
    dir=nld->CAdir;
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
    GWEN_Buffer_AppendString(nbuf, DIRSEP);
    GWEN_Buffer_AppendString(nbuf, numbuf);
    pos=GWEN_Buffer_GetPos(nbuf);
    for (i=0; i<GWEN_NL_SSL_MAXCOLL; i++) {
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
    if (i>=GWEN_NL_SSL_MAXCOLL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Maximum number of hash collisions reached!");
      GWEN_Buffer_free(nbuf);
      return -1;
    }

    DBG_DEBUG(GWEN_LOGDOMAIN, "Saving file as \"%s\"", GWEN_Buffer_GetStart(nbuf));
    f=fopen(GWEN_Buffer_GetStart(nbuf), "w+");
    if (!f) {
      DBG_ERROR(GWEN_LOGDOMAIN, "fopen(\"%s\", \"w+\"): %s",
		GWEN_Buffer_GetStart(nbuf), strerror(errno));
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
int GWEN_NetLayerSsl_HandleInCert(GWEN_NETLAYER *nl, X509 *cert) {
  GWEN_NL_SSL *nld;
  char *certbuf;
  long vr;
  const char *s;
  int isNew;
  int isError;
  int isWarning;
  const GWEN_INETADDRESS *peerAddr;
  GWEN_ERRORCODE err;
  GWEN_NL_SSL_ASKADDCERT_RESULT res;
  int isErr;
  char addrBuffer[256];
  GWEN_SSLCERTDESCR *cd;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  certbuf=X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
  DBG_INFO(GWEN_LOGDOMAIN, "Got a certificate: %s", certbuf);

  /* setup certificate */
  isNew=0;
  isError=0;
  isWarning=0;
  nld->peerCertificate=GWEN_NetLayerSsl_Cert2Descr(cert);
  cd=nld->peerCertificate;

  peerAddr=GWEN_NetLayer_GetPeerAddr(nl);
  assert(peerAddr);
 
  err=GWEN_InetAddr_GetAddress(peerAddr, addrBuffer,
                               sizeof(addrBuffer)-1);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
  }
  else {
    GWEN_SslCertDescr_SetIpAddress(cd, addrBuffer);
  }

  /* setup statusText and statusCode */
  vr=SSL_get_verify_result(nld->ssl);
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

  GWEN_SslCertDescr_SetStatusText(cd, s);
  GWEN_SslCertDescr_SetIsNew(cd, isNew);
  GWEN_SslCertDescr_SetIsError(cd, isError);
  GWEN_SslCertDescr_SetIsWarning(cd, isWarning);

  if (vr!=X509_V_OK && nld->secure) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Invalid peer certificate in secure mode, aborting");
    free(certbuf);
    return -1;
  }

  /* ask user */
  isErr=0;
  DBG_DEBUG(GWEN_LOGDOMAIN,
            "Unknown certificate \"%s\", asking user",
            certbuf);
  res=GWEN_NetLayerSsl_AskAddCert(nl, cd);

  switch(res) {

  case GWEN_NetLayerSsl_AskAddCertResult_Error:
    DBG_ERROR(GWEN_LOGDOMAIN, "Error asking user");
    isErr=1;
    break;

  case GWEN_NetLayerSsl_AskAddCertResult_No:
    DBG_ERROR(GWEN_LOGDOMAIN, "User doesn't trust the certificate");
    isErr=1;
    break;

  case GWEN_NetLayerSsl_AskAddCertResult_Tmp:
    DBG_INFO(GWEN_LOGDOMAIN, "Temporarily trusting certificate");
    break;

  case GWEN_NetLayerSsl_AskAddCertResult_Perm:
    DBG_NOTICE(GWEN_LOGDOMAIN, "Adding certificate to trusted certs");
    if (GWEN_NetLayerSsl_SaveCert(nl, cert, nld->CAdir, !isNew)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error saving certificate");
      isErr=1;
    }
    break;

  case GWEN_NetLayerSsl_AskAddCertResult_Incoming:
    if (!nld->newCAdir) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No dir for incoming connections given");
      isErr=1;
    }
    else {
      DBG_NOTICE(GWEN_LOGDOMAIN, "Adding certificate to incoming certs");
      if (GWEN_NetLayerSsl_SaveCert(nl, cert, nld->newCAdir, 1)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Error saving certificate");
        isErr=1;
      }
    }
    break;

  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected result");
    break;
  } /* switch */

  if (isErr)
    return -1;

  if (vr==X509_V_OK) {
    DBG_INFO(GWEN_LOGDOMAIN, "Secure connection with peer \"%s\"",
             GWEN_SslCertDescr_GetCommonName(nld->peerCertificate));
    nld->isSecure=1;
  }

  free(certbuf);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayerSsl_SetAskAddCertFn(GWEN_NETLAYER *nl,
                                      GWEN_NL_SSL_ASKADDCERT_FN fn,
                                      void *user_data) {
  GWEN_NL_SSL *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  nld->askAddCertFn=fn;
  nld->askAddCertData=user_data;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NL_SSL_ASKADDCERT_RESULT
GWEN_NetLayerSsl_AskAddCert(GWEN_NETLAYER *nl,
                            const GWEN_SSLCERTDESCR *cd){
  GWEN_NL_SSL *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  if (nld->askAddCertFn)
    return nld->askAddCertFn(nl, cd, nld->askAddCertData);
  else
    return GWEN_NetLayerSsl_AskAddCertResult_No;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayerSsl_Work(GWEN_NETLAYER *nl) {
  GWEN_NL_SSL *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;
  GWEN_NETLAYER_STATUS st;
  GWEN_NETLAYER_STATUS bst;
  GWEN_NETLAYER_RESULT bres=GWEN_NetLayerResult_Idle;
  GWEN_NETLAYER_RESULT res;

#define GWEN_NL_SSL_MERGE_RESULTS(r1, r2)    \
  if (r1==GWEN_NetLayerResult_Idle)           \
    r1=r2;                                    \
  else if (r1!=GWEN_NetLayerResult_Changed) { \
    if (r2==GWEN_NetLayerResult_Changed)      \
      r1=GWEN_NetLayerResult_Changed;          \
  }

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  res=GWEN_NetLayer_Work(baseLayer);
  DBG_VERBOUS(GWEN_LOGDOMAIN,
              "Result of BaseLayer work: %s",
              GWEN_NetLayerResult_toString(res));
  if (res==GWEN_NetLayerResult_Error) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return res;
  }
  GWEN_NL_SSL_MERGE_RESULTS(bres, res);

  st=GWEN_NetLayer_GetStatus(nl);
  bst=GWEN_NetLayer_GetStatus(baseLayer);

  res=GWEN_NetLayerResult_Idle;

  if (st==GWEN_NetLayerStatus_Connecting) {
    int handled=0;

    if (nld->mode==GWEN_NetLayerSslMode_PConnecting) {
      handled=1;
      if (bst==GWEN_NetLayerStatus_Connected) {
        nld->mode=GWEN_NetLayerSslMode_PConnected;
        res=GWEN_NetLayerResult_Changed;
        GWEN_NL_SSL_MERGE_RESULTS(bres, res);
      }
      else if (bst!=GWEN_NetLayerStatus_Connecting) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Bad status of base layer (%s)",
                  GWEN_NetLayerStatus_toString(bst));
        SSL_free(nld->ssl);
        nld->ssl=0;
        SSL_CTX_free(nld->ssl_ctx);
        nld->ssl_ctx=0;
        nld->bio=0; /* FIXME: Does SSL_free free this ? */
        GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disabled);
        return GWEN_NetLayerResult_Error;
      }
    } /* if PConnecting */

    if (nld->mode==GWEN_NetLayerSslMode_PConnected) {
      handled=1;
      GWEN_SslCertDescr_free(nld->peerCertificate);
      nld->peerCertificate=0;
      nld->isSecure=0;

      if (bst!=GWEN_NetLayerStatus_Connected) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Bad status of base layer (%s)",
                  GWEN_NetLayerStatus_toString(bst));
        SSL_free(nld->ssl);
        nld->ssl=0;
        SSL_CTX_free(nld->ssl_ctx);
        nld->ssl_ctx=0;
        nld->bio=0; /* FIXME: Does SSL_free free this ? */
        GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disabled);
        return GWEN_NetLayerResult_Error;
      }

      /* socket ready now setup SSL */
      if (GWEN_NetLayer_GetFlags(nl) & GWEN_NETLAYER_FLAGS_PASSIVE)
        nld->ssl_ctx=SSL_CTX_new(SSLv23_server_method());
      else
        nld->ssl_ctx=SSL_CTX_new(SSLv23_client_method());

      if (GWEN_NetLayerSsl_Setup(nl)) {
        SSL_free(nld->ssl);
        nld->ssl=0;
        SSL_CTX_free(nld->ssl_ctx);
        nld->ssl_ctx=0;
        nld->bio=0; /* FIXME: Does SSL_free free this ? */
        DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup SSL conntection");
        GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disabled);
        return GWEN_NetLayerResult_Error;
      }
      nld->mode=GWEN_NetLayerSslMode_LConnecting;
      res=GWEN_NetLayerResult_Changed;
      GWEN_NL_SSL_MERGE_RESULTS(bres, res);
    } /* if PConnected */

    if (nld->mode==GWEN_NetLayerSslMode_LConnecting) {
      X509 *cert;
      handled=1;

      if (bst!=GWEN_NetLayerStatus_Connected) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Bad status of base layer (%s)",
                  GWEN_NetLayerStatus_toString(bst));
        SSL_free(nld->ssl);
        nld->ssl=0;
        SSL_CTX_free(nld->ssl_ctx);
        nld->ssl_ctx=0;
        nld->bio=0; /* FIXME: Does SSL_free free this ? */
        GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disabled);
        return GWEN_NetLayerResult_Error;
      }

      /* check for established SSL */
      ERR_clear_error();
      if (GWEN_NetLayer_GetFlags(nl) & GWEN_NETLAYER_FLAGS_PASSIVE) {
        DBG_VERBOUS(GWEN_LOGDOMAIN, "Calling accept");
        rv=SSL_accept(nld->ssl);
      }
      else {
        DBG_VERBOUS(GWEN_LOGDOMAIN, "Calling connect");
        rv=SSL_connect(nld->ssl);
      }

      if (rv!=1) {
        int sslerr;
    
        sslerr=SSL_get_error(nld->ssl, rv);
        if (sslerr==SSL_ERROR_WANT_READ ||
            sslerr==SSL_ERROR_WANT_WRITE)
          return GWEN_NetLayerResult_WouldBlock; /* would block */
        else {
          if (sslerr==SSL_ERROR_SYSCALL && errno==0) {
            DBG_ERROR(GWEN_LOGDOMAIN, "Connection just went down (%d: %s)",
                      sslerr,
                      GWEN_NetLayerSsl_ErrorString(sslerr));
            SSL_free(nld->ssl);
            nld->ssl=0;
            SSL_CTX_free(nld->ssl_ctx);
            nld->ssl_ctx=0;
            nld->bio=0; /* FIXME: Does SSL_free free this ? */
    
            /* connection closed, no real error */
            GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
            return GWEN_NetLayerResult_Changed; /* changed */
          } /* if error is syscall and errno==0 */
          else {
            if (sslerr==SSL_ERROR_ZERO_RETURN) {
              DBG_INFO(GWEN_LOGDOMAIN, "Connection closed");
              SSL_free(nld->ssl);
              nld->ssl=0;
              SSL_CTX_free(nld->ssl_ctx);
              nld->ssl_ctx=0;
              nld->bio=0; /* FIXME: Does SSL_free free this ? */
    
              /* connection closed, no real error */
              GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
              return GWEN_NetLayerResult_Changed; /* changed */
            }
            else {
              DBG_ERROR(GWEN_LOGDOMAIN, "List of pending SSL errors:");
              ERR_print_errors_fp(stderr); /* DEBUG */
              DBG_ERROR(GWEN_LOGDOMAIN, "SSL error: %s (%d)",
                        GWEN_NetLayerSsl_ErrorString(sslerr),
                        sslerr);
              return GWEN_NetLayerResult_Error;
            }
          } /* if !(error is syscall and errno==0) */
        } /* if error is not wantRead or wantWrite */
      } /* if error */

      /* now logically connected */
      DBG_INFO(GWEN_LOGDOMAIN, "Logically connected");
      GWEN_SslCertDescr_free(nld->peerCertificate);
      nld->peerCertificate=0;

      /* show info about used cipher */
      if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Notice) {
        SSL_CIPHER *ci;
        char buffer[256];
        const char *p;

        ci=SSL_get_current_cipher(nld->ssl);
        assert(ci);

        p=SSL_CIPHER_description(ci, buffer, sizeof(buffer));
        DBG_INFO(GWEN_LOGDOMAIN, "Connected using \"%s\"", p);
      }

      cert=SSL_get_peer_certificate(nld->ssl);
      if (!cert) {
        if (nld->secure) {
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "Peer did not send a certificate, abort");
          SSL_free(nld->ssl);
          nld->ssl=0;
          SSL_CTX_free(nld->ssl_ctx);
          nld->ssl_ctx=0;
          nld->bio=0; /* FIXME: Does SSL_free free this ? */

          GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disabled);
          return GWEN_NetLayerResult_Error;
        }
        else {
          DBG_WARN(GWEN_LOGDOMAIN, "Peer did not send a certificate");
        }
      }
      else {
        rv=GWEN_NetLayerSsl_HandleInCert(nl, cert);
        X509_free(cert);
        if (rv) {
          DBG_INFO(GWEN_LOGDOMAIN, "Invalid peer certificate");
          SSL_free(nld->ssl);
          nld->ssl=0;
          SSL_CTX_free(nld->ssl_ctx);
          nld->ssl_ctx=0;
          nld->bio=0; /* FIXME: Does SSL_free free this ? */

          GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disabled);
          return GWEN_NetLayerResult_Error;
        }
      }

      DBG_INFO(GWEN_LOGDOMAIN, "SSL connection established (%s)",
               (nld->isSecure)?"verified":"not verified");

      GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Connected);
      nld->mode=GWEN_NetLayerSslMode_LConnected;
      res=GWEN_NetLayerResult_Changed;
      GWEN_NL_SSL_MERGE_RESULTS(bres, res);
    } /* if LConnecting */
    if (!handled) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected mode %d", nld->mode);
      SSL_free(nld->ssl);
      nld->ssl=0;
      SSL_CTX_free(nld->ssl_ctx);
      nld->ssl_ctx=0;
      nld->bio=0; /* FIXME: Does SSL_free free this ? */
      GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disabled);
      return GWEN_NetLayerResult_Error;
    }
  } /* if Connecting */

  if (st==GWEN_NetLayerStatus_Connected) {
    if (bst!=GWEN_NetLayerStatus_Connected) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Bad status of base layer (%s)",
                GWEN_NetLayerStatus_toString(bst));
      SSL_free(nld->ssl);
      nld->ssl=0;
      SSL_CTX_free(nld->ssl_ctx);
      nld->ssl_ctx=0;
      nld->bio=0; /* FIXME: Does SSL_free free this ? */
      GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disabled);
      return GWEN_NetLayerResult_Error;
    }
  }

  if (st==GWEN_NetLayerStatus_Listening) {
    GWEN_NETLAYER *newNl;

    newNl=GWEN_NetLayer_GetIncomingLayer(baseLayer);
    if (newNl) {
      GWEN_NETLAYER *newNlSsl;

      DBG_NOTICE(GWEN_LOGDOMAIN, "New incoming connection");
      newNlSsl=GWEN_NetLayerSsl_new(newNl,
                                    nld->CAdir,
                                    nld->newCAdir,
                                    nld->ownCertFile,
                                    nld->dhFolder,
                                    nld->secure);
      GWEN_NetLayer_AddFlags(newNlSsl, GWEN_NETLAYER_FLAGS_PASSIVE);
      GWEN_NetLayer_SetStatus(newNlSsl, GWEN_NetLayerStatus_Connecting);
      GWEN_NetLayerSsl_SetMode(newNlSsl, GWEN_NetLayerSslMode_PConnected);
      GWEN_NetLayer_free(newNl);
      GWEN_NetLayer_AddIncomingLayer(nl, newNlSsl);
      res=GWEN_NetLayerResult_Changed;
      GWEN_NL_SSL_MERGE_RESULTS(bres, res);
    }
  }

  return bres;
#undef GWEN_NL_SSL_MERGE_RESULTS
}



/* -------------------------------------------------------------- FUNCTION */
DH *GWEN_NetLayerSsl_tmp_dh_callback(SSL *s, int is_export, int keylength){
  GWEN_NETLAYER *nl;
  GWEN_NL_SSL *nld;
  char buffer[256];
  int rv;
  FILE *f;
  DH *dh;
  int codes;

  assert(gwen_netlayerssl_sslidx!=-1);
  nl=SSL_get_ex_data(s, gwen_netlayerssl_sslidx);

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  rv=snprintf(buffer, sizeof(buffer), "%s/dh-%d.pem",
              nld->dhFolder, keylength);
  if (rv>=sizeof(buffer) || rv<1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Path for dhFolder too long (%d)", rv);
    return 0;
  }

  f=fopen(buffer, "r");
  if (f==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No DH file for keylength %d found in \"%s\"",
              keylength, nld->dhFolder);
    return 0;
  }

  dh=PEM_read_DHparams(f, NULL, NULL, NULL);
  fclose(f);
  if (dh==0) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "SSL Error: Bad DH file");
    return 0;
  }

  /* check for usability */
  rv=DH_check(dh, &codes);
  if (!rv) {
    int sslerr;

    sslerr=SSL_get_error(nld->ssl, rv);
    DBG_ERROR(GWEN_LOGDOMAIN, "SSL DH_check error: %s (%d)",
              GWEN_NetLayerSsl_ErrorString(sslerr),
              sslerr);
    DH_free(dh);
    return 0;
  }

  if (codes & DH_CHECK_P_NOT_PRIME){
    DBG_ERROR(GWEN_LOGDOMAIN, "SSL DH error: p is not prime");
    DH_free(dh);
    return 0;
  }

  if ((codes & DH_NOT_SUITABLE_GENERATOR) &&
      (codes & DH_CHECK_P_NOT_SAFE_PRIME)){
    DBG_ERROR(GWEN_LOGDOMAIN, "SSL DH error : "
              "neither suitable generator or safe prime");
    DH_free(dh);
    return 0;
  }

  /* DH params seem to be ok */
  return dh;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayerSsl_SetCiphers(GWEN_NETLAYER *nl, const char *ciphers){
  GWEN_NL_SSL *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  free(nld->cipherList);
  if (ciphers)
    nld->cipherList=strdup(ciphers);
  else
    nld->cipherList=0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_SSLCIPHER_LIST *GWEN_NetLayerSsl_GetCipherList(){
  STACK_OF(SSL_CIPHER) *ciphers;
  SSL *ssl;
  SSL_CTX *ctx;
  const char *p;

  ctx=SSL_CTX_new(SSLv23_client_method());
  ssl=SSL_new(ctx);

  ciphers=(STACK_OF(SSL_CIPHER)*)SSL_get_ciphers(ssl);
  if (ciphers) {
    GWEN_SSLCIPHER_LIST *ll;
    SSL_CIPHER *curr;
    int n;
    char buffer[256];

    ll=GWEN_SslCipher_List_new();
    for (n=0;n<sk_SSL_CIPHER_num(ciphers) ; n++) {
      curr=sk_SSL_CIPHER_value(ciphers, n);
      p=SSL_CIPHER_get_name(curr);
      if (p) {
        GWEN_SSLCIPHER *ciph;

        ciph=GWEN_SslCipher_new();
        GWEN_SslCipher_SetName(ciph, p);
        GWEN_SslCipher_SetBits(ciph, SSL_CIPHER_get_bits(curr, 0));
        p=SSL_CIPHER_get_version(curr);
        if (p)
          GWEN_SslCipher_SetVersion(ciph, p);
        p=SSL_CIPHER_description(curr, buffer, sizeof(buffer));
        if (p)
          GWEN_SslCipher_SetDescription(ciph, p);
        GWEN_SslCipher_List_Add(ciph, ll);
      } /* if cipher name */
    } /* for */
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    if (GWEN_SslCipher_List_GetCount(ll)==0) {
      DBG_WARN(GWEN_LOGDOMAIN, "No valid ciphers");
      GWEN_SslCipher_List_free(ll);
      return 0;
    }
    return ll;
  } /* if ciphers */
  else {
    DBG_WARN(GWEN_LOGDOMAIN, "No ciphers");
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return 0;
  }
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSsl_GenerateCertAndKeyFile(const char *fname,
                                            int bits,
                                            int serial,
                                            int days,
                                            const GWEN_SSLCERTDESCR *cd) {
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
    days=GWEN_NL_SSL_DEFAULT_CERT_DAYS;
  X509_gmtime_adj(X509_get_notAfter(x),(long)60*60*24*days);
  X509_set_pubkey(x, pk);

  name=X509_NAME_new();

  p=GWEN_SslCertDescr_GetCountryName(cd);
  if (!p || !*p)
    p="DE";
  ne=X509_NAME_ENTRY_create_by_NID(NULL,
                                   NID_countryName,
                                   V_ASN1_APP_CHOOSE,
                                   (unsigned char*)p,
                                   -1);
  X509_NAME_add_entry(name, ne, 0, 0);

  p=GWEN_SslCertDescr_GetCommonName(cd);
  if (p && *p) {
    X509_NAME_ENTRY_create_by_NID(&ne, NID_commonName,
                                  V_ASN1_APP_CHOOSE,
                                  (unsigned char*)p,
                                  -1);
    X509_NAME_add_entry(name, ne, -1, 0);
  }

  p=GWEN_SslCertDescr_GetOrganizationName(cd);
  if (p && *p) {
    X509_NAME_ENTRY_create_by_NID(&ne, NID_organizationName,
                                  V_ASN1_APP_CHOOSE,
                                  (unsigned char*)p,
                                  -1);
    X509_NAME_add_entry(name, ne, -1, 0);
  }

  p=GWEN_SslCertDescr_GetOrganizationalUnitName(cd);
  if (p && *p) {
    X509_NAME_ENTRY_create_by_NID(&ne, NID_organizationalUnitName,
                                  V_ASN1_APP_CHOOSE,
                                  (unsigned char*)p,
                                  -1);
    X509_NAME_add_entry(name, ne, -1, 0);
  }

  p=GWEN_SslCertDescr_GetLocalityName(cd);
  if (p && *p) {
    X509_NAME_ENTRY_create_by_NID(&ne, NID_localityName,
                                  V_ASN1_APP_CHOOSE,
                                  (unsigned char*)p,
                                  -1);
    X509_NAME_add_entry(name, ne, -1, 0);
  }

  p=GWEN_SslCertDescr_GetStateOrProvinceName(cd);
  if (p && *p) {
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
void GWEN_NetLayerSsl__GenerateDhFile_Callback(int i, int j, void *p) {
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
int GWEN_NetLayerSsl_GenerateDhFile(const char *fname, int bits) {
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
			    GWEN_NetLayerSsl__GenerateDhFile_Callback,
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



/* -------------------------------------------------------------- FUNCTION */
const GWEN_SSLCERTDESCR*
GWEN_NetLayerSsl_GetPeerCertificate(const GWEN_NETLAYER *nl) {
  GWEN_NL_SSL *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  return nld->peerCertificate;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSsl_GetIsSecure(const GWEN_NETLAYER *nl) {
  GWEN_NL_SSL *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SSL, nl);
  assert(nld);

  return nld->isSecure;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSsl_ImportCertFile(const char *fname,
				    const char *dname,
				    int overwrite) {
  X509 *cert;
  X509_NAME *nm;
  FILE *f;
  GWEN_BUFFER *nbuf;
  unsigned long hash;
  char numbuf[32];
  int i;
  GWEN_TYPE_UINT32 pos;

  /* read certificate file */
  f=fopen(fname, "r");
  if (!f) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(%s): %s", fname, strerror(errno));
    return -1;
  }
  cert=PEM_read_X509(f, NULL, NULL, NULL);
  fclose(f);
  if (!cert) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "file \"%s\" does not contain a valid certificate",
	      fname);
    return -1;
  }

  nm=X509_get_subject_name(cert);
  hash=X509_NAME_hash(nm);
  snprintf(numbuf, sizeof(numbuf), "%08lx", hash);
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  GWEN_Buffer_AppendString(nbuf, dname);
  GWEN_Buffer_AppendString(nbuf, DIRSEP);
  GWEN_Buffer_AppendString(nbuf, numbuf);
  pos=GWEN_Buffer_GetPos(nbuf);
  for (i=0; i<GWEN_NL_SSL_MAXCOLL; i++) {
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
  if (i>=GWEN_NL_SSL_MAXCOLL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Maximum number of hash collisions reached!");
    GWEN_Buffer_free(nbuf);
    X509_free(cert);
    return -1;
  }

  DBG_DEBUG(GWEN_LOGDOMAIN, "Saving file as \"%s\"",
	    GWEN_Buffer_GetStart(nbuf));
  f=fopen(GWEN_Buffer_GetStart(nbuf), "w+");
  if (!f) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(\"%s\", \"w+\"): %s",
	      GWEN_Buffer_GetStart(nbuf), strerror(errno));
    GWEN_Buffer_free(nbuf);
    X509_free(cert);
    return -1;
  }
  GWEN_Buffer_free(nbuf);
  X509_free(cert);

  return 0;
}






