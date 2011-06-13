/***************************************************************************
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


#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/pathmanager.h>

#include "base/debug.h"
#include "base/logger_l.h"

#include "base/error_l.h"
#include "base/memory_l.h"
#include "base/pathmanager_l.h"
#include "base/plugin_l.h"
#include "base/i18n_l.h"

#include "os/inetaddr_l.h"
#include "os/inetsocket_l.h"
#include "os/libloader_l.h"
#include "os/process_l.h"

#include "parser/dbio_l.h"
#include "parser/configmgr_l.h"
#include "crypt3/cryptkey_l.h"
#include "crypttoken/ctplugin_l.h"

#include "binreloc.h"

#include <gnutls/gnutls.h>


/* for regkey stuff */
#ifdef OS_WIN32
# define DIRSEP "\\"
# include <windows.h>
#else
# define DIRSEP "/"
#endif


/* Watch out: Make sure these are identical with the identifiers
   in gwenhywfar.iss.in ! */
#define GWEN_REGKEY_PATHS        "Software\\Gwenhywfar\\Paths"
#define GWEN_REGNAME_PREFIX      "prefix"
#define GWEN_REGNAME_LIBDIR      "libdir"
#define GWEN_REGNAME_PLUGINDIR   "plugindir"
#define GWEN_REGNAME_SYSCONFDIR  "sysconfdir"
#define GWEN_REGNAME_LOCALEDIR   "localedir"
#define GWEN_REGNAME_DATADIR     "pkgdatadir"
#define GWEN_REGNAME_SYSDATADIR  "sysdatadir"



static unsigned int gwen_is_initialized=0;
static int gwen_binreloc_initialized=0;

char *GWEN__get_plugindir (const char *default_dir);

