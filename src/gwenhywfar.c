/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Sep 11 2003
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/gwenhywfar.h>

#include "base/error_p.h"
#include "inetaddr_p.h"
#include "inetsocket_p.h"
#include "libloader_p.h"
#include "io/bufferedio_p.h"
#include "gwenhywfar/debug.h"
#include "gwenhywfar/logger.h"
#include "ipc/ipc_p.h"
#include "gwenhywfar/crypt.h"


static GWEN_LOGGER *gwen_default_logger=0;
static unsigned int gwen_is_initialized=0;


GWEN_ERRORCODE GWEN_Init() {
  GWEN_ERRORCODE err;

  if (gwen_is_initialized==0) {
    /* setup default logger */
    gwen_default_logger=GWEN_Logger_new();
    GWEN_Logger_SetDefaultLogger(gwen_default_logger);
    GWEN_Logger_SetLevel(0, GWEN_LoggerLevelNotice);

    DBG_DEBUG(0, "Initializing Error module");
    GWEN_Error_ModuleInit();
    DBG_DEBUG(0, "Initializing InetAddr module");
    err=GWEN_InetAddr_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(0, "Initializing Socket module");
    err=GWEN_Socket_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(0, "Initializing Libloader module");
    err=GWEN_LibLoader_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(0, "Initializing BufferedIO module");
    err=GWEN_BufferedIO_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(0, "Initializing IPC module");
    err=GWEN_IPC_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(0, "Initializing Crypt module");
    err=GWEN_Crypt_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    /* add here more modules */

  }
  gwen_is_initialized++;

  return 0;

}



GWEN_ERRORCODE GWEN_Fini() {
  GWEN_ERRORCODE err;

  err=0;

  if (gwen_is_initialized==0)
    return 0;

  gwen_is_initialized--;
  if (gwen_is_initialized==0) {
    /* add here more modules */
    if (!GWEN_Error_IsOk(GWEN_Crypt_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(0, "GWEN__Fini: "
                "Could not deinitialze module Crypt");
    }
    if (!GWEN_Error_IsOk(GWEN_IPC_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(0, "GWEN__Fini: "
                "Could not deinitialze module IPC");
    }
    if (!GWEN_Error_IsOk(GWEN_BufferedIO_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(0, "GWEN__Fini: "
                "Could not deinitialze module BufferedIO");
    }
    if (!GWEN_Error_IsOk(GWEN_LibLoader_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(0, "GWEN_Fini: "
	      "Could not deinitialze module LibLoader");
    }
    if (!GWEN_Error_IsOk(GWEN_Socket_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(0, "GWEN_Fini: "
                "Could not deinitialze module Socket");
    }
    if (!GWEN_Error_IsOk(GWEN_InetAddr_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                       GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(0, "GWEN_Fini: "
                "Could not deinitialze module InetAddr");
    }
    GWEN_Error_ModuleFini();
    GWEN_Logger_SetDefaultLogger(0);
    GWEN_Logger_free(gwen_default_logger);
  }
  return err;
}



void GWEN_Version(int *major,
                  int *minor,
                  int *patchlevel,
                  int *build){
  *major=GWENHYWFAR_VERSION_MAJOR;
  *minor=GWENHYWFAR_VERSION_MINOR;
  *patchlevel=GWENHYWFAR_VERSION_PATCHLEVEL;
  *build=GWENHYWFAR_VERSION_BUILD;
}





