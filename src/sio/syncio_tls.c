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
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#include <gcrypt.h>



GWEN_INHERIT(GWEN_SYNCIO, GWEN_SYNCIO_TLS)


GWEN_SYNCIO *GWEN_SyncIo_Tls_new(GWEN_SYNCIO *baseIo) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_TLS *xio;

  assert(baseIo);
  sio=GWEN_SyncIo_new(GWEN_SYNCIO_TLS_TYPE, baseIo);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_TLS, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio, xio, GWEN_SyncIo_Tls_FreeData);

  GWEN_SyncIo_SetConnectFn(sio, GWEN_SyncIo_Tls_Connect);
  GWEN_SyncIo_SetDisconnectFn(sio, GWEN_SyncIo_Tls_Disconnect);
  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_Tls_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_Tls_Write);

  return sio;
}



void GWENHYWFAR_CB GWEN_SyncIo_Tls_FreeData(void *bp, void *p) {
  GWEN_SYNCIO_TLS *xio;

  xio=(GWEN_SYNCIO_TLS*) p;
  free(xio->localCertFile);
  free(xio->localKeyFile);
  free(xio->localTrustFile);
  free(xio->dhParamFile);
  free(xio->hostName);
  GWEN_FREE_OBJECT(xio);
}



const char *GWEN_SyncIo_Tls_GetLocalCertFile(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  return xio->localCertFile;
}



void GWEN_SyncIo_Tls_SetLocalCertFile(GWEN_SYNCIO *sio, const char *s) {
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  free(xio->localCertFile);
  if (s) xio->localCertFile=strdup(s);
  else xio->localCertFile=NULL;
}



const char *GWEN_SyncIo_Tls_GetLocalKeyFile(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  return xio->localKeyFile;
}



void GWEN_SyncIo_Tls_SetLocalKeyFile(GWEN_SYNCIO *sio, const char *s) {
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  free(xio->localKeyFile);
  if (s) xio->localKeyFile=strdup(s);
  else xio->localKeyFile=NULL;
}



const char *GWEN_SyncIo_Tls_GetLocalTrustFile(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  return xio->localTrustFile;
}



void GWEN_SyncIo_Tls_SetLocalTrustFile(GWEN_SYNCIO *sio, const char *s) {
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  free(xio->localTrustFile);
  if (s) xio->localTrustFile=strdup(s);
  else xio->localTrustFile=NULL;
}



const char *GWEN_SyncIo_Tls_GetDhParamFile(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  return xio->dhParamFile;
}



void GWEN_SyncIo_Tls_SetDhParamFile(GWEN_SYNCIO *sio, const char *s) {
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  free(xio->dhParamFile);
  if (s) xio->dhParamFile=strdup(s);
  else xio->dhParamFile=NULL;
}



const char *GWEN_SyncIo_Tls_GetRemoteHostName(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  return xio->hostName;
}



void GWEN_SyncIo_Tls_SetRemoteHostName(GWEN_SYNCIO *sio, const char *s) {
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  free(xio->hostName);
  if (s) xio->hostName=strdup(s);
  else xio->hostName=NULL;
}



GWEN_SSLCERTDESCR *GWEN_SyncIo_Tls_GetPeerCertDescr(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_TLS *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  return xio->peerCertDescr;
}



