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
 * @file gwenhyfwar/error.h
 * @short This file contains the module for error handling.
 */

#ifndef GWENHYFWAR_ERROR_P_H
#define GWENHYFWAR_ERROR_P_H

#include <gwenhyfwar/gwenhyfwarapi.h>
#include <gwenhyfwar/error.h>

/**
 * @defgroup mod_error Error module
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

/*
 * Allow this to be used from C and C++
 */
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @struct
 * When registering an error type this form is needed.
 */

struct GWEN_ERRORTYPEREGISTRATIONFORM {
  GWEN_ERRORTYPEREGISTRATIONFORM *next;

  /**
   * Pointer to the function that returns a descriptive error message
   * for a given error code (must be set by the caller).
   */
  GWEN_ERRORMESSAGEPTR msgptr;
  /**
   * If the registration applied this field holds the number assigned
   * to this code. The calling module is then guaranteed to have this
   * code for itself (i.e. this number will not assigned to any other
   * module). So when returning an ERRORCODE specific to the module
   * you can use this number as the "type".
   */
  int typ;

  /**
   * This holds the name of the error type. It should be human readable
   * and quite descriptive (like the "Socket" module will use "Socket" as
   * a type name). This way you can alway resolve the number of a known
   * error type by given its name. This field must be set by the caller.
   * This string must be zero terminated (standard c-string).
   */
  char name[16];
};


/**
 * @name Initialization
 */
/*@{*/
/**
 * Initializes this module.
 */
GWENHYFWAR_API GWEN_ERRORCODE GWEN_Error_ModuleInit();

/**
 * Deinitializes this module.
 */
GWENHYFWAR_API GWEN_ERRORCODE GWEN_Error_ModuleFini();

/*@}*/



#ifdef __cplusplus
}
#endif

/*@} group mod_error */


#endif /* MOD_ERROR_H */


