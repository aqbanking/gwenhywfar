/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 08 2003
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


#ifndef GWEN_KEYSPEC_H
#define GWEN_KEYSPEC_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/list2.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_KEYSPEC GWEN_KEYSPEC;

GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_KEYSPEC, GWENHYWFAR_API)
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_KEYSPEC, GWEN_KeySpec, GWENHYWFAR_API)
GWEN_LIST2_FUNCTION_LIB_DEFS(GWEN_KEYSPEC, GWEN_KeySpec, GWENHYWFAR_API)
void GWEN_KeySpec_List2_freeAll(GWEN_KEYSPEC_LIST2 *ksl);


GWENHYWFAR_API
GWEN_KEYSPEC *GWEN_KeySpec_new();
GWENHYWFAR_API
GWEN_KEYSPEC *GWEN_KeySpec_dup(const GWEN_KEYSPEC *ks);
GWENHYWFAR_API
void GWEN_KeySpec_free(GWEN_KEYSPEC *ks);

GWENHYWFAR_API
  const char *GWEN_KeySpec_GetKeyType(const GWEN_KEYSPEC *ks);
GWENHYWFAR_API
  void GWEN_KeySpec_SetKeyType(GWEN_KEYSPEC *ks,
                               const char *s);

/**
 * Returns the status of the key. This property is not used by gwen, but it
 * might be used by applications. Therefore the values of this property are
 * defined by the application which uses it.
 */
GWENHYWFAR_API
int GWEN_KeySpec_GetStatus(const GWEN_KEYSPEC *ks);
GWENHYWFAR_API
void GWEN_KeySpec_SetStatus(GWEN_KEYSPEC *ks, int i);

GWENHYWFAR_API
  const char *GWEN_KeySpec_GetKeyName(const GWEN_KEYSPEC *ks);
GWENHYWFAR_API
  void GWEN_KeySpec_SetKeyName(GWEN_KEYSPEC *ks,
                               const char *s);

GWENHYWFAR_API
  const char *GWEN_KeySpec_GetOwner(const GWEN_KEYSPEC *ks);
GWENHYWFAR_API
  void GWEN_KeySpec_SetOwner(GWEN_KEYSPEC *ks,
                             const char *s);

GWENHYWFAR_API
  unsigned int GWEN_KeySpec_GetNumber(const GWEN_KEYSPEC *ks);
GWENHYWFAR_API
  void GWEN_KeySpec_SetNumber(GWEN_KEYSPEC *ks,
                              unsigned int i);

GWENHYWFAR_API
  unsigned int GWEN_KeySpec_GetVersion(const GWEN_KEYSPEC *ks);
GWENHYWFAR_API
  void GWEN_KeySpec_SetVersion(GWEN_KEYSPEC *ks,
                               unsigned int i);

GWENHYWFAR_API
unsigned int GWEN_KeySpec_GetKeyLength(const GWEN_KEYSPEC *ks);

GWENHYWFAR_API
void GWEN_KeySpec_SetKeyLength(GWEN_KEYSPEC *ks, unsigned int i);


GWENHYWFAR_API
void GWEN_KeySpec_Dump(const GWEN_KEYSPEC *ks, FILE *f, unsigned int indent);


GWENHYWFAR_API int GWEN_KeySpec_toDb(const GWEN_KEYSPEC *ks, GWEN_DB_NODE *n);
GWENHYWFAR_API GWEN_KEYSPEC *GWEN_KeySpec_fromDb(GWEN_DB_NODE *n);


#ifdef __cplusplus
}
#endif



#endif /* GWEN_KEYSPEC_H */


