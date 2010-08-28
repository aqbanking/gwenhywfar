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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "cfgdir_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/urlfns.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>



GWEN_INHERIT(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR)



GWEN_CONFIGMGR *GWEN_ConfigMgrDir_Factory(GWEN_UNUSED GWEN_PLUGIN *pl, const char *url) {
  GWEN_CONFIGMGR *cfg;

  cfg=GWEN_ConfigMgrDir_new(url);
  GWEN_ConfigMgr_SetGetGroupFn(cfg, GWEN_ConfigMgrDir_GetGroup);
  GWEN_ConfigMgr_SetSetGroupFn(cfg, GWEN_ConfigMgrDir_SetGroup);
  GWEN_ConfigMgr_SetLockGroupFn(cfg, GWEN_ConfigMgrDir_LockGroup);
  GWEN_ConfigMgr_SetUnlockGroupFn(cfg, GWEN_ConfigMgrDir_UnlockGroup);
  GWEN_ConfigMgr_SetGetUniqueIdFn(cfg, GWEN_ConfigMgrDir_GetUniqueId);
  GWEN_ConfigMgr_SetDeleteGroupFn(cfg, GWEN_ConfigMgrDir_DeleteGroup);
  GWEN_ConfigMgr_SetListGroupsFn(cfg, GWEN_ConfigMgrDir_ListGroups);
  GWEN_ConfigMgr_SetListSubGroupsFn(cfg, GWEN_ConfigMgrDir_ListSubGroups);

  return cfg;
}



GWEN_PLUGIN *configmgr_dir_factory(GWEN_PLUGIN_MANAGER *pm,
				   const char *modName,
				   const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=GWEN_ConfigMgr_Plugin_new(pm, modName, fileName);
  assert(pl);

  GWEN_ConfigMgr_Plugin_SetFactoryFn(pl, GWEN_ConfigMgrDir_Factory);

  return pl;

}



GWEN_CONFIGMGR *GWEN_ConfigMgrDir_new(const char *url) {
  GWEN_CONFIGMGR *cfg;
  GWEN_CONFIGMGR_DIR *xcfg;
  GWEN_URL *gurl;

  cfg=GWEN_ConfigMgr_new(url);
  GWEN_NEW_OBJECT(GWEN_CONFIGMGR_DIR, xcfg);
  GWEN_INHERIT_SETDATA(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR, cfg, xcfg,
                       GWEN_ConfigMgrDir_FreeData);
  xcfg->fileLocks=GWEN_FSLock_List_new();

  gurl=GWEN_Url_fromString(url);
  if (gurl) {
    const char *s;
    GWEN_BUFFER *nbuf;

    nbuf=GWEN_Buffer_new(0, 256, 0, 1);
    s=GWEN_Url_GetServer(gurl);
    if (s)
      GWEN_Buffer_AppendString(nbuf, s);
    s=GWEN_Url_GetPath(gurl);
    if (s) {
      /* Does this begin with a slash? If not, we add one, but only if
	 this isn't a drive letter (for windows!) */
      if (*s!='/' && *s!='\\'
	  && !(isalpha(s[0]) && s[1] == ':'
	       && (s[2] == '/' || s[2] == '\\'))) {
	GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
      }
      GWEN_Buffer_AppendString(nbuf, s);
    }
    xcfg->folder=strdup(GWEN_Buffer_GetStart(nbuf));
    GWEN_Url_free(gurl);
    GWEN_Buffer_free(nbuf);
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad URL [%s]", url);
    GWEN_ConfigMgr_free(cfg);
    return NULL;
  }

  return cfg;
}



GWENHYWFAR_CB void GWEN_ConfigMgrDir_FreeData(GWEN_UNUSED void *bp, void *p) {
  GWEN_CONFIGMGR_DIR *xcfg;

  xcfg=(GWEN_CONFIGMGR_DIR*) p;

  if (GWEN_FSLock_List_GetCount(xcfg->fileLocks)) {
    GWEN_FSLOCK *lck;

    DBG_WARN(GWEN_LOGDOMAIN, "Some groups are still locked, unlocking");
    lck=GWEN_FSLock_List_First(xcfg->fileLocks);
    while(lck) {
      GWEN_FSLOCK_RESULT res;

      res=GWEN_FSLock_Unlock(lck);
      if (res!=GWEN_FSLock_ResultOk) {
	DBG_INFO(GWEN_LOGDOMAIN, "Unable to unlock file [%s]: %d",
                 GWEN_FSLock_GetName(lck), res);
      }
      lck=GWEN_FSLock_List_Next(lck);
    }
  }
  GWEN_FSLock_List_free(xcfg->fileLocks);

  free(xcfg->folder);
  GWEN_FREE_OBJECT(xcfg);
}



