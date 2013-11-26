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

#include <gnutls/gnutls.h>


#if GNUTLS_VERSION_NUMBER < 0x030204
#  define GWEN_TLS_CIPHER_PRIORITIES_DEFAULT "SECURE256"
#else
#  define GWEN_TLS_CIPHER_PRIORITIES_DEFAULT "PFS"
#endif

#define GWEN_TLS_CIPHER_PRIORITIES_DISABLE_UNSAFE "-ARCFOUR-128:-AES-128-CBC:-CAMELLIA-128-CBC:-3DES-CBC"



typedef struct GWEN_SYNCIO_TLS GWEN_SYNCIO_TLS;
struct GWEN_SYNCIO_TLS {
  char *localCertFile;
  char *localKeyFile;
  char *localTrustFile;
  char *dhParamFile;

  char *hostName;

  gnutls_session_t session;
  gnutls_certificate_credentials_t credentials;

  int prepared;

  GWEN_SSLCERTDESCR *peerCertDescr;
  uint32_t peerCertFlags;
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


static int GWEN_SyncIo_Tls__readFile(const char *fname, GWEN_BUFFER *buf);

static ssize_t GWEN_SyncIo_Tls_Pull(gnutls_transport_ptr_t p, void *buf, size_t len);
static ssize_t GWEN_SyncIo_Tls_Push(gnutls_transport_ptr_t p, const void *buf, size_t len);


void GWEN_SyncIo_Tls_UndoPrepare(GWEN_SYNCIO *sio);
int GWEN_SyncIo_Tls_Prepare(GWEN_SYNCIO *sio);

static void GWEN_SyncIo_Tls_ShowCipherInfo(GWEN_SYNCIO *sio);


#endif


