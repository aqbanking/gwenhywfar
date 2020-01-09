/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2020 by Martin Preuss
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

/** @file idlist.h
 * @short A list of uint64_t objects
 */


#ifndef GWENHYWFAR_IDLIST64_H
#define GWENHYWFAR_IDLIST64_H


#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/simpleptrlist.h>


#ifdef __cplusplus
extern "C" {
#endif
typedef GWEN_SIMPLEPTRLIST GWEN_IDLIST64;
typedef struct GWEN_IDLIST64_ITERATOR GWEN_IDLIST64_ITERATOR;
#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
extern "C" {
#endif



/**
 * @defgroup MOD_IDLIST64 ID list module
 * @ingroup MOD_BASE
 * @short A list of uint64_t objects
 *
 * This is basically a list for uint64_t objects,
 * but since it uses tables instead of those list objects normally used
 * throughout Gwen it is less memory consuming.
 */
/*@{*/



GWENHYWFAR_API GWEN_IDLIST64 *GWEN_IdList64_new(void);
GWENHYWFAR_API GWEN_IDLIST64 *GWEN_IdList64_newWithSteps(uint64_t steps);
GWENHYWFAR_API void GWEN_IdList64_Attach(GWEN_IDLIST64 *idl);
GWENHYWFAR_API void GWEN_IdList64_free(GWEN_IDLIST64 *idl);

GWENHYWFAR_API void GWEN_IdList64_Clear(GWEN_IDLIST64 *idl);

GWENHYWFAR_API GWEN_IDLIST64 *GWEN_IdList64_dup(const GWEN_IDLIST64 *oldList);
GWENHYWFAR_API GWEN_IDLIST64 *GWEN_IdList64_LazyCopy(GWEN_IDLIST64 *oldList);

/**
 * Adds an id to the list, returns its index. This function does no doublecheck.
 */
GWENHYWFAR_API int64_t GWEN_IdList64_AddId(GWEN_IDLIST64 *idl, uint64_t id);

/**
 * Removes the first occurrence of the given id.
 * @return 0 if deleted, !=0 if the id wasn't found
 */
GWENHYWFAR_API int GWEN_IdList64_DelId(GWEN_IDLIST64 *idl, uint64_t id);

/**
 * Checks whether the given id exists in the idlist.
 * @return 1 if found, 0 otherwise
 */
GWENHYWFAR_API int GWEN_IdList64_HasId(const GWEN_IDLIST64 *idl, uint64_t id);

GWENHYWFAR_API int64_t GWEN_IdList64_GetIdAt(const GWEN_IDLIST64 *idl, uint64_t index);


GWENHYWFAR_API uint64_t GWEN_IdList64_GetEntryCount(const GWEN_IDLIST64 *idl);

/**
 * Sorts the ids in ascending order
 */
GWENHYWFAR_API int GWEN_IdList64_Sort(GWEN_IDLIST64 *idl);


GWENHYWFAR_API int GWEN_IdList64_ReverseSort(GWEN_IDLIST64 *idl);


GWENHYWFAR_API int GWEN_IdList64_Test(void);


/*@}*/



/**
 * @defgroup MOD_IDLIST64_ITERATOR Iterator for ID list module
 * @ingroup MOD_BASE
 * @short Iterator for a list of uint64_t objects
 *
 */
/*@{*/

GWENHYWFAR_API GWEN_IDLIST64_ITERATOR *GWEN_IdList64_Iterator_new(GWEN_IDLIST64 *idl);

GWENHYWFAR_API void GWEN_IdList64_Iterator_free(GWEN_IDLIST64_ITERATOR *it);

GWENHYWFAR_API uint64_t GWEN_IdList64_Iterator_GetFirstId(GWEN_IDLIST64_ITERATOR *it);

GWENHYWFAR_API uint64_t GWEN_IdList64_Iterator_GetNextId(GWEN_IDLIST64_ITERATOR *it);

/*@}*/



#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_IDLIST64_H */


