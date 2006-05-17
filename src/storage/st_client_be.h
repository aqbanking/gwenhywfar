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

#ifndef GWEN_STO_CLIENT_BE_H
#define GWEN_STO_CLIENT_BE_H

#include <gwenhywfar/st_client.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>


GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_STO_CLIENT, GWENHYWFAR_API)
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_STO_CLIENT, GWEN_StoClient, GWENHYWFAR_API)

#include <gwenhywfar/st_type_be.h>
#include <gwenhywfar/st_object_be.h>
#include <gwenhywfar/st_log.h>
#include <gwenhywfar/idlist.h>


GWENHYWFAR_API
GWEN_STO_CLIENT *GWEN_StoClient_new(GWEN_STO_STORAGE *st,
                                    GWEN_TYPE_UINT32 id,
                                    const char *userName);

GWENHYWFAR_API
void GWEN_StoClient_free(GWEN_STO_CLIENT *cl);


GWENHYWFAR_API
GWEN_STO_TYPE_LIST *GWEN_StoClient_GetTypeList(const GWEN_STO_CLIENT *cl);

GWENHYWFAR_API
void GWEN_StoClient_AddType(GWEN_STO_CLIENT *cl, GWEN_STO_TYPE *ty);

GWENHYWFAR_API
GWEN_STO_TYPE *GWEN_StoClient_FindTypeByName(const GWEN_STO_CLIENT *cl,
                                             const char *typeName,
                                             const char *name);


GWENHYWFAR_API
GWEN_STO_OBJECT_LIST *GWEN_StoClient_GetObjectList(const GWEN_STO_CLIENT *cl);

GWENHYWFAR_API
void GWEN_StoClient_AddObject(GWEN_STO_CLIENT *cl, GWEN_STO_OBJECT *o);

GWENHYWFAR_API
GWEN_STO_OBJECT *GWEN_StoClient_FindObject(const GWEN_STO_CLIENT *cl,
                                           GWEN_TYPE_UINT32 id);

GWENHYWFAR_API
GWEN_IDLIST *GWEN_StoClient_GetOpenObjectIdList(const GWEN_STO_CLIENT *cl);

GWENHYWFAR_API
void GWEN_StoClient_AddOpenObjectId(GWEN_STO_CLIENT *cl,
                                    GWEN_TYPE_UINT32 oid);

GWENHYWFAR_API
void GWEN_StoClient_DelOpenObjectId(GWEN_STO_CLIENT *cl,
                                    GWEN_TYPE_UINT32 oid);

GWENHYWFAR_API
GWEN_IDLIST *GWEN_StoClient_GetOpenTypeIdList(const GWEN_STO_CLIENT *cl);

GWENHYWFAR_API
void GWEN_StoClient_AddOpenTypeId(GWEN_STO_CLIENT *cl,
                                  GWEN_TYPE_UINT32 oid);

GWENHYWFAR_API
void GWEN_StoClient_DelOpenTypeId(GWEN_STO_CLIENT *cl,
                                  GWEN_TYPE_UINT32 oid);

GWENHYWFAR_API
GWEN_STO_LOG_LIST *GWEN_StoClient_GetLogList(const GWEN_STO_CLIENT *cl);

GWENHYWFAR_API
void GWEN_StoClient_AddLog(GWEN_STO_CLIENT *cl, GWEN_STO_LOG *log);

#endif



