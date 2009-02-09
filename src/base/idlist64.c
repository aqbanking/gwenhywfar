/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: idlist64.c 1102 2006-12-30 19:39:37Z martin $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
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


#include "idlist64_p.h"
#include <gwenhywfar/debug.h>


#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(GWEN_IDTABLE64, GWEN_IdTable64)
/* No trailing semicolon here because this is a macro call */



GWEN_IDTABLE64 *GWEN_IdTable64_new(){
  GWEN_IDTABLE64 *idt;

  GWEN_NEW_OBJECT(GWEN_IDTABLE64, idt);
  idt->refCount=1;
  GWEN_LIST_INIT(GWEN_IDTABLE64, idt);

  idt->freeEntries=GWEN_IDTABLE64_MAXENTRIES;
  return idt;
}



void GWEN_IdTable64_free(GWEN_IDTABLE64 *idt){
  if (idt) {
    assert(idt->refCount);
    if (--(idt->refCount)==0) {
      GWEN_LIST_FINI(GWEN_IDTABLE64, idt);
      GWEN_FREE_OBJECT(idt);
    }
  }
}



void GWEN_IdTable64_Attach(GWEN_IDTABLE64 *idt){
  assert(idt);
  assert(idt->refCount);
  idt->refCount++;
}



static inline int GWEN_IdTable64_AddId(GWEN_IDTABLE64 *idt, uint64_t id){
  unsigned int i;

  for (i=0; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
    if (idt->entries[i]==0) {
      idt->entries[i]=id;
      idt->freeEntries--;
      return 0;
    }
  } /* for */
  return -1;
}



static inline int GWEN_IdTable64_HasId(const GWEN_IDTABLE64 *idt, uint64_t id){
  unsigned int i;

  for (i=0; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
    if (idt->entries[i]==id) {
      return 1;
    }
  } /* for */
  return 0;
}



static inline int GWEN_IdTable64_DelId(GWEN_IDTABLE64 *idt, uint64_t id){
  unsigned int i;

  for (i=0; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
    if (idt->entries[i]==id) {
      idt->entries[i]=0;
      idt->freeEntries++;
      return 0;
    }
  } /* for */
  return -1;
}



static inline int GWEN_IdTable64_IsEmpty(const GWEN_IDTABLE64 *idt){
  return GWEN_IDTABLE64_MAXENTRIES==idt->freeEntries;
}



static inline int GWEN_IdTable64_IsFull(const GWEN_IDTABLE64 *idt){
  return idt->freeEntries==0;
}



static inline unsigned int GWEN_IdTable64_GetCount(const GWEN_IDTABLE64 *idt){
  return GWEN_IDTABLE64_MAXENTRIES-idt->freeEntries;
}



static inline uint64_t GWEN_IdTable64_GetFirstId(GWEN_IDTABLE64 *idt){
  unsigned int i;

  assert(idt);
  idt->current=0;
  for (i=0; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
    if (idt->entries[i]!=0) {
      idt->current=i;
      return idt->entries[i];
    }
  } /* for */
  return 0;
}



static inline uint64_t GWEN_IdTable64_GetNextId(GWEN_IDTABLE64 *idt){
  unsigned int i;

  for (i=idt->current+1; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
    if (idt->entries[i]!=0) {
      idt->current=i;
      return idt->entries[i];
    }
  } /* for */
  idt->current=GWEN_IDTABLE64_MAXENTRIES;
  return 0;
}



static inline uint64_t GWEN_IdTable64_GetFirstId2(const GWEN_IDTABLE64 *idt,
                                                  uint64_t *tabIdx){
  unsigned int i;

  for (i=0; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
    if (idt->entries[i]!=0) {
      *tabIdx=i;
      return idt->entries[i];
    }
  } /* for */
  return 0;
}



static inline uint64_t GWEN_IdTable64_GetNextId2(const GWEN_IDTABLE64 *idt,
                                                 uint64_t *tabIdx){
  unsigned int i;

  for (i=(*tabIdx)+1; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
    if (idt->entries[i]!=0) {
      *tabIdx=i;
      return idt->entries[i];
    }
  } /* for */
  return 0;
}






