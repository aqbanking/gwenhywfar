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

#ifndef GWEN_SMPSTO_STORAGE_P_H
#define GWEN_SMPSTO_STORAGE_P_H

#include "smp_storage_be.h"


typedef struct GWEN_SMPSTO_STORAGE GWEN_SMPSTO_STORAGE;
struct GWEN_SMPSTO_STORAGE {

  GWEN_STO_CLIENT *lockHolder;
  GWEN_TYPE_UINT32 lastRuntimeId;

  GWEN_SMPSTO_NEXTUNIQUEID_FN nextUniqueIdFn;
  GWEN_SMPSTO_CREATEDB_FN createDbFn;
  GWEN_SMPSTO_LOADDB_FN loadDbFn;

  GWEN_SMPSTO_WRITETYPE_FN writeTypeFn;
  GWEN_SMPSTO_CREATETYPE_FN createTypeFn;
  GWEN_SMPSTO_DUPTYPE_FN dupTypeFn;

  GWEN_SMPSTO_READOBJECT_FN readObjectFn;
  GWEN_SMPSTO_WRITEOBJECT_FN writeObjectFn;
  GWEN_SMPSTO_CREATEOBJECT_FN createObjectFn;
  GWEN_SMPSTO_DELETEOBJECT_FN deleteObjectFn;

  GWEN_SMPSTO_ADDLOG_FN addLogFn;
};

void GWEN_SmpStoStorage_FreeData(void *bp, void *p);

int GWEN_SmpStoStorage_Create(GWEN_STO_STORAGE *st,
                              GWEN_STO_CLIENT *cl);
int GWEN_SmpStoStorage_Open(GWEN_STO_STORAGE *st,
                            GWEN_STO_CLIENT *cl,
                            GWEN_TYPE_UINT32 openFlags);
int GWEN_SmpStoStorage_Close(GWEN_STO_STORAGE *st,
                             GWEN_STO_CLIENT *cl,
                             const char *reason);
int GWEN_SmpStoStorage_RegisterClient(GWEN_STO_STORAGE *st,
                                      const char *userName,
                                      GWEN_STO_CLIENT **pcl);
int GWEN_SmpStoStorage_UnregisterClient(GWEN_STO_STORAGE *st,
                                        GWEN_STO_CLIENT *cl);
int GWEN_SmpStoStorage_BeginEdit(GWEN_STO_STORAGE *st,
                                 GWEN_STO_CLIENT *cl);
int GWEN_SmpStoStorage_EndEdit(GWEN_STO_STORAGE *st,
                               GWEN_STO_CLIENT *cl,
                               GWEN_STO_CLOSEMODE cm);
int GWEN_SmpStoStorage_CreateType(GWEN_STO_STORAGE *st,
                                  GWEN_STO_CLIENT *cl,
                                  const char *typeName,
                                  const char *name,
                                  GWEN_STO_TYPE **pts);
int GWEN_SmpStoStorage_OpenType(GWEN_STO_STORAGE *st,
                                GWEN_STO_CLIENT *cl,
                                const char *typeName,
                                const char *name,
                                GWEN_TYPE_UINT32 flags,
                                GWEN_STO_TYPE **pts);
int GWEN_SmpStoStorage_CloseType(GWEN_STO_STORAGE *st,
                                 GWEN_STO_CLIENT *cl,
                                 GWEN_STO_TYPE *ts);
int GWEN_SmpStoStorage_FindFirstObject(GWEN_STO_STORAGE *st,
                                       GWEN_STO_CLIENT *cl,
                                       GWEN_STO_TYPE *ts,
                                       GWEN_STO_FIND **pfnd,
                                       GWEN_TYPE_UINT32 *id);
int GWEN_SmpStoStorage_FindNextObject(GWEN_STO_STORAGE *st,
                                      GWEN_STO_CLIENT *cl,
                                      GWEN_STO_TYPE *ts,
                                      GWEN_STO_FIND *fnd,
                                      GWEN_TYPE_UINT32 *id);
int GWEN_SmpStoStorage_CloseFind(GWEN_STO_STORAGE *st,
                                 GWEN_STO_CLIENT *cl,
                                 GWEN_STO_TYPE *ts,
                                 GWEN_STO_FIND *fnd);
int GWEN_SmpStoStorage_CreateObject(GWEN_STO_STORAGE *st,
                                    GWEN_STO_CLIENT *cl,
                                    GWEN_STO_TYPE *ts,
                                    GWEN_STO_OBJECT **po);

/**
 * This function searches in the clients local list for the given object.
 * If found its openCounter is incremented and the object returned.
 * Otherwise the object will be read via @ref GWEN_SmpSto_ReadObject and
 * added to the clients local list (in case of write access) or to the global
 * list (if read access only).
 */
int GWEN_SmpStoStorage_OpenObject(GWEN_STO_STORAGE *st,
                                  GWEN_STO_CLIENT *cl,
                                  GWEN_STO_TYPE *ts,
                                  GWEN_TYPE_UINT32 id,
                                  GWEN_TYPE_UINT32 openFlags,
                                  GWEN_STO_OBJECT **po);
int GWEN_SmpStoStorage_CloseObject(GWEN_STO_STORAGE *st,
                                   GWEN_STO_CLIENT *cl,
                                   GWEN_STO_TYPE *ts,
                                   GWEN_STO_OBJECT *o);
int GWEN_SmpStoStorage_DeleteObject(GWEN_STO_STORAGE *st,
                                    GWEN_STO_CLIENT *cl,
                                    GWEN_STO_TYPE *ts,
                                    GWEN_STO_OBJECT *o);

int GWEN_SmpStoStorage_LockObject(GWEN_STO_STORAGE *st,
                                  GWEN_STO_CLIENT *cl,
                                  GWEN_STO_TYPE *ts,
                                  GWEN_TYPE_UINT32 id,
                                  GWEN_STO_LOCKMODE lm);



int GWEN_SmpStoStorage__DecObjectOpenCount(GWEN_STO_STORAGE *st,
                                           GWEN_STO_OBJECT *o);













#endif



