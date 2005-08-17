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
#include <strings.h>
#include <ctype.h>

#ifdef OS_WIN32
# include <windows.h>
#endif

static GWEN_PLUGIN_MANAGER_LIST *gwen_plugin_manager__list=0;


GWEN_INHERIT_FUNCTIONS(GWEN_PLUGIN)
GWEN_LIST_FUNCTIONS(GWEN_PLUGIN, GWEN_Plugin)
GWEN_INHERIT_FUNCTIONS(GWEN_PLUGIN_MANAGER)
GWEN_LIST_FUNCTIONS(GWEN_PLUGIN_MANAGER, GWEN_PluginManager)



GWEN_ERRORCODE GWEN_Plugin_ModuleInit(){
  gwen_plugin_manager__list=GWEN_PluginManager_List_new();
  return 0;
}



GWEN_ERRORCODE GWEN_Plugin_ModuleFini(){
  GWEN_PluginManager_List_free(gwen_plugin_manager__list);
  return 0;
}



GWEN_PLUGIN *GWEN_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
                             const char *name,
                             const char *fileName){
  GWEN_PLUGIN *p;

  assert(pm);
  assert(name);
  GWEN_NEW_OBJECT(GWEN_PLUGIN, p);
  DBG_MEM_INC("GWEN_PLUGIN", 0);
  p->refCount=1;
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
    DBG_MEM_DEC("GWEN_PLUGIN");
    assert(p->refCount);
    if (--(p->refCount)==0) {
      GWEN_INHERIT_FINI(GWEN_PLUGIN, p);
      free(p->name);
      free(p->fileName);
      if (p->libLoader) {
	GWEN_LibLoader_CloseLibrary(p->libLoader);
	GWEN_LibLoader_free(p->libLoader);
      }
      GWEN_LIST_FINI(GWEN_PLUGIN, p);
      GWEN_FREE_OBJECT(p);
    } /* if refCount reaches zero */
  } /* if p */
}



