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
#include "stringlist2_p.h"
#include "debug.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif


GWEN_STRINGLIST2 *GWEN_StringList2_new(void){
  GWEN_STRINGLIST2 *sl2;
  GWEN_REFPTR_INFO *rpi;

  GWEN_NEW_OBJECT(GWEN_STRINGLIST2, sl2);
  rpi=GWEN_RefPtrInfo_new();
  GWEN_RefPtrInfo_SetFreeFn(rpi,
                            (GWEN_REFPTR_INFO_FREE_FN)free);
  sl2->listPtr=GWEN_List_new();
  GWEN_List_SetRefPtrInfo(sl2->listPtr, rpi);
  GWEN_RefPtrInfo_free(rpi);

  return sl2;
}



void GWEN_StringList2_free(GWEN_STRINGLIST2 *sl2){
  if (sl2) {
    GWEN_List_free(sl2->listPtr);
    GWEN_FREE_OBJECT(sl2);
  }
}



GWEN_STRINGLIST2 *GWEN_StringList2_dup(GWEN_STRINGLIST2 *sl2){
  GWEN_STRINGLIST2 *nsl2;

  GWEN_NEW_OBJECT(GWEN_STRINGLIST2, nsl2);
  nsl2->listPtr=GWEN_List_dup(sl2->listPtr);
  nsl2->senseCase=sl2->senseCase;

  return nsl2;
}



int GWEN_StringList2_toDb(GWEN_STRINGLIST2 *sl2, GWEN_DB_NODE *db, const char *name) {
  GWEN_DB_DeleteVar(db, name);

  if (sl2) {
    GWEN_STRINGLIST2_ITERATOR *it;

    it=GWEN_StringList2_First(sl2);
    if (it) {
      const char *s;

      s=GWEN_StringList2Iterator_Data(it);
      while(s) {
	int rv;

	rv=GWEN_DB_SetCharValue(db, 0, name, s);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  return rv;
	}

	s=GWEN_StringList2Iterator_Next(it);
      }
      GWEN_StringList2Iterator_free(it);
    }
  }

  return 0;
}



GWEN_STRINGLIST2 *GWEN_StringList2_fromDb(GWEN_DB_NODE *db, const char *name, GWEN_STRINGLIST2_INSERTMODE m) {
  GWEN_STRINGLIST2 *sl2;
  int i;

  sl2=GWEN_StringList2_new();
  for (i=0; ; i++) {
    const char *s;

    s=GWEN_DB_GetCharValue(db, name, i, NULL);
    if (!s)
      break;
    GWEN_StringList2_AppendString(sl2, s, 0, m);
  }

  return sl2;
}



int GWEN_StringList2_toXml(GWEN_STRINGLIST2 *sl2, GWEN_XMLNODE *node) {
  GWEN_STRINGLIST2_ITERATOR *it;
  
  it=GWEN_StringList2_First(sl2);
  if (it) {
    const char *s;
    
    s=GWEN_StringList2Iterator_Data(it);
    while(s) {
      GWEN_XMLNode_SetCharValue(node, "elem", s);
      s=GWEN_StringList2Iterator_Next(it);
    }
    GWEN_StringList2Iterator_free(it);
  }

  return 0;
}



GWEN_STRINGLIST2 *GWEN_StringList2_fromXml(GWEN_XMLNODE *node, GWEN_STRINGLIST2_INSERTMODE m) {
  GWEN_STRINGLIST2 *sl2;
  GWEN_XMLNODE *n;

  sl2=GWEN_StringList2_new();


  n=GWEN_XMLNode_GetFirstTag(node);
  while(n) {
    GWEN_XMLNODE *dn;

    dn=GWEN_XMLNode_GetFirstData(n);
    if (dn) {
      const char *s;

      s=GWEN_XMLNode_GetData(dn);
      if (s) {
	GWEN_StringList2_AppendString(sl2, s, 0, m);
      }
    }
    n=GWEN_XMLNode_GetNextTag(n);
  }

  return sl2;
}



void GWEN_StringList2_SetSenseCase(GWEN_STRINGLIST2 *sl2, int i){
  assert(sl2);
  sl2->senseCase=i;
}



