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

#ifndef GWEN_STO_STORAGE_L_H
#define GWEN_STO_STORAGE_L_H


#include <gwenhywfar/st_storage.h>
#include <gwenhywfar/st_client.h>
#include <gwenhywfar/st_type.h>
#include <gwenhywfar/st_find.h>
#include <gwenhywfar/st_object.h>


GWEN_ERRORCODE GWEN_StoStorage_ModuleInit();
GWEN_ERRORCODE GWEN_StoStorage_ModuleFini();



int GWEN_StoStorage_Create(GWEN_STO_STORAGE *st,
                           GWEN_STO_CLIENT *cl);

int GWEN_StoStorage_Open(GWEN_STO_STORAGE *st,
                         GWEN_STO_CLIENT *cl,
                         GWEN_TYPE_UINT32 openFlags);

int GWEN_StoStorage_Close(GWEN_STO_STORAGE *st,
                          GWEN_STO_CLIENT *cl,
                          const char *reason);

int GWEN_StoStorage_BeginEdit(GWEN_STO_STORAGE *st,
                              GWEN_STO_CLIENT *cl);
int GWEN_StoStorage_EndEdit(GWEN_STO_STORAGE *st,
                            GWEN_STO_CLIENT *cl,
                            GWEN_STO_CLOSEMODE cm);


int GWEN_StoStorage_CreateType(GWEN_STO_STORAGE *st,
                               GWEN_STO_CLIENT *cl,
                               const char *typeName,
                               const char *name,
                               GWEN_STO_TYPE **pts);


int GWEN_StoStorage_OpenType(GWEN_STO_STORAGE *st,
                             GWEN_STO_CLIENT *cl,
                             const char *typeName,
                             const char *name,
                             GWEN_TYPE_UINT32 openFlags,
                             GWEN_STO_TYPE **pts);

int GWEN_StoStorage_CloseType(GWEN_STO_STORAGE *st,
                              GWEN_STO_CLIENT *cl,
                              GWEN_STO_TYPE *ts,
                              GWEN_STO_CLOSEMODE cm);



int GWEN_StoStorage_FindFirstObject(GWEN_STO_STORAGE *st,
                                    GWEN_STO_CLIENT *cl,
                                    GWEN_STO_TYPE *ts,
                                    GWEN_STO_FIND **pfnd,
                                    GWEN_TYPE_UINT32 *id);

int GWEN_StoStorage_FindNextObject(GWEN_STO_STORAGE *st,
                                   GWEN_STO_CLIENT *cl,
                                   GWEN_STO_TYPE *ts,
                                   GWEN_STO_FIND *fnd,
                                   GWEN_TYPE_UINT32 *id);

int GWEN_StoStorage_CloseFind(GWEN_STO_STORAGE *st,
                              GWEN_STO_CLIENT *cl,
                              GWEN_STO_TYPE *ts,
                              GWEN_STO_FIND *fnd);

int GWEN_StoStorage_CreateObject(GWEN_STO_STORAGE *st,
                                 GWEN_STO_CLIENT *cl,
                                 GWEN_STO_TYPE *ts,
                                 GWEN_STO_OBJECT **po);

int GWEN_StoStorage_OpenObject(GWEN_STO_STORAGE *st,
                               GWEN_STO_CLIENT *cl,
                               GWEN_STO_TYPE *ts,
                               GWEN_TYPE_UINT32 id,
                               GWEN_TYPE_UINT32 openFlags,
                               GWEN_STO_OBJECT **po);

int GWEN_StoStorage_CloseObject(GWEN_STO_STORAGE *st,
                                GWEN_STO_CLIENT *cl,
                                GWEN_STO_TYPE *ts,
                                GWEN_STO_OBJECT *o,
                                GWEN_STO_CLOSEMODE cm);

int GWEN_StoStorage_DeleteObject(GWEN_STO_STORAGE *st,
                                 GWEN_STO_CLIENT *cl,
                                 GWEN_STO_TYPE *ts,
                                 GWEN_STO_OBJECT *o);

int GWEN_StoStorage_LockObject(GWEN_STO_STORAGE *st,
                               GWEN_STO_CLIENT *cl,
                               GWEN_STO_TYPE *ts,
                               GWEN_TYPE_UINT32 id,
                               GWEN_STO_LOCKMODE lm);


#endif



