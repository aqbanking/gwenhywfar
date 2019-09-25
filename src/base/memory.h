/***************************************************************************
 begin       : Sat Jun 28 2003
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


#ifndef GWENHYWFAR_MEMORY_H
#define GWENHYWFAR_MEMORY_H

#include <gwenhywfar/gwenhywfarapi.h>



#ifdef __cplusplus
extern "C" {
#endif



GWENHYWFAR_API void *GWEN_Memory_malloc(size_t dsize);
GWENHYWFAR_API void GWEN_Memory_dealloc(void *p);

GWENHYWFAR_API void *GWEN_Memory_realloc(void *oldp, size_t nsize);

GWENHYWFAR_API char *GWEN_Memory_strdup(const char *s);



#define GWEN_MEM_NEW(typ, memptr) \
  memptr=(typ*)GWEN_Memory_malloc(sizeof(typ));

#define GWEN_MEM_FREE(varname) \
  GWEN_Memory_dealloc((void*)varname);


#define GWEN_NEW_OBJECT(typ, varname)\
  {\
    varname=(typ*)GWEN_Memory_malloc(sizeof(typ)); \
    memset(varname, 0, sizeof(typ));\
  }

#define GWEN_FREE_OBJECT(varname) \
  GWEN_Memory_dealloc((void*)varname);


#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_MEMORY_H */

