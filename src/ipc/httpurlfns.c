/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri May 07 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "httpurl_p.h"
#include "httpurlfns.h"
#include <gwenhywfar/debug.h>


GWEN_HTTP_URL *GWEN_HttpUrl_fromString(const char *str) {
  GWEN_HTTP_URL *url;
  const char *s;
  const char *p;

  url=GWEN_HttpUrl_new();
  s=str;
  /* read protocol (if any) */
  p=strchr(s, ':');
  if (p) {
    char *buf;

    /* got protocol settings */
    buf=(char*)malloc(p-s+1);
    assert(buf);
    memmove(buf, s, p-s+1);
    buf[p-s]=0;
    GWEN_HttpUrl_SetProtocol(url, buf);
    free(buf);
    s=p+1;
  }

  /* skip slashes (if any) */
  if (*s=='/')
    s++;
  if (*s=='/')
    s++;

  /* read server */
  if (!*s) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No server given");
    GWEN_HttpUrl_free(url);
    return 0;
  }
  p=s;
  while(*p && *p!='/' && *p!='?')
    p++;
  if (p!=s) {
    char *buf;

    /* got server */
    buf=(char*)malloc(p-s+1);
    assert(buf);
    memmove(buf, s, p-s+1);
    buf[p-s]=0;
    GWEN_HttpUrl_SetServer(url, buf);
    free(buf);
    s=p;
  }

  if (*s=='/') {
    p=s;
    while(*p && *p!='?')
      p++;
    if (p!=s) {
      char *buf;

      /* got server */
      buf=(char*)malloc(p-s+1);
      assert(buf);
      memmove(buf, s, p-s+1);
      buf[p-s]=0;
      GWEN_HttpUrl_SetPath(url, buf);
      free(buf);
      s=p;
    }
  }
  else {
    GWEN_HttpUrl_SetPath(url, "/");
    if (*s)
      s++;
  }

  /* read vars */
  while(*s && *s=='?') {
    GWEN_BUFFER *bName;
    GWEN_BUFFER *bValue;

    bName=GWEN_Buffer_new(0, 256, 0, 1);
    bValue=GWEN_Buffer_new(0, 256, 0, 1);
    s++;
    p=s;
    while(*p && *p!='?' && *p!='=')
      p++;
    if (p!=s)
      GWEN_Buffer_AppendBytes(bName, s, (p-s));
    s=p;
    if (*p=='=') {
      s++;
      p=s;
      while(*p && *p!='?')
        p++;
      if (p!=s)
        GWEN_Buffer_AppendBytes(bValue, s, (p-s));
      s=p;
    }
    /* store variable/value pair */
    if (GWEN_Buffer_GetUsedBytes(bName))
      GWEN_DB_SetCharValue(GWEN_HttpUrl_GetVars(url),
                           GWEN_DB_FLAGS_DEFAULT,
                           GWEN_Buffer_GetStart(bName),
                           GWEN_Buffer_GetStart(bValue));
    GWEN_Buffer_free(bValue);
    GWEN_Buffer_free(bName);
  }

  url->url=strdup(str);

  return url;
}



const char *GWEN_HttpUrl_toString(const GWEN_HTTP_URL *url) {
  assert(url);
  if (!url->_modified && (url->url==0 || url->url[0]==0)) {
    GWEN_BUFFER *buf;
    GWEN_DB_NODE *dbV;

    buf=GWEN_Buffer_new(0, 64, 0, 1);
    if (url->protocol) {
      GWEN_Buffer_AppendString(buf, url->protocol);
      GWEN_Buffer_AppendString(buf, "://");
    }
    if (url->server) {
      GWEN_Buffer_AppendString(buf, url->server);
    }
    if (url->port) {
      char numbuf[32];

      snprintf(numbuf, sizeof(numbuf), "%d", url->port);
      GWEN_Buffer_AppendString(buf, ":");
      GWEN_Buffer_AppendString(buf, numbuf);
    }
    if (url->path) {
      GWEN_Buffer_AppendString(buf, url->path);
    }
    dbV=GWEN_DB_GetFirstVar(url->vars);
    while(dbV) {
      const char *s;

      s=GWEN_DB_VariableName(dbV);
      if (s) {
        GWEN_DB_NODE *dbVal;

        GWEN_Buffer_AppendString(buf, "?");
        GWEN_Buffer_AppendString(buf, s);
        dbVal=GWEN_DB_GetFirstValue(dbV);
        if (dbVal) {
          s=GWEN_DB_GetCharValueFromNode(dbVal);
          if (s) {
            GWEN_Buffer_AppendString(buf, "=");
            GWEN_Buffer_AppendString(buf, s);
          }
        }
      }
      dbV=GWEN_DB_GetNextVar(dbV);
    }
  }
  return url->url;
}



