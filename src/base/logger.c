/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
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

#include "logger_p.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif
#include <string.h>
#ifdef HAVE_TIME_H
# include <time.h>
#endif
#include <unistd.h>


static GWEN_LOGGER *gwen_logger=0;


GWEN_LOGGER *GWEN_Logger_new(){
  GWEN_LOGGER *lg;

  GWEN_NEW_OBJECT(GWEN_LOGGER, lg);
  lg->enabled=1;
  lg->logType=GWEN_LoggerTypeConsole;
  lg->logLevel=GWEN_LoggerLevelError;
  return lg;
}



void GWEN_Logger_free(GWEN_LOGGER *lg){
  if (lg) {
    free(lg->logFile);
    free(lg->logIdent);
    free(lg);
  }
}



void GWEN_Logger_AddLogger(GWEN_LOGGER *oldLogger, GWEN_LOGGER *newLogger){
  assert(newLogger);

  if (oldLogger==0)
    oldLogger=gwen_logger;

  if (!oldLogger) {
    gwen_logger=newLogger;
  }
  else {
    GWEN_LIST_ADD(GWEN_LOGGER, newLogger, &(oldLogger->next));
  }
}



void GWEN_Logger_SetDefaultLogger(GWEN_LOGGER *lg){
  if (gwen_logger==0 || lg==0)
    gwen_logger=lg;
}



int GWEN_Logger_Open(GWEN_LOGGER *lg,
                     const char *ident,
                     const char *file,
                     GWEN_LOGGER_LOGTYPE logtype,
                     GWEN_LOGGER_FACILITY facility){
  if (lg==0)
    lg=gwen_logger;
  assert(lg);
  lg->logType=logtype;

  GWEN_Logger_SetIdent(lg, ident);
  GWEN_Logger_SetFilename(lg, file);

  if (logtype==GWEN_LoggerTypeFile) {
    /* logging to a file */
    if (file==0) {
      lg->logType=GWEN_LoggerTypeConsole;
      lg->enabled=1;
      fprintf(stderr,"LOGGER: No filename given, will log to console.\n");
    }
    else {
      lg->logType=GWEN_LoggerTypeFile;
      lg->enabled=1;
    }
  }
#ifdef HAVE_SYSLOG_H
  else if (logtype==GWEN_LoggerTypeSyslog) {
    /* caller wants to log via syslog */
    int fac;

    switch(facility) {
    case GWEN_LoggerFacilityAuth:
      fac=LOG_AUTH;
      break;
    case GWEN_LoggerFacilityDaemon:
      fac=LOG_DAEMON;
      break;
    case GWEN_LoggerFacilityMail:
      fac=LOG_MAIL;
      break;
    case GWEN_LoggerFacilityNews:
      fac=LOG_NEWS;
      break;
    case GWEN_LoggerFacilityUser:
    default:
      fac=LOG_USER;
      break;
    }

    openlog(ident,
	    LOG_CONS |
	    LOG_PID,
	    fac);
    lg->logType=GWEN_LoggerTypeSyslog;
    lg->enabled=1;
  } /* if syslog */
#endif /* ifdef HAVE_SYSLOG_H */

  else {
    /* console */
    lg->logType=GWEN_LoggerTypeConsole;
    lg->enabled=1;
  }

  return GWEN_Logger_Log(lg, GWEN_LoggerLevelDebug, "started");
}



void GWEN_Logger_Close(GWEN_LOGGER *lg){
  if (lg==0)
    lg=gwen_logger;
  assert(lg);
  GWEN_Logger_Log(lg, GWEN_LoggerLevelDebug,"stopped");
  lg->logType=GWEN_LoggerTypeConsole;
  lg->enabled=0;
#ifdef HAVE_SYSLOG_H
  closelog();
#endif
}


