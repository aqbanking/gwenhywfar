/***************************************************************************
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "io_tls_p.h"
#include <gwenhywfar/iolayer_be.h>
#include <gwenhywfar/io_codec_be.h>
#include <gwenhywfar/iorequest_be.h>
#include <gwenhywfar/io_socket.h>

#include "i18n_l.h"
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/directory.h>

#include <assert.h>
#include <errno.h>

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>


#include <gwenhywfar/text.h> /* debug */



GWEN_INHERIT(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS)


#ifndef HAVE_GNUTLS_TRANSPORT_SET_ERRNO
/* #warning "gnutls_transport_set_errno is not defined" */
#endif




GWEN_IO_LAYER *GWEN_Io_LayerTls_new(GWEN_IO_LAYER *baseLayer) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_TLS *xio;

  io=GWEN_Io_LayerCodec_new(GWEN_IO_LAYER_TLS_TYPE, baseLayer);
  assert(io);
  GWEN_NEW_OBJECT(GWEN_IO_LAYER_TLS, xio);
  assert(xio);
  GWEN_INHERIT_SETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io, xio, GWEN_Io_LayerTls_freeData);

  xio->workOnRequestsFn=GWEN_Io_Layer_SetWorkOnRequestsFn(io, GWEN_Io_LayerTls_WorkOnRequests);
  xio->addRequestFn=GWEN_Io_Layer_SetAddRequestFn(io, GWEN_Io_LayerTls_AddRequest);
  xio->delRequestFn=GWEN_Io_Layer_SetDelRequestFn(io, GWEN_Io_LayerTls_DelRequest);
  xio->hasWaitingRequestsFn=GWEN_Io_Layer_SetHasWaitingRequestsFn(io, GWEN_Io_LayerTls_HasWaitingRequests);

  GWEN_Io_LayerCodec_SetEncodeFn(io, GWEN_Io_LayerTls_Encode);
  GWEN_Io_LayerCodec_SetDecodeFn(io, GWEN_Io_LayerTls_Decode);

  return io;
}



GWENHYWFAR_CB
void GWEN_Io_LayerTls_freeData(void *bp, void *p) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_TLS *xio;

  io=(GWEN_IO_LAYER*) bp;
  assert(io);
  xio=(GWEN_IO_LAYER_TLS*) p;
  assert(xio);

  free(xio->dhParamFile);
  free(xio->localCertFile);
  free(xio->localKeyFile);
  free(xio->localTrustFile);
  free(xio->hostName);
  if (xio->prepared) {
    gnutls_deinit(xio->session);
    gnutls_certificate_free_credentials(xio->credentials);
    xio->prepared=0;
  }

  GWEN_SslCertDescr_free(xio->peerCertDescr);

}



const char *GWEN_Io_LayerTls_GetLocalCertFile(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_TLS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  return xio->localCertFile;
}



void GWEN_Io_LayerTls_SetLocalCertFile(GWEN_IO_LAYER *io, const char *s) {
  GWEN_IO_LAYER_TLS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  free(xio->localCertFile);
  if (s) xio->localCertFile=strdup(s);
  else xio->localCertFile=NULL;
}



const char *GWEN_Io_LayerTls_GetLocalKeyFile(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_TLS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  return xio->localKeyFile;
}



void GWEN_Io_LayerTls_SetLocalKeyFile(GWEN_IO_LAYER *io, const char *s) {
  GWEN_IO_LAYER_TLS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  free(xio->localKeyFile);
  if (s) xio->localKeyFile=strdup(s);
  else xio->localKeyFile=NULL;
}



const char *GWEN_Io_LayerTls_GetLocalTrustFile(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_TLS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  return xio->localTrustFile;
}



void GWEN_Io_LayerTls_SetLocalTrustFile(GWEN_IO_LAYER *io, const char *s) {
  GWEN_IO_LAYER_TLS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  free(xio->localTrustFile);
  if (s) xio->localTrustFile=strdup(s);
  else xio->localTrustFile=NULL;
}



const char *GWEN_Io_LayerTls_GetDhParamFile(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_TLS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  return xio->dhParamFile;
}



void GWEN_Io_LayerTls_SetDhParamFile(GWEN_IO_LAYER *io, const char *s) {
  GWEN_IO_LAYER_TLS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  free(xio->dhParamFile);
  if (s) xio->dhParamFile=strdup(s);
  else xio->dhParamFile=NULL;
}



