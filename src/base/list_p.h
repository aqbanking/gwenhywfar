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


#ifndef GWENHYWFAR_LIST_P_H
#define GWENHYWFAR_LIST_P_H

#include <gwenhywfar/list.h>
#include <gwenhywfar/inherit.h>


typedef struct GWEN_LIST_ENTRY GWEN_LIST_ENTRY;
typedef struct GWEN_CONSTLIST_ENTRY GWEN_CONSTLIST_ENTRY;
typedef struct GWEN__LISTPTR GWEN__LISTPTR;

struct GWEN_LIST_ENTRY {
  GWEN_LIST_ENTRY *previous;
  GWEN_LIST_ENTRY *next;
  GWEN_REFPTR *dataPtr;
  unsigned int usage;
  unsigned int linkCount;
};


struct GWEN_CONSTLIST_ENTRY {
  GWEN_CONSTLIST_ENTRY *previous;
  GWEN_CONSTLIST_ENTRY *next;
  const void *data;
  unsigned int usage;
};


struct GWEN__LISTPTR {
  GWEN_TYPE_UINT32 refCount;
  GWEN_LIST_ENTRY *first;
  GWEN_LIST_ENTRY *last;
  unsigned int size;
  GWEN_REFPTR_INFO *refPtrInfo;
};


GWEN__LISTPTR *GWEN__ListPtr_new();
void GWEN__ListPtr_free(GWEN__LISTPTR *lp);
void GWEN__ListPtr_Attach(GWEN__LISTPTR *lp);
void GWEN__ListPtr_Clear(GWEN__LISTPTR *lp);
GWEN__LISTPTR *GWEN__ListPtr_dup(GWEN__LISTPTR *lp);


struct GWEN_LIST {
  GWEN_INHERIT_ELEMENT(GWEN_LIST)
  GWEN__LISTPTR *listPtr;
  GWEN_REFPTR_INFO *refPtrInfo;
};


struct GWEN_CONSTLIST {
  GWEN_CONSTLIST_ENTRY *first;
  GWEN_CONSTLIST_ENTRY *last;
  unsigned int size;
};



struct GWEN_LIST_ITERATOR {
  const GWEN_LIST *list;

  GWEN_LIST_ENTRY *current;
};


struct GWEN_CONSTLIST_ITERATOR {
  const GWEN_CONSTLIST *list;

  GWEN_CONSTLIST_ENTRY *current;
};



#endif



