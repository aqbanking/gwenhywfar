/***************************************************************************
 $RCSfile$
                             -------------------
    begin       : Fri Feb 15 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "httpsession_p.h"
#include "i18n_l.h"

#include <gwenhywfar/syncio.h>
#include <gwenhywfar/syncio_tls.h>
#include <gwenhywfar/syncio_http.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <assert.h>


GWEN_INHERIT_FUNCTIONS(GWEN_HTTP_SESSION)



GWEN_HTTP_SESSION *GWEN_HttpSession_new(const char *url, const char *defaultProto, int defaultPort) {
  GWEN_HTTP_SESSION *sess;

  GWEN_NEW_OBJECT(GWEN_HTTP_SESSION, sess);
  assert(sess);
  sess->usage=1;
  GWEN_INHERIT_INIT(GWEN_HTTP_SESSION, sess);
  if (url)
    sess->url=strdup(url);
  if (defaultProto)
    sess->defaultProtocol=strdup(defaultProto);
  sess->defaultPort=defaultPort;

  return sess;
}



void GWEN_HttpSession_Attach(GWEN_HTTP_SESSION *sess) {
  assert(sess);
  assert(sess->usage);
  sess->usage++;
}



void GWEN_HttpSession_free(GWEN_HTTP_SESSION *sess) {
  if (sess) {
    assert(sess->usage);
    if (sess->usage==1) {
      GWEN_INHERIT_FINI(GWEN_HTTP_SESSION, sess);
      GWEN_SyncIo_free(sess->syncIo);
      free(sess->url);
      free(sess->defaultProtocol);
      free(sess->httpUserAgent);
      free(sess->httpContentType);
      GWEN_FREE_OBJECT(sess);
    }
    else {
      sess->usage--;
    }
  }
}



uint32_t GWEN_HttpSession_GetFlags(const GWEN_HTTP_SESSION *sess) {
  assert(sess);
  assert(sess->usage);

  return sess->flags;
}



void GWEN_HttpSession_SetFlags(GWEN_HTTP_SESSION *sess, uint32_t fl) {
  assert(sess);
  assert(sess->usage);

  sess->flags=fl;
}



void GWEN_HttpSession_AddFlags(GWEN_HTTP_SESSION *sess, uint32_t fl) {
  assert(sess);
  assert(sess->usage);

  sess->flags|=fl;
}



void GWEN_HttpSession_SubFlags(GWEN_HTTP_SESSION *sess, uint32_t fl) {
  assert(sess);
  assert(sess->usage);

  sess->flags&=~fl;
}



const char *GWEN_HttpSession_GetHttpUserAgent(const GWEN_HTTP_SESSION *sess) {
  assert(sess);
  assert(sess->usage);

  return sess->httpUserAgent;
}



void GWEN_HttpSession_SetHttpUserAgent(GWEN_HTTP_SESSION *sess, const char *s) {
  assert(sess);
  assert(sess->usage);

  free(sess->httpUserAgent);
  if (s)
    sess->httpUserAgent=strdup(s);
  else
    sess->httpUserAgent=NULL;
}



const char *GWEN_HttpSession_GetHttpContentType(const GWEN_HTTP_SESSION *sess) {
  assert(sess);
  assert(sess->usage);

  return sess->httpContentType;
}



void GWEN_HttpSession_SetHttpContentType(GWEN_HTTP_SESSION *sess, const char *s) {
  assert(sess);
  assert(sess->usage);

  free(sess->httpContentType);
  if (s)
    sess->httpContentType=strdup(s);
  else
    sess->httpContentType=NULL;
}



int GWEN_HttpSession_GetHttpVMajor(const GWEN_HTTP_SESSION *sess) {
  assert(sess);
  assert(sess->usage);

  return sess->httpVMajor;
}



void GWEN_HttpSession_SetHttpVMajor(GWEN_HTTP_SESSION *sess, int i) {
  assert(sess);
  assert(sess->usage);

  sess->httpVMajor=i;
}



int GWEN_HttpSession_GetHttpVMinor(const GWEN_HTTP_SESSION *sess) {
  assert(sess);
  assert(sess->usage);

  return sess->httpVMinor;
}



void GWEN_HttpSession_SetHttpVMinor(GWEN_HTTP_SESSION *sess, int i) {
  assert(sess);
  assert(sess->usage);

  sess->httpVMinor=i;
}






int GWEN_HttpSession_Init(GWEN_HTTP_SESSION *sess) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO *sioTls;
  GWEN_DB_NODE *db;
  int rv;

  rv=GWEN_Gui_GetSyncIo(sess->url,
			(sess->defaultProtocol)?(sess->defaultProtocol):"http",
			sess->defaultPort,
			&sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (strcasecmp(GWEN_SyncIo_GetTypeName(sio), GWEN_SYNCIO_HTTP_TYPE)!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "URL does not lead to a HTTP layer");
    GWEN_SyncIo_free(sio);
    return GWEN_ERROR_INVALID;
  }

  /* prepare TLS layer */
  sioTls=GWEN_SyncIo_GetBaseIoByTypeName(sio, GWEN_SYNCIO_TLS_TYPE);
  if (sioTls) {
    GWEN_SyncIo_AddFlags(sioTls,
			 GWEN_SYNCIO_TLS_FLAGS_ALLOW_V1_CA_CRT|
			 GWEN_SYNCIO_TLS_FLAGS_ADD_TRUSTED_CAS);

    if (sess->flags & GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3)
      GWEN_SyncIo_AddFlags(sioTls, GWEN_SYNCIO_TLS_FLAGS_FORCE_SSL_V3);
  }


  /* prepare HTTP out header */
  db=GWEN_SyncIo_Http_GetDbHeaderOut(sio);
  if (sess->flags & GWEN_HTTP_SESSION_FLAGS_NO_CACHE) {
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "Pragma", "no-cache");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "Cache-control", "no cache");
  }
  if (sess->httpContentType)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "Content-type", sess->httpContentType);

  if (sess->httpUserAgent)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "User-Agent", sess->httpUserAgent);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Connection", "close");
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Content-length", 0);

  sess->syncIo=sio;

  return 0;
}



