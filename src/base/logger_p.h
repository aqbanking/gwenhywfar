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

#include <gwenhyfwar/logger.h>

#ifdef __cplusplus
extern "C" {
#endif


struct GWEN_LOGGER {
  int enabled;
  GWEN_LOGGER_LOGTYPE logType;
  char *logFile;
  char *logIdent;
  GWEN_LOGGER_LEVEL logLevel;
  GWEN_LOGGERFUNCTIONLOG logFunction;
};



int _Logger_CreateMessage(GWEN_LOGGER *lg,
                          GWEN_LOGGER_LEVEL priority, const char *s,
			  char *buffer, int bufsize);
int _Logger_Log(GWEN_LOGGER *lg,
                GWEN_LOGGER_LEVEL priority, const char *s);



#ifdef __cplusplus
}
#endif


#endif /* #ifndef GWEN_LOGGER_P_H */


