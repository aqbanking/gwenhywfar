/***************************************************************************
    begin       : Wed Sep 12 2012
    copyright   : (C) 2012 by Martin Preuss
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

/** @file pointerlist.h
 * @short A list of pointers
 */


#ifndef GWENHYWFAR_POINTERLIST_H
#define GWENHYWFAR_POINTERLIST_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct GWEN_POINTERLIST GWEN_POINTERLIST;
typedef struct GWEN_POINTERLIST_ITERATOR GWEN_POINTERLIST_ITERATOR;
#ifdef __cplusplus
}
#endif

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup MOD_POINTERLIST Pointer list module
 * @ingroup MOD_BASE
 * @short A list of pointers
 *
 * This is basically a list for pointers,
 * but since it uses tables instead of those list objects normally used
 * throughout Gwen it is less memory consuming.
 */
/*@{*/
/**
 * Constructor.
 */
GWENHYWFAR_API
GWEN_POINTERLIST *GWEN_PointerList_new(void);

/**
 * Destructor.
 */
GWENHYWFAR_API
void GWEN_PointerList_free(GWEN_POINTERLIST *idl);



GWENHYWFAR_API
void GWEN_PointerList_Attach(GWEN_POINTERLIST *idl);


/**
 * Returns a new id list which contains all the ids of the given list
 * in the same order. However, the list returned might be more
 * is compact (i.e. it has fewer wholes), so the positions of ids
 * within the list might have changed!.
 */
GWENHYWFAR_API
GWEN_POINTERLIST *GWEN_PointerList_dup(const GWEN_POINTERLIST *idl);


/**
 * Removes all ptrs from the list thus leaving it empty.
 */
GWENHYWFAR_API
void GWEN_PointerList_Clear(GWEN_POINTERLIST *idl);

/**
 * Adds a ptr to the list. This function does no doublecheck.
 */
GWENHYWFAR_API
int GWEN_PointerList_AddPtr(GWEN_POINTERLIST *idl, void *ptr);

/**
 * Removes the first occurrence of the given ptr.
 * @return 0 if deleted, !=0 if the ptr wasn't found
 */
GWENHYWFAR_API
int GWEN_PointerList_DelPtr(GWEN_POINTERLIST *idl, void *ptr);

/**
 * Checks whether the given ptr exists in the ptrlist.
 * @return 0 if found, !=0 otherwise
 */
GWENHYWFAR_API
int GWEN_PointerList_HasPtr(const GWEN_POINTERLIST *idl, void *ptr);

/**
 */
GWENHYWFAR_API
void *GWEN_PointerList_GetFirstPtr(const GWEN_POINTERLIST *idl, uint64_t *hdl);

/**
 */
GWENHYWFAR_API
void *GWEN_PointerList_GetNextPtr(const GWEN_POINTERLIST *idl, uint64_t *hdl);


/**
 * Always append ptr at the end of the list.
 */
GWENHYWFAR_API
int GWEN_PointerList_AppendPtr(GWEN_POINTERLIST *idl, void *ptr);

GWENHYWFAR_API
void *GWEN_PointerList_GetPtrAt(const GWEN_POINTERLIST *idl, uint64_t index);


GWENHYWFAR_API
uint64_t GWEN_PointerList_GetEntryCount(const GWEN_POINTERLIST *idl);

/*@}*/




/**
 * @defgroup MOD_POINTERLIST_ITERATOR Iterator for Pointer list module
 * @ingroup MOD_BASE
 * @short Iterator for a list of uint64_t objects
 *
 */
/*@{*/

GWENHYWFAR_API
GWEN_POINTERLIST_ITERATOR *GWEN_PointerList_Iterator_new(GWEN_POINTERLIST *idl);

GWENHYWFAR_API
void GWEN_PointerList_Iterator_free(GWEN_POINTERLIST_ITERATOR *it);

GWENHYWFAR_API
void *GWEN_PointerList_Iterator_GetFirstId(GWEN_POINTERLIST_ITERATOR *it);

GWENHYWFAR_API
void *GWEN_PointerList_Iterator_GetNextId(GWEN_POINTERLIST_ITERATOR *it);

/*@}*/


#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_POINTERLIST_H */


