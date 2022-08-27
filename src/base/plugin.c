/***************************************************************************
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
#include <gwenhywfar/debug.h>

#include <ctype.h>

#ifdef OS_WIN32
# include <windows.h>
#endif

static GWEN_PLUGIN_MANAGER_LIST *gwen_plugin_manager__list=0;


GWEN_INHERIT_FUNCTIONS(GWEN_PLUGIN)
GWEN_LIST_FUNCTIONS(GWEN_PLUGIN, GWEN_Plugin)
GWEN_INHERIT_FUNCTIONS(GWEN_PLUGIN_MANAGER)
GWEN_LIST_FUNCTIONS(GWEN_PLUGIN_MANAGER, GWEN_PluginManager)


static void _setLibLoader(GWEN_PLUGIN *p, GWEN_LIBLOADER *ll);
static GWEN_PLUGIN *_findPluginInListByName(GWEN_PLUGIN_MANAGER *pm, const char *s);
static GWEN_PLUGIN *_createPluginFromLibloader(GWEN_PLUGIN_MANAGER *pm, GWEN_LIBLOADER *libLoader, const char *modname);
static GWEN_LIBLOADER *_searchAndLoadModule(GWEN_PLUGIN_MANAGER *pm, const char *modname);
static GWEN_PLUGIN_DESCRIPTION_LIST2 *_getPluginDescrsFromPluginList(GWEN_PLUGIN_MANAGER *pm);
static GWEN_PLUGIN_DESCRIPTION_LIST2 *_loadPluginDescrs(GWEN_PLUGIN_MANAGER *pm);
static GWEN_PLUGIN_DESCRIPTION_LIST2 *_combinedPluginDescrList(const GWEN_PLUGIN_DESCRIPTION_LIST2 *pdlLoaded,
							       const GWEN_PLUGIN_DESCRIPTION_LIST2 *pdlFromPluginList);
static GWEN_PLUGIN_DESCRIPTION *_findPluginDescrByNameInList2(const GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl, const char *name);




int GWEN_Plugin_ModuleInit(void)
{
  gwen_plugin_manager__list=GWEN_PluginManager_List_new();
  return 0;
}



int GWEN_Plugin_ModuleFini(void)
{
  GWEN_PluginManager_List_free(gwen_plugin_manager__list);
  return 0;
}



GWEN_PLUGIN *GWEN_Plugin_new(GWEN_PLUGIN_MANAGER *pm, const char *name, const char *fileName)
{
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



void GWEN_Plugin_free(GWEN_PLUGIN *p)
{
  if (p) {
    DBG_MEM_DEC("GWEN_PLUGIN");
    assert(p->refCount);
    if (--(p->refCount)==0) {
      GWEN_INHERIT_FINI(GWEN_PLUGIN, p);
      GWEN_PluginDescription_free(p->pluginDescription);
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



void GWEN_Plugin_Attach(GWEN_PLUGIN *p)
{
  assert(p);
  assert(p->refCount);
  DBG_MEM_INC("GWEN_PLUGIN", 1);
  p->refCount++;
}



GWEN_PLUGIN_MANAGER *GWEN_Plugin_GetManager(const GWEN_PLUGIN *p)
{
  assert(p);
  return p->manager;
}



const char *GWEN_Plugin_GetName(const GWEN_PLUGIN *p)
{
  assert(p);
  return p->name;
}



const char *GWEN_Plugin_GetFileName(const GWEN_PLUGIN *p)
{
  assert(p);
  return p->fileName;
}



GWEN_LIBLOADER *GWEN_Plugin_GetLibLoader(const GWEN_PLUGIN *p)
{
  assert(p);
  return p->libLoader;
}



void _setLibLoader(GWEN_PLUGIN *p, GWEN_LIBLOADER *ll)
{
  assert(p);
  p->libLoader=ll;
}



const GWEN_PLUGIN_DESCRIPTION *GWEN_Plugin_GetPluginDescription(const GWEN_PLUGIN *p)
{
  assert(p);
  return p->pluginDescription;
}



void GWEN_Plugin_SetPluginDescription(GWEN_PLUGIN *p, GWEN_PLUGIN_DESCRIPTION *pd)
{
  assert(p);
  if (p->pluginDescription)
    GWEN_PluginDescription_free(p->pluginDescription);
  p->pluginDescription=pd;
}







GWEN_PLUGIN_MANAGER *GWEN_PluginManager_new(const char *name, const char *destLib)
{
  GWEN_PLUGIN_MANAGER *pm;

  assert(name);
  assert(destLib);
  GWEN_NEW_OBJECT(GWEN_PLUGIN_MANAGER, pm);
  DBG_MEM_INC("GWEN_PLUGIN_MANAGER", 0);
  GWEN_INHERIT_INIT(GWEN_PLUGIN_MANAGER, pm);
  GWEN_LIST_INIT(GWEN_PLUGIN_MANAGER, pm);
  pm->name=strdup(name);
  pm->destLib=strdup(destLib);
  pm->plugins=GWEN_Plugin_List_new();

  return pm;
}



void GWEN_PluginManager_free(GWEN_PLUGIN_MANAGER *pm)
{
  if (pm) {
    DBG_MEM_DEC("GWEN_PLUGIN_MANAGER");
    GWEN_Plugin_List_free(pm->plugins);
    GWEN_INHERIT_FINI(GWEN_PLUGIN_MANAGER, pm);
    free(pm->destLib);
    free(pm->name);
    GWEN_LIST_FINI(GWEN_PLUGIN_MANAGER, pm);
    GWEN_FREE_OBJECT(pm);
  }
}



const char *GWEN_PluginManager_GetName(const GWEN_PLUGIN_MANAGER *pm)
{
  assert(pm);
  return pm->name;
}



int GWEN_PluginManager_AddPath(GWEN_PLUGIN_MANAGER *pm, const char *callingLib, const char *s)
{
  assert(pm);
  return GWEN_PathManager_AddPath(callingLib, pm->destLib, pm->name, s);
}



int GWEN_PluginManager_AddRelPath(GWEN_PLUGIN_MANAGER *pm, const char *callingLib, const char *s, GWEN_PATHMANAGER_RELMODE rm)
{
  assert(pm);
  return GWEN_PathManager_AddRelPath(callingLib, pm->destLib, pm->name, s, rm);
}



int GWEN_PluginManager_InsertPath(GWEN_PLUGIN_MANAGER *pm, const char *callingLib, const char *s)
{
  assert(pm);
  return GWEN_PathManager_InsertPath(callingLib, pm->destLib, pm->name, s);
}



int GWEN_PluginManager_InsertRelPath(GWEN_PLUGIN_MANAGER *pm, const char *callingLib, const char *relpath, GWEN_PATHMANAGER_RELMODE rm)
{
  assert(pm);
  return GWEN_PathManager_InsertRelPath(callingLib, pm->destLib, pm->name, relpath, rm);
}



int GWEN_PluginManager_RemovePath(GWEN_PLUGIN_MANAGER *pm, const char *callingLib, const char *s)
{
  assert(pm);
  return GWEN_PathManager_RemovePath(callingLib, pm->destLib, pm->name, s);
}



#ifdef OS_WIN32
int GWEN_PluginManager_AddPathFromWinReg(GWEN_PLUGIN_MANAGER *pm,
                                         const char *callingLib,
                                         const char *keypath,
                                         const char *varname)
{
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
  if (RegOpenKey(HKEY_LOCAL_MACHINE, nbuffer, &hkey)) {
    DBG_INFO(GWEN_LOGDOMAIN, "RegOpenKey %s failed.", keypath);
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
      return GWEN_PathManager_AddPath(callingLib,
                                      pm->destLib,
                                      pm->name,
                                      (char *)vbuffer);
    }
  } /* for */

  RegCloseKey(hkey);
  DBG_INFO(GWEN_LOGDOMAIN, "In RegKey \"%s\" the variable \"%s\" does not exist", keypath, varname);
  return 1;

}

