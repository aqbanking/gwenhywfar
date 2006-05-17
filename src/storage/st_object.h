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

#ifndef GWEN_STO_OBJECT_H
#define GWEN_STO_OBJECT_H

#include <gwenhywfar/types.h>
#include <gwenhywfar/list2.h>

#define GWEN_STO_OBJECT_FLAGS_DELETED   0x00000001
#define GWEN_STO_OBJECT_FLAGS_TO_DELETE 0x00000002


typedef struct GWEN_STO_OBJECT GWEN_STO_OBJECT;


GWEN_LIST2_FUNCTION_LIB_DEFS(GWEN_STO_OBJECT, GWEN_StoObject, GWENHYWFAR_API)


#include <gwenhywfar/st_type.h>


GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_StoObject_GetId(const GWEN_STO_OBJECT *o);

GWENHYWFAR_API
GWEN_STO_TYPE *GWEN_StoObject_GetType(const GWEN_STO_OBJECT *o);

GWENHYWFAR_API
int GWEN_StoObject_GetRefCount(const GWEN_STO_OBJECT *o);

GWENHYWFAR_API
int GWEN_StoObject_IncRefCount(GWEN_STO_OBJECT *o);

GWENHYWFAR_API
int GWEN_StoObject_DecRefCount(GWEN_STO_OBJECT *o);

GWENHYWFAR_API
void GWEN_StoObject_SetCharValue(GWEN_STO_OBJECT *o,
                                 const char *varName,
                                 const char *value,
                                 int overwrite);

GWENHYWFAR_API
const char *GWEN_StoObject_GetCharValue(const GWEN_STO_OBJECT *o,
                                        const char *varName,
                                        int idx,
                                        const char *defValue);

#endif



