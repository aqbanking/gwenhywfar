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

#ifndef GWEN_STO_OBJECT_BE_H
#define GWEN_STO_OBJECT_BE_H

#include <gwenhywfar/st_object.h>
#include <gwenhywfar/st_client.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/list.h>
#include <gwenhywfar/inherit.h>


GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_STO_OBJECT, GWEN_StoObject, GWENHYWFAR_API)
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_STO_OBJECT, GWENHYWFAR_API)


GWENHYWFAR_API
GWEN_STO_OBJECT *GWEN_StoObject_new(GWEN_STO_TYPE *ty,
                                    GWEN_TYPE_UINT32 id);

GWENHYWFAR_API
void GWEN_StoObject_free(GWEN_STO_OBJECT *o);

GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_StoObject_GetFlags(const GWEN_STO_OBJECT *o);

GWENHYWFAR_API
void GWEN_StoObject_SetFlags(GWEN_STO_OBJECT *o, GWEN_TYPE_UINT32 fl);

GWENHYWFAR_API
void GWEN_StoObject_AddFlags(GWEN_STO_OBJECT *o, GWEN_TYPE_UINT32 fl);

GWENHYWFAR_API
void GWEN_StoObject_SubFlags(GWEN_STO_OBJECT *o, GWEN_TYPE_UINT32 fl);

GWENHYWFAR_API
GWEN_DB_NODE *GWEN_StoObject_GetDbDataNode(const GWEN_STO_OBJECT *o);

GWENHYWFAR_API
void GWEN_StoObject_SetDbDataNode(GWEN_STO_OBJECT *o, GWEN_DB_NODE *db);

GWENHYWFAR_API
void GWEN_StoObject_SetRefCount(GWEN_STO_OBJECT *o, int i);

GWENHYWFAR_API
int GWEN_StoObject_IsModified(const GWEN_STO_OBJECT *o);

GWENHYWFAR_API
int GWEN_StoObject_GetOpenCount(const GWEN_STO_OBJECT *o);

GWENHYWFAR_API
int GWEN_StoObject_IncOpenCount(GWEN_STO_OBJECT *o);

GWENHYWFAR_API
int GWEN_StoObject_DecOpenCount(GWEN_STO_OBJECT *o);

GWENHYWFAR_API
void GWEN_StoObject_SetOpenCount(GWEN_STO_OBJECT *o, int i);

GWENHYWFAR_API
GWEN_STO_CLIENT *GWEN_StoObject_GetLockHolder(const GWEN_STO_OBJECT *o);

GWENHYWFAR_API
void GWEN_StoObject_SetLockHolder(GWEN_STO_OBJECT *o,
                                  GWEN_STO_CLIENT *cl);

GWENHYWFAR_API
int GWEN_StoObject_GetLockCount(const GWEN_STO_OBJECT *o);

GWENHYWFAR_API
void GWEN_StoObject_IncLockCount(GWEN_STO_OBJECT *o);

GWENHYWFAR_API
void GWEN_StoObject_DecLockCount(GWEN_STO_OBJECT *o);

GWENHYWFAR_API
void GWEN_StoObject_SetLockCount(GWEN_STO_OBJECT *o, int i);


GWENHYWFAR_API
GWEN_STO_CLIENT *GWEN_StoObject_GetOwner(const GWEN_STO_OBJECT *o);

GWENHYWFAR_API
void GWEN_StoObject_SetOwner(GWEN_STO_OBJECT *o, GWEN_STO_CLIENT *cl);

GWENHYWFAR_API
GWEN_STO_OBJECT *GWEN_StoObject_GetOriginalObject(const GWEN_STO_OBJECT *o);

GWENHYWFAR_API
void GWEN_StoObject_SetOriginalObject(GWEN_STO_OBJECT *o,
                                      GWEN_STO_OBJECT *oo);


GWENHYWFAR_API
void GWEN_StoObject_SetModified(GWEN_STO_OBJECT *o, int i);

#endif