int GWEN_ConfigMgrDir__GetUniqueId(GWEN_CONFIGMGR *cfg,
				   const char *groupName,
				   uint32_t *pUniqueId){
  GWEN_CONFIGMGR_DIR *xcfg;
  GWEN_BUFFER *nbuf;
  uint32_t uniqueId=0;
  GWEN_FSLOCK *lck;
  GWEN_FSLOCK_RESULT res;
  FILE *f;
  int rv;

  assert(cfg);
  xcfg=GWEN_INHERIT_GETDATA(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR, cfg);
  assert(xcfg);

  assert(xcfg->folder);
  assert(groupName);

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nbuf, xcfg->folder);
  GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Text_EscapeToBuffer(groupName, nbuf);
  GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(nbuf, "uniqueid");

  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(nbuf),
			    GWEN_PATH_FLAGS_CHECKROOT |
			    GWEN_PATH_FLAGS_VARIABLE);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  lck=GWEN_FSLock_new(GWEN_Buffer_GetStart(nbuf), GWEN_FSLock_TypeFile);
  res=GWEN_FSLock_Lock(lck, 60000, 0);
  if (res!=GWEN_FSLock_ResultOk) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Could not lock group [%s]: %d",
	      groupName, res);
    GWEN_FSLock_free(lck);
    GWEN_Buffer_free(nbuf);
    return GWEN_ERROR_LOCK;
  }

  /* read last id */
  f=fopen(GWEN_Buffer_GetStart(nbuf), "r");
  if (f) {
    int i;

    if (1!=fscanf(f, "%d", &i))
      i=0;
    uniqueId=i;
    fclose(f);
  }

  uniqueId++;

  f=fopen(GWEN_Buffer_GetStart(nbuf), "w");
  if (f==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(%s, \"w\"): %s",
	      GWEN_Buffer_GetStart(nbuf),
	      strerror(errno));
    GWEN_FSLock_Unlock(lck);
    GWEN_FSLock_free(lck);
    GWEN_Buffer_free(nbuf);
    return GWEN_ERROR_LOCK;
  }
  fprintf(f, "%d", (int)uniqueId);
  if (fclose(f)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(%s, \"w\"): %s",
	      GWEN_Buffer_GetStart(nbuf),
	      strerror(errno));
    GWEN_FSLock_Unlock(lck);
    GWEN_FSLock_free(lck);
    GWEN_Buffer_free(nbuf);
    return GWEN_ERROR_LOCK;
  }

  GWEN_FSLock_Unlock(lck);
  GWEN_FSLock_free(lck);
  GWEN_Buffer_free(nbuf);

  *pUniqueId=uniqueId;
  return 0;
}



void GWEN_ConfigMgrDir_AddGroupDirName(GWEN_CONFIGMGR *cfg,
				       const char *groupName,
				       GWEN_BUFFER *nbuf) {
  GWEN_CONFIGMGR_DIR *xcfg;
  uint32_t pos;
  char *p;

  assert(cfg);
  xcfg=GWEN_INHERIT_GETDATA(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR, cfg);
  assert(xcfg);

  assert(xcfg->folder);
  assert(groupName);

  GWEN_Buffer_AppendString(nbuf, xcfg->folder);
  GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  pos=GWEN_Buffer_GetPos(nbuf);
  GWEN_Text_EscapeToBuffer(groupName, nbuf);
  p=GWEN_Buffer_GetStart(nbuf)+pos;
  while(*p) {
    *p=tolower(*p);
    p++;
  }

}



