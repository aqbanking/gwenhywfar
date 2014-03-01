/***************************************************************************
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

#define DISABLE_DEBUGLOG


#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/misc.h>
#include "stringlist_p.h"
#include "debug.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif



GWEN_STRINGLIST *GWEN_StringList_new(void){
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
    GWEN_FREE_OBJECT(sl);
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
    GWEN_FREE_OBJECT(sl);
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



GWEN_STRINGLIST *GWEN_StringList_fromTabString(const char *s, int checkDup) {
  GWEN_STRINGLIST *sl;

  sl=GWEN_StringList_new();
  if (s && *s) {
    while(*s) {
      const char *t;
      char *tmpStr;
  
      t=strchr(s, '\t');
      if (t) {
	int len;
  
	len=(t-s);
	tmpStr=(char*) malloc(len+1);
	assert(tmpStr);
	memmove(tmpStr, s, len);
	tmpStr[len]=0;
	/* add partial string, take it over */
	GWEN_StringList_AppendString(sl, tmpStr, 1, checkDup);
	s=t+1;
      }
      else {
	/* just add the remaining string (don't take over, copy!) */
	GWEN_StringList_AppendString(sl, s, 0, checkDup);
	break;
      }
    }
  }

  return sl;
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
  sl->count=0;
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
  sl->count++;
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



