/***************************************************************************
  $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri Nov 22 2002
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


#ifndef GWENHYFWAR_LIBLOADER_H
#define GWENHYFWAR_LIBLOADER_H "$Id"

#define GWEN_LIBLOADER_ERROR_TYPE "LIBLOADER"
#define GWEN_LIBLOADER_ERROR_COULD_NOT_LOAD  1
#define GWEN_LIBLOADER_ERROR_NOT_OPEN        2
#define GWEN_LIBLOADER_ERROR_COULD_NOT_CLOSE 3
#define GWEN_LIBLOADER_ERROR_COULD_RESOLVE   4

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/error.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup MOD_LIBLOADER Library Loading Funtions
 * @ingroup MOD_OS
 * @short This module allows loading of shared libraries
 *
 * This module can be used to load libraries and to lookup symbols inside
 * them.
 * @author Martin Preuss<martin@libchipcard.de>
 */
/*@{*/


GWENHYFWAR_API typedef struct GWEN_LIBLOADERSTRUCT GWEN_LIBLOADER;


GWENHYFWAR_API GWEN_LIBLOADER *GWEN_LibLoader_new();
GWENHYFWAR_API void GWEN_LibLoader_free(GWEN_LIBLOADER *h);

GWENHYFWAR_API
  GWEN_ERRORCODE GWEN_LibLoader_OpenLibrary(GWEN_LIBLOADER *h,
                                            const char *name);
GWENHYFWAR_API
  GWEN_ERRORCODE GWEN_LibLoader_CloseLibrary(GWEN_LIBLOADER *h);
GWENHYFWAR_API
  GWEN_ERRORCODE GWEN_LibLoader_Resolve(GWEN_LIBLOADER *h,
                                        const char *name, void **p);

/*@}*/

#ifdef __cplusplus
}
#endif


#endif /* GWENHYFWAR_LIBLOADER_H */


