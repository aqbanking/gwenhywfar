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


#ifndef GWENHYWFAR_LIST_H
#define GWENHYWFAR_LIST_H


#ifdef __cplusplus
extern "C" {
#endif

#include <gwenhywfar/gwenhywfarapi.h>
/* This is needed for PalmOS, because it define some functions needed */
#include <string.h>
#include <stdio.h>


/** @defgroup GWEN_LIST Generic List Handling
 *  @ingroup MOD_BASE
 * The macros of this group facilitates typesafe use of lists.
 */
/*@{*/

typedef struct GWEN_LIST GWEN_LIST;

typedef void *(*GWEN_LIST_FOREACH_CB)(void *element, void *user_data);

typedef struct GWEN_CONSTLIST GWEN_CONSTLIST;

typedef const void *(*GWEN_CONSTLIST_FOREACH_CB)(const void *element,
						 void *user_data);

typedef struct GWEN_LIST_ITERATOR GWEN_LIST_ITERATOR;

typedef struct GWEN_CONSTLIST_ITERATOR GWEN_CONSTLIST_ITERATOR;


GWENHYWFAR_API
GWEN_LIST *GWEN_List_new();

GWENHYWFAR_API
void GWEN_List_free(GWEN_LIST *l);

GWENHYWFAR_API
void GWEN_List_Dump(GWEN_LIST *l, FILE *f, unsigned int indent);

GWENHYWFAR_API
void GWEN_List_PushBack(GWEN_LIST *l, void *p);

GWENHYWFAR_API
void GWEN_List_PushFront(GWEN_LIST *l, void *p);

GWENHYWFAR_API
void *GWEN_List_GetFront(GWEN_LIST *l);

GWENHYWFAR_API
void *GWEN_List_GetBack(GWEN_LIST *l);

GWENHYWFAR_API
void GWEN_List_Erase(GWEN_LIST *l, GWEN_LIST_ITERATOR *it);

GWENHYWFAR_API
unsigned int GWEN_List_GetSize(GWEN_LIST *l);

GWENHYWFAR_API
void GWEN_List_PopBack(GWEN_LIST *l);

GWENHYWFAR_API
void GWEN_List_PopFront(GWEN_LIST *l);

GWENHYWFAR_API
void GWEN_List_Clear(GWEN_LIST *l);

GWENHYWFAR_API
void *GWEN_List_ForEach(GWEN_LIST *l, GWEN_LIST_FOREACH_CB fn, void *user_data);

GWENHYWFAR_API
GWEN_LIST_ITERATOR *GWEN_List_First(GWEN_LIST *l);

GWENHYWFAR_API
GWEN_LIST_ITERATOR *GWEN_List_Last(GWEN_LIST *l);

GWENHYWFAR_API
GWEN_LIST_ITERATOR *GWEN_ListIterator_new(GWEN_LIST *l);

GWENHYWFAR_API
void GWEN_ListIterator_free(GWEN_LIST_ITERATOR *li);

GWENHYWFAR_API
void *GWEN_ListIterator_Previous(GWEN_LIST_ITERATOR *li);

GWENHYWFAR_API
void *GWEN_ListIterator_Next(GWEN_LIST_ITERATOR *li);

GWENHYWFAR_API
void *GWEN_ListIterator_Data(GWEN_LIST_ITERATOR *li);





GWENHYWFAR_API
GWEN_CONSTLIST *GWEN_ConstList_new();

GWENHYWFAR_API
void GWEN_ConstList_free(GWEN_CONSTLIST *l);

GWENHYWFAR_API
void GWEN_ConstList_PushBack(GWEN_CONSTLIST *l, const void *p);

GWENHYWFAR_API
void GWEN_ConstList_PushFront(GWEN_CONSTLIST *l, const void *p);

GWENHYWFAR_API
const void *GWEN_ConstList_GetFront(GWEN_CONSTLIST *l);

GWENHYWFAR_API
const void *GWEN_ConstList_GetBack(GWEN_CONSTLIST *l);

GWENHYWFAR_API
unsigned int GWEN_ConstList_GetSize(GWEN_CONSTLIST *l);

GWENHYWFAR_API
void GWEN_ConstList_PopBack(GWEN_CONSTLIST *l);

GWENHYWFAR_API
void GWEN_ConstList_PopFront(GWEN_CONSTLIST *l);

GWENHYWFAR_API
void GWEN_ConstList_Clear(GWEN_CONSTLIST *l);

GWENHYWFAR_API
const void *GWEN_ConstList_ForEach(GWEN_CONSTLIST *l, 
				   GWEN_CONSTLIST_FOREACH_CB fn,
				   void *user_data);

GWENHYWFAR_API
GWEN_CONSTLIST_ITERATOR *GWEN_ConstList_First(GWEN_CONSTLIST *l);

GWENHYWFAR_API
GWEN_CONSTLIST_ITERATOR *GWEN_ConstList_Last(GWEN_CONSTLIST *l);

GWENHYWFAR_API
GWEN_CONSTLIST_ITERATOR *GWEN_ConstListIterator_new(GWEN_CONSTLIST *l);

GWENHYWFAR_API
void GWEN_ConstListIterator_free(GWEN_CONSTLIST_ITERATOR *li);

GWENHYWFAR_API
const void *GWEN_ConstListIterator_Previous(GWEN_CONSTLIST_ITERATOR *li);

GWENHYWFAR_API
const void *GWEN_ConstListIterator_Next(GWEN_CONSTLIST_ITERATOR *li);

GWENHYWFAR_API
const void *GWEN_ConstListIterator_Data(GWEN_CONSTLIST_ITERATOR *li);



/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif


#endif



