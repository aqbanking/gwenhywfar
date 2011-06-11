/***************************************************************************
 begin       : Sat Jun 28 2003
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


#ifndef GWEN_LIST1_P_H
#define GWEN_LIST1_P_H


#include "list1.h"



struct GWEN_LIST1_ELEMENT {
  GWEN_LIST1 *listPtr;
  void *data;
  GWEN_LIST1_ELEMENT *prevElement;
  GWEN_LIST1_ELEMENT *nextElement;
};


struct GWEN_LIST1 {
  uint32_t count;
  GWEN_LIST1_ELEMENT *firstElement;
  GWEN_LIST1_ELEMENT *lastElement;

  GWEN_LIST1_SORT_FN sortFunction;
};




typedef struct GWEN_LIST1_SORT_CTX GWEN_LIST1_SORT_CTX;
struct GWEN_LIST1_SORT_CTX {
  GWEN_LIST1 *list;
  int param;
};
static GWEN_LIST1_SORT_CTX *GWEN_List1_SortCtx_new(GWEN_LIST1 *list, int param);
static void GWEN_List1_SortCtx_free(GWEN_LIST1_SORT_CTX *ctx);




typedef struct GWEN_LIST1_SORT_ELEM GWEN_LIST1_SORT_ELEM;
struct GWEN_LIST1_SORT_ELEM {
  GWEN_LIST1_SORT_CTX *context;
  GWEN_LIST1_ELEMENT *element;
};
static GWEN_LIST1_SORT_ELEM *GWEN_List1_SortElem_new(GWEN_LIST1_SORT_CTX *ctx, GWEN_LIST1_ELEMENT *elem);
static void GWEN_List1_SortElem_free(GWEN_LIST1_SORT_ELEM *e);




#endif


