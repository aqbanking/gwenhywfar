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


GWEN_STRINGLIST2 *GWEN_StringList2_new(){
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

  if (m!=GWEN_StringList2_IntertModeAlwaysAdd) {
    GWEN_STRINGLIST2_ITERATOR *it;

    it=GWEN_StringList2__GetString(sl2, s);
    if (it) {
      if (m==GWEN_StringList2_IntertModeNoDouble) {
        if (take)
          free((void*)s);
        GWEN_StringList2Iterator_free(it);
        return 0;
      }
      if (m==GWEN_StringList2_IntertModeReuse) {
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
    GWEN_RefPtr_AddFlags(rp, GWEN_REFPTR_FLAGS_AUTODELETE);
  rp=GWEN_RefPtr_new((void*)s, GWEN_List_GetRefPtrInfo(sl2->listPtr));
  GWEN_List_PushBackRefPtr(sl2->listPtr, rp);
  return 1;
}



int GWEN_StringList2_InsertString(GWEN_STRINGLIST2 *sl2,
                                  const char *s,
                                  int take,
                                  GWEN_STRINGLIST2_INSERTMODE m) {
  assert(sl2);
  assert(s);

  if (m!=GWEN_StringList2_IntertModeAlwaysAdd) {
    GWEN_STRINGLIST2_ITERATOR *it;

    it=GWEN_StringList2__GetString(sl2, s);
    if (it) {
      if (m==GWEN_StringList2_IntertModeNoDouble) {
        if (take)
          free((void*)s);
        GWEN_StringList2Iterator_free(it);
        return 0;
      }
      if (m==GWEN_StringList2_IntertModeReuse) {
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
    GWEN_List_PushFront(sl2->listPtr, (void*)s);
  else
    GWEN_List_PushFront(sl2->listPtr, strdup(s));
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