int GWEN_Logger__CreateMessage(GWEN_LOGGER *lg,
                               GWEN_LOGGER_LEVEL priority, const char *s,
                               char *buffer, unsigned int bufsize) {
#ifdef HAVE_SNPRINTF
  unsigned int i;
#endif /* HAVE_SNPRINTF */
#ifdef HAVE_TIME_H
  struct tm *t;
  time_t tt;
#endif /* HAVE_TIME_H */

  assert(lg);
  if (lg->logIdent) {
    if (strlen(s)+strlen(lg->logIdent)+32>=bufsize) {
      fprintf(stderr," LOGGER: Logbuffer too small (1).\n");
      return 1;
    }
  }

#ifdef HAVE_TIME_H
  tt=time(0);
  t=localtime(&tt);

# ifdef HAVE_SNPRINTF
  buffer[bufsize-1]=0;
#  ifdef HAVE_GETPID
  i=snprintf(buffer, bufsize-1,
	     "%d:%04d/%02d/%02d %02d-%02d-%02d:%s(%d):%s\n",priority,
	     t->tm_year+1900, t->tm_mon+1, t->tm_mday,
	     t->tm_hour, t->tm_min, t->tm_sec,
	     lg->logIdent, getpid(), s);
#  else
  i=snprintf(buffer, bufsize-1,
	     "%d:%04d/%02d/%02d %02d-%02d-%02d:%s:%s\n",priority,
	     t->tm_year+1900, t->tm_mon+1, t->tm_mday,
	     t->tm_hour, t->tm_min, t->tm_sec,
	     lg->logIdent, s);
#  endif /* HAVE_GETPID */
  if (i>=bufsize) {
    fprintf(stderr," LOGGER: Logbuffer too small (2).\n");
    return 1;
  }
# else   /* HAVE_SNPRINTF */
#  ifdef HAVE_GETPID
  sprintf(buffer,"%d:%04d/%02d/%02d %02d-%02d-%02d:%s(%d):%s\n",priority,
	  t->tm_year+1900, t->tm_mon+1, t->tm_mday,
	  t->tm_hour, t->tm_min, t->tm_sec,
          lg->logIdent, getpid(), s);
#  else
  sprintf(buffer,"%d:%04d/%02d/%02d %02d-%02d-%02d:%s:%s\n",priority,
	  t->tm_year+1900, t->tm_mon+1, t->tm_mday,
	  t->tm_hour, t->tm_min, t->tm_sec,
	  lg->logIdent, s);
#  endif /* HAVE_GETPID */
# endif  /* HAVE_SNPRINTF */
#else    /* HAVE_TIME_H */
# ifdef HAVE_SNPRINTF
  buffer[bufsize-1]=0;
  i=snprintf(buffer, bufsize-1,
	     "%d:%s:%s\n",priority,
	     lg->logIdent, s);
  if (i>=bufsize) {
    fprintf(stderr," LOGGER: Logbuffer too small (3).\n");
    return 1;
  }
# else   /* HAVE_SNPRINTF */
  sprintf(buffer,"%d:%s:%s\n",priority,
	  lg->logIdent, s);
# endif  /* HAVE_SNPRINTF */
#endif   /* HAVE_TIME_H */
  return 0;
}



