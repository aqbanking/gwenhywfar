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

#include <gwenhywfar/io_socket.h>
#include <gwenhywfar/io_tls.h>
#include <gwenhywfar/io_http.h>
#include <gwenhywfar/io_buffered.h>
#include <gwenhywfar/iomanager.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <assert.h>


GWEN_INHERIT_FUNCTIONS(GWEN_HTTP_SESSION)



GWEN_HTTP_SESSION *GWEN_HttpSession_new(const char *url,
					uint32_t guiid) {
  GWEN_HTTP_SESSION *sess;

  GWEN_NEW_OBJECT(GWEN_HTTP_SESSION, sess);
  assert(sess);
  sess->usage=1;
  GWEN_INHERIT_INIT(GWEN_HTTP_SESSION, sess);
  if (url)
    sess->url=GWEN_Url_fromString(url);
  sess->guiid=guiid;

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
      GWEN_Io_Layer_free(sess->ioLayer);
      free(sess->httpUserAgent);
      free(sess->httpContentType);
      GWEN_FREE_OBJECT(sess);
    }
    else {
      sess->usage--;
    }
  }
}



GWEN_IO_LAYER *GWEN_HttpSession_GetIoLayer(const GWEN_HTTP_SESSION *sess) {
  assert(sess);
  assert(sess->usage);

  return sess->ioLayer;
}



uint32_t GWEN_HttpSession_GetGuiId(const GWEN_HTTP_SESSION *sess) {
  assert(sess);
  assert(sess->usage);

  return sess->guiid;
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






int GWEN_HttpSession__SetAddress(GWEN_HTTP_SESSION *sess,
				 GWEN_INETADDRESS *addr,
				 const char *peerAddr) {
  int err;

  err=GWEN_InetAddr_SetAddress(addr, peerAddr);
  if (err) {
    char dbgbuf[256];

    snprintf(dbgbuf, sizeof(dbgbuf)-1,
	     I18N("Resolving hostname \"%s\" ..."),
	     peerAddr);
    dbgbuf[sizeof(dbgbuf)-1]=0;
    GWEN_Gui_ProgressLog(sess->guiid,
			 GWEN_LoggerLevel_Notice,
			 dbgbuf);
    DBG_INFO(GWEN_LOGDOMAIN, "Resolving hostname \"%s\"",
	     peerAddr);
    err=GWEN_InetAddr_SetName(addr, peerAddr);
    if (err) {
      snprintf(dbgbuf, sizeof(dbgbuf)-1,
	       I18N("Unknown hostname \"%s\""),
	       peerAddr);
      dbgbuf[sizeof(dbgbuf)-1]=0;
      GWEN_Gui_ProgressLog(sess->guiid,
			   GWEN_LoggerLevel_Error,
			   dbgbuf);
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Error resolving hostname \"%s\":",
		peerAddr);
      DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
      return err;
    }
    else {
      char addrBuf[256];

      err=GWEN_InetAddr_GetAddress(addr, addrBuf, sizeof(addrBuf)-1);
      addrBuf[sizeof(addrBuf)-1]=0;
      if (err) {
	DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
      }
      else {
	snprintf(dbgbuf, sizeof(dbgbuf)-1,
		 I18N("IP address is %s"),
		 addrBuf);
	dbgbuf[sizeof(dbgbuf)-1]=0;
	GWEN_Gui_ProgressLog(sess->guiid,
			     GWEN_LoggerLevel_Notice,
			     dbgbuf);
      }
    }
  }

  return 0;
}



