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


GWEN_URL *GWEN_Url_fromString(const char *str)
{
  GWEN_URL *url;
  GWEN_DB_NODE *dbVars;
  const char *s;
  const char *p;
  int starts_with_drive_letter = 0;
  int had_userinfo = 0;

  if (!str || !*str) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Empty url");
    return 0;
  }

  url=GWEN_Url_new();
  dbVars=GWEN_DB_Group_new("vars");
  GWEN_Url_SetVars(url, dbVars);
  GWEN_DB_Group_free(dbVars);
  dbVars=0;

  s=str;

  /* Check for a drive letter, which occurs on windows, but this will
     always be one single alpha character followed by a colon followed
     by a directory separator. */
  if (s && isalpha((unsigned char)s[0]) && s[1] == ':'
      && (s[2] == '/' || s[2] == '\\')) {
    starts_with_drive_letter = 1;
  }

  /* read protocol (if any) */
  p = 0;
  if (!starts_with_drive_letter && isalpha((unsigned char)s[0])) {
    const char *q = s + 1;

    while (*q && (isalnum((unsigned char)*q) || *q=='+' || *q=='-' || *q=='.'))
      q++;
    if (q[0]==':' && q[1]=='/' && q[2]=='/')
      p = q;
  }
  if (p) {
    if (p[1]=='/' && p[2]=='/') {
      char *buf;

      /* got protocol settings */
      buf=(char *)malloc(p-s+1);
      if (!buf) {
        GWEN_Url_free(url);
        return 0;
      }
      memmove(buf, s, p-s);
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

  /* read user/password (only inside authority, before path/query/fragment) */
  p=0;
  if (!starts_with_drive_letter) {
    const char *q=s;

    while (*q && *q!='/' && *q!='\\' && *q!='?' && *q!='#') {
      if (*q=='@')
        p=q;
      q++;
    }
  }
  if (p) {
    char *upw;
    char *pw;
    char *pat;

    had_userinfo = 1;

    upw=(char *)malloc(p-s+1);
    if (!upw) {
      GWEN_Url_free(url);
      return 0;
    }
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
      && s && isalpha((unsigned char)s[0]) && s[1] == ':'
      && (s[2] == '/' || s[2] == '\\')) {
    starts_with_drive_letter = 1;
  }

  /* read server */
  if (!*s) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No server given in url \"%s\"", str);
    GWEN_Url_free(url);
    return 0;
  }
  if (!starts_with_drive_letter && *s=='[') {
    int inZone=0;

    p=s+1;
    while (*p && *p!=']') {
      unsigned char ch=(unsigned char)*p;

      if (*p=='%' && !inZone && p>s+1) {
        inZone=1;
        if (!p[1] || p[1]==']')
          break;
      }
      else if (inZone) {
        if (!(isalnum(ch) || ch=='_' || ch=='-' || ch=='.'))
          break;
      }
      else if (!(isxdigit(ch) || ch==':' || ch=='.')) {
        break;
      }
      p++;
    }
    if (*p!=']' || p==s+1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid IPv6 host literal in url \"%s\"", str);
      GWEN_Url_free(url);
      return 0;
    }
    p++;
    if (*p && *p!=':' && *p!='/' && *p!='\\' && *p!='?' && *p!='#') {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid character after IPv6 host literal in url \"%s\"", str);
      GWEN_Url_free(url);
      return 0;
    }
    {
      char *buf;

      buf=(char *)malloc(p-s+1);
      if (!buf) {
        GWEN_Url_free(url);
        return 0;
      }
      memmove(buf, s, p-s);
      buf[p-s]=0;
      GWEN_Url_SetServer(url, buf);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Server: [%s]", buf);
      free(buf);
      s=p;
    }
  }
  else {
    p=s;
    while (!starts_with_drive_letter && *p && *p!=':' && *p!='/' && *p!='\\' && *p!='?' && *p!='#' && *p!='@')
      p++;
    if (p!=s) {
      char *buf;

      /* got server */
      buf=(char *)malloc(p-s+1);
      if (!buf) {
        GWEN_Url_free(url);
        return 0;
      }
      memmove(buf, s, p-s);
      buf[p-s]=0;
      GWEN_Url_SetServer(url, buf);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Server: [%s]", buf);
      free(buf);
      s=p;
    }
  }

  if (!GWEN_Url_GetServer(url) &&
      (had_userinfo ||
       (url->protocol && (strcasecmp(url->protocol, "http")==0 || strcasecmp(url->protocol, "https")==0)))) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No server given in url \"%s\"", str);
    GWEN_Url_free(url);
    return 0;
  }

  /* get port */
  if (*s==':') {
    p=++s;
    while (*p && *p!='?' && *p!='/' && *p!='\\' && *p!='#')
      p++;
    if ((p-s)>5) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad port in url \"%s\"", str);
      GWEN_Url_free(url);
      return 0;
    }
    if (p>s) {
      int port=0;
      const char *q;

      for (q=s; q<p; q++) {
        if (*q<'0' || *q>'9') {
          DBG_ERROR(GWEN_LOGDOMAIN, "Bad port in url \"%s\"", str);
          GWEN_Url_free(url);
          return 0;
        }
        port=port*10+(*q-'0');
      }
      if (port<1 || port>65535) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Port out of range (%d) in url \"%s\"", port, str);
        GWEN_Url_free(url);
        return 0;
      }
      url->port=port;
      s=p;
    }
    else {
      if (url->protocol) {
        if (strcasecmp(url->protocol, "http")==0)
          url->port=80;
        else if (strcasecmp(url->protocol, "https")==0)
          url->port=443;
      }
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
  if (starts_with_drive_letter || *s=='/' || *s=='\\') {
    p=s;
    while (*p && *p!='?' && *p!='#')
      p++;
    if (p!=s) {
      char *buf;

      /* got path */
      buf=(char *)malloc(p-s+1);
      if (!buf) {
        GWEN_Url_free(url);
        return 0;
      }
      memmove(buf, s, p-s);
      buf[p-s]=0;
      GWEN_Url_SetPath(url, buf);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Path: [%s]", buf);
      free(buf);
      s=p;
    }
  }
  else {
    GWEN_Url_SetPath(url, "/");
    if (*s && *s!='?' && *s!='#')
      s++;
  }

  /* read vars */
  while (*s && (*s=='?' || *s=='&')) {
    GWEN_BUFFER *bName;
    GWEN_BUFFER *bValue;

    bName=GWEN_Buffer_new(0, 256, 0, 1);
    bValue=GWEN_Buffer_new(0, 256, 0, 1);
    s++;
    p=s;
    while (*p && *p!='&' && *p!='=' && *p!='#')
      p++;
    if (p!=s)
      GWEN_Buffer_AppendBytes(bName, s, (p-s));
    s=p;
    if (*p=='=') {
      s++;
      p=s;
      while (*p && *p!='&' && *p!='#')
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
  if (!url->url) {
    GWEN_Url_free(url);
    return 0;
  }
  url->_modified=0;
  return url;
}



int GWEN_Url_toString(const GWEN_URL *url, GWEN_BUFFER *buf)
{
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
      while (dbV) {
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



void GWEN_Url_toUiShortString(const GWEN_URL *url, GWEN_BUFFER *buf)
{

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



GWEN_URL *GWEN_Url_fromCommandString(const char *str)
{
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
    while (*p && *p!='?')
      p++;
    if (p!=s) {
      char *buf;

      /* got path */
      buf=(char *)malloc(p-s+1);
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
  while (*s && *s=='?') {
    GWEN_BUFFER *bName;
    GWEN_BUFFER *bValue;

    bName=GWEN_Buffer_new(0, 256, 0, 1);
    bValue=GWEN_Buffer_new(0, 256, 0, 1);
    s++;
    p=s;
    while (*p && *p!='?' && *p!='=')
      p++;
    if (p!=s)
      GWEN_Buffer_AppendBytes(bName, s, (p-s));
    s=p;
    if (*p=='=') {
      s++;
      p=s;
      while (*p && *p!='?')
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



int GWEN_Url_toCommandString(const GWEN_URL *url, GWEN_BUFFER *buf)
{
  GWEN_DB_NODE *dbV;

  assert(url);

  if (url->path) {
    GWEN_Buffer_AppendString(buf, url->path);
  }
  if (url->vars) {
    dbV=GWEN_DB_GetFirstVar(url->vars);
    while (dbV) {
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