GWEN_IDLIST64 *GWEN_IdList64_new(){
  GWEN_IDLIST64 *idl;

  GWEN_NEW_OBJECT(GWEN_IDLIST64, idl);
  idl->refCount=1;
  idl->idTables=GWEN_IdTable64_List_new();
  return idl;
}



void GWEN_IdList64_Attach(GWEN_IDLIST64 *idl) {
  assert(idl);
  assert(idl->refCount);
  idl->refCount++;
}



void GWEN_IdList64_free(GWEN_IDLIST64 *idl){
  if (idl) {
    assert(idl->refCount);
    if (--(idl->refCount)==0) {
      GWEN_IdTable64_List_free(idl->idTables);
      GWEN_FREE_OBJECT(idl);
    }
  }
}



int GWEN_IdList64_AddId(GWEN_IDLIST64 *idl, uint64_t id){
  GWEN_IDTABLE64 *idt;

  assert(idl);

  idl->current=0;
  idt=GWEN_IdTable64_List_First(idl->idTables);
  /* find free table */
  while(idt) {
    if (!GWEN_IdTable64_IsFull(idt))
      break;
    idt=GWEN_IdTable64_List_Next(idt);
  } /* while */

  if (!idt) {
    idt=GWEN_IdTable64_new();
    GWEN_IdTable64_List_Add(idt, idl->idTables);;
  }

  GWEN_IdTable64_AddId(idt, id);
  idl->entryCount++;
  return 0;
}



int GWEN_IdList64_DelId(GWEN_IDLIST64 *idl, uint64_t id){
  GWEN_IDTABLE64 *idt;

  assert(idl);

  idl->current=0;
  idt=GWEN_IdTable64_List_First(idl->idTables);
  /* find table */
  while(idt) {
    if (!GWEN_IdTable64_DelId(idt, id)) {
      /* found a table which had this id */
      GWEN_IdList64_Clean(idl);
      idl->entryCount--;
      return 0;
    }
    idt=GWEN_IdTable64_List_Next(idt);
  } /* while */
  return -1;
}



int GWEN_IdList64_HasId(const GWEN_IDLIST64 *idl, uint64_t id){
  GWEN_IDTABLE64 *idt;

  assert(idl);

  idt=GWEN_IdTable64_List_First(idl->idTables);
  /* find free table */
  while(idt) {
    if (GWEN_IdTable64_HasId(idt, id))
      return 1;
    idt=GWEN_IdTable64_List_Next(idt);
  } /* while */
  return 0;
}



void GWEN_IdList64_Clean(GWEN_IDLIST64 *idl) {
  GWEN_IDTABLE64 *idt;

  assert(idl);
  idl->current=0;
  idt=GWEN_IdTable64_List_First(idl->idTables);
  /* free empty tables */
  while(idt) {
    GWEN_IDTABLE64 *next;

    next=GWEN_IdTable64_List_Next(idt);
    if (GWEN_IdTable64_IsEmpty(idt)) {
      /*GWEN_IdTable64_List_Del(idt);*/
      GWEN_IdTable64_free(idt);
    }
    idt=next;
  } /* while */
}



uint64_t GWEN_IdList64_GetFirstId(GWEN_IDLIST64 *idl){
  GWEN_IDTABLE64 *idt;

  assert(idl);

  idt=GWEN_IdTable64_List_First(idl->idTables);
  /* find table which contains the first id */
  while(idt) {
    GWEN_IDTABLE64 *next;
    uint64_t id;

    next=GWEN_IdTable64_List_Next(idt);
    id=GWEN_IdTable64_GetFirstId(idt);
    if (id) {
      idl->current=idt;
      return id;
    }
    idt=next;
  } /* while */

  return 0;
}



