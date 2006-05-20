/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: csv_p.h 120 2003-12-03 23:29:33Z aquamaniac $
 begin       : Thu Oct 30 2003
 copyright   : (C) 2003 by Martin Preuss
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

#ifndef GWEN_STO_STORAGE_BE_H
#define GWEN_STO_STORAGE_BE_H


#include <gwenhywfar/st_storage_be.h>
#include <gwenhywfar/st_client_be.h>
#include <gwenhywfar/st_type_be.h>
#include <gwenhywfar/st_object_be.h>
#include <gwenhywfar/st_find.h>
#include <gwenhywfar/inherit.h>


GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_STO_STORAGE, GWENHYWFAR_API)
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_STO_STORAGE, GWEN_StoStorage, GWENHYWFAR_API)


typedef int (*GWEN_STO_STORAGE_REGISTERCLIENT_FN)(GWEN_STO_STORAGE *st,
                                                  const char *userName,
                                                  GWEN_STO_CLIENT **pcl);
typedef int (*GWEN_STO_STORAGE_UNREGISTERCLIENT_FN)(GWEN_STO_STORAGE *st,
                                                    GWEN_STO_CLIENT *cl);


typedef int (*GWEN_STO_STORAGE_CREATE_FN)(GWEN_STO_STORAGE *st,
                                          GWEN_STO_CLIENT *cl);

typedef int (*GWEN_STO_STORAGE_OPEN_FN)(GWEN_STO_STORAGE *st,
                                        GWEN_STO_CLIENT *cl,
                                        GWEN_TYPE_UINT32 openFlags);

typedef int (*GWEN_STO_STORAGE_CLOSE_FN)(GWEN_STO_STORAGE *st,
                                         GWEN_STO_CLIENT *cl,
                                         const char *reason);

typedef int (*GWEN_STO_STORAGE_BEGINEDIT_FN)(GWEN_STO_STORAGE *st,
                                             GWEN_STO_CLIENT *cl);
typedef int (*GWEN_STO_STORAGE_ENDEDIT_FN)(GWEN_STO_STORAGE *st,
                                           GWEN_STO_CLIENT *cl,
                                           GWEN_STO_CLOSEMODE cm);


typedef int (*GWEN_STO_STORAGE_CREATETYPE_FN)(GWEN_STO_STORAGE *st,
                                              GWEN_STO_CLIENT *cl,
                                              const char *typeName,
                                              const char *name,
                                              GWEN_STO_TYPE **pts);


typedef int (*GWEN_STO_STORAGE_OPENTYPE_FN)(GWEN_STO_STORAGE *st,
                                            GWEN_STO_CLIENT *cl,
                                            const char *typeName,
                                            const char *name,
                                            GWEN_TYPE_UINT32 openFlags,
                                            GWEN_STO_TYPE **pts);

typedef int (*GWEN_STO_STORAGE_CLOSETYPE_FN)(GWEN_STO_STORAGE *st,
                                             GWEN_STO_CLIENT *cl,
                                             GWEN_STO_TYPE *ts);



typedef int (*GWEN_STO_STORAGE_FINDFIRSTOBJECT_FN)(GWEN_STO_STORAGE *st,
                                                   GWEN_STO_CLIENT *cl,
                                                   GWEN_STO_TYPE *ts,
                                                   GWEN_STO_FIND **pfnd,
                                                   GWEN_TYPE_UINT32 *id);
typedef int (*GWEN_STO_STORAGE_FINDNEXTOBJECT_FN)(GWEN_STO_STORAGE *st,
                                                  GWEN_STO_CLIENT *cl,
                                                  GWEN_STO_TYPE *ts,
                                                  GWEN_STO_FIND *fnd,
                                                  GWEN_TYPE_UINT32 *id);
typedef int (*GWEN_STO_STORAGE_CLOSEFIND_FN)(GWEN_STO_STORAGE *st,
                                             GWEN_STO_CLIENT *cl,
                                             GWEN_STO_TYPE *ts,
                                             GWEN_STO_FIND *fnd);

