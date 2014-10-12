/***************************************************************************
 begin       : Wed Mar 31 2004
 copyright   : (C) 2004-2010 by Martin Preuss
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

#include "dbio_p.h"
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/path.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/syncio.h>
#include <gwenhywfar/syncio_file.h>
#include <gwenhywfar/syncio_memory.h>
#include <gwenhywfar/fslock.h>
#include <gwenhywfar/pathmanager.h>


/* TODO: #include <gwenhywfar/plugin.h> */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif


#ifdef OS_WIN32
# define DIRSEP "\\"
# include <windows.h>
#else
# define DIRSEP "/"
#endif



GWEN_LIST_FUNCTIONS(GWEN_DBIO, GWEN_DBIO)
GWEN_INHERIT_FUNCTIONS(GWEN_DBIO)

GWEN_INHERIT(GWEN_PLUGIN, GWEN_DBIO_PLUGIN)


int GWEN_DBIO_ModuleInit(void) {
  GWEN_PLUGIN_MANAGER *pm;
  int err;
  GWEN_STRINGLIST *sl;

  pm=GWEN_PluginManager_new("dbio", GWEN_PM_LIBNAME);
  err=GWEN_PluginManager_Register(pm);
  if (err) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not register DBIO plugin manager");
    return err;
  }

  /* create plugin paths */
  sl=GWEN_PathManager_GetPaths(GWEN_PM_LIBNAME, GWEN_PM_PLUGINDIR);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;
    GWEN_BUFFER *pbuf;

    pbuf=GWEN_Buffer_new(0, 256, 0, 1);

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      GWEN_Buffer_AppendString(pbuf, GWEN_StringListEntry_Data(se));
      GWEN_Buffer_AppendString(pbuf, DIRSEP GWEN_DBIO_FOLDER);
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



int GWEN_DBIO_ModuleFini(void) {
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_FindPluginManager("dbio");
  if (pm) {
    int rv;

    rv=GWEN_PluginManager_Unregister(pm);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not unregister DBIO plugin manager (%d)", rv);
      return rv;
    }
    else
      GWEN_PluginManager_free(pm);
  }

  return 0;
}





GWEN_PLUGIN *GWEN_DBIO_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
                                  const char *name,
                                  const char *fileName) {
  GWEN_PLUGIN *pl;
  GWEN_DBIO_PLUGIN *pldbio;

  pl=GWEN_Plugin_new(pm, name, fileName);
  GWEN_NEW_OBJECT(GWEN_DBIO_PLUGIN, pldbio);
  GWEN_INHERIT_SETDATA(GWEN_PLUGIN, GWEN_DBIO_PLUGIN, pl, pldbio,
                       GWEN_DBIO_Plugin_FreeData);

  return pl;
}



void GWENHYWFAR_CB GWEN_DBIO_Plugin_FreeData(GWEN_UNUSED void *bp, void *p) {
  GWEN_DBIO_PLUGIN *pldbio;

  pldbio=(GWEN_DBIO_PLUGIN*)p;
  GWEN_FREE_OBJECT(pldbio);
}



GWEN_DBIO *GWEN_DBIO_Plugin_Factory(GWEN_PLUGIN *pl) {
  GWEN_DBIO_PLUGIN *pldbio;

  assert(pl);
  pldbio=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_DBIO_PLUGIN, pl);
  assert(pldbio);

  assert(pldbio->factoryFn);
  return pldbio->factoryFn(pl);
}



void GWEN_DBIO_Plugin_SetFactoryFn(GWEN_PLUGIN *pl,
                                   GWEN_DBIO_PLUGIN_FACTORYFN f) {
  GWEN_DBIO_PLUGIN *pldbio;

  assert(pl);
  pldbio=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_DBIO_PLUGIN, pl);
  assert(pldbio);

  pldbio->factoryFn=f;
}








GWEN_DBIO *GWEN_DBIO_new(const char *name, const char *descr) {
  GWEN_DBIO *dbio;

  assert(name);
  GWEN_NEW_OBJECT(GWEN_DBIO, dbio);
  GWEN_LIST_INIT(GWEN_DBIO, dbio);
  GWEN_INHERIT_INIT(GWEN_DBIO, dbio);
  dbio->name=strdup(name);
  if (descr)
    dbio->descr=strdup(descr);

  dbio->usage=1;
  return dbio;
}



