/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
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


#include "idlist_p.h"
#include <gwenhywfar/debug.h>


#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(GWEN_IDTABLE, GWEN_IdTable)
/* No trailing semicolon here because this is a macro call */



GWEN_IDTABLE *GWEN_IdTable_new(){
  GWEN_IDTABLE *idt;

  GWEN_NEW_OBJECT(GWEN_IDTABLE, idt);
  GWEN_LIST_INIT(GWEN_IDTABLE, idt);

  idt->freeEntries=GWEN_IDTABLE_MAXENTRIES;
  return idt;
}



void GWEN_IdTable_free(GWEN_IDTABLE *idt){
  if (idt) {
    GWEN_LIST_FINI(GWEN_IDTABLE, idt);
    GWEN_FREE_OBJECT(idt);
  }
}



int GWEN_IdTable_AddId(GWEN_IDTABLE *idt, GWEN_TYPE_UINT32 id){
  unsigned int i;

  assert(idt);
  assert(id);

  for (i=0; i<GWEN_IDTABLE_MAXENTRIES; i++) {
    if (idt->entries[i]==0) {
      idt->entries[i]=id;
      idt->freeEntries--;
      return 0;
    }
  } /* for */
  return 0;
}



int GWEN_IdTable_HasId(const GWEN_IDTABLE *idt, GWEN_TYPE_UINT32 id){
  unsigned int i;

  assert(idt);
  assert(id);

  for (i=0; i<GWEN_IDTABLE_MAXENTRIES; i++) {
    if (idt->entries[i]==id) {
      return 1;
    }
  } /* for */
  return 0;
}



int GWEN_IdTable_DelId(GWEN_IDTABLE *idt, GWEN_TYPE_UINT32 id){
  unsigned int i;

  assert(idt);
  assert(id);

  for (i=0; i<GWEN_IDTABLE_MAXENTRIES; i++) {
    if (idt->entries[i]==id) {
      idt->entries[i]=0;
      idt->freeEntries++;
      return 0;
    }
  } /* for */
  return -1;
}



int GWEN_IdTable_IsEmpty(const GWEN_IDTABLE *idt){
  assert(idt);
  return GWEN_IDTABLE_MAXENTRIES==idt->freeEntries;
}



int GWEN_IdTable_IsFull(const GWEN_IDTABLE *idt){
  assert(idt);
  return idt->freeEntries==0;
}



unsigned int GWEN_IdTable_GetCount(const GWEN_IDTABLE *idt){
  assert(idt);
  return GWEN_IDTABLE_MAXENTRIES-idt->freeEntries;
}



GWEN_TYPE_UINT32 GWEN_IdTable_GetFirstId(GWEN_IDTABLE *idt){
  unsigned int i;

  assert(idt);
  idt->current=0;
  for (i=0; i<GWEN_IDTABLE_MAXENTRIES; i++) {
    if (idt->entries[i]!=0) {
      idt->current=i;
      return idt->entries[i];
    }
  } /* for */
  return 0;
}



GWEN_TYPE_UINT32 GWEN_IdTable_GetNextId(GWEN_IDTABLE *idt){
  unsigned int i;

  assert(idt);

  for (i=idt->current+1; i<GWEN_IDTABLE_MAXENTRIES; i++) {
    if (idt->entries[i]!=0) {
      idt->current=i;
      return idt->entries[i];
    }
  } /* for */
  idt->current=GWEN_IDTABLE_MAXENTRIES;
  return 0;
}



GWEN_IDLIST *GWEN_IdList_new(){
  GWEN_IDLIST *idl;

  GWEN_NEW_OBJECT(GWEN_IDLIST, idl);
  idl->idTables=GWEN_IdTable_List_new();
  return idl;
}



void GWEN_IdList_free(GWEN_IDLIST *idl){
  if (idl) {
    GWEN_IdTable_List_free(idl->idTables);
    GWEN_FREE_OBJECT(idl);
  }
}



int GWEN_IdList_AddId(GWEN_IDLIST *idl, GWEN_TYPE_UINT32 id){
  GWEN_IDTABLE *idt;

  assert(idl);

  idl->current=0;
  idt=GWEN_IdTable_List_First(idl->idTables);
  /* find free table */
  while(idt) {
    if (!GWEN_IdTable_IsFull(idt))
      break;
    idt=GWEN_IdTable_List_Next(idt);
  } /* while */

  if (!idt) {
    idt=GWEN_IdTable_new();
    GWEN_IdTable_List_Add(idt, idl->idTables);;
  }

  GWEN_IdTable_AddId(idt, id);
  idl->entryCount++;
  return 0;
}



int GWEN_IdList_DelId(GWEN_IDLIST *idl, GWEN_TYPE_UINT32 id){
  GWEN_IDTABLE *idt;

  assert(idl);

  idl->current=0;
  idt=GWEN_IdTable_List_First(idl->idTables);
  /* find table */
  while(idt) {
    if (!GWEN_IdTable_DelId(idt, id)) {
      /* found a table which had this id */
      GWEN_IdList_Clean(idl);
      idl->entryCount--;
      return 0;
    }
    idt=GWEN_IdTable_List_Next(idt);
  } /* while */
  return -1;
}



