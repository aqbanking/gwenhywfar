/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Oct 02 2002
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

/**
 * @file gwenhywfar/error.h
 * @short This file contains the module for error handling.
 */

#ifndef GWENHYWFAR_ERROR_H
#define GWENHYWFAR_ERROR_H

#include <gwenhywfar/gwenhywfarapi.h>


/*
 * Allow this to be used from C and C++
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup MOD_ERROR Error module
 * @ingroup MOD_BASE
 * @short This module does all error handling
 * @author Martin Preuss<martin@libchipcard.de>
 *
 * All errors are grouped into error types. There is no predefined error
 * type, all modules, which want to take advantage of this module should
 * register their own error type.
 * When registering an error type, this module learns about the new error
 * type:
 * <ul>
 *  <li>name (like <i>Socket</i>, <i>InetAddress</i>, <i>Time</i> etc.)</li>
 *  <li>function to create human readable error messages for this type</li>
 * </ul>
 */
/*@{*/


/**
 * An error code is 32 bits long and has the following bit format:
 * <table BORDER="1">
 * <tr><td><b>Bit(s)</b></td><td><b>Meaning</b></td></tr>
 * <tr><td>31-30</td> <td>Severity (0-3)</td></tr>
 * <tr><td>29</td>    <td>Custom error</td></tr>
 * <tr><td>28</td>    <td>Reserved</td></tr>
 * <tr><td>27-16</td> <td>Type (specifies the type of the error, such as
 *           "ERROR_CLASS_SOCKET", "ERROR_CLASS_TIME" etc)</td></tr>
 * <tr><td>15-0</td>  <td>Code (16 bit error code)</td></tr>
 * </table>
 */
GWENHYWFAR_API typedef unsigned long GWEN_ERRORCODE;

GWENHYWFAR_API typedef struct GWEN_ERRORTYPEREGISTRATIONFORM
  GWEN_ERRORTYPEREGISTRATIONFORM;

/**
 * A function of this type returns a descriptive string for the given
 * error. So it is supposed to translate a 16 bit error value into a human
 * readable text (please note that this code is PART of a ERRORCODE, not the
 * whole ERRORCODE itself !).
 */
GWENHYWFAR_API typedef const char* (*GWEN_ERRORMESSAGEPTR)(int c);


int GWEN_ErrorType_GetType(GWEN_ERRORTYPEREGISTRATIONFORM *f);


/**
 * The error code is a debug level code.
 */
#define GWEN_ERROR_SEVERITY_DEBUG   0
/**
 * The error code is a information level code.
 */
#define GWEN_ERROR_SEVERITY_INFO    1
/**
 * The error code is a warning level code.
 */
#define GWEN_ERROR_SEVERITY_WARN 2
/**
 * The error code is a error level code.
 */
#define GWEN_ERROR_SEVERITY_ERR   3

/**
 * This class is used for ERROR-related errors ;-)
 */
#define GWEN_ERROR_TYPE_ERROR 0

#define GWEN_ERROR_UNSPECIFIED          1
#define GWEN_ERROR_COULD_NOT_REGISTER   2
#define GWEN_ERROR_COULD_NOT_UNREGISTER 3
#define GWEN_ERROR_INVALID_BUFFERSIZE   4


/**
 * @name Error Type Registration Form
 */
/*@{*/

GWEN_ERRORTYPEREGISTRATIONFORM *GWEN_ErrorType_new();
void GWEN_ErrorType_free(GWEN_ERRORTYPEREGISTRATIONFORM *f);

void GWEN_ErrorType_SetType(GWEN_ERRORTYPEREGISTRATIONFORM *f, int c);
void GWEN_ErrorType_SetName(GWEN_ERRORTYPEREGISTRATIONFORM *f,
                            const char *name);

void GWEN_ErrorType_SetMsgPtr(GWEN_ERRORTYPEREGISTRATIONFORM *f,
                              GWEN_ERRORMESSAGEPTR msgptr);

GWEN_ERRORMESSAGEPTR
  GWEN_ErrorType_GetMsgPtr(GWEN_ERRORTYPEREGISTRATIONFORM *f);
/*@}*/



/**
 * @name Error Type Registration
 * These functions are used to allocate error types. Each module should
 * allocate a type to allow descriptive error messages.
 * Since function pointers are involved you should carefully register and
 * unregister the error types.
 */
