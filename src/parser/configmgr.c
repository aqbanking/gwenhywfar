/***************************************************************************
 begin       : Mon Aug 11 2008
 copyright   : (C) 2008,2017 by Martin Preuss
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

#define DISABLE_DEBUGLOG


#include "configmgr_p.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/url.h>
#include <gwenhywfar/debug.h>


#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>



GWEN_INHERIT_FUNCTIONS(GWEN_CONFIGMGR)

GWEN_INHERIT(GWEN_PLUGIN, GWEN_CONFIGMGR_PLUGIN)



int GWEN_ConfigMgr_ModuleInit(void)
{
  GWEN_PLUGIN_MANAGER *pm;
  int err;
  GWEN_STRINGLIST *sl;

  pm=GWEN_PluginManager_new(GWEN_CONFIGMGR_PLUGIN_NAME, GWEN_PM_LIBNAME);
  err=GWEN_PluginManager_Register(pm);
  if (err) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not register ConfigMgr plugin manager");
    return err;
  }

  /* create plugin paths */
  sl=GWEN_PathManager_GetPaths(GWEN_PM_LIBNAME, GWEN_PM_PLUGINDIR);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;
    GWEN_BUFFER *pbuf;

    pbuf=GWEN_Buffer_new(0, 256, 0, 1);

    se=GWEN_StringList_FirstEntry(sl);
    while (se) {
      GWEN_Buffer_AppendString(pbuf, GWEN_StringListEntry_Data(se));
      GWEN_Buffer_AppendString(pbuf, GWEN_DIR_SEPARATOR_S GWEN_CONFIGMGR_FOLDER);
      DBG_INFO(GWEN_LOGDOMAIN, "Adding plugin path [%s]",
               GWEN_Buffer_GetStart(pbuf));
      GWEN_PluginManager_AddPath(pm, GWEN_PM_LIBNAME,
                                 GWEN_Buffer_GetStart(pbuf));
      GWEN_Buffer_Reset(pbuf);
      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_Buffer_free(pbuf);
    GWEN_StringList_free(sl);
  }

  return 0;
}



int GWEN_ConfigMgr_ModuleFini(void)
{
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_FindPluginManager(GWEN_CONFIGMGR_PLUGIN_NAME);
  if (pm) {
    int rv;

    rv=GWEN_PluginManager_Unregister(pm);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not unregister ConfigMgr plugin manager (%d)", rv);
      return rv;
    }
    else
      GWEN_PluginManager_free(pm);
  }

  return 0;
}




GWEN_CONFIGMGR *GWEN_ConfigMgr_new(const char *url)
{
  GWEN_CONFIGMGR *mgr;

  GWEN_NEW_OBJECT(GWEN_CONFIGMGR, mgr);
  GWEN_INHERIT_INIT(GWEN_CONFIGMGR, mgr);

  if (url)
    mgr->url=strdup(url);

  return mgr;
}



void GWEN_ConfigMgr_free(GWEN_CONFIGMGR *mgr)
{
  if (mgr) {
    GWEN_INHERIT_FINI(GWEN_CONFIGMGR, mgr);
    free(mgr->url);
    GWEN_FREE_OBJECT(mgr);
  }
}



GWEN_CONFIGMGR_GETGROUP_FN GWEN_ConfigMgr_SetGetGroupFn(GWEN_CONFIGMGR *mgr,
                                                        GWEN_CONFIGMGR_GETGROUP_FN f)
{
  GWEN_CONFIGMGR_GETGROUP_FN of;

  assert(mgr);
  of=mgr->getGroupFn;
  mgr->getGroupFn=f;

  return of;
}



GWEN_CONFIGMGR_SETGROUP_FN GWEN_ConfigMgr_SetSetGroupFn(GWEN_CONFIGMGR *mgr,
                                                        GWEN_CONFIGMGR_SETGROUP_FN f)
{
  GWEN_CONFIGMGR_SETGROUP_FN of;

  assert(mgr);
  of=mgr->setGroupFn;
  mgr->setGroupFn=f;

  return of;
}



GWEN_CONFIGMGR_HASGROUP_FN GWEN_ConfigMgr_SetHasGroupFn(GWEN_CONFIGMGR *mgr,
                                                        GWEN_CONFIGMGR_HASGROUP_FN f)
{
  GWEN_CONFIGMGR_HASGROUP_FN of;

  assert(mgr);
  of=mgr->hasGroupFn;
  mgr->hasGroupFn=f;

  return of;

}



