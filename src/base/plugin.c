/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Apr 03 2003
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

#include "plugin_p.h"
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>




GWEN_INHERIT_FUNCTIONS(GWEN_PLUGIN)
GWEN_LIST_FUNCTIONS(GWEN_PLUGIN, GWEN_Plugin)
GWEN_INHERIT_FUNCTIONS(GWEN_PLUGIN_MANAGER)
GWEN_LIST_FUNCTIONS(GWEN_PLUGIN_MANAGER, GWEN_PluginManager)



GWEN_PLUGIN *GWEN_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
                             const char *name,
                             const char *fileName){
  GWEN_PLUGIN *p;

  assert(pm);
  assert(name);
  GWEN_NEW_OBJECT(GWEN_PLUGIN, p);
  GWEN_INHERIT_INIT(GWEN_PLUGIN, p);
  GWEN_LIST_INIT(GWEN_PLUGIN, p);
  p->manager=pm;
  p->name=strdup(name);
  if (fileName)
    p->fileName=strdup(fileName);

  return p;
}



void GWEN_Plugin_free(GWEN_PLUGIN *p){
  if (p) {
    GWEN_INHERIT_FINI(GWEN_PLUGIN, p);
    free(p->name);
    free(p->fileName);
    if (p->libLoader) {
      GWEN_LibLoader_CloseLibrary(p->libLoader);
      GWEN_LibLoader_free(p->libLoader);
    }
    GWEN_LIST_FINI(GWEN_PLUGIN, p);
    GWEN_FREE_OBJECT(p);
  }
}



GWEN_PLUGIN_MANAGER *GWEN_Plugin_GetManager(const GWEN_PLUGIN *p){
  assert(p);
  return p->manager;
}



const char *GWEN_Plugin_GetName(const GWEN_PLUGIN *p){
  assert(p);
  return p->name;
}



const char *GWEN_Plugin_GetFileName(const GWEN_PLUGIN *p){
  assert(p);
  return p->fileName;
}



GWEN_LIBLOADER *GWEN_Plugin_GetLibLoader(const GWEN_PLUGIN *p){
  assert(p);
  return p->libLoader;
}








GWEN_PLUGIN_MANAGER *GWEN_PluginManager_new(const char *name){
}



void GWEN_PluginManager_free(GWEN_PLUGIN_MANAGER *pm){
}



const char *GWEN_PluginManager_GetName(const GWEN_PLUGIN_MANAGER *pm){
  assert(pm);
  return pm->name;
}



int GWEN_PluginManager_AddPath(GWEN_PLUGIN_MANAGER *pm,
                               const char *s){
  assert(pm);
  return GWEN_StringList_AppendString(pm->paths, s, 0, 1);
}



GWEN_PLUGIN *GWEN_PluginManager_LoadPlugin(GWEN_PLUGIN_MANAGER *pm,
                                           const char *s){
}



GWEN_PLUGIN *GWEN_PluginManager_LoadPluginFile(GWEN_PLUGIN_MANAGER *pm,
                                               const char *s){
}



GWEN_PLUGIN *GWEN_PluginManager_GetPlugin(GWEN_PLUGIN_MANAGER *pm,
                                          const char *s){
}














