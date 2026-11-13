/***************************************************************************
 begin       : Wed Apr 28 2010
 copyright   : (C) 2010, 2016 by Martin Preuss
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

#define DISABLE_DEBUGLOG

/*#define GWEN_TLS_DEBUG*/

/* #define GWEN_TLS_USE_OLD_CODE */

#include "syncio_tls_p.h"
#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/text.h>

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef GWEN_TLS_OPENSSL
# include <openssl/ssl.h>
# include <openssl/err.h>
# include <openssl/x509v3.h>
# include <openssl/core_names.h>
#else
# include <gnutls/gnutls.h>
# include <gnutls/x509.h>
# include <gcrypt.h>
#endif



GWEN_INHERIT(GWEN_SYNCIO, GWEN_SYNCIO_TLS)


#ifndef OS_WIN32
const char *SYNCIO_TLS_SYSTEM_CERTFILES[]= {
  "/etc/ssl/certs/ca-certificates.crt",
  "/etc/ssl/ca-bundle.pem",
#ifdef GWEN_TLS_OPENSSL
  "/etc/ssl/cert.pem",
#endif
  NULL
};
#endif




GWEN_SYNCIO *GWEN_SyncIo_Tls_new(GWEN_SYNCIO *baseIo)
{
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_TLS *xio;

  assert(baseIo);
  sio=GWEN_SyncIo_new(GWEN_SYNCIO_TLS_TYPE, baseIo);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_TLS, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio, xio, GWEN_SyncIo_Tls_FreeData);

  /* preset data */
  xio->checkCertFn=GWEN_SyncIo_Tls_Internal_CheckCert;

  /* set virtual functions */
  GWEN_SyncIo_SetConnectFn(sio, GWEN_SyncIo_Tls_Connect);
  GWEN_SyncIo_SetDisconnectFn(sio, GWEN_SyncIo_Tls_Disconnect);
  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_Tls_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_Tls_Write);

  return sio;
}



void GWENHYWFAR_CB GWEN_SyncIo_Tls_FreeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_SYNCIO_TLS *xio;

  xio=(GWEN_SYNCIO_TLS *) p;
  free(xio->localCertFile);
  free(xio->localKeyFile);
  free(xio->localTrustFile);
  free(xio->dhParamFile);
  free(xio->hostName);
  GWEN_SslCertDescr_free(xio->peerCertDescr);
  GWEN_FREE_OBJECT(xio);
}



GWEN_SIO_TLS_CHECKCERT_FN GWEN_SyncIo_Tls_SetCheckCertFn(GWEN_SYNCIO *sio, GWEN_SIO_TLS_CHECKCERT_FN f)
{
  GWEN_SYNCIO_TLS *xio;
  GWEN_SIO_TLS_CHECKCERT_FN oldF;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  oldF=xio->checkCertFn;
  xio->checkCertFn=f;
  return oldF;
}



int GWENHYWFAR_CB GWEN_SyncIo_Tls_Internal_CheckCert(GWEN_SYNCIO *sio,
                                                     const GWEN_SSLCERTDESCR *cert)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  DBG_WARN(GWEN_LOGDOMAIN, "No checkCertFn set, using GWEN_GUI");
  return GWEN_Gui_CheckCert(cert, sio, 0);
}



int GWEN_SyncIo_Tls_CheckCert(GWEN_SYNCIO *sio, const GWEN_SSLCERTDESCR *cert)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  if (xio->checkCertFn) {
    /* try my own checkCert function first */
    return xio->checkCertFn(sio, cert);
  }
  else {
    /* none set, call the check cert function of GWEN_GUI (for older code) */
    DBG_ERROR(GWEN_LOGDOMAIN, "No checkCertFn set, falling back to GUI (SNH!)");
    return GWEN_SyncIo_Tls_Internal_CheckCert(sio, cert);
  }
}



const char *GWEN_SyncIo_Tls_GetLocalCertFile(const GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  return xio->localCertFile;
}



void GWEN_SyncIo_Tls_SetLocalCertFile(GWEN_SYNCIO *sio, const char *s)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  free(xio->localCertFile);
  if (s)
    xio->localCertFile=strdup(s);
  else
    xio->localCertFile=NULL;
}



const char *GWEN_SyncIo_Tls_GetLocalKeyFile(const GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  return xio->localKeyFile;
}



void GWEN_SyncIo_Tls_SetLocalKeyFile(GWEN_SYNCIO *sio, const char *s)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  free(xio->localKeyFile);
  if (s)
    xio->localKeyFile=strdup(s);
  else
    xio->localKeyFile=NULL;
}



const char *GWEN_SyncIo_Tls_GetLocalTrustFile(const GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  return xio->localTrustFile;
}



void GWEN_SyncIo_Tls_SetLocalTrustFile(GWEN_SYNCIO *sio, const char *s)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  free(xio->localTrustFile);
  if (s)
    xio->localTrustFile=strdup(s);
  else
    xio->localTrustFile=NULL;
}



const char *GWEN_SyncIo_Tls_GetDhParamFile(const GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  return xio->dhParamFile;
}



void GWEN_SyncIo_Tls_SetDhParamFile(GWEN_SYNCIO *sio, const char *s)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  free(xio->dhParamFile);
  if (s)
    xio->dhParamFile=strdup(s);
  else
    xio->dhParamFile=NULL;
}



const char *GWEN_SyncIo_Tls_GetRemoteHostName(const GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  return xio->hostName;
}



void GWEN_SyncIo_Tls_SetRemoteHostName(GWEN_SYNCIO *sio, const char *s)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  free(xio->hostName);
  if (s)
    xio->hostName=strdup(s);
  else
    xio->hostName=NULL;
}



GWEN_SSLCERTDESCR *GWEN_SyncIo_Tls_GetPeerCertDescr(const GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  return xio->peerCertDescr;
}



#ifndef GWEN_TLS_OPENSSL
int GWEN_SyncIo_Tls__readFile(const char *fname, GWEN_BUFFER *buf)
{
  FILE *f;

  f=fopen(fname, "r");
  if (f==NULL)
    return GWEN_ERROR_IO;

  while (!feof(f)) {
    int rv;

    GWEN_Buffer_AllocRoom(buf, 512);
    rv=fread(GWEN_Buffer_GetPosPointer(buf), 1, 512, f);
    if (rv==0)
      break;
    else if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "fread(%s): %s", fname, strerror(errno));
      fclose(f);
      return GWEN_ERROR_IO;
    }
    else {
      GWEN_Buffer_IncrementPos(buf, rv);
      GWEN_Buffer_AdjustUsedBytes(buf);
    }
  }
  fclose(f);
  return 0;
}
#endif




#ifdef GWEN_TLS_OPENSSL

/* The OpenSSL code below is the OpenSSL implementation that Gwenhywfar used until 2007, ported to GWEN_SYNCIO:
 * src/crypt/ossl/nl_ssl.c and src/crypt/ossl/bio_netlayer.c (removed in da13b417), and
 * GWEN_NetTransportSSL_StartDisconnect() from src/crypt/ossl/nettransportssl.c (removed in e43961fb). */

static CRYPTO_ONCE gwen_syncio_tls_once=CRYPTO_ONCE_STATIC_INIT;
static int gwen_syncio_tls_sslidx=-1;
static BIO_METHOD *bio_syncio_methods=NULL;



static void GWEN_SyncIo_Tls_InitOnce(void)
{
  gwen_syncio_tls_sslidx=SSL_get_ex_new_index(0, 0, 0, 0, 0);

  /* BIO methods are opaque since OpenSSL 1.1, and BIO type indexes are a process-wide resource of OpenSSL, so
   * all connections share one BIO method, which lives until the process ends */
  bio_syncio_methods=BIO_meth_new(BIO_get_new_index() | BIO_TYPE_SOURCE_SINK, "GWEN_SyncIo");
  if (bio_syncio_methods) {
    BIO_meth_set_write(bio_syncio_methods, BIO_syncio_write);
    BIO_meth_set_read(bio_syncio_methods, BIO_syncio_read);
    BIO_meth_set_puts(bio_syncio_methods, BIO_syncio_puts);
    BIO_meth_set_ctrl(bio_syncio_methods, BIO_syncio_ctrl);
    BIO_meth_set_create(bio_syncio_methods, BIO_syncio_create);
    BIO_meth_set_destroy(bio_syncio_methods, BIO_syncio_destroy);
  }
}



BIO *BIO_syncio_new(GWEN_SYNCIO *sio)
{
  BIO *bio;

  assert(sio);
  bio=BIO_new(bio_syncio_methods);
  if (bio==NULL)
    return NULL;
  BIO_set_data(bio, sio);
  GWEN_SyncIo_Attach(sio);
  /* GWEN_SyncIo_Tls disconnects the base layer itself */
  BIO_set_shutdown(bio, BIO_NOCLOSE);
  return bio;
}



int BIO_syncio_create(BIO *bio)
{
  DBG_VERBOUS(GWEN_LOGDOMAIN, "BIO method: Create");
  BIO_set_init(bio, 1);
  BIO_clear_flags(bio, ~0);
  return 1;
}



int BIO_syncio_destroy(BIO *bio)
{
  DBG_VERBOUS(GWEN_LOGDOMAIN, "BIO method: Destroy");
  if (bio) {
    GWEN_SYNCIO *sio;

    sio=(GWEN_SYNCIO *) BIO_get_data(bio);
    if (sio) {
      if (BIO_get_shutdown(bio)) {
        /* disconnect layer */
        GWEN_SyncIo_Disconnect(sio);
        BIO_set_init(bio, 0);
        BIO_clear_flags(bio, ~0);
      }
      GWEN_SyncIo_free(sio); /* detach from syncio */
    }
    return 1;
  }
  else
    return 0;
}



int BIO_syncio_read(BIO *bio, char *in, int inl)
{
  DBG_VERBOUS(GWEN_LOGDOMAIN, "BIO method: Read(%d)", inl);
  if (in!=NULL) {
    GWEN_SYNCIO *sio;
    int rv;

    sio=(GWEN_SYNCIO *) BIO_get_data(bio);
    BIO_clear_retry_flags(bio);
    if (sio==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "No syncio in BIO");
      return -1;
    }

    rv=GWEN_SyncIo_Read(sio, (uint8_t *) in, (uint32_t) inl);
    if (rv>=0) {
      return rv;
    }
    else if (rv==GWEN_ERROR_TIMEOUT || rv==GWEN_ERROR_INTERRUPTED) {
      BIO_set_retry_read(bio);
      return -1;
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return -1;
    }
  }
  return 0;
}



int BIO_syncio_write(BIO *bio, const char *out, int outl)
{
  DBG_VERBOUS(GWEN_LOGDOMAIN, "BIO method: Write(%d)", outl);
  if (out!=NULL) {
    GWEN_SYNCIO *sio;
    int rv;

    sio=(GWEN_SYNCIO *) BIO_get_data(bio);
    BIO_clear_retry_flags(bio);
    if (sio==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "No syncio in BIO");
      return -1;
    }

    rv=GWEN_SyncIo_Write(sio, (const uint8_t *) out, (uint32_t) outl);
    if (rv>=0) {
      return rv;
    }
    else if (rv==GWEN_ERROR_TIMEOUT || rv==GWEN_ERROR_INTERRUPTED) {
      BIO_set_retry_write(bio);
      return -1;
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return -1;
    }
  }
  return 0;
}