typedef int (*GWEN_STO_STORAGE_CREATEOBJECT_FN)(GWEN_STO_STORAGE *st,
                                                GWEN_STO_CLIENT *cl,
                                                GWEN_STO_TYPE *ts,
                                                GWEN_STO_OBJECT **po);

typedef int (*GWEN_STO_STORAGE_OPENOBJECT_FN)(GWEN_STO_STORAGE *st,
                                              GWEN_STO_CLIENT *cl,
                                              GWEN_STO_TYPE *ts,
                                              GWEN_TYPE_UINT32 id,
                                              GWEN_TYPE_UINT32 openFlags,
                                              GWEN_STO_OBJECT **po);

typedef int (*GWEN_STO_STORAGE_CLOSEOBJECT_FN)(GWEN_STO_STORAGE *st,
                                               GWEN_STO_CLIENT *cl,
                                               GWEN_STO_TYPE *ts,
                                               GWEN_STO_OBJECT *o);

typedef int (*GWEN_STO_STORAGE_DELETEOBJECT_FN)(GWEN_STO_STORAGE *st,
                                                GWEN_STO_CLIENT *cl,
                                                GWEN_STO_TYPE *ts,
                                                GWEN_STO_OBJECT *o);

typedef int (*GWEN_STO_STORAGE_LOCKOBJECT_FN)(GWEN_STO_STORAGE *st,
                                              GWEN_STO_CLIENT *cl,
                                              GWEN_STO_TYPE *ts,
                                              GWEN_TYPE_UINT32 id,
                                              GWEN_STO_LOCKMODE lm);


GWENHYWFAR_API
GWEN_STO_STORAGE *GWEN_StoStorage_new(const char *typeName,
                                      const char *address);


GWENHYWFAR_API
GWEN_STO_CLIENT_LIST*
  GWEN_StoStorage_GetClientList(const GWEN_STO_STORAGE *st);

GWENHYWFAR_API
GWEN_STO_CLIENT *GWEN_StoStorage_FindClient(const GWEN_STO_STORAGE *st,
                                            GWEN_TYPE_UINT32 id);


GWENHYWFAR_API
GWEN_STO_TYPE_LIST*
  GWEN_StoStorage_GetTypeList(const GWEN_STO_STORAGE *st);

GWENHYWFAR_API
GWEN_STO_TYPE *GWEN_StoStorage_FindTypeById(const GWEN_STO_STORAGE *st,
                                            GWEN_TYPE_UINT32 id);

GWENHYWFAR_API
GWEN_STO_TYPE *GWEN_StoStorage_FindTypeByName(const GWEN_STO_STORAGE *st,
                                              const char *typeName,
                                              const char *name);

GWENHYWFAR_API
void GWEN_StoStorage_AddType(GWEN_STO_STORAGE *st, GWEN_STO_TYPE *ty);


GWENHYWFAR_API
GWEN_STO_OBJECT_LIST*
  GWEN_StoStorage_GetObjectList(const GWEN_STO_STORAGE *st);

GWENHYWFAR_API
GWEN_STO_OBJECT *GWEN_StoStorage_FindObject(const GWEN_STO_STORAGE *st,
                                            GWEN_TYPE_UINT32 id);

GWENHYWFAR_API
void GWEN_StoStorage_AddObject(GWEN_STO_STORAGE *st, GWEN_STO_OBJECT *o);



GWENHYWFAR_API
GWEN_STO_CLOSEMODE GWEN_StoCloseMode_fromString(const char *s);

GWENHYWFAR_API
const char *GWEN_StoCloseMode_toString(GWEN_STO_CLOSEMODE cm);

GWENHYWFAR_API
GWEN_STO_LOCKMODE GWEN_StoLockMode_fromString(const char *s);

GWENHYWFAR_API
const char *GWEN_StoLockMode_toString(GWEN_STO_LOCKMODE lm);


