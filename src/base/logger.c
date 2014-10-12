/***************************************************************************
    begin       : Sun Dec 05 2003
    copyright   : (C) 2003 by Martin Preuss
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

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>

#include "logger_p.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#ifdef HAVE_TIME_H
# include <time.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif


static GWEN_LOGGER_DOMAIN *gwen_loggerdomains=0;



int GWEN_Logger_ModuleInit(void) {
  const char *s;
  GWEN_LOGGER_LEVEL ll=GWEN_LoggerLevel_Warning;

  GWEN_Logger_Open(GWEN_LOGDOMAIN,
                   "gwen",
                   0,
                   GWEN_LoggerType_Console,
                   GWEN_LoggerFacility_User);
  s=getenv("GWEN_LOGLEVEL");
  if (s) {
    ll=GWEN_Logger_Name2Level(s);
    if (ll==GWEN_LoggerLevel_Unknown)
      ll=GWEN_LoggerLevel_Warning;
  }
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, ll);
  return 0;
}



int GWEN_Logger_ModuleFini(void) {
  GWEN_LOGGER_DOMAIN *ld;

  while((ld=gwen_loggerdomains)) {
    GWEN_LoggerDomain_Del(ld);
    GWEN_LoggerDomain_free(ld);
  }
  return 0;
}



GWEN_LOGGER_DOMAIN *GWEN_LoggerDomain_new(const char *name) {
  GWEN_LOGGER_DOMAIN *ld;

  assert(name);
  GWEN_NEW_OBJECT(GWEN_LOGGER_DOMAIN, ld);
  ld->name=strdup(name);
  return ld;
}



void GWEN_LoggerDomain_free(GWEN_LOGGER_DOMAIN *ld) {
  if (ld) {
    free(ld->name);
    GWEN_Logger_free(ld->logger);
    GWEN_FREE_OBJECT(ld);
  }
}


GWEN_LOGGER_DOMAIN *GWEN_LoggerDomain_Find(const char *name) {
  GWEN_LOGGER_DOMAIN *ld;

  assert(name);
  ld=gwen_loggerdomains;
  while(ld) {
    if (strcasecmp(ld->name, name)==0)
      break;
    ld=ld->next;
  }

  return ld;
}



void GWEN_LoggerDomain_Add(GWEN_LOGGER_DOMAIN *ld) {
  assert(ld);
  GWEN_LIST_INSERT(GWEN_LOGGER_DOMAIN, ld, &gwen_loggerdomains);
}



void GWEN_LoggerDomain_Del(GWEN_LOGGER_DOMAIN *ld) {
  assert(ld);
  GWEN_LIST_DEL(GWEN_LOGGER_DOMAIN, ld, &gwen_loggerdomains);
}



GWEN_LOGGER *GWEN_LoggerDomain_GetLogger(const char *name) {
  GWEN_LOGGER_DOMAIN *ld;

  if (!name)
    name="default";

  ld=GWEN_LoggerDomain_Find(name);
  if (ld) {
    return ld->logger;
  }
  ld=GWEN_LoggerDomain_new(name);
  ld->logger=GWEN_Logger_new(ld);

  GWEN_LoggerDomain_Add(ld);
  return ld->logger;
}



GWEN_LOGGER *GWEN_Logger_new(GWEN_LOGGER_DOMAIN *domain) {
  GWEN_LOGGER *lg;

  GWEN_NEW_OBJECT(GWEN_LOGGER, lg);
  lg->usage=1;
  lg->enabled=1;
  lg->logType=GWEN_LoggerType_Console;
  lg->logLevel=GWEN_LoggerLevel_Error;
  lg->domain=domain;
  return lg;
}



void GWEN_Logger_free(GWEN_LOGGER *lg) {
  if (lg) {
    assert(lg->usage);
    if (--(lg->usage)==0) {
      free(lg->logFile);
      free(lg->logIdent);
      GWEN_FREE_OBJECT(lg);
    }
  }
}



void GWEN_Logger_Attach(GWEN_LOGGER *lg) {
  assert(lg);
  lg->usage++;
}



void GWEN_Logger_AddLogger(GWEN_LOGGER *oldLogger, GWEN_LOGGER *newLogger) {
  assert(newLogger);

  assert(oldLogger);
  GWEN_LIST_ADD(GWEN_LOGGER, newLogger, &(oldLogger->next));
}



void GWEN_Logger_SetDefaultLogger(GWEN_UNUSED GWEN_LOGGER *lg) {
  fprintf(stderr, "GWEN_Logger_SetDefaultLogger: Deprecated function\n");
}



int GWEN_Logger_Open(const char *logDomain,
                     const char *ident,
                     const char *file,
                     GWEN_LOGGER_LOGTYPE logtype,
                     GWEN_LOGGER_FACILITY facility) {
  GWEN_LOGGER *lg;

  lg=GWEN_LoggerDomain_GetLogger(logDomain);
  assert(lg);
  lg->logType=logtype;

  GWEN_Logger_SetIdent(logDomain, ident);
  GWEN_Logger_SetFilename(logDomain, file);

  if (logtype==GWEN_LoggerType_File) {
    /* logging to a file */
    if (file==0) {
      lg->logType=GWEN_LoggerType_Console;
      lg->enabled=1;
      fprintf(stderr,"LOGGER: No filename given, will log to console.\n");
    }
    else {
      lg->logType=GWEN_LoggerType_File;
      lg->enabled=1;
    }
  }
