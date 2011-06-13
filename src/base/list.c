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

#define DISABLE_DEBUGLOG


#include "list_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>


GWEN_INHERIT_FUNCTIONS(GWEN_LIST)



GWEN_LIST_ENTRY *GWEN_ListEntry_new(void){
  GWEN_LIST_ENTRY *le;

  GWEN_NEW_OBJECT(GWEN_LIST_ENTRY, le);
  le->usage=1;
  return le;
}



void GWEN_ListEntry_free(GWEN_LIST_ENTRY *le){
  if (le) {
    if (le->usage) {
      le->usage--;
      if (le->usage==0) {
        /* unlink */
        le->previous=0;
	le->next=0;
        DBG_VERBOUS(GWEN_LOGDOMAIN, "Freeing entry");
        GWEN_RefPtr_free(le->dataPtr);
        /* really free */
        GWEN_FREE_OBJECT(le);
      }
    }
  }
}



GWEN__LISTPTR *GWEN__ListPtr_new(void){
  GWEN__LISTPTR *lp;

  GWEN_NEW_OBJECT(GWEN__LISTPTR, lp);
  lp->refCount=1;
  return lp;
}



void GWEN__ListPtr_free(GWEN__LISTPTR *lp){
  if (lp) {
    assert(lp->refCount);
    if (--(lp->refCount)==0) {
      GWEN__ListPtr_Clear(lp);
      GWEN_FREE_OBJECT(lp);
    }
  }
}



void GWEN__ListPtr_Attach(GWEN__LISTPTR *lp){
  assert(lp);
  assert(lp->refCount);
  lp->refCount++;
}



void GWEN__ListPtr_Clear(GWEN__LISTPTR *lp){
  GWEN_LIST_ENTRY *le;

  assert(lp);
  le=lp->first;
  while(le) {
    GWEN_LIST_ENTRY *nle;

    nle=le->next;
    GWEN_ListEntry_free(le);
    le=nle;
  } /* while */
  lp->first=0;
  lp->last=0;
  lp->size=0;
}



GWEN__LISTPTR *GWEN__ListPtr_dup(GWEN__LISTPTR *lp){
  GWEN__LISTPTR *nlp;
  GWEN_LIST_ENTRY *le;

  nlp=GWEN__ListPtr_new();
  assert(lp);
  le=lp->first;
  while(le) {
    GWEN_LIST_ENTRY *nle;

    nle=GWEN_ListEntry_new();
    if (le->dataPtr)
      nle->dataPtr=GWEN_RefPtr_dup(le->dataPtr);
    /* push back */
    nle->previous=nlp->last;
    if (nlp->last)
      nlp->last->next=nle;
    nlp->last=nle;
    if (!(nlp->first))
      nlp->first=nle;
    nlp->size++;
    nle->linkCount=le->linkCount;

    le=le->next;
  } /* while */

  return nlp;
}








GWEN_LIST *GWEN_List_new(void){
  GWEN_LIST *l;

  GWEN_NEW_OBJECT(GWEN_LIST, l);
  GWEN_INHERIT_INIT(GWEN_LIST, l);
  l->listPtr=GWEN__ListPtr_new();
  return l;
}



void GWEN_List_free(GWEN_LIST *l){
  if (l) {
    GWEN_INHERIT_FINI(GWEN_LIST, l);
    GWEN__ListPtr_free(l->listPtr);
    GWEN_RefPtrInfo_free(l->refPtrInfo);
    GWEN_FREE_OBJECT(l);
  }
}



GWEN_LIST *GWEN_List_dup(const GWEN_LIST *l){
  GWEN_LIST *nl;
  
  assert(l);
  assert(l->listPtr);
  GWEN_NEW_OBJECT(GWEN_LIST, nl);
  GWEN_INHERIT_INIT(GWEN_LIST, nl);
  nl->listPtr=l->listPtr;
  GWEN__ListPtr_Attach(nl->listPtr);
  return nl;
}



GWEN_REFPTR_INFO *GWEN_List_GetRefPtrInfo(const GWEN_LIST *l){
  assert(l);
  return l->refPtrInfo;
}



void GWEN_List_SetRefPtrInfo(GWEN_LIST *l, GWEN_REFPTR_INFO *rpi){
  assert(l);
  if (rpi)
    GWEN_RefPtrInfo_Attach(rpi);
  GWEN_RefPtrInfo_free(l->refPtrInfo);
  l->refPtrInfo=rpi;
}



