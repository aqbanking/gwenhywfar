/***************************************************************************
    begin       : Wed May 08 2013
    copyright   : (C) 2013 by Martin Preuss
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


#ifndef GWENHYWFAR_MULTICACHE_H
#define GWENHYWFAR_MULTICACHE_H


#include <gwenhywfar/list1.h>
#include <gwenhywfar/misc.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup MOD_MULTICACHE Multi Cache Module
 * @ingroup MOD_BASE
 *
 * @brief This file contains the definition of a GWEN_MULTICACHE, a
 * cache for multiple types of data.
 *
 * The idea behind this is to make it easier to handle multiple types of objects
 * by a single central cache object. You only set the maximum total size the central
 * cache object is allowed to provide and that object will use this sum of memory to
 * provide a caching mechanism for multiple different types.
 *
 * You'll have to create a single central cache object (GWEN_MULTICACHE). Then you can
 * create a GWEN_MULTICACHE_TYPE object for each kind of data you want to have managed.
 * For the GWEN_MULTICACHE_TYPE object you need to provide functions which will increase
 * and decrease the reference counter of the object cached.
 *
 */
/*@{*/


typedef struct GWEN_MULTICACHE GWEN_MULTICACHE;
typedef struct GWEN_MULTICACHE_TYPE GWEN_MULTICACHE_TYPE;


typedef int GWENHYWFAR_CB (*GWEN_MULTICACHE_TYPE_ATTACH_FN)(void *p);
typedef int GWENHYWFAR_CB (*GWEN_MULTICACHE_TYPE_FREE_FN)(void *p);



/** @name Per-type Cache Objects
 *
 * This object describes one of the data types which can be managed by a central
 * cache object.
 */
/*@{*/

GWENHYWFAR_API GWEN_MULTICACHE_TYPE *GWEN_MultiCache_Type_new(GWEN_MULTICACHE *mc);

GWENHYWFAR_API void GWEN_MultiCache_Type_free(GWEN_MULTICACHE_TYPE *ct);

GWENHYWFAR_API void *GWEN_MultiCache_Type_GetData(const GWEN_MULTICACHE_TYPE *ct, uint32_t id);
GWENHYWFAR_API void *GWEN_MultiCache_Type_GetDataWithParams(const GWEN_MULTICACHE_TYPE *ct, uint32_t id,
                                                           uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4);

GWENHYWFAR_API void GWEN_MultiCache_Type_SetData(GWEN_MULTICACHE_TYPE *ct, uint32_t id, void *ptr, uint32_t size);
GWENHYWFAR_API void GWEN_MultiCache_Type_SetDataWithParams(GWEN_MULTICACHE_TYPE *ct, uint32_t id, void *ptr, uint32_t size,
                                                           uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4);

GWENHYWFAR_API void GWEN_MultiCache_Type_PurgeData(GWEN_MULTICACHE_TYPE *ct, uint32_t id);

GWENHYWFAR_API void GWEN_MultiCache_Type_PurgeAll(GWEN_MULTICACHE_TYPE *ct);


GWENHYWFAR_API void GWEN_MultiCache_Type_SetAttachFn(GWEN_MULTICACHE_TYPE *ct, GWEN_MULTICACHE_TYPE_ATTACH_FN fn);

GWENHYWFAR_API void GWEN_MultiCache_Type_SetFreeFn(GWEN_MULTICACHE_TYPE *ct, GWEN_MULTICACHE_TYPE_FREE_FN fn);

/*@}*/



/** @name Central Cache Object
 *
 * This is the central cache object which can hold multiple types of data.
 */
/*@{*/
GWENHYWFAR_API GWEN_MULTICACHE *GWEN_MultiCache_new(uint64_t maxSize);

GWENHYWFAR_API void GWEN_MultiCache_free(GWEN_MULTICACHE *mc);


GWENHYWFAR_API uint64_t GWEN_MultiCache_GetMaxSizeUsed(const GWEN_MULTICACHE *mc);

/*@}*/



/*@}*/



#ifdef __cplusplus
}
#endif




#endif

