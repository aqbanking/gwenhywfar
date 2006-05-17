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

#ifndef GWEN_STO_CLIENT_P_H
#define GWEN_STO_CLIENT_P_H

#include "st_client_be.h"


struct GWEN_STO_CLIENT {
  GWEN_INHERIT_ELEMENT(GWEN_STO_CLIENT)
  GWEN_LIST_ELEMENT(GWEN_STO_CLIENT)

  GWEN_STO_STORAGE *storage;
  GWEN_TYPE_UINT32 id;
  char *userName;

  GWEN_STO_TYPE_LIST *typeList;
  GWEN_STO_OBJECT_LIST *objectList;
  GWEN_STO_LOG_LIST *logList;

  GWEN_IDLIST *openObjectIdList;
  GWEN_IDLIST *openTypeIdList;

};


#endif



