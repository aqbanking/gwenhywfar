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


#ifndef GWENHYWFAR_FILTER_P_H
#define GWENHYWFAR_FILTER_P_H


#define GWEN_FILTER_BUFFERSIZE 1024

#include "filter.h"
#include <gwenhywfar/misc.h>


GWEN_LIST_FUNCTION_DEFS(GWEN_FILTER, GWEN_Filter)


struct GWEN_FILTER {
  GWEN_INHERIT_ELEMENT(GWEN_FILTER)
  GWEN_LIST_ELEMENT(GWEN_FILTER)
  char *filterName;
  GWEN_FILTER_LIST *nextElements;
  GWEN_RINGBUFFER *inBuffer;
  GWEN_RINGBUFFER *outBuffer;

  GWEN_FILTER_WORKFN workFn;
};



GWEN_FILTER_RESULT GWEN_Filter__Work(GWEN_FILTER *f);




#endif










