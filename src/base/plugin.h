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


typedef struct GWEN_PLUGIN GWEN_PLUGIN;
typedef struct GWEN_PLUGIN_MANAGER GWEN_PLUGIN_MANAGER;


GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_PLUGIN, GWENHYWFAR_API)
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_PLUGIN_MANAGER, GWENHYWFAR_API)




GWEN_PLUGIN *GWEN_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
                             const char *name,
                             const char *fileName);
void GWEN_Plugin_free(GWEN_PLUGIN *p);

GWEN_PLUGIN_MANAGER *GWEN_Plugin_GetManager(const GWEN_PLUGIN *p);
const char *GWEN_Plugin_GetName(const GWEN_PLUGIN *p);
const char *GWEN_Plugin_GetFileName(const GWEN_PLUGIN *p);
GWEN_LIBLOADER *GWEN_Plugin_GetLibLoader(const GWEN_PLUGIN *p);







GWEN_PLUGIN_MANAGER *GWEN_PluginManager_new(const char *name);
void GWEN_PluginManager_free(GWEN_PLUGIN_MANAGER *pm);


const char *GWEN_PluginManager_GetName(const GWEN_PLUGIN_MANAGER *pm);
int GWEN_PluginManager_AddPath(GWEN_PLUGIN_MANAGER *pm,
                               const char *s);

GWEN_PLUGIN *GWEN_PluginManager_LoadPlugin(GWEN_PLUGIN_MANAGER *pm,
                                           const char *s);
GWEN_PLUGIN *GWEN_PluginManager_LoadPluginFile(GWEN_PLUGIN_MANAGER *pm,
                                               const char *s);


GWEN_PLUGIN *GWEN_PluginManager_GetPlugin(GWEN_PLUGIN_MANAGER *pm,
                                          const char *s);


#endif