int GWEN_HttpSession_Init(GWEN_HTTP_SESSION *sess) {
  int port;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER *ioBase;
  int rv;
  GWEN_DB_NODE *db;
  const char *s;
  int isHttps;

  /* prepare socket layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  io=GWEN_Io_LayerSocket_new(sk);

  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  rv=GWEN_HttpSession__SetAddress(sess, addr, GWEN_Url_GetServer(sess->url));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_InetAddr_free(addr);
    return rv;
  }

  /* determine protocol */
  s=GWEN_Url_GetProtocol(sess->url);
  if (s) {
    if (strcasecmp(s, "http")==0) {
      isHttps=0;
    }
    else if (strcasecmp(s, "https")==0) {
      isHttps=1;
    }
    else {
      char dbgbuf[256];

      DBG_ERROR(GWEN_LOGDOMAIN, "Unknown protocol \"%s\"", s);
      snprintf(dbgbuf, sizeof(dbgbuf)-1,
	       I18N("Unknown protocol \"%s\""),
	       s);
      dbgbuf[sizeof(dbgbuf)-1]=0;

      GWEN_Gui_ProgressLog(sess->guiid,
			   GWEN_LoggerLevel_Error,
			   dbgbuf);
      GWEN_InetAddr_free(addr);
      return GWEN_ERROR_INVALID;
    }
  }
  else {
    /* default */
    isHttps=1;
  }

  port=GWEN_Url_GetPort(sess->url);
  if (port==0) {
    if (isHttps)
      port=443;
    else
      port=80;
  }
  GWEN_InetAddr_SetPort(addr, port);
  GWEN_Io_LayerSocket_SetPeerAddr(io, addr);

  /* prepare TLS layer */
  if (isHttps) {
    ioBase=io;
    io=GWEN_Io_LayerTls_new(ioBase);
    if (io==NULL) {
      GWEN_InetAddr_free(addr);
      GWEN_Io_Layer_free(ioBase);
      return GWEN_ERROR_GENERIC;
    }
    GWEN_Io_Layer_AddFlags(io,
			   GWEN_IO_LAYER_TLS_FLAGS_ALLOW_V1_CA_CRT|
			   GWEN_IO_LAYER_TLS_FLAGS_ADD_TRUSTED_CAS);

    if (sess->flags & GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3)
      GWEN_Io_Layer_AddFlags(io, GWEN_IO_LAYER_TLS_FLAGS_FORCE_SSL_V3);

    GWEN_Io_LayerTls_SetRemoteHostName(io, GWEN_Url_GetServer(sess->url));
  }

  /* prepare buffered layer */
  ioBase=io;
  io=GWEN_Io_LayerBuffered_new(ioBase);
  if (io==NULL) {
    GWEN_InetAddr_free(addr);
    GWEN_Io_Layer_free(ioBase);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Io_Layer_AddFlags(io, GWEN_IO_LAYER_BUFFERED_FLAGS_DOSMODE);

  /* prepare HTTP layer */
  ioBase=io;
  io=GWEN_Io_LayerHttp_new(ioBase);
  if (io==NULL) {
    GWEN_InetAddr_free(addr);
    GWEN_Io_Layer_free(ioBase);
    return GWEN_ERROR_GENERIC;
  }

  /* prepare HTTP command line */
  db=GWEN_Io_LayerHttp_GetDbCommandOut(io);
  if (sess->httpVMajor) {
    char numbuf[32];

    snprintf(numbuf, sizeof(numbuf)-1, "HTTP/%d.%d",
	     sess->httpVMajor, sess->httpVMinor);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "protocol", numbuf);
  }
  else
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "protocol", "HTTP/1.0");

  if (1) {
    GWEN_BUFFER *pbuf;

    pbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Url_toCommandString(sess->url, pbuf);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(pbuf);
      GWEN_InetAddr_free(addr);
      GWEN_Io_Layer_free(ioBase);
      return rv;
    }
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "url",
			 GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);
  }

  /* prepare HTTP out header */
  db=GWEN_Io_LayerHttp_GetDbHeaderOut(io);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "Host", GWEN_Url_GetServer(sess->url));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "Pragma", "no-cache");
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "Cache-control", "no cache");
  if (sess->httpContentType)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "Content-type", sess->httpContentType);

  if (sess->httpUserAgent)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "User-Agent", sess->httpUserAgent);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Connection", "close");
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Content-length", 0);

  /* now register the layer */
  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not register io layer (%d)", rv);
    GWEN_InetAddr_free(addr);
    GWEN_Io_Layer_free(io);
    return 0;
  }

  sess->ioLayer=io;
  GWEN_InetAddr_free(addr);
  return 0;
}