#ifdef HAVE_SYSLOG_H
  else if (logtype==GWEN_LoggerType_Syslog) {
    /* caller wants to log via syslog */
    int fac;

    switch(facility) {
    case GWEN_LoggerFacility_Auth:
      fac=LOG_AUTH;
      break;
    case GWEN_LoggerFacility_Daemon:
      fac=LOG_DAEMON;
      break;
    case GWEN_LoggerFacility_Mail:
      fac=LOG_MAIL;
      break;
    case GWEN_LoggerFacility_News:
      fac=LOG_NEWS;
      break;
    case GWEN_LoggerFacility_User:
    case GWEN_LoggerFacility_Unknown:
    default:
      fac=LOG_USER;
      break;
    }

    openlog(ident,
            LOG_CONS |
            LOG_PID,
            fac);
    lg->enabled=1;
  } /* if syslog */
#endif /* ifdef HAVE_SYSLOG_H */

  else {
    /* console or function */
    lg->enabled=1;
  }

  lg->open=1;

  return GWEN_Logger_Log(logDomain, GWEN_LoggerLevel_Debug, "started");
}



void GWEN_Logger_Close(const char *logDomain) {
  GWEN_LOGGER *lg;

  lg=GWEN_LoggerDomain_GetLogger(logDomain);
  assert(lg);
  GWEN_Logger_Log(logDomain, GWEN_LoggerLevel_Debug, "stopped");
  lg->logType=GWEN_LoggerType_Console;
  lg->enabled=0;
#ifdef HAVE_SYSLOG_H
  closelog();
#endif
  lg->open=0;
  /* remove logdomain after it has been closed */
  GWEN_LoggerDomain_Del(lg->domain);
  GWEN_LoggerDomain_free(lg->domain);
}



int GWEN_Logger_IsOpen(const char *logDomain) {
  GWEN_LOGGER_DOMAIN *ld;

  if (!logDomain)
    logDomain="default";
  ld=GWEN_LoggerDomain_Find(logDomain);
  if (ld)
    return ld->logger->open;
  return 0;
}


