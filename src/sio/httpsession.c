/***************************************************************************
    begin       : Fri Feb 15 2008
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "httpsession_p.h"
#include "i18n_l.h"

#include <gwenhywfar/syncio.h>
#include <gwenhywfar/syncio_socket.h>
#include <gwenhywfar/syncio_tls.h>
#include <gwenhywfar/syncio_http.h>
#include <gwenhywfar/syncio_file.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>

#include <assert.h>
#include <unistd.h>


GWEN_INHERIT_FUNCTIONS(GWEN_HTTP_SESSION)



GWEN_HTTP_SESSION *GWEN_HttpSession_new(const char *url, const char *defaultProto, int defaultPort)
{
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



GWEN_HTTP_SESSION *GWEN_HttpSession_fromSocketPassive(GWEN_SOCKET *sk, const char *proto, int port)
{
  GWEN_HTTP_SESSION *sess;
  GWEN_SYNCIO *baseSio;
  GWEN_SYNCIO *sio;

  GWEN_NEW_OBJECT(GWEN_HTTP_SESSION, sess);
  assert(sess);
  sess->usage=1;
  GWEN_INHERIT_INIT(GWEN_HTTP_SESSION, sess);

  baseSio=GWEN_SyncIo_Socket_TakeOver(sk);
  if (baseSio==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error on GWEN_SyncIo_Socket_TakeOver()");
    GWEN_HttpSession_free(sess);
    return NULL;
  }

  /* extend syncio to support the given protocol */
  sio=GWEN_Gui_ExtendSyncIo(NULL, proto, port, baseSio);
  if (sio==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error on GWEN_Gui_ExtendSyncIo()");
    GWEN_HttpSession_free(sess);
    return NULL;
  }

  sess->syncIo=sio;
  sess->flags|=GWEN_HTTP_SESSION_FLAGS_PASSIVE;

  /* add PASSIVE flag to every syncIO in the chain */
  while(sio) {
    GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FLAGS_PASSIVE);
    sio=GWEN_SyncIo_GetBaseIo(sio);
  }

  return sess;
}



void GWEN_HttpSession_Attach(GWEN_HTTP_SESSION *sess)
{
  assert(sess);
  assert(sess->usage);
  sess->usage++;
}