/*@{*/

/**
 * Register an error type. This function is called by other modules
 * to register a unique type value for itself.
 * @return 0 on error (success otherwise)
 */
GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_Error_RegisterType(GWEN_ERRORTYPEREGISTRATIONFORM *tptr);

/**
 * When removing a module it should always unregister its assigned error
 * type to avoid segfaultes (due to pointers pointing to nowhere).
 * @return 0 on error (success otherwise)
 */
GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_Error_UnregisterType(GWEN_ERRORTYPEREGISTRATIONFORM *tptr);
/*@}*/

/**
 * @name Error Type Lookup
 * These functions lookup error types by name or by number.
 */
/*@{*/

/**
 * This function returns the type number for the given type name.
 * @return type number (-1 on error)
 */
GWENHYWFAR_API int GWEN_Error_FindType(const char *name);

/**
 * Returns the name of the type referenced by the its type number.
 * This function is used when composing a human readable error string.
 * @return name of the type (0 on error)
 * @param t type number
 */
GWENHYWFAR_API const char *GWEN_Error_GetTypename(int t);
/*@}*/


/**
 * @name Getters And Setters
 * These functions get and set parts of an ERRORCODE.
 */
/*@{*/
/**
 * Creates an error code based on the arguments given
 * @return error code based on the given arguments
 * @param iscustom if !=0, then this is a custom code. Such an error code
 * can be used by applications. No module of this library will produce
 * errors with this set, so you can use this to take advantage of the
 * ERRORCODE management even without registering you own error type.
 * @param severity severity level of this code (might be simply a debug code,
 * a warning, an information or an error)
 * @param typ type of this code. As you read above all errors are grouped
 * into error types.
 * @param code error code. This needs only to be unique within the error
 * type (in fact that was the reason to introduce the "error type")
 */
GWENHYWFAR_API GWEN_ERRORCODE GWEN_Error_new(int iscustom, int severity,
                                             int typ, int code);

/**
 * Checks whether the code really contains an error. If it contains a
 * debug/info/warn code then it will be treaten as "ok".
 * @return !=0 if ok, 0 if it really is an error
 */
GWENHYWFAR_API int GWEN_Error_IsOk(GWEN_ERRORCODE c);

/**
 * Returns the severity of the error
 */
GWENHYWFAR_API int GWEN_Error_GetSeverity(GWEN_ERRORCODE c);

/**
 * Sets the severity level.
 */
GWENHYWFAR_API void GWEN_Error_SetSeverity(GWEN_ERRORCODE *c, int v);

/**
 * Checks whether this is a custom error code (which means that the error
 * type and code has to be handled differently. Such a code might be used
 * in programs).
 * @return !=0 if it is a custom code, 0 otherwise
 */
GWENHYWFAR_API int GWEN_Error_IsCustom(GWEN_ERRORCODE c);

/**
 * Sets the custom flag.
 */
GWENHYWFAR_API void GWEN_Error_SetIsCustom(GWEN_ERRORCODE *c, int iscustom);

/**
 * Returns the error type.
 */
GWENHYWFAR_API int GWEN_Error_GetType(GWEN_ERRORCODE c);

/**
 * Sets the error type.
 */
GWENHYWFAR_API void GWEN_Error_SetType(GWEN_ERRORCODE *c, int v);

/**
 * Returns the 16 bit error code value.
 */
GWENHYWFAR_API int GWEN_Error_GetCode(GWEN_ERRORCODE c);

/**
 * Sets the error code.
 */
GWENHYWFAR_API void GWEN_Error_SetCode(GWEN_ERRORCODE *c, int v);
/*@}*/


/**
 * @name Verbosity
 * Composing error messages.
 */
/*@{*/
/**
 * Composes a human readable error string which dumps all parts of an
 * ERRORCODE.
 * @return 1 detailed error message created. If some parts of the ERRORCODE
 * could not be translated (e.g. unassigned type number etc) a 0 is returned.
 * @param c error code
 * @param buffer pointer to a buffer to receive the message
 * @param bsize size of that buffer in bytes
 */
GWENHYWFAR_API int GWEN_Error_ToString(GWEN_ERRORCODE c,
                                       char *buffer,
                                       int bsize);
/*@}*/

#ifdef __cplusplus
}
#endif

/*@} group mod_error */


#endif /* MOD_ERROR_H */


