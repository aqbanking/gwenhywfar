/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Wed Mar 31 2004
 copyright   : (C) 2004 by Martin Preuss
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
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>

/* TODO: #include <gwenhywfar/plugin.h> */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#ifdef OS_WIN32
# define DIRSEP "\\"
# include <windows.h>
#else
# define DIRSEP "/"
#endif



GWEN_LIST_FUNCTIONS(GWEN_DBIO, GWEN_DBIO)
GWEN_INHERIT_FUNCTIONS(GWEN_DBIO)

GWEN_INHERIT(GWEN_PLUGIN, GWEN_DBIO_PLUGIN)


GWEN_ERRORCODE GWEN_DBIO_ModuleInit(){
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_BUFFER *pbuf;

  pm=GWEN_PluginManager_new("dbio");
  if (GWEN_PluginManager_Register(pm)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not register DBIO plugin manager");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_ERROR_TYPE_ERROR,
                          GWEN_ERROR_UNSPECIFIED);
  }
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);

  if (GWEN_GetPluginPath(pbuf)) {
    GWEN_Buffer_free(pbuf);
    GWEN_PluginManager_free(pm);
    return GWEN_Error_new(0,
			  GWEN_ERROR_SEVERITY_ERR,
			  GWEN_ERROR_TYPE_ERROR,
			  GWEN_ERROR_UNSPECIFIED);
  }

  GWEN_Buffer_AppendString(pbuf, DIRSEP GWEN_DBIO_FOLDER);
  GWEN_PluginManager_AddPath(pm, GWEN_Buffer_GetStart(pbuf));
  GWEN_Buffer_free(pbuf);

  return 0;
}



GWEN_ERRORCODE GWEN_DBIO_ModuleFini(){
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_FindPluginManager("dbio");
  if (pm) {
    int rv;

    rv=GWEN_PluginManager_Unregister(pm);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Could not unregister DBIO plugin manager (%d)", rv);
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



void GWENHYWFAR_CB GWEN_DBIO_Plugin_FreeData(void *bp, void *p) {
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








GWEN_DBIO *GWEN_DBIO_new(const char *name, const char *descr){
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



void GWEN_DBIO_free(GWEN_DBIO *dbio){
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



void GWEN_DBIO_Attach(GWEN_DBIO *dbio){
  assert(dbio);
  dbio->usage++;
}



int GWEN_DBIO_Import(GWEN_DBIO *dbio,
                     GWEN_BUFFEREDIO *bio,
                     GWEN_TYPE_UINT32 flags,
                     GWEN_DB_NODE *db,
                     GWEN_DB_NODE *params) {
  assert(dbio);
  assert(bio);
  assert(db);

  if (dbio->importFn)
    return dbio->importFn(dbio, bio, flags, db, params);
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No import function set");
    return -1;
  }
}



int GWEN_DBIO_Export(GWEN_DBIO *dbio,
                     GWEN_BUFFEREDIO *bio,
                     GWEN_TYPE_UINT32 flags,
                     GWEN_DB_NODE *db,
                     GWEN_DB_NODE *params) {
  assert(dbio);
  assert(bio);
  assert(db);

  if (dbio->exportFn)
    return dbio->exportFn(dbio, bio, flags, db, params);
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



const char *GWEN_DBIO_GetName(const GWEN_DBIO *dbio){
  assert(dbio);
  return dbio->name;
}



const char *GWEN_DBIO_GetDescription(const GWEN_DBIO *dbio){
  assert(dbio);
  return dbio->descr;
}



void GWEN_DBIO_SetImportFn(GWEN_DBIO *dbio, GWEN_DBIO_IMPORTFN f){
  assert(dbio);
  dbio->importFn=f;
}



void GWEN_DBIO_SetExportFn(GWEN_DBIO *dbio, GWEN_DBIO_EXPORTFN f){
  assert(dbio);
  dbio->exportFn=f;
}


void GWEN_DBIO_SetCheckFileFn(GWEN_DBIO *dbio, GWEN_DBIO_CHECKFILEFN f){
  assert(dbio);
  dbio->checkFileFn=f;
}



GWEN_DBIO *GWEN_DBIO_GetPlugin(const char *modname){
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

















