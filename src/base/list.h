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
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/refptr.h>
/* This is needed for PalmOS, because it define some functions needed */
#include <string.h>
#include <stdio.h>


/** @defgroup GWEN_LIST Generic List Handling
 *  @ingroup MOD_BASE
 * The macros of this group facilitates typesafe use of lists.
 */
/*@{*/

/** @brief Doubly-linked list. 
 *
 * The list contains pointer to data objects, with the ability to
 * iterate over the list in both directions. */
typedef struct GWEN_LIST GWEN_LIST;

/** Callback function for one list element. */
typedef void *(*GWEN_LIST_FOREACH_CB)(void *element, void *user_data);

/** @brief Doubly-linked list with const objects. 
 *
 * The list contains pointer to const data objects, with the ability
 * to iterate over the list in both directions. */
typedef struct GWEN_LIST GWEN_CONSTLIST;

/** Callback function for one const list element. */
typedef const void *(*GWEN_CONSTLIST_FOREACH_CB)(const void *element,
						 void *user_data);

/** An iterator for the doubly-linked list, i.e. a pointer to a
    specific element */
typedef struct GWEN_LIST_ITERATOR GWEN_LIST_ITERATOR;

/** An iterator for the const doubly-linked list, i.e. a pointer to a
    specific element */
typedef struct GWEN_LIST_ITERATOR GWEN_CONSTLIST_ITERATOR;


/** allow inheriting of lists */
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_LIST, GWENHYWFAR_API)


/** Constructor. Returns a new empty list. */
GWENHYWFAR_API
GWEN_LIST *GWEN_List_new();

/** Destructor. Frees all of the memory used by this list. The list
 * elements are not freed (FIXME: Describe how to free the list
 * elements). */
GWENHYWFAR_API
void GWEN_List_free(GWEN_LIST *l);

/** Duplicates a list by returning a reference and using
 * reference-counting. */
GWENHYWFAR_API
GWEN_LIST *GWEN_List_dup(const GWEN_LIST *l);


GWENHYWFAR_API
void GWEN_List_Unshare(GWEN_LIST *l);


/**
 * Dumps the contents of the list to an open file (e.g. stderr).
 */
GWENHYWFAR_API
void GWEN_List_Dump(const GWEN_LIST *l, FILE *f, unsigned int indent);

/**
 * Appends an element to a list, making it the new last element.
 */
GWENHYWFAR_API
void GWEN_List_PushBack(GWEN_LIST *l, void *p);

/**
 * Appends an element to a list, making it the new last element.
 */
GWENHYWFAR_API
void GWEN_List_PushBackRefPtr(GWEN_LIST *l, GWEN_REFPTR *rp);

/**
 * Inserts an element at the start of the list, making it the new
 * first element.
 */
GWENHYWFAR_API
void GWEN_List_PushFront(GWEN_LIST *l, void *p);

/**
 * Inserts an element at the start of the list, making it the new
 * first element.
 */
GWENHYWFAR_API
void GWEN_List_PushFrontRefPtr(GWEN_LIST *l, GWEN_REFPTR *rp);

/**
 * Returns the first element of the list. (The element is not
 * removed from the list.)
 */
GWENHYWFAR_API
void *GWEN_List_GetFront(const GWEN_LIST *l);

/**
 * Returns the first element of the list. (The element is not
 * removed from the list.)
 */
GWENHYWFAR_API
GWEN_REFPTR *GWEN_List_GetFrontRefPtr(const GWEN_LIST *l);

/**
 * Returns the last element of the list. (The element is not
 * removed from the list.)
 */
GWENHYWFAR_API
void *GWEN_List_GetBack(const GWEN_LIST *l);

/**
 * Returns the last element of the list. (The element is not
 * removed from the list.)
 */
GWENHYWFAR_API
GWEN_REFPTR *GWEN_List_GetBackRefPtr(const GWEN_LIST *l);

/**
 * Removes the element currently pointed to by the given iterator
 * from the list. (The element is not freed.)
 */
GWENHYWFAR_API
void GWEN_List_Erase(GWEN_LIST *l, GWEN_LIST_ITERATOR *it);

/** Returns the size of this list, i.e. the number of elements in this
 * list. 
 *
 * This number is counted in the list metadata, so this is a cheap
 * operation. */
GWENHYWFAR_API
unsigned int GWEN_List_GetSize(const GWEN_LIST *l);