void GWEN_List_PushBackRefPtr(GWEN_LIST *l, GWEN_REFPTR *rp){
  GWEN_LIST_ENTRY *le;
  GWEN__LISTPTR *lp;

  if (l->listPtr->refCount>1) {
    GWEN__LISTPTR *nlp;

    /* only copy the list if someone else is using it */
    nlp=GWEN__ListPtr_dup(l->listPtr);
    GWEN__ListPtr_free(l->listPtr);
    l->listPtr=nlp;
  }
  lp=l->listPtr;

  le=GWEN_ListEntry_new();
  le->dataPtr=rp;
  le->previous=lp->last;
  if (lp->last)
    lp->last->next=le;
  lp->last=le;
  if (!(lp->first))
    lp->first=le;
  lp->size++;
  le->linkCount=1;
}



void GWEN_List_PushBack(GWEN_LIST *l, void *p){
  GWEN_List_PushBackRefPtr(l, GWEN_RefPtr_new(p, l->refPtrInfo));
}



void GWEN_List_PushFrontRefPtr(GWEN_LIST *l, GWEN_REFPTR *rp){
  GWEN_LIST_ENTRY *le;
  GWEN__LISTPTR *lp;

  if (l->listPtr->refCount>1) {
    GWEN__LISTPTR *nlp;

    /* only copy the list if someone else is using it */
    nlp=GWEN__ListPtr_dup(l->listPtr);
    GWEN__ListPtr_free(l->listPtr);
    l->listPtr=nlp;
  }
  lp=l->listPtr;

  le=GWEN_ListEntry_new();
  le->dataPtr=rp;
  le->next=lp->first;
  if (lp->first)
    lp->first->previous=le;
  lp->first=le;
  if (!(lp->last))
    lp->last=le;
  lp->size++;
  le->linkCount=1;
}



void GWEN_List_PushFront(GWEN_LIST *l, void *p){
  GWEN_List_PushFrontRefPtr(l, GWEN_RefPtr_new(p, l->refPtrInfo));
}



void *GWEN_List_GetFront(const GWEN_LIST *l){
  assert(l);
  assert(l->listPtr);
  if (l->listPtr->first)
    return GWEN_RefPtr_GetData(l->listPtr->first->dataPtr);
  return 0;
}



GWEN_REFPTR *GWEN_List_GetFrontRefPtr(const GWEN_LIST *l){
  assert(l);
  assert(l->listPtr);
  if (l->listPtr->first)
    return l->listPtr->first->dataPtr;
  return 0;
}



void *GWEN_List_GetBack(const GWEN_LIST *l){
  assert(l);
  assert(l->listPtr);
  if (l->listPtr->last)
    return GWEN_RefPtr_GetData(l->listPtr->last->dataPtr);
  return 0;
}



GWEN_REFPTR *GWEN_List_GetBackRefPtr(const GWEN_LIST *l){
  assert(l);
  assert(l->listPtr);
  if (l->listPtr->last)
    return l->listPtr->last->dataPtr;
  return 0;
}



unsigned int GWEN_List_GetSize(const GWEN_LIST *l){
  assert(l);
  assert(l->listPtr);
  return l->listPtr->size;
}

int GWEN_List_IsEmpty(const GWEN_LIST *l) {
  return GWEN_List_GetSize(l) == 0;
}


void GWEN_List_PopBack(GWEN_LIST *l){
  GWEN_LIST_ENTRY *le;
  GWEN__LISTPTR *lp;

  assert(l);
  assert(l->listPtr);
  if (l->listPtr->last==0)
    return;
  if (l->listPtr->refCount>1) {
    GWEN__LISTPTR *nlp;

    /* only copy the list if someone else is using it */
    nlp=GWEN__ListPtr_dup(l->listPtr);
    GWEN__ListPtr_free(l->listPtr);
    l->listPtr=nlp;
  }
  lp=l->listPtr;

  le=lp->last;
  if (le) {
    le->linkCount=0;
    lp->last=le->previous;
    if (le->previous) {
      le->previous->next=0;
    }
    else {
      lp->last=0;
      lp->first=0;
    }
    GWEN_ListEntry_free(le);
    lp->size--;
  }
}



void GWEN_List_PopFront(GWEN_LIST *l){
  GWEN_LIST_ENTRY *le;
  GWEN__LISTPTR *lp;

  assert(l);
  assert(l->listPtr);
  if (l->listPtr->first==0)
    return;
  if (l->listPtr->refCount>1) {
    GWEN__LISTPTR *nlp;

    /* only copy the list if someone else is using it */
    nlp=GWEN__ListPtr_dup(l->listPtr);
    GWEN__ListPtr_free(l->listPtr);
    l->listPtr=nlp;
  }
  lp=l->listPtr;

  le=lp->first;
  if (le) {
    le->linkCount=0;
    lp->first=le->next;
    if (le->next) {
      le->next->previous=0;
    }
    else {
      lp->first=0;
      lp->last=0;
    }
    GWEN_ListEntry_free(le);
    lp->size--;
  }
}