#else /* OS_WIN32 */

int GWEN_PluginManager_AddPathFromWinReg(GWEN_UNUSED GWEN_PLUGIN_MANAGER *pm,
                                         GWEN_UNUSED const char *callingLib,
                                         GWEN_UNUSED const char *keypath,
                                         GWEN_UNUSED const char *varname)
{
  return 0;
}
#endif /* OS_WIN32 */



GWEN_PLUGIN *GWEN_PluginManager_LoadPlugin(GWEN_PLUGIN_MANAGER *pm, const char *modname)
{
  GWEN_LIBLOADER *libLoader;
  GWEN_PLUGIN *plugin;

  libLoader=_searchAndLoadModule(pm, modname);
  if (libLoader==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "No suitable module found for %s/%s", pm->name, modname);
    return NULL;
  }

  plugin=_createPluginFromLibloader(pm, libLoader, modname);
  if (plugin==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not create plugin from module");
    GWEN_LibLoader_CloseLibrary(libLoader);
    GWEN_LibLoader_free(libLoader);
    return NULL;
  }

  _setLibLoader(plugin, libLoader);
  return plugin;
}



GWEN_LIBLOADER *_searchAndLoadModule(GWEN_PLUGIN_MANAGER *pm, const char *modname)
{
  GWEN_LIBLOADER *libLoader;
  GWEN_BUFFER *nbuf;
  const char *s;
  GWEN_STRINGLIST *pathList;
  GWEN_STRINGLISTENTRY *se;

  assert(pm);
  libLoader=GWEN_LibLoader_new();
  pathList=GWEN_PathManager_GetPaths(pm->destLib, pm->name);
  if (pathList==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No paths for plugins (%s)", pm->name);
    GWEN_LibLoader_free(libLoader);
    return NULL;
  }
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=modname;
  while (*s)
    GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  se=GWEN_StringList_FirstEntry(pathList);

  while (se) {
    const char *fname;

    fname=GWEN_StringListEntry_Data(se);
    assert(fname);
    if (GWEN_LibLoader_OpenLibraryWithPath(libLoader, fname, GWEN_Buffer_GetStart(nbuf))==0)
      break;
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Could not load plugin \"%s\" from \"%s\"", modname, fname);
    }
    se=GWEN_StringListEntry_Next(se);
  } /* while */
  if (!se) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Plugin \"%s\" not found.", modname);
    GWEN_Buffer_free(nbuf);
    GWEN_StringList_free(pathList);
    GWEN_LibLoader_free(libLoader);
    return NULL;
  }
  GWEN_Buffer_free(nbuf);
  GWEN_StringList_free(pathList);

  return libLoader;
}



