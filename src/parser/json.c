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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "./json_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>

#include <ctype.h>



GWEN_TREE2_FUNCTIONS(GWEN_JSON_ELEM, GWEN_JsonElement);




GWEN_JSON_ELEM *GWEN_JsonElement_new(int t, const char *sData)
{
  GWEN_JSON_ELEM *je;

  GWEN_NEW_OBJECT(GWEN_JSON_ELEM, je);
  GWEN_TREE2_INIT(GWEN_JSON_ELEM, je, GWEN_JsonElement);
  GWEN_JsonElementSetType(je, t);
  GWEN_JsonElementSetData(je, sData);
  return je;
}



void GWEN_JsonElement_free(GWEN_JSON_ELEM *je)
{
  if (je) {
    GWEN_TREE2_FINI(GWEN_JSON_ELEM, je, GWEN_JsonElement);
    if (je->elementData)
      free(je->elementData);
    GWEN_FREE_OBJECT(je);
  }
}



int GWEN_JsonElementGetType(const GWEN_JSON_ELEM *je)
{
  if (je)
    return je->elementType;
  return GWEN_JSON_ELEMTYPE_UNKNOWN;
}



void GWEN_JsonElementSetType(GWEN_JSON_ELEM *je, int t)
{
  if (je)
    je->elementType=t;
}



const char *GWEN_JsonElementGetData(const GWEN_JSON_ELEM *je)
{
  if (je)
    return je->elementData;
  return NULL;
}



void GWEN_JsonElementSetData(GWEN_JSON_ELEM *je, const char *s)
{
  if (je) {
    free(je->elementData);
    je->elementData=(s&&*s)?strdup(s):NULL;
  }
}