uint64_t GWEN_IdList64_GetNextId(GWEN_IDLIST64 *idl){
  GWEN_IDTABLE64 *idt;
  uint64_t id;

  assert(idl);

  idt=idl->current;
  if (idt) {
    id=GWEN_IdTable64_GetNextId(idt);
    if (id) {
      idl->current=idt;
      return id;
    }
  }
  else {
    idl->current=0;
    return 0;
  }

  idt=GWEN_IdTable64_List_Next(idt);
  while (idt) {
    id=GWEN_IdTable64_GetFirstId(idt);
    if (id) {
      idl->current=idt;
      return id;
    }
    idt=GWEN_IdTable64_List_Next(idt);
  } /* while */

  idl->current=0;
  return 0;
}



int GWEN_IdList64_Sort(GWEN_IDLIST64 *idl){
  GWEN_IDLIST64_ITERATOR *it;
  GWEN_IDTABLE64 *idt;
  unsigned int cnt;
  uint64_t *ptr;
  unsigned int i;

  assert(idl);

  /* count ids */
  idt=GWEN_IdTable64_List_First(idl->idTables);
  cnt=0;
  while(idt) {
    GWEN_IDTABLE64 *next;

    next=GWEN_IdTable64_List_Next(idt);
    cnt+=GWEN_IdTable64_GetCount(idt);
    idt=next;
  } /* while */

  if (!cnt)
    return 0;

  /* move ids to a temporary list */
  ptr=(uint64_t*)malloc(sizeof(uint64_t)*cnt);
  assert(ptr);

  it=GWEN_IdList64_Iterator_new(idl);
  for (i=0; i<cnt; i++) {
    uint64_t id;

    if (i==0)
      id=GWEN_IdList64_Iterator_GetFirstId(it);
    else
      id=GWEN_IdList64_Iterator_GetNextId(it);
    assert(id);
    ptr[i]=id;
  } /* for */
  GWEN_IdList64_Iterator_free(it);

  /* remove all tables (we will add sorted tables later) */
  GWEN_IdTable64_List_Clear(idl->idTables);
  idl->current=0;

  /* sort temporary list */
  while(1) {
    int rpl;

    rpl=0;
    for (i=0; i<(cnt-1); i++) {
      if (ptr[i]>ptr[i+1]) {
        uint64_t id;

        id=ptr[i];
        ptr[i]=ptr[i+1];
        ptr[i+1]=id;
        rpl=1;
      }
    } /* for */
    if (!rpl)
      break;
  } /* while */

  /* move back sorted list of ids from temporary list */
  for (i=0; i<cnt; i++) {
    GWEN_IdList64_AddId(idl, ptr[i]);
  }
  free(ptr);

  return 0;
}



void GWEN_IdList64_Clear(GWEN_IDLIST64 *idl){
  assert(idl);
  GWEN_IdTable64_List_Clear(idl->idTables);
  idl->entryCount=0;
  idl->current=0;
}



GWEN_IDLIST64 *GWEN_IdList64_dup(const GWEN_IDLIST64 *idl){
  GWEN_IDLIST64 *nidl;
  GWEN_IDTABLE64 *idt;

  assert(idl);
  nidl=GWEN_IdList64_new();

  idt=GWEN_IdTable64_List_First(idl->idTables);
  while(idt) {
    if (idt->freeEntries!=GWEN_IDTABLE64_MAXENTRIES){
      int i;

      for (i=0; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
	if (idt->entries[i]!=0)
	  GWEN_IdList64_AddId(nidl, idt->entries[i]);
      }
    }
    idt=GWEN_IdTable64_List_Next(idt);
  }

  return nidl;
}



uint64_t GWEN_IdList64_GetFirstId2(const GWEN_IDLIST64 *idl,
                                         uint64_t *pos){
  GWEN_IDTABLE64 *idt;
  int tabNum=0;

  assert(idl);

  idt=GWEN_IdTable64_List_First(idl->idTables);
  /* find free table */
  while(idt) {
    GWEN_IDTABLE64 *next;
    uint64_t id;
    uint64_t tabIdx;

    next=GWEN_IdTable64_List_Next(idt);
    id=GWEN_IdTable64_GetFirstId2(idt, &tabIdx);
    if (id) {
      *pos=(tabNum*GWEN_IDTABLE64_MAXENTRIES)+tabIdx;
      return id;
    }
    tabNum++;
    idt=next;
  } /* while */

  return 0;
}



