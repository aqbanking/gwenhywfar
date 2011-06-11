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


static GWENHYWFAR_CB int GWEN_List1__defaultSortFn(const void *a, const void *b, int ascending) {
  return 0;
}



GWEN_LIST1 *GWEN_List1_new() {
  GWEN_LIST1 *l;

  GWEN_NEW_OBJECT(GWEN_LIST1, l);
  l->sortFunction=GWEN_List1__defaultSortFn;
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



#if 0
static int GWEN_List1__compar_asc(const void *a, const void *b) {
  const GWEN_LIST1_ELEMENT * const * pse1 = a, * const * pse2 = b;
  const GWEN_LIST1_ELEMENT *se1 = *pse1, *se2 = *pse2;

  return (se1->listPtr->sortFunction)(se1->data, se2->data, 1);
}



static int GWEN_List1__compar_desc(const void *a, const void *b) {
  const GWEN_LIST1_ELEMENT * const * pse1 = a, * const * pse2 = b;
  const GWEN_LIST1_ELEMENT *se1 = *pse1, *se2 = *pse2;

  return (se1->listPtr->sortFunction)(se1->data, se2->data, 0);
}



GWEN_LIST1_SORT_FN GWEN_List1_SetSortFn(GWEN_LIST1 *l, GWEN_LIST1_SORT_FN fn) {
  GWEN_LIST1_SORT_FN oldFn;

  assert(l);
  oldFn=l->sortFunction;
  l->sortFunction=fn;
  return oldFn;
}



void GWEN_List1_Sort(GWEN_LIST1 *l, int ascending) {
  GWEN_LIST1_ELEMENT **tmpEntries;
  GWEN_LIST1_ELEMENT *sentry;
  GWEN_LIST1_ELEMENT **psentry;
  uint32_t count;
  uint32_t i;

  if (l->count<1)
    return;

  count=l->count;

  /* sort entries into a linear pointer list */
  tmpEntries=(GWEN_LIST1_ELEMENT **)malloc((count+1)* sizeof(GWEN_LIST1_ELEMENT*));
  assert(tmpEntries);
  psentry=tmpEntries;

  sentry=l->firstElement;
  while(sentry) {
    GWEN_LIST1_ELEMENT *next;

    *(psentry++)=sentry;
    next=sentry->nextElement;
    sentry->prevElement=NULL;
    sentry->nextElement=NULL;
    sentry->listPtr=l;
    sentry=next;
  } /* while */
  *psentry=NULL;

  /* clear list */
  l->count=0;
  l->firstElement=NULL;
  l->lastElement=NULL;

  /* sort */
  if (ascending)
    qsort(tmpEntries, count, sizeof(GWEN_LIST1_ELEMENT*), GWEN_List1__compar_asc);
  else
    qsort(tmpEntries, count, sizeof(GWEN_LIST1_ELEMENT*), GWEN_List1__compar_desc);

  /* sort entries back into GWEN_LIST1 according to temporary list */
  psentry=tmpEntries;
  /* we use "<=count" because the list contains count+1 elements */
  for (i=0; i<=count; i++) {
    if (*psentry) {
      (*psentry)->listPtr=NULL;
      GWEN_List1_Add(l, *psentry);
    }
    psentry++;
  } /* for */

  free(tmpEntries);
}
#endif









/* -------------------------------------------------------------------------------------------------
 *                                         Sort
 * -------------------------------------------------------------------------------------------------
 */


static int GWEN_List1__compar(const void *a, const void *b) {
  const GWEN_LIST1_SORT_ELEM * const * pse1 = a, * const * pse2 = b;
  const GWEN_LIST1_SORT_ELEM *se1 = *pse1, *se2 = *pse2;
  const GWEN_LIST1_SORT_CTX *ctx=se1->context;

  const GWEN_LIST1_ELEMENT * e1=se1->element;
  const GWEN_LIST1_ELEMENT * e2=se2->element;

  return (ctx->list->sortFunction)(e1->data, e2->data, ctx->param);
}



GWEN_LIST1_SORT_FN GWEN_List1_SetSortFn(GWEN_LIST1 *l, GWEN_LIST1_SORT_FN fn) {
  GWEN_LIST1_SORT_FN oldFn;

  assert(l);
  oldFn=l->sortFunction;
  l->sortFunction=fn;
  return oldFn;
}












GWEN_LIST1_SORT_CTX *GWEN_List1_SortCtx_new(GWEN_LIST1 *list, int param) {
  GWEN_LIST1_SORT_CTX *ctx;

  GWEN_NEW_OBJECT(GWEN_LIST1_SORT_CTX, ctx);
  ctx->list=list;
  ctx->param=param;
  return ctx;
}



void GWEN_List1_SortCtx_free(GWEN_LIST1_SORT_CTX *ctx) {
  if (ctx) {
    GWEN_FREE_OBJECT(ctx);
  }
}



GWEN_LIST1_SORT_ELEM *GWEN_List1_SortElem_new(GWEN_LIST1_SORT_CTX *ctx, GWEN_LIST1_ELEMENT *elem) {
  GWEN_LIST1_SORT_ELEM *e;

  GWEN_NEW_OBJECT(GWEN_LIST1_SORT_ELEM, e);
  e->context=ctx;
  e->element=elem;
  return e;
}



void GWEN_List1_SortElem_free(GWEN_LIST1_SORT_ELEM *e) {
  if (e) {
    GWEN_FREE_OBJECT(e);
  }
}



void GWEN_List1_Sort(GWEN_LIST1 *l, int ascending) {
  GWEN_LIST1_SORT_ELEM **tmpEntries;
  GWEN_LIST1_SORT_ELEM **psentry;
  GWEN_LIST1_ELEMENT *sentry;
  uint32_t count;
  uint32_t i;
  GWEN_LIST1_SORT_CTX *sortContext;

  if (l->count<1)
    return;

  count=l->count;

  sortContext=GWEN_List1_SortCtx_new(l, ascending);

  /* sort entries into a linear pointer list */
  tmpEntries=(GWEN_LIST1_SORT_ELEM **)malloc((count+1)* sizeof(GWEN_LIST1_SORT_ELEM*));
  assert(tmpEntries);
  psentry=tmpEntries;

  sentry=l->firstElement;
  while(sentry) {
    GWEN_LIST1_ELEMENT *next;
    GWEN_LIST1_SORT_ELEM *se;

    se=GWEN_List1_SortElem_new(sortContext, sentry);
    *(psentry++)=se;
    next=sentry->nextElement;
    sentry->prevElement=NULL;
    sentry->nextElement=NULL;
    sentry->listPtr=l;
    sentry=next;
  } /* while */
  *psentry=NULL;

  /* clear list */
  l->count=0;
  l->firstElement=NULL;
  l->lastElement=NULL;

  /* sort */
  qsort(tmpEntries, count, sizeof(GWEN_LIST1_SORT_ELEM*), GWEN_List1__compar);

  /* sort entries back into GWEN_LIST1 according to temporary list */
  psentry=tmpEntries;
  /* we use "<=count" because the list contains count+1 elements */
  for (i=0; i<=count; i++) {
    GWEN_LIST1_SORT_ELEM *se;

    se=*psentry;
    if (se) {
      sentry=se->element;
      sentry->listPtr=NULL;
      GWEN_List1_Add(l, sentry);
      GWEN_List1_SortElem_free(se);
      *psentry=NULL;
    }
    psentry++;
  } /* for */

  free(tmpEntries);
  GWEN_List1_SortCtx_free(sortContext);
}