GWEN_CONFIGMGR_LOCKGROUP_FN GWEN_ConfigMgr_SetLockGroupFn(GWEN_CONFIGMGR *mgr,
                                                          GWEN_CONFIGMGR_LOCKGROUP_FN f)
{
  GWEN_CONFIGMGR_LOCKGROUP_FN of;

  assert(mgr);
  of=mgr->lockGroupFn;
  mgr->lockGroupFn=f;

  return of;
}



GWEN_CONFIGMGR_UNLOCKGROUP_FN GWEN_ConfigMgr_SetUnlockGroupFn(GWEN_CONFIGMGR *mgr,
                                                              GWEN_CONFIGMGR_UNLOCKGROUP_FN f)
{
  GWEN_CONFIGMGR_UNLOCKGROUP_FN of;

  assert(mgr);
  of=mgr->unlockGroupFn;
  mgr->unlockGroupFn=f;

  return of;
}



GWEN_CONFIGMGR_GETUNIQUEID_FN GWEN_ConfigMgr_SetGetUniqueIdFn(GWEN_CONFIGMGR *mgr,
                                                              GWEN_CONFIGMGR_GETUNIQUEID_FN f)
{
  GWEN_CONFIGMGR_GETUNIQUEID_FN of;

  assert(mgr);
  of=mgr->getUniqueIdFn;
  mgr->getUniqueIdFn=f;

  return of;
}



GWEN_CONFIGMGR_MKUNIQUEIDFROMID_FN GWEN_ConfigMgr_SetMkUniqueIdFromIdFn(GWEN_CONFIGMGR *mgr,
                                                                        GWEN_CONFIGMGR_MKUNIQUEIDFROMID_FN f)
{
  GWEN_CONFIGMGR_MKUNIQUEIDFROMID_FN of;

  assert(mgr);
  of=mgr->mkUniqueIdFromIdFn;
  mgr->mkUniqueIdFromIdFn=f;

  return of;
}



GWEN_CONFIGMGR_DELETEGROUP_FN GWEN_ConfigMgr_SetDeleteGroupFn(GWEN_CONFIGMGR *mgr,
                                                              GWEN_CONFIGMGR_DELETEGROUP_FN f)
{
  GWEN_CONFIGMGR_DELETEGROUP_FN of;

  assert(mgr);
  of=mgr->deleteGroupFn;
  mgr->deleteGroupFn=f;

  return of;
}



GWEN_CONFIGMGR_LISTGROUPS_FN GWEN_ConfigMgr_SetListGroupsFn(GWEN_CONFIGMGR *mgr,
                                                            GWEN_CONFIGMGR_LISTGROUPS_FN f)
{
  GWEN_CONFIGMGR_LISTGROUPS_FN of;

  assert(mgr);
  of=mgr->listGroupsFn;
  mgr->listGroupsFn=f;

  return of;
}



GWEN_CONFIGMGR_LISTSUBGROUPS_FN GWEN_ConfigMgr_SetListSubGroupsFn(GWEN_CONFIGMGR *mgr,
                                                                  GWEN_CONFIGMGR_LISTSUBGROUPS_FN f)
{
  GWEN_CONFIGMGR_LISTSUBGROUPS_FN of;

  assert(mgr);
  of=mgr->listSubGroupsFn;
  mgr->listSubGroupsFn=f;

  return of;
}



