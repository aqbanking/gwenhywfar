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

/* Internationalization */
#ifdef ENABLE_NLS
# include <libintl.h>
# include <locale.h>
#endif


#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/directory.h>

#include "base/debug.h"
#include "base/logger_l.h"

#include "base/error_l.h"
#include "base/memory_l.h"
#include "base/plugin_l.h"
#include "base/i18n_l.h"

#include "os/inetaddr_l.h"
#include "os/inetsocket_l.h"
#include "os/libloader_l.h"
#include "os/process_l.h"

#include "io/bufferedio_l.h"
#include "parser/dbio_l.h"
#include "crypt/crypt_l.h"
#include "net/net_l.h"
#include "base/waitcallback_l.h"


/* for regkey stuff */
#ifdef OS_WIN32
# define DIRSEP "\\"
# include <windows.h>
#else
# define DIRSEP "/"
#endif


static unsigned int gwen_is_initialized=0;



GWEN_ERRORCODE GWEN_Init() {
  GWEN_ERRORCODE err;

  if (gwen_is_initialized==0) {
    err=GWEN_Logger_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    GWEN_Error_ModuleInit();
    err=GWEN_Memory_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing I18N module");
    err=GWEN_I18N_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing InetAddr module");
    err=GWEN_InetAddr_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing Socket module");
    err=GWEN_Socket_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing Libloader module");
    err=GWEN_LibLoader_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing BufferedIO module");
    err=GWEN_BufferedIO_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing Crypt module");
    err=GWEN_Crypt_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing Process module");
    err=GWEN_Process_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing Network module");
    err=GWEN_Net_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing Plugin module");
    err=GWEN_Plugin_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing DataBase IO module");
    err=GWEN_DBIO_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing WaitCallback module");
    err=GWEN_WaitCallback_ModuleInit();
    if (!GWEN_Error_IsOk(err))
      return err;
    /* add more modules here */

  }
  gwen_is_initialized++;

  return 0;

}



GWEN_ERRORCODE GWEN_Fini() {
  GWEN_ERRORCODE err;
  const char *s;

  err=0;

  if (gwen_is_initialized==0)
    return 0;

  gwen_is_initialized--;
  if (gwen_is_initialized==0) {
    /* add more modules here */
    if (!GWEN_Error_IsOk(GWEN_WaitCallback_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module WaitCallback");
    }
    if (!GWEN_Error_IsOk(GWEN_DBIO_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module DBIO");
    }
    if (!GWEN_Error_IsOk(GWEN_Plugin_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module Plugin");
    }
    if (!GWEN_Error_IsOk(GWEN_Net_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module Net");
    }
    if (!GWEN_Error_IsOk(GWEN_Process_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module Process");
    }
    if (!GWEN_Error_IsOk(GWEN_Crypt_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module Crypt");
    }
    if (!GWEN_Error_IsOk(GWEN_BufferedIO_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module BufferedIO");
    }
    if (!GWEN_Error_IsOk(GWEN_LibLoader_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
	      "Could not deinitialze module LibLoader");
    }
    if (!GWEN_Error_IsOk(GWEN_Socket_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module Socket");
    }
    if (!GWEN_Error_IsOk(GWEN_InetAddr_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                       GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module InetAddr");
    }

    if (!GWEN_Error_IsOk(GWEN_Memory_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module Memory");
    }
    if (!GWEN_Error_IsOk(GWEN_I18N_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                         GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module I18N");
    }
    GWEN_Error_ModuleFini();

    s=getenv("GWEN_MEMORY_DEBUG");
    if (s) {
      int i;

      if (1==sscanf(s, "%i", &i))
        GWEN_MemoryDebug_Dump(i);
    }

    /* must be deinitialized at last */
    if (!GWEN_Error_IsOk(GWEN_Logger_ModuleFini())) {
      err=GWEN_Error_new(0,
                         GWEN_ERROR_SEVERITY_ERR,
                         0,
                       GWEN_ERROR_COULD_NOT_UNREGISTER);
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module Logger");
    }
  }

  GWEN_MemoryDebug_CleanUp();
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







int GWEN__GetValueFromWinReg(const char *keyPath,
                             const char *varName,
                             GWEN_BUFFER *nbuf){
#ifdef OS_WIN32
  HKEY hkey;
  TCHAR nbuffer[MAX_PATH];
  BYTE vbuffer[MAX_PATH];
  DWORD nsize;
  DWORD vsize;
  DWORD typ;
  int i;

  snprintf(nbuffer, sizeof(nbuffer), keyPath);

  /* open the key */
  if (RegOpenKey(HKEY_CURRENT_USER,
                 keyPath,
                 &hkey)){
    DBG_ERROR(GWEN_LOGDOMAIN,
              "RegOpenKey \"%s\" failed.", keyPath);
    return -1;
  }

  /* find the variablename  */
  for (i=0;; i++) {
    nsize=sizeof(nbuffer);
    vsize=sizeof(vbuffer);
    if (ERROR_SUCCESS!=RegEnumValue(hkey,
                                    i,    /* index */
                                    nbuffer,
                                    &nsize,
                                    0,       /* reserved */
                                    &typ,
                                    vbuffer,
                                    &vsize))
      break;
    if (strcasecmp(nbuffer, varName)==0 && typ==REG_SZ) {
      /* variable found */
      RegCloseKey(hkey);
      GWEN_Buffer_AppendString(nbuf, (char*)vbuffer);
      return 0;
    }
  } /* for */

  RegCloseKey(hkey);
  return -1;

#else /* OS_WIN32 */
  return -1;
#endif /* OS_WIN32 */
}



int GWEN_GetInstallPath(GWEN_BUFFER *pbuf) {
  if (GWEN__GetValueFromWinReg("Software\\Gwenhywfar\\Paths",
                               "prefix", pbuf)!=0) {
    GWEN_Directory_OsifyPath(GWEN_INSTALL_DIR, pbuf, 1);
  }
  return 0;
}



int GWEN_GetPluginPath(GWEN_BUFFER *pbuf) {
  int rv;

  rv=GWEN_GetInstallPath(pbuf);
  if (rv)
    return rv;

  GWEN_Buffer_AppendString(pbuf, DIRSEP);
  GWEN_Buffer_AppendString(pbuf, "lib");
  GWEN_Buffer_AppendString(pbuf, DIRSEP);
  GWEN_Buffer_AppendString(pbuf, "gwenhywfar");
  GWEN_Buffer_AppendString(pbuf, DIRSEP);
  GWEN_Buffer_AppendString(pbuf, "plugins");
  GWEN_Buffer_AppendString(pbuf, DIRSEP);
  GWEN_Buffer_AppendString(pbuf, GWEN_EFFECTIVE_VERSTR);

  return 0;
}






