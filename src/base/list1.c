/***************************************************************************
    begin       : Sat Nov 15 2003
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "list1_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>




GWEN_LIST1 *GWEN_List1_new() {
  GWEN_LIST1 *l;

  GWEN_NEW_OBJECT(GWEN_LIST1, l);
  return l;
}


void GWEN_List1_free(GWEN_LIST1 *l) {
  if (l) {
    GWEN_FREE_OBJECT(l);
  }
}



int GWEN_List1_GetCount(const GWEN_LIST1 *l) {
  assert(l);
  return l->count;
}



int GWEN_List1_Add(GWEN_LIST1 *l, GWEN_LIST1_ELEMENT *el) {
  assert(l);
  assert(el);
  if (el->listPtr) {
    /* element is already part of another list */
    DBG_ERROR(GWEN_LOGDOMAIN, "Element is already part of a list");
    assert(0);
    return -1;
  }

  if (l->firstElement==0)
    l->firstElement=el;

  el->prevElement=l->lastElement;
  if (l->lastElement)
    l->lastElement->nextElement=el;
  l->lastElement=el;

  el->listPtr=l;
  l->count++;

  return 0;
}



int GWEN_List1_AddList(GWEN_LIST1 *dest, GWEN_LIST1 *l) {
  GWEN_LIST1_ELEMENT *el;

  assert(dest);
  assert(l);

  while((el=l->firstElement)) {
    GWEN_List1_Del(el);
    GWEN_List1_Add(dest, el);
  }

  return 0;
}



int GWEN_List1_Insert(GWEN_LIST1 *l, GWEN_LIST1_ELEMENT *el) {
  assert(l);
  assert(el);
  if (el->listPtr) {
    /* element is already part of another list */
    DBG_ERROR(GWEN_LOGDOMAIN, "Element is already part of a list");
    return -1;
  }

  el->nextElement=l->firstElement;
  l->firstElement=el;

  if (l->lastElement==0)
    l->lastElement=el;

  if (el->nextElement)
    el->nextElement->prevElement=el;

  el->listPtr=l;
  l->count++;

  return 0;
}



int GWEN_List1_Del(GWEN_LIST1_ELEMENT *el) {
  GWEN_LIST1 *l;

  assert(el);
  l=el->listPtr;

  if (l==0) {
    /* not part of any list */
    DBG_ERROR(GWEN_LOGDOMAIN, "Element is not part of any list");
    return -1;
  }

  /* unlink from previous */
  if (el->prevElement)
    el->prevElement->nextElement=el->nextElement;

  /* unlink from next */
  if (el->nextElement)
    el->nextElement->prevElement=el->prevElement;

  /* unlink from list */
  if (l->firstElement==el)
    l->firstElement=el->nextElement;
  if (l->lastElement==el)
    l->lastElement=el->prevElement;
  l->count--;

  el->nextElement=0;
  el->prevElement=0;
  el->listPtr=0;

  return 0;
}



void *GWEN_List1_GetFirst(const GWEN_LIST1 *l) {
  if (l->firstElement)
    return l->firstElement->data;
  return 0;
}



void *GWEN_List1_GetLast(const GWEN_LIST1 *l) {
  if (l->lastElement)
    return l->lastElement->data;
  return 0;
}






GWEN_LIST1_ELEMENT *GWEN_List1Element_new(void *d) {
  GWEN_LIST1_ELEMENT *el;

  GWEN_NEW_OBJECT(GWEN_LIST1_ELEMENT, el);
  el->data=d;

  return el;
}



void GWEN_List1Element_free(GWEN_LIST1_ELEMENT *el) {
  if (el) {
    if (el->listPtr)
      GWEN_List1_Del(el);
    GWEN_FREE_OBJECT(el);
  }
}



void *GWEN_List1Element_GetData(const GWEN_LIST1_ELEMENT *el) {
  return el->data;
}



void *GWEN_List1Element_GetPrevious(const GWEN_LIST1_ELEMENT *el){
  if (el->prevElement)
    return el->prevElement->data;
  return 0;
}



void *GWEN_List1Element_GetNext(const GWEN_LIST1_ELEMENT *el){
  if (el->nextElement)
    return el->nextElement->data;
  return 0;
}