long BIO_syncio_ctrl(BIO *bio, int cmd, long num, void *ptr)
{
  long rv=1;

  DBG_VERBOUS(GWEN_LOGDOMAIN, "BIO method: Ctl(%d, %li)", cmd, num);
  switch (cmd) {

  case BIO_C_GET_FD:
    rv=-1;
    break;

  case BIO_CTRL_GET_CLOSE:
    rv=BIO_get_shutdown(bio);
    break;

  case BIO_CTRL_FLUSH:
    rv=1;
    break;

  case BIO_CTRL_SET_CLOSE:
    BIO_set_shutdown(bio, (int)num);
    break;

  case BIO_CTRL_RESET:
  case BIO_C_FILE_SEEK:
  case BIO_C_FILE_TELL:
  case BIO_CTRL_INFO:
  case BIO_C_SET_FD:
  case BIO_CTRL_PENDING:
  case BIO_CTRL_WPENDING:
  case BIO_CTRL_DUP:
  default:
    rv=0;
    break;
  }
  return rv;
}



int BIO_syncio_puts(BIO *bp, const char *str)
{
  int l, rv;

  DBG_VERBOUS(GWEN_LOGDOMAIN, "BIO method: Puts(\"%s\")", str);
  l=strlen(str);
  rv=BIO_syncio_write(bp, str, l);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }
  return rv;
}



const char *GWEN_SyncIo_Tls_ErrorString(unsigned int e)
{
  const char *s;

  switch (e) {
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



/* Appends the text of the most recent entry in the OpenSSL error queue, which tells more than
 * GWEN_SyncIo_Tls_ErrorString() */
static void GWEN_SyncIo_Tls_ErrorText(int sslerr, char *buffer, size_t bufferSize)
{
  unsigned long e;

  e=ERR_peek_last_error();
  if (e) {
    char ebuf[200];

    ERR_error_string_n(e, ebuf, sizeof(ebuf));
    snprintf(buffer, bufferSize, "%s: %s", GWEN_SyncIo_Tls_ErrorString(sslerr), ebuf);
  }
  else
    snprintf(buffer, bufferSize, "%s", GWEN_SyncIo_Tls_ErrorString(sslerr));
}



void GWEN_SyncIo_Tls_InfoCallBack(const SSL *s, int where, int ret)
{
  const char *str;
  int w;

  w=where& ~SSL_ST_MASK;
  if (w & SSL_ST_CONNECT)
    str="SSL_connect";
  else if (w & SSL_ST_ACCEPT)
    str="SSL_accept";
  else
    str="undefined";
  (void) str; /* only used by debug output, which may be compiled out */

  if (where & SSL_CB_LOOP) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "%s: %s", str, SSL_state_string_long(s));
  }
  else if (where & SSL_CB_ALERT) {
    str=(where & SSL_CB_READ)?"read":"write";
    DBG_DEBUG(GWEN_LOGDOMAIN, "SSL3 alert %s: %s: %s",
              str,
              SSL_alert_type_string_long(ret),
              SSL_alert_desc_string_long(ret));
  }
  else if (where & SSL_CB_EXIT) {
    if (ret==0) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "%s: failed in \"%s\"",
                str,
                SSL_state_string_long(s));
    }
    else if (ret<0) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "%s: error in \"%s\"",
                str,
                SSL_state_string_long(s));
    }
  }
}



/* Never aborts the handshake: like the GnuTLS build, it records the verification errors as
 * GWEN_SSL_CERT_FLAGS_*, and the application decides about the certificate in GWEN_SyncIo_Tls_CheckCert(). */
int GWEN_SyncIo_Tls_VerifyCallBack(int preverify_ok, X509_STORE_CTX *ctx)
{
  int err;

  err=X509_STORE_CTX_get_error(ctx);
  if (!preverify_ok) {
    SSL *ssl;
    GWEN_SYNCIO *sio;
    GWEN_SYNCIO_TLS *xio;

    DBG_DEBUG(GWEN_LOGDOMAIN, "Verify error %d: \"%s\"",
              err, X509_verify_cert_error_string(err));

    ssl=X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
    sio=SSL_get_ex_data(ssl, gwen_syncio_tls_sslidx);
    assert(sio);
    xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
    assert(xio);

    /* every error makes the certificate invalid, as GNUTLS_CERT_INVALID does */
    xio->verifyFlags|=GWEN_SSL_CERT_FLAGS_INVALID;
    if (err==X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY ||
        err==X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT ||
        err==X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE ||
        err==X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT ||
        err==X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Unknown certificate, will not abort yet");
      xio->verifyFlags|=GWEN_SSL_CERT_FLAGS_SIGNER_NOT_FOUND;
    }
    else if (err==X509_V_ERR_CERT_SIGNATURE_FAILURE ||
             err==X509_V_ERR_CRL_SIGNATURE_FAILURE)
      xio->verifySignatureFailure=1;
    else if (err==X509_V_ERR_CERT_REVOKED)
      xio->verifyFlags|=GWEN_SSL_CERT_FLAGS_REVOKED;
  }
  return 1;
}



char *GWEN_SyncIo_Tls_GetCertEntry(const X509_NAME *nm, int nid)
{
  const X509_NAME_ENTRY *e;
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
    /* X509_NAME_ENTRY is opaque since OpenSSL 1.1 */
    p=ASN1_STRING_get0_data(X509_NAME_ENTRY_get_data(e));
    len=ASN1_STRING_length(X509_NAME_ENTRY_get_data(e));
    if (p) {
      cpy=(char *)malloc(len+1);
      memmove(cpy, p, len);
      cpy[len]=0;
      return cpy;
    }
  } /* for */

  return 0;
}



GWEN_TIME *GWEN_SyncIo_Tls_ASN_GetTime(const ASN1_TIME *d)
{
  /* ASN1_TIME is opaque since OpenSSL 1.1 */
  if (ASN1_STRING_get0_data(d)) {
    const char *s;
    unsigned int i;
    GWEN_TIME *ti;
    int year, month, day;
    int hour, min, sec=0;
    int isUtc;

    s=(const char *)(ASN1_STRING_get0_data(d));
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



/* Hex string of a big number, with a leading zero byte if the top bit is set (as GnuTLS exports it) */
static void GWEN_SyncIo_Tls_BnToHex(const BIGNUM *bn, GWEN_BUFFER *kbuf)
{
  unsigned char buffer[1025];
  int l;

  l=BN_num_bytes(bn);
  if (l<=0 || l>(int)(sizeof(buffer)-1))
    return;
  buffer[0]=0;
  l=BN_bn2bin(bn, buffer+1);
  if (buffer[1] & 0x80)
    GWEN_Text_ToHexBuffer((const char *) buffer, l+1, kbuf, 0, 0, 0);
  else
    GWEN_Text_ToHexBuffer((const char *)(buffer+1), l, kbuf, 0, 0, 0);
}



GWEN_SSLCERTDESCR *GWEN_SyncIo_Tls_Cert2Descr(X509 *cert)
{
  const X509_NAME *nm;
  const ASN1_TIME *d;
  EVP_PKEY *pktmp;
  unsigned int  md_size;
  unsigned char md[EVP_MAX_MD_SIZE];
  char *s;
  GWEN_SSLCERTDESCR *cd;

  nm=X509_get_subject_name(cert);

  /* setup certificate descr */
  cd=GWEN_SslCertDescr_new();

  s=GWEN_SyncIo_Tls_GetCertEntry(nm, NID_commonName);
  if (s) {
    GWEN_SslCertDescr_SetCommonName(cd, s);
    free(s);
  }

  s=GWEN_SyncIo_Tls_GetCertEntry(nm, NID_organizationName);
  if (s) {
    GWEN_SslCertDescr_SetOrganizationName(cd, s);
    free(s);
  }

  s=GWEN_SyncIo_Tls_GetCertEntry(nm, NID_organizationalUnitName);
  if (s) {
    GWEN_SslCertDescr_SetOrganizationalUnitName(cd, s);
    free(s);
  }

  s=GWEN_SyncIo_Tls_GetCertEntry(nm, NID_countryName);
  if (s) {
    GWEN_SslCertDescr_SetCountryName(cd, s);
    free(s);
  }

  s=GWEN_SyncIo_Tls_GetCertEntry(nm, NID_localityName);
  if (s) {
    GWEN_SslCertDescr_SetLocalityName(cd, s);
    free(s);
  }

  s=GWEN_SyncIo_Tls_GetCertEntry(nm, NID_stateOrProvinceName);
  if (s) {
    GWEN_SslCertDescr_SetStateOrProvinceName(cd, s);
    free(s);
  }

  d=X509_get0_notBefore(cert);
  if (d) {
    GWEN_TIME *ti;

    ti=GWEN_SyncIo_Tls_ASN_GetTime(d);
    if (ti) {
      GWEN_SslCertDescr_SetNotBefore(cd, ti);
      GWEN_Time_free(ti);
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error in notBefore date");
    }
  }

  d=X509_get0_notAfter(cert);
  if (d) {
    GWEN_TIME *ti;

    ti=GWEN_SyncIo_Tls_ASN_GetTime(d);
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
    if (EVP_PKEY_is_a(pktmp, "RSA")) {
      BIGNUM *n=NULL;
      BIGNUM *e=NULL;
      GWEN_BUFFER *kbuf;

      kbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (EVP_PKEY_get_bn_param(pktmp, OSSL_PKEY_PARAM_RSA_N, &n)==1 && n) {
        GWEN_SyncIo_Tls_BnToHex(n, kbuf);
        if (GWEN_Buffer_GetUsedBytes(kbuf))
          GWEN_SslCertDescr_SetPubKeyModulus(cd, GWEN_Buffer_GetStart(kbuf));
        GWEN_Buffer_Reset(kbuf);
      }

      if (EVP_PKEY_get_bn_param(pktmp, OSSL_PKEY_PARAM_RSA_E, &e)==1 && e) {
        GWEN_SyncIo_Tls_BnToHex(e, kbuf);
        if (GWEN_Buffer_GetUsedBytes(kbuf))
          GWEN_SslCertDescr_SetPubKeyExponent(cd, GWEN_Buffer_GetStart(kbuf));
      }
      GWEN_Buffer_free(kbuf);
      BN_free(n);
      BN_free(e);
      ERR_clear_error();
    } /* if there is data for the public key */
    EVP_PKEY_free(pktmp);
  } /* if there is a pubkey */

  /* fingerprint (MD5) */
  md_size=0;
  if (!X509_digest(cert, EVP_md5(), md, &md_size)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Error building fingerprint of the certificate");
    ERR_clear_error();
  }
  if (!md_size) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Empty fingerprint of the certificate");
    GWEN_SslCertDescr_free(cd);
    return NULL;
  }
  else {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Text_ToHexBuffer(/* GCC4 pointer-signedness fix: */ (char *)md,
                                                                 md_size, dbuf, 2, ':', 0)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not convert fingerprint to hex");
    }
    else {
      GWEN_SslCertDescr_SetFingerPrint(cd, GWEN_Buffer_GetStart(dbuf));
    }
    GWEN_Buffer_free(dbuf);
  }

  /* fingerprint (SHA1) */
  md_size=0;
  if (!X509_digest(cert, EVP_sha1(), md, &md_size)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Error building fingerprint of the certificate");
    ERR_clear_error();
  }
  if (!md_size) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Empty fingerprint of the certificate");
    GWEN_SslCertDescr_free(cd);
    return NULL;
  }
  else {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Text_ToHexBuffer(/* GCC4 pointer-signedness fix: */ (char *)md,
                                                                 md_size, dbuf, 2, ':', 0)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not convert fingerprint to hex");
    }
    else {
      GWEN_SslCertDescr_SetFingerPrintSha1(cd, GWEN_Buffer_GetStart(dbuf));
    }
    GWEN_Buffer_free(dbuf);
  }

  /* fingerprint (SHA512) */
  md_size=0;
  if (!X509_digest(cert, EVP_sha512(), md, &md_size)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Error building fingerprint of the certificate");
    ERR_clear_error();
  }
  if (!md_size) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Empty fingerprint of the certificate");
    GWEN_SslCertDescr_free(cd);
    return NULL;
  }
  else {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Text_ToHexBuffer(/* GCC4 pointer-signedness fix: */ (char *)md,
                                                                 md_size, dbuf, 2, ':', 0)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not convert fingerprint to hex");
    }
    else {
      GWEN_SslCertDescr_SetFingerPrintSha512(cd, GWEN_Buffer_GetStart(dbuf));
    }
    GWEN_Buffer_free(dbuf);
  }

  return cd;
}