void GWEN_HttpSession_free(GWEN_HTTP_SESSION *sess)
{
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



GWEN_HTTPSESSION_INITSYNCIO_FN GWEN_HttpSession_SetInitSyncIoFn(GWEN_HTTP_SESSION *sess,
                                                                GWEN_HTTPSESSION_INITSYNCIO_FN f)
{
  GWEN_HTTPSESSION_INITSYNCIO_FN oldFn;

  oldFn=sess->initSyncIoFn;
  sess->initSyncIoFn=f;
  return oldFn;
}



uint32_t GWEN_HttpSession_GetFlags(const GWEN_HTTP_SESSION *sess)
{
  assert(sess);
  assert(sess->usage);

  return sess->flags;
}



void GWEN_HttpSession_SetFlags(GWEN_HTTP_SESSION *sess, uint32_t fl)
{
  assert(sess);
  assert(sess->usage);

  sess->flags=fl;
}



void GWEN_HttpSession_AddFlags(GWEN_HTTP_SESSION *sess, uint32_t fl)
{
  assert(sess);
  assert(sess->usage);

  sess->flags|=fl;
}



void GWEN_HttpSession_SubFlags(GWEN_HTTP_SESSION *sess, uint32_t fl)
{
  assert(sess);
  assert(sess->usage);

  sess->flags&=~fl;
}



const char *GWEN_HttpSession_GetHttpUserAgent(const GWEN_HTTP_SESSION *sess)
{
  assert(sess);
  assert(sess->usage);

  return sess->httpUserAgent;
}



void GWEN_HttpSession_SetHttpUserAgent(GWEN_HTTP_SESSION *sess, const char *s)
{
  assert(sess);
  assert(sess->usage);

  free(sess->httpUserAgent);
  if (s)
    sess->httpUserAgent=strdup(s);
  else
    sess->httpUserAgent=NULL;
}



const char *GWEN_HttpSession_GetHttpContentType(const GWEN_HTTP_SESSION *sess)
{
  assert(sess);
  assert(sess->usage);

  return sess->httpContentType;
}



void GWEN_HttpSession_SetHttpContentType(GWEN_HTTP_SESSION *sess, const char *s)
{
  assert(sess);
  assert(sess->usage);

  free(sess->httpContentType);
  if (s)
    sess->httpContentType=strdup(s);
  else
    sess->httpContentType=NULL;
}



int GWEN_HttpSession_GetHttpVMajor(const GWEN_HTTP_SESSION *sess)
{
  assert(sess);
  assert(sess->usage);

  return sess->httpVMajor;
}



void GWEN_HttpSession_SetHttpVMajor(GWEN_HTTP_SESSION *sess, int i)
{
  assert(sess);
  assert(sess->usage);

  sess->httpVMajor=i;
}



int GWEN_HttpSession_GetHttpVMinor(const GWEN_HTTP_SESSION *sess)
{
  assert(sess);
  assert(sess->usage);

  return sess->httpVMinor;
}



void GWEN_HttpSession_SetHttpVMinor(GWEN_HTTP_SESSION *sess, int i)
{
  assert(sess);
  assert(sess->usage);

  sess->httpVMinor=i;
}






int GWEN_HttpSession_Init(GWEN_HTTP_SESSION *sess)
{
  GWEN_SYNCIO *sioTls;
  GWEN_DB_NODE *db;
  int rv;

  if (!(sess->flags & GWEN_HTTP_SESSION_FLAGS_PASSIVE)) {  /* client mode */
    GWEN_SYNCIO *sio;

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

    /* allow derived classes to modify the given GWEN_SIO */
    rv=GWEN_HttpSession_InitSyncIo(sess, sio);
    if (rv<0 && rv!=GWEN_ERROR_NOT_IMPLEMENTED) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_free(sio);
      return rv;
    }
    sess->syncIo=sio;
  }

  if (sess->syncIo==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No SYNCIO object, SNH!");
    return GWEN_ERROR_INTERNAL;
  }

  /* prepare TLS layer */
  sioTls=GWEN_SyncIo_GetBaseIoByTypeName(sess->syncIo, GWEN_SYNCIO_TLS_TYPE);
  if (sioTls) {
    if (!(sess->flags & GWEN_HTTP_SESSION_FLAGS_PASSIVE)) {  /* client mode */
      GWEN_SyncIo_AddFlags(sioTls,
                           GWEN_SYNCIO_TLS_FLAGS_ALLOW_V1_CA_CRT|
                           GWEN_SYNCIO_TLS_FLAGS_ADD_TRUSTED_CAS);
    }
    else { /* server mode */
    }

    if (sess->flags & GWEN_HTTP_SESSION_FLAGS_TLS_IGN_PREMATURE_CLOSE) {
      /* make TLS layer ignore problem of premature connection termination */
      GWEN_SyncIo_AddFlags(sioTls, GWEN_SYNCIO_TLS_FLAGS_IGN_PREMATURE_CLOSE);
    }
  }


  /* prepare HTTP out header */
  db=GWEN_SyncIo_Http_GetDbHeaderOut(sess->syncIo);
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

  return 0;
}