int GWEN_HttpSession_Fini(GWEN_HTTP_SESSION *sess) {
  assert(sess);
  assert(sess->usage);

  if (sess->syncIo) {
    GWEN_SyncIo_Disconnect(sess->syncIo);
    GWEN_SyncIo_free(sess->syncIo);
    sess->syncIo=NULL;
  }

  return 0;
}



int GWEN_HttpSession_SendPacket(GWEN_HTTP_SESSION *sess,
                                const char *httpCommand,
				const uint8_t *buf, uint32_t blen) {
  int rv;

  assert(sess);
  assert(sess->usage);

  /* first connect to server */
  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Notice,
		       I18N("Connecting to server..."));
  rv=GWEN_SyncIo_Connect(sess->syncIo);
  if (rv==GWEN_ERROR_SSL) {
    GWEN_SYNCIO *sioTls;

    /* try again with alternated SSLv3 flag */
    DBG_NOTICE(GWEN_LOGDOMAIN,
	       "SSL-Error connecting (%d), retrying", rv);
    GWEN_SyncIo_Disconnect(sess->syncIo);

    sioTls=GWEN_SyncIo_GetBaseIoByTypeName(sess->syncIo, GWEN_SYNCIO_TLS_TYPE);
    if (sioTls) {
      if (sess->flags & GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3) {
	DBG_INFO(GWEN_LOGDOMAIN, "Retrying to connect (non-SSLv3)");
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Info,
			     I18N("Retrying to connect (non-SSLv3)"));
	GWEN_SyncIo_SubFlags(sioTls, GWEN_SYNCIO_TLS_FLAGS_FORCE_SSL_V3);
	rv=GWEN_SyncIo_Connect(sess->syncIo);
	if (rv==0) {
	  GWEN_HttpSession_SubFlags(sess, GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3);
	}
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "Retrying to connect (SSLv3)");
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Info,
			     I18N("Retrying to connect (SSLv3)"));
	GWEN_SyncIo_AddFlags(sioTls, GWEN_SYNCIO_TLS_FLAGS_FORCE_SSL_V3);
	rv=GWEN_SyncIo_Connect(sess->syncIo);
	if (rv==0) {
	  GWEN_HttpSession_AddFlags(sess, GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3);
	}
      }
    }
  }

  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not connect to server (%d)", rv);
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("Could not connect to server"));
    GWEN_SyncIo_Disconnect(sess->syncIo);
    return rv;
  }
  else {
    GWEN_DB_NODE *db;

    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Info,
			 I18N("Connected."));

    /* set command */
    db=GWEN_SyncIo_Http_GetDbCommandOut(sess->syncIo);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "command",
			 httpCommand);

    /* set content length */
    db=GWEN_SyncIo_Http_GetDbHeaderOut(sess->syncIo);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"Content-length", blen);

    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Info,
			 I18N("Sending message..."));

    /* send request */
    rv=GWEN_SyncIo_WriteForced(sess->syncIo, buf, blen);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Could not send message (%d)", rv);
      GWEN_Gui_ProgressLog2(0,
			    GWEN_LoggerLevel_Error,
			    I18N("Could not send message (%d)"),
			    rv);
      GWEN_SyncIo_Disconnect(sess->syncIo);
      return rv;
    }

    DBG_INFO(GWEN_LOGDOMAIN, "Message sent.");
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Info,
			 I18N("Message sent."));
    return 0;
  }
}



