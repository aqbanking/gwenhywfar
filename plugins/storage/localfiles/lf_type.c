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

#include "lf_type_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/smp_type_be.h>


GWEN_INHERIT(GWEN_STO_TYPE, LOCALFILES_TYPE)



GWEN_STO_TYPE *LocalFilesType_new(GWEN_STO_STORAGE *storage,
                                  GWEN_TYPE_UINT32 id,
                                  const char *typeName,
                                  const char *name,
                                  const char *baseFolder) {
  GWEN_STO_TYPE *ty;
  LOCALFILES_TYPE *xty;

  ty=GWEN_SmpStoType_new(storage, id, typeName, name);
  GWEN_NEW_OBJECT(LOCALFILES_TYPE, xty);
  if (baseFolder)
    xty->baseFolder=strdup(baseFolder);
  GWEN_INHERIT_SETDATA(GWEN_STO_TYPE, LOCALFILES_TYPE, ty, xty,
                       LocalFilesType_FreeData);

  return ty;
}



GWEN_STO_TYPE *LocalFilesType_dup(const GWEN_STO_TYPE *oty) {
  GWEN_STO_TYPE *ty;
  LOCALFILES_TYPE *xty;
  const char *s;

  ty=GWEN_SmpStoType_dup(oty);
  GWEN_NEW_OBJECT(LOCALFILES_TYPE, xty);
  s=LocalFilesType_GetBaseFolder(oty);
  if (s)
    xty->baseFolder=strdup(s);
  GWEN_INHERIT_SETDATA(GWEN_STO_TYPE, LOCALFILES_TYPE, ty, xty,
                       LocalFilesType_FreeData);

  return ty;
}



GWEN_STO_TYPE *LocalFilesType_fromDb(GWEN_STO_STORAGE *st,
                                     GWEN_DB_NODE *db,
                                     const char *baseFolder) {
  GWEN_STO_TYPE *ty;
  GWEN_TYPE_UINT32 id;
  const char *typeName;
  const char *name;
  GWEN_DB_NODE *dbVarDefs;

  id=GWEN_DB_GetIntValue(db, "id", 0, 0);
  if (!id) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No type id");
    return 0;
  }

  typeName=GWEN_DB_GetCharValue(db, "typeName", 0, 0);
  if (!typeName || !*typeName) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No type name");
    return 0;
  }

  name=GWEN_DB_GetCharValue(db, "name", 0, 0);
  if (!name || !*name) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No name");
    return 0;
  }

  /* create type */
  ty=LocalFilesType_new(st, id, typeName, name, baseFolder);
  /* read vardefs (if any) */
  dbVarDefs=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "vardefs");
  if (dbVarDefs) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_FindFirstGroup(dbVarDefs, "vardef");
    while(dbT) {
      GWEN_STO_VARDEF *vd;

      vd=GWEN_StoVarDef_fromDb(dbT);
      GWEN_StoType_AddVarNoLog(ty, vd);

      dbT=GWEN_DB_FindNextGroup(dbT, "vardef");
    }
  }
  return ty;
}



void LocalFilesType_toDb(GWEN_STO_TYPE *ty, GWEN_DB_NODE *db) {
  LOCALFILES_TYPE *xty;
  const char *s;
  GWEN_STO_VARDEF *vd;
  GWEN_DB_NODE *dbVarDefs;

  assert(ty);
  xty=GWEN_INHERIT_GETDATA(GWEN_STO_TYPE, LOCALFILES_TYPE, ty);
  assert(xty);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "id", GWEN_StoType_GetId(ty));
  s=GWEN_StoType_GetTypeName(ty);
  assert(s);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "typeName", s);
  s=GWEN_StoType_GetName(ty);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "name", s);

  dbVarDefs=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                             "vardefs");
  vd=GWEN_StoVarDef_List_First(GWEN_StoType_GetVarList(ty));
  while(vd) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_Group_new("vardef");
    GWEN_StoVarDef_toDb(vd, dbT);
    GWEN_DB_AddGroup(dbVarDefs, dbT);
    vd=GWEN_StoVarDef_List_Next(vd);
  }
}



void GWENHYWFAR_CB LocalFilesType_FreeData(void *bp, void *p) {
  LOCALFILES_TYPE *xty;

  xty=(LOCALFILES_TYPE*) p;
  free(xty->baseFolder);
  GWEN_FREE_OBJECT(xty);
}



const char *LocalFilesType_GetBaseFolder(const GWEN_STO_TYPE *ty) {
  LOCALFILES_TYPE *xty;

  assert(ty);
  xty=GWEN_INHERIT_GETDATA(GWEN_STO_TYPE, LOCALFILES_TYPE, ty);
  assert(xty);

  return xty->baseFolder;
}








