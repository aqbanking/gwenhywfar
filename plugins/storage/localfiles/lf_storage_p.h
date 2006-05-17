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

#ifndef LF_STORAGE_P_H
#define LF_STORAGE_P_H


#include "lf_storage_l.h"



typedef struct LOCALFILES_STORAGE LOCALFILES_STORAGE;
struct LOCALFILES_STORAGE {
  int dummy;
};

static void LocalFilesStorage_FreeData(void *bp, void *p);



static int LocalFiles_NextUniqueId(GWEN_STO_STORAGE *st,
                                   GWEN_TYPE_UINT32 *pid);

static int LocalFiles_CreateDb(GWEN_STO_STORAGE *st);

static int LocalFiles_LoadDb(GWEN_STO_STORAGE *st);

static int LocalFiles_WriteType(GWEN_STO_STORAGE *st,
                                GWEN_STO_TYPE *ty);

static int LocalFiles_CreateType(GWEN_STO_STORAGE *st,
                                 const char *typeName,
                                 const char *name,
                                 GWEN_STO_TYPE **pty);

static int LocalFiles_DupType(GWEN_STO_STORAGE *st,
                              const GWEN_STO_TYPE *ty,
                              GWEN_STO_TYPE **pty);

static int LocalFiles_ReadObject(GWEN_STO_STORAGE *st,
                                 GWEN_STO_TYPE *ty,
                                 GWEN_TYPE_UINT32 id,
                                 GWEN_STO_OBJECT **po);

static int LocalFiles_WriteObject(GWEN_STO_STORAGE *st,
                                  GWEN_STO_OBJECT *o);

static int LocalFiles_CreateObject(GWEN_STO_STORAGE *st,
                                   GWEN_STO_TYPE *ty,
                                   GWEN_STO_OBJECT **po);

static int LocalFiles_DeleteObject(GWEN_STO_STORAGE *st,
                                   GWEN_STO_OBJECT *o);

static int LocalFiles_AddLog(GWEN_STO_STORAGE *st, const GWEN_STO_LOG *log);



static int LocalFiles__ScanObject_Files(GWEN_STO_STORAGE *st,
                                        GWEN_STO_TYPE *ty,
                                        const char *folder,
                                        GWEN_TYPE_UINT32 idSoFar);

static int LocalFiles__ScanObject_Folders(GWEN_STO_STORAGE *st,
                                          GWEN_STO_TYPE *ty,
                                          const char *folder,
                                          GWEN_TYPE_UINT32 idSoFar,
                                          int level);

static int LocalFiles__ScanTypeObjects(GWEN_STO_STORAGE *st,
                                       GWEN_STO_TYPE *ty);


static void LocalFiles__GetObjectPath(GWEN_STO_STORAGE *st,
                                      GWEN_STO_TYPE *ty,
                                      GWEN_TYPE_UINT32 id,
                                      GWEN_BUFFER *fbuf);


#endif