int GWEN_StringList_GetStringPos(const GWEN_STRINGLIST *sl, const char *s){
  GWEN_STRINGLISTENTRY *se;
  int i;

  assert(sl);
  se=sl->first;
  if (sl->senseCase) {
    i=0;
    while(se) {
      if (strcmp(se->data, s)==0) {
	return i;
      }
      i++;
      se=se->next;
    } /* while */
    return -1;
  }
  else {
    i=0;
    while(se) {
      if (strcasecmp(se->data, s)==0) {
	return i;
      }
      i++;
      se=se->next;
    } /* while */
    return -1;
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



static int GWEN_StringList__compar_asc_nocase(const void *a, const void *b) {
  const GWEN_STRINGLISTENTRY * const * pse1 = a, * const * pse2 = b;
  const GWEN_STRINGLISTENTRY *se1 = *pse1, *se2 = *pse2;
  if (se1 && se2 && se1->data && se2->data)
    return strcmp(se1->data, se2->data);
  else
    return 0;
}
static int GWEN_StringList__compar_desc_nocase(const void *a, const void *b) {
  const GWEN_STRINGLISTENTRY * const * pse1 = a, * const * pse2 = b;
  const GWEN_STRINGLISTENTRY *se1 = *pse1, *se2 = *pse2;
  if (se1 && se2 && se1->data && se2->data)
    return strcmp(se2->data, se1->data);
  else
    return 0;
}
static int GWEN_StringList__compar_asc_case(const void *a, const void *b) {
  const GWEN_STRINGLISTENTRY * const * pse1 = a, * const * pse2 = b;
  const GWEN_STRINGLISTENTRY *se1 = *pse1, *se2 = *pse2;
  if (se1 && se2 && se1->data && se2->data)
    return strcasecmp(se1->data, se2->data);
  else
    return 0;
}
static int GWEN_StringList__compar_desc_case(const void *a, const void *b) {
  const GWEN_STRINGLISTENTRY * const * pse1 = a, * const * pse2 = b;
  const GWEN_STRINGLISTENTRY *se1 = *pse1, *se2 = *pse2;
  if (se1 && se2 && se1->data && se2->data)
    return strcasecmp(se2->data, se1->data);
  else
    return 0;
}

static int GWEN_StringList__compar_asc_int(const void *a, const void *b) {
  const GWEN_STRINGLISTENTRY * const * pse1 = a, * const * pse2 = b;
  const GWEN_STRINGLISTENTRY *se1 = *pse1, *se2 = *pse2;
  if (se1 && se2 && se1->data && se2->data) {
    int i1, i2;

    i1=atoi(se1->data);
    i2=atoi(se2->data);
    return (i1>i2) - (i1<i2);
  }
  else
    return 0;
}

static int GWEN_StringList__compar_desc_int(const void *a, const void *b) {
  const GWEN_STRINGLISTENTRY * const * pse1 = a, * const * pse2 = b;
  const GWEN_STRINGLISTENTRY *se1 = *pse1, *se2 = *pse2;
  if (se1 && se2 && se1->data && se2->data) {
    int i1, i2;

    i1=atoi(se1->data);
    i2=atoi(se2->data);
    return (i2>i1) - (i2<i1);
  }
  else
    return 0;
}



void GWEN_StringList_Sort(GWEN_STRINGLIST *l,
			  int ascending,
                          GWEN_STRINGLIST_SORT_MODE sortMode) {
  GWEN_STRINGLISTENTRY **tmpEntries;
  GWEN_STRINGLISTENTRY *sentry;
  GWEN_STRINGLISTENTRY **psentry;

  if (l->count<2)
    return;

  /* sort entries into a linear pointer list */
  tmpEntries=(GWEN_STRINGLISTENTRY **)malloc((l->count+1)*
					    sizeof(GWEN_STRINGLISTENTRY*));
  assert(tmpEntries);
  sentry=l->first;
  psentry=tmpEntries;
  while(sentry) {
    *(psentry++)=sentry;
    sentry=sentry->next;
  } /* while */
  *psentry=0;

  /* sort */
  switch(sortMode) {
  case GWEN_StringList_SortModeNoCase:
    if (ascending)
      qsort(tmpEntries, l->count, sizeof(GWEN_STRINGLISTENTRY*),
            GWEN_StringList__compar_asc_nocase);
    else
      qsort(tmpEntries, l->count, sizeof(GWEN_STRINGLISTENTRY*),
            GWEN_StringList__compar_desc_nocase);
    break;

  case GWEN_StringList_SortModeCase:
    if (ascending)
      qsort(tmpEntries, l->count, sizeof(GWEN_STRINGLISTENTRY*),
            GWEN_StringList__compar_asc_case);
    else
      qsort(tmpEntries, l->count, sizeof(GWEN_STRINGLISTENTRY*),
            GWEN_StringList__compar_desc_case);
    break;

  case GWEN_StringList_SortModeInt:
    if (ascending)
      qsort(tmpEntries, l->count, sizeof(GWEN_STRINGLISTENTRY*),
            GWEN_StringList__compar_asc_int);
    else
      qsort(tmpEntries, l->count, sizeof(GWEN_STRINGLISTENTRY*),
            GWEN_StringList__compar_desc_int);
    break;

  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown sortmode %d", sortMode);
  }

  /* sort entries back into GWEN_STRINGLIST */
  psentry=tmpEntries;
  sentry=0;
  while(*psentry) {
    if (sentry)
      sentry->next=*psentry;
    else
      l->first=*psentry;
    sentry=*psentry;
    psentry++;
  } /* while */
  sentry->next=NULL;

  free(tmpEntries);

}



const char *GWEN_StringList_StringAt(const GWEN_STRINGLIST *sl, int idx) {
  GWEN_STRINGLISTENTRY *se;

  assert(sl);
  se=sl->first;
  while(se) {
    if (idx--==0)
      return se->data;
    se=se->next;
  } /* while */
  return 0;
}



GWEN_STRINGLIST *GWEN_StringList_fromString(const char *str, const char *delimiters, int checkDouble) {
  if (str && *str) {
    GWEN_STRINGLIST *sl;
    const unsigned char *s;
  
    sl=GWEN_StringList_new();
    s=(const unsigned char*)str;
  
    while(*s) {
      /* skip blanks */
      while(*s && *s<33)
	s++;

      if (*s) {
	const unsigned char *pStart;
	int len;

	/* read word */
	pStart=s;
        //s++;
	while(*s && strchr(delimiters, *s)==NULL)
	  s++;
	len=s-pStart;
    
	if (len) {
	  char *toAdd;

	  toAdd=(char*) malloc(len+1);
	  assert(toAdd);

	  memmove(toAdd, pStart, len);
	  toAdd[len]=0;
    
	  GWEN_StringList_AppendString(sl, toAdd, 1, checkDouble);
	}
      }

      if (*s==0)
	break;
      s++;
    }
  
    if (GWEN_StringList_Count(sl)==0) {
      GWEN_StringList_free(sl);
      return NULL;
    }
    return sl;
  }
  else
    return NULL;
}