int GWEN_Init(void) {
  int err;

  if (gwen_is_initialized==0) {
    err=GWEN_Memory_ModuleInit();
    if (err)
      return err;
    err=GWEN_Logger_ModuleInit();
    if (err)
      return err;

    gnutls_global_init();

    if (gwen_binreloc_initialized==0) {
#ifdef ENABLE_BINRELOC
      BrInitError br_error;

      /* Init binreloc. Note: It is not totally clear whether the correct
       function might still be br_init() instead of br_init_lib(). */
      if (!br_init_lib(&br_error)) {
        DBG_INFO(GWEN_LOGDOMAIN, "Error on br_init: %d\n", br_error);
        gwen_binreloc_initialized=-1;
      }
      else
	gwen_binreloc_initialized=1;
#else
      gwen_binreloc_initialized=1;
#endif
    }

    GWEN_Error_ModuleInit();

    err=GWEN_PathManager_ModuleInit();
    if (err)
      return err;

    /* Define some paths used by gwenhywfar; add the windows
       registry entries first, because on Unix those functions
       simply do nothing and on windows they will ensure that the
       most valid paths (which are those from the registry) are
       first in the path lists. */

    /* ---------------------------------------------------------------------
     * $sysconfdir e.g. "/etc" */
    GWEN_PathManager_DefinePath(GWEN_PM_LIBNAME, GWEN_PM_SYSCONFDIR);
    GWEN_PathManager_AddPathFromWinReg(GWEN_PM_LIBNAME,
				       GWEN_PM_LIBNAME,
				       GWEN_PM_SYSCONFDIR,
				       GWEN_REGKEY_PATHS,
				       GWEN_REGNAME_SYSCONFDIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PathManager_AddRelPath(GWEN_PM_LIBNAME,
				GWEN_PM_LIBNAME,
				GWEN_PM_SYSCONFDIR,
				GWEN_SYSCONF_DIR,
				GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PathManager_AddPath(GWEN_PM_LIBNAME,
			     GWEN_PM_LIBNAME,
			     GWEN_PM_SYSCONFDIR,
			     GWEN_SYSCONF_DIR);
#endif

    /* ---------------------------------------------------------------------
     * $localedir e.g. "/usr/share/locale" */
    GWEN_PathManager_DefinePath(GWEN_PM_LIBNAME, GWEN_PM_LOCALEDIR);
    GWEN_PathManager_AddPathFromWinReg(GWEN_PM_LIBNAME,
				       GWEN_PM_LIBNAME,
				       GWEN_PM_LOCALEDIR,
				       GWEN_REGKEY_PATHS,
				       GWEN_REGNAME_LOCALEDIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PathManager_AddRelPath(GWEN_PM_LIBNAME,
				GWEN_PM_LIBNAME,
				GWEN_PM_LOCALEDIR,
				LOCALEDIR,
				GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PathManager_AddPath(GWEN_PM_LIBNAME,
			     GWEN_PM_LIBNAME,
			     GWEN_PM_LOCALEDIR,
			     LOCALEDIR);
#endif

    /* ---------------------------------------------------------------------
     * $plugindir e.g. "/usr/lib/gwenhywfar/plugins/0" */
    GWEN_PathManager_DefinePath(GWEN_PM_LIBNAME, GWEN_PM_PLUGINDIR);
    GWEN_PathManager_AddPathFromWinReg(GWEN_PM_LIBNAME,
				       GWEN_PM_LIBNAME,
				       GWEN_PM_PLUGINDIR,
				       GWEN_REGKEY_PATHS,
				       GWEN_REGNAME_PLUGINDIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PathManager_AddRelPath(GWEN_PM_LIBNAME,
				GWEN_PM_LIBNAME,
				GWEN_PM_PLUGINDIR,
				PLUGINDIR,
				GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PathManager_AddPath(GWEN_PM_LIBNAME,
			     GWEN_PM_LIBNAME,
			     GWEN_PM_PLUGINDIR,
			     PLUGINDIR);
#endif

    /* ---------------------------------------------------------------------
     * datadir e.g. "/usr/share/gwenhywfar" */
    GWEN_PathManager_DefinePath(GWEN_PM_LIBNAME, GWEN_PM_DATADIR);
    GWEN_PathManager_AddPathFromWinReg(GWEN_PM_LIBNAME,
				       GWEN_PM_LIBNAME,
				       GWEN_PM_DATADIR,
				       GWEN_REGKEY_PATHS,
				       GWEN_REGNAME_DATADIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PathManager_AddRelPath(GWEN_PM_LIBNAME,
				GWEN_PM_LIBNAME,
				GWEN_PM_DATADIR,
				GWEN_DATADIR,
				GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PathManager_AddPath(GWEN_PM_LIBNAME,
			     GWEN_PM_LIBNAME,
			     GWEN_PM_DATADIR,
			     GWEN_DATADIR);
#endif

    /* ---------------------------------------------------------------------
     * system datadir e.g. "/usr/share" */
    GWEN_PathManager_DefinePath(GWEN_PM_LIBNAME, GWEN_PM_SYSDATADIR);
    GWEN_PathManager_AddPathFromWinReg(GWEN_PM_LIBNAME,
				       GWEN_PM_LIBNAME,
				       GWEN_PM_SYSDATADIR,
				       GWEN_REGKEY_PATHS,
				       GWEN_REGNAME_SYSDATADIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PathManager_AddRelPath(GWEN_PM_LIBNAME,
				GWEN_PM_LIBNAME,
				GWEN_PM_SYSDATADIR,
				GWEN_SYSDATADIR,
				GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PathManager_AddPath(GWEN_PM_LIBNAME,
			     GWEN_PM_LIBNAME,
			     GWEN_PM_SYSDATADIR,
			     GWEN_SYSDATADIR);
#endif

    /* Initialize other modules. */
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing I18N module");
    err=GWEN_I18N_ModuleInit();
    if (err)
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing InetAddr module");
    err=GWEN_InetAddr_ModuleInit();
    if (err)
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing Socket module");
    err=GWEN_Socket_ModuleInit();
    if (err)
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing Libloader module");
    err=GWEN_LibLoader_ModuleInit();
    if (err)
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing Crypt3 module");
    err=GWEN_Crypt3_ModuleInit();
    if (err)
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing Process module");
    err=GWEN_Process_ModuleInit();
    if (err)
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing Plugin module");
    err=GWEN_Plugin_ModuleInit();
    if (err)
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing DataBase IO module");
    err=GWEN_DBIO_ModuleInit();
    if (err)
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing ConfigMgr module");
    err=GWEN_ConfigMgr_ModuleInit();
    if (err)
      return err;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Initializing CryptToken2 module");
    err=GWEN_Crypt_Token_ModuleInit();
    if (err)
      return err;
    /* add more modules here */

  }
  gwen_is_initialized++;

  return 0;

}



int GWEN_Fini(void) {
  int err;

  err=0;

  if (gwen_is_initialized==0)
    return 0;

  gwen_is_initialized--;
  if (gwen_is_initialized==0) {
    int lerr;

    /* add more modules here */
    lerr=GWEN_Crypt_Token_ModuleFini();
    if (lerr) {
      err=lerr;
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
		"Could not deinitialze module CryptToken2");
    }
    lerr=GWEN_ConfigMgr_ModuleFini();
    if (lerr) {
      err=lerr;
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
		"Could not deinitialze module ConfigMgr");
    }
    lerr=GWEN_DBIO_ModuleFini();
    if (lerr) {
      err=lerr;
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
		"Could not deinitialze module DBIO");
    }
    lerr=GWEN_Plugin_ModuleFini();
    if (lerr) {
      err=lerr;
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
		"Could not deinitialze module Plugin");
    }
    lerr=GWEN_Process_ModuleFini();
    if (lerr) {
      err=lerr;
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
		"Could not deinitialze module Process");
    }
    lerr=GWEN_Crypt3_ModuleFini();
    if (lerr) {
      err=lerr;
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
		"Could not deinitialze module Crypt3");
    }
    lerr=GWEN_LibLoader_ModuleFini();
    if (lerr) {
      err=lerr;
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
	      "Could not deinitialze module LibLoader");
    }
    lerr=GWEN_Socket_ModuleFini();
    if (lerr) {
      err=lerr;
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
		"Could not deinitialze module Socket");
    }
    lerr=GWEN_InetAddr_ModuleFini();
    if (lerr) {
      err=lerr;
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module InetAddr");
    }

    lerr=GWEN_I18N_ModuleFini();
    if (lerr) {
      err=lerr;
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
		"Could not deinitialze module I18N");
    }

    lerr=GWEN_PathManager_ModuleFini();
    if (lerr) {
      err=lerr;
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
                "Could not deinitialze module PathManager");
    }

    GWEN_Error_ModuleFini();

    /* these two modules must be deinitialized at last */
    lerr=GWEN_Logger_ModuleFini();
    if (lerr) {
      err=lerr;
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
		"Could not deinitialze module Logger");
    }

    lerr=GWEN_Memory_ModuleFini();
    if (lerr) {
      err=lerr;
      DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_Fini: "
		"Could not deinitialze module Memory");
    }

    gnutls_global_deinit();

  }

  return err;
}



int GWEN_Fini_Forced(void) {
  if (gwen_is_initialized)
    gwen_is_initialized=1;
  return GWEN_Fini();
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