int GWEN_StringList2_AppendString(GWEN_STRINGLIST2 *sl2,
                                  const char *s,
                                  int take,
                                  GWEN_STRINGLIST2_INSERTMODE m) {
  GWEN_REFPTR *rp;

  assert(sl2);
  assert(s);

  if (m!=GWEN_StringList2_IntertMode_AlwaysAdd) {
    GWEN_STRINGLIST2_ITERATOR *it;

    it=GWEN_StringList2__GetString(sl2, s);
    if (it) {
      if (m==GWEN_StringList2_IntertMode_NoDouble) {
        if (take)
          free((void*)s);
        GWEN_StringList2Iterator_free(it);
        return 0;
      }
      if (m==GWEN_StringList2_IntertMode_Reuse) {
        GWEN_ListIterator_IncLinkCount((GWEN_LIST_ITERATOR*)it);
        if (take)
          free((void*)s);
        GWEN_StringList2Iterator_free(it);
        return 0;
      }
      GWEN_StringList2Iterator_free(it);
    }
  }

  if (take)
    rp=GWEN_RefPtr_new((void*)s, GWEN_List_GetRefPtrInfo(sl2->listPtr));
  else
    rp=GWEN_RefPtr_new(strdup(s), GWEN_List_GetRefPtrInfo(sl2->listPtr));
  GWEN_RefPtr_AddFlags(rp, GWEN_REFPTR_FLAGS_AUTODELETE);
  GWEN_List_PushBackRefPtr(sl2->listPtr, rp);
  return 1;
}



int GWEN_StringList2_InsertString(GWEN_STRINGLIST2 *sl2,
                                  const char *s,
                                  int take,
                                  GWEN_STRINGLIST2_INSERTMODE m) {
  GWEN_REFPTR *rp;

  assert(sl2);
  assert(s);

  if (m!=GWEN_StringList2_IntertMode_AlwaysAdd) {
    GWEN_STRINGLIST2_ITERATOR *it;

    it=GWEN_StringList2__GetString(sl2, s);
    if (it) {
      if (m==GWEN_StringList2_IntertMode_NoDouble) {
        if (take)
          free((void*)s);
        GWEN_StringList2Iterator_free(it);
        return 0;
      }
      if (m==GWEN_StringList2_IntertMode_Reuse) {
        GWEN_ListIterator_IncLinkCount((GWEN_LIST_ITERATOR*)it);
        if (take)
          free((void*)s);
        GWEN_StringList2Iterator_free(it);
        return 0;
      }
      GWEN_StringList2Iterator_free(it);
    }
  }

  if (take)
    rp=GWEN_RefPtr_new((void*)s, GWEN_List_GetRefPtrInfo(sl2->listPtr));
  else
    rp=GWEN_RefPtr_new(strdup(s), GWEN_List_GetRefPtrInfo(sl2->listPtr));
  GWEN_RefPtr_AddFlags(rp, GWEN_REFPTR_FLAGS_AUTODELETE);
  GWEN_List_PushFrontRefPtr(sl2->listPtr, rp);
  return 1;
}



int GWEN_StringList2_RemoveString(GWEN_STRINGLIST2 *sl2,
                                  const char *s){
  GWEN_STRINGLIST2_ITERATOR *it;

  it=GWEN_StringList2__GetString(sl2, s);
  if (it) {
    int lc;

    lc=GWEN_ListIterator_GetLinkCount(it);
    GWEN_List_Erase(sl2->listPtr, it);
    GWEN_StringList2Iterator_free(it);
    if (lc<2)
      return 1;
  }

  return 0;
}



int GWEN_StringList2_HasString(const GWEN_STRINGLIST2 *sl2,
                               const char *s){
  GWEN_STRINGLIST2_ITERATOR *it;
  int gotIt;

  it=GWEN_StringList2_First(sl2);
  gotIt=0;
  if (it) {
    const char *t;

    t=GWEN_StringList2Iterator_Data(it);
    if (sl2->senseCase) {
      while(t) {
        if (strcmp(s, t)) {
          gotIt=1;
          break;
        }
        t=GWEN_StringList2Iterator_Next(it);
      }
    }
    else {
      while(t) {
        if (strcasecmp(s, t)) {
          gotIt=1;
          break;
        }
        t=GWEN_StringList2Iterator_Next(it);
      }
    }
    GWEN_StringList2Iterator_free(it);
  }

  return gotIt;
}



