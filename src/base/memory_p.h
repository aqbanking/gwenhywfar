/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sat Jun 28 2003
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


#ifndef GWENHYWFAR_MEMORY_P_H
#define GWENHYWFAR_MEMORY_P_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GWEN_MEMORY__OBJECT_STRING GWEN_MEMORY__OBJECT_STRING;
typedef struct GWEN_MEMORY__OBJECT GWEN_MEMORY__OBJECT;

#ifdef __cplusplus
}
#endif


#include <gwenhywfar/misc.h>
#include <gwenhywfar/memory.h>
#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#define GWEN_MEMORY_ENV_DEBUG "GWEN_MEMORY_DEBUG"
#define GWEN_MEMORY_ENV_DUMP_ALL "GWEN_MEMORY_DUMP_ALL"
#define GWEN_MEMORY_ENV_DOUBLE_CHECK "GWEN_MEMORY_DOUBLE_CHECK"



#ifdef __cplusplus
extern "C" {
#endif


  GWEN_LIST_FUNCTION_DEFS(GWEN_MEMORY__OBJECT_STRING,
                          GWEN_Memory__Object_String)
  /* No trailing semicolon here because this is a macro call */


  struct GWEN_MEMORY__OBJECT_STRING {
    GWEN_LIST_ELEMENT(GWEN_MEMORY__OBJECT_STRING)
    char *text;
  };

  GWEN_MEMORY__OBJECT_STRING*
    GWEN_Memory__Object_String_new(const char *s);

  void GWEN_Memory__Object_String_free(GWEN_MEMORY__OBJECT_STRING *s);





  GWEN_LIST_FUNCTION_DEFS(GWEN_MEMORY__OBJECT, GWEN_Memory__Object)

  struct GWEN_MEMORY__OBJECT {
    GWEN_LIST_ELEMENT(GWEN_MEMORY__OBJECT)

    void *object;
    char *typeName;
    char *locationNew;
    GWEN_MEMORY__OBJECT_STRING_LIST *locationsFree;
    GWEN_MEMORY__OBJECT_STRING_LIST *locationsAttach;
    int usage;
  };

  GWEN_MEMORY__OBJECT *GWEN_Memory__Object_new(void *ptr,
                                               const char *typeName,
                                               const char *location);
  void GWEN_Memory__Object_free(GWEN_MEMORY__OBJECT *o);


#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_MEMORY_P_H */