void GWEN_ConfigMgrDir_AddGroupFileName(GWEN_CONFIGMGR *cfg,
					const char *groupName,
					const char *subGroupName,
					GWEN_BUFFER *nbuf) {
  GWEN_CONFIGMGR_DIR *xcfg;
  uint32_t pos;
  char *p;

  assert(cfg);
  xcfg=GWEN_INHERIT_GETDATA(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR, cfg);
  assert(xcfg);

  assert(xcfg->folder);
  assert(groupName);
  assert(subGroupName);

  GWEN_Buffer_AppendString(nbuf, xcfg->folder);
  pos=GWEN_Buffer_GetPos(nbuf);

  GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Text_EscapeToBuffer(groupName, nbuf);
  GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Text_EscapeToBuffer(subGroupName, nbuf);
  GWEN_Buffer_AppendString(nbuf, ".conf");

  p=GWEN_Buffer_GetStart(nbuf)+pos;
  while(*p) {
    *p=tolower(*p);
    p++;
  }
}



GWEN_FSLOCK *GWEN_ConfigMgrDir_FindLock(GWEN_CONFIGMGR *cfg, const char *fname) {
  GWEN_CONFIGMGR_DIR *xcfg;

  assert(cfg);
  xcfg=GWEN_INHERIT_GETDATA(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR, cfg);
  assert(xcfg);

  if (GWEN_FSLock_List_GetCount(xcfg->fileLocks)) {
    GWEN_FSLOCK *lck;

    lck=GWEN_FSLock_List_First(xcfg->fileLocks);
    while(lck) {
      if (strcasecmp(GWEN_FSLock_GetName(lck), fname)==0)
        return lck;
      lck=GWEN_FSLock_List_Next(lck);
    }
  }

  return NULL;
}



int GWEN_ConfigMgrDir_GetGroup(GWEN_CONFIGMGR *cfg,
			       const char *groupName,
			       const char *subGroupName,
			       GWEN_DB_NODE **pDb) {
  GWEN_BUFFER *nbuf;
  GWEN_CONFIGMGR_DIR *xcfg;
  int rv;
  GWEN_DB_NODE *db;

  assert(cfg);
  xcfg=GWEN_INHERIT_GETDATA(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR, cfg);
  assert(xcfg);

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_ConfigMgrDir_AddGroupFileName(cfg, groupName, subGroupName, nbuf);

  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(nbuf),
			    GWEN_PATH_FLAGS_CHECKROOT |
			    GWEN_PATH_FLAGS_VARIABLE);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not access [%s]: %d",
	     GWEN_Buffer_GetStart(nbuf), rv);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  db=GWEN_DB_Group_new(subGroupName);
  rv=GWEN_DB_ReadFile(db,
		      GWEN_Buffer_GetStart(nbuf),
		      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_PATH_FLAGS_CREATE_GROUP |
		      GWEN_DB_FLAGS_ALLOW_EMPTY_STREAM);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not read [%s]: %d",
	     GWEN_Buffer_GetStart(nbuf), rv);
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  GWEN_Buffer_free(nbuf);
  *pDb=db;
  return 0;
}



int GWEN_ConfigMgrDir_SetGroup(GWEN_CONFIGMGR *cfg,
			       const char *groupName,
			       const char *subGroupName,
			       GWEN_DB_NODE *db) {
  GWEN_BUFFER *nbuf;
  GWEN_CONFIGMGR_DIR *xcfg;
  int rv;
  GWEN_FSLOCK *lck;

  assert(cfg);
  xcfg=GWEN_INHERIT_GETDATA(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR, cfg);
  assert(xcfg);

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_ConfigMgrDir_AddGroupFileName(cfg, groupName, subGroupName, nbuf);

  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(nbuf),
			    GWEN_PATH_FLAGS_CHECKROOT |
			    GWEN_PATH_FLAGS_VARIABLE);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not access [%s]: %d",
	     GWEN_Buffer_GetStart(nbuf), rv);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  lck=GWEN_ConfigMgrDir_FindLock(cfg, GWEN_Buffer_GetStart(nbuf));
  if (lck==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Group [%s/%s] is not locked",
	      groupName, subGroupName);
    GWEN_Buffer_free(nbuf);
    return GWEN_ERROR_LOCK;
  }

  rv=GWEN_DB_WriteFile(db,
		       GWEN_Buffer_GetStart(nbuf),
		       GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not write [%s]: %d",
	     GWEN_Buffer_GetStart(nbuf), rv);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  GWEN_Buffer_free(nbuf);
  return 0;
}