int GWEN_HttpSession_Fini(GWEN_HTTP_SESSION *sess)
{
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
                                const uint8_t *buf, uint32_t blen)
{
  int rv;

  assert(sess);
  assert(sess->usage);

  /* first connect to server */
  GWEN_Gui_ProgressLog(0,
                       GWEN_LoggerLevel_Debug,
                       I18N("Connecting to server..."));
  rv=GWEN_SyncIo_Connect(sess->syncIo);
  if (rv<0) {
    if (rv==GWEN_ERROR_SSL) {
      DBG_NOTICE(GWEN_LOGDOMAIN,
                 "SSL-Error connecting (%d)", rv);
    }
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
                         GWEN_LoggerLevel_Debug,
                         I18N("Connected."));

    /* set command */
    db=GWEN_SyncIo_Http_GetDbCommandOut(sess->syncIo);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "command",
                         httpCommand);
    if (sess->httpVMajor) {
      char numbuf[32];

      snprintf(numbuf, sizeof(numbuf)-1, "HTTP/%d.%d",
               sess->httpVMajor, sess->httpVMinor);
      numbuf[sizeof(numbuf)-1]=0;
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "protocol",
                           numbuf);
    }
    else
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "protocol",
                           "HTTP/1.0");

    /* set content length */
    db=GWEN_SyncIo_Http_GetDbHeaderOut(sess->syncIo);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "Content-length", blen);

    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Debug,
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
                         GWEN_LoggerLevel_Debug,
                         I18N("Message sent."));
    return 0;
  }
}



int GWEN_HttpSession__RecvPacket(GWEN_HTTP_SESSION *sess, GWEN_BUFFER *buf)
{
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

      if (GWEN_Logger_GetLevel(GWEN_LOGDOMAIN)>=GWEN_LoggerLevel_Info) {
        DBG_INFO(GWEN_LOGDOMAIN, "Detailed Error Log For Packet:");

        if (dbHeaderIn) {
          DBG_INFO(GWEN_LOGDOMAIN, "Recevied this HTTP header:");
          GWEN_DB_Dump(dbHeaderIn, 2);
        }
        else {
          DBG_INFO(GWEN_LOGDOMAIN, "-- No HTTP header recevied --");
        }

        if (GWEN_Buffer_GetUsedBytes(buf)) {
          DBG_INFO(GWEN_LOGDOMAIN, "Recevied this body:");
          GWEN_Text_LogString(GWEN_Buffer_GetStart(buf), GWEN_Buffer_GetUsedBytes(buf),
                              GWEN_LOGDOMAIN, GWEN_LoggerLevel_Info);
        }
        else {
          DBG_INFO(GWEN_LOGDOMAIN, "-- No body recevied --");
        }

      }

      if (rv==301 || rv==303 || rv==305 || rv==307) {
        /* moved */
        if (dbHeaderIn) {
          const char *s;

          s=GWEN_DB_GetCharValue(dbHeaderIn, "Location", 0, 0);
          if (s) {
            switch (rv) {
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



int GWEN_HttpSession_RecvPacket(GWEN_HTTP_SESSION *sess, GWEN_BUFFER *buf)
{
  int rv;
  uint32_t pos;

  /* read response */
  pos=GWEN_Buffer_GetPos(buf);
  for (;;) {
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Debug,
                         I18N("Receiving response..."));
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
                         GWEN_LoggerLevel_Debug,
                         I18N("Received continuation response."));
    GWEN_Buffer_Crop(buf, 0, pos);
  }

  GWEN_Gui_ProgressLog(0,
                       GWEN_LoggerLevel_Debug,
                       I18N("Response received."));

  /* disconnect */
  GWEN_Gui_ProgressLog(0,
                       GWEN_LoggerLevel_Debug,
                       I18N("Disconnecting from server..."));
  GWEN_SyncIo_Disconnect(sess->syncIo);
  GWEN_Gui_ProgressLog(0,
                       GWEN_LoggerLevel_Debug,
                       I18N("Disconnected."));
  return rv;
}



int GWEN_HttpSession__RecvPacketToSio(GWEN_HTTP_SESSION *sess, GWEN_SYNCIO *sio)
{
  int rv;

  assert(sess);
  assert(sess->usage);

  rv=GWEN_SyncIo_Http_RecvBodyToSio(sess->syncIo, sio);
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
            switch (rv) {
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



int GWEN_HttpSession_RecvPacketToFile(GWEN_HTTP_SESSION *sess, const char *fname)
{
  int rv;

  /* read response */
  for (;;) {
    GWEN_SYNCIO *sio;

    sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_CreateAlways);
    GWEN_SyncIo_AddFlags(sio,
                         GWEN_SYNCIO_FILE_FLAGS_READ |
                         GWEN_SYNCIO_FILE_FLAGS_WRITE |
                         GWEN_SYNCIO_FILE_FLAGS_UREAD |
                         GWEN_SYNCIO_FILE_FLAGS_UWRITE |
                         GWEN_SYNCIO_FILE_FLAGS_GREAD |
                         GWEN_SYNCIO_FILE_FLAGS_GWRITE);
    rv=GWEN_SyncIo_Connect(sio);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_free(sio);
      return rv;
    }

    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Debug,
                         I18N("Receiving response..."));
    rv=GWEN_HttpSession__RecvPacketToSio(sess, sio);
    if (rv<0 || rv<200 || rv>299) {
      DBG_INFO(GWEN_LOGDOMAIN,
               "Error receiving packet (%d)", rv);
      GWEN_SyncIo_Disconnect(sio);
      GWEN_SyncIo_free(sio);
      unlink(fname);
      GWEN_SyncIo_Disconnect(sess->syncIo);
      return rv;
    }
    if (rv!=100) {
      int rv2;

      /* flush file and close it */
      rv2=GWEN_SyncIo_Flush(sio);
      if (rv2<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv2);
        GWEN_SyncIo_free(sio);
        return rv2;
      }
      rv2=GWEN_SyncIo_Disconnect(sio);
      if (rv2<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv2);
        GWEN_SyncIo_free(sio);
        return rv2;
      }
      GWEN_SyncIo_free(sio);
      break;
    }
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Debug,
                         I18N("Received continuation response."));
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    unlink(fname);
  }

  GWEN_Gui_ProgressLog(0,
                       GWEN_LoggerLevel_Debug,
                       I18N("Response received."));

  /* disconnect */
  GWEN_Gui_ProgressLog(0,
                       GWEN_LoggerLevel_Debug,
                       I18N("Disconnecting from server..."));
  GWEN_SyncIo_Disconnect(sess->syncIo);
  GWEN_Gui_ProgressLog(0,
                       GWEN_LoggerLevel_Debug,
                       I18N("Disconnected."));
  return rv;
}