int GWEN_IdList_HasId(const GWEN_IDLIST *idl, GWEN_TYPE_UINT32 id){
  GWEN_IDTABLE *idt;

  assert(idl);

  idt=GWEN_IdTable_List_First(idl->idTables);
  /* find free table */
  while(idt) {
    if (GWEN_IdTable_HasId(idt, id))
      return 1;
    idt=GWEN_IdTable_List_Next(idt);
  } /* while */
  return 0;
}



void GWEN_IdList_Clean(GWEN_IDLIST *idl) {
  GWEN_IDTABLE *idt;

  assert(idl);
  idl->current=0;
  idt=GWEN_IdTable_List_First(idl->idTables);
  /* find free table */
  while(idt) {
    GWEN_IDTABLE *next;

    next=GWEN_IdTable_List_Next(idt);
    if (GWEN_IdTable_IsEmpty(idt)) {
      GWEN_IdTable_List_Del(idt);
      GWEN_IdTable_free(idt);
    }
    idt=next;
  } /* while */
}



GWEN_TYPE_UINT32 GWEN_IdList_GetFirstId(GWEN_IDLIST *idl){
  GWEN_IDTABLE *idt;

  assert(idl);

  idt=GWEN_IdTable_List_First(idl->idTables);
  /* find free table */
  while(idt) {
    GWEN_IDTABLE *next;
    GWEN_TYPE_UINT32 id;

    next=GWEN_IdTable_List_Next(idt);
    id=GWEN_IdTable_GetFirstId(idt);
    if (id) {
      idl->current=idt;
      return id;
    }
    idt=next;
  } /* while */

  return 0;
}



GWEN_TYPE_UINT32 GWEN_IdList_GetNextId(GWEN_IDLIST *idl){
  GWEN_IDTABLE *idt;
  GWEN_TYPE_UINT32 id;

  assert(idl);

  idt=idl->current;
  if (idt) {
    id=GWEN_IdTable_GetNextId(idt);
    if (id) {
      idl->current=idt;
      return id;
    }
  }
  else {
    idl->current=0;
    return 0;
  }

  idt=GWEN_IdTable_List_Next(idt);
  while (idt) {
    id=GWEN_IdTable_GetFirstId(idt);
    if (id) {
      idl->current=idt;
      return id;
    }
    idt=GWEN_IdTable_List_Next(idt);
  } /* while */

  idl->current=0;
  return 0;
}



int GWEN_IdList_Sort(GWEN_IDLIST *idl){
  GWEN_IDTABLE *idt;
  unsigned int cnt;
  GWEN_TYPE_UINT32 *ptr;
  unsigned int i;

  assert(idl);

  /* count ids */
  idt=GWEN_IdTable_List_First(idl->idTables);
  cnt=0;
  while(idt) {
    GWEN_IDTABLE *next;

    next=GWEN_IdTable_List_Next(idt);
    cnt+=GWEN_IdTable_GetCount(idt);
    idt=next;
  } /* while */

  if (!cnt)
    return 0;

  /* move ids to a temporary list */
  ptr=(GWEN_TYPE_UINT32*)malloc(sizeof(GWEN_TYPE_UINT32)*cnt);
  assert(ptr);

  for (i=0; i<cnt; i++) {
    GWEN_TYPE_UINT32 id;

    if (i==0)
      id=GWEN_IdList_GetFirstId(idl);
    else
      id=GWEN_IdList_GetNextId(idl);
    assert(id);
    ptr[i]=id;
  } /* for */

  GWEN_IdTable_List_Clear(idl->idTables);
  idl->current=0;

  /* sort temporary list */
  while(1) {
    int rpl;

    rpl=0;
    for (i=0; i<(cnt-1); i++) {
      if (ptr[i]>ptr[i+1]) {
        GWEN_TYPE_UINT32 id;

        id=ptr[i];
        ptr[i]=ptr[i+1];
        ptr[i+1]=id;
        rpl=1;
      }
    } /* for */
    if (!rpl)
      break;
  } /* while */

  /* move back from temporary list */
  for (i=0; i<cnt; i++) {
    GWEN_IdList_AddId(idl, ptr[i]);
  }
  free(ptr);

  return 0;
}



void GWEN_IdList_Clear(GWEN_IDLIST *idl){
  assert(idl);
  GWEN_IdTable_List_Clear(idl->idTables);
  idl->entryCount=0;
  idl->current=0;
}



GWEN_IDLIST *GWEN_IdList_dup(const GWEN_IDLIST *idl){
  GWEN_IDLIST *nidl;
  GWEN_IDTABLE *idt;

  assert(idl);
  nidl=GWEN_IdList_new();

  idt=GWEN_IdTable_List_First(idl->idTables);
  while(idt) {
    if (idt->freeEntries!=GWEN_IDTABLE_MAXENTRIES){
      int i;

      for (i=0; i<GWEN_IDTABLE_MAXENTRIES; i++) {
	if (idt->entries[i]!=0)
	  GWEN_IdList_AddId(nidl, idt->entries[i]);
      }
    }
    idt=GWEN_IdTable_List_Next(idt);
  }

  return nidl;
}













