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


#ifndef GWENHYWFAR_LIBLOADER_P_H
#define GWENHYWFAR_LIBLOADER_P_H "$Id"

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/libloader.h>
#include <gwenhywfar/error.h>


#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API struct GWEN_LIBLOADERSTRUCT {
  void *handle;
};


GWENHYWFAR_API GWEN_ERRORCODE GWEN_LibLoader_ModuleInit();
GWENHYWFAR_API GWEN_ERRORCODE GWEN_LibLoader_ModuleFini();


GWEN_ERRORCODE GWEN_LibLoader_LoadLibrary(GWEN_LIBLOADER *h,
                                          const char *name);


#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_LIBLOADER_P_H */