void GWEN_List_Clear(GWEN_LIST *l){
  /* GWEN__LISTPTR *lp; */

  assert(l);
  if (l->listPtr->refCount>1) {
    GWEN__LISTPTR *nlp;

    /* only copy the list if someone else is using it */
    nlp=GWEN__ListPtr_dup(l->listPtr);
    GWEN__ListPtr_free(l->listPtr);
    l->listPtr=nlp;
  }
  else
    GWEN__ListPtr_Clear(l->listPtr);
}



void *GWEN_List_ForEach(GWEN_LIST *l, 
			GWEN_LIST_FOREACH_CB fn, void *user_data){
  GWEN_LIST_ITERATOR *it;
  void *el;
  assert(l);

  it=GWEN_List_First(l);
  if (!it)
    return 0;
  el=GWEN_ListIterator_Data(it);
  while(el) {
    el=fn(el, user_data);
    if (el) {
      GWEN_ListIterator_free(it);
      return el;
    }
    el=GWEN_ListIterator_Next(it);
  }
  GWEN_ListIterator_free(it);
  return 0;
}



void GWEN_List_Unshare(GWEN_LIST *l) {
  if (l->listPtr->refCount>1) {
    GWEN__LISTPTR *nlp;

    /* only copy the list if someone else is using it */
    nlp=GWEN__ListPtr_dup(l->listPtr);
    GWEN__ListPtr_free(l->listPtr);
    l->listPtr=nlp;
  }
}



void GWEN_List_Erase(GWEN_LIST *l, GWEN_LIST_ITERATOR *it){
  GWEN_LIST_ENTRY *current;
  GWEN__LISTPTR *lp;

  assert(l);
  assert(l->listPtr);
  if (l->listPtr->refCount>1) {
    GWEN_LIST_ENTRY *tle;
    GWEN__LISTPTR *nlp;
    int i;

    /* find the position of the iterator within current list */
    tle=it->current;
    assert(tle);
    i=0;
    while(tle->previous) {
      i++;
      tle=tle->previous;
    }

    /* copy the list */
    nlp=GWEN__ListPtr_dup(l->listPtr);
    GWEN__ListPtr_free(l->listPtr);
    l->listPtr=nlp;

    /* seek and set the iterator position */
    tle=l->listPtr->first;
    assert(tle);
    while(tle && i--) {
      tle=tle->next;
    }
    assert(tle);
    it->current=tle;
  }
  lp=l->listPtr;

  assert(it);
  if (it->current) {
    current=it->current;
    if (it->current->linkCount==1) {
      /* unlink from list */
      if (lp->first==current)
        lp->first=current->next;
      if (lp->last==current)
        lp->last=current->previous;

      /* unlink from next */
      if (current->next) {
        it->current=current->next;
        current->next->usage++;
        current->next->previous=current->previous;
      }
      else
        it->current=0;
      /* unlink from previous */
      if (current->previous)
        current->previous->next=current->next;
      /* free */
      current->usage--;
      GWEN_ListEntry_free(current);
      lp->size--;
    }
    else {
      /* move iterator forwards even if the current entry has not
       * been deleted. Thus making the return condition clear to the
       * caller.
       */
      if (current->next) {
        it->current=current->next;
        current->next->usage++;
      }
      else
        it->current=0;
      current->usage--;
      it->current->linkCount--;
    }
  }
}



GWEN_LIST_ITERATOR *GWEN_List_FindIter(GWEN_LIST *l, const void *p) {
 GWEN_LIST_ITERATOR *li;

  li=GWEN_List_First(l);
  if (li) {
    void *d;

    d=GWEN_ListIterator_Data(li);
    while(d) {
      if (d==p) {
        return li;
      }
      d=GWEN_ListIterator_Next(li);
    }
    GWEN_ListIterator_free(li);
  }
  return 0;
}



const void *GWEN_List_Contains(GWEN_LIST *l, const void *p) {
  GWEN_LIST_ITERATOR *li;

  li = GWEN_List_FindIter(l, p);
  if (li) {
    GWEN_ListIterator_free(li);
    return p;
  }
  return 0;
}



void GWEN_List_Remove(GWEN_LIST *l, const void *p) {
  GWEN_LIST_ITERATOR *li;

  li = GWEN_List_FindIter(l, p);
  if (li) {
    GWEN_List_Erase(l, li);
    GWEN_ListIterator_free(li);
  }
}