int GWEN_HttpSession_Fini(GWEN_HTTP_SESSION *sess) {
  assert(sess);
  assert(sess->usage);

  GWEN_Io_Layer_free(sess->ioLayer);
  sess->ioLayer=0;
  return 0;
}



int GWEN_HttpSession_SendPacket(GWEN_HTTP_SESSION *sess,
                                const char *httpCommand,
				const uint8_t *buf, uint32_t blen,
				int timeout) {
  int rv;

  assert(sess);
  assert(sess->usage);

  /* first connect to server */
  GWEN_Gui_ProgressLog(sess->guiid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Connecting to server..."));
  rv=GWEN_Io_Layer_ConnectRecursively(sess->ioLayer, NULL, 0,
				      sess->guiid, 30000);
  if (rv==GWEN_ERROR_SSL) {
    GWEN_IO_LAYER *ioTls;

    /* try again with alternated SSLv3 flag */
    DBG_NOTICE(GWEN_LOGDOMAIN,
	       "SSL-Error connecting (%d), retrying", rv);
    GWEN_Io_Layer_DisconnectRecursively(sess->ioLayer, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					sess->guiid, 2000);
    ioTls=GWEN_Io_Layer_FindBaseLayerByType(sess->ioLayer,
					    GWEN_IO_LAYER_TLS_TYPE);
    assert(ioTls);

    if (sess->flags & GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3) {
      DBG_INFO(GWEN_LOGDOMAIN, "Retrying to connect (non-SSLv3)");
      GWEN_Gui_ProgressLog(sess->guiid,
			   GWEN_LoggerLevel_Info,
			   I18N("Retrying to connect (non-SSLv3)"));
      GWEN_Io_Layer_SubFlags(ioTls, GWEN_IO_LAYER_TLS_FLAGS_FORCE_SSL_V3);
      rv=GWEN_Io_Layer_ConnectRecursively(sess->ioLayer, NULL, 0,
					  sess->guiid, 30000);
      if (rv==0) {
	GWEN_HttpSession_SubFlags(sess, GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3);
      }
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "Retrying to connect (SSLv3)");
      GWEN_Gui_ProgressLog(sess->guiid,
			   GWEN_LoggerLevel_Info,
			   I18N("Retrying to connect (SSLv3)"));
      GWEN_Io_Layer_AddFlags(ioTls, GWEN_IO_LAYER_TLS_FLAGS_FORCE_SSL_V3);
      rv=GWEN_Io_Layer_ConnectRecursively(sess->ioLayer, NULL, 0,
					  sess->guiid, 30000);
      if (rv==0) {
	GWEN_HttpSession_AddFlags(sess, GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3);
      }
    }
  }

  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not connect to server (%d)", rv);
    GWEN_Gui_ProgressLog(sess->guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("Could not connect to server"));
    GWEN_Io_Layer_DisconnectRecursively(sess->ioLayer, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					sess->guiid, 2000);
    return rv;
  }
  else {
    GWEN_DB_NODE *db;

    GWEN_Gui_ProgressLog(sess->guiid,
			 GWEN_LoggerLevel_Info,
			 I18N("Connected."));

    /* set command */
    db=GWEN_Io_LayerHttp_GetDbCommandOut(sess->ioLayer);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "command",
			 httpCommand);

    /* set content length */
    db=GWEN_Io_LayerHttp_GetDbHeaderOut(sess->ioLayer);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"Content-length", blen);

    GWEN_Gui_ProgressLog(sess->guiid,
			 GWEN_LoggerLevel_Info,
			 I18N("Sending message..."));
    rv=GWEN_Io_Layer_WriteBytes(sess->ioLayer,
				buf, blen,
				GWEN_IO_REQUEST_FLAGS_PACKETBEGIN |
				GWEN_IO_REQUEST_FLAGS_PACKETEND |
				GWEN_IO_REQUEST_FLAGS_FLUSH,
				sess->guiid, timeout);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Could not send message (%d)", rv);
      GWEN_Gui_ProgressLog(sess->guiid,
			   GWEN_LoggerLevel_Error,
			   I18N("Could not send message"));
      GWEN_Io_Layer_DisconnectRecursively(sess->ioLayer, NULL,
					  GWEN_IO_REQUEST_FLAGS_FORCE,
					  sess->guiid, 2000);
      return rv;
    }

    DBG_INFO(GWEN_LOGDOMAIN, "Message sent.");
    GWEN_Gui_ProgressLog(sess->guiid,
			 GWEN_LoggerLevel_Info,
			 I18N("Message sent."));
    return 0;
  }
}