const char *GWEN_Io_LayerTls_GetRemoteHostName(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_TLS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  return xio->hostName;
}



void GWEN_Io_LayerTls_SetRemoteHostName(GWEN_IO_LAYER *io, const char *s) {
  GWEN_IO_LAYER_TLS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  free(xio->hostName);
  if (s) xio->hostName=strdup(s);
  else xio->hostName=NULL;
}



GWEN_SSLCERTDESCR *GWEN_Io_LayerTls_GetPeerCertDescr(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_TLS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  return xio->peerCertDescr;
}



int GWEN_Io_LayerTls__readFile(const char *fname, GWEN_BUFFER *buf) {
  FILE *f;

  f=fopen(fname, "r");
  if (f==NULL)
    return GWEN_ERROR_IO;

  while(!feof(f)) {
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



int GWEN_Io_LayerTls_Prepare(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_TLS *xio;
  int rv;
  uint32_t lflags;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  lflags=GWEN_Io_Layer_GetFlags(io);
  DBG_INFO(GWEN_LOGDOMAIN, "Preparing SSL (%08x)", lflags);

  /* init session */
  if (lflags & GWEN_IO_LAYER_FLAGS_PASSIVE) {
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

  /* set default priority */
  rv=gnutls_set_default_priority(xio->session);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_set_default_priority: %d (%s)", rv, gnutls_strerror(rv));
    gnutls_deinit(xio->session);
    return GWEN_ERROR_GENERIC;
  }

  /* possibly force protocol priority */
  if (lflags & GWEN_IO_LAYER_TLS_FLAGS_FORCE_SSL_V3) {
    const int proto_prio[2] = { GNUTLS_SSL3, 0 };

    rv=gnutls_protocol_set_priority(xio->session, proto_prio);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_protocol_set_priority: %d (%s)", rv, gnutls_strerror(rv));
      gnutls_deinit(xio->session);
      return GWEN_ERROR_GENERIC;
    }
  }

  /* let a server request peer certs */
  if ((lflags & GWEN_IO_LAYER_FLAGS_PASSIVE) &&
      (lflags & GWEN_IO_LAYER_TLS_FLAGS_REQUEST_CERT))
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
  if (lflags & GWEN_IO_LAYER_TLS_FLAGS_ADD_TRUSTED_CAS) {
    GWEN_STRINGLIST *paths;

    /* try to find out trust file */
    paths=GWEN_PathManager_GetPaths(GWEN_PM_LIBNAME, GWEN_PM_DATADIR);
    if (paths) {
      GWEN_BUFFER *nbuf;

      nbuf=GWEN_Buffer_new(0, 256, 0, 1);
      rv=GWEN_Directory_FindFileInPaths(paths,
					"ca-bundle.crt",
					nbuf);
      GWEN_StringList_free(paths);
      if (rv==0) {
	DBG_INFO(GWEN_LOGDOMAIN,
		 "Using default ca-bundle from [%s]",
		 GWEN_Buffer_GetStart(nbuf));
	GWEN_Io_LayerTls_SetLocalTrustFile(io, GWEN_Buffer_GetStart(nbuf));
      }
      else {
        DBG_WARN(GWEN_LOGDOMAIN, "Default bundle file not found");
      }
      GWEN_Buffer_free(nbuf);
    }
  }

  /* possibly set trust file */
  if (xio->localTrustFile) {
    rv=gnutls_certificate_set_x509_trust_file(xio->credentials,
					      xio->localTrustFile,
					      GNUTLS_X509_FMT_PEM);
    if (rv<=0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_certificate_set_x509_trust_file: %d (%s)", rv, gnutls_strerror(rv));
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
    rv=GWEN_Io_LayerTls__readFile(xio->dhParamFile, dbuf);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(dbuf);
      gnutls_certificate_free_credentials(xio->credentials);
      gnutls_deinit(xio->session);
      return rv;
    }
    else {
      gnutls_datum d;
      gnutls_dh_params dh_params=NULL;

      rv=gnutls_dh_params_init(&dh_params);
      if (rv<0) {
	GWEN_Buffer_free(dbuf);
	DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_dh_params_init: %d (%s)", rv, gnutls_strerror(rv));
	gnutls_certificate_free_credentials(xio->credentials);
	gnutls_deinit(xio->session);
	return GWEN_ERROR_GENERIC;
      }

      d.size=GWEN_Buffer_GetUsedBytes(dbuf);
      d.data=(unsigned char*)GWEN_Buffer_GetStart(dbuf);

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

  /* we use our own push/pull functions */
  gnutls_transport_set_ptr(xio->session, (gnutls_transport_ptr_t)io);
  gnutls_transport_set_push_function(xio->session, GWEN_Io_LayerTls_Push);
  gnutls_transport_set_pull_function(xio->session, GWEN_Io_LayerTls_Pull);
  gnutls_transport_set_lowat(xio->session, 0);

  xio->prepared=1;

  return 0;
}



int GWEN_Io_LayerTls_GetPeerCert(GWEN_IO_LAYER *io, uint32_t guiid) {
  GWEN_IO_LAYER_TLS *xio;
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
  GWEN_BUFFER *sbuf=NULL;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  lflags=GWEN_Io_Layer_GetFlags(io);

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
  if (lflags & GWEN_IO_LAYER_TLS_FLAGS_ALLOW_V1_CA_CRT)
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
    GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Warning,
			 I18N("Signer not found"));
    errFlags|=GWEN_SSL_CERT_FLAGS_SIGNER_NOT_FOUND;
  }

  if (status & GNUTLS_CERT_INVALID) {
    DBG_INFO(GWEN_LOGDOMAIN, "Certificate is not trusted");
    GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Warning,
			 I18N("Certificate is not trusted"));
    errFlags|=GWEN_SSL_CERT_FLAGS_INVALID;
  }

  if (status & GNUTLS_CERT_REVOKED) {
    DBG_INFO(GWEN_LOGDOMAIN, "Certificate has been revoked");
    GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Warning,
			 I18N("Certificate has been revoked"));
    errFlags|=GWEN_SSL_CERT_FLAGS_REVOKED;
  }

  cert_list=gnutls_certificate_get_peers(xio->session, &cert_list_size);
  if (cert_list==NULL || cert_list_size==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "No peer certificates found");
    return GWEN_ERROR_NO_DATA;
  }

  for (i=0; i<cert_list_size; i++) {
    gnutls_x509_crt_t cert;
    time_t t;

    rv=gnutls_x509_crt_init(&cert);
    if (rv!=0) {
      DBG_INFO(GWEN_LOGDOMAIN, "gnutls_x509_crt_init: %d (%s)", rv, gnutls_strerror(rv));
      return GWEN_ERROR_GENERIC;
    }

    rv=gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER);
    if (rv!=0) {
      DBG_INFO(GWEN_LOGDOMAIN, "gnutls_x509_crt_import: %d (%s)", rv, gnutls_strerror(rv));
      gnutls_x509_crt_deinit(cert);
      return GWEN_ERROR_GENERIC;
    }

    if (i==0) {
      /* get fingerprint */
      size=16;
      rv=gnutls_x509_crt_get_fingerprint(cert, GNUTLS_DIG_MD5, buffer1, &size);
      if (rv!=0) {
	DBG_INFO(GWEN_LOGDOMAIN, "gnutls_x509_crt_get_fingerprint: %d (%s)", rv, gnutls_strerror(rv));
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

      if (xio->hostName) {
        DBG_INFO(GWEN_LOGDOMAIN, "Checking hostname [%s]", xio->hostName);
	if (!gnutls_x509_crt_check_hostname(cert, xio->hostName)) {
	  DBG_WARN(GWEN_LOGDOMAIN,
		   "Certificate was not issued for this host");
	  GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Warning,
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
	GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Warning,
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
	if (xio->hostName && strcasecmp(xio->hostName, buffer1)!=0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "Owner of certificate does not match hostname");
	  errFlags|=GWEN_SSL_CERT_FLAGS_BAD_HOSTNAME;
	}
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

  xio->peerCertDescr=certDescr;
  xio->peerCertFlags=errFlags;

  return 0;
}



ssize_t GWEN_Io_LayerTls_Pull(gnutls_transport_ptr_t p, void *buf, size_t len) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_TLS *xio;
  int rv;

  io=(GWEN_IO_LAYER*) p;
  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "TLS PULL: %d bytes", (int)len);
  rv=GWEN_Io_LayerCodec_EnsureReadOk(io);
  if (rv) {
    if (rv==GWEN_ERROR_TRY_AGAIN || rv==GWEN_ERROR_IN_PROGRESS) {
#ifdef HAVE_GNUTLS_TRANSPORT_SET_ERRNO
      gnutls_transport_set_errno(xio->session, EAGAIN);
#else
      errno=EAGAIN;
#endif
      return (ssize_t)-1;
    }
    else {
#ifdef HAVE_GNUTLS_TRANSPORT_SET_ERRNO
      gnutls_transport_set_errno(xio->session, EINVAL);
#else
      errno=EINVAL;
#endif
      return (ssize_t)-1;
    }
  }
  else {
    GWEN_RINGBUFFER *rbuf;
    uint32_t maxBytes;
    const uint8_t *src;

    rbuf=GWEN_Io_LayerCodec_GetReadBuffer(io);
    assert(buf);
    maxBytes=GWEN_RingBuffer_GetMaxUnsegmentedRead(rbuf);
    if (maxBytes>len)
      maxBytes=len;
    src=(const uint8_t*)GWEN_RingBuffer_GetReadPointer(rbuf);
    if (maxBytes) {
      memmove(buf, src, maxBytes);
      GWEN_RingBuffer_SkipBytesRead(rbuf, maxBytes);
    }

#ifdef HAVE_GNUTLS_TRANSPORT_SET_ERRNO
    gnutls_transport_set_errno(xio->session, 0);
#else
    errno=0;
#endif
    DBG_VERBOUS(GWEN_LOGDOMAIN, "TLS PULL: returning %d bytes", maxBytes);
    /*GWEN_Text_DumpString(buf, maxBytes, stderr, 2);*/
    return maxBytes;
  }
}