int GWEN_Logger__CreateMessage(GWEN_LOGGER *lg,
                               GWEN_LOGGER_LEVEL priority, const char *s,
                               GWEN_BUFFER *mbuf) {
#ifdef HAVE_SNPRINTF
  unsigned int i;
#endif /* HAVE_SNPRINTF */
#ifdef HAVE_TIME_H
  struct tm *t;
  time_t tt;
#endif /* HAVE_TIME_H */
  char buffer[256];

  assert(lg);
  if (lg->logIdent) {
    if (strlen(lg->logIdent)+32>=sizeof(buffer)) {
      fprintf(stderr," LOGGER: Logbuffer too small (1).\n");
      return 1;
    }
  }

#ifdef HAVE_TIME_H
  tt=time(0);
  t=localtime(&tt);

# ifdef HAVE_SNPRINTF
#  ifdef HAVE_GETPID
  i=snprintf(buffer, sizeof(buffer)-1,
             "%d:%04d/%02d/%02d %02d-%02d-%02d:%s(%d):",priority,
             t->tm_year+1900, t->tm_mon+1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec,
             lg->logIdent, (int)getpid());
#  else
  i=snprintf(buffer, sizeof(buffer)-1,
             "%d:%04d/%02d/%02d %02d-%02d-%02d:%s:",priority,
             t->tm_year+1900, t->tm_mon+1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec,
             lg->logIdent);
#  endif /* HAVE_GETPID */
  if (i>=sizeof(buffer)) {
    fprintf(stderr," LOGGER: Logbuffer too small (2).\n");
    return 1;
  }
# else   /* HAVE_SNPRINTF */
#  ifdef HAVE_GETPID
  sprintf(buffer,"%d:%04d/%02d/%02d %02d-%02d-%02d:%s(%d):",priority,
          t->tm_year+1900, t->tm_mon+1, t->tm_mday,
          t->tm_hour, t->tm_min, t->tm_sec,
          lg->logIdent, (int)getpid());
#  else
  sprintf(buffer,"%d:%04d/%02d/%02d %02d-%02d-%02d:%s:",priority,
          t->tm_year+1900, t->tm_mon+1, t->tm_mday,
          t->tm_hour, t->tm_min, t->tm_sec,
          lg->logIdent);
#  endif /* HAVE_GETPID */
# endif  /* HAVE_SNPRINTF */
#else    /* HAVE_TIME_H */
# ifdef HAVE_SNPRINTF
  buffer[sizeof(buffer)-1]=0;
  i=snprintf(buffer, sizeof(buffer)-1,
             "%d:%s:",priority,
             lg->logIdent);
  if (i>=sizeof(buffer)) {
    fprintf(stderr," LOGGER: Logbuffer too small (3).\n");
    return 1;
  }
# else   /* HAVE_SNPRINTF */
  sprintf(buffer,"%d:%s:",priority,
          lg->logIdent);
# endif  /* HAVE_SNPRINTF */
#endif   /* HAVE_TIME_H */
  GWEN_Buffer_AppendString(mbuf, buffer);
  GWEN_Buffer_AppendString(mbuf, s);
  GWEN_Buffer_AppendByte(mbuf, '\n');
  return 0;
}



int GWEN_Logger_CreateLogMessage(const char *logDomain,
                                 GWEN_LOGGER_LEVEL priority, const char *s,
                                 GWEN_BUFFER *mbuf) {
  GWEN_LOGGER *lg;

  lg=GWEN_LoggerDomain_GetLogger(logDomain);
  assert(lg);

  return GWEN_Logger__CreateMessage(lg, priority, s, mbuf);
}



int GWEN_Logger__Log(GWEN_LOGGER *lg,
                     GWEN_LOGGER_LEVEL priority, const char *s) {
  while(lg) {
    FILE *f;
#ifdef HAVE_SYSLOG_H
    int pri;
#endif /* HAVE_SYSLOG_H */
    GWEN_BUFFER *mbuf;
    int rv;

    assert(lg);
    if (priority>lg->logLevel)
      /* priority too low, don't log */
      return 0;

    mbuf=GWEN_Buffer_new(0, 256, 0, 1);
    switch(lg->logType) {
    case GWEN_LoggerType_File:
      rv=GWEN_Logger__CreateMessage(lg, priority, s, mbuf);
      if (rv) {
        GWEN_Buffer_free(mbuf);
        return rv;
      }

      f=fopen(lg->logFile,"a+");
      if (f==0) {
        fprintf(stderr,
                "LOGGER: Unable to open file \"%s\" (%s)\n",
                lg->logFile,
                strerror(errno));
        lg->logType=GWEN_LoggerType_Console;
        GWEN_Buffer_free(mbuf);
        return 1;
      }

      if (fwrite(GWEN_Buffer_GetStart(mbuf),
                 GWEN_Buffer_GetUsedBytes(mbuf), 1, f)!=1) {
        fprintf(stderr,
                "LOGGER: Unable to write to file \"%s\" (%s)\n",
                lg->logFile,
                strerror(errno));
        fclose(f);
        lg->logType=GWEN_LoggerType_Console;
        GWEN_Buffer_free(mbuf);
        return 1;
      }
      if (fclose(f)) {
        fprintf(stderr,
                "LOGGER: Unable to close file \"%s\" (%s)\n",
                lg->logFile,
                strerror(errno));
        lg->logType=GWEN_LoggerType_Console;
        GWEN_Buffer_free(mbuf);
        return 1;
      }
      break;

#ifdef HAVE_SYSLOG_H
    case GWEN_LoggerType_Syslog:
      switch(priority) {
      case GWEN_LoggerLevel_Emergency:
        pri=LOG_EMERG;
        break;
      case GWEN_LoggerLevel_Alert:
        pri=LOG_ALERT;
        break;
      case GWEN_LoggerLevel_Critical:
        pri=LOG_CRIT;
        break;
      case GWEN_LoggerLevel_Error:
        pri=LOG_ERR;
        break;
      case GWEN_LoggerLevel_Warning:
        pri=LOG_WARNING;
        break;
      case GWEN_LoggerLevel_Notice:
        pri=LOG_NOTICE;
        break;
      case GWEN_LoggerLevel_Info:
        pri=LOG_NOTICE;
        break;

      case GWEN_LoggerLevel_Debug:
      case GWEN_LoggerLevel_Verbous:
      case GWEN_LoggerLevel_Unknown:
      default:
        pri=LOG_DEBUG;
        break;
      } /* switch */
      syslog(pri,"%s",s);
      break;
#endif /* HAVE_SYSLOG_H */

    case GWEN_LoggerType_Function:
      if (lg->logFunction==0) {
        fprintf(stderr,
                "LOGGER: Logtype is \"Function\", but no function is set.\n");
        GWEN_Buffer_free(mbuf);
        return 1;
      }
      rv=GWEN_Logger__CreateMessage(lg, priority, s, mbuf);
      if (rv) {
        GWEN_Buffer_free(mbuf);
        return rv;
      }
      (lg->logFunction)(GWEN_Buffer_GetStart(mbuf));
      break;

    case GWEN_LoggerType_Console:
    case GWEN_LoggerType_Unknown:
    default:
      rv=GWEN_Logger__CreateMessage(lg, priority, s, mbuf);
      if (rv) {
        GWEN_Buffer_free(mbuf);
        return rv;
      }

      fprintf(stderr, "%s", GWEN_Buffer_GetStart(mbuf));
      break;
    } /* switch */
    lg=lg->next;
    GWEN_Buffer_free(mbuf);
  } /* while lg */
  return 0;
}



