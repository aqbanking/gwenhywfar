/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
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

#include "list_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>



GWEN_LIST_ENTRY *GWEN_ListEntry_new(){
  GWEN_LIST_ENTRY *le;

  GWEN_NEW_OBJECT(GWEN_LIST_ENTRY, le);
  le->usage=1;
  return le;
}



void GWEN_ListEntry_free(GWEN_LIST_ENTRY *le){
  if (le) {
    if (le->usage) {
      /* unlink in any case */
      le->previous=0;
      le->next=0;
      le->usage--;
      if (le->usage) {
        /* really free */
        free(le);
      }
    }
  }
}



GWEN_LIST *GWEN_List_new(){
  GWEN_LIST *l;

  GWEN_NEW_OBJECT(GWEN_LIST, l);
  return l;
}



void GWEN_List_free(GWEN_LIST *l){
  if (l) {
    GWEN_List_Clear(l);
    free(l);
  }
}



void GWEN_List_PushBack(GWEN_LIST *l, void *p){
  GWEN_LIST_ENTRY *le;

  le=GWEN_ListEntry_new();
  le->data=p;
  le->previous=l->last;
  l->last=le;
  if (!(l->first))
    l->first=le;
  l->size++;
}



void GWEN_List_PushFront(GWEN_LIST *l, void *p){
  GWEN_LIST_ENTRY *le;

  le=GWEN_ListEntry_new();
  le->data=p;
  le->next=l->first;
  if (l->first)
    l->first->previous=le;
  l->first=le;
  if (!(l->last))
    l->last=le;
  l->size++;
}



void *GWEN_List_GetFront(GWEN_LIST *l){
  assert(l);
  if (l->first)
    return l->first->data;
  return 0;
}



void *GWEN_List_GetBack(GWEN_LIST *l){
  assert(l);
  if (l->last)
    return l->last->data;
  return 0;
}



unsigned int GWEN_List_GetSize(GWEN_LIST *l){
  assert(l);
  return l->size;
}



void GWEN_List_PopBack(GWEN_LIST *l){
  GWEN_LIST_ENTRY *le;

  assert(l);
  le=l->last;
  if (le) {
    if (le->previous)
      le->previous->next=0;
    else {
      l->last=0;
      l->first=0;
    }
    GWEN_ListEntry_free(le);
    l->size--;
  }
}



void GWEN_List_PopFront(GWEN_LIST *l){
  GWEN_LIST_ENTRY *le;

  assert(l);
  le=l->first;
  if (le) {
    if (le->next)
      le->next->previous=0;
    else {
      l->first=0;
      l->last=0;
    }
    GWEN_ListEntry_free(le);
    l->size--;
  }
}



void GWEN_List_Clear(GWEN_LIST *l){
  GWEN_LIST_ENTRY *le;

  assert(l);
  le=l->first;
  while(le) {
    GWEN_LIST_ENTRY *nle;

    nle=le->next;
    GWEN_ListEntry_free(le);
    le=nle;
  } /* while */
}



void GWEN_List_Erase(GWEN_LIST *l, GWEN_LIST_ITERATOR *it){
  GWEN_LIST_ENTRY *current;

  assert(l);
  assert(it);
  if (it->current) {
    current=it->current;
    /* unlink from next */
    if (current->next) {
      it->current=current->next;
      current->next=current->previous;
    }
    else
      it->current=0;
    /* unlink from previous */
    if (current->previous)
      current->previous=current->next;
    /* free */
    GWEN_ListEntry_free(current);
  }
}



GWEN_LIST_ITERATOR *GWEN_List_First(GWEN_LIST *l){
  GWEN_LIST_ITERATOR *li;

  assert(l);
  li=GWEN_ListIterator_new(l);
  li->current=l->first;
  if (li->current)
    li->current->usage++;
  return li;
}



GWEN_LIST_ITERATOR *GWEN_List_Last(GWEN_LIST *l){
  GWEN_LIST_ITERATOR *li;

  assert(l);
  li=GWEN_ListIterator_new(l);
  li->current=l->last;
  if (li->current)
    li->current->usage++;
  return li;
}



GWEN_LIST_ITERATOR *GWEN_ListIterator_new(GWEN_LIST *l){
  GWEN_LIST_ITERATOR *li;

  GWEN_NEW_OBJECT(GWEN_LIST_ITERATOR, li);
  li->list=l;
  return li;
}



void GWEN_ListIterator_free(GWEN_LIST_ITERATOR *li){
  if (li) {
    if (li->current)
      GWEN_ListEntry_free(li->current);
    free(li);
  }
}



void *GWEN_ListIterator_Previous(GWEN_LIST_ITERATOR *li){
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
    return le->data;
  }
  return 0;
}



void *GWEN_ListIterator_Next(GWEN_LIST_ITERATOR *li){
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
    return le->data;
  }
  return 0;
}



void *GWEN_ListIterator_Data(GWEN_LIST_ITERATOR *li){
  assert(li);

  if (li->current)
    return li->current->data;
  return 0;
}





/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                 ConstList
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


GWEN_CONSTLIST_ENTRY *GWEN_ConstListEntry_new(){
  GWEN_CONSTLIST_ENTRY *le;

  GWEN_NEW_OBJECT(GWEN_CONSTLIST_ENTRY, le);
  le->usage=1;
  return le;
}



void GWEN_ConstListEntry_free(GWEN_CONSTLIST_ENTRY *le){
  if (le) {
    if (le->usage) {
      /* unlink in any case */
      le->previous=0;
      le->next=0;
      le->usage--;
      if (le->usage) {
        /* really free */
        free(le);
      }
    }
  }
}



