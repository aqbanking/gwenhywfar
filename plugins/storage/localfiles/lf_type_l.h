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

#ifndef LF_TYPE_L_H
#define LF_TYPE_L_H


#include <gwenhywfar/st_type_be.h>
#include <gwenhywfar/db.h>


GWEN_STO_TYPE *LocalFilesType_new(GWEN_STO_STORAGE *storage,
                                  GWEN_TYPE_UINT32 id,
                                  const char *typeName,
                                  const char *name,
                                  const char *baseFolder);

GWEN_STO_TYPE *LocalFilesType_dup(const GWEN_STO_TYPE *oty);

GWEN_STO_TYPE *LocalFilesType_fromDb(GWEN_STO_STORAGE *storage,
                                     GWEN_DB_NODE *db,
                                     const char *baseFolder);
void LocalFilesType_toDb(GWEN_STO_TYPE *ty, GWEN_DB_NODE *db);

const char *LocalFilesType_GetBaseFolder(const GWEN_STO_TYPE *ty);


#endif