int GWEN_HttpSession__RecvPacket(GWEN_HTTP_SESSION *sess,
				 GWEN_BUFFER *buf, int timeout) {
  int rv;
  GWEN_DB_NODE *db;
  int code;

  assert(sess);
  assert(sess->usage);

  /* recv packet (this reads the HTTP body) */
  rv=GWEN_Io_Layer_ReadPacketToBuffer(sess->ioLayer, buf, 0,
				      sess->guiid, 30000);
  if (rv<0) {
    if (GWEN_Buffer_GetUsedBytes(buf)) {
      /* data received, check for common error codes */
      if (rv==GWEN_ERROR_EOF || (rv==GWEN_ERROR_IO)) {
	DBG_INFO(GWEN_LOGDOMAIN,
		 "We received an error, but we still got data, "
		 "so we ignore the error here");
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "No message received (%d)", rv);
	GWEN_Gui_ProgressLog(sess->guiid,
			     GWEN_LoggerLevel_Error,
			     I18N("No message received"));
	return rv;
      }
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "No message received (%d)", rv);
      GWEN_Gui_ProgressLog(sess->guiid,
			   GWEN_LoggerLevel_Error,
			   I18N("No message received"));
      return rv;
    }
  }

  /* check for status and log it */
  db=GWEN_Io_LayerHttp_GetDbStatusIn(sess->ioLayer);
  code=GWEN_DB_GetIntValue(db, "code", 0, 0);
  if (code) {
    GWEN_BUFFER *lbuf;
    char sbuf[32];
    const char *text;

    lbuf=GWEN_Buffer_new(0, 64, 0, 1);
    GWEN_Buffer_AppendString(lbuf, "HTTP-Status: ");
    snprintf(sbuf, sizeof(sbuf)-1, "%d", code);
    sbuf[sizeof(sbuf)-1]=0;
    GWEN_Buffer_AppendString(lbuf, sbuf);
    text=GWEN_DB_GetCharValue(db, "text", 0, NULL);
    if (text) {
      GWEN_Buffer_AppendString(lbuf, " (");
      GWEN_Buffer_AppendString(lbuf, text);
      GWEN_Buffer_AppendString(lbuf, ")");
    }
    DBG_DEBUG(GWEN_LOGDOMAIN, "Status: %d (%s)",
	      code, text);
    if (code<200 || code>299) {
      /* response is only ok for continuation (100) code */
      if (code!=100) {
	GWEN_DB_NODE *dbHeaderIn;

        dbHeaderIn=GWEN_Io_LayerHttp_GetDbHeaderIn(sess->ioLayer);
	DBG_ERROR(GWEN_LOGDOMAIN,
		  "Got an error response (%d: %s)",
		  code, text);
	GWEN_Gui_ProgressLog(sess->guiid,
			     GWEN_LoggerLevel_Error,
			     GWEN_Buffer_GetStart(lbuf));
	GWEN_Buffer_Reset(lbuf);

	if (code==301 || code==303 || code==305 || code==307) {
	  /* moved */
	  if (dbHeaderIn) {
	    const char *s;

	    s=GWEN_DB_GetCharValue(dbHeaderIn, "Location", 0, 0);
	    if (s) {
	      switch(code) {
	      case 301:
	      case 303:
		GWEN_Buffer_AppendString(lbuf,
					 I18N("HTTP: Moved permanently"));
		break;
	      case 305:
		GWEN_Buffer_AppendString(lbuf,
					 I18N("HTTP: Use proxy"));
		break;
	      case 307:
		GWEN_Buffer_AppendString(lbuf,
					 I18N("HTTP: Moved temporarily"));
		break;
	      default:
		GWEN_Buffer_AppendString(lbuf,
					 I18N("HTTP: Moved"));
	      } /* switch */
	      GWEN_Buffer_AppendString(lbuf, " (");
	      GWEN_Buffer_AppendString(lbuf, s);
	      GWEN_Buffer_AppendString(lbuf, ")");

	      GWEN_Gui_ProgressLog(sess->guiid,
				   GWEN_LoggerLevel_Warning,
                                   GWEN_Buffer_GetStart(lbuf));
	    }
	  }
	}
      }
      else {
	GWEN_Gui_ProgressLog(sess->guiid,
			     GWEN_LoggerLevel_Notice,
			     GWEN_Buffer_GetStart(lbuf));
      }
    }
    else {
      GWEN_Gui_ProgressLog(sess->guiid,
			   GWEN_LoggerLevel_Info,
			   GWEN_Buffer_GetStart(lbuf));
    }
    GWEN_Buffer_free(lbuf);
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No HTTP status code received");
    GWEN_Gui_ProgressLog(sess->guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("No HTTP status code received"));
    code=GWEN_ERROR_BAD_DATA;
  }

  return code;
}



