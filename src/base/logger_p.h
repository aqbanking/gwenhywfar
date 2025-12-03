/***************************************************************************
    begin       : Sun Dec 05 2003
    copyright   : (C) 2025 by Martin Preuss
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


#ifdef __cplusplus
}
#endif


#endif /* #ifndef GWEN_LOGGER_P_H */


