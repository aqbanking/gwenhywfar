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

#ifndef GWEN_SMPSTO_TYPE_BE_H
#define GWEN_SMPSTO_TYPE_BE_H

#include <gwenhywfar/st_storage_be.h>
#include <gwenhywfar/st_type_be.h>
#include <gwenhywfar/idlist.h>


GWENHYWFAR_API
GWEN_STO_TYPE *GWEN_SmpStoType_new(GWEN_STO_STORAGE *storage,
				   GWEN_TYPE_UINT32 id,
				   const char *typeName,
				   const char *name);

GWENHYWFAR_API
GWEN_STO_TYPE *GWEN_SmpStoType_dup(const GWEN_STO_TYPE *oty);

GWENHYWFAR_API
GWEN_IDLIST *GWEN_SmpStoType_GetObjectIdList(const GWEN_STO_TYPE *ty);





#endif