ssize_t GWEN_Io_LayerTls_Push(gnutls_transport_ptr_t p, const void *buf, size_t len) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_TLS *xio;
  int rv;

  io=(GWEN_IO_LAYER*) p;
  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "TLS PUSH: %d bytes", (int)len);
  /*GWEN_Text_DumpString(buf, len, stderr, 2);*/

  rv=GWEN_Io_LayerCodec_CheckWriteOut(io);
  if (rv) {
    if (rv==GWEN_ERROR_TRY_AGAIN) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
#ifdef HAVE_GNUTLS_TRANSPORT_SET_ERRNO
      gnutls_transport_set_errno(xio->session, EAGAIN);
#else
      errno=EAGAIN;
#endif
      return (ssize_t)-1;
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
#ifdef HAVE_GNUTLS_TRANSPORT_SET_ERRNO
      gnutls_transport_set_errno(xio->session, EINVAL);
#else
      errno=EINVAL;
#endif
      return (ssize_t)-1;
    }
  }
  else {
    GWEN_RINGBUFFER *rbuf;
    uint32_t maxBytes;
    uint8_t *dst;
    uint32_t guiid=0;

    if (xio->connectRequest)
      guiid=GWEN_Io_Request_GetGuiId(xio->connectRequest);
    else if (xio->connectRequest)
      guiid=GWEN_Io_Request_GetGuiId(xio->disconnectRequest);

    rbuf=GWEN_Io_LayerCodec_GetWriteBuffer(io);
    assert(rbuf);
    maxBytes=GWEN_RingBuffer_GetMaxUnsegmentedWrite(rbuf);
    if (maxBytes>len)
      maxBytes=len;
    dst=(uint8_t*)GWEN_RingBuffer_GetWritePointer(rbuf);
    if (maxBytes) {
      memmove(dst, buf, maxBytes);
      GWEN_RingBuffer_SkipBytesWrite(rbuf, maxBytes);
    }

#ifdef HAVE_GNUTLS_TRANSPORT_SET_ERRNO
    gnutls_transport_set_errno(xio->session, 0);
#else
    errno=0;
#endif
    DBG_DEBUG(GWEN_LOGDOMAIN, "TLS PUSH: written %d bytes", maxBytes);
    return maxBytes;
  }
}



