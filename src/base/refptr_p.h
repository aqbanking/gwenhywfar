/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Jan 25 2004
    copyright   : (C) 2004 by Martin Preuss
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


#ifndef GWEN_REFPTR_P_H
#define GWEN_REFPTR_P_H


#include <gwenhywfar/refptr.h>


typedef struct GWEN_REFPTR_POBJECT GWEN_REFPTR_POBJECT;


struct GWEN_REFPTR_INFO {
  uint32_t refCount;
  uint32_t flags;
  GWEN_REFPTR_INFO_FREE_FN freeFn;
  GWEN_REFPTR_INFO_DUP_FN dupFn;
};



struct GWEN_REFPTR_POBJECT {
  uint32_t refCount;
  uint32_t flags;
  GWEN_REFPTR_INFO *infoPtr;
  void *ptr;
};



struct GWEN_REFPTR {
  GWEN_REFPTR_POBJECT *objectPtr;
};



GWEN_REFPTR_POBJECT *GWEN_RefPtrObject_new(void *dp, GWEN_REFPTR_INFO *rpi);
void GWEN_RefPtrObject_free(GWEN_REFPTR_POBJECT *rpo);
void GWEN_RefPtrObject_Attach(GWEN_REFPTR_POBJECT *rpo);


#endif