int GWEN_Logger__Log(GWEN_LOGGER *lg,
                     GWEN_LOGGER_LEVEL priority, const char *s){
  while(lg) {
    FILE *f;
#ifdef HAVE_SYSLOG_H
    int pri;
#endif /* HAVE_SYSLOG_H */
    char buffer[300];
    int rv;

    assert(lg);
    if (priority>lg->logLevel)
      /* priority too low, don't log */
      return 0;

    switch(lg->logType) {
    case GWEN_LoggerTypeFile:
      rv=GWEN_Logger__CreateMessage(lg, priority, s,
                                    buffer, sizeof(buffer));
      if (rv)
        return rv;

      f=fopen(lg->logFile,"a+");
      if (f==0) {
        fprintf(stderr,
                "LOGGER: Unable to open file \"%s\" (%s)\n",
                lg->logFile,
                strerror(errno));
        lg->logType=GWEN_LoggerTypeConsole;
        return 1;
      }

      rv=fprintf(f, "%s", buffer);
      if (rv==-1 || rv!=(int)strlen(buffer)) {
        fprintf(stderr,
                "LOGGER: Unable to write to file \"%s\" (%s)\n",
                lg->logFile,
                strerror(errno));
        fclose(f);
        lg->logType=GWEN_LoggerTypeConsole;
        return 1;
      }
      if (fclose(f)) {
        fprintf(stderr,
                "LOGGER: Unable to close file \"%s\" (%s)\n",
                lg->logFile,
                strerror(errno));
        lg->logType=GWEN_LoggerTypeConsole;
        return 1;
      }
      break;

#ifdef HAVE_SYSLOG_H
    case GWEN_LoggerTypeSyslog:
      switch(priority) {
      case GWEN_LoggerLevelEmergency:
        pri=LOG_EMERG;
        break;
      case GWEN_LoggerLevelAlert:
        pri=LOG_ALERT;
        break;
      case GWEN_LoggerLevelCritical:
        pri=LOG_CRIT;
        break;
      case GWEN_LoggerLevelError:
        pri=LOG_ERR;
        break;
      case GWEN_LoggerLevelWarning:
        pri=LOG_WARNING;
        break;
      case GWEN_LoggerLevelNotice:
        pri=LOG_NOTICE;
        break;
      case GWEN_LoggerLevelInfo:
        pri=LOG_NOTICE;
        break;

      case GWEN_LoggerLevelDebug:
      case GWEN_LoggerLevelVerbous:
      default:
        pri=LOG_DEBUG;
        break;
      } /* switch */
      syslog(pri,"%s",s);
      break;
#endif /* HAVE_SYSLOG_H */

    case GWEN_LoggerTypeFunction:
      if (lg->logFunction==0) {
        fprintf(stderr,
                "LOGGER: Logtype is \"Function\", but no function is set.\n");
        return 1;
      }
      rv=GWEN_Logger__CreateMessage(lg, priority, s,
                                    buffer, sizeof(buffer));
      if (rv)
        return rv;
      (lg->logFunction)(buffer);
      break;

    case GWEN_LoggerTypeConsole:
    default:
      rv=GWEN_Logger__CreateMessage(lg, priority, s,
                                    buffer, sizeof(buffer));
      if (rv)
        return rv;

      fprintf(stderr, "%s", buffer);
      break;
    } /* switch */
    lg=lg->next;
  } /* while lg */
  return 0;
}



int GWEN_Logger_Log(GWEN_LOGGER *lg,
                    GWEN_LOGGER_LEVEL priority, const char *s){
  char buffer[256];
  const char *p;
  int rv;
  unsigned int i;

  if (lg==0)
    lg=gwen_logger;
  assert(lg);

  if (!lg->enabled)
    return 1;

  /* copy buffer, exchange all newlines by 0 */
  for (i=0; i<strlen(s)+1; i++) {
    if (s[i]=='\n') {
      buffer[i]=0;
    }
    else
      buffer[i]=s[i];
  }
  buffer[i]=0; /* add final 0 */

  /* now log each line */
  rv=0;
  p=buffer;
  while (*p) {
    rv|=GWEN_Logger__Log(lg, priority, p);
    while(*p)
      p++;
    p++;
  }
  return rv;
}



void GWEN_Logger_Enable(GWEN_LOGGER *lg, int f){
  if (lg==0)
    lg=gwen_logger;
  assert(lg);
  lg->enabled=f;
}



int GWEN_Logger_IsEnabled(GWEN_LOGGER *lg){
  if (lg==0)
    lg=gwen_logger;
  assert(lg);
  return lg->enabled;
}



void GWEN_Logger_SetLevel(GWEN_LOGGER *lg, GWEN_LOGGER_LEVEL l){
  if (lg==0)
    lg=gwen_logger;
  assert(lg);
  lg->logLevel=l;
}