GWEN_STRINGLIST2_ITERATOR*
GWEN_StringList2__GetString(const GWEN_STRINGLIST2 *sl2,
                            const char *s){
  GWEN_STRINGLIST2_ITERATOR *it;
  GWEN_REFPTR *rp;

  it=GWEN_StringList2_First(sl2);
  if (it) {
    rp=GWEN_ListIterator_DataRefPtr((GWEN_LIST_ITERATOR*)it);

    if (sl2->senseCase) {
      while(rp) {
        const char *t;

        t=(const char*)GWEN_RefPtr_GetData(rp);
        assert(t);
        if (strcmp(s, t)==0)
          return it;
        rp=GWEN_ListIterator_NextRefPtr((GWEN_LIST_ITERATOR*)it);
      }
    }
    else {
      while(rp) {
        const char *t;

        t=(const char*)GWEN_RefPtr_GetData(rp);
        assert(t);
        if (strcasecmp(s, t)==0)
          return it;
        rp=GWEN_ListIterator_NextRefPtr((GWEN_LIST_ITERATOR*)it);
      }
    }
    GWEN_StringList2Iterator_free(it);
  }

  return 0;
}



const char *GWEN_StringList2_GetStringAt(const GWEN_STRINGLIST2 *sl2, int idx) {
  GWEN_STRINGLIST2_ITERATOR *it;
  GWEN_REFPTR *rp;

  it=GWEN_StringList2_First(sl2);
  if (it) {
    rp=GWEN_ListIterator_DataRefPtr((GWEN_LIST_ITERATOR*)it);

    while(rp) {
      const char *t;

      t=(const char*)GWEN_RefPtr_GetData(rp);
      assert(t);
      if (idx--==0) {
	GWEN_StringList2Iterator_free(it);
	return t;
      }
      rp=GWEN_ListIterator_NextRefPtr((GWEN_LIST_ITERATOR*)it);
    }
    GWEN_StringList2Iterator_free(it);
  }

  return NULL;
}













GWEN_STRINGLIST2_ITERATOR *GWEN_StringList2_First(const GWEN_STRINGLIST2 *l) {
  assert(l);
  return (GWEN_STRINGLIST2_ITERATOR*) GWEN_List_First(l->listPtr);
}



GWEN_STRINGLIST2_ITERATOR *GWEN_StringList2_Last(const GWEN_STRINGLIST2 *l) {
  assert(l);
  return (GWEN_STRINGLIST2_ITERATOR*) GWEN_List_Last(l->listPtr);
}



void GWEN_StringList2Iterator_free(GWEN_STRINGLIST2_ITERATOR *li) {
  assert(li);
  GWEN_ListIterator_free((GWEN_LIST_ITERATOR*)li);
}


const char *GWEN_StringList2Iterator_Previous(GWEN_STRINGLIST2_ITERATOR *li) {
  assert(li);
  return (const char*) GWEN_ListIterator_Previous((GWEN_LIST_ITERATOR*)li);
}


const char *GWEN_StringList2Iterator_Next(GWEN_STRINGLIST2_ITERATOR *li) {
  assert(li);
  return (const char*) GWEN_ListIterator_Next((GWEN_LIST_ITERATOR*)li);
}


const char *GWEN_StringList2Iterator_Data(GWEN_STRINGLIST2_ITERATOR *li) {
  assert(li);
  return (const char*) GWEN_ListIterator_Data((GWEN_LIST_ITERATOR*)li);
}



GWEN_REFPTR*
GWEN_StringList2Iterator_DataRefPtr(GWEN_STRINGLIST2_ITERATOR *li) {
  assert(li);
  return (GWEN_REFPTR*) GWEN_ListIterator_DataRefPtr((GWEN_LIST_ITERATOR*)li);
}



unsigned int
GWEN_StringList2Iterator_GetLinkCount(const GWEN_STRINGLIST2_ITERATOR *li){
  assert(li);
  return GWEN_ListIterator_GetLinkCount((const GWEN_LIST_ITERATOR*)li);
}



unsigned int GWEN_StringList2_GetCount(const GWEN_STRINGLIST2 *l) {
  assert(l);
  return GWEN_List_GetSize(l->listPtr);
}



void GWEN_StringList2_Dump(const GWEN_STRINGLIST2 *sl2){
  GWEN_STRINGLIST2_ITERATOR *it;

  it=GWEN_StringList2_First(sl2);
  if (it) {
    const char *t;
    int i;

    t=GWEN_StringList2Iterator_Data(it);
    i=0;
    while(t) {
      fprintf(stderr, "String %d: \"%s\" [%d]\n", i, t,
              GWEN_StringList2Iterator_GetLinkCount(it));
      t=GWEN_StringList2Iterator_Next(it);
    }
    GWEN_StringList2Iterator_free(it);
  }
  else {
    fprintf(stderr, "Empty string list.\n");
  }
}