static int GWEN_SyncIo_Tls_IsIpAddress(const char *s)
{
  ASN1_OCTET_STRING *ip;

  ip=a2i_IPADDRESS(s);
  if (ip) {
    ASN1_OCTET_STRING_free(ip);
    return 1;
  }
  ERR_clear_error();
  return 0;
}



static int GWEN_SyncIo_Tls_LoadVerifyLocations(SSL_CTX *ssl_ctx, const char *CAfile)
{
  int rv;

  rv=SSL_CTX_load_verify_locations(ssl_ctx, CAfile, NULL);
  if (rv==0) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "SSL: Could not load certificate location "
             "(was: \"%s\")",
             CAfile);
    ERR_clear_error();
    return GWEN_ERROR_GENERIC;
  }
  DBG_INFO(GWEN_LOGDOMAIN, "Added trusted certs from [%s]", CAfile);
  return 0;
}



/* Returns 1 if the trust store holds certificates; certificates in a hashed directory are only looked up on use and
 * are not counted */
static int GWEN_SyncIo_Tls_HasTrustedCerts(SSL_CTX *ssl_ctx)
{
  STACK_OF(X509_OBJECT) *objs;
  int n;

  objs=X509_STORE_get1_objects(SSL_CTX_get_cert_store(ssl_ctx));
  n=objs?sk_X509_OBJECT_num(objs):0;
  sk_X509_OBJECT_pop_free(objs, X509_OBJECT_free);
  return (n>0)?1:0;
}



static int GWEN_SyncIo_Tls_AddTrustedCas(SSL_CTX *ssl_ctx)
{
  int trustFileSet=0;

#if GWEN_TLS_USE_SYSTEM_CERTIFICATES
  /* the system's default trust store as configured in OpenSSL; this also succeeds when the default locations do
   * not exist (e.g. an OpenSSL installed by a package manager into its own prefix), so check what was loaded */
  if (SSL_CTX_set_default_verify_paths(ssl_ctx)!=1) {
    DBG_WARN(GWEN_LOGDOMAIN, "SSL_CTX_set_default_verify_paths() failed");
    ERR_clear_error();
  }
  if (GWEN_SyncIo_Tls_HasTrustedCerts(ssl_ctx)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Added default trusted certs from system");
    trustFileSet=1;
  }
# ifdef OS_WIN32
  /* the Windows certificate store (OpenSSL >= 3.2) */
  if (SSL_CTX_load_verify_store(ssl_ctx, "org.openssl.winstore:")==1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Added trusted certs from Windows certificate store");
    trustFileSet=1;
  }
  else {
    DBG_WARN(GWEN_LOGDOMAIN, "Could not load Windows certificate store");
    ERR_clear_error();
  }
# endif
#endif

#ifdef OS_WIN32
  if (trustFileSet==0) {
    char defaultPath[2*_MAX_PATH+1];
    const char *defaultFile = "ca-bundle.crt";
    GWEN_STRINGLIST *paths;
    GWEN_BUFFER *nbuf;
    int rv;

    if (GWEN_Directory_GetPrefixDirectory(defaultPath, sizeof(defaultPath))) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not get install prefix");
      return GWEN_ERROR_GENERIC;
    }
    if (strcat_s(defaultPath, sizeof(defaultPath), "\\share\\gwenhywfar")) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No memory on creating search path");
      return GWEN_ERROR_GENERIC;
    }

    paths=GWEN_StringList_new();
    GWEN_StringList_AppendString(paths, defaultPath, 0, 0);

    nbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Directory_FindFileInPaths(paths, defaultFile, nbuf);
    GWEN_StringList_free(paths);
    if (rv==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Using default ca-bundle from [%s]", GWEN_Buffer_GetStart(nbuf));
      if (GWEN_SyncIo_Tls_LoadVerifyLocations(ssl_ctx, GWEN_Buffer_GetStart(nbuf))==0)
        trustFileSet=1;
    }
    GWEN_Buffer_free(nbuf);
  }
#else
  if (trustFileSet==0) {
    int i;

    for (i=0; SYNCIO_TLS_SYSTEM_CERTFILES[i]; i++) {
      const char *sCertFile=SYNCIO_TLS_SYSTEM_CERTFILES[i];

      if (0==GWEN_Directory_GetPath(sCertFile, GWEN_PATH_FLAGS_NAMEMUSTEXIST | GWEN_PATH_FLAGS_VARIABLE)) {
        DBG_INFO(GWEN_LOGDOMAIN, "Found system-wide cert bundle in %s", sCertFile);
        if (GWEN_SyncIo_Tls_LoadVerifyLocations(ssl_ctx, sCertFile)==0)
          trustFileSet=1;
        break;
      }
    }
  }
#endif

  if (trustFileSet==0) {
    DBG_WARN(GWEN_LOGDOMAIN, "No default bundle file found");
  }
  return 0;
}



int GWEN_SyncIo_Tls_Setup(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;
  int rv;
  uint32_t lflags;
  GWEN_SYNCIO *baseIo;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  lflags=GWEN_SyncIo_GetFlags(sio);

  /* enable all workarounds for bugs in other SSL implementation for
   * maximum compatibility */
  SSL_CTX_set_options(xio->ssl_ctx, SSL_OP_ALL);

  if (xio->localCertFile && xio->localKeyFile) {
    char errBuf[256];

    /* load own certificate file */
    DBG_DEBUG(GWEN_LOGDOMAIN, "Loading certificate and keys");
    rv=SSL_CTX_use_certificate_chain_file(xio->ssl_ctx,
                                          xio->localCertFile);
    if (!rv) {
      GWEN_SyncIo_Tls_ErrorText(SSL_ERROR_SSL, errBuf, sizeof(errBuf));
      DBG_INFO(GWEN_LOGDOMAIN, "SSL error reading certfile: %s", errBuf);
      return -1;
    }

    /* load keys */
    rv=SSL_CTX_use_PrivateKey_file(xio->ssl_ctx,
                                   xio->localKeyFile,
                                   SSL_FILETYPE_PEM);
    if (!rv) {
      GWEN_SyncIo_Tls_ErrorText(SSL_ERROR_SSL, errBuf, sizeof(errBuf));
      DBG_INFO(GWEN_LOGDOMAIN, "SSL error reading keyfile: %s", errBuf);
      return -1;
    }
    if (!SSL_CTX_check_private_key(xio->ssl_ctx)) {
      DBG_INFO(GWEN_LOGDOMAIN,
               "Private key does not match the certificate public key");
      return -1;
    }
  }

  /* setup locations of certificates */
  if (lflags & GWEN_SYNCIO_TLS_FLAGS_ADD_TRUSTED_CAS) {
    rv=GWEN_SyncIo_Tls_AddTrustedCas(xio->ssl_ctx);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  if (xio->localTrustFile) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Loading certificate locations");
    rv=GWEN_SyncIo_Tls_LoadVerifyLocations(xio->ssl_ctx, xio->localTrustFile);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* setup server stuff (OpenSSL accepts X.509 v1 trust anchors, GWEN_SYNCIO_TLS_FLAGS_ALLOW_V1_CA_CRT needs
   * no setting) */
  if (lflags & GWEN_SYNCIO_FLAGS_PASSIVE) {
    if (lflags & GWEN_SYNCIO_TLS_FLAGS_REQUEST_CERT)
      SSL_CTX_set_verify(xio->ssl_ctx,
                         SSL_VERIFY_PEER |
                         SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                         GWEN_SyncIo_Tls_VerifyCallBack);
    else
      SSL_CTX_set_verify(xio->ssl_ctx,
                         SSL_VERIFY_PEER,
                         GWEN_SyncIo_Tls_VerifyCallBack);
  }
  else {
    /* active connection */
    SSL_CTX_set_verify(xio->ssl_ctx,
                       SSL_VERIFY_PEER,
                       GWEN_SyncIo_Tls_VerifyCallBack);
  }

  xio->ssl=SSL_new(xio->ssl_ctx);
  if (xio->ssl==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "SSL_new() failed");
    return -1;
  }
  SSL_set_ex_data(xio->ssl, gwen_syncio_tls_sslidx, sio);

  if (lflags & GWEN_SYNCIO_FLAGS_PASSIVE)
    SSL_set_accept_state(xio->ssl);
  else {
    SSL_set_connect_state(xio->ssl);
    /* if hostname set try to set it (SNI must not contain an IP address) */
    if (xio->hostName && !GWEN_SyncIo_Tls_IsIpAddress(xio->hostName)) {
      if (SSL_set_tlsext_host_name(xio->ssl, xio->hostName)!=1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "SSL_set_tlsext_host_name(%s) failed, ignoring", xio->hostName);
        ERR_clear_error();
      }
    }
  }

  /* set info callback */
  SSL_set_info_callback(xio->ssl, GWEN_SyncIo_Tls_InfoCallBack);

  /* tell SSL to use our BIO */
  xio->bio=BIO_syncio_new(baseIo);
  if (xio->bio==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not create BIO");
    return -1;
  }
  SSL_set_bio(xio->ssl, xio->bio, xio->bio);

  return 0;
}



int GWEN_SyncIo_Tls_Prepare(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;
  uint32_t lflags;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  lflags=GWEN_SyncIo_GetFlags(sio);
  DBG_INFO(GWEN_LOGDOMAIN, "Preparing SSL (%08x)", lflags);

  if (CRYPTO_THREAD_run_once(&gwen_syncio_tls_once, GWEN_SyncIo_Tls_InitOnce)!=1 ||
      gwen_syncio_tls_sslidx==-1 || bio_syncio_methods==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not initialize OpenSSL data");
    return GWEN_ERROR_GENERIC;
  }

  /* socket ready now setup SSL */
  ERR_clear_error();
  if (lflags & GWEN_SYNCIO_FLAGS_PASSIVE)
    xio->ssl_ctx=SSL_CTX_new(TLS_server_method());
  else
    xio->ssl_ctx=SSL_CTX_new(TLS_client_method());

  if (xio->ssl_ctx==NULL || GWEN_SyncIo_Tls_Setup(sio)) {
    SSL_free(xio->ssl);
    xio->ssl=0;
    SSL_CTX_free(xio->ssl_ctx);
    xio->ssl_ctx=0;
    xio->bio=0; /* FIXME: Does SSL_free free this ? */
    ERR_clear_error();
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not setup SSL connection");
    return GWEN_ERROR_GENERIC;
  }

  xio->verifyFlags=0;
  xio->verifySignatureFailure=0;
  xio->prepared=1;

  return 0;
}



void GWEN_SyncIo_Tls_UndoPrepare(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  if (xio->prepared) {
    SSL_free(xio->ssl);
    xio->ssl=0;
    SSL_CTX_free(xio->ssl_ctx);
    xio->ssl_ctx=0;
    xio->bio=0; /* FIXME: Does SSL_free free this ? */
    xio->prepared=0;
  }
}



