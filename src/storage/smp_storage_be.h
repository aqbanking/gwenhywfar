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

#ifndef GWEN_SMP_STORAGE_BE_H
#define GWEN_SMP_STORAGE_BE_H

/** @file smp_storage_be.h
 *
 * This file contains a simplified storage framework.
 * It makes it much easier to correctly implement a GWEN storage because
 * fewer functions have to be implemented.
 * This module automatically handles object locks and storage-wide locks
 * (as well as BeginEdit/EndEdit sessions), so derived classes only have
 * to implement reading/writing data (e.g. using files or real databases).
 */

#include <gwenhywfar/st_storage_be.h>


/** @name Constructor
 *
 */
/*@{*/
GWENHYWFAR_API
GWEN_STO_STORAGE *GWEN_SmpStoStorage_new(const char *typeName,
                                         const char *address);
/*@}*/




/** @name Prototypes for Virtual Functions
 *
 */
/*@{*/
typedef int (*GWEN_SMPSTO_NEXTUNIQUEID_FN)(GWEN_STO_STORAGE *st,
                                           GWEN_STO_TYPE *ty,
                                           GWEN_TYPE_UINT32 *pid);
typedef int (*GWEN_SMPSTO_CREATEDB_FN)(GWEN_STO_STORAGE *st);
typedef int (*GWEN_SMPSTO_LOADDB_FN)(GWEN_STO_STORAGE *st);
typedef int (*GWEN_SMPSTO_WRITETYPE_FN)(GWEN_STO_STORAGE *st,
                                        GWEN_STO_TYPE *ty);
typedef int (*GWEN_SMPSTO_CREATETYPE_FN)(GWEN_STO_STORAGE *st,
                                         const char *typeName,
                                         const char *name,
                                         GWEN_STO_TYPE **pty);
typedef int (*GWEN_SMPSTO_DUPTYPE_FN)(GWEN_STO_STORAGE *st,
                                      const GWEN_STO_TYPE *ty,
                                      GWEN_STO_TYPE **pty);


typedef int (*GWEN_SMPSTO_READOBJECT_FN)(GWEN_STO_STORAGE *st,
                                         GWEN_STO_TYPE *ts,
                                         GWEN_TYPE_UINT32 id,
                                         GWEN_STO_OBJECT **po);
typedef int (*GWEN_SMPSTO_WRITEOBJECT_FN)(GWEN_STO_STORAGE *st,
                                          GWEN_STO_OBJECT *o);
typedef int (*GWEN_SMPSTO_DELETEOBJECT_FN)(GWEN_STO_STORAGE *st,
                                           GWEN_STO_OBJECT *o);
typedef int (*GWEN_SMPSTO_CREATEOBJECT_FN)(GWEN_STO_STORAGE *st,
                                           GWEN_STO_TYPE *ts,
                                           GWEN_STO_OBJECT **po);

typedef int (*GWEN_SMPSTO_ADDLOG_FN)(GWEN_STO_STORAGE *st,
                                     const GWEN_STO_LOG *log);
/*@}*/




/** @name Setters For Virtual Functions
 *
 */
/*@{*/

GWENHYWFAR_API
GWEN_SMPSTO_NEXTUNIQUEID_FN
  GWEN_SmpSto_SetNextUniqueIdFn(GWEN_STO_STORAGE *st,
                                GWEN_SMPSTO_NEXTUNIQUEID_FN f);

GWENHYWFAR_API
GWEN_SMPSTO_CREATEDB_FN
  GWEN_SmpSto_SetCreateDbFn(GWEN_STO_STORAGE *st,
                            GWEN_SMPSTO_CREATEDB_FN f);

GWENHYWFAR_API
GWEN_SMPSTO_LOADDB_FN
  GWEN_SmpSto_SetLoadDbFn(GWEN_STO_STORAGE *st,
                          GWEN_SMPSTO_LOADDB_FN f);

GWENHYWFAR_API
GWEN_SMPSTO_WRITETYPE_FN
  GWEN_SmpSto_SetWriteTypeFn(GWEN_STO_STORAGE *st,
                             GWEN_SMPSTO_WRITETYPE_FN f);

