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


#include "url_p.h"
#include "urlfns.h"
#include <gwenhywfar/debug.h>
#include <ctype.h> // for isalpha()


GWEN_URL *GWEN_Url_fromString(const char *str) {
  GWEN_URL *url;
  GWEN_DB_NODE *dbVars;
  const char *s;
  const char *p;
  int starts_with_drive_letter = 0;

  url=GWEN_Url_new();
  dbVars=GWEN_DB_Group_new("vars");
  GWEN_Url_SetVars(url, dbVars);
  GWEN_DB_Group_free(dbVars);
  dbVars=0;

  s=str;

  /* Check for a drive letter, which occurs on windows, but this will
     always be one single alpha character followed by a colon followed
     by a directory separator. */
  if (s && isalpha(s[0]) && s[1] == ':'
      && (s[2] == '/' || s[2] == '\\')) {
    starts_with_drive_letter = 1;
  }

  /* read protocol (if any) */
  p = starts_with_drive_letter ? s : strchr(s, ':');
  if (p) {
    if (p[1]=='/' && p[2]=='/') {
      char *buf;

      /* got protocol settings */
      buf=(char*)malloc(p-s+1);
      assert(buf);
      memmove(buf, s, p-s+1);
      buf[p-s]=0;
      GWEN_Url_SetProtocol(url, buf);
      free(buf);
      s=p+3;
    }
  }

  if (!*s) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No server or path given in url \"%s\"", str);
    GWEN_Url_free(url);
    return 0;
  }

  /* read user/password */
  p=strchr(s, '@');
  if (p) {
    char *upw;
    char *pw;
    char *pat;

    upw=(char*)malloc(p-s+1);
    assert(upw);
    memmove(upw, s, p-s);
    upw[p-s]=0;
    pw=strchr(upw, ':');
    if (pw) {
      /* there is also a password */
      *pw=0;
      pw++;
    }
    pat=strchr(upw, '%');
    if (pat)
      *pat='@';
    GWEN_Url_SetUserName(url, upw);
    if (pw)
      GWEN_Url_SetPassword(url, pw);
    free(upw);
    s=p+1;
  }

  /* Do we now have a drive letter? (After the protocol?) */
  if (!starts_with_drive_letter
      && s && isalpha(s[0]) && s[1] == ':'
      && (s[2] == '/' || s[2] == '\\')) {
    starts_with_drive_letter = 1;
  }

  /* read server */
  if (!*s) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No server given in url \"%s\"", str);
    GWEN_Url_free(url);
    return 0;
  }
  p=s;
  while(!starts_with_drive_letter && *p && *p!=':' && *p!='/' && *p!='?')
    p++;
  if (p!=s) {
    char *buf;

    /* got server */
    buf=(char*)malloc(p-s+1);
    assert(buf);
    memmove(buf, s, p-s+1);
    buf[p-s]=0;
    GWEN_Url_SetServer(url, buf);
    DBG_DEBUG(GWEN_LOGDOMAIN, "Server: [%s]", buf);
    free(buf);
    s=p;
  }

  /* get port */
  if (*s==':') {
    p=++s;
    while(*p && *p!='?' && *p!='/')
      p++;
    if (p!=s) {
      char *buf;
      int port;

      /* got port */
      buf=(char*)malloc(p-s+1);
      assert(buf);
      memmove(buf, s, p-s+1);
      buf[p-s]=0;
      if (sscanf(buf, "%d", &port)!=1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad port (%s) in url \"%s\"", buf, str);
        free(buf);
	GWEN_Url_free(url);
	return 0;
      }
      url->port=port;
      free(buf);
      s=p;
    }
  }
  else {
    if (url->protocol) {
      if (strcasecmp(url->protocol, "http")==0)
        url->port=80;
      else if (strcasecmp(url->protocol, "https")==0)
        url->port=443;
    }
  }

  /* get path */
  if (starts_with_drive_letter || *s=='/') {
    p=s;
    while(*p && *p!='?')
      p++;
    if (p!=s) {
      char *buf;

      /* got path */
      buf=(char*)malloc(p-s+1);
      assert(buf);
      memmove(buf, s, p-s+1);
      buf[p-s]=0;
      GWEN_Url_SetPath(url, buf);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Path: [%s]", buf);
      free(buf);
      s=p;
    }
  }
  else {
    GWEN_Url_SetPath(url, "/");
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
      GWEN_DB_SetCharValue(GWEN_Url_GetVars(url),
                           GWEN_DB_FLAGS_DEFAULT,
                           GWEN_Buffer_GetStart(bName),
                           GWEN_Buffer_GetStart(bValue));
    GWEN_Buffer_free(bValue);
    GWEN_Buffer_free(bName);
  }

  url->url=strdup(str);
  url->_modified=0;
  return url;
}



int GWEN_Url_toString(const GWEN_URL *url, GWEN_BUFFER *buf) {
  assert(url);
  if (!url->_modified && !(url->url==0 || url->url[0]==0)) {
    GWEN_Buffer_AppendString(buf, url->url);
  }
  else {
    GWEN_DB_NODE *dbV;

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
    if (url->vars) {
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
      } /* while */
    }
  }
  return 0;
}



void GWEN_Url_toUiShortString(const GWEN_URL *url, GWEN_BUFFER *buf) {
  GWEN_DB_NODE *dbV;

  assert(url);

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
}



GWEN_URL *GWEN_Url_fromCommandString(const char *str) {
  GWEN_URL *url;
  GWEN_DB_NODE *dbVars;
  const char *s;
  const char *p;

  url=GWEN_Url_new();
  dbVars=GWEN_DB_Group_new("vars");
  GWEN_Url_SetVars(url, dbVars);
  GWEN_DB_Group_free(dbVars);
  dbVars=0;

  s=str;

  /* get path */
  if (*s=='/') {
    p=s;
    while(*p && *p!='?')
      p++;
    if (p!=s) {
      char *buf;

      /* got path */
      buf=(char*)malloc(p-s+1);
      assert(buf);
      memmove(buf, s, p-s+1);
      buf[p-s]=0;
      GWEN_Url_SetPath(url, buf);
      free(buf);
      s=p;
    }
  }
  else {
    GWEN_Url_SetPath(url, "/");
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
      GWEN_DB_SetCharValue(GWEN_Url_GetVars(url),
                           GWEN_DB_FLAGS_DEFAULT,
                           GWEN_Buffer_GetStart(bName),
                           GWEN_Buffer_GetStart(bValue));
    GWEN_Buffer_free(bValue);
    GWEN_Buffer_free(bName);
  }

  url->url=strdup(str);

  return url;
}



int GWEN_Url_toCommandString(const GWEN_URL *url, GWEN_BUFFER *buf) {
  GWEN_DB_NODE *dbV;

  assert(url);

  if (url->path) {
    GWEN_Buffer_AppendString(buf, url->path);
  }
  if (url->vars) {
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

  return 0;
}



