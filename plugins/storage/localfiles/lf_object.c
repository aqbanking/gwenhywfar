/***************************************************************************
 $RCSfile: account.h,v $
 -------------------
 cvs         : $Id: account.h,v 1.16 2006/02/22 19:38:54 aquamaniac Exp $
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "lf_object_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>


GWEN_INHERIT(GWEN_STO_OBJECT, LOCALFILES_OBJECT)



GWEN_STO_OBJECT *LocalFilesObject_new(GWEN_STO_TYPE *ty,
                                      GWEN_TYPE_UINT32 id,
                                      const char *fname) {
  GWEN_STO_OBJECT *o;
  LOCALFILES_OBJECT *xo;

  o=GWEN_StoObject_new(ty, id);
  GWEN_NEW_OBJECT(LOCALFILES_OBJECT, xo);
  GWEN_INHERIT_SETDATA(GWEN_STO_OBJECT, LOCALFILES_OBJECT, o, xo,
                       LocalFilesObject_FreeData);
  if (fname)
    xo->fileName=strdup(fname);

  return o;
}



void LocalFilesObject_FreeData(void *bp, void *p) {
  LOCALFILES_OBJECT *xo;

  xo=(LOCALFILES_OBJECT*) p;
  free(xo->fileName);
  GWEN_FREE_OBJECT(xo);
}



GWEN_STO_OBJECT *LocalFilesObject_fromDb(GWEN_STO_TYPE *ty,
                                         GWEN_TYPE_UINT32 id,
                                         GWEN_DB_NODE *db,
                                         const char *fname) {
  GWEN_STO_OBJECT *o;
  GWEN_DB_NODE *dbValues;

  o=LocalFilesObject_new(ty, id, fname);
  dbValues=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "values");
  if (dbValues)
    GWEN_StoObject_SetDbDataNode(o, GWEN_DB_Group_dup(dbValues));

  return o;
}



void LocalFilesObject_toDb(GWEN_STO_OBJECT *o, GWEN_DB_NODE *db) {
  LOCALFILES_OBJECT *xo;
  GWEN_DB_NODE *dbDataNode;
  GWEN_DB_NODE *dbValues;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(GWEN_STO_OBJECT, LOCALFILES_OBJECT, o);
  assert(xo);

  dbDataNode=GWEN_StoObject_GetDbDataNode(o);
  dbValues=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "values");
  assert(dbValues);
  GWEN_DB_AddGroupChildren(dbValues, dbDataNode);
}



const char *LocalFilesObject_GetFileName(const GWEN_STO_OBJECT *o) {
  LOCALFILES_OBJECT *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(GWEN_STO_OBJECT, LOCALFILES_OBJECT, o);
  assert(xo);

  return xo->fileName;
}