int GWEN_SyncIo_Tls_GetPeerCert(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;
  X509 *cert;
  char *certbuf;
  GWEN_SSLCERTDESCR *cd;
  time_t t0;
  uint32_t errFlags=0;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  if (xio->peerCertDescr) {
    GWEN_SslCertDescr_free(xio->peerCertDescr);
    xio->peerCertDescr=NULL;
  }
  xio->peerCertFlags=0;

  t0=time(NULL);
  if (t0<0) {
    DBG_WARN(GWEN_LOGDOMAIN, "Could not get system time");
    errFlags|=GWEN_SSL_CERT_FLAGS_SYSTEM;
  }

  cert=SSL_get1_peer_certificate(xio->ssl);
  if (!cert) {
    DBG_INFO(GWEN_LOGDOMAIN, "Peer did not send a certificate");
    return GWEN_ERROR_NO_DATA;
  }

  certbuf=X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
  DBG_INFO(GWEN_LOGDOMAIN, "Got a certificate: %s", certbuf);
  OPENSSL_free(certbuf);

  /* setup certificate */
  cd=GWEN_SyncIo_Tls_Cert2Descr(cert);
  if (cd==NULL) {
    X509_free(cert);
    return GWEN_ERROR_GENERIC;
  }

  /* results of the verification during the handshake */
  errFlags|=xio->verifyFlags;
  if (errFlags & GWEN_SSL_CERT_FLAGS_SIGNER_NOT_FOUND) {
    DBG_INFO(GWEN_LOGDOMAIN, "Signer not found");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning, I18N("Signer not found"));
  }
  if (xio->verifySignatureFailure) {
    DBG_INFO(GWEN_LOGDOMAIN, "Certificate signature failure");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning, I18N("Certificate signature failure"));
  }
  if (errFlags & GWEN_SSL_CERT_FLAGS_INVALID) {
    DBG_INFO(GWEN_LOGDOMAIN, "Certificate is not trusted");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning, I18N("Certificate is not trusted"));
  }
  if (errFlags & GWEN_SSL_CERT_FLAGS_REVOKED) {
    DBG_INFO(GWEN_LOGDOMAIN, "Certificate has been revoked");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning, I18N("Certificate has been revoked"));
  }

  /* host name */
  if (xio->hostName) {
    ASN1_OCTET_STRING *ip;
    int matches;

    DBG_INFO(GWEN_LOGDOMAIN, "Checking hostname [%s]", xio->hostName);
    ip=a2i_IPADDRESS(xio->hostName);
    if (ip) {
      matches=(X509_check_ip(cert, ASN1_STRING_get0_data(ip), ASN1_STRING_length(ip), 0)==1);
      ASN1_OCTET_STRING_free(ip);
    }
    else
      matches=(X509_check_host(cert, xio->hostName, 0, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS, NULL)==1);
    ERR_clear_error();
    if (!matches) {
      DBG_WARN(GWEN_LOGDOMAIN, "Certificate was not issued for this host");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning, I18N("Certificate was not issued for this host"));
      errFlags|=GWEN_SSL_CERT_FLAGS_BAD_HOSTNAME;
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "Cert is for this server");
    }
  }
  else {
    DBG_WARN(GWEN_LOGDOMAIN, "Hostname is not set, unable to verify the sender");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning, I18N("No hostname to verify the sender!"));
  }

  /* activation and expiration time */
  if (GWEN_SslCertDescr_GetNotBefore(cd)==NULL || GWEN_SslCertDescr_GetNotAfter(cd)==NULL)
    errFlags|=GWEN_SSL_CERT_FLAGS_BAD_DATA;
  if (ASN1_TIME_cmp_time_t(X509_get0_notBefore(cert), t0)==1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Cert is not yet active");
    errFlags|=GWEN_SSL_CERT_FLAGS_NOT_ACTIVE;
  }
  if (ASN1_TIME_cmp_time_t(X509_get0_notAfter(cert), t0)==-1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Cert has expired");
    errFlags|=GWEN_SSL_CERT_FLAGS_EXPIRED;
  }
  X509_free(cert);

  /* done */
  if (errFlags)
    GWEN_SslCertDescr_SetIsError(cd, 1);
  else
    errFlags|=GWEN_SSL_CERT_FLAGS_OK;

  GWEN_SyncIo_Tls_SetCertStatus(cd, errFlags);

  xio->peerCertDescr=cd;
  xio->peerCertFlags=errFlags;

  return 0;
}



void GWEN_SyncIo_Tls_ShowCipherInfo(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  /* show info about used cipher */
  if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Notice) {
    const SSL_CIPHER *ci;
    char buffer[256];
    const char *p;

    ci=SSL_get_current_cipher(xio->ssl);
    assert(ci);

    p=SSL_CIPHER_description(ci, buffer, sizeof(buffer));
    DBG_INFO(GWEN_LOGDOMAIN, "Connected using \"%s\"", p);
  }
}



int GWEN_SyncIo_Tls_Handshake(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  for (;;) {
    /* check for established SSL */
    ERR_clear_error();
    if (GWEN_SyncIo_GetFlags(sio) & GWEN_SYNCIO_FLAGS_PASSIVE) {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Calling accept");
      rv=SSL_accept(xio->ssl);
    }
    else {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Calling connect");
      rv=SSL_connect(xio->ssl);
    }

    if (rv!=1) {
      int sslerr;
      char errBuf[256];

      sslerr=SSL_get_error(xio->ssl, rv);
      if (sslerr==SSL_ERROR_WANT_READ ||
          sslerr==SSL_ERROR_WANT_WRITE)
        continue; /* would block */

      GWEN_SyncIo_Tls_ErrorText(sslerr, errBuf, sizeof(errBuf));
      if (sslerr==SSL_ERROR_SYSCALL && errno==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Connection just went down (%d: %s)",
                  sslerr, errBuf);
      }
      else if (sslerr==SSL_ERROR_ZERO_RETURN) {
        DBG_INFO(GWEN_LOGDOMAIN, "Connection closed");
      }
      else {
        DBG_INFO(GWEN_LOGDOMAIN, "List of pending SSL errors:");
        if (GWEN_Logger_GetLevel(GWEN_LOGDOMAIN)>=GWEN_LoggerLevel_Info)
          ERR_print_errors_fp(stderr); /* DEBUG */
        DBG_INFO(GWEN_LOGDOMAIN, "SSL error: %s (%d)", errBuf, sslerr);
      }
      GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Error, I18N("TLS Handshake Error: %d (%s)"), sslerr, errBuf);
      ERR_clear_error();
      return GWEN_ERROR_SSL;
    }
    break;
  }

  /* now logically connected */
  DBG_INFO(GWEN_LOGDOMAIN, "Logically connected");
  return 0;
}

#else /* GWEN_TLS_OPENSSL */



#if GWEN_TLS_USE_SYSTEM_CERTIFICATES
# ifndef OS_WIN32
static int GWEN_SyncIo_Tls_AddCaCertFolder(GWEN_SYNCIO *sio, const char *folder)
{
  GWEN_SYNCIO_TLS *xio;
  int rv;
  int successfullTustFileCount=0;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  if (folder && *folder) {
    GWEN_STRINGLIST *fileList;

    fileList=GWEN_StringList_new();
    rv=GWEN_Directory_GetMatchingFilesRecursively(folder, fileList, "*.crt");
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error reading list of certificate files (%d) in folder [%s]",
                rv, folder);
    }
    else {
      GWEN_STRINGLISTENTRY *se;

      se=GWEN_StringList_FirstEntry(fileList);
      while (se) {
        const char *s;

        s=GWEN_StringListEntry_Data(se);
        if (s && *s) {
          rv=gnutls_certificate_set_x509_trust_file(xio->credentials,
                                                    s,
                                                    GNUTLS_X509_FMT_PEM);
          if (rv<=0) {
            DBG_WARN(GWEN_LOGDOMAIN,
                     "gnutls_certificate_set_x509_trust_file(%s): %d (%s)",
                     s, rv, gnutls_strerror(rv));
          }
          else {
            DBG_INFO(GWEN_LOGDOMAIN, "Added %d trusted certs from [%s]", rv, s);
            successfullTustFileCount++;
          }
        }

        se=GWEN_StringListEntry_Next(se);
      } /* while */
    }
    GWEN_StringList_free(fileList);
  }

  if (successfullTustFileCount==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No files added from folder [%s]", folder);
  }

  return successfullTustFileCount;
}
# endif
#endif



