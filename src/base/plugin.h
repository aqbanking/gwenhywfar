/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri Sep 12 2003
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


#ifndef GWENHYWFAR_PLUGIN_H
#define GWENHYWFAR_PLUGIN_H


#include <gwenhywfar/inherit.h>
#include <gwenhywfar/libloader.h>
#include <gwenhywfar/plugindescr.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_PLUGIN GWEN_PLUGIN;
typedef struct GWEN_PLUGIN_MANAGER GWEN_PLUGIN_MANAGER;


GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_PLUGIN, GWENHYWFAR_API)
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_PLUGIN_MANAGER, GWENHYWFAR_API)


typedef GWEN_PLUGIN* (*GWEN_PLUGIN_FACTORYFN)(GWEN_PLUGIN_MANAGER *pm,
                                              const char *name,
                                              const char *fileName);


GWENHYWFAR_API
GWEN_PLUGIN *GWEN_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
                             const char *name,
                             const char *fileName);

GWENHYWFAR_API
void GWEN_Plugin_free(GWEN_PLUGIN *p);

GWENHYWFAR_API
GWEN_PLUGIN_MANAGER *GWEN_Plugin_GetManager(const GWEN_PLUGIN *p);

GWENHYWFAR_API
const char *GWEN_Plugin_GetName(const GWEN_PLUGIN *p);

GWENHYWFAR_API
const char *GWEN_Plugin_GetFileName(const GWEN_PLUGIN *p);

GWENHYWFAR_API
GWEN_LIBLOADER *GWEN_Plugin_GetLibLoader(const GWEN_PLUGIN *p);







GWENHYWFAR_API
GWEN_PLUGIN_MANAGER *GWEN_PluginManager_new(const char *name);

GWENHYWFAR_API
void GWEN_PluginManager_free(GWEN_PLUGIN_MANAGER *pm);


GWENHYWFAR_API
const char *GWEN_PluginManager_GetName(const GWEN_PLUGIN_MANAGER *pm);

/** Add a directory path to lookup plugins from. */
GWENHYWFAR_API
int GWEN_PluginManager_AddPath(GWEN_PLUGIN_MANAGER *pm,
                               const char *s);
/** Add a directory path from the windows registry HKEY_CURRENT_USER,
 * to lookup plugins from. On Non-Windows systems, this function does
 * nothing and returns zero.
 *
 * FIXME: Maybe it should be possible whether to lookup in
 * HKEY_CURRENT_USER vs. HKEY_LOCAL_MACHINE?
 *
 * @return Zero on success, and non-zero on error.
 *
 * @param pm The PluginManager
 *
 * @param keypath The path to the registry key,
 * e.g. "Software\\MySoftware\\Whatever"
 *
 * @param varname The key name (variable name?) inside the given
 * registry key, e.g. "myvariable".
*/
GWENHYWFAR_API
int GWEN_PluginManager_AddPathFromWinReg(GWEN_PLUGIN_MANAGER *pm,
					 const char *keypath,
					 const char *varname);

/** Returns the list of all search paths of the given
 * PluginManager. */
GWENHYWFAR_API 
const GWEN_STRINGLIST *
GWEN_PluginManager_GetPaths(const GWEN_PLUGIN_MANAGER *pm);

GWENHYWFAR_API
GWEN_PLUGIN *GWEN_PluginManager_LoadPlugin(GWEN_PLUGIN_MANAGER *pm,
                                           const char *modName);

GWENHYWFAR_API
GWEN_PLUGIN *GWEN_PluginManager_LoadPluginFile(GWEN_PLUGIN_MANAGER *pm,
					       const char *modName,
					       const char *fname);


GWENHYWFAR_API
GWEN_PLUGIN *GWEN_PluginManager_GetPlugin(GWEN_PLUGIN_MANAGER *pm,
                                          const char *s);

GWENHYWFAR_API
int GWEN_PluginManager_Register(GWEN_PLUGIN_MANAGER *pm);

GWENHYWFAR_API
int GWEN_PluginManager_Unregister(GWEN_PLUGIN_MANAGER *pm);

GWENHYWFAR_API
GWEN_PLUGIN_MANAGER *GWEN_PluginManager_FindPluginManager(const char *s);


GWENHYWFAR_API
GWEN_PLUGIN_DESCRIPTION_LIST2*
GWEN_PluginManager_GetPluginDescrs(GWEN_PLUGIN_MANAGER *pm);


#ifdef __cplusplus
}
#endif

#endif


