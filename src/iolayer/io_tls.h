/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_IOLAYER_TLS_H
#define GWEN_IOLAYER_TLS_H

#include <gwenhywfar/iolayer.h>
#include <gwenhywfar/ssl_cert_descr.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @name Makro Defining the Type Name
 *
 */
/*@{*/
#define GWEN_IO_LAYER_TLS_TYPE "tls"
/*@}*/



#define GWEN_IO_LAYER_TLS_FLAGS_REQUEST_CERT        0x00000001
#define GWEN_IO_LAYER_TLS_FLAGS_FORCE_SSL_V3        0x00000002
#define GWEN_IO_LAYER_TLS_FLAGS_ALLOW_V1_CA_CRT     0x00000004
#define GWEN_IO_LAYER_TLS_FLAGS_NEED_PEER_CERT      0x00000008
#define GWEN_IO_LAYER_TLS_FLAGS_ADD_TRUSTED_CAS     0x00000010
#define GWEN_IO_LAYER_TLS_FLAGS_SET_PASSV_HOST_NAME 0x00000020
#define GWEN_IO_LAYER_TLS_FLAGS_SET_PASSV_HOST_IP   0x00000020

#define GWEN_IO_LAYER_TLS_FLAGS_SECURE              0x00008000




GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_LayerTls_new(GWEN_IO_LAYER *baseLayer);


GWENHYWFAR_API const char *GWEN_Io_LayerTls_GetLocalCertFile(const GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_LayerTls_SetLocalCertFile(GWEN_IO_LAYER *io, const char *s);

GWENHYWFAR_API const char *GWEN_Io_LayerTls_GetLocalKeyFile(const GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_LayerTls_SetLocalKeyFile(GWEN_IO_LAYER *io, const char *s);

GWENHYWFAR_API const char *GWEN_Io_LayerTls_GetLocalTrustFile(const GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_LayerTls_SetLocalTrustFile(GWEN_IO_LAYER *io, const char *s);

GWENHYWFAR_API const char *GWEN_Io_LayerTls_GetDhParamFile(const GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_LayerTls_SetDhParamFile(GWEN_IO_LAYER *io, const char *s);

GWENHYWFAR_API const char *GWEN_Io_LayerTls_GetRemoteHostName(const GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_LayerTls_SetRemoteHostName(GWEN_IO_LAYER *io, const char *s);

GWENHYWFAR_API GWEN_SSLCERTDESCR *GWEN_Io_LayerTls_GetPeerCertDescr(const GWEN_IO_LAYER *io);


#ifdef __cplusplus
}
#endif


#endif