GWENHYWFAR_API
void GWEN_StoStorage_LogCallback(GWEN_STO_STORAGE *st,
                                 const GWEN_STO_LOG *log);



GWENHYWFAR_API
GWEN_STO_STORAGE_CREATE_FN
GWEN_StoStorage_SetCreateFn(GWEN_STO_STORAGE *st,
                            GWEN_STO_STORAGE_CREATE_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_OPEN_FN
GWEN_StoStorage_SetOpenFn(GWEN_STO_STORAGE *st,
                          GWEN_STO_STORAGE_OPEN_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_CLOSE_FN
GWEN_StoStorage_SetCloseFn(GWEN_STO_STORAGE *st,
                           GWEN_STO_STORAGE_CLOSE_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_REGISTERCLIENT_FN
GWEN_StoStorage_SetRegisterClientFn(GWEN_STO_STORAGE *st,
                                    GWEN_STO_STORAGE_REGISTERCLIENT_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_UNREGISTERCLIENT_FN
GWEN_StoStorage_SetUnregisterClientFn(GWEN_STO_STORAGE *st,
                                      GWEN_STO_STORAGE_UNREGISTERCLIENT_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_BEGINEDIT_FN
GWEN_StoStorage_SetBeginEditFn(GWEN_STO_STORAGE *st,
                               GWEN_STO_STORAGE_BEGINEDIT_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_ENDEDIT_FN
GWEN_StoStorage_SetEndEditFn(GWEN_STO_STORAGE *st,
                             GWEN_STO_STORAGE_ENDEDIT_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_CREATETYPE_FN
GWEN_StoStorage_SetCreateTypeFn(GWEN_STO_STORAGE *st,
                                GWEN_STO_STORAGE_CREATETYPE_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_OPENTYPE_FN
GWEN_StoStorage_SetOpenTypeFn(GWEN_STO_STORAGE *st,
                              GWEN_STO_STORAGE_OPENTYPE_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_CLOSETYPE_FN
GWEN_StoStorage_SetCloseTypeFn(GWEN_STO_STORAGE *st,
                               GWEN_STO_STORAGE_CLOSETYPE_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_FINDFIRSTOBJECT_FN
GWEN_StoStorage_SetFindFirstObjectFn(GWEN_STO_STORAGE *st,
                                     GWEN_STO_STORAGE_FINDFIRSTOBJECT_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_FINDNEXTOBJECT_FN
GWEN_StoStorage_SetFindNextObjectFn(GWEN_STO_STORAGE *st,
                                    GWEN_STO_STORAGE_FINDNEXTOBJECT_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_CLOSEFIND_FN
GWEN_StoStorage_SetCloseFindFn(GWEN_STO_STORAGE *st,
                               GWEN_STO_STORAGE_CLOSEFIND_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_CREATEOBJECT_FN
GWEN_StoStorage_SetCreateObjectFn(GWEN_STO_STORAGE *st,
                                  GWEN_STO_STORAGE_CREATEOBJECT_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_OPENOBJECT_FN
GWEN_StoStorage_SetOpenObjectFn(GWEN_STO_STORAGE *st,
                                GWEN_STO_STORAGE_OPENOBJECT_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_CLOSEOBJECT_FN
GWEN_StoStorage_SetCloseObjectFn(GWEN_STO_STORAGE *st,
                                 GWEN_STO_STORAGE_CLOSEOBJECT_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_DELETEOBJECT_FN
GWEN_StoStorage_SetDeleteObjectFn(GWEN_STO_STORAGE *st,
                                  GWEN_STO_STORAGE_DELETEOBJECT_FN f);

GWENHYWFAR_API
GWEN_STO_STORAGE_LOCKOBJECT_FN
GWEN_StoStorage_SetLockObjectFn(GWEN_STO_STORAGE *st,
                                GWEN_STO_STORAGE_LOCKOBJECT_FN f);



#endif