GWENHYWFAR_API
GWEN_SMPSTO_CREATETYPE_FN
  GWEN_SmpSto_SetCreateTypeFn(GWEN_STO_STORAGE *st,
                              GWEN_SMPSTO_CREATETYPE_FN f);

GWENHYWFAR_API
GWEN_SMPSTO_DUPTYPE_FN
  GWEN_SmpSto_SetDupTypeFn(GWEN_STO_STORAGE *st,
                           GWEN_SMPSTO_DUPTYPE_FN f);

GWENHYWFAR_API
GWEN_SMPSTO_READOBJECT_FN
  GWEN_SmpSto_SetReadObjectFn(GWEN_STO_STORAGE *st,
                              GWEN_SMPSTO_READOBJECT_FN f);

GWENHYWFAR_API
GWEN_SMPSTO_WRITEOBJECT_FN
  GWEN_SmpSto_SetWriteObjectFn(GWEN_STO_STORAGE *st,
                               GWEN_SMPSTO_WRITEOBJECT_FN f);

GWENHYWFAR_API
GWEN_SMPSTO_CREATEOBJECT_FN
  GWEN_SmpSto_SetCreateObjectFn(GWEN_STO_STORAGE *st,
                                GWEN_SMPSTO_CREATEOBJECT_FN f);

GWENHYWFAR_API
GWEN_SMPSTO_DELETEOBJECT_FN
  GWEN_SmpSto_SetDeleteObjectFn(GWEN_STO_STORAGE *st,
                                GWEN_SMPSTO_DELETEOBJECT_FN f);

GWENHYWFAR_API
GWEN_SMPSTO_ADDLOG_FN
  GWEN_SmpSto_SetAddLogFn(GWEN_STO_STORAGE *st, GWEN_SMPSTO_ADDLOG_FN f);

/*@}*/



/** @name Virtual Functions
 *
 */
/*@{*/
GWENHYWFAR_API
int GWEN_SmpSto_NextUniqueId(GWEN_STO_STORAGE *st,
                             GWEN_STO_TYPE *ty,
                             GWEN_TYPE_UINT32 *pid);

GWENHYWFAR_API
int GWEN_SmpSto_CreateDb(GWEN_STO_STORAGE *st);

GWENHYWFAR_API
int GWEN_SmpSto_LoadDb(GWEN_STO_STORAGE *st);

GWENHYWFAR_API
int GWEN_SmpSto_WriteType(GWEN_STO_STORAGE *st,
                          GWEN_STO_TYPE *ty);

GWENHYWFAR_API
int GWEN_SmpSto_CreateType(GWEN_STO_STORAGE *st,
                           const char *typeName,
                           const char *name,
                           GWEN_STO_TYPE **pty);

GWENHYWFAR_API
int GWEN_SmpSto_DupType(GWEN_STO_STORAGE *st,
                        const GWEN_STO_TYPE *ty,
                        GWEN_STO_TYPE **pty);

GWENHYWFAR_API
int GWEN_SmpSto_ReadObject(GWEN_STO_STORAGE *st,
                           GWEN_STO_TYPE *ty,
                           GWEN_TYPE_UINT32 id,
                           GWEN_STO_OBJECT **po);

GWENHYWFAR_API
int GWEN_SmpSto_WriteObject(GWEN_STO_STORAGE *st,
                            GWEN_STO_OBJECT *o);

GWENHYWFAR_API
int GWEN_SmpSto_CreateObject(GWEN_STO_STORAGE *st,
                             GWEN_STO_TYPE *ty,
                             GWEN_STO_OBJECT **po);

GWENHYWFAR_API
int GWEN_SmpSto_DeleteObject(GWEN_STO_STORAGE *st,
                             GWEN_STO_OBJECT *o);

GWENHYWFAR_API
int GWEN_SmpSto_AddLog(GWEN_STO_STORAGE *st, const GWEN_STO_LOG *log);

/*@}*/


#endif