int GWEN_Io_LayerTls_Encode(GWEN_IO_LAYER *io, const uint8_t *pBuffer, uint32_t lBuffer) {
  GWEN_IO_LAYER_TLS *xio;
  ssize_t rv;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  rv=gnutls_record_send(xio->session, pBuffer, lBuffer);
  if (rv<0) {
    if (rv==GNUTLS_E_AGAIN)
      return GWEN_ERROR_TRY_AGAIN;
    else if (rv==GNUTLS_E_INTERRUPTED)
      return GWEN_ERROR_INTERRUPTED;
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_record_send: %d (%s)", (int)rv, gnutls_strerror(rv));
      return GWEN_ERROR_IO;
    }
  }
#if 0
  else {
    DBG_ERROR(0, "Sent this:");
    GWEN_Text_DumpString(pBuffer, rv,
			 stderr, 2);
  }
#endif
  return rv;
}



int GWEN_Io_LayerTls_Decode(GWEN_IO_LAYER *io, uint8_t *pBuffer, uint32_t lBuffer) {
  GWEN_IO_LAYER_TLS *xio;
  ssize_t rv;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  rv=gnutls_record_recv(xio->session, pBuffer, lBuffer);
  if (rv<0) {
    if (rv==GNUTLS_E_AGAIN)
      return GWEN_ERROR_TRY_AGAIN;
    else if (rv==GNUTLS_E_INTERRUPTED)
      return GWEN_ERROR_INTERRUPTED;
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_record_recv: %d (%s)", (int)rv, gnutls_strerror(rv));
      return GWEN_ERROR_IO;
    }
  }
  else if (rv==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
    return GWEN_ERROR_EOF;
  }
