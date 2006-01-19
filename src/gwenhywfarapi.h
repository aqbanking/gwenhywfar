/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Wed Sep 02 2002
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

#ifndef GWENHYWFARAPI_H
#define GWENHYWFARAPI_H

#include <gwenhywfar/types.h>


#ifdef BUILDING_GWENHYWFAR
  /* building Gwenhywfar */
# if GWENHYWFAR_SYS_IS_WINDOWS
    /* for windows */
#   ifdef __declspec
#     define GWENHYWFAR_API __declspec (dllexport)
#   else /* if __declspec */
#     define GWENHYWFAR_API
#   endif /* if NOT __declspec */
# else
    /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define GWENHYWFAR_API __attribute__((visibility("default")))
#   else
#     define GWENHYWFAR_API
#   endif
# endif
#else
  /* not building Gwenhywfar */
# if GWENHYWFAR_SYS_IS_WINDOWS
    /* for windows */
#   ifdef __declspec
#     define GWENHYWFAR_API __declspec (dllimport)
#   else /* if __declspec */
#     define GWENHYWFAR_API
#   endif /* if NOT __declspec */
# else
    /* for non-win32 */
#   define GWENHYWFAR_API
# endif
#endif



/* Convenience macros to test the versions of glibc and gcc. Taken
   from <features.h> which does not contain this on MinGW systems.  */
#ifndef __GNUC_PREREQ
# if defined __GNUC__ && defined __GNUC_MINOR__
#  define __GNUC_PREREQ(maj, min) \
        ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else
#  define __GNUC_PREREQ(maj, min) 0
# endif
#endif /* __GNUC_PREREQ */


/* Taken from <sys/cdefs.h> which does not contain this on MinGW
   systems.  */
#ifndef __STRING
# define __STRING(x)     #x
#endif /* __STRING */


/* This is needed for PalmOS, because it define some functions needed */
#include <string.h>
#include <gwenhywfar/system.h>


#ifdef __GNUC__
# define DEPRECATED __attribute__((deprecated))
#else
# define DEPRECATED
#endif /* __GNUC__ */


#ifndef GWEN_NO_COMPAT

#define GWEN_ArgsTypeChar GWEN_ArgsType_Char
#define GWEN_ArgsTypeInt GWEN_ArgsType_Int

#define GWEN_ArgsOutTypeTXT GWEN_ArgsOutType_Txt
#define GWEN_ArgsOutTypeHTML GWEN_ArgsOutType_Html

#define GWEN_StringList2_IntertModeAlwaysAdd GWEN_StringList2_IntertMode_AlwaysAdd
#define GWEN_StringList2_IntertModeNoDouble GWEN_StringList2_IntertMode_NoDouble
#define GWEN_StringList2_IntertModeReuse GWEN_StringList2_IntertMode_Reuse

#define GWEN_LoggerTypeConsole GWEN_LoggerType_Console
#define GWEN_LoggerTypeFile GWEN_LoggerType_File
#define GWEN_LoggerTypeSyslog GWEN_LoggerType_Syslog
#define GWEN_LoggerTypeFunction GWEN_LoggerType_Function
#define GWEN_LoggerTypeUnknown GWEN_LoggerType_Unknown

#define GWEN_LoggerFacilityAuth GWEN_LoggerFacility_Auth
#define GWEN_LoggerFacilityDaemon GWEN_LoggerFacility_Daemon
#define GWEN_LoggerFacilityMail GWEN_LoggerFacility_Mail
#define GWEN_LoggerFacilityNews GWEN_LoggerFacility_News
#define GWEN_LoggerFacilityUser GWEN_LoggerFacility_User
#define GWEN_LoggerFacilityUnknown GWEN_LoggerFacility_Unknown

#define GWEN_LoggerLevelEmergency GWEN_LoggerLevel_Emergency
#define GWEN_LoggerLevelAlert GWEN_LoggerLevel_Alert
#define GWEN_LoggerLevelCritical GWEN_LoggerLevel_Critical
#define GWEN_LoggerLevelError GWEN_LoggerLevel_Error
#define GWEN_LoggerLevelWarning GWEN_LoggerLevel_Warning
#define GWEN_LoggerLevelNotice GWEN_LoggerLevel_Notice
#define GWEN_LoggerLevelInfo GWEN_LoggerLevel_Info
#define GWEN_LoggerLevelDebug GWEN_LoggerLevel_Debug
#define GWEN_LoggerLevelVerbous GWEN_LoggerLevel_Verbous
#define GWEN_LoggerLevelUnknown GWEN_LoggerLevel_Unknown

#endif


#endif