void GWEN_DBIO_free(GWEN_DBIO *dbio) {
  if (dbio) {
    assert(dbio->usage);
    if (--(dbio->usage)==0) {
      GWEN_INHERIT_FINI(GWEN_DBIO, dbio);
      GWEN_LIST_FINI(GWEN_DBIO, dbio);

      free(dbio->name);
      free(dbio->descr);

      GWEN_FREE_OBJECT(dbio);
    }
  }
}



void GWEN_DBIO_Attach(GWEN_DBIO *dbio) {
  assert(dbio);
  dbio->usage++;
}



int GWEN_DBIO_Import(GWEN_DBIO *dbio,
                     GWEN_SYNCIO *sio,
                     GWEN_DB_NODE *db,
                     GWEN_DB_NODE *params,
                     uint32_t flags) {
  assert(dbio);
  assert(sio);
  assert(db);

  if (GWEN_SyncIo_GetStatus(sio) != GWEN_SyncIo_Status_Connected) {
    DBG_ERROR(GWEN_LOGDOMAIN, "GWEN_SYNCIO %s not connected; did you forget to call GWEN_SyncIo_Connect()?", GWEN_SyncIo_GetTypeName(sio));
    return -1;
  }

  if (dbio->importFn)
    return dbio->importFn(dbio, sio, db, params, flags);
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No import function set");
    return -1;
  }
}



int GWEN_DBIO_Export(GWEN_DBIO *dbio,
                     GWEN_SYNCIO *sio,
                     GWEN_DB_NODE *db,
                     GWEN_DB_NODE *params,
                     uint32_t flags) {
  assert(dbio);
  assert(sio);
  assert(db);

  if (dbio->exportFn)
    return dbio->exportFn(dbio, sio, db, params, flags);
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No export function set");
    return -1;
  }
}



GWEN_DBIO_CHECKFILE_RESULT GWEN_DBIO_CheckFile(GWEN_DBIO *dbio,
    const char *fname) {
  assert(dbio);
  assert(fname);

  if (dbio->checkFileFn)
    return dbio->checkFileFn(dbio, fname);
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No checkFile function set");
    return GWEN_DBIO_CheckFileResultUnknown;
  }
}



const char *GWEN_DBIO_GetName(const GWEN_DBIO *dbio) {
  assert(dbio);
  return dbio->name;
}



const char *GWEN_DBIO_GetDescription(const GWEN_DBIO *dbio) {
  assert(dbio);
  return dbio->descr;
}



void GWEN_DBIO_SetImportFn(GWEN_DBIO *dbio, GWEN_DBIO_IMPORTFN f) {
  assert(dbio);
  dbio->importFn=f;
}



void GWEN_DBIO_SetExportFn(GWEN_DBIO *dbio, GWEN_DBIO_EXPORTFN f) {
  assert(dbio);
  dbio->exportFn=f;
}


void GWEN_DBIO_SetCheckFileFn(GWEN_DBIO *dbio, GWEN_DBIO_CHECKFILEFN f) {
  assert(dbio);
  dbio->checkFileFn=f;
}



GWEN_DBIO *GWEN_DBIO_GetPlugin(const char *modname) {
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_DBIO *dbio;

  pm=GWEN_PluginManager_FindPluginManager("dbio");
  if (!pm) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No plugin manager for \"dbio\" found");
    return 0;
  }

  pl=GWEN_PluginManager_GetPlugin(pm, modname);
  if (!pl) {
    DBG_INFO(GWEN_LOGDOMAIN, "DBIO-Plugin \"%s\" not found", modname);
    return 0;
  }

  dbio=GWEN_DBIO_Plugin_Factory(pl);
  if (!dbio) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Plugin did not create a GWEN_DBIO");
  }
  return dbio;
}



