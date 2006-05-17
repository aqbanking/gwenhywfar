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

#ifndef GWEN_STO_TYPE_BE_H
#define GWEN_STO_TYPE_BE_H

#include <gwenhywfar/st_type.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>


GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_STO_TYPE, GWEN_StoType, GWENHYWFAR_API)
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_STO_TYPE, GWENHYWFAR_API)

#include <gwenhywfar/st_storage_be.h>


GWENHYWFAR_API
GWEN_STO_TYPE *GWEN_StoType_new(GWEN_STO_STORAGE *storage,
                                GWEN_TYPE_UINT32 id,
                                const char *typeName,
                                const char *name);

GWENHYWFAR_API
GWEN_STO_TYPE *GWEN_StoType_dup(const GWEN_STO_TYPE *oty);

GWENHYWFAR_API
void GWEN_StoType_free(GWEN_STO_TYPE *ty);

GWENHYWFAR_API
int GWEN_StoType_GetOpenCount(const GWEN_STO_TYPE *ty);

GWENHYWFAR_API
int GWEN_StoType_IncOpenCount(GWEN_STO_TYPE *ty);

GWENHYWFAR_API
int GWEN_StoType_DecOpenCount(GWEN_STO_TYPE *ty);

GWENHYWFAR_API
void GWEN_StoType_SetOpenCount(GWEN_STO_TYPE *ty, int i);

GWENHYWFAR_API
int GWEN_StoType_IsModified(const GWEN_STO_TYPE *ty);

GWENHYWFAR_API
void GWEN_StoType_SetModified(GWEN_STO_TYPE *ty, int i);

GWENHYWFAR_API
GWEN_STO_TYPE *GWEN_StoType_GetOriginalType(const GWEN_STO_TYPE *ty);

GWENHYWFAR_API
void GWEN_StoType_SetOriginalType(GWEN_STO_TYPE *ty,
                                  GWEN_STO_TYPE *oty);

GWENHYWFAR_API
GWEN_STO_CLIENT *GWEN_StoType_GetOwner(const GWEN_STO_TYPE *ty);

GWENHYWFAR_API
void GWEN_StoType_SetOwner(GWEN_STO_TYPE *ty, GWEN_STO_CLIENT *cl);

GWENHYWFAR_API
void GWEN_StoType_SetVarList(GWEN_STO_TYPE *ty,
                             const GWEN_STO_VARDEF_LIST *vl);


/** Takes over ownership of the var
 */
GWENHYWFAR_API
int GWEN_StoType_AddVarNoLog(GWEN_STO_TYPE *ty, GWEN_STO_VARDEF *vdef);


#endif