int GWEN_Logger_Log(const char *logDomain,
                    GWEN_LOGGER_LEVEL priority, const char *s) {
  if (!GWEN_Gui_LogHook(logDomain, priority, s)) {
    const char *p;
    int rv;
    unsigned int i;
    GWEN_BUFFER *mbuf;
    GWEN_LOGGER *lg;

    lg=GWEN_LoggerDomain_GetLogger(logDomain);
    assert(lg);

    if (!lg->enabled)
      return 1;

    if (priority>lg->logLevel)
      /* priority too low, don't log */
      return 0;

    /* temporarily disable logging to avoid endless loops */
    lg->enabled=0;
    /* copy buffer, exchange all newlines by 0 */
    mbuf=GWEN_Buffer_new(0, strlen(s)+1, 0, 1);
    for (i=0; i<strlen(s)+1; i++) {
      if (s[i]=='\n') {
        GWEN_Buffer_AppendByte(mbuf, 0);
      }
      else
        GWEN_Buffer_AppendByte(mbuf, s[i]);
    }

    /* now log each line */
    rv=0;
    p=GWEN_Buffer_GetStart(mbuf);
    while (*p) {
      rv|=GWEN_Logger__Log(lg, priority, p);
      while(*p)
        p++;
      p++;
    }
    GWEN_Buffer_free(mbuf);
    /* reenable logging */
    lg->enabled=1;
    return rv;
  }
  else
    return 0;
}



void GWEN_Logger_Enable(const char *logDomain, int f) {
  GWEN_LOGGER *lg;

  lg=GWEN_LoggerDomain_GetLogger(logDomain);
  assert(lg);
  lg->enabled=f;
}



int GWEN_Logger_IsEnabled(const char *logDomain) {
  GWEN_LOGGER *lg;

  lg=GWEN_LoggerDomain_GetLogger(logDomain);
  assert(lg);
  return lg->enabled;
}



void GWEN_Logger_SetLevel(const char *logDomain, GWEN_LOGGER_LEVEL l) {
  GWEN_LOGGER *lg;

  lg=GWEN_LoggerDomain_GetLogger(logDomain);
  assert(lg);
  lg->logLevel=l;
}



int GWEN_Logger_GetLevel(const char *logDomain) {
  GWEN_LOGGER *lg;

  lg=GWEN_LoggerDomain_GetLogger(logDomain);
  assert(lg);

  return lg->logLevel;
}



