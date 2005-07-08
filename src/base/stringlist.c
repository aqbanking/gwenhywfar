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

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/misc.h>
#include "stringlist_p.h"
#include "debug.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_STRINGLIST *GWEN_StringList_new(){
  GWEN_STRINGLIST *sl;

  GWEN_NEW_OBJECT(GWEN_STRINGLIST, sl);
  assert(sl);
  sl->ignoreRefCount=1;
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



void GWEN_StringList_SetSenseCase(GWEN_STRINGLIST *sl, int i) {
  assert(sl);
  sl->senseCase=i;
}



void GWEN_StringList_SetIgnoreRefCount(GWEN_STRINGLIST *sl, int i) {
  assert(sl);
  sl->ignoreRefCount=i;
}



GWEN_STRINGLISTENTRY *GWEN_StringListEntry_new(const char *s, int take){
  GWEN_STRINGLISTENTRY *sl;

  GWEN_NEW_OBJECT(GWEN_STRINGLISTENTRY, sl);
  assert(sl);
  sl->refCount=1;
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
    if (sl->senseCase) {
      while(se) {
	if (strcmp(se->data, s)==0) {
	  if (take)
	    free((char*)s);
	  se->refCount++;
	  return 0;
	}
	se=se->next;
      } /* while */
    }
    else {
      while(se) {
	if (strcasecmp(se->data, s)==0) {
	  if (take)
	    free((char*)s);
	  se->refCount++;
	  return 0;
	}
	se=se->next;
      } /* while */
    }
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
    if (sl->senseCase) {
      while(se) {
	if (strcmp(se->data, s)==0) {
	  if (take)
	    free((char*)s);
	  se->refCount++;
	  return 0;
	}
	se=se->next;
      } /* while */
    }
    else {
      while(se) {
	if (strcasecmp(se->data, s)==0) {
	  if (take)
	    free((char*)s);
	  se->refCount++;
	  return 0;
	}
	se=se->next;
      } /* while */
    }
  } /* if checkdouble */
  se=GWEN_StringListEntry_new(s, take);
  se->next=sl->first;
  sl->first=se;
  return 1;
}



GWENHYWFAR_API int GWEN_StringList_RemoveString(GWEN_STRINGLIST *sl,
                                                const char *s){
  GWEN_STRINGLISTENTRY *se;

  se=sl->first;
  if (sl->senseCase) {
    while(se) {
      if (strcmp(se->data, s)==0) {
	assert(se->refCount);
	se->refCount--;
	if (sl->ignoreRefCount)
	  GWEN_StringList_RemoveEntry(sl, se);
	else {
	  if (se->refCount==0)
	    GWEN_StringList_RemoveEntry(sl, se);
	}
	return 1;
      }
      se=se->next;
    } /* while */
    return 0;
  }
  else {
    while(se) {
      if (strcasecmp(se->data, s)==0) {
	assert(se->refCount);
	se->refCount--;
	if (sl->ignoreRefCount)
	  GWEN_StringList_RemoveEntry(sl, se);
	else {
	  assert(se->refCount);
	  if (se->refCount==0)
	    GWEN_StringList_RemoveEntry(sl, se);
	}
	return 1;
      }
      se=se->next;
    } /* while */
    return 0;
  }
}



GWEN_STRINGLISTENTRY *GWEN_StringList_FirstEntry(const GWEN_STRINGLIST *sl){
  assert(sl);
  return sl->first;
}



GWEN_STRINGLISTENTRY *GWEN_StringListEntry_Next(const GWEN_STRINGLISTENTRY *se){
  assert(se);
  return se->next;
}



const char *GWEN_StringListEntry_Data(const GWEN_STRINGLISTENTRY *se){
  assert(se);
  return se->data;
}


unsigned int GWEN_StringList_Count(const GWEN_STRINGLIST *sl){
  assert(sl);
  return sl->count;
}



int GWEN_StringList_HasString(const GWEN_STRINGLIST *sl,
                              const char *s){
  GWEN_STRINGLISTENTRY *se;

  assert(sl);
  se=sl->first;
  if (sl->senseCase) {
    while(se) {
      if (strcmp(se->data, s)==0) {
	return 1;
      }
      se=se->next;
    } /* while */
    return 0;
  }
  else {
    while(se) {
      if (strcasecmp(se->data, s)==0) {
	return 1;
      }
      se=se->next;
    } /* while */
    return 0;
  }
}