uint64_t GWEN_IdList64_GetNextId2(const GWEN_IDLIST64 *idl,
				  uint64_t *pos){
  GWEN_IDTABLE64 *idt;
  int i;
  int tabNum;
  uint64_t tabIdx;

  assert(idl);
  tabNum=(*pos)/GWEN_IDTABLE64_MAXENTRIES;
  tabIdx=(*pos)%GWEN_IDTABLE64_MAXENTRIES;

  /* seek table */
  i=tabNum;
  idt=GWEN_IdTable64_List_First(idl->idTables);
  while(i--) idt=GWEN_IdTable64_List_Next(idt);
  assert(idt);

  while(idt) {
    GWEN_IDTABLE64 *next;
    uint64_t id;

    next=GWEN_IdTable64_List_Next(idt);
    id=GWEN_IdTable64_GetNextId2(idt, &tabIdx);
    if (id) {
      *pos=(tabNum*GWEN_IDTABLE64_MAXENTRIES)+tabIdx;
      return id;
    }
    tabNum++;
    idt=next;
  } /* while */

  return 0;
}



GWEN_IDLIST64_ITERATOR *GWEN_IdList64_Iterator_new(GWEN_IDLIST64 *idl) {
  GWEN_IDLIST64_ITERATOR *it;

  assert(idl);
  GWEN_NEW_OBJECT(GWEN_IDLIST64_ITERATOR, it);

  GWEN_IdList64_Attach(idl);
  it->list=idl;

  return it;
}



void GWEN_IdList64_Iterator_free(GWEN_IDLIST64_ITERATOR *it) {
  if (it) {
    if (it->currentTable)
      GWEN_IdTable64_free(it->currentTable);
    GWEN_IdList64_free(it->list);
    GWEN_FREE_OBJECT(it);
  }
}



uint64_t GWEN_IdList64_Iterator_GetFirstId(GWEN_IDLIST64_ITERATOR *it) {
  GWEN_IDTABLE64 *idt;

  assert(it);

  idt=GWEN_IdTable64_List_First(it->list->idTables);
  /* find table which contains the first id */
  while(idt) {
    GWEN_IDTABLE64 *next;
    unsigned int i;

    next=GWEN_IdTable64_List_Next(idt);

    for (i=0; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
      if (idt->entries[i]!=0) {
        /* attach to new table */
	GWEN_IdTable64_Attach(idt);
	/* detach from previous table */
	GWEN_IdTable64_free(it->currentTable);
        /* store current table and index */
	it->currentTable=idt;
	it->currentIndex=i;
	/* return id */
	return idt->entries[i];
      }
    } /* for */

    idt=next;
  } /* while */

  GWEN_IdTable64_free(it->currentTable);
  it->currentTable=NULL;
  it->currentIndex=0;

  return 0;
}



uint64_t GWEN_IdList64_Iterator_GetNextId(GWEN_IDLIST64_ITERATOR *it) {
  GWEN_IDTABLE64 *idt;
  uint32_t startIdx;

  assert(it);

  idt=it->currentTable;
  startIdx=it->currentIndex+1;

  /* find table which contains the next id */
  while(idt) {
    GWEN_IDTABLE64 *next;
    unsigned int i;

    next=GWEN_IdTable64_List_Next(idt);

    for (i=startIdx; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
      if (idt->entries[i]!=0) {
        /* attach to new table */
	GWEN_IdTable64_Attach(idt);
	/* detach from previous table */
	GWEN_IdTable64_free(it->currentTable);
        /* store current table and index */
	it->currentTable=idt;
	it->currentIndex=i;
	/* return id */
	return idt->entries[i];
      }
    } /* for */

    /* reset start index to start at 0 with next table */
    startIdx=0;
    idt=next;
  } /* while */

  GWEN_IdTable64_free(it->currentTable);
  it->currentTable=NULL;
  it->currentIndex=0;

  return 0;
}