void GWEN_Logger_SetIdent(const char *logDomain, const char *id) {
  GWEN_LOGGER *lg;

  lg=GWEN_LoggerDomain_GetLogger(logDomain);
  assert(lg);

  free(lg->logIdent);
  if (id)
    lg->logIdent=strdup(id);
  else
    lg->logIdent=strdup("No ident, please adjust your program");
}



void GWEN_Logger_SetFilename(const char *logDomain, const char *name) {
  GWEN_LOGGER *lg;

  lg=GWEN_LoggerDomain_GetLogger(logDomain);
  assert(lg);

  free(lg->logFile);
  if (name)
    lg->logFile=strdup(name);
  else
    lg->logFile=strdup("");
}



GWEN_LOGGERFUNCTIONLOG GWEN_Logger_SetLogFunction(const char *logDomain,
    GWEN_LOGGERFUNCTIONLOG fn) {
  GWEN_LOGGER *lg;
  GWEN_LOGGERFUNCTIONLOG oldFn;

  lg=GWEN_LoggerDomain_GetLogger(logDomain);
  assert(lg);
  oldFn=lg->logFunction;
  lg->logFunction=fn;
  return oldFn;
}



GWEN_LOGGER_LEVEL GWEN_Logger_Name2Level(const char *name) {
  if (strcasecmp(name, "emergency")==0)
    return GWEN_LoggerLevel_Emergency;
  else if (strcasecmp(name, "alert")==0)
    return GWEN_LoggerLevel_Alert;
  else if (strcasecmp(name, "critical")==0)
    return GWEN_LoggerLevel_Critical;
  else if (strcasecmp(name, "error")==0)
    return GWEN_LoggerLevel_Error;
  else if (strcasecmp(name, "warning")==0)
    return GWEN_LoggerLevel_Warning;
  else if (strcasecmp(name, "notice")==0)
    return GWEN_LoggerLevel_Notice;
  else if (strcasecmp(name, "info")==0)
    return GWEN_LoggerLevel_Info;
  else if (strcasecmp(name, "debug")==0)
    return GWEN_LoggerLevel_Debug;
  else if (strcasecmp(name, "verbous")==0)
    return GWEN_LoggerLevel_Verbous;
  else {
    return GWEN_LoggerLevel_Unknown;
  }
}



const char *GWEN_Logger_Level2Name(GWEN_LOGGER_LEVEL level) {
  const char *s;

  switch(level) {
  case GWEN_LoggerLevel_Emergency:
    s="emergency";
    break;
  case GWEN_LoggerLevel_Alert:
    s="alert";
    break;
  case GWEN_LoggerLevel_Critical:
    s="critical";
    break;
  case GWEN_LoggerLevel_Error:
    s="error";
    break;
  case GWEN_LoggerLevel_Warning:
    s="warning";
    break;
  case GWEN_LoggerLevel_Notice:
    s="notice";
    break;
  case GWEN_LoggerLevel_Info:
    s="info";
    break;
  case GWEN_LoggerLevel_Debug:
    s="debug";
    break;
  case GWEN_LoggerLevel_Verbous:
    s="verbous";
    break;
  case GWEN_LoggerLevel_Unknown:
  default:
    s="unknown";
    break;
  } /* switch */
  return s;
}



GWEN_LOGGER_LOGTYPE GWEN_Logger_Name2Logtype(const char *name) {
  if (strcasecmp(name, "console")==0)
    return GWEN_LoggerType_Console;
  else if (strcasecmp(name, "file")==0)
    return GWEN_LoggerType_File;
  else if (strcasecmp(name, "syslog")==0)
    return GWEN_LoggerType_Syslog;
  else if (strcasecmp(name, "function")==0)
    return GWEN_LoggerType_Function;
  else {
    return GWEN_LoggerType_Unknown;
  }
}



const char *GWEN_Logger_Logtype2Name(GWEN_LOGGER_LOGTYPE lt) {
  const char *s;

  switch(lt) {
  case GWEN_LoggerType_Console:
    s="console";
    break;
  case GWEN_LoggerType_File:
    s="file";
    break;
  case GWEN_LoggerType_Syslog:
    s="syslog";
    break;
  case GWEN_LoggerType_Function:
    s="function";
    break;
  case GWEN_LoggerType_Unknown:
  default:
    s="unknown";
    break;
  } /* switch */
  return s;
}



int GWEN_Logger_Exists(const char *logDomain) {
  assert(logDomain);
  return (GWEN_LoggerDomain_Find(logDomain)!=0);
}








