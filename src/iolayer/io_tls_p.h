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


#ifndef GWEN_IOLAYER_TLS_P_H
#define GWEN_IOLAYER_TLS_P_H

#include <gwenhywfar/io_tls.h>
#include <gwenhywfar/io_codec_be.h>
#include <gwenhywfar/iolayer_be.h>

#include <gnutls/gnutls.h>



typedef struct GWEN_IO_LAYER_TLS GWEN_IO_LAYER_TLS;
struct GWEN_IO_LAYER_TLS {
  char *localCertFile;
  char *localKeyFile;
  char *localTrustFile;
  char *dhParamFile;

  char *hostName;

  gnutls_session_t session;
  gnutls_certificate_credentials_t credentials;

  int prepared;

  GWEN_IO_LAYER_WORKONREQUESTS_FN workOnRequestsFn;
  GWEN_IO_LAYER_ADDREQUEST_FN addRequestFn;
  GWEN_IO_LAYER_DELREQUEST_FN delRequestFn;
  GWEN_IO_LAYER_HASWAITINGREQUESTS_FN hasWaitingRequestsFn;

  GWEN_IO_REQUEST *connectRequest;
  GWEN_IO_REQUEST *disconnectRequest;

  GWEN_SSLCERTDESCR *peerCertDescr;
  uint32_t peerCertFlags;
};


static GWENHYWFAR_CB void GWEN_Io_LayerTls_freeData(void *bp, void *p);

static int GWEN_Io_LayerTls_Prepare(GWEN_IO_LAYER *io);
static int GWEN_Io_LayerTls_GetPeerCert(GWEN_IO_LAYER *io, uint32_t guiid);

static ssize_t GWEN_Io_LayerTls_Pull(gnutls_transport_ptr_t p, void *buf, size_t len);
static ssize_t GWEN_Io_LayerTls_Push(gnutls_transport_ptr_t p, const void *buf, size_t len);

static int GWEN_Io_LayerTls_Encode(GWEN_IO_LAYER *io, const uint8_t *pBuffer, uint32_t lBuffer);
static int GWEN_Io_LayerTls_Decode(GWEN_IO_LAYER *io, uint8_t *pBuffer, uint32_t lBuffer);


static int GWEN_Io_LayerTls_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);
static int GWEN_Io_LayerTls_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);
static int GWEN_Io_LayerTls_HasWaitingRequests(GWEN_IO_LAYER *io);
static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerTls_WorkOnRequests(GWEN_IO_LAYER *io);

static int GWEN_Io_LayerTls__readFile(const char *fname, GWEN_BUFFER *buf);





#endif

