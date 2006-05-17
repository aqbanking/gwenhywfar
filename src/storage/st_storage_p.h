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

#ifndef GWEN_STO_STORAGE_P_H
#define GWEN_STO_STORAGE_P_H

#include "st_storage_be.h"


struct GWEN_STO_STORAGE {
  GWEN_INHERIT_ELEMENT(GWEN_STO_STORAGE)
  GWEN_LIST_ELEMENT(GWEN_STO_STORAGE)

  char *typeName;
  char *address;

  GWEN_STO_CLIENT_LIST *clientList;
  GWEN_STO_TYPE_LIST *typeList;
  GWEN_STO_OBJECT_LIST *objectList;

  GWEN_STO_STORAGE_REGISTERCLIENT_FN registerClientFn;
  GWEN_STO_STORAGE_UNREGISTERCLIENT_FN unregisterClientFn;

  GWEN_STO_STORAGE_CREATE_FN createFn;
  GWEN_STO_STORAGE_OPEN_FN openFn;
  GWEN_STO_STORAGE_CLOSE_FN closeFn;

  GWEN_STO_STORAGE_BEGINEDIT_FN beginEditFn;
  GWEN_STO_STORAGE_ENDEDIT_FN endEditFn;

  GWEN_STO_STORAGE_CREATETYPE_FN createTypeFn;
  GWEN_STO_STORAGE_OPENTYPE_FN openTypeFn;
  GWEN_STO_STORAGE_CLOSETYPE_FN closeTypeFn;

  GWEN_STO_STORAGE_FINDFIRSTOBJECT_FN findFirstObjectFn;
  GWEN_STO_STORAGE_FINDNEXTOBJECT_FN findNextObjectFn;
  GWEN_STO_STORAGE_CLOSEFIND_FN closeFindFn;

  GWEN_STO_STORAGE_CREATEOBJECT_FN createObjectFn;
  GWEN_STO_STORAGE_OPENOBJECT_FN openObjectFn;
  GWEN_STO_STORAGE_CLOSEOBJECT_FN closeObjectFn;
  GWEN_STO_STORAGE_DELETEOBJECT_FN deleteObjectFn;
  GWEN_STO_STORAGE_LOCKOBJECT_FN lockObjectFn;

  GWEN_STO_STORAGE_LOG_CB logCb;

};


#endif



