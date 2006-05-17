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

#ifndef GWEN_STO_FIND_BE_H
#define GWEN_STO_FIND_BE_H

#include <gwenhywfar/st_find.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>


GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_STO_FIND, GWENHYWFAR_API)
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_STO_FIND, GWEN_StoFind, GWENHYWFAR_API)


GWENHYWFAR_API
GWEN_STO_FIND *GWEN_StoFind_new(GWEN_TYPE_UINT32 id);

GWENHYWFAR_API
void GWEN_StoFind_free(GWEN_STO_FIND *fnd);

GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_StoFind_GetId(const GWEN_STO_FIND *fnd);


#endif