int GWEN_SyncIo_Tls_Prepare(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;
  int rv;
  uint32_t lflags;
  const char *custom_ciphers;
  const char *errPos=NULL;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  lflags=GWEN_SyncIo_GetFlags(sio);
  DBG_INFO(GWEN_LOGDOMAIN, "Preparing SSL (%08x)", lflags);

  /* init session */
  if (lflags & GWEN_SYNCIO_FLAGS_PASSIVE) {
    DBG_INFO(GWEN_LOGDOMAIN, "Init as server");
    rv=gnutls_init(&xio->session, GNUTLS_SERVER);
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Init as client");
    rv=gnutls_init(&xio->session, GNUTLS_CLIENT);
  }
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_init: %d (%s)", rv, gnutls_strerror(rv));
    return GWEN_ERROR_GENERIC;
  }

  /* set cipher priorities */
  custom_ciphers=getenv("GWEN_TLS_CIPHER_PRIORITIES");
  /* TODO: make custom ciphers configurable as priority string? */
  if (custom_ciphers && *custom_ciphers) { /* use cipher list from env var */
    GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Info, I18N("TLS: SSL cipher priority list: %s"), custom_ciphers);
    rv=gnutls_priority_set_direct(xio->session, custom_ciphers, &errPos);
    if (rv!=GNUTLS_E_SUCCESS) {
      DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_priority_set_direct using '%s' failed: %s (%d) [%s]",
                custom_ciphers, gnutls_strerror(rv), rv, errPos?errPos:"");
      gnutls_deinit(xio->session);
      return GWEN_ERROR_GENERIC;
    }
  }
  else {   /* use default ciphers from GnuTLS */
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice, I18N("Using GnuTLS default ciphers."));
    rv=gnutls_set_default_priority(xio->session);
    if (rv!=GNUTLS_E_SUCCESS) {
      DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_set_default_priority failed: %s (%d)", gnutls_strerror(rv), rv);
      gnutls_deinit(xio->session);
      return GWEN_ERROR_GENERIC;
    }
  }

  /* protect against too-many-known-ca problem */
  gnutls_handshake_set_max_packet_length(xio->session, 64*1024);

  /* let a server request peer certs */
  if ((lflags & GWEN_SYNCIO_FLAGS_PASSIVE) &&
      (lflags & GWEN_SYNCIO_TLS_FLAGS_REQUEST_CERT))
    gnutls_certificate_server_set_request(xio->session, GNUTLS_CERT_REQUIRE);

  /* prepare cert credentials */
  rv=gnutls_certificate_allocate_credentials(&xio->credentials);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_certificate_allocate_credentials: %d (%s)", rv, gnutls_strerror(rv));
    gnutls_deinit(xio->session);
    return GWEN_ERROR_GENERIC;
  }

  /* possibly set key file and cert file */
  if (xio->localCertFile && xio->localKeyFile) {
    rv=gnutls_certificate_set_x509_key_file(xio->credentials,
                                            xio->localCertFile,
                                            xio->localKeyFile,
                                            GNUTLS_X509_FMT_PEM);
    if (rv<0) {
      if (rv) {
        DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_certificate_set_x509_key_file: %d (%s)", rv, gnutls_strerror(rv));
        gnutls_certificate_free_credentials(xio->credentials);
        gnutls_deinit(xio->session);
        return GWEN_ERROR_GENERIC;
      }
    }
  }

  /* find default trust file if none is selected */
  if (lflags & GWEN_SYNCIO_TLS_FLAGS_ADD_TRUSTED_CAS) {
    int trustFileSet=0;

#if GWEN_TLS_USE_SYSTEM_CERTIFICATES
    /* disable setting of default trust file as discussed on aqbanking-users.
     * The rationale is that without this file being set gnutls should behave
     * correctly on each system.
     * On Linux systems it should use the standard mechanism of the underlying
     * distribution. On Windows the default CA store should be used (if given
     * "--with-default-trust-store-file" to "./configure" of GNUTLS).
     */
    if (trustFileSet==0) {
      /* Adds the system's default trusted CAs in order to verify client or server certificates. */
      rv=gnutls_certificate_set_x509_system_trust(xio->credentials);
      if (rv<=0) {
        DBG_WARN(GWEN_LOGDOMAIN, "gnutls_certificate_set_x509_system_trust: %d (%s)", rv, gnutls_strerror(rv));
      }
      else {
        DBG_INFO(GWEN_LOGDOMAIN, "Added %d default trusted certs from system", rv);
        trustFileSet=1;
      }
    }
#endif

    /* try to find OpenSSL certificates */
# ifdef OS_WIN32
    if (trustFileSet==0) {
      char defaultPath[2*_MAX_PATH+1];
      const char *defaultFile = "ca-bundle.crt";
      GWEN_STRINGLIST *paths;
      GWEN_BUFFER *nbuf;

      if (GWEN_Directory_GetPrefixDirectory(defaultPath, sizeof(defaultPath))) {
        DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_certificate_set_x509_key_file: could not get install prefix");
        return GWEN_ERROR_GENERIC;
      }
      if (strcat_s(defaultPath, sizeof(defaultPath), "\\share\\gwenhywfar")) {
        DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_certificate_set_x509_key_file: no memory on creating search path");
        return GWEN_ERROR_GENERIC;
      }

      paths=GWEN_StringList_new();
      GWEN_StringList_AppendString(paths, defaultPath, 0, 0);

      nbuf=GWEN_Buffer_new(0, 256, 0, 1);
      rv=GWEN_Directory_FindFileInPaths(paths, defaultFile, nbuf);
      GWEN_StringList_free(paths);
      if (rv==0) {
        DBG_INFO(GWEN_LOGDOMAIN,
                 "Using default ca-bundle from [%s]",
                 GWEN_Buffer_GetStart(nbuf));

        rv=gnutls_certificate_set_x509_trust_file(xio->credentials,
                                                  GWEN_Buffer_GetStart(nbuf),
                                                  GNUTLS_X509_FMT_PEM);
        if (rv<=0) {
          DBG_WARN(GWEN_LOGDOMAIN,
                   "gnutls_certificate_set_x509_trust_file(%s): %d (%s)",
                   GWEN_Buffer_GetStart(nbuf), rv, gnutls_strerror(rv));
        }
        else {
          DBG_INFO(GWEN_LOGDOMAIN,
                   "Added %d trusted certs from [%s]", rv, GWEN_Buffer_GetStart(nbuf));
          trustFileSet=1;
        }
      }
      GWEN_Buffer_free(nbuf);
    }
# endif


# ifndef OS_WIN32
    /* try to finde certificate bundle */
    if (trustFileSet==0) {
      int i;
      const char *sCertFile=NULL;

      for (i=0; ; i++) {
        sCertFile=SYNCIO_TLS_SYSTEM_CERTFILES[i];
        if (sCertFile==NULL)
          break;
        if (0==GWEN_Directory_GetPath(sCertFile, GWEN_PATH_FLAGS_NAMEMUSTEXIST | GWEN_PATH_FLAGS_VARIABLE)) {
          DBG_INFO(GWEN_LOGDOMAIN, "Found system-wide cert bundle in %s", sCertFile);
          break;
        }
      }

      if (sCertFile && *sCertFile) {
        rv=gnutls_certificate_set_x509_trust_file(xio->credentials, sCertFile, GNUTLS_X509_FMT_PEM);
        if (rv<=0) {
          DBG_WARN(GWEN_LOGDOMAIN, "gnutls_certificate_set_x509_trust_file(%s): %d (%s)", sCertFile, rv, gnutls_strerror(rv));
        }
        else {
          DBG_INFO(GWEN_LOGDOMAIN, "Added %d trusted certs from [%s]", rv, sCertFile);
          trustFileSet=1;
        }
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "No system-wide certificate bundle found.");
      }
    }

    /* try to find ca-certificates (at least available on Debian systems) */
    if (trustFileSet==0) {
      rv=GWEN_Directory_GetPath("/usr/share/ca-certificates", GWEN_PATH_FLAGS_NAMEMUSTEXIST);
      if (rv>=0) {
        rv=GWEN_SyncIo_Tls_AddCaCertFolder(sio, "/usr/share/ca-certificates");
        if (rv<=0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        }
        else {
          trustFileSet=1;
        }
      }
    }

# endif


    if (trustFileSet==0) {

      /* TODO: use gnutls_certificate_set_x509_system_trust() */
      trustFileSet=1;
    }



    if (trustFileSet==0) {
      DBG_WARN(GWEN_LOGDOMAIN, "No default bundle file found");
    }
  }

  /* possibly set trust file */
  if (xio->localTrustFile) {
    rv=gnutls_certificate_set_x509_trust_file(xio->credentials,
                                              xio->localTrustFile,
                                              GNUTLS_X509_FMT_PEM);
    if (rv<=0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "gnutls_certificate_set_x509_trust_file(%s): %d (%s)",
                (xio->localTrustFile)?(xio->localTrustFile):"-none-",
                rv, gnutls_strerror(rv));
      gnutls_certificate_free_credentials(xio->credentials);
      gnutls_deinit(xio->session);
      return GWEN_ERROR_GENERIC;
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN,
               "Added %d trusted certs", rv);
    }
  }

  /* possibly set DH params */
  if (xio->dhParamFile) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_SyncIo_Tls__readFile(xio->dhParamFile, dbuf);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(dbuf);
      gnutls_certificate_free_credentials(xio->credentials);
      gnutls_deinit(xio->session);
      return rv;
    }
    else {
      gnutls_datum_t d;
      gnutls_dh_params_t dh_params=NULL;

      rv=gnutls_dh_params_init(&dh_params);
      if (rv<0) {
        GWEN_Buffer_free(dbuf);
        DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_dh_params_init: %d (%s)", rv, gnutls_strerror(rv));
        gnutls_certificate_free_credentials(xio->credentials);
        gnutls_deinit(xio->session);
        return GWEN_ERROR_GENERIC;
      }

      d.size=GWEN_Buffer_GetUsedBytes(dbuf);
      d.data=(unsigned char *)GWEN_Buffer_GetStart(dbuf);

      rv=gnutls_dh_params_import_pkcs3(dh_params, &d, GNUTLS_X509_FMT_PEM);
      if (rv<0) {
        GWEN_Buffer_free(dbuf);
        DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_dh_params_import_pkcs3: %d (%s)", rv, gnutls_strerror(rv));
        gnutls_certificate_free_credentials(xio->credentials);
        gnutls_deinit(xio->session);
        return GWEN_ERROR_GENERIC;
      }
      GWEN_Buffer_free(dbuf);

      gnutls_certificate_set_dh_params(xio->credentials, dh_params);
    }
  }

  /* set credentials in TLS session */
  rv=gnutls_credentials_set(xio->session, GNUTLS_CRD_CERTIFICATE, xio->credentials);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_credentials_set: %d (%s)", rv, gnutls_strerror(rv));
    gnutls_certificate_free_credentials(xio->credentials);
    gnutls_deinit(xio->session);
    return GWEN_ERROR_GENERIC;
  }

  /* if hostname set try to set it */
  if (xio->hostName) {
    rv=gnutls_server_name_set(xio->session, GNUTLS_NAME_DNS, xio->hostName, strlen(xio->hostName));
    if (rv!=GNUTLS_E_SUCCESS) {
      DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_server_name_set: %d (%s), ignoring", rv, gnutls_strerror(rv));
    }
  }

  /* we use our own push/pull functions */
  gnutls_transport_set_ptr(xio->session, (gnutls_transport_ptr_t)sio);
  gnutls_transport_set_push_function(xio->session, GWEN_SyncIo_Tls_Push);
  gnutls_transport_set_pull_function(xio->session, GWEN_SyncIo_Tls_Pull);
#if GNUTLS_VERSION_NUMBER < 0x020c00
  /* This function must be set to 0 in GNUTLS versions < 2.12.0 because we use
   * custom push/pull functions.
   * In GNUTLS 2.12.x this is set to 0 and since version 3 this functions is removed
   * completely.
   * So we only call this function now for GNUTLS < 2.12.0.
   */
  gnutls_transport_set_lowat(xio->session, 0);
#endif

  xio->prepared=1;

  return 0;
}



void GWEN_SyncIo_Tls_UndoPrepare(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  if (xio->prepared) {
    gnutls_certificate_free_credentials(xio->credentials);
    gnutls_deinit(xio->session);
    xio->prepared=0;
  }
}