#if 0
  else {
    DBG_ERROR(0, "Received this:");
    GWEN_Text_DumpString(pBuffer, rv,
			 stderr, 2);
  }
#endif

  return rv;
}



int GWEN_Io_LayerTls_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_TLS *xio;
  GWEN_IO_LAYER_STATUS st;
  uint32_t lflags;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  st=GWEN_Io_Layer_GetStatus(io);
  lflags=GWEN_Io_Layer_GetFlags(io);

  switch(GWEN_Io_Request_GetType(r)) {
  case GWEN_Io_Request_TypeConnect:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusUnconnected &&
	st!=GWEN_Io_Layer_StatusDisconnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "Socket is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_INVALID);
      return GWEN_ERROR_NOT_OPEN;
    }

    /* check whether we already have a read request */
    if (xio->connectRequest) {
      DBG_INFO(GWEN_LOGDOMAIN, "There already is a connect request");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_IN_PROGRESS);
      return GWEN_ERROR_IN_PROGRESS;
    }
    else {
      /* prepare structures if necessary */
      GWEN_Io_LayerCodec_Reset(io);
      if (xio->prepared==0) {
	int rv;

	rv=GWEN_Io_LayerTls_Prepare(io);
	if (rv) {
	  GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, rv);
	  GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
	  return rv;
	}
      }

      /* enqueue request */
      xio->connectRequest=r;
      GWEN_Io_Request_Attach(xio->connectRequest);
      GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusConnecting);
    }
    break;

  case GWEN_Io_Request_TypeDisconnect:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "File is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      GWEN_Io_LayerCodec_Reset(io);
      return GWEN_ERROR_NOT_OPEN;
    }
    else {
      /* enqueue request */
      xio->disconnectRequest=r;
      GWEN_Io_Request_Attach(xio->disconnectRequest);
      GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnecting);
    }
    break;

  default:
    if (xio->addRequestFn)
      return xio->addRequestFn(io, r);
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "This request type is not supported (%d)", GWEN_Io_Request_GetType(r));
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_SUPPORTED);
      return GWEN_ERROR_NOT_SUPPORTED;
    }
  }

  return 0;
}



int GWEN_Io_LayerTls_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_TLS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  switch(GWEN_Io_Request_GetType(r)) {
  case GWEN_Io_Request_TypeConnect:
    if (xio->connectRequest==r) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Aborted connect request");
      GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
      GWEN_Io_LayerCodec_AbortRequests(io, GWEN_ERROR_ABORTED);
      xio->connectRequest=NULL;
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_ABORTED);
      GWEN_Io_Request_free(r);
      if (xio->prepared) {
	gnutls_deinit(xio->session);
	gnutls_certificate_free_credentials(xio->credentials);
	xio->prepared=0;
      }
    }
    else {
      /* not my request */
      DBG_INFO(GWEN_LOGDOMAIN, "Connect request not registered with this io layer");
      return GWEN_ERROR_INVALID;
    }
    break;

  case GWEN_Io_Request_TypeDisconnect:
    if (xio->disconnectRequest==r) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Aborted connect request");

      GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
      if (xio->connectRequest) {
	GWEN_IO_REQUEST *r;

	r=xio->connectRequest;
	xio->connectRequest=NULL;
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_ABORTED);
	GWEN_Io_Request_free(r);
      }
    }
    else {
      /* not my request */
      DBG_INFO(GWEN_LOGDOMAIN, "Disconnect request not registered with this io layer");
      return GWEN_ERROR_INVALID;
    }
    break;

  default:
    if (xio->delRequestFn)
      return xio->delRequestFn(io, r);
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "This request type is not supported (%d)", GWEN_Io_Request_GetType(r));
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_SUPPORTED);
      return GWEN_ERROR_NOT_SUPPORTED;
    }
  }

  return 0;
}



