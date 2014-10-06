/***************************************************************************
    begin       : Fri Jan 02 2009
    copyright   : (C) 2009 by Martin Preuss
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

#include "tree_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>




GWEN_TREE *GWEN_Tree_new(void) {
  GWEN_TREE *l;

  GWEN_NEW_OBJECT(GWEN_TREE, l);
  return l;
}


void GWEN_Tree_free(GWEN_TREE *l) {
  if (l) {
    GWEN_FREE_OBJECT(l);
  }
}



int GWEN_Tree_GetCount(const GWEN_TREE *l) {
  assert(l);
  return l->count;
}



void GWEN_Tree_Add(GWEN_TREE *l, GWEN_TREE_ELEMENT *el) {
  assert(l);
  assert(el);
  if (el->treePtr) {
    /* element is already part of another list */
    DBG_ERROR(GWEN_LOGDOMAIN, "Element is already part of a list");
    assert(0);
  }
  else {
    if (l->firstElement==0)
      l->firstElement=el;

    el->prevElement=l->lastElement;
    if (l->lastElement)
      l->lastElement->nextElement=el;
    l->lastElement=el;

    el->treePtr=l;
    el->parent=NULL;
    l->count++;
  }
}



void GWEN_Tree_AddList(GWEN_TREE *dest, GWEN_TREE *l) {
  GWEN_TREE_ELEMENT *el;

  assert(dest);
  assert(l);

  while((el=l->firstElement)) {
    GWEN_Tree_Del(el);
    GWEN_Tree_Add(dest, el);
  }
}



void GWEN_Tree_Insert(GWEN_TREE *l, GWEN_TREE_ELEMENT *el) {
  assert(l);
  assert(el);
  if (el->treePtr) {
    /* element is already part of another list */
    DBG_ERROR(GWEN_LOGDOMAIN, "Element is already part of a list");
  }
  else {
    el->nextElement=l->firstElement;
    l->firstElement=el;

    if (l->lastElement==0)
      l->lastElement=el;

    el->treePtr=l;
    el->parent=NULL;
    l->count++;
  }
}



void GWEN_Tree_Del(GWEN_TREE_ELEMENT *el) {
  GWEN_TREE *l;

  l=el->treePtr;

  if (l==0) {
    /* not part of any list */
    DBG_ERROR(GWEN_LOGDOMAIN, "Element is not part of any list");
  }
  else {
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

    /* unlink from parent */
    if (el->parent) {
      if (el->parent->firstChild==el)
        el->parent->firstChild=el->nextElement;
      if (el->parent->lastChild==el)
        el->parent->lastChild=el->prevElement;
      el->parent->count--;
    }

    el->nextElement=NULL;
    el->prevElement=NULL;
    el->parent=NULL;
    el->treePtr=NULL;
  }
}



void GWEN_Tree_AddChild(GWEN_TREE_ELEMENT *where, GWEN_TREE_ELEMENT *el) {
  if (el->treePtr) {
    /* element is already part of another tree */
    DBG_ERROR(GWEN_LOGDOMAIN, "Element is already part of a tree");
    assert(0);
  }
  else {
    if (where->firstChild==0)
      where->firstChild=el;

    el->prevElement=where->lastChild;
    if (where->lastChild)
      where->lastChild->nextElement=el;
    where->lastChild=el;

    el->parent=where;

    el->treePtr=where->treePtr;
    el->treePtr->count++;
    where->count++;
  }
}



void GWEN_Tree_InsertChild(GWEN_TREE_ELEMENT *where, GWEN_TREE_ELEMENT *el) {
  if (el->treePtr) {
    /* element is already part of another list */
    DBG_ERROR(GWEN_LOGDOMAIN, "Element is already part of a tree");
    assert(0);
  }
  else {
    el->nextElement=where->firstChild;
    where->firstChild=el;

    if (where->lastChild==NULL)
      where->lastChild=el;

    el->parent=where;

    el->treePtr=where->treePtr;
    el->treePtr->count++;
    where->count++;
  }
}



void *GWEN_Tree_GetFirst(const GWEN_TREE *l) {
  if (l->firstElement)
    return l->firstElement->data;
  return 0;
}



void *GWEN_Tree_GetLast(const GWEN_TREE *l) {
  if (l->lastElement)
    return l->lastElement->data;
  return 0;
}





GWEN_TREE_ELEMENT *GWEN_TreeElement_new(void *d) {
  GWEN_TREE_ELEMENT *el;

  GWEN_NEW_OBJECT(GWEN_TREE_ELEMENT, el);
  el->data=d;

  return el;
}



void GWEN_TreeElement_free(GWEN_TREE_ELEMENT *el) {
  if (el) {
    if (el->treePtr)
      GWEN_Tree_Del(el);
    if (el->firstChild) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Tree element still has children");
      assert(0);
    }
    GWEN_FREE_OBJECT(el);
  }
}



void *GWEN_TreeElement_GetPrevious(const GWEN_TREE_ELEMENT *el) {
  if (el->prevElement)
    return el->prevElement->data;
  return 0;
}



void *GWEN_TreeElement_GetNext(const GWEN_TREE_ELEMENT *el) {
  if (el->nextElement)
    return el->nextElement->data;
  return 0;
}



void *GWEN_TreeElement_GetBelow(const GWEN_TREE_ELEMENT *el) {
  if (el->firstChild)                               /* look down */
    return el->firstChild->data;
  else if (el->nextElement)                         /* look right */
    return el->nextElement->data;
  else {
    /* look for a parent which has a right neighbour */
    while(el && el->parent) {
      if (el->parent->nextElement)                  /* look right of parent */
        return el->parent->nextElement->data;
      /* parent has no right neighbour, consult its parent */
      el=el->parent;
    }
  }

  return NULL;
}



void *GWEN_TreeElement_GetFirstChild(const GWEN_TREE_ELEMENT *el) {
  if (el->firstChild)
    return el->firstChild->data;
  return NULL;
}



void *GWEN_TreeElement_GetLastChild(const GWEN_TREE_ELEMENT *el) {
  if (el->lastChild)
    return el->lastChild->data;
  return NULL;
}



void *GWEN_TreeElement_GetParent(const GWEN_TREE_ELEMENT *el) {
  if (el->parent)
    return el->parent->data;
  return NULL;
}



uint32_t GWEN_TreeElement_GetChildrenCount(const GWEN_TREE_ELEMENT *el) {
  assert(el);
  return el->count;
}








