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

#ifndef GWEN_LOGGER_H
#define GWEN_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

GWENHYWFAR_API
typedef struct GWEN_LOGGER GWEN_LOGGER;


GWENHYWFAR_API
typedef void (*GWEN_LOGGERFUNCTIONLOG)(const char *s);


GWENHYWFAR_API typedef enum {
  GWEN_LoggerTypeConsole,
  GWEN_LoggerTypeFile,
  GWEN_LoggerTypeSyslog,
  GWEN_LoggerTypeFunction
} GWEN_LOGGER_LOGTYPE;


GWENHYWFAR_API typedef enum {
  GWEN_LoggerFacilityAuth=0,
  GWEN_LoggerFacilityDaemon,
  GWEN_LoggerFacilityMail,
  GWEN_LoggerFacilityNews,
  GWEN_LoggerFacilityUser
} GWEN_LOGGER_FACILITY;


GWENHYWFAR_API typedef enum {
  GWEN_LoggerLevelEmergency=0,
  GWEN_LoggerLevelAlert,
  GWEN_LoggerLevelCritical,
  GWEN_LoggerLevelError,
  GWEN_LoggerLevelWarning,
  GWEN_LoggerLevelNotice,
  GWEN_LoggerLevelInfo,
  GWEN_LoggerLevelDebug
} GWEN_LOGGER_LEVEL;



GWENHYWFAR_API
GWEN_LOGGER *GWEN_Logger_new();

GWENHYWFAR_API
void GWEN_Logger_free(GWEN_LOGGER *lg);



/**
 * Only sets the new default logger if it not already is set or if the new
 * default logger is NULL.
 * You must call GWEN_Logger_Open on that logger prior to calling this
 * function.
 */
GWENHYWFAR_API
void GWEN_Logger_SetDefaultLogger(GWEN_LOGGER *lg);


/**
 * Sets up logging. It automatically enables logging.
 * @author Martin Preuss<martin@libchipcard.de>
 * @param ident this string is prepended to each message logged to identify
 * the logging program
 * @param file name of the file to log to. If this is empty and syslog is
 * available, then all messages are logged via syslog. If syslog is not
 * available, all messages are logged to the console.
 * @param logtype how to log (via syslog, to a file, to the console etc)
 * @param facility what kind of program the log message comes from
 */
GWENHYWFAR_API int GWEN_Logger_Open(GWEN_LOGGER *lg,
                                   const char *ident,
                                   const char *file,
                                   GWEN_LOGGER_LOGTYPE logtype,
                                   GWEN_LOGGER_FACILITY facility);

/**
 * Shuts down logging. Automatically disables logging.
 * @author Martin Preuss<martin@libchipcard.de>
 */
GWENHYWFAR_API void GWEN_Logger_Close(GWEN_LOGGER *lg);

/**
 * Log a message.
 * @author Martin Preuss<martin@libchipcard.de>
 * @param priority priority of the message
 * @param s string to log. This string is cut at all occurences of a newline
 * character thus splitting it into multiple log lines if necessary
 */
GWENHYWFAR_API int GWEN_Logger_Log(GWEN_LOGGER *lg,
                                   GWEN_LOGGER_LEVEL priority, const char *s);

/**
 * Enables or disables logging.
 * @author Martin Preuss<martin@libchipcard.de>
 * @param f if 0 then logging is disabled, otherwise it is enabled
 */
GWENHYWFAR_API void GWEN_Logger_Enable(GWEN_LOGGER *lg,
                                       int f);

/**
 * Checks whether logging is enabled.
 * @author Martin Preuss<martin@libchipcard.de>
 * @return 0 if disabled, 1 otherwise
 */
GWENHYWFAR_API int GWEN_Logger_IsEnabled(GWEN_LOGGER *lg);

/**
 * Sets the logger level. All messages with a priority up to the given one
 * will be logged, all others will not.
 * @author Martin Preuss<martin@libchipcard.de>
 * @param l maximum level to be logged
 */
GWENHYWFAR_API void GWEN_Logger_SetLevel(GWEN_LOGGER *lg,
                                         GWEN_LOGGER_LEVEL l);

/**
 * Returns the current log level.
 * @author Martin Preuss<martin@libchipcard.de>
 */
GWENHYWFAR_API int GWEN_Logger_GetLevel(GWEN_LOGGER *lg);


/**
 * Set ident string. This string is prepended to every log message and
 * should contain the name of the running program.
 * @author Martin Preuss<martin@libchipcard.de>
 */
GWENHYWFAR_API void GWEN_Logger_SetIdent(GWEN_LOGGER *lg,
                                        const char *id);

/**
 * Set the name of the file to be used in when logging to a file.
 */
void GWEN_Logger_SetFilename(GWEN_LOGGER *lg, const char *name);


/**
 * Set logging function. This function is used to log messages in mode
 * LoggerTypeFunction.
 * @author Martin Preuss<martin@libchipcard.de>
 */
GWENHYWFAR_API
  GWEN_LOGGERFUNCTIONLOG GWEN_Logger_SetLogFunction(GWEN_LOGGER *lg,
                                                    GWEN_LOGGERFUNCTIONLOG fn);

#ifdef __cplusplus
}
#endif


#endif /* #ifndef CH_LOGGER_H */