void GWEN_Plugin_Attach(GWEN_PLUGIN *p){
  assert(p);
  assert(p->refCount);
  DBG_MEM_INC("GWEN_PLUGIN", 1);
  p->refCount++;
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



void GWEN_Plugin_SetLibLoader(GWEN_PLUGIN *p, GWEN_LIBLOADER *ll){
  assert(p);
  p->libLoader=ll;
}








GWEN_PLUGIN_MANAGER *GWEN_PluginManager_new(const char *name){
  GWEN_PLUGIN_MANAGER *pm;

  assert(name);
  GWEN_NEW_OBJECT(GWEN_PLUGIN_MANAGER, pm);
  DBG_MEM_INC("GWEN_PLUGIN_MANAGER", 0);
  GWEN_INHERIT_INIT(GWEN_PLUGIN_MANAGER, pm);
  GWEN_LIST_INIT(GWEN_PLUGIN_MANAGER, pm);
  pm->name=strdup(name);
  pm->paths=GWEN_StringList_new();
  /* let the string list handle reference counters */
  GWEN_StringList_SetIgnoreRefCount(pm->paths, 0);
  pm->plugins=GWEN_Plugin_List_new();

  return pm;
}



void GWEN_PluginManager_free(GWEN_PLUGIN_MANAGER *pm){
  if (pm) {
    DBG_MEM_DEC("GWEN_PLUGIN_MANAGER");
    GWEN_Plugin_List_free(pm->plugins);
    GWEN_INHERIT_FINI(GWEN_PLUGIN_MANAGER, pm);
    GWEN_StringList_free(pm->paths);
    free(pm->name);
    GWEN_LIST_FINI(GWEN_PLUGIN_MANAGER, pm);
    GWEN_FREE_OBJECT(pm);
  }
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



int GWEN_PluginManager_InsertPath(GWEN_PLUGIN_MANAGER *pm,
                                  const char *s) {
  assert(pm);
  return GWEN_StringList_InsertString(pm->paths, s, 0, 1);
}



int GWEN_PluginManager_RemovePath(GWEN_PLUGIN_MANAGER *pm,
                                  const char *s) {
  assert(pm);
  return GWEN_StringList_RemoveString(pm->paths, s);
}



int GWEN_PluginManager_AddPathFromWinReg(GWEN_PLUGIN_MANAGER *pm,
					 const char *keypath,
					 const char *varname)
{
#ifdef OS_WIN32
  HKEY hkey;
  TCHAR nbuffer[MAX_PATH];
  BYTE vbuffer[MAX_PATH];
  DWORD nsize;
  DWORD vsize;
  DWORD typ;
  int i;

  assert(pm);

  snprintf(nbuffer, sizeof(nbuffer), keypath);

  /* open the key */
  if (RegOpenKey(HKEY_CURRENT_USER, nbuffer, &hkey)){
    DBG_ERROR(GWEN_LOGDOMAIN, "RegOpenKey %s failed.", keypath);
    return 1;
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
    if (strcasecmp(nbuffer, varname)==0 && typ==REG_SZ) {
      /* variable found */
      RegCloseKey(hkey);
      return GWEN_StringList_AppendString(pm->paths, (char*)vbuffer, 0, 1);
    }
  } /* for */

  RegCloseKey(hkey);
  DBG_ERROR(GWEN_LOGDOMAIN, 
	    "In RegKey \"%s\" the variable \"%s\" does not exist", 
	    keypath, varname);
  return 1;

#else /* OS_WIN32 */
  return 0;
#endif /* OS_WIN32 */
}



GWEN_PLUGIN *GWEN_PluginManager_LoadPlugin(GWEN_PLUGIN_MANAGER *pm,
                                           const char *modname){
  GWEN_LIBLOADER *ll;
  GWEN_PLUGIN *plugin;
  GWEN_PLUGIN_FACTORYFN fn;
  void *p;
  GWEN_BUFFER *nbuf;
  const char *s;
  const char *fname;
  GWEN_ERRORCODE err;
  GWEN_STRINGLISTENTRY *se;

  ll=GWEN_LibLoader_new();

  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  se=GWEN_StringList_FirstEntry(pm->paths);
  fname=0;
  while(se) {
    fname=GWEN_StringListEntry_Data(se);
    assert(fname);
    if (GWEN_LibLoader_OpenLibraryWithPath(ll, fname,
					   GWEN_Buffer_GetStart(nbuf))==0)
      break;
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN,
		"Could not load plugin \"%s\" from \"%s\"", modname, fname);
    }
    se=GWEN_StringListEntry_Next(se);
  }
  if (!se) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Plugin \"%s\" not found.", modname);
    GWEN_Buffer_free(nbuf);
    GWEN_LibLoader_free(ll);
    return 0;
  }
  GWEN_Buffer_free(nbuf);

  /* create name of init function */
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=pm->name;
  while(*s) GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendByte(nbuf, '_');
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendString(nbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(nbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_Buffer_free(nbuf);
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }
  GWEN_Buffer_free(nbuf);

  fn=(GWEN_PLUGIN_FACTORYFN)p;
  assert(fn);
  plugin=fn(pm, modname, fname);
  if (!plugin) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error in plugin: No plugin created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  GWEN_Plugin_SetLibLoader(plugin, ll);
  return plugin;
}



GWEN_PLUGIN *GWEN_PluginManager_LoadPluginFile(GWEN_PLUGIN_MANAGER *pm,
                                               const char *modname,
                                               const char *fname){
  GWEN_LIBLOADER *ll;
  GWEN_PLUGIN *plugin;
  GWEN_PLUGIN_FACTORYFN fn;
  void *p;
  GWEN_BUFFER *nbuf;
  const char *s;
  GWEN_ERRORCODE err;

  ll=GWEN_LibLoader_new();
  if (GWEN_LibLoader_OpenLibrary(ll, fname)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Could not load plugin \"%s\" (%s)", modname, fname);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* create name of init function */
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=pm->name;
  while(*s) GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendByte(nbuf, '_');
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendString(nbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(nbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_Buffer_free(nbuf);
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }
  GWEN_Buffer_free(nbuf);

  fn=(GWEN_PLUGIN_FACTORYFN)p;
  assert(fn);
  plugin=fn(pm, modname, fname);
  if (!plugin) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error in plugin: No plugin created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  GWEN_Plugin_SetLibLoader(plugin, ll);

  return plugin;
}



GWEN_PLUGIN *GWEN_PluginManager__FindPlugin(GWEN_PLUGIN_MANAGER *pm,
                                            const char *s){
  GWEN_PLUGIN *p;

  assert(pm);
  p=GWEN_Plugin_List_First(pm->plugins);
  while(p) {
    if (strcasecmp(p->name, s)==0)
      break;
    p=GWEN_Plugin_List_Next(p);
  }

  return p;
}



GWEN_PLUGIN *GWEN_PluginManager_GetPlugin(GWEN_PLUGIN_MANAGER *pm,
                                          const char *s){
  GWEN_PLUGIN *p;

  p=GWEN_PluginManager__FindPlugin(pm, s);
  if (p)
    return p;
  p=GWEN_PluginManager_LoadPlugin(pm, s);
  if (p) {
    GWEN_Plugin_List_Add(p, pm->plugins);
    return p;
  }
  DBG_INFO(GWEN_LOGDOMAIN, "Plugin \"%s\" not found", s);
  return 0;
}



GWEN_PLUGIN_MANAGER *GWEN_PluginManager_FindPluginManager(const char *s){
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_List_First(gwen_plugin_manager__list);
  while(pm) {
    if (strcasecmp(pm->name, s)==0)
      break;
    pm=GWEN_PluginManager_List_Next(pm);
  }

  return pm;
}



int GWEN_PluginManager_Register(GWEN_PLUGIN_MANAGER *pm){
  GWEN_PLUGIN_MANAGER *tpm;

  assert(gwen_plugin_manager__list);
  assert(pm);
  tpm=GWEN_PluginManager_FindPluginManager(pm->name);
  if (tpm) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Plugin type \"%s\" already registered",
              pm->name);
    return -1;
  }
  GWEN_PluginManager_List_Add(pm, gwen_plugin_manager__list);
  DBG_INFO(GWEN_LOGDOMAIN,
           "Plugin type \"%s\" registered",
           pm->name);
  return 0;
}