GWEN_PLUGIN *_createPluginFromLibloader(GWEN_PLUGIN_MANAGER *pm, GWEN_LIBLOADER *libLoader, const char *modname)
{
  GWEN_BUFFER *nbuf;
  const char *s;
  int err;
  GWEN_PLUGIN *plugin;
  GWEN_PLUGIN_FACTORYFN fn;
  void *p;

  /* create name of init function */
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=pm->name;
  while (*s)
    GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendByte(nbuf, '_');
  s=modname;
  while (*s)
    GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendString(nbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(libLoader, GWEN_Buffer_GetStart(nbuf), &p);
  if (err) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_Buffer_free(nbuf);
    return NULL;
  }
  GWEN_Buffer_free(nbuf);

  fn=(GWEN_PLUGIN_FACTORYFN)p;
  assert(fn);
  plugin=fn(pm, modname, "");
  if (!plugin) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error in plugin: No plugin created for %s/%s", pm->name, modname);
    return NULL;
  }
  else {
    GWEN_PLUGIN_DESCRIPTION *pd;

    pd=GWEN_PluginDescription_new();
    GWEN_PluginDescription_SetName(pd, modname);
    GWEN_PluginDescription_SetType(pd, pm->name);
    GWEN_Plugin_SetPluginDescription(plugin, pd);
  }

  return plugin;
}



GWEN_PLUGIN *GWEN_PluginManager_LoadPluginFile(GWEN_PLUGIN_MANAGER *pm, const char *modname, const char *fname)
{
  GWEN_LIBLOADER *ll;
  GWEN_PLUGIN *plugin;
  GWEN_PLUGIN_FACTORYFN fn;
  void *p;
  GWEN_BUFFER *nbuf;
  const char *s;
  int err;

  ll=GWEN_LibLoader_new();
  if (GWEN_LibLoader_OpenLibrary(ll, fname)) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Could not load plugin \"%s\" (%s)", modname, fname);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* create name of init function */
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=pm->name;
  while (*s)
    GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendByte(nbuf, '_');
  s=modname;
  while (*s)
    GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendString(nbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(nbuf), &p);
  if (err) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
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
    DBG_INFO(GWEN_LOGDOMAIN, "Error in plugin: No plugin created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  _setLibLoader(plugin, ll);

  return plugin;
}



GWEN_PLUGIN *GWEN_PluginManager_GetPlugin(GWEN_PLUGIN_MANAGER *pm, const char *s)
{
  GWEN_PLUGIN *p;

  p=_findPluginInListByName(pm, s);
  if (p) {
    DBG_INFO(GWEN_LOGDOMAIN, "Returning plugin \"%s\" from internal list", s);
    return p;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Trying to load plugin \"%s\"", s);
  p=GWEN_PluginManager_LoadPlugin(pm, s);
  if (p) {
    GWEN_Plugin_List_Add(p, pm->plugins);
    DBG_INFO(GWEN_LOGDOMAIN, "Plugin \"%s\" loaded and added to internal list", s);
    return p;
  }
  DBG_INFO(GWEN_LOGDOMAIN, "Plugin \"%s\" not found", s);
  return 0;
}



