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

#ifndef LF_OBJECT_L_H
#define LF_OBJECT_L_H

#include <gwenhywfar/st_object_be.h>


GWEN_STO_OBJECT *LocalFilesObject_new(GWEN_STO_TYPE *ty,
                                      GWEN_TYPE_UINT32 id,
                                      const char *fname);

GWEN_STO_OBJECT *LocalFilesObject_fromDb(GWEN_STO_TYPE *ty,
                                         GWEN_TYPE_UINT32 id,
                                         GWEN_DB_NODE *db,
                                         const char *fname);
void LocalFilesObject_toDb(GWEN_STO_OBJECT *o, GWEN_DB_NODE *db);

const char *LocalFilesObject_GetFileName(const GWEN_STO_OBJECT *o);




#endif