int GWEN_SyncIo_Tls_GetPeerCert(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;
  const gnutls_datum_t *cert_list;
  unsigned int cert_list_size;
  size_t size;
  GWEN_SSLCERTDESCR *certDescr;
  char buffer1[64];
  time_t t0;
  int rv;
  uint32_t lflags;
  uint32_t errFlags=0;
  int i;
  unsigned int status;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  lflags=GWEN_SyncIo_GetFlags(sio);

  if (xio->peerCertDescr) {
    GWEN_SslCertDescr_free(xio->peerCertDescr);
    xio->peerCertDescr=NULL;
  }
  xio->peerCertFlags=0;

  t0=time(NULL);
  if (t0<0) {
    DBG_WARN(GWEN_LOGDOMAIN, "Could not get system time");
    errFlags|=GWEN_SSL_CERT_FLAGS_SYSTEM;
  }

  /* create new cert description, check cert on the fly */
  certDescr=GWEN_SslCertDescr_new();

  /* some general tests */
  if (lflags & GWEN_SYNCIO_TLS_FLAGS_ALLOW_V1_CA_CRT)
    gnutls_certificate_set_verify_flags(xio->credentials,
                                        GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT);

  rv=gnutls_certificate_verify_peers2(xio->session, &status);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "gnutls_certificate_verify_peers2: %d (%s)", rv, gnutls_strerror(rv));
    GWEN_SslCertDescr_free(certDescr);
    return GWEN_ERROR_SSL_SECURITY;
  }

  if (gnutls_certificate_type_get(xio->session)!=GNUTLS_CRT_X509) {
    DBG_INFO(GWEN_LOGDOMAIN, "Certificate is not X.509");

    GWEN_SslCertDescr_free(certDescr);
    return GWEN_ERROR_SSL_SECURITY;
  }

  if (status & GNUTLS_CERT_SIGNER_NOT_FOUND) {
    DBG_INFO(GWEN_LOGDOMAIN, "Signer not found");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning,
                         I18N("Signer not found"));
    errFlags|=GWEN_SSL_CERT_FLAGS_SIGNER_NOT_FOUND;
  }

  if (status & GNUTLS_CERT_SIGNATURE_FAILURE) {
    DBG_INFO(GWEN_LOGDOMAIN, "Certificate signature failure");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning,
                         I18N("Certificate signature failure"));
    errFlags|=GWEN_SSL_CERT_FLAGS_INVALID;
  }

  if (status & GNUTLS_CERT_INVALID) {
    DBG_INFO(GWEN_LOGDOMAIN, "Certificate is not trusted");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning,
                         I18N("Certificate is not trusted"));
    errFlags|=GWEN_SSL_CERT_FLAGS_INVALID;
  }

  if (status & GNUTLS_CERT_REVOKED) {
    DBG_INFO(GWEN_LOGDOMAIN, "Certificate has been revoked");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning,
                         I18N("Certificate has been revoked"));
    errFlags|=GWEN_SSL_CERT_FLAGS_REVOKED;
  }

  cert_list=gnutls_certificate_get_peers(xio->session, &cert_list_size);
  if (cert_list==NULL || cert_list_size==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "No peer certificates found");
    return GWEN_ERROR_NO_DATA;
  }

  for (i=0; i<(int) cert_list_size; i++) {
    gnutls_x509_crt_t cert;
    time_t t;

    rv=gnutls_x509_crt_init(&cert);
    if (rv!=0) {
      DBG_INFO(GWEN_LOGDOMAIN, "gnutls_x509_crt_init: %d (%s)", rv, gnutls_strerror(rv));
      return GWEN_ERROR_GENERIC;
    }

    rv=gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER); /* TODO: shouldn't we use the index?? */
    if (rv!=0) {
      DBG_INFO(GWEN_LOGDOMAIN, "gnutls_x509_crt_import: %d (%s)", rv, gnutls_strerror(rv));
      gnutls_x509_crt_deinit(cert);
      return GWEN_ERROR_GENERIC;
    }

    if (i==0) {
      gnutls_datum_t n= {NULL, 0};
      gnutls_datum_t e= {NULL, 0};

      /* get public key from cert, if any */
      rv=gnutls_x509_crt_get_pk_rsa_raw(cert, &n, &e);
      if (rv!=0) {
        DBG_INFO(GWEN_LOGDOMAIN, "gnutls_x509_crt_get_pk_rsa_raw: %d (%s)", rv, gnutls_strerror(rv));
      }
      else {
        GWEN_BUFFER *kbuf;

        DBG_INFO(GWEN_LOGDOMAIN, "Key stored within certificate, extracting (modlen=%d, explen=%d)",
                 n.size, e.size);

        kbuf=GWEN_Buffer_new(0, 256, 0, 1);

        if (n.data && n.size) {
          /* store public modulus */
          GWEN_Text_ToHexBuffer((const char *)(n.data), n.size, kbuf, 0, 0, 0);
          GWEN_SslCertDescr_SetPubKeyModulus(certDescr, GWEN_Buffer_GetStart(kbuf));
          GWEN_Buffer_Reset(kbuf);
        }

        if (e.data && e.size) {
          /* store public exponent */
          GWEN_Text_ToHexBuffer((const char *)(e.data), e.size, kbuf, 0, 0, 0);
          GWEN_SslCertDescr_SetPubKeyExponent(certDescr, GWEN_Buffer_GetStart(kbuf));
          GWEN_Buffer_Reset(kbuf);
        }

        GWEN_Buffer_free(kbuf);
        if (n.data)
          gcry_free(n.data);
        if (e.data)
          gcry_free(e.data);
      }

      /* get fingerprint (MD5) */
      size=16;
      rv=gnutls_x509_crt_get_fingerprint(cert, GNUTLS_DIG_MD5, buffer1, &size);
      if (rv!=0) {
        DBG_INFO(GWEN_LOGDOMAIN, "gnutls_x509_crt_get_fingerprint(MD5): %d (%s)", rv, gnutls_strerror(rv));
        GWEN_SslCertDescr_free(certDescr);
        gnutls_x509_crt_deinit(cert);
        return GWEN_ERROR_GENERIC;
      }
      else {
        GWEN_BUFFER *dbuf;

        dbuf=GWEN_Buffer_new(0, 256, 0, 1);
        if (GWEN_Text_ToHexBuffer(/* GCC4 pointer-signedness fix: */ buffer1,
                                                                     size, dbuf, 2, ':', 0)) {
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "Could not convert fingerprint to hex");
        }
        else {
          GWEN_SslCertDescr_SetFingerPrint(certDescr, GWEN_Buffer_GetStart(dbuf));
        }
        GWEN_Buffer_free(dbuf);
      }

      /* get fingerprint (SHA1) */
      size=sizeof(buffer1);
      rv=gnutls_x509_crt_get_fingerprint(cert, GNUTLS_DIG_SHA1, buffer1, &size);
      if (rv!=0) {
        DBG_INFO(GWEN_LOGDOMAIN, "gnutls_x509_crt_get_fingerprint(SHA1): %d (%s)", rv, gnutls_strerror(rv));
        GWEN_SslCertDescr_free(certDescr);
        gnutls_x509_crt_deinit(cert);
        return GWEN_ERROR_GENERIC;
      }
      else {
        GWEN_BUFFER *dbuf;

        dbuf=GWEN_Buffer_new(0, 256, 0, 1);
        if (GWEN_Text_ToHexBuffer(/* GCC4 pointer-signedness fix: */ buffer1,
                                                                     size, dbuf, 2, ':', 0)) {
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "Could not convert fingerprint to hex");
        }
        else {
          GWEN_SslCertDescr_SetFingerPrintSha1(certDescr, GWEN_Buffer_GetStart(dbuf));
        }
        GWEN_Buffer_free(dbuf);
      }

      /* get fingerprint (SHA512) */
      size=sizeof(buffer1);
      rv=gnutls_x509_crt_get_fingerprint(cert, GNUTLS_DIG_SHA512, buffer1, &size);
      if (rv!=0) {
        DBG_INFO(GWEN_LOGDOMAIN, "gnutls_x509_crt_get_fingerprint(SHA512): %d (%s)", rv, gnutls_strerror(rv));
        GWEN_SslCertDescr_free(certDescr);
        gnutls_x509_crt_deinit(cert);
        return GWEN_ERROR_GENERIC;
      }
      else {
        GWEN_BUFFER *dbuf;

        dbuf=GWEN_Buffer_new(0, 256, 0, 1);
        if (GWEN_Text_ToHexBuffer(/* GCC4 pointer-signedness fix: */ buffer1,
                                                                     size, dbuf, 2, ':', 0)) {
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "Could not convert fingerprint to hex");
        }
        else {
          GWEN_SslCertDescr_SetFingerPrintSha512(certDescr, GWEN_Buffer_GetStart(dbuf));
        }
        GWEN_Buffer_free(dbuf);
      }


      if (xio->hostName) {
        DBG_INFO(GWEN_LOGDOMAIN, "Checking hostname [%s]", xio->hostName);
        if (!gnutls_x509_crt_check_hostname(cert, xio->hostName)) {
          DBG_WARN(GWEN_LOGDOMAIN,
                   "Certificate was not issued for this host");
          GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning,
                               I18N("Certificate was not issued for this host"));
          errFlags|=GWEN_SSL_CERT_FLAGS_BAD_HOSTNAME;
        }
        else {
          DBG_INFO(GWEN_LOGDOMAIN, "Cert is for this server");
        }
      }
      else {
        DBG_WARN(GWEN_LOGDOMAIN,
                 "Hostname is not set, unable to verify the sender");
        GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning,
                             I18N("No hostname to verify the sender!"));
      }

    }

    /* get activation time */
    t=gnutls_x509_crt_get_activation_time(cert);
    if (t<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "gnutls_x509_crt_get_activation_time: %d (%s)", rv, gnutls_strerror(rv));
      errFlags|=GWEN_SSL_CERT_FLAGS_BAD_DATA;
    }
    else {
      if (t>t0) {
        DBG_INFO(GWEN_LOGDOMAIN, "Cert is not yet active");
        errFlags|=GWEN_SSL_CERT_FLAGS_NOT_ACTIVE;
      }
      if (i==0) {
        GWEN_TIME *ti;

        ti=GWEN_Time_fromSeconds(t);
        if (ti)
          GWEN_SslCertDescr_SetNotBefore(certDescr, ti);
        GWEN_Time_free(ti);
      }
    }

    /* get expiration time */
    t=gnutls_x509_crt_get_expiration_time(cert);
    if (t<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "gnutls_x509_crt_get_expiration_time: %d (%s)", rv, gnutls_strerror(rv));
      errFlags|=GWEN_SSL_CERT_FLAGS_BAD_DATA;
    }
    else {
      if (t<t0) {
        DBG_INFO(GWEN_LOGDOMAIN, "Cert has expired");
        errFlags|=GWEN_SSL_CERT_FLAGS_EXPIRED;
      }
      if (i==0) {
        GWEN_TIME *ti;

        ti=GWEN_Time_fromSeconds(t);
        if (ti)
          GWEN_SslCertDescr_SetNotAfter(certDescr, ti);
        GWEN_Time_free(ti);
      }
    }

    if (i==0) {
      /* get owner information, but only for first cert */
      size=sizeof(buffer1)-1;
      rv=gnutls_x509_crt_get_dn_by_oid(cert, GNUTLS_OID_X520_COMMON_NAME, 0, 0, buffer1, &size);
      if (rv==0) {
        GWEN_SslCertDescr_SetCommonName(certDescr, buffer1);
      }

      size=sizeof(buffer1)-1;
      rv=gnutls_x509_crt_get_dn_by_oid(cert, GNUTLS_OID_X520_ORGANIZATION_NAME, 0, 0, buffer1, &size);
      if (rv==0)
        GWEN_SslCertDescr_SetOrganizationName(certDescr, buffer1);

      size=sizeof(buffer1)-1;
      rv=gnutls_x509_crt_get_dn_by_oid(cert, GNUTLS_OID_X520_ORGANIZATIONAL_UNIT_NAME, 0, 0, buffer1, &size);
      if (rv==0)
        GWEN_SslCertDescr_SetOrganizationalUnitName(certDescr, buffer1);

      size=sizeof(buffer1)-1;
      rv=gnutls_x509_crt_get_dn_by_oid(cert, GNUTLS_OID_X520_LOCALITY_NAME, 0, 0, buffer1, &size);
      if (rv==0)
        GWEN_SslCertDescr_SetLocalityName(certDescr, buffer1);

      size=sizeof(buffer1)-1;
      rv=gnutls_x509_crt_get_dn_by_oid(cert, GNUTLS_OID_X520_STATE_OR_PROVINCE_NAME, 0, 0, buffer1, &size);
      if (rv==0)
        GWEN_SslCertDescr_SetStateOrProvinceName(certDescr, buffer1);

      size=sizeof(buffer1)-1;
      rv=gnutls_x509_crt_get_dn_by_oid(cert, GNUTLS_OID_X520_COUNTRY_NAME, 0, 0, buffer1, &size);
      if (rv==0)
        GWEN_SslCertDescr_SetCountryName(certDescr, buffer1);
    }

    gnutls_x509_crt_deinit(cert);
  }

  /* done */
  if (errFlags)
    GWEN_SslCertDescr_SetIsError(certDescr, 1);
  else
    errFlags|=GWEN_SSL_CERT_FLAGS_OK;

  GWEN_SyncIo_Tls_SetCertStatus(certDescr, errFlags);

#if 0
  if (1) {
    GWEN_DB_NODE *dbTest;

    dbTest=GWEN_DB_Group_new("Cert");
    GWEN_SslCertDescr_toDb(certDescr, dbTest);
    GWEN_DB_Dump(dbTest, 2);
    GWEN_DB_Group_free(dbTest);
  }
#endif

  xio->peerCertDescr=certDescr;
  xio->peerCertFlags=errFlags;

  return 0;
}



ssize_t GWEN_SyncIo_Tls_Pull(gnutls_transport_ptr_t p, void *buf, size_t len)
{
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_TLS *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  sio=(GWEN_SYNCIO *) p;
  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "TLS PULL: %d bytes", (int)len);
  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  rv=GWEN_SyncIo_Read(baseIo, buf, len);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    gnutls_transport_set_errno(xio->session, errno);
    return (ssize_t)-1;
  }

  gnutls_transport_set_errno(xio->session, 0);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "TLS PULL: returning %d bytes", rv);
  /*GWEN_Text_DumpString(buf, rv, 2);*/
  return rv;
}



ssize_t GWEN_SyncIo_Tls_Push(gnutls_transport_ptr_t p, const void *buf, size_t len)
{
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_TLS *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  sio=(GWEN_SYNCIO *) p;
  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "TLS PUSH: %d bytes", (int)len);
  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  rv=GWEN_SyncIo_Write(baseIo, buf, len);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    gnutls_transport_set_errno(xio->session, errno);
    return (ssize_t)-1;
  }

  gnutls_transport_set_errno(xio->session, 0);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "TLS PUSH: returning %d bytes", rv);
  /*GWEN_Text_DumpString(buf, rv, 2);*/
  return rv;
}