int GWEN_ConfigMgr_GetGroup(GWEN_CONFIGMGR *mgr,
                            const char *groupName,
                            const char *subGroupName,
                            GWEN_DB_NODE **pDb)
{
  assert(mgr);
  if (mgr->getGroupFn)
    return mgr->getGroupFn(mgr, groupName, subGroupName, pDb);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_ConfigMgr_SetGroup(GWEN_CONFIGMGR *mgr,
                            const char *groupName,
                            const char *subGroupName,
                            GWEN_DB_NODE *db)
{
  assert(mgr);
  if (mgr->setGroupFn)
    return mgr->setGroupFn(mgr, groupName, subGroupName, db);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_ConfigMgr_HasGroup(GWEN_CONFIGMGR *mgr,
                            const char *groupName,
                            const char *subGroupName)
{
  assert(mgr);
  if (mgr->hasGroupFn)
    return mgr->hasGroupFn(mgr, groupName, subGroupName);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_ConfigMgr_LockGroup(GWEN_CONFIGMGR *mgr,
                             const char *groupName,
                             const char *subGroupName)
{
  assert(mgr);
  if (mgr->lockGroupFn)
    return mgr->lockGroupFn(mgr, groupName, subGroupName);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_ConfigMgr_UnlockGroup(GWEN_CONFIGMGR *mgr,
                               const char *groupName,
                               const char *subGroupName)
{
  assert(mgr);
  if (mgr->unlockGroupFn)
    return mgr->unlockGroupFn(mgr, groupName, subGroupName);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_ConfigMgr_GetUniqueId(GWEN_CONFIGMGR *mgr,
                               const char *groupName,
                               char *buffer,
                               uint32_t bufferLen)
{
  assert(mgr);
  if (mgr->getUniqueIdFn)
    return mgr->getUniqueIdFn(mgr, groupName, buffer, bufferLen);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_ConfigMgr_MkUniqueIdFromId(GWEN_CONFIGMGR *mgr,
                                    const char *groupName,
                                    uint32_t uid,
                                    int doCheck,
                                    char *buffer,
                                    uint32_t bufferLen)
{
  assert(mgr);
  if (mgr->mkUniqueIdFromIdFn)
    return mgr->mkUniqueIdFromIdFn(mgr, groupName, uid, doCheck, buffer, bufferLen);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_ConfigMgr_DeleteGroup(GWEN_CONFIGMGR *mgr,
                               const char *groupName,
                               const char *subGroupName)
{
  assert(mgr);
  if (mgr->deleteGroupFn)
    return mgr->deleteGroupFn(mgr, groupName, subGroupName);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_ConfigMgr_ListGroups(GWEN_CONFIGMGR *mgr,
                              GWEN_STRINGLIST *sl)
{
  assert(mgr);
  if (mgr->listGroupsFn)
    return mgr->listGroupsFn(mgr, sl);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}


int GWEN_ConfigMgr_ListSubGroups(GWEN_CONFIGMGR *mgr,
                                 const char *groupName,
                                 GWEN_STRINGLIST *sl)
{
  assert(mgr);
  if (mgr->listSubGroupsFn)
    return mgr->listSubGroupsFn(mgr, groupName, sl);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}









GWEN_PLUGIN *GWEN_ConfigMgr_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
                                       const char *name,
                                       const char *fileName)
{
  GWEN_PLUGIN *pl;
  GWEN_CONFIGMGR_PLUGIN *xpl;

  pl=GWEN_Plugin_new(pm, name, fileName);
  GWEN_NEW_OBJECT(GWEN_CONFIGMGR_PLUGIN, xpl);
  GWEN_INHERIT_SETDATA(GWEN_PLUGIN, GWEN_CONFIGMGR_PLUGIN, pl, xpl,
                       GWEN_ConfigMgr_Plugin_FreeData);

  return pl;
}



void GWENHYWFAR_CB GWEN_ConfigMgr_Plugin_FreeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_CONFIGMGR_PLUGIN *xpl;

  xpl=(GWEN_CONFIGMGR_PLUGIN *)p;
  GWEN_FREE_OBJECT(xpl);
}



void GWEN_ConfigMgr_Plugin_SetFactoryFn(GWEN_PLUGIN *pl,
                                        GWEN_CONFIGMGR_PLUGIN_FACTORYFN f)
{
  GWEN_CONFIGMGR_PLUGIN *xpl;

  assert(pl);
  xpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_CONFIGMGR_PLUGIN, pl);
  assert(xpl);

  xpl->factoryFn=f;
}



GWEN_CONFIGMGR *GWEN_ConfigMgr_Plugin_Factory(GWEN_PLUGIN *pl,
                                              const char *url)
{
  GWEN_CONFIGMGR_PLUGIN *xpl;

  assert(pl);
  xpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_CONFIGMGR_PLUGIN, pl);
  assert(xpl);

  assert(xpl->factoryFn);
  return xpl->factoryFn(pl, url);
}



GWEN_CONFIGMGR *GWEN_ConfigMgr_Factory(const char *url)
{
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CONFIGMGR *mgr=NULL;
  GWEN_URL *purl;
  const char *modname;

  pm=GWEN_PluginManager_FindPluginManager(GWEN_CONFIGMGR_PLUGIN_NAME);
  if (!pm) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No plugin manager for \"ConfigMgr\" found");
    return 0;
  }

  purl=GWEN_Url_fromString(url);
  if (purl==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "Invalid url [%s]", url);
    return NULL;
  }

  modname=GWEN_Url_GetProtocol(purl);
  if (modname==NULL)
    modname="file";

  pl=GWEN_PluginManager_GetPlugin(pm, modname);
  if (!pl) {
    DBG_INFO(GWEN_LOGDOMAIN, "ConfigMgr-Plugin \"%s\" not found", modname);
    GWEN_Url_free(purl);
    return 0;
  }
  GWEN_Url_free(purl);

  mgr=GWEN_ConfigMgr_Plugin_Factory(pl, url);
  if (mgr==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Plugin did not create a GWEN_CONFIGMGR");
  }

  return mgr;
}