GWEN_LIST_ITERATOR *GWEN_List_First(const GWEN_LIST *l){
  GWEN_LIST_ITERATOR *li;

  assert(l);
  assert(l->listPtr);
  if (l->listPtr->first==0)
    return 0;
  li=GWEN_ListIterator_new(l);
  li->current=l->listPtr->first;
  if (li->current) {
    li->current->usage++;
  }
  return li;
}



GWEN_LIST_ITERATOR *GWEN_List_Last(const GWEN_LIST *l){
  GWEN_LIST_ITERATOR *li;

  assert(l);
  assert(l->listPtr);
  if (l->listPtr->last==0)
    return 0;
  li=GWEN_ListIterator_new(l);
  li->current=l->listPtr->last;
  if (li->current)
    li->current->usage++;
  return li;
}



void GWEN_List_Dump(const GWEN_LIST *l, FILE *f, unsigned int indent){
  GWEN_LIST_ENTRY *le;
  unsigned int i;

  fprintf(f, "List contains %d entries\n", l->listPtr->size);
  le=l->listPtr->first;
  while(le) {
    for (i=0; i<indent; i++) fprintf(f, " ");
    fprintf(f, "List entry %p\n", (void*)le);
    for (i=0; i<indent; i++) fprintf(f, " ");
    fprintf(f, " Usage   : %d\n", le->usage);
    for (i=0; i<indent; i++) fprintf(f, " ");
    fprintf(f, " Previous: %p\n", (void*)le->previous);
    for (i=0; i<indent; i++) fprintf(f, " ");
    fprintf(f, " Next    : %p\n", (void*)le->next);
    for (i=0; i<indent; i++) fprintf(f, " ");
    fprintf(f, " Data    : %p\n", (void*)GWEN_RefPtr_GetData(le->dataPtr));
    le=le->next;
  } /* while */
}




GWEN_LIST_ITERATOR *GWEN_ListIterator_new(const GWEN_LIST *l){
  GWEN_LIST_ITERATOR *li;

  GWEN_NEW_OBJECT(GWEN_LIST_ITERATOR, li);
  li->list=l;
  return li;
}



void GWEN_ListIterator_free(GWEN_LIST_ITERATOR *li){
  if (li) {
    if (li->current)
      GWEN_ListEntry_free(li->current);
    GWEN_FREE_OBJECT(li);
  }
}



void *GWEN_ListIterator_Previous(GWEN_LIST_ITERATOR *li){
  GWEN_REFPTR *rp;

  assert(li);
  rp=GWEN_ListIterator_PreviousRefPtr(li);
  if (!rp)
    return 0;
  return GWEN_RefPtr_GetData(rp);
}



GWEN_REFPTR *GWEN_ListIterator_PreviousRefPtr(GWEN_LIST_ITERATOR *li){
  GWEN_LIST_ENTRY *le;

  assert(li);

  le=li->current;
  if (le)
    le=le->previous;
  if (li->current)
    GWEN_ListEntry_free(li->current);
  li->current=le;
  if (le) {
    le->usage++;
    return le->dataPtr;
  }
  return 0;
}



void *GWEN_ListIterator_Next(GWEN_LIST_ITERATOR *li){
  GWEN_REFPTR *rp;

  assert(li);
  rp=GWEN_ListIterator_NextRefPtr(li);
  if (!rp)
    return 0;
  return GWEN_RefPtr_GetData(rp);
}



GWEN_REFPTR *GWEN_ListIterator_NextRefPtr(GWEN_LIST_ITERATOR *li){
  GWEN_LIST_ENTRY *le;

  assert(li);

  le=li->current;
  if (le)
    le=le->next;
  if (li->current)
    GWEN_ListEntry_free(li->current);
  li->current=le;
  if (le) {
    le->usage++;
    return le->dataPtr;
  }
  return 0;
}



void *GWEN_ListIterator_Data(GWEN_LIST_ITERATOR *li){
  assert(li);

  if (li->current)
    return GWEN_RefPtr_GetData(li->current->dataPtr);
  return 0;
}



GWEN_REFPTR *GWEN_ListIterator_DataRefPtr(GWEN_LIST_ITERATOR *li){
  assert(li);

  if (li->current)
    return li->current->dataPtr;
  return 0;
}



void GWEN_ListIterator_IncLinkCount(GWEN_LIST_ITERATOR *li){
  assert(li);

  if (li->current)
    li->current->linkCount++;
}



unsigned int GWEN_ListIterator_GetLinkCount(const GWEN_LIST_ITERATOR *li){
  assert(li);

  assert(li->current);
  return li->current->linkCount;
}








