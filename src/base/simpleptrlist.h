/***************************************************************************
    begin       : Fri Dec 06 2019
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

#ifndef GWENHYWFAR_SIMPLEPTRLIST_H
#define GWENHYWFAR_SIMPLEPTRLIST_H

#include <gwenhywfar/types.h>



typedef struct GWEN_SIMPLEPTRLIST GWEN_SIMPLEPTRLIST;

GWENHYWFAR_API GWEN_SIMPLEPTRLIST *GWEN_SimplePtrList_new(uint64_t startEntries, uint64_t steps);
GWENHYWFAR_API void GWEN_SimplePtrList_Attach(GWEN_SIMPLEPTRLIST *pl);
GWENHYWFAR_API void GWEN_SimplePtrList_free(GWEN_SIMPLEPTRLIST *pl);

GWENHYWFAR_API GWEN_SIMPLEPTRLIST *GWEN_SimplePtrList_LazyCopy(GWEN_SIMPLEPTRLIST *oldList);


GWENHYWFAR_API void *GWEN_SimplePtrList_GetPtrAt(const GWEN_SIMPLEPTRLIST *pl, uint64_t idx);
GWENHYWFAR_API void *GWEN_SimplePtrList_SetPtrAt(GWEN_SIMPLEPTRLIST *pl, uint64_t idx, void *p);
GWENHYWFAR_API int64_t GWEN_SimplePtrList_AddPtr(GWEN_SIMPLEPTRLIST *pl, void *p);

GWENHYWFAR_API uint64_t GWEN_SimplePtrList_GetSteps(const GWEN_SIMPLEPTRLIST *pl);
GWENHYWFAR_API void GWEN_SimplePtrList_SetSteps(GWEN_SIMPLEPTRLIST *pl, uint64_t steps);



GWENHYWFAR_API uint64_t GWEN_SimplePtrList_GetMaxEntries(const GWEN_SIMPLEPTRLIST *pl);
GWENHYWFAR_API uint64_t GWEN_SimplePtrList_GetUsedEntries(const GWEN_SIMPLEPTRLIST *pl);

GWENHYWFAR_API void *GWEN_SimplePtrList_GetEntries(const GWEN_SIMPLEPTRLIST *pl);


GWENHYWFAR_API int GWEN_SimplePtrList_Test(void);


#endif /* GWENHYWFAR_SIMPLEPTRLIST_P_H */

