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
#include "./json_dump.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>

#include <ctype.h>



void GWEN_JsonElement_DumpToBuffer(const GWEN_JSON_ELEM *jeRoot, int indent, GWEN_BUFFER *buf)
{
  const GWEN_JSON_ELEM *je;
  const char *s;

  GWEN_Buffer_FillWithBytes(buf, ' ', indent);
  s=GWEN_JsonElementGetData(jeRoot);
  switch(GWEN_JsonElementGetType(jeRoot)) {
  case GWEN_JSON_ELEMTYPE_KEY:    GWEN_Buffer_AppendArgs(buf,   "type=KEY, ..... data=%s", s?s:"<empty"); break;
  case GWEN_JSON_ELEMTYPE_NULL:   GWEN_Buffer_AppendString(buf, "type=NULL"); break;
  case GWEN_JSON_ELEMTYPE_BOOL:   GWEN_Buffer_AppendArgs(buf,   "type=BOOL, .... data=%s", s?s:"<empty"); break;
  case GWEN_JSON_ELEMTYPE_NUM:    GWEN_Buffer_AppendArgs(buf,   "type=NUM, ..... data=%s", s?s:"<empty"); break;
  case GWEN_JSON_ELEMTYPE_STRING: GWEN_Buffer_AppendArgs(buf,   "type=STRING, .. data=%s", s?s:"<empty"); break;
  case GWEN_JSON_ELEMTYPE_ARRAY:  GWEN_Buffer_AppendString(buf, "type=ARRAY"); break;
  case GWEN_JSON_ELEMTYPE_OBJECT: GWEN_Buffer_AppendString(buf, "type=OBJECT"); break;
  default:                        GWEN_Buffer_AppendArgs(buf,   "type=<unknown>, data=%s", s?s:"<empty"); break;
  }
  GWEN_Buffer_AppendString(buf, "\n");

  je=GWEN_JsonElement_Tree2_GetFirstChild(jeRoot);
  while(je) {
    GWEN_JsonElement_DumpToBuffer(je, indent+2, buf);
    je=GWEN_JsonElement_Tree2_GetNext(je);
  }
}