GWENHYWFAR_API
GWEN_REFPTR_INFO *GWEN_List_GetRefPtrInfo(const GWEN_LIST *l);

GWENHYWFAR_API
void GWEN_List_SetRefPtrInfo(GWEN_LIST *l, GWEN_REFPTR_INFO *rpi);

/**
 * Removes the list's last element from the list. (The element is not
 * freed.)
 */
GWENHYWFAR_API
void GWEN_List_PopBack(GWEN_LIST *l);

/**
 * Removes the list's first element from the list. (The element is not
 * freed.)
 */
GWENHYWFAR_API
void GWEN_List_PopFront(GWEN_LIST *l);

/**
 * Removes all list elements from the list. (The elements are not
 * freed. FIXME: is this true?)
 */
GWENHYWFAR_API
void GWEN_List_Clear(GWEN_LIST *l);


/** Traverses the list, calling the callback function 'func' on
 * each list element.  Traversal will stop when 'func' returns a
 * non-NULL value, and the routine will return with that
 * value. Otherwise the routine will return NULL.
 *
 * @param list The list to traverse.
 * @param func The function to be called with each list element.
 * @param user_data A pointer passed on to the function 'func'.
 * @return The non-NULL pointer returned by 'func' as soon as it
 * returns one. Otherwise (i.e. 'func' always returns NULL)
 * returns NULL.
 */
GWENHYWFAR_API
void *GWEN_List_ForEach(GWEN_LIST *list, GWEN_LIST_FOREACH_CB func,
			void *user_data);

/** Return an iterator pointing to the first element in the list */
GWENHYWFAR_API
GWEN_LIST_ITERATOR *GWEN_List_First(const GWEN_LIST *l);

/** Returns an iterator pointing to the last element in the list. */
GWENHYWFAR_API
GWEN_LIST_ITERATOR *GWEN_List_Last(const GWEN_LIST *l);

/**
 * Creates a list iterator for the given list. (FIXME: Should be private).
 */
GWENHYWFAR_API
GWEN_LIST_ITERATOR *GWEN_ListIterator_new(const GWEN_LIST *l);

/** Frees a list iterator. */
GWENHYWFAR_API
void GWEN_ListIterator_free(GWEN_LIST_ITERATOR *li);

/**
 * Moves the list iterator to the predecessor of the currenty selected
 * element and returns that predecessor element.
 */
GWENHYWFAR_API
void *GWEN_ListIterator_Previous(GWEN_LIST_ITERATOR *li);

/**
 * Moves the list iterator to the predecessor of the currenty selected
 * element and returns that predecessor element.
 */
GWENHYWFAR_API
GWEN_REFPTR *GWEN_ListIterator_PreviousRefPtr(GWEN_LIST_ITERATOR *li);

/**
 * Moves the list iterator to the successor of the currenty selected
 * element and returns that successor element.
 */
GWENHYWFAR_API
void *GWEN_ListIterator_Next(GWEN_LIST_ITERATOR *li);

/**
 * Moves the list iterator to the successor of the currenty selected
 * element and returns that successor element.
 */
GWENHYWFAR_API
GWEN_REFPTR *GWEN_ListIterator_NextRefPtr(GWEN_LIST_ITERATOR *li);

/**
 * Returns the pointer to the element stored at the list position the
 * iterator currently points to.
 */
GWENHYWFAR_API
void *GWEN_ListIterator_Data(GWEN_LIST_ITERATOR *li);

/**
 * Returns the pointer to the element stored at the list position the
 * iterator currently points to.
 */
GWENHYWFAR_API
GWEN_REFPTR *GWEN_ListIterator_DataRefPtr(GWEN_LIST_ITERATOR *li);

GWENHYWFAR_API
void GWEN_ListIterator_IncLinkCount(GWEN_LIST_ITERATOR *li);

GWENHYWFAR_API
unsigned int GWEN_ListIterator_GetLinkCount(const GWEN_LIST_ITERATOR *li);




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
GWEN_CONSTLIST_ITERATOR *GWEN_ConstList_First(const GWEN_CONSTLIST *l);

GWENHYWFAR_API
GWEN_CONSTLIST_ITERATOR *GWEN_ConstList_Last(const GWEN_CONSTLIST *l);

GWENHYWFAR_API
GWEN_CONSTLIST_ITERATOR *GWEN_ConstListIterator_new(const GWEN_CONSTLIST *l);

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