GWEN_STRINGLIST *GWEN_StringList_dup(const GWEN_STRINGLIST *sl){
  GWEN_STRINGLISTENTRY *se;
  GWEN_STRINGLIST *newsl;

  assert(sl);
  newsl=GWEN_StringList_new();

  se=sl->first;
  while(se) {
    GWEN_STRINGLISTENTRY *newse;

    newse=GWEN_StringListEntry_new(se->data, 0);
    GWEN_StringList_AppendEntry(newsl, newse);
    se=se->next;
  } /* while */

  return newsl;
}


void *GWEN_StringList_ForEach(const GWEN_STRINGLIST *l, 
			      void *(*func)(const char *s, void *u), 
			      void *user_data) {
  GWEN_STRINGLISTENTRY *it;
  const char *el;
  void *result = 0;
  assert(l);

  it = GWEN_StringList_FirstEntry(l);
  if (!it)
    return 0;
  while(it) {
    el = GWEN_StringListEntry_Data(it);
    result = func(el, user_data);
    if (result) {
      return result;
    }
    it = GWEN_StringListEntry_Next(it);
  }
  return 0;
}



const char *GWEN_StringList_FirstString(const GWEN_STRINGLIST *l){
  assert(l);
  if (l->first==0)
    return 0;
  return l->first->data;
}



void GWEN_StringList_Sort(GWEN_STRINGLIST *l,
			  int ascending,
			  int senseCase) {
  GWEN_STRINGLISTENTRY **tmpEntries;
  GWEN_STRINGLISTENTRY *se;
  GWEN_STRINGLISTENTRY **pse;

  if (l->count<1)
    return;

  /* sort entries into a linear pointer list */
  tmpEntries=(GWEN_STRINGLISTENTRY **)malloc((l->count+1)*
					     sizeof(GWEN_STRINGLISTENTRY*));
  assert(tmpEntries);
  se=l->first;
  pse=tmpEntries;
  while(se) {
    GWEN_STRINGLISTENTRY *nse;

    *(pse++)=se;
    nse=se->next;
    se->next=0;
    se=nse;
  } /* while */
  *pse=0;

  /* sort */
  if (!ascending && !senseCase) {
    while(1) {
      int exchanged=0;
  
      pse=tmpEntries;
      while(*pse) {
	GWEN_STRINGLISTENTRY *se1, *se2;
	const char *s1, *s2;
  
	se1=pse[0];
	se2=pse[1];
	if (se2==0)
	  break;
	s1=se1->data;
	s2=se2->data;
	if (strcasecmp(s1, s2)<0) {
	  pse[0]=se2;
	  pse[1]=se1;
	  exchanged=1;
	}
	pse++;
      } /* while */
      if (!exchanged)
	break;
    } /* while */
  }
  else if (!ascending && senseCase) {
    while(1) {
      int exchanged=0;
  
      pse=tmpEntries;
      while(*pse) {
	GWEN_STRINGLISTENTRY *se1, *se2;
	const char *s1, *s2;
  
	se1=pse[0];
	se2=pse[1];
	if (se2==0)
	  break;
	s1=se1->data;
	s2=se2->data;
	if (strcmp(s1, s2)<0) {
	  pse[0]=se2;
	  pse[1]=se1;
	  exchanged=1;
	}
	pse++;
      } /* while */
      if (!exchanged)
	break;
    } /* while */
  }
  else if (ascending && !senseCase) {
    while(1) {
      int exchanged=0;
  
      pse=tmpEntries;
      while(*pse) {
	GWEN_STRINGLISTENTRY *se1, *se2;
	const char *s1, *s2;
  
	se1=pse[0];
	se2=pse[1];
	if (se2==0)
	  break;
	s1=se1->data;
	s2=se2->data;
	if (strcasecmp(s1, s2)>0) {
	  pse[0]=se2;
	  pse[1]=se1;
	  exchanged=1;
	}
	pse++;
      } /* while */
      if (!exchanged)
	break;
    } /* while */
  }
  else {
    while(1) {
      int exchanged=0;
  
      pse=tmpEntries;
      while(*pse) {
	GWEN_STRINGLISTENTRY *se1, *se2;
	const char *s1, *s2;
  
	se1=pse[0];
	se2=pse[1];
	if (se2==0)
	  break;
	s1=se1->data;
	s2=se2->data;
	if (strcmp(s1, s2)>0) {
	  pse[0]=se2;
	  pse[1]=se1;
	  exchanged=1;
	}
	pse++;
      } /* while */
      if (!exchanged)
	break;
    } /* while */
  }

  /* sort entries back into GWEN_STRINGLIST */
  pse=tmpEntries;
  se=0;
  while(*pse) {
    (*pse)->next=0;
    if (se)
      se->next=*pse;
    else
      l->first=*pse;
    se=*pse;
    pse++;
  } /* while */

  free(tmpEntries);

}




