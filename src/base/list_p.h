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

#ifdef __cplusplus
extern "C" {
#endif

#include <gwenhyfwar/list.h>


typedef struct GWEN_LIST_ENTRY GWEN_LIST_ENTRY;
typedef struct GWEN_CONSTLIST_ENTRY GWEN_CONSTLIST_ENTRY;


struct GWEN_LIST_ENTRY {
  GWEN_LIST_ENTRY *previous;
  GWEN_LIST_ENTRY *next;
  void *data;
  unsigned int usage;
};


struct GWEN_CONSTLIST_ENTRY {
  GWEN_CONSTLIST_ENTRY *previous;
  GWEN_CONSTLIST_ENTRY *next;
  const void *data;
  unsigned int usage;
};


struct GWEN_LIST {
  GWEN_LIST_ENTRY *first;
  GWEN_LIST_ENTRY *last;
  unsigned int size;
};


struct GWEN_CONSTLIST {
  GWEN_CONSTLIST_ENTRY *first;
  GWEN_CONSTLIST_ENTRY *last;
  unsigned int size;
};



struct GWEN_LIST_ITERATOR {
  GWEN_LIST *list;

  GWEN_LIST_ENTRY *current;
};


struct GWEN_CONSTLIST_ITERATOR {
  GWEN_CONSTLIST *list;

  GWEN_CONSTLIST_ENTRY *current;
};



#ifdef __cplusplus
}
#endif


#endif



