/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Fri Feb 07 2003
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


#ifndef GWENHYWFAR_FILTER_H
#define GWENHYWFAR_FILTER_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/ringbuffer.h>
#include <gwenhywfar/inherit.h>


typedef struct GWEN_FILTER GWEN_FILTER;


GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_FILTER, GWENHYWFAR_API)


typedef enum {
  GWEN_Filter_ResultOk=0,
  GWEN_Filter_ResultNeedMore,
  GWEN_Filter_ResultFull,
  GWEN_Filter_ResultError
} GWEN_FILTER_RESULT;


typedef GWEN_FILTER_RESULT (*GWEN_FILTER_WORKFN)(GWEN_FILTER *f);


GWEN_FILTER *GWEN_Filter_new(const char *name);
void GWEN_Filter_free(GWEN_FILTER *f);
GWEN_RINGBUFFER *GWEN_Filter_GetInBuffer(const GWEN_FILTER *f);
GWEN_RINGBUFFER *GWEN_Filter_GetOutBuffer(const GWEN_FILTER *f);

GWEN_FILTER_RESULT GWEN_Filter_Work(GWEN_FILTER *f, int oneLoop);


void GWEN_Filter_SetWorkFn(GWEN_FILTER *f, GWEN_FILTER_WORKFN fn);


void GWEN_Filter_AppendNext(GWEN_FILTER *fPredecessor, GWEN_FILTER *fNew);



#endif