int GWEN_Io_LayerTls_HasWaitingRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_TLS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  if (xio->connectRequest || xio->disconnectRequest)
    return 1;
  if (xio->hasWaitingRequestsFn)
    return xio->hasWaitingRequestsFn(io);

  return 0;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerTls_WorkOnRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_TLS *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_TLS, io);
  assert(xio);

  /* work on connect request */
  if (xio->connectRequest) {
    GWEN_IO_REQUEST *r;
    int rv;

    r=xio->connectRequest;
    GWEN_Io_LayerCodec_SetCurrentGuiId(io, GWEN_Io_Request_GetGuiId(r));
    rv=gnutls_handshake(xio->session);
    if (rv<0) {
      if (rv!=GNUTLS_E_AGAIN &&
	  rv!=GNUTLS_E_INTERRUPTED) {
	DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_handshake: %d (%s) [%s]",
		  rv, gnutls_strerror(rv), gnutls_error_is_fatal(rv)?"fatal":"non-fatal");
	GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
	xio->connectRequest=NULL;
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_SSL);
	GWEN_Io_Request_free(r);
	if (xio->prepared) {
	  gnutls_deinit(xio->session);
	  gnutls_certificate_free_credentials(xio->credentials);
	  xio->prepared=0;
	}
	doneSomething=1;
      }
    }
    else {
      /* check certificate */
      doneSomething=1;
      GWEN_Io_Layer_SubFlags(io, GWEN_IO_LAYER_TLS_FLAGS_SECURE);
      rv=GWEN_Io_LayerTls_GetPeerCert(io, GWEN_Io_Request_GetGuiId(r));
      if (rv) {
	if (GWEN_Io_Layer_GetFlags(io) & GWEN_IO_LAYER_TLS_FLAGS_NEED_PEER_CERT) {
	  DBG_INFO(GWEN_LOGDOMAIN, "No peer certificate when needed, aborting connection");
	  GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
	  xio->connectRequest=NULL;
	  GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_SSL_SECURITY);
	  GWEN_Io_Request_free(r);
	  if (xio->prepared) {
	    gnutls_deinit(xio->session);
	    gnutls_certificate_free_credentials(xio->credentials);
	    xio->prepared=0;
	  }
	}
	else {
	  DBG_INFO(GWEN_LOGDOMAIN, "SSL connected (insecure)");
	  GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusConnected);
	  xio->connectRequest=NULL;
	  GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	  GWEN_Io_Request_free(r);
	}
      }
      else {
	/* present cert to the user */
	rv=GWEN_Gui_CheckCert(xio->peerCertDescr, io, GWEN_Io_Request_GetGuiId(r));
	if (rv) {
	  DBG_INFO(GWEN_LOGDOMAIN, "Peer cert not accepted (%d), aborting", rv);
	  GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
	  xio->connectRequest=NULL;
	  GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_SSL_SECURITY);
	  GWEN_Io_Request_free(r);
	  if (xio->prepared) {
	    gnutls_deinit(xio->session);
	    gnutls_certificate_free_credentials(xio->credentials);
	    xio->prepared=0;
	  }
	}
	else {
	  DBG_INFO(GWEN_LOGDOMAIN, "SSL connected (secure)");
          GWEN_Io_Layer_AddFlags(io, GWEN_IO_LAYER_TLS_FLAGS_SECURE);
	  GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusConnected);
	  xio->connectRequest=NULL;
	  GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	  GWEN_Io_Request_free(r);
	}
      }
    }
  }
  if (xio->disconnectRequest) {
    GWEN_IO_REQUEST *r;
    int rv;

    r=xio->disconnectRequest;
    GWEN_Io_LayerCodec_SetCurrentGuiId(io, GWEN_Io_Request_GetGuiId(r));
    rv=gnutls_bye(xio->session, GNUTLS_SHUT_RDWR);
    if (rv<0) {
      if (rv!=GNUTLS_E_AGAIN &&
	  rv!=GNUTLS_E_INTERRUPTED) {
	DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_bye: %d (%s)", rv, gnutls_strerror(rv));
	GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
	xio->disconnectRequest=NULL;
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_SSL);
	GWEN_Io_Request_free(r);
	GWEN_Io_LayerCodec_Reset(io);
	if (xio->prepared) {
	  gnutls_deinit(xio->session);
	  gnutls_certificate_free_credentials(xio->credentials);
	  xio->prepared=0;
	}
	doneSomething=1;
      }
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "SSL disconnected");
      GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
      xio->disconnectRequest=NULL;
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
      GWEN_Io_Request_free(r);
      GWEN_Io_LayerCodec_Reset(io);
      if (xio->prepared) {
	gnutls_deinit(xio->session);
	gnutls_certificate_free_credentials(xio->credentials);
	xio->prepared=0;
      }
      doneSomething=1;
    }
  }

  if (GWEN_Io_Layer_GetStatus(io)==GWEN_Io_Layer_StatusListening) {
    GWEN_IO_LAYER *newIo;

    newIo=GWEN_Io_Layer_GetNextIncomingLayer(GWEN_Io_Layer_GetBaseLayer(io));
    if (newIo) {
      GWEN_IO_LAYER *newNewIo;
      uint32_t fl;

      fl=GWEN_Io_Layer_GetFlags(io);
      newNewIo=GWEN_Io_LayerTls_new(newIo);
      GWEN_Io_Layer_AddFlags(newNewIo, GWEN_IO_LAYER_FLAGS_PASSIVE);
      GWEN_Io_Layer_AddFlags(newNewIo, fl & 0xffff);
      if (xio->localCertFile)
	GWEN_Io_LayerTls_SetLocalCertFile(newNewIo, xio->localCertFile);
      if (xio->localKeyFile)
	GWEN_Io_LayerTls_SetLocalKeyFile(newNewIo, xio->localKeyFile);
      if (xio->localTrustFile)
	GWEN_Io_LayerTls_SetLocalTrustFile(newNewIo, xio->localTrustFile);
      if (xio->dhParamFile)
	GWEN_Io_LayerTls_SetDhParamFile(newNewIo, xio->dhParamFile);

      if ((fl & GWEN_IO_LAYER_TLS_FLAGS_SET_PASSV_HOST_NAME) ||
	  (fl & GWEN_IO_LAYER_TLS_FLAGS_SET_PASSV_HOST_IP)) {
	GWEN_IO_LAYER *sp;

	/* try to retrieve the address from socket io layer if there is any */
	sp=GWEN_Io_Layer_FindBaseLayerByType(newIo, GWEN_IO_LAYER_SOCKET_TYPE);
	if (sp) {
	  GWEN_INETADDRESS *addr;

	  addr=GWEN_Io_LayerSocket_GetPeerAddr(sp);
	  if (addr) {
	    char addrBuffer[128];

            if (fl & GWEN_IO_LAYER_TLS_FLAGS_SET_PASSV_HOST_NAME)
	      GWEN_InetAddr_GetName(addr, addrBuffer, sizeof(addrBuffer)-1);
	    else
	      GWEN_InetAddr_GetAddress(addr, addrBuffer, sizeof(addrBuffer)-1);
	    addrBuffer[sizeof(addrBuffer)-1]=0;
	    if (*addrBuffer) {
	      DBG_INFO(GWEN_LOGDOMAIN, "Setting remote addr to [%s]", addrBuffer);
	      GWEN_Io_LayerTls_SetRemoteHostName(newNewIo, addrBuffer);
	    }
	  }
	}
      }

      GWEN_Io_Layer_AddIncomingLayer(io, newNewIo);
      doneSomething=1;
    }
  }

  if (xio->workOnRequestsFn &&
      xio->workOnRequestsFn(io)!=GWEN_Io_Layer_WorkResultBlocking) {
    doneSomething=1;
  }

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}


