void GWEN_SyncIo_Tls_ShowCipherInfo(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;
  const char *s;
  gnutls_kx_algorithm_t kx;
  GWEN_BUFFER *cbuf;
  GWEN_BUFFER *sbuf;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  cbuf=GWEN_Buffer_new(0, 256, 0, 1);
  sbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* protocol */
  s=gnutls_protocol_get_name(gnutls_protocol_get_version(xio->session));
  if (s && *s) {
    if (GWEN_Buffer_GetUsedBytes(cbuf))
      GWEN_Buffer_AppendString(cbuf, " ");
    GWEN_Buffer_AppendString(cbuf, "Protocol: ");
    GWEN_Buffer_AppendString(cbuf, s);

    GWEN_Buffer_AppendString(sbuf, s);
  }
  GWEN_Buffer_AppendString(sbuf, ":");

  /* key exchange algorithm */
  kx=gnutls_kx_get(xio->session);
  s=gnutls_kx_get_name(kx);
  if (s && *s) {
    if (GWEN_Buffer_GetUsedBytes(cbuf))
      GWEN_Buffer_AppendString(cbuf, " ");
    GWEN_Buffer_AppendString(cbuf, "Key exchange algorithm: ");
    GWEN_Buffer_AppendString(cbuf, s);
    GWEN_Buffer_AppendString(sbuf, s);
  }
  GWEN_Buffer_AppendString(sbuf, "-");

  /* cipher */
  s=gnutls_cipher_get_name(gnutls_cipher_get(xio->session));
  if (s && *s) {
    if (GWEN_Buffer_GetUsedBytes(cbuf))
      GWEN_Buffer_AppendString(cbuf, " ");
    GWEN_Buffer_AppendString(cbuf, "cipher algorithm: ");
    GWEN_Buffer_AppendString(cbuf, s);
    GWEN_Buffer_AppendString(sbuf, s);
  }
  GWEN_Buffer_AppendString(sbuf, ":");

  /* MAC algorithm */
  s=gnutls_mac_get_name(gnutls_mac_get(xio->session));
  if (s && *s) {
    if (GWEN_Buffer_GetUsedBytes(cbuf))
      GWEN_Buffer_AppendString(cbuf, " ");
    GWEN_Buffer_AppendString(cbuf, "MAC algorithm: ");
    GWEN_Buffer_AppendString(cbuf, s);
    GWEN_Buffer_AppendString(sbuf, s);
  }


  DBG_NOTICE(GWEN_LOGDOMAIN, "%s", GWEN_Buffer_GetStart(cbuf));
  GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Info, I18N("TLS: SSL-Ciphers negotiated: %s"), GWEN_Buffer_GetStart(sbuf));
  GWEN_Buffer_free(cbuf);
  GWEN_Buffer_free(sbuf);

  /* possibly show warning */
  switch (gnutls_cipher_get(xio->session)) {
  case GNUTLS_CIPHER_ARCFOUR_128:
  case GNUTLS_CIPHER_3DES_CBC:
  case GNUTLS_CIPHER_AES_128_CBC:
  case GNUTLS_CIPHER_ARCFOUR_40:
  case GNUTLS_CIPHER_CAMELLIA_128_CBC:
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("TLS: Warning - The server has chosen unsafe SSL-Ciphers!"));
    break;
  case GNUTLS_CIPHER_AES_256_CBC:
  case GNUTLS_CIPHER_CAMELLIA_256_CBC:
  case GNUTLS_CIPHER_RC2_40_CBC:
  case GNUTLS_CIPHER_DES_CBC:
#ifdef GNUTLS_CIPHER_AES_192_CBC
  case GNUTLS_CIPHER_AES_192_CBC: /* new in gnutls-2.9.8, so i.e. not available in gnutls-2.8.x */
#endif
  default:
    break;
  }
}



int GWEN_SyncIo_Tls_Handshake(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  do {
    rv=gnutls_handshake(xio->session);
  }
  while (rv==GNUTLS_E_AGAIN || rv==GNUTLS_E_INTERRUPTED);

  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_handshake: %d (%s) [%s]",
              rv, gnutls_strerror(rv), gnutls_error_is_fatal(rv)?"fatal":"non-fatal");
    if (rv==GNUTLS_E_UNEXPECTED_PACKET_LENGTH) {
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("A TLS handshake error occurred. "
                                "If you are using AqBanking you should "
                                "consider enabling the option "
                                "\"force SSLv3\" in the user settings "
                                "dialog."));
    }
    else {
      GWEN_Gui_ProgressLog2(0,
                            GWEN_LoggerLevel_Error,
                            I18N("TLS Handshake Error: %d (%s)"),
                            rv,
                            gnutls_strerror(rv));
    }
    return GWEN_ERROR_SSL;
  }

  return 0;
}

#endif /* GWEN_TLS_OPENSSL */



void GWEN_SyncIo_Tls_SetCertStatus(GWEN_SSLCERTDESCR *certDescr, uint32_t errFlags)
{
  GWEN_BUFFER *sbuf;

  sbuf=GWEN_Buffer_new(0, 256, 0, 1);

  if (errFlags & GWEN_SSL_CERT_FLAGS_SIGNER_NOT_FOUND) {
    if (GWEN_Buffer_GetUsedBytes(sbuf))
      GWEN_Buffer_AppendString(sbuf, "; ");
    GWEN_Buffer_AppendString(sbuf, I18N("Signer not found"));
  }

  if (errFlags & GWEN_SSL_CERT_FLAGS_INVALID) {
    if (GWEN_Buffer_GetUsedBytes(sbuf))
      GWEN_Buffer_AppendString(sbuf, "; ");
    GWEN_Buffer_AppendString(sbuf, I18N("Certificate is not trusted"));
  }

  if (errFlags & GWEN_SSL_CERT_FLAGS_REVOKED) {
    if (GWEN_Buffer_GetUsedBytes(sbuf))
      GWEN_Buffer_AppendString(sbuf, "; ");
    GWEN_Buffer_AppendString(sbuf, I18N("Certificate has been revoked"));
  }

  if (errFlags & GWEN_SSL_CERT_FLAGS_EXPIRED) {
    if (GWEN_Buffer_GetUsedBytes(sbuf))
      GWEN_Buffer_AppendString(sbuf, "; ");
    GWEN_Buffer_AppendString(sbuf, I18N("Certificate has expired"));
  }

  if (errFlags & GWEN_SSL_CERT_FLAGS_NOT_ACTIVE) {
    if (GWEN_Buffer_GetUsedBytes(sbuf))
      GWEN_Buffer_AppendString(sbuf, "; ");
    GWEN_Buffer_AppendString(sbuf, I18N("Certificate is not active yet"));
  }

  if (errFlags & GWEN_SSL_CERT_FLAGS_BAD_HOSTNAME) {
    if (GWEN_Buffer_GetUsedBytes(sbuf))
      GWEN_Buffer_AppendString(sbuf, "; ");
    GWEN_Buffer_AppendString(sbuf, I18N("Certificate owner does not match hostname"));
  }

  if (errFlags & GWEN_SSL_CERT_FLAGS_BAD_DATA) {
    if (GWEN_Buffer_GetUsedBytes(sbuf))
      GWEN_Buffer_AppendString(sbuf, "; ");
    GWEN_Buffer_AppendString(sbuf, I18N("Certificate contains invalid information"));
  }

  if (errFlags & GWEN_SSL_CERT_FLAGS_SYSTEM) {
    if (GWEN_Buffer_GetUsedBytes(sbuf))
      GWEN_Buffer_AppendString(sbuf, "; ");
    GWEN_Buffer_AppendString(sbuf, I18N("A system error occurred while checking the certificate"));
  }

  if (errFlags & GWEN_SSL_CERT_FLAGS_OK) {
    if (GWEN_Buffer_GetUsedBytes(sbuf))
      GWEN_Buffer_AppendString(sbuf, "; ");
    GWEN_Buffer_AppendString(sbuf, I18N("The certificate is valid"));
  }

  GWEN_SslCertDescr_SetStatusText(certDescr, GWEN_Buffer_GetStart(sbuf));
  GWEN_SslCertDescr_SetStatusFlags(certDescr, errFlags);
  GWEN_Buffer_free(sbuf);
}



int GWENHYWFAR_CB GWEN_SyncIo_Tls_Connect(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  if (GWEN_SyncIo_GetFlags(sio) & GWEN_SYNCIO_FLAGS_PASSIVE) {
    if (GWEN_SyncIo_GetStatus(baseIo)!=GWEN_SyncIo_Status_Connected) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Base layer is not connected");
      return GWEN_ERROR_NOT_CONNECTED;
    }
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Connecting base layer");
    rv=GWEN_SyncIo_Connect(baseIo);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    DBG_INFO(GWEN_LOGDOMAIN, "Base layer connected");
  }

  rv=GWEN_SyncIo_Tls_Prepare(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(baseIo);
    return rv;
  }

  rv=GWEN_SyncIo_Tls_Handshake(sio);
  if (rv<0) {
    GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
    GWEN_SyncIo_Tls_UndoPrepare(sio);
    GWEN_SyncIo_Disconnect(baseIo);
    return GWEN_ERROR_SSL;
  }
  else {
    /* show session info */
    GWEN_SyncIo_Tls_ShowCipherInfo(sio);

    /* check certificate */
    GWEN_SyncIo_SubFlags(sio, GWEN_SYNCIO_TLS_FLAGS_SECURE);
    rv=GWEN_SyncIo_Tls_GetPeerCert(sio);
    if (rv<0) {
      if (GWEN_SyncIo_GetFlags(sio) & GWEN_SYNCIO_TLS_FLAGS_NEED_PEER_CERT) {
        DBG_ERROR(GWEN_LOGDOMAIN, "No peer certificate when needed, aborting connection");
        GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
        GWEN_SyncIo_Tls_UndoPrepare(sio);
        GWEN_SyncIo_Disconnect(baseIo);
        return GWEN_ERROR_SSL_SECURITY;
      }
      else {
        DBG_INFO(GWEN_LOGDOMAIN, "SSL connected (insecure)");
        GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Connected);
        return 0;
      }
    }
    else {
      /* present cert to the user */
      rv=GWEN_SyncIo_Tls_CheckCert(sio, xio->peerCertDescr);
      if (rv<0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Peer cert not accepted (%d), aborting", rv);
        GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
        GWEN_SyncIo_Tls_UndoPrepare(sio);
        GWEN_SyncIo_Disconnect(baseIo);
        return GWEN_ERROR_SSL_SECURITY;
      }
      else {
        DBG_INFO(GWEN_LOGDOMAIN, "SSL connected (secure)");
        GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_TLS_FLAGS_SECURE);
        GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Connected);
        return 0;
      }
    }
  }
}



#ifdef GWEN_TLS_OPENSSL

int GWENHYWFAR_CB GWEN_SyncIo_Tls_Disconnect(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* check status */
  if (GWEN_SyncIo_GetStatus(sio)!=GWEN_SyncIo_Status_Connected) {
    DBG_INFO(GWEN_LOGDOMAIN, "Not connected");
    GWEN_SyncIo_Tls_UndoPrepare(sio);
    GWEN_SyncIo_Disconnect(baseIo);
    return GWEN_ERROR_NOT_CONNECTED;
  }

  rv=SSL_shutdown(xio->ssl);
  if (!rv)
    rv=SSL_shutdown(xio->ssl);

  if (rv==1 || rv==-1) {
    /* connection closed */
    DBG_INFO(GWEN_LOGDOMAIN, "Connection closed");
  }

  GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
  GWEN_SyncIo_Tls_UndoPrepare(sio);
  GWEN_SyncIo_Disconnect(baseIo);
  return 0;
}



