/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Apr 03 2003
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhyfwar/gwenhyfwarapi.h>
#include <gwenhyfwar/misc.h>
#include "stringlist_p.h"
#include "debug.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_STRINGLIST *GWEN_StringList_new(){
  GWEN_STRINGLIST *sl;

  GWEN_NEW_OBJECT(GWEN_STRINGLIST, sl);
  assert(sl);
  return sl;
}



void GWEN_StringList_free(GWEN_STRINGLIST *sl){
  GWEN_STRINGLISTENTRY *curr, *next;

  if (sl) {
    curr=sl->first;
    while(curr) {
      next=curr->next;
      GWEN_StringListEntry_free(curr);
      curr=next;
    } /* while */
    free(sl);
  }
}



GWEN_STRINGLISTENTRY *GWEN_StringListEntry_new(const char *s, int take){
  GWEN_STRINGLISTENTRY *sl;

  GWEN_NEW_OBJECT(GWEN_STRINGLISTENTRY, sl);
  assert(sl);
  if (s) {
    if (take)
      sl->data=s;
    else
      sl->data=strdup(s);
  }
  return sl;
}



void GWEN_StringListEntry_ReplaceString(GWEN_STRINGLISTENTRY *e,
					const char *s,
					int take){
  assert(e);
  if (e->data)
    free((void*)(e->data));
  if (take)
    e->data=s;
  else
    e->data=strdup(s);
}



void GWEN_StringListEntry_free(GWEN_STRINGLISTENTRY *sl){
  if (sl) {
    if (sl->data)
      free((void*)(sl->data));
    free(sl);
  }
}



void GWEN_StringList_AppendEntry(GWEN_STRINGLIST *sl,
				 GWEN_STRINGLISTENTRY *se){
  GWEN_STRINGLISTENTRY *curr;

  assert(sl);
  assert(se);

  curr=sl->first;
  if (!curr) {
    sl->first=se;
  }
  else {
    while(curr->next) {
      curr=curr->next;
    }
    curr->next=se;
  }
  sl->count++;
}



void GWEN_StringList_RemoveEntry(GWEN_STRINGLIST *sl,
				 GWEN_STRINGLISTENTRY *se){
  GWEN_STRINGLISTENTRY *curr;

  assert(sl);
  assert(se);

  curr=sl->first;
  if (curr) {
    if (curr==se) {
      sl->first=curr->next;
      if (sl->count)
	sl->count--;
    }
    else {
      while(curr->next!=se) {
	curr=curr->next;
      }
      if (curr) {
	curr->next=se->next;
        if (sl->count)
	  sl->count--;
      }
    }
  }
}



void GWEN_StringList_Clear(GWEN_STRINGLIST *sl){
  GWEN_STRINGLISTENTRY *se, *next;

  assert(sl);
  se=sl->first;
  sl->first=0;
  while (se) {
    next=se->next;
    GWEN_StringListEntry_free(se);
    se=next;
  } /* while */
}



int GWEN_StringList_AppendString(GWEN_STRINGLIST *sl,
				 const char *s,
				 int take,
				 int checkDouble){
  GWEN_STRINGLISTENTRY *se;

  if (checkDouble) {
    se=sl->first;
    while(se) {
      if (strcmp(se->data, s)==0) {
	if (take)
	  free((char*)s);
        return 0;
      }
      se=se->next;
    } /* while */
  } /* if checkdouble */

  se=GWEN_StringListEntry_new(s, take);
  GWEN_StringList_AppendEntry(sl, se);
  return 1;
}



int GWEN_StringList_InsertString(GWEN_STRINGLIST *sl,
				 const char *s,
				 int take,
				 int checkDouble){
  GWEN_STRINGLISTENTRY *se;

  if (checkDouble) {
    se=sl->first;
    while(se) {
      if (strcmp(se->data, s)==0) {
	if (take)
	  free((char*)s);
	return 0;
      }
      se=se->next;
    } /* while */
  } /* if checkdouble */

  se=GWEN_StringListEntry_new(s, take);
  se->next=sl->first;
  sl->first=se;
  return 1;
}



GWENHYFWAR_API int GWEN_StringList_RemoveString(GWEN_STRINGLIST *sl,
                                                const char *s){
  GWEN_STRINGLISTENTRY *se;

  se=sl->first;
  while(se) {
    if (strcmp(se->data, s)==0) {
      GWEN_StringList_RemoveEntry(sl, se);
      return 1;
    }
    se=se->next;
  } /* while */

  return 0;
}



GWEN_STRINGLISTENTRY *GWEN_StringList_FirstEntry(GWEN_STRINGLIST *sl){
  assert(sl);
  return sl->first;
}



GWEN_STRINGLISTENTRY *GWEN_StringListEntry_Next(GWEN_STRINGLISTENTRY *se){
  assert(se);
  return se->next;
}



const char *GWEN_StringListEntry_Data(GWEN_STRINGLISTENTRY *se){
  assert(se);
  return se->data;
}




