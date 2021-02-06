/***************************************************************************
 begin       : Fri Feb 07 2003
 copyright   : (C) 2021 by Martin Preuss
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

/* included from gui.c */



GWEN_SYNCIO *GWEN_Gui_ExtendSyncIo(const char *url,
                                   const char *defaultProto,
                                   int defaultPort,
                                   GWEN_SYNCIO *baseSio)
{
  GWEN_GUI *gui;
  GWEN_URL *u=NULL;
  const char *sProtocol=NULL;
  int port=0;
  const char *addr=NULL;

  gui=GWEN_Gui_GetGui();
  assert(gui);

  if (!(url && *url)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Empty URL");
    return NULL;
  }

  if (url && *url) {
    u=GWEN_Url_fromString(url);
    if (u==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid URL [%s]", url);
      return NULL;
    }

    /* determine protocol and port */
    sProtocol=GWEN_Url_GetProtocol(u);
    if (!(sProtocol && *sProtocol))
      sProtocol=defaultProto;
    port=GWEN_Url_GetPort(u);
    if (port<1)
      port=defaultPort;
    addr=GWEN_Url_GetServer(u);
    if (!(addr && *addr)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Missing server in URL [%s]", url);
      GWEN_Url_free(u);
      return NULL;
    }
  }

  if (!(sProtocol && *sProtocol))
    sProtocol="http";
  if (port<1)
    port=80;

  if (strcasecmp(sProtocol, "http")==0 ||
      strcasecmp(sProtocol, "https")==0) {
    GWEN_SYNCIO *sio;
    GWEN_SYNCIO *baseLayer;
    GWEN_DB_NODE *db;
    GWEN_BUFFER *tbuf;
    int rv;

    baseLayer=baseSio;

    if (strcasecmp(sProtocol, "https")==0) {
      /* create TLS layer */
      sio=GWEN_SyncIo_Tls_new(baseLayer);
      if (sio==NULL) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        GWEN_SyncIo_free(baseLayer);
        GWEN_Url_free(u);
        return NULL;
      }
      if (addr)
        GWEN_SyncIo_Tls_SetRemoteHostName(sio, addr);
      baseLayer=sio;
    }

    /* create buffered layer as needed for HTTP */
    sio=GWEN_SyncIo_Buffered_new(baseLayer);
    if (sio==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_SyncIo_free(baseLayer);
      GWEN_Url_free(u);
      return NULL;
    }
    baseLayer=sio;

    /* create HTTP layer */
    sio=GWEN_SyncIo_Http_new(baseLayer);
    if (sio==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_SyncIo_free(baseLayer);
      GWEN_Url_free(u);
      return NULL;
    }

    /* setup default command and header */
    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    db=GWEN_SyncIo_Http_GetDbCommandOut(sio);

    /* get command string (e.g. server-relative path plus variables) */
    rv=GWEN_Url_toCommandString(u, tbuf);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid path in URL, ignoring (%d)", rv);
    }
    else
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "url", GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);

    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "command", "GET");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "protocol", "HTTP/1.0");

    /* preset some headers */
    db=GWEN_SyncIo_Http_GetDbHeaderOut(sio);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Host", addr);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Connection", "close");

    /* done */
    GWEN_Url_free(u);
    return sio;
  }
  else {
    /* just return raw base layer */
    DBG_INFO(GWEN_LOGDOMAIN, "No special handling of protocol \"%s\", returning raw base layer.",
             sProtocol?sProtocol:"(empty)");
    return baseSio;
  }
}



int GWENHYWFAR_CB GWEN_Gui_Internal_GetSyncIo(GWEN_UNUSED GWEN_GUI *gui,
                                              const char *url,
                                              const char *defaultProto,
                                              int defaultPort,
                                              GWEN_SYNCIO **pSio)
{
  GWEN_URL *u;
  const char *s;
  int port;
  const char *addr;
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO *baseLayer;

  if (!(url && *url)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Empty URL");
    return GWEN_ERROR_INVALID;
  }

  u=GWEN_Url_fromString(url);
  if (u==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid URL [%s]", url);
    return GWEN_ERROR_INVALID;
  }

  /* determine protocol and port */
  s=GWEN_Url_GetProtocol(u);
  if (!(s && *s))
    s=defaultProto;
  if (!(s && *s))
    s="http";
  port=GWEN_Url_GetPort(u);
  if (port<1)
    port=defaultPort;
  if (port<1)
    port=80;
  addr=GWEN_Url_GetServer(u);
  if (!(addr && *addr)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing server in URL [%s]", url);
    GWEN_Url_free(u);
    return GWEN_ERROR_INVALID;
  }


  /* create base io */
  sio=GWEN_SyncIo_Socket_new(GWEN_SocketTypeTCP, GWEN_AddressFamilyIP);
  if (sio==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    GWEN_Url_free(u);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_SyncIo_Socket_SetAddress(sio, addr);
  GWEN_SyncIo_Socket_SetPort(sio, port);
  baseLayer=sio;

  sio=GWEN_Gui_ExtendSyncIo(url, defaultProto, defaultPort, baseLayer);
  if (sio==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    GWEN_SyncIo_free(baseLayer);
    GWEN_Url_free(u);
    return GWEN_ERROR_GENERIC;
  }

  /* done */
  GWEN_Url_free(u);
  *pSio=sio;
  return 0;
}