int GWEN_SyncIo_Tls__readFile(const char *fname, GWEN_BUFFER *buf) {
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




#if 0
static int GWEN_SyncIo_Tls_AddCaCertFolder(GWEN_SYNCIO *sio, const char *folder) {
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
      while(se) {
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
            DBG_INFO(GWEN_LOGDOMAIN,
                     "Added %d trusted certs from [%s]", rv, s);
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
#endif



int GWEN_SyncIo_Tls_Prepare(GWEN_SYNCIO *sio) {
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
  } else { /* use default ciphers from GnuTLS */
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
#if 0
# ifndef OS_WIN32
    /* try to find OpenSSL certificates */
    if (trustFileSet==0) {
      GWEN_STRINGLIST *paths;
      GWEN_BUFFER *nbuf;

      paths=GWEN_StringList_new();
      GWEN_StringList_AppendString(paths, "/etc/ssl/certs", 0, 0);

      nbuf=GWEN_Buffer_new(0, 256, 0, 1);
      rv=GWEN_Directory_FindFileInPaths(paths,
                                        "ca-certificates.crt",
                                        nbuf);
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
#endif


    if (trustFileSet==0) {
      GWEN_STRINGLIST *paths;

      /* try to find our trust file */
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
          rv=gnutls_certificate_set_x509_trust_file(xio->credentials,
              GWEN_Buffer_GetStart(nbuf),
              GNUTLS_X509_FMT_PEM);
          if (rv<=0) {
            DBG_ERROR(GWEN_LOGDOMAIN,
                      "gnutls_certificate_set_x509_trust_file(%s): %d (%s)",
                      GWEN_Buffer_GetStart(nbuf), rv, gnutls_strerror(rv));
          }
          else {
            DBG_INFO(GWEN_LOGDOMAIN,
                     "Added %d trusted certs", rv);
            trustFileSet=1;
          }
        }
        GWEN_Buffer_free(nbuf);
      }
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



void GWEN_SyncIo_Tls_UndoPrepare(GWEN_SYNCIO *sio) {
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



int GWEN_SyncIo_Tls_GetPeerCert(GWEN_SYNCIO *sio) {
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
  GWEN_BUFFER *sbuf=NULL;

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

  for (i=0; i<cert_list_size; i++) {
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
          GWEN_Text_ToHexBuffer((const char*)(n.data), n.size, kbuf, 0, 0, 0);
          GWEN_SslCertDescr_SetPubKeyModulus(certDescr, GWEN_Buffer_GetStart(kbuf));
          GWEN_Buffer_Reset(kbuf);
        }

        if (e.data && e.size) {
          /* store public exponent */
          GWEN_Text_ToHexBuffer((const char*)(e.data), e.size, kbuf, 0, 0, 0);
          GWEN_SslCertDescr_SetPubKeyExponent(certDescr, GWEN_Buffer_GetStart(kbuf));
          GWEN_Buffer_Reset(kbuf);
        }

        GWEN_Buffer_free(kbuf);
        if (n.data)
          gcry_free(n.data);
        if (e.data)
          gcry_free(e.data);
      }

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



ssize_t GWEN_SyncIo_Tls_Pull(gnutls_transport_ptr_t p, void *buf, size_t len) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_TLS *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  sio=(GWEN_SYNCIO*) p;
  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "TLS PULL: %d bytes", (int)len);
  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  rv=GWEN_SyncIo_Read(baseIo, buf, len);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
#ifdef HAVE_GNUTLS_TRANSPORT_SET_ERRNO
    gnutls_transport_set_errno(xio->session, errno);
#endif
    return (ssize_t)-1;
  }

#ifdef HAVE_GNUTLS_TRANSPORT_SET_ERRNO
  gnutls_transport_set_errno(xio->session, 0);
#else
  errno=0;
#endif
  DBG_VERBOUS(GWEN_LOGDOMAIN, "TLS PULL: returning %d bytes", rv);
  /*GWEN_Text_DumpString(buf, rv, 2);*/
  return rv;
}



ssize_t GWEN_SyncIo_Tls_Push(gnutls_transport_ptr_t p, const void *buf, size_t len) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_TLS *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  sio=(GWEN_SYNCIO*) p;
  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "TLS PUSH: %d bytes", (int)len);
  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  rv=GWEN_SyncIo_Write(baseIo, buf, len);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
#ifdef HAVE_GNUTLS_TRANSPORT_SET_ERRNO
    gnutls_transport_set_errno(xio->session, errno);
#endif
    return (ssize_t)-1;
  }

#ifdef HAVE_GNUTLS_TRANSPORT_SET_ERRNO
  gnutls_transport_set_errno(xio->session, 0);
#endif
  DBG_VERBOUS(GWEN_LOGDOMAIN, "TLS PUSH: returning %d bytes", rv);
  /*GWEN_Text_DumpString(buf, rv, 2);*/
  return rv;
}



void GWEN_SyncIo_Tls_ShowCipherInfo(GWEN_SYNCIO *sio) {
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
  switch(gnutls_cipher_get(xio->session)) {
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
  case GNUTLS_CIPHER_AES_192_CBC: // new in gnutls-2.9.8, so i.e. not available in gnutls-2.8.x
  default:
    break;
  }
}



int GWENHYWFAR_CB GWEN_SyncIo_Tls_Connect(GWEN_SYNCIO *sio) {
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
      rv=GWEN_Gui_CheckCert(xio->peerCertDescr, sio, 0);
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



int GWENHYWFAR_CB GWEN_SyncIo_Tls_Disconnect(GWEN_SYNCIO *sio) {
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
                                       uint32_t size) {
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
      return GWEN_ERROR_SSL_PREMATURE_CLOSE;
    }
#endif
    return GWEN_ERROR_SSL;
  }

#ifdef GWEN_TLS_DEBUG
  DBG_ERROR(0, "Received this:");
  GWEN_Text_DumpString((const char*) buffer, rv, 2);
#endif

  return rv;
}



int GWENHYWFAR_CB GWEN_SyncIo_Tls_Write(GWEN_SYNCIO *sio,
                                        const uint8_t *buffer,
                                        uint32_t size) {
  GWEN_SYNCIO_TLS *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_TLS, sio);
  assert(xio);

#ifdef GWEN_TLS_DEBUG
  DBG_ERROR(0, "Sending this:");
  GWEN_Text_DumpString((const char*) buffer, size, 2);
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







