/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
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

#ifndef GWENHYFWAR_MISC_H
#define GWENHYFWAR_MISC_H

#include <gwenhyfwar/gwenhyfwarapi.h>
#include <stdlib.h>
#include <assert.h>


#define GWEN_NEW_OBJECT(typ, varname) \
  varname=(typ *)malloc(sizeof(typ)); \
  assert(varname); \
  memset(varname,0,sizeof(typ));



#define GWEN_LIST_ADD(typ, sr, head) {\
  typ *curr;                \
                            \
  assert(sr);               \
  assert(head);             \
                            \
  curr=*head;               \
  if (!curr) {              \
    *head=sr;               \
  }                         \
  else {                    \
    while(curr->next) {     \
      curr=curr->next;      \
    }                       \
    curr->next=sr;          \
  }}


#define GWEN_LIST_DEL(typ, sr, head) {\
  typ *curr;                   \
                               \
  assert(sr);                  \
  assert(head);                \
  curr=*head;                  \
  if (curr) {                  \
    if (curr==sr) {            \
      *head=curr->next;        \
    }                          \
    else {                     \
      while(curr->next!=sr) {  \
	curr=curr->next;       \
      }                        \
      if (curr)                \
	curr->next=sr->next;   \
    }                          \
  }                            \
  sr->next=0;}


#endif