int GWEN_ConfigMgrDir_LockGroup(GWEN_CONFIGMGR *cfg,
				const char *groupName,
				const char *subGroupName) {
  GWEN_BUFFER *nbuf;
  GWEN_CONFIGMGR_DIR *xcfg;
  int rv;
  GWEN_FSLOCK *lck;
  GWEN_FSLOCK_RESULT res;

  assert(cfg);
  xcfg=GWEN_INHERIT_GETDATA(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR, cfg);
  assert(xcfg);

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_ConfigMgrDir_AddGroupFileName(cfg, groupName, subGroupName, nbuf);

  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(nbuf),
			    GWEN_PATH_FLAGS_CHECKROOT |
			    GWEN_PATH_FLAGS_VARIABLE);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not access [%s]: %d",
	     GWEN_Buffer_GetStart(nbuf), rv);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  lck=GWEN_ConfigMgrDir_FindLock(cfg, GWEN_Buffer_GetStart(nbuf));
  if (lck) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Group [%s/%s] is already locked",
	      groupName, subGroupName);
    GWEN_Buffer_free(nbuf);
    return GWEN_ERROR_LOCK;
  }

  lck=GWEN_FSLock_new(GWEN_Buffer_GetStart(nbuf), GWEN_FSLock_TypeFile);
  res=GWEN_FSLock_Lock(lck, 60000, 0);
  if (res!=GWEN_FSLock_ResultOk) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Could not lock group [%s/%s]: %d",
	      groupName, subGroupName, res);
    GWEN_FSLock_free(lck);
    GWEN_Buffer_free(nbuf);
    return GWEN_ERROR_LOCK;
  }

  GWEN_FSLock_List_Add(lck, xcfg->fileLocks);
  GWEN_Buffer_free(nbuf);
  return 0;
}



int GWEN_ConfigMgrDir_UnlockGroup(GWEN_CONFIGMGR *cfg,
				  const char *groupName,
				  const char *subGroupName) {
  GWEN_BUFFER *nbuf;
  GWEN_CONFIGMGR_DIR *xcfg;
  GWEN_FSLOCK *lck;
  GWEN_FSLOCK_RESULT res;

  assert(cfg);
  xcfg=GWEN_INHERIT_GETDATA(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR, cfg);
  assert(xcfg);

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_ConfigMgrDir_AddGroupFileName(cfg, groupName, subGroupName, nbuf);

  lck=GWEN_ConfigMgrDir_FindLock(cfg, GWEN_Buffer_GetStart(nbuf));
  if (lck==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Group [%s/%s] is not locked",
	      groupName, subGroupName);
    GWEN_Buffer_free(nbuf);
    return GWEN_ERROR_LOCK;
  }

  res=GWEN_FSLock_Unlock(lck);
  if (res!=GWEN_FSLock_ResultOk) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Could not unlock group [%s/%s]: %d",
	      groupName, subGroupName, res);
    GWEN_FSLock_List_Del(lck);
    GWEN_FSLock_free(lck);
    GWEN_Buffer_free(nbuf);
    return GWEN_ERROR_LOCK;
  }

  GWEN_FSLock_List_Del(lck);
  GWEN_FSLock_free(lck);
  GWEN_Buffer_free(nbuf);

  return 0;
}



int GWEN_ConfigMgrDir_GetUniqueId(GWEN_CONFIGMGR *cfg,
				  const char *groupName,
				  char *buffer,
				  uint32_t bufferLen) {
  GWEN_CONFIGMGR_DIR *xcfg;
  int rv;
  uint32_t uid;
  char ubuf[64];

  assert(cfg);
  xcfg=GWEN_INHERIT_GETDATA(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR, cfg);
  assert(xcfg);

  rv=GWEN_ConfigMgrDir__GetUniqueId(cfg, groupName, &uid);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not create unique id (%d)", rv);
    return rv;
  }
  snprintf(ubuf, sizeof(ubuf)-1, "uid::%08x", uid);
  ubuf[sizeof(ubuf)-1]=0;

  /* return new id */
  strncpy(buffer, ubuf, bufferLen-1);
  buffer[bufferLen-1]=0;

  return 0;
}