int GWEN_Logger_GetLevel(GWEN_LOGGER *lg) {
  if (lg==0)
    lg=gwen_logger;
  assert(lg);

  return lg->logLevel;
}



void GWEN_Logger_SetIdent(GWEN_LOGGER *lg, const char *id){
  if (lg==0)
    lg=gwen_logger;
  assert(lg);

  free(lg->logIdent);
  if (id)
    lg->logIdent=strdup(id);
  else
    lg->logIdent=strdup("No ident, please adjust your program");
}



void GWEN_Logger_SetFilename(GWEN_LOGGER *lg, const char *name){
  if (lg==0)
    lg=gwen_logger;
  assert(lg);

  free(lg->logFile);
  if (name)
    lg->logFile=strdup(name);
  else
    lg->logFile=strdup("");
}



GWEN_LOGGER_LEVEL GWEN_Logger_Name2Level(const char *name) {
  if (strcasecmp(name, "emergency")==0)
    return GWEN_LoggerLevelEmergency;
  else if (strcasecmp(name, "alert")==0)
    return GWEN_LoggerLevelAlert;
  else if (strcasecmp(name, "critical")==0)
    return GWEN_LoggerLevelCritical;
  else if (strcasecmp(name, "error")==0)
    return GWEN_LoggerLevelError;
  else if (strcasecmp(name, "warning")==0)
    return GWEN_LoggerLevelWarning;
  else if (strcasecmp(name, "notice")==0)
    return GWEN_LoggerLevelNotice;
  else if (strcasecmp(name, "info")==0)
    return GWEN_LoggerLevelInfo;
  else if (strcasecmp(name, "debug")==0)
    return GWEN_LoggerLevelDebug;
  else if (strcasecmp(name, "verbous")==0)
    return GWEN_LoggerLevelVerbous;
  else {
    return GWEN_LoggerLevelUnknown;
  }
}



const char *GWEN_Logger_Level2Name(GWEN_LOGGER_LEVEL level) {
  const char *s;

  switch(level) {
  case GWEN_LoggerLevelEmergency:
    s="emergency"; break;
  case GWEN_LoggerLevelAlert:
    s="alert"; break;
  case GWEN_LoggerLevelCritical:
    s="critical"; break;
  case GWEN_LoggerLevelError:
    s="error"; break;
  case GWEN_LoggerLevelWarning:
    s="warning"; break;
  case GWEN_LoggerLevelNotice:
    s="notice"; break;
  case GWEN_LoggerLevelInfo:
    s="info"; break;
  case GWEN_LoggerLevelDebug:
    s="debug"; break;
  case GWEN_LoggerLevelVerbous:
    s="verbous"; break;
  case GWEN_LoggerLevelUnknown:
  default:
    s="unknown"; break;
  } /* switch */
  return s;
}



GWEN_LOGGER_LOGTYPE GWEN_Logger_Name2Logtype(const char *name) {
  if (strcasecmp(name, "console")==0)
    return GWEN_LoggerTypeConsole;
  else if (strcasecmp(name, "file")==0)
    return GWEN_LoggerTypeFile;
  else if (strcasecmp(name, "syslog")==0)
    return GWEN_LoggerTypeSyslog;
  else if (strcasecmp(name, "function")==0)
    return GWEN_LoggerTypeFunction;
  else {
    return GWEN_LoggerTypeUnknown;
  }
}



const char *GWEN_Logger_Logtype2Name(GWEN_LOGGER_LOGTYPE lt) {
  const char *s;

  switch(lt) {
  case GWEN_LoggerTypeConsole:
    s="console"; break;
  case GWEN_LoggerTypeFile:
    s="file"; break;
  case GWEN_LoggerTypeSyslog:
    s="syslog"; break;
  case GWEN_LoggerTypeFunction:
    s="function"; break;
  default:
    s="unknown"; break;
  } /* switch */
  return s;
}







