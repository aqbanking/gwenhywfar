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
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/path.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>


GWEN_LIST_FUNCTIONS(GWEN_DBIO, GWEN_DBIO);
GWEN_INHERIT_FUNCTIONS(GWEN_DBIO);



static GWEN_DBIO_LIST *gwen_dbio__list=0;


GWEN_ERRORCODE GWEN_DBIO_ModuleInit(){
  gwen_dbio__list=GWEN_DBIO_List_new();
  return 0;
}



GWEN_ERRORCODE GWEN_DBIO_ModuleFini(){
  GWEN_DBIO_List_free(gwen_dbio__list);
  return 0;
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

      if (dbio->libLoader) {
        GWEN_LibLoader_CloseLibrary(dbio->libLoader);
        GWEN_LibLoader_free(dbio->libLoader);
      }
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
    DBG_INFO(0, "No import function set");
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
    DBG_INFO(0, "No export function set");
    return -1;
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



GWEN_LIBLOADER *GWEN_DBIO_GetLibLoader(const GWEN_DBIO *dbio){
  assert(dbio);
  return dbio->libLoader;
}



void GWEN_DBIO_SetLibLoader(GWEN_DBIO *dbio, GWEN_LIBLOADER *ll){
  assert(dbio);
  assert(dbio->libLoader==0);
  dbio->libLoader=ll;
}




int GWEN_DBIO_Register(GWEN_DBIO *dbio) {
  assert(gwen_dbio__list);
  assert(dbio);

  GWEN_DBIO_List_Insert(dbio, gwen_dbio__list);

  return 0;
}



GWEN_DBIO *GWEN_DBIO_Find(const char *name) {
  GWEN_DBIO *dbio;

  assert(gwen_dbio__list);
  assert(name);

  dbio=GWEN_DBIO_List_First(gwen_dbio__list);
  while(dbio) {
    if (strcasecmp(GWEN_DBIO_GetName(dbio), name)==0)
      return dbio;
    dbio=GWEN_DBIO_List_Next(dbio);
  } /* while */

  return 0;
}



void GWEN_DBIO_SetImportFn(GWEN_DBIO *dbio, GWEN_DBIO_IMPORTFN f){
  assert(dbio);
  dbio->importFn=f;
}



void GWEN_DBIO_SetExportFn(GWEN_DBIO *dbio, GWEN_DBIO_EXPORTFN f){
  assert(dbio);
  dbio->exportFn=f;
}



GWEN_DBIO *GWEN_DBIO_LoadPluginFile(const char *modname, const char *fname){
  GWEN_LIBLOADER *ll;
  GWEN_DBIO *dbio;
  GWEN_DBIO_FACTORYFN fn;
  void *p;
  GWEN_BUFFER *nbuf;
  const char *s;
  GWEN_ERRORCODE err;

  ll=GWEN_LibLoader_new();
  if (GWEN_LibLoader_OpenLibrary(ll, fname)) {
    DBG_ERROR(0, "Could not load DBIO plugin \"%s\" (%s)", modname, fname);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* create name of init function */
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendString(nbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(nbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(nbuf);
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }
  GWEN_Buffer_free(nbuf);

  fn=(GWEN_DBIO_FACTORYFN)p;
  assert(fn);
  dbio=fn();
  if (!dbio) {
    DBG_ERROR(0, "Error in plugin: No DBIO created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  GWEN_DBIO_SetLibLoader(dbio, ll);

  return dbio;
}



GWEN_DBIO *GWEN_DBIO_LoadPlugin(const char *modname){
  GWEN_LIBLOADER *ll;
  GWEN_DBIO *dbio;
  GWEN_DBIO_FACTORYFN fn;
  void *p;
  GWEN_BUFFER *nbuf;
  const char *s;
  GWEN_ERRORCODE err;

  ll=GWEN_LibLoader_new();
  if (GWEN_LibLoader_OpenLibraryWithPath(ll,
                                         GWENHYWFAR_PLUGINS
                                         "/"
                                         GWEN_DBIO_FOLDER,
                                         modname)) {
    DBG_ERROR(0, "Could not load DBIO plugin \"%s\"", modname);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* create name of init function */
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendString(nbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(nbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(nbuf);
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }
  GWEN_Buffer_free(nbuf);

  fn=(GWEN_DBIO_FACTORYFN)p;
  assert(fn);
  dbio=fn();
  if (!dbio) {
    DBG_ERROR(0, "Error in plugin: No DBIO created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  GWEN_DBIO_SetLibLoader(dbio, ll);

  return dbio;
}



GWEN_DBIO *GWEN_DBIO_GetPlugin(const char *modname){
  GWEN_DBIO *dbio;

  dbio=GWEN_DBIO_Find(modname);
  if (dbio) {
    GWEN_DBIO_Attach(dbio);
    return dbio;
  }
  dbio=GWEN_DBIO_LoadPlugin(modname);
  if (!dbio)
    return 0;

  if (strcasecmp(GWEN_DBIO_GetName(dbio), modname)!=0) {
    DBG_ERROR(0, "Plugin \"%s\" does not support the expected DBIO \"%s\"",
              GWEN_DBIO_GetName(dbio), modname);
    GWEN_DBIO_free(dbio);
    return 0;
  }
  GWEN_DBIO_Register(dbio);
  GWEN_DBIO_Attach(dbio);
  return dbio;
}

