/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                 ConstList
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



GWEN_CONSTLIST *GWEN_ConstList_new(void){
  return GWEN_List_new();
}



void GWEN_ConstList_free(GWEN_CONSTLIST *l){
  GWEN_List_free(l);
}



void GWEN_ConstList_PushBack(GWEN_CONSTLIST *l, const void *p){
  GWEN_List_PushBack(l, (void*)p);
}



void GWEN_ConstList_PushFront(GWEN_CONSTLIST *l, const void *p){
  GWEN_List_PushFront(l, (void*)p);
}



const void *GWEN_ConstList_GetFront(const GWEN_CONSTLIST *l){
  return GWEN_List_GetFront(l);
}



const void *GWEN_ConstList_GetBack(const GWEN_CONSTLIST *l){
  return GWEN_List_GetBack(l);
}



unsigned int GWEN_ConstList_GetSize(const GWEN_CONSTLIST *l){
  return GWEN_List_GetSize(l);
}

int GWEN_ConstList_IsEmpty(const GWEN_LIST *l) {
  return GWEN_ConstList_GetSize(l) == 0;
}



void GWEN_ConstList_PopBack(GWEN_CONSTLIST *l){
  GWEN_List_PopBack(l);
}



void GWEN_ConstList_PopFront(GWEN_CONSTLIST *l){
  GWEN_List_PopFront(l);
}



void GWEN_ConstList_Erase(GWEN_CONSTLIST *l, GWEN_CONSTLIST_ITERATOR *it){
  GWEN_List_Erase(l, it);
}



void GWEN_ConstList_Clear(GWEN_CONSTLIST *l){
  GWEN_List_Clear(l);
}


const void *GWEN_ConstList_ForEach(GWEN_CONSTLIST *l, 
				   GWEN_CONSTLIST_FOREACH_CB fn,
				   void *user_data){
  GWEN_LIST_ITERATOR *it;
  const void *el;
  assert(l);

  it = GWEN_List_First(l);
  if (!it)
    return 0;
  el = GWEN_ListIterator_Data(it);
  while(el) {
    el = fn(el, user_data);
    if (el) {
      GWEN_ListIterator_free(it);
      return el;
    }
    el = GWEN_ListIterator_Next(it);
  }
  GWEN_ListIterator_free(it);
  return 0;
}



GWEN_CONSTLIST_ITERATOR *GWEN_ConstList_FindIter(const GWEN_CONSTLIST *l, const void *p) {
 GWEN_CONSTLIST_ITERATOR *li;

  li=GWEN_ConstList_First(l);
  if (li) {
    const void *d;

    d=GWEN_ConstListIterator_Data(li);
    while(d) {
      if (d==p) {
        return li;
      }
      d=GWEN_ConstListIterator_Next(li);
    }
    GWEN_ConstListIterator_free(li);
  }
  return 0;
}

const void *GWEN_ConstList_Contains(const GWEN_CONSTLIST *l, const void *p) {
  GWEN_CONSTLIST_ITERATOR *li;

  li = GWEN_ConstList_FindIter(l, p);
  if (li) {
    GWEN_ConstListIterator_free(li);
    return p;
  }
  return 0;
}

void GWEN_ConstList_Remove(GWEN_CONSTLIST *l, const void *p) {
  GWEN_CONSTLIST_ITERATOR *li;

  li = GWEN_ConstList_FindIter(l, p);
  if (li) {
    GWEN_ConstList_Erase(l, li);
  }
}

GWEN_CONSTLIST_ITERATOR *GWEN_ConstList_First(const GWEN_CONSTLIST *l){
  return GWEN_List_First(l);
}



GWEN_CONSTLIST_ITERATOR *GWEN_ConstList_Last(const GWEN_CONSTLIST *l){
  return GWEN_List_Last(l);
}



GWEN_CONSTLIST_ITERATOR *GWEN_ConstListIterator_new(const GWEN_CONSTLIST *l){
  return GWEN_ListIterator_new(l);
}



void GWEN_ConstListIterator_free(GWEN_CONSTLIST_ITERATOR *li){
  GWEN_ListIterator_free(li);
}



const void *GWEN_ConstListIterator_Previous(GWEN_CONSTLIST_ITERATOR *li){
  return GWEN_ListIterator_Previous(li);
}



const void *GWEN_ConstListIterator_Next(GWEN_CONSTLIST_ITERATOR *li){
  return GWEN_ListIterator_Next(li);
}



const void *GWEN_ConstListIterator_Data(GWEN_CONSTLIST_ITERATOR *li){
  return GWEN_ListIterator_Data(li);
}