int GWEN_HttpSession_ConnectionTest(GWEN_HTTP_SESSION *sess)
{
  int rv;

  assert(sess);
  assert(sess->usage);

  /* connect to server */
  GWEN_Gui_ProgressLog(0,
                       GWEN_LoggerLevel_Notice,
                       I18N("Connecting to server..."));
  rv=GWEN_SyncIo_Connect(sess->syncIo);
  if (rv<0) {
    if (rv==GWEN_ERROR_SSL) {
      DBG_NOTICE(GWEN_LOGDOMAIN, "SSL-Error connecting (%d)", rv);
    }
    DBG_INFO(GWEN_LOGDOMAIN, "Could not connect to server (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Could not connect to server"));
    GWEN_SyncIo_Disconnect(sess->syncIo);
    return rv;
  }
  else {
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Notice,
                         I18N("Connected."));

    GWEN_SyncIo_Disconnect(sess->syncIo);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Notice,
                         I18N("Disconnected."));
    return 0;
  }
}



int GWEN_HttpSession_InitSyncIo(GWEN_HTTP_SESSION *sess, GWEN_SYNCIO *sio)
{
  if (sess->initSyncIoFn)
    return sess->initSyncIoFn(sess, sio);
  DBG_INFO(GWEN_LOGDOMAIN, "initSyncIoFn not set");
  return GWEN_ERROR_NOT_IMPLEMENTED;
}







