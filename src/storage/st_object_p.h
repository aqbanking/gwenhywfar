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

#ifndef GWEN_STO_OBJECT_P_H
#define GWEN_STO_OBJECT_P_H

#include "st_object_be.h"


struct GWEN_STO_OBJECT {
  GWEN_INHERIT_ELEMENT(GWEN_STO_OBJECT)
  GWEN_LIST_ELEMENT(GWEN_STO_OBJECT)

  GWEN_STO_TYPE *typ;
  GWEN_TYPE_UINT32 id;
  GWEN_TYPE_UINT32 flags;

  GWEN_DB_NODE *dbDataNode;
  int refCount;

  GWEN_STO_CLIENT *lockHolder;
  int lockCount;

  GWEN_STO_CLIENT *owner;

  GWEN_STO_OBJECT *origObject;

  int openCount;
  int modified;
};


#endif