int GWENHYWFAR_CB GWEN_SyncIo_Tls_Read(GWEN_SYNCIO *sio,
                                       uint8_t *buffer,
                                       uint32_t size)
{
  GWEN_SYNCIO_TLS *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* check status */
  if (GWEN_SyncIo_GetStatus(sio)!=GWEN_SyncIo_Status_Connected) {
    DBG_INFO(GWEN_LOGDOMAIN, "Socket is not connected");
    return GWEN_ERROR_INVALID;
  }

  assert(xio->ssl);

  DBG_DEBUG(GWEN_LOGDOMAIN,
            "Reading up to %d bytes while status \"%s\"",
            (int) size,
            SSL_state_string_long(xio->ssl));

  for (;;) {
    /* try to read */
    ERR_clear_error();
    rv=SSL_read(xio->ssl, buffer, (int) size);
    if (rv<1) {
      int sslerr;

      sslerr=SSL_get_error(xio->ssl, rv);
      if (sslerr==SSL_ERROR_WANT_READ ||
          sslerr==SSL_ERROR_WANT_WRITE)
        continue; /* would block */
      else {
        if (sslerr==SSL_ERROR_SYSCALL && errno==0) {
          DBG_INFO(GWEN_LOGDOMAIN, "Connection just went down (%d: %s)",
                   sslerr,
                   GWEN_SyncIo_Tls_ErrorString(sslerr));
          GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
          GWEN_SyncIo_Tls_UndoPrepare(sio);
          GWEN_SyncIo_Disconnect(baseIo);

          /* connection closed, no real error */
          return 0;
        } /* if error is syscall and errno==0 */
        else {
          if (sslerr==SSL_ERROR_ZERO_RETURN) {
            DBG_INFO(GWEN_LOGDOMAIN, "Connection closed");
            GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
            GWEN_SyncIo_Tls_UndoPrepare(sio);
            GWEN_SyncIo_Disconnect(baseIo);

            /* connection closed, no real error */
            return 0;
          }
          else {
            char errBuf[256];

            GWEN_SyncIo_Tls_ErrorText(sslerr, errBuf, sizeof(errBuf));
            DBG_INFO(GWEN_LOGDOMAIN, "List of pending SSL errors:");
            if (GWEN_Logger_GetLevel(GWEN_LOGDOMAIN)>=GWEN_LoggerLevel_Info)
              ERR_print_errors_fp(stderr); /* DEBUG */
            DBG_ERROR(GWEN_LOGDOMAIN, "SSL error: %s (%d)", errBuf, sslerr);
            ERR_clear_error();
            return GWEN_ERROR_READ;
          }
        } /* if !(error is syscall and errno==0) */
      } /* if error is not wantRead or wantWrite */
    } /* if rv<1 */
    break;
  }

  DBG_DEBUG(GWEN_LOGDOMAIN, "Read %d bytes:", rv);
  GWEN_Text_LogString((const char *) buffer, rv, GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

  if (getenv("GWEN_SSL_DEBUG")) {
    FILE *f;

    DBG_DEBUG(GWEN_LOGDOMAIN, "Saving...");
    f=fopen("/tmp/read.bin", "a+");
    if (!f) {
      DBG_INFO(GWEN_LOGDOMAIN, "fopen: %s", strerror(errno));
    }
    else {
      if (fwrite(buffer, rv, 1, f)!=1) {
        DBG_INFO(GWEN_LOGDOMAIN, "fwrite: %s", strerror(errno));
      }
      if (fclose(f)) {
        DBG_INFO(GWEN_LOGDOMAIN, "fclose: %s", strerror(errno));
      }
    }
  }

  return rv;
}



int GWENHYWFAR_CB GWEN_SyncIo_Tls_Write(GWEN_SYNCIO *sio,
                                        const uint8_t *buffer,
                                        uint32_t size)
{
  GWEN_SYNCIO_TLS *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* check status */
  if (GWEN_SyncIo_GetStatus(sio)!=GWEN_SyncIo_Status_Connected) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Socket is not connected");
    return GWEN_ERROR_INVALID;
  }

  assert(xio->ssl);

  DBG_DEBUG(GWEN_LOGDOMAIN,
            "Writing up to %d bytes while status \"%s\"",
            (int) size,
            SSL_state_string_long(xio->ssl));

  for (;;) {
    /* try to write */
    ERR_clear_error();
    rv=SSL_write(xio->ssl, buffer, (int) size);
    if (rv<1) {
      int sslerr;

      sslerr=SSL_get_error(xio->ssl, rv);
      if (sslerr==SSL_ERROR_WANT_READ ||
          sslerr==SSL_ERROR_WANT_WRITE)
        continue; /* would block */
      else {
        if (sslerr==SSL_ERROR_SYSCALL && errno==0) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Connection just went down (%d: %s)",
                    sslerr,
                    GWEN_SyncIo_Tls_ErrorString(sslerr));
          GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
          GWEN_SyncIo_Tls_UndoPrepare(sio);
          GWEN_SyncIo_Disconnect(baseIo);

          /* connection closed, no real error */
          return 0;
        } /* if error is syscall and errno==0 */
        else {
          if (sslerr==SSL_ERROR_ZERO_RETURN) {
            DBG_INFO(GWEN_LOGDOMAIN, "Connection closed");
            GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
            GWEN_SyncIo_Tls_UndoPrepare(sio);
            GWEN_SyncIo_Disconnect(baseIo);

            /* connection closed, no real error */
            return 0;
          }
          else {
            char errBuf[256];

            GWEN_SyncIo_Tls_ErrorText(sslerr, errBuf, sizeof(errBuf));
            DBG_ERROR(GWEN_LOGDOMAIN, "List of pending SSL errors:");
            if (GWEN_Logger_GetLevel(GWEN_LOGDOMAIN)>=GWEN_LoggerLevel_Info)
              ERR_print_errors_fp(stderr); /* DEBUG */
            DBG_ERROR(GWEN_LOGDOMAIN, "SSL error: %s (%d)", errBuf, sslerr);
            GWEN_Gui_ProgressLog2(0,
                                  GWEN_LoggerLevel_Error,
                                  I18N("Error on SSL_write: %d (%s)"),
                                  sslerr,
                                  errBuf);
            ERR_clear_error();
            return GWEN_ERROR_READ;
          }
        } /* if !(error is syscall and errno==0) */
      } /* if error is not wantRead or wantWrite */
    } /* if rv<1 */
    break;
  }

  DBG_DEBUG(GWEN_LOGDOMAIN, "Written %d bytes:", rv);
  GWEN_Text_LogString((const char *) buffer, rv, GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

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

  return rv;
}

#else /* GWEN_TLS_OPENSSL */



int GWENHYWFAR_CB GWEN_SyncIo_Tls_Disconnect(GWEN_SYNCIO *sio)
{
  GWEN_SYNCIO_TLS *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  if (GWEN_SyncIo_GetStatus(sio)!=GWEN_SyncIo_Status_Connected) {
    DBG_INFO(GWEN_LOGDOMAIN, "Not connected");
    GWEN_SyncIo_Tls_UndoPrepare(sio);
    GWEN_SyncIo_Disconnect(baseIo);
    return GWEN_ERROR_NOT_CONNECTED;
  }

  do {
    rv=gnutls_bye(xio->session, GNUTLS_SHUT_RDWR);
  }
  while (rv==GNUTLS_E_AGAIN || rv==GNUTLS_E_INTERRUPTED);

  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_bye: %d (%s)", rv, gnutls_strerror(rv));
    GWEN_Gui_ProgressLog2(0,
                          GWEN_LoggerLevel_Info,
                          I18N("Error on gnutls_bye: %d (%s)"),
                          rv,
                          gnutls_strerror(rv));
    GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
    GWEN_SyncIo_Tls_UndoPrepare(sio);
    GWEN_SyncIo_Disconnect(baseIo);
    return GWEN_ERROR_SSL;
  }

  GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
  GWEN_SyncIo_Tls_UndoPrepare(sio);
  GWEN_SyncIo_Disconnect(baseIo);
  return 0;
}



int GWENHYWFAR_CB GWEN_SyncIo_Tls_Read(GWEN_SYNCIO *sio,
                                       uint8_t *buffer,
                                       uint32_t size)
{
  GWEN_SYNCIO_TLS *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  if (GWEN_SyncIo_GetStatus(sio)!=GWEN_SyncIo_Status_Connected) {
    DBG_INFO(GWEN_LOGDOMAIN, "Not connected");
    GWEN_SyncIo_Tls_UndoPrepare(sio);
    GWEN_SyncIo_Disconnect(baseIo);
    return GWEN_ERROR_NOT_CONNECTED;
  }

  do {
    rv=gnutls_record_recv(xio->session, buffer, size);
  }
  while (rv==GNUTLS_E_AGAIN || rv==GNUTLS_E_INTERRUPTED);

  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_record_recv: %d (%s)", rv, gnutls_strerror(rv));
#if 0
    GWEN_Gui_ProgressLog2(0,
                          GWEN_LoggerLevel_Error,
                          I18N("Error on gnutls_record_recv: %d (%s)"),
                          rv,
                          gnutls_strerror(rv));
#endif
    GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
    GWEN_SyncIo_Tls_UndoPrepare(sio);
    GWEN_SyncIo_Disconnect(baseIo);
#ifdef GNUTLS_E_PREMATURE_TERMINATION
    if (rv==GNUTLS_E_PREMATURE_TERMINATION) {
      if (GWEN_SyncIo_GetFlags(sio) & GWEN_SYNCIO_TLS_FLAGS_IGN_PREMATURE_CLOSE) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Detected premature disconnect by server (violates specs!), ignoring.");
        return 0; /* report EOF */
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Detected premature disconnect by server (violates specs!)");
        return GWEN_ERROR_SSL_PREMATURE_CLOSE;
      }
    }
#endif
    return GWEN_ERROR_SSL;
  }

#ifdef GWEN_TLS_DEBUG
  DBG_ERROR(0, "Received this:");
  GWEN_Text_DumpString((const char *) buffer, rv, 2);
#endif

  return rv;
}



int GWENHYWFAR_CB GWEN_SyncIo_Tls_Write(GWEN_SYNCIO *sio,
                                        const uint8_t *buffer,
                                        uint32_t size)
{
  GWEN_SYNCIO_TLS *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

#ifdef GWEN_TLS_DEBUG
  DBG_ERROR(0, "Sending this:");
  GWEN_Text_DumpString((const char *) buffer, size, 2);
#endif

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  if (GWEN_SyncIo_GetStatus(sio)!=GWEN_SyncIo_Status_Connected) {
    DBG_INFO(GWEN_LOGDOMAIN, "Not connected");
    GWEN_SyncIo_Tls_UndoPrepare(sio);
    GWEN_SyncIo_Disconnect(baseIo);
    return GWEN_ERROR_NOT_CONNECTED;
  }

  do {
    rv=gnutls_record_send(xio->session, buffer, size);
  }
  while (rv==GNUTLS_E_AGAIN || rv==GNUTLS_E_INTERRUPTED);

  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_record_send: %d (%s)", rv, gnutls_strerror(rv));
    GWEN_Gui_ProgressLog2(0,
                          GWEN_LoggerLevel_Error,
                          I18N("Error on gnutls_record_send: %d (%s)"),
                          rv,
                          gnutls_strerror(rv));
    GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
    GWEN_SyncIo_Tls_UndoPrepare(sio);
    GWEN_SyncIo_Disconnect(baseIo);
    return GWEN_ERROR_SSL;
  }

  return rv;
}

#endif /* GWEN_TLS_OPENSSL */
