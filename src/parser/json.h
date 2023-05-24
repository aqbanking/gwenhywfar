/***************************************************************************
 copyright   : (C) 2023 by Martin Preuss
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


#ifndef GWENHYWFAR_JSON_H
#define GWENHYWFAR_JSON_H


#include <gwenhywfar/tree2.h>



typedef struct GWEN_JSON_ELEM GWEN_JSON_ELEM;
GWEN_TREE2_FUNCTION_LIB_DEFS(GWEN_JSON_ELEM, GWEN_JsonElement, GWENHYWFAR_API);


enum {
  GWEN_JSON_ELEMTYPE_UNKNOWN=0,
  GWEN_JSON_ELEMTYPE_KEY=1,
  GWEN_JSON_ELEMTYPE_NULL,
  GWEN_JSON_ELEMTYPE_BOOL,
  GWEN_JSON_ELEMTYPE_NUM,
  GWEN_JSON_ELEMTYPE_STRING,
  GWEN_JSON_ELEMTYPE_ARRAY,
  GWEN_JSON_ELEMTYPE_OBJECT
};



GWENHYWFAR_API GWEN_JSON_ELEM *GWEN_JsonElement_new(int t, const char *sData);
GWENHYWFAR_API void GWEN_JsonElement_free(GWEN_JSON_ELEM *je);

GWENHYWFAR_API int GWEN_JsonElementGetType(const GWEN_JSON_ELEM *je);
GWENHYWFAR_API void GWEN_JsonElementSetType(GWEN_JSON_ELEM *je, int t);

GWENHYWFAR_API const char *GWEN_JsonElementGetData(const GWEN_JSON_ELEM *je);
GWENHYWFAR_API void GWEN_JsonElementSetData(GWEN_JSON_ELEM *je, const char *s);



#endif