int GWEN_ConfigMgrDir_DeleteGroup(GWEN_CONFIGMGR *cfg,
				  const char *groupName,
				  const char *subGroupName) {
  GWEN_BUFFER *nbuf;
  GWEN_CONFIGMGR_DIR *xcfg;
  int rv;
  GWEN_FSLOCK *lck;
  GWEN_FSLOCK_RESULT res;

  assert(cfg);
  xcfg=GWEN_INHERIT_GETDATA(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR, cfg);
  assert(xcfg);

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_ConfigMgrDir_AddGroupFileName(cfg, groupName, subGroupName, nbuf);

  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(nbuf),
			    GWEN_PATH_FLAGS_CHECKROOT |
			    GWEN_PATH_FLAGS_VARIABLE);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not access [%s]: %d",
	     GWEN_Buffer_GetStart(nbuf), rv);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  lck=GWEN_ConfigMgrDir_FindLock(cfg, GWEN_Buffer_GetStart(nbuf));
  if (lck) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Group [%s/%s] is still locked",
	      groupName, subGroupName);
    GWEN_Buffer_free(nbuf);
    return GWEN_ERROR_LOCK;
  }

  lck=GWEN_FSLock_new(GWEN_Buffer_GetStart(nbuf), GWEN_FSLock_TypeFile);
  res=GWEN_FSLock_Lock(lck, 60000, 0);
  if (res!=GWEN_FSLock_ResultOk) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Could not lock group [%s/%s]: %d",
	      groupName, subGroupName, res);
    GWEN_FSLock_free(lck);
    GWEN_Buffer_free(nbuf);
    return GWEN_ERROR_LOCK;
  }

  unlink(GWEN_Buffer_GetStart(nbuf));

  GWEN_FSLock_Unlock(lck);
  GWEN_FSLock_free(lck);
  GWEN_Buffer_free(nbuf);

  return 0;
}



int GWEN_ConfigMgrDir_ListGroups(GWEN_CONFIGMGR *cfg,
				 GWEN_STRINGLIST *sl) {
  GWEN_CONFIGMGR_DIR *xcfg;
  int rv;

  assert(cfg);
  xcfg=GWEN_INHERIT_GETDATA(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR, cfg);
  assert(xcfg);

  assert(xcfg->folder);

  rv=GWEN_Directory_GetDirEntries(xcfg->folder, sl, NULL);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_ConfigMgrDir_ListSubGroups(GWEN_CONFIGMGR *cfg,
				    const char *groupName,
				    GWEN_STRINGLIST *sl) {
  GWEN_BUFFER *nbuf;
  GWEN_CONFIGMGR_DIR *xcfg;
  int rv;
  GWEN_STRINGLIST *rawsl;

  assert(cfg);
  xcfg=GWEN_INHERIT_GETDATA(GWEN_CONFIGMGR, GWEN_CONFIGMGR_DIR, cfg);
  assert(xcfg);

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_ConfigMgrDir_AddGroupDirName(cfg, groupName, nbuf);

  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(nbuf),
			    GWEN_PATH_FLAGS_CHECKROOT |
                            GWEN_PATH_FLAGS_PATHMUSTEXIST |
			    GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Path not found (%d)", rv);
    GWEN_Buffer_free(nbuf);
    return 0;
  }

  rawsl=GWEN_StringList_new();
  rv=GWEN_Directory_GetFileEntries(GWEN_Buffer_GetStart(nbuf), rawsl, "*.conf");
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_StringList_free(rawsl);
    GWEN_Buffer_free(nbuf);
    return rv;
  }
  else {
    GWEN_STRINGLISTENTRY *se;
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    se=GWEN_StringList_FirstEntry(rawsl);
    while(se) {
      char buffer[256];
      char *p;

      strncpy(buffer, GWEN_StringListEntry_Data(se), sizeof(buffer)-1);
      buffer[sizeof(buffer)-1]=0;
      p=strrchr(buffer, '.');
      if (p) {
	*p=0;
	GWEN_Text_UnescapeToBuffer(buffer, tbuf);
	GWEN_StringList_AppendString(sl, GWEN_Buffer_GetStart(tbuf), 0, 1);
        GWEN_Buffer_Reset(tbuf);
      }

      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_Buffer_free(tbuf);
    GWEN_StringList_free(rawsl);
    GWEN_Buffer_free(nbuf);
    return 0;
  }
}