int GWEN_DBIO_ExportToFile(GWEN_DBIO *dbio,
                           const char *fname,
                           GWEN_DB_NODE *db,
                           GWEN_DB_NODE *params,
                           uint32_t dbflags) {
  int rv;
  GWEN_FSLOCK *lck=0;
  GWEN_SYNCIO *sio;

  /* if locking requested */
  if (dbflags & GWEN_DB_FLAGS_LOCKFILE) {
    GWEN_FSLOCK_RESULT res;

    lck=GWEN_FSLock_new(fname, GWEN_FSLock_TypeFile);
    assert(lck);
    res=GWEN_FSLock_Lock(lck, GWEN_DB_DEFAULT_LOCK_TIMEOUT, 0);
    if (res!=GWEN_FSLock_ResultOk) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not apply lock to file \"%s\" (%d)",
                fname, res);
      GWEN_FSLock_free(lck);
      return -1;
    }
  }

  /* open file */
  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_CreateAlways);
  if (dbflags & GWEN_DB_FLAGS_APPEND_FILE)
    GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_APPEND);
  GWEN_SyncIo_AddFlags(sio,
                       GWEN_SYNCIO_FILE_FLAGS_READ |
                       GWEN_SYNCIO_FILE_FLAGS_WRITE |
                       GWEN_SYNCIO_FILE_FLAGS_UREAD |
                       GWEN_SYNCIO_FILE_FLAGS_UWRITE);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    if (lck) {
      GWEN_FSLock_Unlock(lck);
      GWEN_FSLock_free(lck);
    }
    return rv;
  }

  rv=GWEN_DBIO_Export(dbio, sio, db, params, dbflags);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    if (lck) {
      GWEN_FSLock_Unlock(lck);
      GWEN_FSLock_free(lck);
    }
    return rv;
  }

  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    if (lck) {
      GWEN_FSLock_Unlock(lck);
      GWEN_FSLock_free(lck);
    }
    return rv;
  }
  GWEN_SyncIo_free(sio);

  /* remove lock, if any */
  if (lck) {
    GWEN_FSLOCK_RESULT res;

    res=GWEN_FSLock_Unlock(lck);
    if (res!=GWEN_FSLock_ResultOk) {
      DBG_WARN(GWEN_LOGDOMAIN,
               "Could not remove lock on file \"%s\" (%d)",
               fname, res);
    }
    GWEN_FSLock_free(lck);
  }

  return 0;
}



int GWEN_DBIO_ExportToBuffer(GWEN_DBIO *dbio,
                             GWEN_BUFFER *buf,
                             GWEN_DB_NODE *db,
                             GWEN_DB_NODE *params,
                             uint32_t flags) {
  GWEN_SYNCIO *sio;
  int rv;

  /* create SyncIO, don't take over buf */
  sio=GWEN_SyncIo_Memory_new(buf, 0);
  rv=GWEN_DBIO_Export(dbio, sio, db, params, flags);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  GWEN_SyncIo_free(sio);
  return 0;
}



int GWEN_DBIO_ImportFromFile(GWEN_DBIO *dbio,
                             const char *fname,
                             GWEN_DB_NODE *db,
                             GWEN_DB_NODE *params,
                             uint32_t dbflags) {
  GWEN_SYNCIO *sio;
  int rv;
  GWEN_FSLOCK *lck=0;

  /* if locking requested */
  if (dbflags & GWEN_DB_FLAGS_LOCKFILE) {
    GWEN_FSLOCK_RESULT res;

    lck=GWEN_FSLock_new(fname, GWEN_FSLock_TypeFile);
    assert(lck);
    res=GWEN_FSLock_Lock(lck, GWEN_DB_DEFAULT_LOCK_TIMEOUT, 0);
    if (res!=GWEN_FSLock_ResultOk) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not apply lock to file \"%s\" (%d)",
                fname, res);
      GWEN_FSLock_free(lck);
      return GWEN_ERROR_IO;
    }
  }

  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    if (lck) {
      GWEN_FSLock_Unlock(lck);
      GWEN_FSLock_free(lck);
    }
    return rv;
  }

  /* read from file */
  rv=GWEN_DBIO_Import(dbio, sio, db, params, dbflags);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    if (lck) {
      GWEN_FSLock_Unlock(lck);
      GWEN_FSLock_free(lck);
    }
    return rv;
  }

  /* close io layer */
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  /* remove lock, if any */
  if (lck) {
    GWEN_FSLOCK_RESULT res;

    res=GWEN_FSLock_Unlock(lck);
    if (res!=GWEN_FSLock_ResultOk) {
      DBG_WARN(GWEN_LOGDOMAIN,
               "Could not remove lock on file \"%s\" (%d)",
               fname, res);
    }
    GWEN_FSLock_free(lck);
  }

  return 0;
}














