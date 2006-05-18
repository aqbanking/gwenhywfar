/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: idlist_p.h 1048 2006-05-17 17:15:35Z martin $
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

#ifndef GWENHYWFAR_IDMAP_H
#define GWENHYWFAR_IDMAP_H


#include <gwenhywfar/types.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_IDMAP GWEN_IDMAP;

typedef enum {
  GWEN_IdMapResult_Ok=0,
  GWEN_IdMapResult_NoFit,
  GWEN_IdMapResult_NotFound
} GWEN_IDMAP_RESULT;


typedef enum {
  GWEN_IdMapAlgo_Unknown=0,
  GWEN_IdMapAlgo_Hex4
} GWEN_IDMAP_ALGO;


/** @name Macros for Typesafe ID maps
 *
 */
/*@{*/
#ifndef GWEN_DUMMY_EMPTY_ARG
/** Necessary for MSVC compiler because it does not accept a left-out
    macro argument. */
# define GWEN_DUMMY_EMPTY_ARG
#endif


#define GWEN_IDMAP_FUNCTION_LIB_DEFS(t, pr, decl) \
  typedef GWEN_IDMAP t##_IDMAP;                                       \
                                                                      \
  decl t##_IDMAP *pr##_IdMap_new(GWEN_IDMAP_ALGO algo);               \
  decl void pr##_IdMap_free(t##_IDMAP *l);                            \
  decl GWEN_IDMAP_RESULT pr##_IdMap_Insert(t##_IDMAP *l,              \
                                           GWEN_TYPE_UINT32 id,       \
                                           t* ptr);                   \
  decl GWEN_IDMAP_RESULT pr##_IdMap_Remove(t##_IDMAP *l,              \
                                           GWEN_TYPE_UINT32 id);      \
  decl t* pr##_IdMap_Find(t##_IDMAP *l, GWEN_TYPE_UINT32 id);         \
  decl GWEN_IDMAP_RESULT pr##_IdMap_GetFirst(t##_IDMAP *map,         \
                                              GWEN_TYPE_UINT32 *pid); \
  decl GWEN_IDMAP_RESULT pr##_IdMap_GetNext(t##_IDMAP *map,          \
                                             GWEN_TYPE_UINT32 *pid);  \
  decl GWEN_TYPE_UINT32 pr##_IdMap_GetSize(const GWEN_IDMAP *map);

#define GWEN_IDMAP_FUNCTION_DEFS(t, pr) \
  GWEN_IDMAP_FUNCTION_LIB_DEFS(t, pr, GWEN_DUMMY_EMPTY_ARG)


#define GWEN_IDMAP_FUNCTIONS(t, pr) \
  t##_IDMAP *pr##_IdMap_new(GWEN_IDMAP_ALGO algo) {                  \
    return (t##_IDMAP*)GWEN_IdMap_new(algo);                         \
  }                                                                  \
                                                                     \
  void pr##_IdMap_free(t##_IDMAP *l) {                               \
    GWEN_IdMap_free((GWEN_IDMAP*)l);                                 \
  }                                                                  \
                                                                     \
  GWEN_IDMAP_RESULT pr##_IdMap_Insert(t##_IDMAP *l,                  \
                                      GWEN_TYPE_UINT32 id,           \
                                      t* ptr) {                      \
    return GWEN_IdMap_Insert((GWEN_IDMAP*)l, id, (void*) ptr);       \
  }                                                                  \
                                                                     \
  GWEN_IDMAP_RESULT pr##_IdMap_Remove(t##_IDMAP *l,                  \
                                      GWEN_TYPE_UINT32 id){          \
    return GWEN_IdMap_Remove((GWEN_IDMAP*)l, id);                    \
  }                                                                  \
\
  t* pr##_IdMap_Find(t##_IDMAP *l, GWEN_TYPE_UINT32 id) {            \
    return GWEN_IdMap_Find((GWEN_IDMAP*)l, id);                      \
  }                                                                  \
                                                                     \
  GWEN_IDMAP_RESULT pr##_IdMap_GetFirst(t##_IDMAP *l,               \
                                        GWEN_TYPE_UINT32 *pid) {     \
    return GWEN_IdMap_GetFirst((GWEN_IDMAP*)l, pid);                \
  }                                                                  \
                                                                     \
  GWEN_IDMAP_RESULT pr##_IdMap_GetNext(t##_IDMAP *l,                \
                                       GWEN_TYPE_UINT32 *pid) {      \
    return GWEN_IdMap_GetNext((GWEN_IDMAP*)l, pid);                 \
  }                                                                  \
                                                                     \
  GWEN_TYPE_UINT32 pr##_IdMap_GetSize(const GWEN_IDMAP *l) {       \
    return GWEN_IdMap_GetSize((const GWEN_IDMAP*)l);                 \
  }
/*@}*/



GWENHYWFAR_API
GWEN_IDMAP *GWEN_IdMap_new(GWEN_IDMAP_ALGO algo);

GWENHYWFAR_API
void GWEN_IdMap_free(GWEN_IDMAP *map);

GWENHYWFAR_API
GWEN_IDMAP_RESULT GWEN_IdMap_Insert(GWEN_IDMAP *map,
				    GWEN_TYPE_UINT32 id,
				    void *ptr);

GWENHYWFAR_API
GWEN_IDMAP_RESULT GWEN_IdMap_Remove(GWEN_IDMAP *map,
                                    GWEN_TYPE_UINT32 id);

GWENHYWFAR_API
void *GWEN_IdMap_Find(GWEN_IDMAP *map, GWEN_TYPE_UINT32 id);


GWENHYWFAR_API
GWEN_IDMAP_RESULT GWEN_IdMap_GetFirst(GWEN_IDMAP *map,
                                       GWEN_TYPE_UINT32 *pid);

GWENHYWFAR_API
GWEN_IDMAP_RESULT GWEN_IdMap_GetNext(GWEN_IDMAP *map,
                                     GWEN_TYPE_UINT32 *pid);

GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_IdMap_GetSize(const GWEN_IDMAP *map);


#ifdef __cplusplus
}
#endif


#endif