int GWEN_HttpSession_RecvPacket(GWEN_HTTP_SESSION *sess,
				GWEN_BUFFER *buf, int timeout) {
  int rv;
  uint32_t pos;

  /* read response */
  pos=GWEN_Buffer_GetPos(buf);
  for (;;) {
    GWEN_Gui_ProgressLog(sess->guiid,
			 GWEN_LoggerLevel_Info,
			 I18N("Waiting for response..."));
    rv=GWEN_HttpSession__RecvPacket(sess, buf, timeout);
    if (rv<0 || rv<200 || rv>299) {
      DBG_INFO(GWEN_LOGDOMAIN,
	       "Error receiving packet (%d)", rv);
      GWEN_Io_Layer_DisconnectRecursively(sess->ioLayer, NULL,
					  GWEN_IO_REQUEST_FLAGS_FORCE,
					  sess->guiid, 2000);
      return rv;
    }
    if (rv!=100)
      break;
    GWEN_Gui_ProgressLog(sess->guiid,
			 GWEN_LoggerLevel_Info,
			 I18N("Received continuation response."));
    GWEN_Buffer_Crop(buf, 0, pos);
  }

  GWEN_Gui_ProgressLog(sess->guiid,
		       GWEN_LoggerLevel_Info,
		       I18N("Response received."));

  /* disconnect */
  GWEN_Gui_ProgressLog(sess->guiid,
		       GWEN_LoggerLevel_Info,
		       I18N("Disconnecting from server..."));
  GWEN_Io_Layer_DisconnectRecursively(sess->ioLayer, NULL,
				      GWEN_IO_REQUEST_FLAGS_FORCE,
				      sess->guiid, 2000);
  GWEN_Gui_ProgressLog(sess->guiid,
		       GWEN_LoggerLevel_Info,
		       I18N("Disconnected."));

  return 0;
}







