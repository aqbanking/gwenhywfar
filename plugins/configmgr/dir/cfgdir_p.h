/***************************************************************************
 begin       : Sat Sep 27 2008
 copyright   : (C) 2008 by Martin Preuss
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


#ifndef GWENHYWFAR_CFGMGR_CFGDIR_P_H
#define GWENHYWFAR_CFGMGR_CFGDIR_P_H

#include <gwenhywfar/configmgr_be.h>
#include <gwenhywfar/fslock.h>



typedef struct GWEN_CONFIGMGR_DIR GWEN_CONFIGMGR_DIR;
struct GWEN_CONFIGMGR_DIR {
  char *folder;
  GWEN_FSLOCK_LIST *fileLocks;
};



GWEN_CONFIGMGR *GWEN_ConfigMgrDir_Factory(GWEN_PLUGIN *pl, const char *url);

GWENHYWFAR_EXPORT
GWEN_PLUGIN *configmgr_dir_factory(GWEN_PLUGIN_MANAGER *pm,
                                   const char *modName,
                                   const char *fileName);




GWEN_CONFIGMGR *GWEN_ConfigMgrDir_new(const char *url);
GWENHYWFAR_CB void GWEN_ConfigMgrDir_FreeData(void *bp, void *p);


int GWEN_ConfigMgrDir_GetGroup(GWEN_CONFIGMGR *mgr,
                               const char *groupName,
                               const char *subGroupName,
                               GWEN_DB_NODE **pDb);

int GWEN_ConfigMgrDir_SetGroup(GWEN_CONFIGMGR *mgr,
                               const char *groupName,
                               const char *subGroupName,
                               GWEN_DB_NODE *db);

int GWEN_ConfigMgrDir_LockGroup(GWEN_CONFIGMGR *mgr,
                                const char *groupName,
                                const char *subGroupName);

int GWEN_ConfigMgrDir_UnlockGroup(GWEN_CONFIGMGR *mgr,
                                  const char *groupName,
                                  const char *subGroupName);

int GWEN_ConfigMgrDir_GetUniqueId(GWEN_CONFIGMGR *mgr,
                                  const char *groupName,
                                  char *buffer,
                                  uint32_t bufferLen);

int GWEN_ConfigMgrDir_MkUniqueIdFromId(GWEN_CONFIGMGR *cfg,
                                       const char *groupName,
                                       uint32_t uid,
                                       int doCheck,
                                       char *buffer,
                                       uint32_t bufferLen);


int GWEN_ConfigMgrDir_DeleteGroup(GWEN_CONFIGMGR *mgr,
                                  const char *groupName,
                                  const char *subGroupName);

int GWEN_ConfigMgrDir_ListGroups(GWEN_CONFIGMGR *mgr,
                                 GWEN_STRINGLIST *sl);

int GWEN_ConfigMgrDir_ListSubGroups(GWEN_CONFIGMGR *mgr,
                                    const char *groupName,
                                    GWEN_STRINGLIST *sl);


void GWEN_ConfigMgrDir_AddGroupDirName(GWEN_CONFIGMGR *cfg,
                                       const char *groupName,
                                       GWEN_BUFFER *nbuf);
void GWEN_ConfigMgrDir_AddGroupFileName(GWEN_CONFIGMGR *cfg,
                                        const char *groupName,
                                        const char *subGroupName,
                                        GWEN_BUFFER *nbuf);
GWEN_FSLOCK *GWEN_ConfigMgrDir_FindLock(GWEN_CONFIGMGR *cfg, const char *fname);


static int GWEN_ConfigMgrDir__GetUniqueId(GWEN_CONFIGMGR *cfg,
                                          const char *groupName,
                                          uint32_t *pUniqueId);

static int GWEN_ConfigMgrDir__UpdateLastUniqueId(GWEN_CONFIGMGR *cfg,
                                                 const char *groupName,
                                                 uint32_t uid);


#endif

