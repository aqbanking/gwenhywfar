/***************************************************************************
 begin       : Thu Jul 04 2019
 copyright   : (C) 2019 by Martin Preuss
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

#include "tree2_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>




GWEN_TREE2_ELEMENT *GWEN_Tree2Element_new(void *d)
{
  GWEN_TREE2_ELEMENT *el;

  GWEN_NEW_OBJECT(GWEN_TREE2_ELEMENT, el);
  el->data=d;

  return el;
}



void GWEN_Tree2Element_free(GWEN_TREE2_ELEMENT *el)
{
  if (el) {
    if (el->firstChild) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Tree element still has children");
      assert(0);
    }
    GWEN_FREE_OBJECT(el);
  }
}



void GWEN_Tree2_Unlink(GWEN_TREE2_ELEMENT *el)
{

  /* unlink from previous */
  if (el->prevElement)
    el->prevElement->nextElement=el->nextElement;

  /* unlink from next */
  if (el->nextElement)
    el->nextElement->prevElement=el->prevElement;

  /* unlink from parent */
  if (el->parent) {
    if (el->parent->firstChild==el)
      el->parent->firstChild=el->nextElement;
    if (el->parent->lastChild==el)
      el->parent->lastChild=el->prevElement;
  }

  el->nextElement=NULL;
  el->prevElement=NULL;
  el->parent=NULL;
}



void GWEN_Tree2_Replace(GWEN_TREE2_ELEMENT *elToReplace, GWEN_TREE2_ELEMENT *elReplacement)
{
  elReplacement->nextElement=NULL;
  elReplacement->prevElement=NULL;
  elReplacement->parent=NULL;

  /* relink with previous */
  if (elToReplace->prevElement)
    elToReplace->prevElement->nextElement=elReplacement;
  elReplacement->prevElement=elToReplace->prevElement;

  /* relink with next */
  if (elToReplace->nextElement)
    elToReplace->nextElement->prevElement=elReplacement;
  elReplacement->nextElement=elToReplace->nextElement;

  /* relink with parent */
  if (elToReplace->parent) {
    elReplacement->parent=elToReplace->parent;
    if (elToReplace->parent->firstChild==elToReplace)
      elToReplace->parent->firstChild=elReplacement;
    if (elToReplace->parent->lastChild==elToReplace)
      elToReplace->parent->lastChild=elReplacement;
  }

  elToReplace->nextElement=NULL;
  elToReplace->prevElement=NULL;
  elToReplace->parent=NULL;
}



void GWEN_Tree2_AddChild(GWEN_TREE2_ELEMENT *where, GWEN_TREE2_ELEMENT *el)
{
  if (where->firstChild==NULL)
    where->firstChild=el;

  el->prevElement=where->lastChild;
  if (where->lastChild)
    where->lastChild->nextElement=el;
  where->lastChild=el;

  el->parent=where;
}



void GWEN_Tree2_InsertChild(GWEN_TREE2_ELEMENT *where, GWEN_TREE2_ELEMENT *el)
{
  el->nextElement=where->firstChild;
  where->firstChild=el;

  if (where->lastChild==NULL)
    where->lastChild=el;

  el->parent=where;
}



void *GWEN_Tree2Element_GetPrevious(const GWEN_TREE2_ELEMENT *el)
{
  if (el->prevElement)
    return el->prevElement->data;
  return 0;
}



void *GWEN_Tree2Element_GetNext(const GWEN_TREE2_ELEMENT *el)
{
  if (el->nextElement)
    return el->nextElement->data;
  return 0;
}



void *GWEN_Tree2Element_GetBelow(const GWEN_TREE2_ELEMENT *el)
{
  if (el->firstChild)                               /* look down */
    return el->firstChild->data;
  else if (el->nextElement)                         /* look right */
    return el->nextElement->data;
  else {
    /* look for a parent which has a right neighbour */
    while (el && el->parent) {
      if (el->parent->nextElement)                  /* look right of parent */
        return el->parent->nextElement->data;
      /* parent has no right neighbour, consult its parent */
      el=el->parent;
    }
  }

  return NULL;
}



void *GWEN_Tree2Element_GetFirstChild(const GWEN_TREE2_ELEMENT *el)
{
  if (el->firstChild)
    return el->firstChild->data;
  return NULL;
}



void *GWEN_Tree2Element_GetLastChild(const GWEN_TREE2_ELEMENT *el)
{
  if (el->lastChild)
    return el->lastChild->data;
  return NULL;
}



void *GWEN_Tree2Element_GetParent(const GWEN_TREE2_ELEMENT *el)
{
  if (el->parent)
    return el->parent->data;
  return NULL;
}



