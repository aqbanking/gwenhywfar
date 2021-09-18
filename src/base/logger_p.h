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

#ifndef GWEN_LOGGER_P_H
#define GWEN_LOGGER_P_H

#include "logger_l.h"
#include <gwenhywfar/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_LOGGER_DOMAIN GWEN_LOGGER_DOMAIN;
typedef struct GWEN_LOGGER GWEN_LOGGER;


struct GWEN_LOGGER {
  GWEN_LOGGER *next;
  GWEN_LOGGER_DOMAIN *domain;
  int enabled;
  int open;
  GWEN_LOGGER_LOGTYPE logType;
  char *logFile;
  char *logIdent;
  GWEN_LOGGER_LEVEL logLevel;
  GWEN_LOGGERFUNCTIONLOG logFunction;
  uint32_t usage;
};


struct GWEN_LOGGER_DOMAIN {
  GWEN_LOGGER_DOMAIN *next;
  char *name;
  GWEN_LOGGER *logger;
};
GWEN_LOGGER_DOMAIN *GWEN_LoggerDomain_new(const char *name);
void GWEN_LoggerDomain_free(GWEN_LOGGER_DOMAIN *ld);
GWEN_LOGGER_DOMAIN *GWEN_LoggerDomain_Find(const char *name);
void GWEN_LoggerDomain_Add(GWEN_LOGGER_DOMAIN *ld);
void GWEN_LoggerDomain_Del(GWEN_LOGGER_DOMAIN *ld);
GWEN_LOGGER *GWEN_LoggerDomain_GetLogger(const char *name);


int GWEN_Logger__CreateMessage(GWEN_LOGGER *lg,
                               GWEN_LOGGER_LEVEL priority, const char *s,
                               GWEN_BUFFER *mbuf);

int GWEN_Logger__Log(GWEN_LOGGER *lg,
                     GWEN_LOGGER_LEVEL priority, const char *s);



GWEN_LOGGER *GWEN_Logger_new(GWEN_LOGGER_DOMAIN *domain);

void GWEN_Logger_free(GWEN_LOGGER *lg);

void GWEN_Logger_Attach(GWEN_LOGGER *lg);


/**
 * Adds a logger to the given one. So if the old logger is to log something
 * then the newly added logger will log the same message as well.
 * The new logger must already be open (via @ref GWEN_Logger_Open).
 */
GWENHYWFAR_API
void GWEN_Logger_AddLogger(GWEN_LOGGER *oldLogger, GWEN_LOGGER *newLogger);


#ifndef NO_DEPRECATED_SYMBOLS
/**
 * DEPRECATED. Only sets the new default logger if it not already is
 * set or if the new default logger is NULL.  You must call
 * GWEN_Logger_Open on that logger prior to calling this function.
 */
GWENHYWFAR_API
void GWEN_Logger_SetDefaultLogger(GWEN_LOGGER *lg) GWEN_DEPRECATED ;
#endif // ifndef NO_DEPRECATED_SYMBOLS


#ifdef __cplusplus
}
#endif


#endif /* #ifndef GWEN_LOGGER_P_H */


