/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Nov 28 2002
 copyright   : (C) 2002 by Martin Preuss
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


#ifndef GWEN_DEBUG_H
#define GWEN_DEBUG_H

#include <stdio.h>
#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/logger.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (defined HAVE_FUNC && (DEBUGMODE>10))
# define DBG_ENTER fprintf(stderr,"Enter \""__func__"\" \n")
# define DBG_LEAVE fprintf(stderr,"Leave \""__func__"\" \n")
#else
# define DBG_ENTER
# define DBG_LEAVE
#endif

#ifndef HAVE_SNPRINTF
# error Not using snprintf! Maybe "config.h" is not included?
#endif

#define DBG_ERROR(dbg_logger, format, args...) if (1){\
  char dbg_buffer[256]; \
  snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
  __FILE__":%5d: " format  , __LINE__ , ## args); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelError, dbg_buffer);};

#define DBG_ERROR_ERR(dbg_logger, dbg_err) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 GWEN_Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelError, dbg_buffer);};


#define DBG_WARN(dbg_logger, format, args...) {\
  char dbg_buffer[256]; \
  snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
  __FILE__":%5d: " format  , __LINE__ , ## args); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelWarning, dbg_buffer);};

#define DBG_WARN_ERR(dbg_logger, dbg_err) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 GWEN_Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelWarning, dbg_buffer);};


#define DBG_NOTICE(dbg_logger, format, args...) \
 if (GWEN_Logger_GetLevel(dbg_logger)>=GWEN_LoggerLevelNotice) {\
 char dbg_buffer[256]; \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: " format  , __LINE__ , ## args); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelNotice, dbg_buffer);};

#define DBG_NOTICE_ERR(dbg_logger, dbg_err) \
 if (GWEN_Logger_GetLevel(dbg_logger)>=GWEN_LoggerLevelNotice) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 GWEN_Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelNotice, dbg_buffer);};


#define DBG_INFO(dbg_logger, format, args...) \
 if (GWEN_Logger_GetLevel(dbg_logger)>=GWEN_LoggerLevelInfo) {\
  char dbg_buffer[256]; \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: " format  , __LINE__ , ## args); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelInfo, dbg_buffer);};

#define DBG_INFO_ERR(dbg_logger, dbg_err) \
 if (GWEN_Logger_GetLevel(dbg_logger)>=GWEN_LoggerLevelInfo) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 GWEN_Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelInfo, dbg_buffer);};




#ifndef DISABLE_DEBUGLOG

#define DBG_DEBUG(dbg_logger, format, args...) \
 if (GWEN_Logger_GetLevel(dbg_logger)>=GWEN_LoggerLevelDebug) {\
 char dbg_buffer[256]; \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: " format  , __LINE__ , ## args); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelDebug, dbg_buffer);};

#define DBG_DEBUG_ERR(dbg_logger, dbg_err) \
 if (GWEN_Logger_GetLevel(dbg_logger)>=GWEN_LoggerLevelDebug) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 GWEN_Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelDebug, dbg_buffer);};


#define DBG_VERBOUS(dbg_logger, format, args...) \
 if (GWEN_Logger_GetLevel(dbg_logger)>=GWEN_LoggerLevelDebug) {\
 char dbg_buffer[256]; \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: " format  , __LINE__ , ## args); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelDebug, dbg_buffer);};

#define DBG_VERBOUS_ERR(dbg_logger, dbg_err) \
 if (GWEN_Logger_GetLevel(dbg_logger)>=GWEN_LoggerLevelDebug) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 GWEN_Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelDebug, dbg_buffer);};



#else

#define DBG_DEBUG(dbg_logger, format, args...)

#define DBG_DEBUG_ERR(dbg_logger, dbg_err)

#define DBG_VERBOUS(dbg_logger, format, args...)

#define DBG_VERBOUS_ERR(dbg_logger, dbg_err)

#endif /* DISABLE_DEBUGLOG */


#ifdef __cplusplus
}
#endif


#endif