GWEN_CONSTLIST *GWEN_ConstList_new(){
  GWEN_CONSTLIST *l;

  GWEN_NEW_OBJECT(GWEN_CONSTLIST, l);
  return l;
}



void GWEN_ConstList_free(GWEN_CONSTLIST *l){
  if (l) {
    GWEN_ConstList_Clear(l);
    free(l);
  }
}



void GWEN_ConstList_PushBack(GWEN_CONSTLIST *l, const void *p){
  GWEN_CONSTLIST_ENTRY *le;

  le=GWEN_ConstListEntry_new();
  le->data=p;
  le->previous=l->last;
  l->last=le;
  if (!(l->first))
    l->first=le;
  l->size++;
}



void GWEN_ConstList_PushFront(GWEN_CONSTLIST *l, const void *p){
  GWEN_CONSTLIST_ENTRY *le;

  le=GWEN_ConstListEntry_new();
  le->data=p;
  le->next=l->first;
  if (l->first)
    l->first->previous=le;
  l->first=le;
  if (!(l->last))
    l->last=le;
  l->size++;
}



const void *GWEN_ConstList_GetFront(GWEN_CONSTLIST *l){
  assert(l);
  if (l->first)
    return l->first->data;
  return 0;
}



const void *GWEN_ConstList_GetBack(GWEN_CONSTLIST *l){
  assert(l);
  if (l->last)
    return l->last->data;
  return 0;
}



unsigned int GWEN_ConstList_GetSize(GWEN_CONSTLIST *l){
  assert(l);
  return l->size;
}



void GWEN_ConstList_PopBack(GWEN_CONSTLIST *l){
  GWEN_CONSTLIST_ENTRY *le;

  assert(l);
  le=l->last;
  if (le) {
    if (le->previous)
      le->previous->next=0;
    else {
      l->last=0;
      l->first=0;
    }
    GWEN_ConstListEntry_free(le);
    l->size--;
  }
}



void GWEN_ConstList_PopFront(GWEN_CONSTLIST *l){
  GWEN_CONSTLIST_ENTRY *le;

  assert(l);
  le=l->first;
  if (le) {
    if (le->next)
      le->next->previous=0;
    else {
      l->first=0;
      l->last=0;
    }
    GWEN_ConstListEntry_free(le);
    l->size--;
  }
}



void GWEN_ConstList_Erase(GWEN_CONSTLIST *l, GWEN_CONSTLIST_ITERATOR *it){
  GWEN_CONSTLIST_ENTRY *current;

  assert(l);
  assert(it);
  if (it->current) {
    current=it->current;
    /* unlink from next */
    if (current->next) {
      it->current=current->next;
      current->next=current->previous;
    }
    else
      it->current=0;
    /* unlink from previous */
    if (current->previous)
      current->previous=current->next;
    /* free */
    GWEN_ConstListEntry_free(current);
  }
}



void GWEN_ConstList_Clear(GWEN_CONSTLIST *l){
  GWEN_CONSTLIST_ENTRY *le;

  assert(l);
  le=l->first;
  while(le) {
    GWEN_CONSTLIST_ENTRY *nle;

    nle=le->next;
    GWEN_ConstListEntry_free(le);
    le=nle;
  } /* while */
}



GWEN_CONSTLIST_ITERATOR *GWEN_ConstList_First(GWEN_CONSTLIST *l){
  GWEN_CONSTLIST_ITERATOR *li;

  assert(l);
  li=GWEN_ConstListIterator_new(l);
  li->current=l->first;
  if (li->current)
    li->current->usage++;
  return li;
}



GWEN_CONSTLIST_ITERATOR *GWEN_ConstList_Last(GWEN_CONSTLIST *l){
  GWEN_CONSTLIST_ITERATOR *li;

  assert(l);
  li=GWEN_ConstListIterator_new(l);
  li->current=l->last;
  if (li->current)
    li->current->usage++;
  return li;
}



GWEN_CONSTLIST_ITERATOR *GWEN_ConstListIterator_new(GWEN_CONSTLIST *l){
  GWEN_CONSTLIST_ITERATOR *li;

  GWEN_NEW_OBJECT(GWEN_CONSTLIST_ITERATOR, li);
  li->list=l;
  return li;
}



void GWEN_ConstListIterator_free(GWEN_CONSTLIST_ITERATOR *li){
  if (li) {
    if (li->current)
      GWEN_ConstListEntry_free(li->current);
    free(li);
  }
}



const void *GWEN_ConstListIterator_Previous(GWEN_CONSTLIST_ITERATOR *li){
  GWEN_CONSTLIST_ENTRY *le;

  assert(li);

  le=li->current;
  if (le)
    le=le->previous;
  if (li->current)
    GWEN_ConstListEntry_free(li->current);
  li->current=le;
  if (le) {
    le->usage++;
    return le->data;
  }
  return 0;
}



const void *GWEN_ConstListIterator_Next(GWEN_CONSTLIST_ITERATOR *li){
  GWEN_CONSTLIST_ENTRY *le;

  assert(li);

  le=li->current;
  if (le)
    le=le->next;
  if (li->current)
    GWEN_ConstListEntry_free(li->current);
  li->current=le;
  if (le) {
    le->usage++;
    return le->data;
  }
  return 0;
}



const void *GWEN_ConstListIterator_Data(GWEN_CONSTLIST_ITERATOR *li){
  assert(li);

  if (li->current)
    return li->current->data;
  return 0;
}






