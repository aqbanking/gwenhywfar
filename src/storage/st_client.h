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

#ifndef GWEN_STO_CLIENT_H
#define GWEN_STO_CLIENT_H

#include <gwenhywfar/list2.h>

typedef struct GWEN_STO_CLIENT GWEN_STO_CLIENT;

GWEN_LIST2_FUNCTION_LIB_DEFS(GWEN_STO_CLIENT, GWEN_StoClient, GWENHYWFAR_API)

#include <gwenhywfar/st_storage.h>
#include <gwenhywfar/st_type.h>
#include <gwenhywfar/st_find.h>
#include <gwenhywfar/st_object.h>


GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_StoClient_GetId(const GWEN_STO_CLIENT *cl);

GWENHYWFAR_API
const char *GWEN_StoClient_GetUserName(const GWEN_STO_CLIENT *cl);


/** @name Create, Open and Close a Storage
 *
 */
/*@{*/

GWENHYWFAR_API
int GWEN_StoClient_Create(GWEN_STO_CLIENT *cl);

GWENHYWFAR_API
int GWEN_StoClient_Open(GWEN_STO_CLIENT *cl,
                        GWEN_TYPE_UINT32 openFlags);

GWENHYWFAR_API
int GWEN_StoClient_Close(GWEN_STO_CLIENT *cl, const char *reason);
/*@}*/



/** @name Begin and End Edit Sessions
 *
 */
/*@{*/
GWENHYWFAR_API
int GWEN_StoClient_BeginEdit(GWEN_STO_CLIENT *cl);

GWENHYWFAR_API
int GWEN_StoClient_EndEdit(GWEN_STO_CLIENT *cl,
                           GWEN_STO_CLOSEMODE cm);
/*@}*/


/** @name Create, Open and Close Types
 *
 */
/*@{*/
GWENHYWFAR_API
int GWEN_StoClient_CreateType(GWEN_STO_CLIENT *cl,
                              const char *typeName,
                              const char *name,
                              GWEN_STO_TYPE **pts);


GWENHYWFAR_API
int GWEN_StoClient_OpenType(GWEN_STO_CLIENT *cl,
                            const char *typeName,
                            const char *name,
                            GWEN_TYPE_UINT32 openFlags,
                            GWEN_STO_TYPE **pts);

GWENHYWFAR_API
int GWEN_StoClient_CloseType(GWEN_STO_CLIENT *cl,
                             GWEN_STO_TYPE *ts,
                             GWEN_STO_CLOSEMODE cm);
/*@}*/



/** @name Enumerate Objects
 *
 */
/*@{*/
GWENHYWFAR_API
int GWEN_StoClient_FindFirstObject(GWEN_STO_CLIENT *cl,
                                   GWEN_STO_TYPE *ts,
                                   GWEN_STO_FIND **pfnd,
                                   GWEN_TYPE_UINT32 *id);

GWENHYWFAR_API
int GWEN_StoClient_FindNextObject(GWEN_STO_CLIENT *cl,
                                  GWEN_STO_TYPE *ts,
                                  GWEN_STO_FIND *fnd,
                                  GWEN_TYPE_UINT32 *id);

GWENHYWFAR_API
int GWEN_StoClient_CloseFind(GWEN_STO_CLIENT *cl,
                             GWEN_STO_TYPE *ts,
                             GWEN_STO_FIND *fnd);
/*@}*/


/** @name Create, Open and Close Objects
 *
 */
/*@{*/
GWENHYWFAR_API
int GWEN_StoClient_CreateObject(GWEN_STO_CLIENT *cl,
                                GWEN_STO_TYPE *ts,
                                GWEN_STO_OBJECT **po);

GWENHYWFAR_API
int GWEN_StoClient_OpenObject(GWEN_STO_CLIENT *cl,
                              GWEN_STO_TYPE *ts,
                              GWEN_TYPE_UINT32 id,
                              GWEN_TYPE_UINT32 openFlags,
                              GWEN_STO_OBJECT **po);

GWENHYWFAR_API
int GWEN_StoClient_CloseObject(GWEN_STO_CLIENT *cl,
                               GWEN_STO_TYPE *ts,
                               GWEN_STO_OBJECT *o,
                               GWEN_STO_CLOSEMODE cm);

GWENHYWFAR_API
int GWEN_StoClient_LockObject(GWEN_STO_CLIENT *cl,
                              GWEN_STO_TYPE *ts,
                              GWEN_TYPE_UINT32 id,
                              GWEN_STO_LOCKMODE lm);
/*@}*/



#endif



