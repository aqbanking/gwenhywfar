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
#define GWEN_MEMORY_ENV_NO_FREE "GWEN_MEMORY_NO_FREE"
#define GWEN_MEMORY_ENV_VERBOUS "GWEN_MEMORY_VERBOUS"


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

GWEN_MEMORY__OBJECT_STRING *
GWEN_Memory__Object_String_new(const char *s);

void GWEN_Memory__Object_String_free(GWEN_MEMORY__OBJECT_STRING *s);





GWEN_LIST_FUNCTION_DEFS(GWEN_MEMORY__OBJECT, GWEN_Memory__Object)

struct GWEN_MEMORY__OBJECT {
  GWEN_LIST_ELEMENT(GWEN_MEMORY__OBJECT)

  void *object;
  size_t size;
  char *typeName;
  char *locationNew;
  GWEN_MEMORY__OBJECT_STRING_LIST *locationsFree;
  GWEN_MEMORY__OBJECT_STRING_LIST *locationsAttach;
  int usage;
};

GWEN_MEMORY__OBJECT *GWEN_Memory__Object_new(void *ptr,
                                             size_t size,
                                             const char *typeName,
                                             const char *location);
void GWEN_Memory__Object_free(GWEN_MEMORY__OBJECT *o);




#define GWEN_MEMORY_MASK_INUSE      0x8000
#define GWEN_MEMORY_MASK_MALLOCED   0x4000
#define GWEN_MEMORY_MASK_LEN        0x3fff
#define GWEN_MEMORY_TABLE_LEN       (16*1024)
#define GWEN_MEMORY_SIZELEN         2
#define GWEN_MEMORY_MAXBLOCK (GWEN_MEMORY_TABLE_LEN-GWEN_MEMORY_SIZELEN-2)
#define GWEN_MEMORY_MINREMAIN       2
#define GWEN_MEMORY_EXTERNAL        0xffff

#define GWEN_MEMORY_READSIZE(p)     ((p)[0]+((p)[1]<<8))
#define GWEN_MEMORY_WRITESIZE(p, s) \
  { (p)[0]=((s) & 0xff); (p)[1]=(((s)>>8) & 0xff);}
#define GWEN_MEMORY_GETDATA(p) ((p)+GWEN_MEMORY_SIZELEN)
#define GWEN_MEMORY_GETSTART(p) ((p)-GWEN_MEMORY_SIZELEN)

#define GWEN_MEMORY_GRANULARITY 32

#define GWEN_MEMORY_COLLECT_AFTER    (1024*1024)


typedef struct GWEN_MEMORY_TABLE GWEN_MEMORY_TABLE;
struct GWEN_MEMORY_TABLE {
  GWEN_MEMORY_TABLE *next;
  unsigned char data[GWEN_MEMORY_TABLE_LEN];
};

GWEN_MEMORY_TABLE *GWEN_Memory_Table_new();
void GWEN_Memory_Table_free(GWEN_MEMORY_TABLE *mt);

void GWEN_Memory_Table_Append(GWEN_MEMORY_TABLE *head,
                              GWEN_MEMORY_TABLE *mt);


void GWEN_Memory_Table__Dump(GWEN_MEMORY_TABLE *mt);
void GWEN_Memory_Table__Collect(GWEN_MEMORY_TABLE *mt);


#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_MEMORY_P_H */

