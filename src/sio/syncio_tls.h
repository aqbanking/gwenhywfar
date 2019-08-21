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


#ifndef GWENHYWFAR_SYNCIO_TLS_H
#define GWENHYWFAR_SYNCIO_TLS_H

#include <gwenhywfar/syncio.h>
#include <gwenhywfar/ssl_cert_descr.h>


#define GWEN_SYNCIO_TLS_TYPE "tls"


#define GWEN_SYNCIO_TLS_FLAGS_REQUEST_CERT          0x00000001

#define GWEN_SYNCIO_TLS_FLAGS_ALLOW_V1_CA_CRT       0x00000004
#define GWEN_SYNCIO_TLS_FLAGS_NEED_PEER_CERT        0x00000008
#define GWEN_SYNCIO_TLS_FLAGS_ADD_TRUSTED_CAS       0x00000010
#define GWEN_SYNCIO_TLS_FLAGS_SET_PASSV_HOST_NAME   0x00000020 /* not used atm */
#define GWEN_SYNCIO_TLS_FLAGS_SET_PASSV_HOST_IP     0x00000020 /* not used atm */

/** ignore error "GNUTLS_E_PREMATURE_TERMINATION" */
#define GWEN_SYNCIO_TLS_FLAGS_IGN_PREMATURE_CLOSE   0x00000200

#define GWEN_SYNCIO_TLS_FLAGS_SECURE                0x00008000



#ifdef __cplusplus
extern "C" {
#endif



GWENHYWFAR_API
GWEN_SYNCIO *GWEN_SyncIo_Tls_new(GWEN_SYNCIO *baseIo);


GWENHYWFAR_API const char *GWEN_SyncIo_Tls_GetLocalCertFile(const GWEN_SYNCIO *sio);
GWENHYWFAR_API void GWEN_SyncIo_Tls_SetLocalCertFile(GWEN_SYNCIO *sio, const char *s);

GWENHYWFAR_API const char *GWEN_SyncIo_Tls_GetLocalKeyFile(const GWEN_SYNCIO *sio);
GWENHYWFAR_API void GWEN_SyncIo_Tls_SetLocalKeyFile(GWEN_SYNCIO *sio, const char *s);

GWENHYWFAR_API const char *GWEN_SyncIo_Tls_GetLocalTrustFile(const GWEN_SYNCIO *sio);
GWENHYWFAR_API void GWEN_SyncIo_Tls_SetLocalTrustFile(GWEN_SYNCIO *sio, const char *s);

GWENHYWFAR_API const char *GWEN_SyncIo_Tls_GetLocalTrustFolder(const GWEN_SYNCIO *sio);
GWENHYWFAR_API void GWEN_SyncIo_Tls_SetLocalTrustFolder(GWEN_SYNCIO *sio, const char *s);


GWENHYWFAR_API const char *GWEN_SyncIo_Tls_GetDhParamFile(const GWEN_SYNCIO *sio);
GWENHYWFAR_API void GWEN_SyncIo_Tls_SetDhParamFile(GWEN_SYNCIO *sio, const char *s);

GWENHYWFAR_API const char *GWEN_SyncIo_Tls_GetRemoteHostName(const GWEN_SYNCIO *sio);
GWENHYWFAR_API void GWEN_SyncIo_Tls_SetRemoteHostName(GWEN_SYNCIO *sio, const char *s);

GWENHYWFAR_API GWEN_SSLCERTDESCR *GWEN_SyncIo_Tls_GetPeerCertDescr(const GWEN_SYNCIO *sio);



typedef int GWENHYWFAR_CB(*GWEN_SIO_TLS_CHECKCERT_FN)(GWEN_SYNCIO *sio, const GWEN_SSLCERTDESCR *cert);
GWENHYWFAR_API GWEN_SIO_TLS_CHECKCERT_FN GWEN_SyncIo_Tls_SetCheckCertFn(GWEN_SYNCIO *sio, GWEN_SIO_TLS_CHECKCERT_FN f);


#ifdef __cplusplus
}
#endif


#endif


