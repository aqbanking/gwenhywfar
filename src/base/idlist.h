/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
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

/** @file gwenhywfar/idlist.h
 * @short A list of GWEN_TYPE_UINT32 objects
 */


#ifndef GWENHYWFAR_IDLIST_H
#define GWENHYWFAR_IDLIST_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct GWEN_IDLIST GWEN_IDLIST;
#ifdef __cplusplus
}
#endif

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup MOD_IDLIST ID list module
 * @ingroup MOD_BASE
 * @short A list of GWEN_TYPE_UINT32 objects
 *
 * This is basically a list for GWEN_TYPE_UINT32 objects,
 * but since it uses tables instead of those list objects normally used
 * throughout Gwen it is less memory consuming.
 */
/*@{*/
/**
 * Constructor.
 */
GWEN_IDLIST *GWEN_IdList_new();

/**
 * Destructor.
 */
void GWEN_IdList_free(GWEN_IDLIST *idl);

/**
 * Adds an id to the list. This function does no doublecheck.
 */
int GWEN_IdList_AddId(GWEN_IDLIST *idl, GWEN_TYPE_UINT32 id);

/**
 * Removes the first occurrence of the given id.
 * @return 0 if deleted, !=0 if the id wasn't found
 */
int GWEN_IdList_DelId(GWEN_IDLIST *idl, GWEN_TYPE_UINT32 id);

/**
 * Checks whether the given id exists in the idlist.
 * @return 0 if found, !=0 otherwise
 */
int GWEN_IdList_HasId(const GWEN_IDLIST *idl, GWEN_TYPE_UINT32 id);

/**
 * Returns the first id from the list.
 */
GWEN_TYPE_UINT32 GWEN_IdList_GetFirstId(GWEN_IDLIST *idl);

GWEN_TYPE_UINT32 GWEN_IdList_GetNextId(GWEN_IDLIST *idl);
/*@}*/


#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_IDLIST_H */