GWEN_PLUGIN_MANAGER *GWEN_PluginManager_FindPluginManager(const char *s)
{
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_List_First(gwen_plugin_manager__list);
  while (pm) {
    if (strcasecmp(pm->name, s)==0)
      break;
    pm=GWEN_PluginManager_List_Next(pm);
  }

  return pm;
}



int GWEN_PluginManager_Register(GWEN_PLUGIN_MANAGER *pm)
{
  GWEN_PLUGIN_MANAGER *tpm;
  int rv;

  assert(gwen_plugin_manager__list);
  assert(pm);
  tpm=GWEN_PluginManager_FindPluginManager(pm->name);
  if (tpm) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Plugin type \"%s\" already registered",
              pm->name);
    return -1;
  }

  rv=GWEN_PathManager_DefinePath(pm->destLib, pm->name);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not define path for plugin [%s:%s]",
             pm->destLib, pm->name);
    return rv;
  }

  GWEN_PluginManager_List_Add(pm, gwen_plugin_manager__list);
  DBG_INFO(GWEN_LOGDOMAIN,
           "Plugin type \"%s\" registered",
           pm->name);
  return 0;
}



int GWEN_PluginManager_Unregister(GWEN_PLUGIN_MANAGER *pm)
{
  GWEN_PLUGIN_MANAGER *tpm;
  int rv;

  assert(gwen_plugin_manager__list);
  assert(pm);
  tpm=GWEN_PluginManager_FindPluginManager(pm->name);
  if (!tpm) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Plugin type \"%s\" not registered",
              pm->name);
    return -1;
  }

  rv=GWEN_PathManager_UndefinePath(pm->destLib, pm->name);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not undefine path for plugin [%s:%s]",
             pm->destLib, pm->name);
    return rv;
  }

  GWEN_PluginManager_List_Del(pm);
  DBG_INFO(GWEN_LOGDOMAIN,
           "Plugin type \"%s\" unregistered",
           pm->name);
  return 0;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *GWEN_PluginManager_GetPluginDescrs(GWEN_PLUGIN_MANAGER *pm)
{
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pdlLoaded;
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pdlFromPluginList;

  pdlFromPluginList=_getPluginDescrsFromPluginList(pm);
  pdlLoaded=_loadPluginDescrs(pm);

  if (pdlFromPluginList && pdlLoaded){
    GWEN_PLUGIN_DESCRIPTION_LIST2 *pdlOut;

    pdlOut=_combinedPluginDescrList(pdlLoaded, pdlFromPluginList);
    GWEN_PluginDescription_List2_free(pdlFromPluginList);
    GWEN_PluginDescription_List2_free(pdlLoaded);
    return pdlOut;
  }
  else if (pdlFromPluginList) {
    return pdlFromPluginList;
  }
  else
    return pdlLoaded;
}



GWEN_STRINGLIST *GWEN_PluginManager_GetPaths(const GWEN_PLUGIN_MANAGER *pm)
{
  assert(pm);
  return GWEN_PathManager_GetPaths(pm->destLib, pm->name);
}



