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
#include <gwenhywfar/path.h>

#include <ctype.h>



GWEN_TREE2_FUNCTIONS(GWEN_JSON_ELEM, GWEN_JsonElement);



static GWEN_JSON_ELEM *_findByTypeAndData(GWEN_JSON_ELEM *je, int t, const char *s);
static void *_handlePath(const char *entry, void *data, int idx, uint32_t flags);



GWEN_JSON_ELEM *GWEN_JsonElement_new(int t, const char *sData)
{
  GWEN_JSON_ELEM *je;

  GWEN_NEW_OBJECT(GWEN_JSON_ELEM, je);
  GWEN_TREE2_INIT(GWEN_JSON_ELEM, je, GWEN_JsonElement);
  GWEN_JsonElement_SetType(je, t);
  GWEN_JsonElement_SetData(je, sData);
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



int GWEN_JsonElement_GetType(const GWEN_JSON_ELEM *je)
{
  if (je)
    return je->elementType;
  return GWEN_JSON_ELEMTYPE_UNKNOWN;
}



void GWEN_JsonElement_SetType(GWEN_JSON_ELEM *je, int t)
{
  if (je)
    je->elementType=t;
}



const char *GWEN_JsonElement_GetData(const GWEN_JSON_ELEM *je)
{
  if (je)
    return je->elementData;
  return NULL;
}



void GWEN_JsonElement_SetData(GWEN_JSON_ELEM *je, const char *s)
{
  if (je) {
    free(je->elementData);
    je->elementData=(s&&*s)?strdup(s):NULL;
  }
}




GWEN_JSON_ELEM *GWEN_JsonElement_FindFirstByTypeAndData(const GWEN_JSON_ELEM *je, int t, const char *s)
{
  GWEN_JSON_ELEM *jeChild;

  jeChild=GWEN_JsonElement_Tree2_GetFirstChild(je);
  return _findByTypeAndData(jeChild, t, s);
}



GWEN_JSON_ELEM *GWEN_JsonElement_FindNextByTypeAndData(const GWEN_JSON_ELEM *je, int t, const char *s)
{
  GWEN_JSON_ELEM *jeNext;

  jeNext=GWEN_JsonElement_Tree2_GetNext(je);
  return _findByTypeAndData(jeNext, t, s);
}



GWEN_JSON_ELEM *GWEN_JsonElement_FindByIdxTypeAndData(const GWEN_JSON_ELEM *jeSearch, int t, const char *s, int idx)
{
  GWEN_JSON_ELEM *je;

  je=GWEN_JsonElement_FindFirstByTypeAndData(jeSearch, t, s);
  while(je && idx) {
    je=GWEN_JsonElement_FindNextByTypeAndData(je, t, s);
    idx--;
  }
  return (je && idx==0)?je:NULL;
}




GWEN_JSON_ELEM *GWEN_JsonElement_GetElementByPath(GWEN_JSON_ELEM *je, const char *path, uint32_t flags)
{
  return (GWEN_JSON_ELEM *)GWEN_Path_HandleWithIdx(path, je, flags, _handlePath);
}



GWEN_JSON_ELEM *_findByTypeAndData(GWEN_JSON_ELEM *je, int t, const char *s)
{
  DBG_ERROR(NULL, "Searching for \"%s\" [%d]", s, t);
  while(je) {
    int currentType;

    currentType=GWEN_JsonElement_GetType(je);
    if (t==0 || (currentType==t)) {
      if (s && *s) {
        const char *s2;

        s2=GWEN_JsonElement_GetData(je);
	DBG_ERROR(NULL, "  Comparing \"%s\" [%d] against \"%s\" [%d]", s2?s2:"<empty>", currentType, s?s:"<empty>", t);
        if (s2 && strcasecmp(s, s2)==0)
	  return je;
      }
      else
        return je;
    }
    else {
      if (t==GWEN_JSON_ELEMTYPE_KEY && (currentType==GWEN_JSON_ELEMTYPE_ARRAY || currentType==GWEN_JSON_ELEMTYPE_OBJECT)) {
	GWEN_JSON_ELEM *je2;

	DBG_ERROR(NULL, "Searching below [%d]", currentType);
	je2=GWEN_JsonElement_Tree2_GetFirstChild(je);
	if (je2) {
	  je2=_findByTypeAndData(je2, t, s);
	  if (je2)
	    return je2;
	}
      }
    }
    je=GWEN_JsonElement_Tree2_GetNext(je);
  }
  return NULL;
}



void *_handlePath(const char *entry, void *data, int idx, GWEN_UNUSED uint32_t flags)
{
  GWEN_JSON_ELEM *jeParent;
  GWEN_JSON_ELEM *je;

  jeParent=(GWEN_JSON_ELEM*) data;

  je=GWEN_JsonElement_FindByIdxTypeAndData(jeParent, GWEN_JSON_ELEMTYPE_KEY, entry, idx);
  if (je==NULL) {
    /* does not exist */
    return NULL;
  }
  je=GWEN_JsonElement_Tree2_GetFirstChild(je);
  return je;
}