int GWEN_HttpSession__RecvPacket(GWEN_HTTP_SESSION *sess, GWEN_BUFFER *buf) {
  int rv;

  assert(sess);
  assert(sess->usage);

  rv=GWEN_SyncIo_Http_RecvBody(sess->syncIo, buf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else if (rv<200 || rv>299) {
    /* response is only ok for continuation (100) code */
    if (rv==100) {
      DBG_INFO(GWEN_LOGDOMAIN, "Continue...");
    }
    else {
      GWEN_DB_NODE *dbHeaderIn;

      dbHeaderIn=GWEN_SyncIo_Http_GetDbHeaderIn(sess->syncIo);

      if (rv==301 || rv==303 || rv==305 || rv==307) {
	/* moved */
	if (dbHeaderIn) {
	  const char *s;

	  s=GWEN_DB_GetCharValue(dbHeaderIn, "Location", 0, 0);
	  if (s) {
	    switch(rv) {
	    case 301:
	    case 303:
	      GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Warning, I18N("HTTP: Moved permanently to %s"), s);
	      break;
	    case 305:
	      GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Warning, I18N("HTTP: Use proxy at %s"), s);
	      break;
	    case 307:
	      GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Warning, I18N("HTTP: Moved temporarily to %s"), s);
	      break;
	    default:
	      GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Warning, I18N("HTTP: Moved to %s"), s);
	    } /* switch */
	  }
	}
      } /* if moved */
    }
  }

  return rv;
}



int GWEN_HttpSession_RecvPacket(GWEN_HTTP_SESSION *sess, GWEN_BUFFER *buf) {
  int rv;
  uint32_t pos;

  /* read response */
  pos=GWEN_Buffer_GetPos(buf);
  for (;;) {
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Info,
			 I18N("Waiting for response..."));
    rv=GWEN_HttpSession__RecvPacket(sess, buf);
    if (rv<0 || rv<200 || rv>299) {
      DBG_INFO(GWEN_LOGDOMAIN,
	       "Error receiving packet (%d)", rv);
      GWEN_SyncIo_Disconnect(sess->syncIo);
      return rv;
    }
    if (rv!=100)
      break;
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Info,
			 I18N("Received continuation response."));
    GWEN_Buffer_Crop(buf, 0, pos);
  }

  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Info,
		       I18N("Response received."));

  /* disconnect */
  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Info,
		       I18N("Disconnecting from server..."));
  GWEN_SyncIo_Disconnect(sess->syncIo);
  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Info,
		       I18N("Disconnected."));
  return 0;
}



int GWEN_HttpSession_ConnectionTest(GWEN_HTTP_SESSION *sess) {
  int rv;

  assert(sess);
  assert(sess->usage);

  /* first connect to server */
  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Notice,
		       I18N("Connecting to server..."));
  rv=GWEN_SyncIo_Connect(sess->syncIo);
  if (rv==GWEN_ERROR_SSL) {
    GWEN_SYNCIO *sioTls;

    /* try again with alternated SSLv3 flag */
    DBG_NOTICE(GWEN_LOGDOMAIN,
	       "SSL-Error connecting (%d), retrying", rv);
    GWEN_SyncIo_Disconnect(sess->syncIo);

    sioTls=GWEN_SyncIo_GetBaseIoByTypeName(sess->syncIo, GWEN_SYNCIO_TLS_TYPE);
    if (sioTls) {
      if (sess->flags & GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3) {
	DBG_INFO(GWEN_LOGDOMAIN, "Retrying to connect (non-SSLv3)");
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Info,
			     I18N("Retrying to connect (non-SSLv3)"));
	GWEN_SyncIo_SubFlags(sioTls, GWEN_SYNCIO_TLS_FLAGS_FORCE_SSL_V3);
	rv=GWEN_SyncIo_Connect(sess->syncIo);
	if (rv==0) {
	  GWEN_HttpSession_SubFlags(sess, GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3);
	}
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "Retrying to connect (SSLv3)");
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Info,
			     I18N("Retrying to connect (SSLv3)"));
	GWEN_SyncIo_AddFlags(sioTls, GWEN_SYNCIO_TLS_FLAGS_FORCE_SSL_V3);
	rv=GWEN_SyncIo_Connect(sess->syncIo);
	if (rv==0) {
	  GWEN_HttpSession_AddFlags(sess, GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3);
	}
      }
    }
  }

  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not connect to server (%d)", rv);
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("Could not connect to server"));
    GWEN_SyncIo_Disconnect(sess->syncIo);
    return rv;
  }
  else {
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Info,
			 I18N("Connected."));

    GWEN_SyncIo_Disconnect(sess->syncIo);
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Info,
			 I18N("Disconnected."));
    return 0;
  }
}