GWEN_PLUGIN_DESCRIPTION *GWEN_PluginManager_GetPluginDescr(GWEN_PLUGIN_MANAGER *pm, const char *modName)
{
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
      while (d) {
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



void GWEN_PluginManager_AddPlugin(GWEN_PLUGIN_MANAGER *pm, GWEN_PLUGIN *p)
{
  const char *modname;

  modname=GWEN_Plugin_GetName(p);
  if (modname && *modname) {
    const GWEN_PLUGIN_DESCRIPTION *pdInPlugin;

    pdInPlugin=GWEN_Plugin_GetPluginDescription(p);
    if (pdInPlugin==NULL) {
      GWEN_PLUGIN_DESCRIPTION *pd;

      DBG_INFO(GWEN_LOGDOMAIN, "No plugin description for \"%s\", creating one", modname);
      pd=GWEN_PluginDescription_new();
      GWEN_PluginDescription_SetName(pd, modname);
      GWEN_PluginDescription_SetType(pd, pm->name);
      GWEN_Plugin_SetPluginDescription(p, pd);
    }
    DBG_INFO(GWEN_LOGDOMAIN, "Adding plugin [%s] of type [%s]", modname, pm->name);
    GWEN_Plugin_List_Add(p, pm->plugins);
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Plugin to add has no name (type %s), not adding", pm->name);
  }
}



GWEN_PLUGIN *_findPluginInListByName(GWEN_PLUGIN_MANAGER *pm, const char *s)
{
  GWEN_PLUGIN *p;

  assert(pm);
  p=GWEN_Plugin_List_First(pm->plugins);
  while (p) {
    if (strcasecmp(p->name, s)==0)
      break;
    p=GWEN_Plugin_List_Next(p);
  }

  return p;
}


GWEN_PLUGIN_DESCRIPTION_LIST2 *_getPluginDescrsFromPluginList(GWEN_PLUGIN_MANAGER *pm)
{
  GWEN_PLUGIN *p;

  assert(pm);
  p=GWEN_Plugin_List_First(pm->plugins);
  if (p) {
    GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl;

    pdl=GWEN_PluginDescription_List2_new();
    while (p) {
      const GWEN_PLUGIN_DESCRIPTION *pd;

      pd=GWEN_Plugin_GetPluginDescription(p);
      if (pd)
	GWEN_PluginDescription_List2_PushBack(pdl, GWEN_PluginDescription_dup(pd));
      p=GWEN_Plugin_List_Next(p);
    } /* while(p) */

    if (GWEN_PluginDescription_List2_GetSize(pdl)==0) {
      GWEN_PluginDescription_List2_free(pdl);
      return NULL;
    }

    return pdl;
  }

  return NULL;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *_loadPluginDescrs(GWEN_PLUGIN_MANAGER *pm)
{
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pl;
  GWEN_STRINGLIST *sl;
  GWEN_STRINGLISTENTRY *se;

  sl=GWEN_PathManager_GetPaths(pm->destLib, pm->name);
  if (sl==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No paths for plugins (%s)", pm->name);
    return NULL;
  }
  se=GWEN_StringList_FirstEntry(sl);
  if (!se) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No paths given");
    GWEN_StringList_free(sl);
    return 0;
  }

  pl=GWEN_PluginDescription_List2_new();
  while (se) {
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
    GWEN_StringList_free(sl);
    return 0;
  }

  GWEN_StringList_free(sl);
  return pl;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *_combinedPluginDescrList(const GWEN_PLUGIN_DESCRIPTION_LIST2 *pdlLoaded,
							const GWEN_PLUGIN_DESCRIPTION_LIST2 *pdlFromPluginList)
{
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pdlOut;
  GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *iter;

  pdlOut=GWEN_PluginDescription_List2_new();

  /* first add entries from loaded list (more details in those entries) */
  iter=GWEN_PluginDescription_List2_First(pdlLoaded);
  if (iter) {
    GWEN_PLUGIN_DESCRIPTION *pd=NULL;

    pd=GWEN_PluginDescription_List2Iterator_Data(iter);
    while(pd) {
      GWEN_PluginDescription_List2_PushBack(pdlOut, pd);
      pd=GWEN_PluginDescription_List2Iterator_Next(iter);
    }

    GWEN_PluginDescription_List2Iterator_free(iter);
  }

  /* only add those entries from pluginList which are not already in the output list */
  iter=GWEN_PluginDescription_List2_First(pdlFromPluginList);
  if (iter) {
    GWEN_PLUGIN_DESCRIPTION *pd=NULL;

    pd=GWEN_PluginDescription_List2Iterator_Data(iter);
    while(pd) {
      const char *name;

      name=GWEN_PluginDescription_GetName(pd);
      if (name && *name && NULL==_findPluginDescrByNameInList2(pdlOut, name))
	GWEN_PluginDescription_List2_PushBack(pdlOut, pd);
      pd=GWEN_PluginDescription_List2Iterator_Next(iter);
    }

    GWEN_PluginDescription_List2Iterator_free(iter);
  }

  if (GWEN_PluginDescription_List2_GetSize(pdlOut)==0) {
    GWEN_PluginDescription_List2_free(pdlOut);
    return NULL;
  }

  return pdlOut;
}



GWEN_PLUGIN_DESCRIPTION *_findPluginDescrByNameInList2(const GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl, const char *name)
{
  GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *iter;
  GWEN_PLUGIN_DESCRIPTION *pd=NULL;

  iter=GWEN_PluginDescription_List2_First(pdl);
  if (iter) {

    pd=GWEN_PluginDescription_List2Iterator_Data(iter);
    while(pd) {
      const char *nameInList;

      nameInList=GWEN_PluginDescription_GetName(pd);
      if (nameInList && *nameInList && strcasecmp(nameInList, name)==0)
	break;
      pd=GWEN_PluginDescription_List2Iterator_Next(iter);
    }

    GWEN_PluginDescription_List2Iterator_free(iter);
  }

  return pd;
}


