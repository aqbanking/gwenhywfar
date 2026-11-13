/***************************************************************************
 begin       : Wed Apr 28 2010
 copyright   : (C) 2010 by Martin Preuss
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


#ifndef GWENHYWFAR_SYNCIO_TLS_P_H
#define GWENHYWFAR_SYNCIO_TLS_P_H

#include <gwenhywfar/syncio_tls.h>
#include <gwenhywfar/gwentime.h>

#ifdef GWEN_TLS_OPENSSL
# include <openssl/ssl.h>
#else
# include <gnutls/gnutls.h>


# if GNUTLS_VERSION_NUMBER < 0x030204
#  define GWEN_TLS_CIPHER_PRIORITIES_DEFAULT      "SECURE256:SECURE128"
# else
#  define GWEN_TLS_CIPHER_PRIORITIES_DEFAULT      "PFS:SECURE128"
# endif

# define GWEN_TLS_CIPHER_PRIORITIES_UNSAFE         "SECURE128"
# define GWEN_TLS_CIPHER_PRIORITIES_DISABLE_UNSAFE "-ARCFOUR-128:-AES-128-CBC:-CAMELLIA-128-CBC:-3DES-CBC"
#endif



typedef struct GWEN_SYNCIO_TLS GWEN_SYNCIO_TLS;
struct GWEN_SYNCIO_TLS {
  char *localCertFile;
  char *localKeyFile;
  char *localTrustFile;
  char *dhParamFile;

  char *hostName;

#ifdef GWEN_TLS_OPENSSL
  BIO *bio;
  SSL_CTX *ssl_ctx;
  SSL *ssl;
#else
  gnutls_session_t session;
  gnutls_certificate_credentials_t credentials;
#endif

  int prepared;

  GWEN_SSLCERTDESCR *peerCertDescr;
  uint32_t peerCertFlags;
  GWEN_SIO_TLS_CHECKCERT_FN checkCertFn;
};


static void GWENHYWFAR_CB GWEN_SyncIo_Tls_FreeData(void *bp, void *p);

int GWENHYWFAR_CB GWEN_SyncIo_Tls_Connect(GWEN_SYNCIO *sio);

int GWENHYWFAR_CB GWEN_SyncIo_Tls_Disconnect(GWEN_SYNCIO *sio);


int GWENHYWFAR_CB GWEN_SyncIo_Tls_Read(GWEN_SYNCIO *sio,
                                       uint8_t *buffer,
                                       uint32_t size);



int GWENHYWFAR_CB GWEN_SyncIo_Tls_Write(GWEN_SYNCIO *sio,
                                        const uint8_t *buffer,
                                        uint32_t size);

static int GWENHYWFAR_CB GWEN_SyncIo_Tls_Internal_CheckCert(GWEN_SYNCIO *sio, const GWEN_SSLCERTDESCR *cert);


#ifndef GWEN_TLS_OPENSSL
static int GWEN_SyncIo_Tls__readFile(const char *fname, GWEN_BUFFER *buf);
#endif

#ifdef GWEN_TLS_OPENSSL
static BIO *BIO_syncio_new(GWEN_SYNCIO *sio);
static int BIO_syncio_create(BIO *bio);
static int BIO_syncio_destroy(BIO *bio);
static int BIO_syncio_read(BIO *bio, char *in, int inl);
static int BIO_syncio_write(BIO *bio, const char *out, int outl);
static long BIO_syncio_ctrl(BIO *bio, int cmd, long num, void *ptr);
static int BIO_syncio_puts(BIO *bp, const char *str);

static int GWEN_SyncIo_Tls_Setup(GWEN_SYNCIO *sio);
static const char *GWEN_SyncIo_Tls_ErrorString(unsigned int e);
static void GWEN_SyncIo_Tls_InfoCallBack(const SSL *s, int where, int ret);
static int GWEN_SyncIo_Tls_VerifyCallBack(int preverify_ok, X509_STORE_CTX *ctx);
static char *GWEN_SyncIo_Tls_GetCertEntry(X509_NAME *nm, int nid);
static GWEN_TIME *GWEN_SyncIo_Tls_ASN_GetTime(ASN1_TIME *d);
static GWEN_SSLCERTDESCR *GWEN_SyncIo_Tls_Cert2Descr(X509 *cert);
#else
static ssize_t GWEN_SyncIo_Tls_Pull(gnutls_transport_ptr_t p, void *buf, size_t len);
static ssize_t GWEN_SyncIo_Tls_Push(gnutls_transport_ptr_t p, const void *buf, size_t len);
#endif


void GWEN_SyncIo_Tls_UndoPrepare(GWEN_SYNCIO *sio);
int GWEN_SyncIo_Tls_Prepare(GWEN_SYNCIO *sio);

static void GWEN_SyncIo_Tls_ShowCipherInfo(GWEN_SYNCIO *sio);
static int GWEN_SyncIo_Tls_Handshake(GWEN_SYNCIO *sio);
static void GWEN_SyncIo_Tls_SetCertStatus(GWEN_SSLCERTDESCR *certDescr, uint32_t errFlags);


static int GWEN_SyncIo_Tls_CheckCert(GWEN_SYNCIO *sio, const GWEN_SSLCERTDESCR *cert);

static int GWEN_SyncIo_Tls_GetPeerCert(GWEN_SYNCIO *sio);

#endif