int GWEN_PluginManager_Unregister(GWEN_PLUGIN_MANAGER *pm){
  GWEN_PLUGIN_MANAGER *tpm;

  assert(gwen_plugin_manager__list);
  assert(pm);
  tpm=GWEN_PluginManager_FindPluginManager(pm->name);
  if (!tpm) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Plugin type \"%s\" not registered",
              pm->name);
    return -1;
  }
  GWEN_PluginManager_List_Del(pm);
  DBG_INFO(GWEN_LOGDOMAIN,
           "Plugin type \"%s\" unregistered",
           pm->name);
  return 0;
}



GWEN_PLUGIN_DESCRIPTION_LIST2*
GWEN_PluginManager_GetPluginDescrs(GWEN_PLUGIN_MANAGER *pm){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pl;
  GWEN_STRINGLISTENTRY *se;

  se=GWEN_StringList_FirstEntry(pm->paths);
  if (!se) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No paths given");
    return 0;
  }

  pl=GWEN_PluginDescription_List2_new();
  while(se) {
    int rv;
    const char *path;

    path=GWEN_StringListEntry_Data(se);
    assert(path);
    rv=GWEN_LoadPluginDescrsByType(path, pm->name, pl);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN,
               "Error loading plugin description in \"%s\"", path);
    }
    se=GWEN_StringListEntry_Next(se);
  } /* while */

  if (GWEN_PluginDescription_List2_GetSize(pl)==0) {
    GWEN_PluginDescription_List2_free(pl);
    return 0;
  }
  return pl;
}


const GWEN_STRINGLIST *
GWEN_PluginManager_GetPaths(const GWEN_PLUGIN_MANAGER *pm){
  assert(pm);
  return pm->paths;
}



GWEN_PLUGIN_DESCRIPTION*
GWEN_PluginManager_GetPluginDescr(GWEN_PLUGIN_MANAGER *pm,
                                  const char *modName) {
  GWEN_PLUGIN_DESCRIPTION_LIST2 *dl;

  dl=GWEN_PluginManager_GetPluginDescrs(pm);
  if (dl==0)
    return 0;
  else {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *dit;

    dit=GWEN_PluginDescription_List2_First(dl);
    if (dit) {
      GWEN_PLUGIN_DESCRIPTION *d;

      d=GWEN_PluginDescription_List2Iterator_Data(dit);
      while(d) {
        if (strcasecmp(GWEN_PluginDescription_GetName(d), modName)==0)
          break;
        d=GWEN_PluginDescription_List2Iterator_Next(dit);
      }
      GWEN_PluginDescription_List2Iterator_free(dit);

      if (d) {
        d=GWEN_PluginDescription_dup(d);
        GWEN_PluginDescription_List2_freeAll(dl);
        return d;
      }
    }
    GWEN_PluginDescription_List2_freeAll(dl);
  }

  return 0;
}






