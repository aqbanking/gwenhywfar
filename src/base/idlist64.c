/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
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

#include "idlist64_p.h"
#include <gwenhywfar/debug.h>


#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Uncomment this to use the old compact mode of entry storage
 * compact means that empty entries within tables are filled from begin on when
 * adding new entries (this was the old mode of id allocation).
 * The new mode is to add entries to the last table. If the last table is full
 * a new table is created and the new id added to that table.
 */
/*#define GWEN_IDLIST64_COMPACT */


GWEN_IDTABLE64 *GWEN_IdTable64_new(void)
{
  GWEN_IDTABLE64 *idt;

  GWEN_NEW_OBJECT(GWEN_IDTABLE64, idt);
  idt->refCount=1;

  idt->freeEntries=GWEN_IDTABLE64_MAXENTRIES;
  return idt;
}



void GWEN_IdTable64_free(GWEN_IDTABLE64 *idt)
{
  if (idt) {
    assert(idt->refCount);
    if (--(idt->refCount)==0) {
      GWEN_FREE_OBJECT(idt);
    }
  }
}



#if 0
void GWEN_IdTable64_Attach(GWEN_IDTABLE64 *idt)
{
  assert(idt);
  assert(idt->refCount);
  idt->refCount++;
}
#endif



static inline int GWEN_IdTable64_AddId(GWEN_IDTABLE64 *idt, uint64_t id)
{
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



static inline int GWEN_IdTable64_AppendId(GWEN_IDTABLE64 *idt, uint64_t id)
{
  if (idt->freeEntries) {
    unsigned int i;

    i=GWEN_IDTABLE64_MAXENTRIES-idt->freeEntries;
    idt->entries[i]=id;
    idt->freeEntries--;
    return 0;
  }
  else
    return -1;
}



static inline int GWEN_IdTable64_HasId(const GWEN_IDTABLE64 *idt, uint64_t id)
{
  unsigned int i;

  for (i=0; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
    if (idt->entries[i]==id) {
      return 1;
    }
  } /* for */
  return 0;
}



static inline int GWEN_IdTable64_DelId(GWEN_IDTABLE64 *idt, uint64_t id)
{
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



static inline int GWEN_IdTable64_IsEmpty(const GWEN_IDTABLE64 *idt)
{
  return GWEN_IDTABLE64_MAXENTRIES==idt->freeEntries;
}



static inline int GWEN_IdTable64_IsFull(const GWEN_IDTABLE64 *idt)
{
  return idt->freeEntries==0;
}


#if 0
static inline unsigned int GWEN_IdTable64_GetCount(const GWEN_IDTABLE64 *idt)
{
  return GWEN_IDTABLE64_MAXENTRIES-idt->freeEntries;
}



static inline uint64_t GWEN_IdTable64_GetFirstId(GWEN_IDTABLE64 *idt)
{
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



static inline uint64_t GWEN_IdTable64_GetNextId(GWEN_IDTABLE64 *idt)
{
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
                                                  uint64_t *tabIdx)
{
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
                                                 uint64_t *tabIdx)
{
  unsigned int i;

  for (i=(*tabIdx)+1; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
    if (idt->entries[i]!=0) {
      *tabIdx=i;
      return idt->entries[i];
    }
  } /* for */
  return 0;
}
#endif





GWEN_IDLIST64 *GWEN_IdList64_new(void)
{
  GWEN_IDLIST64 *idl;

  GWEN_NEW_OBJECT(GWEN_IDLIST64, idl);
  idl->refCount=1;
  return idl;
}



void GWEN_IdList64_Attach(GWEN_IDLIST64 *idl)
{
  assert(idl);
  assert(idl->refCount);
  idl->refCount++;
}



void GWEN_IdList64_free(GWEN_IDLIST64 *idl)
{
  if (idl) {
    assert(idl->refCount);
    if (idl->refCount==1) {
      GWEN_IdList64_Clear(idl);
      idl->refCount=0;
      GWEN_FREE_OBJECT(idl);
    }
    else
      idl->refCount--;
  }
}



void GWEN_IdList64_AddTable(GWEN_IDLIST64 *idl, GWEN_IDTABLE64 *idt)
{
  GWEN_IDTABLE64 **tablePtr;
  uint32_t idx;

  assert(idl);

  tablePtr=idl->pIdTablePointers;
  for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
    if (*tablePtr==NULL)
      break;
  } /* while */

  if (idx>=idl->idTableCount) {
    uint32_t newCount;
    GWEN_IDTABLE64 **newPtr;

    /* resize */
    newCount=idl->idTableCount+GWEN_IDLIST64_STEP;
    newPtr=(GWEN_IDTABLE64 **)realloc(idl->pIdTablePointers, sizeof(GWEN_IDTABLE64 *)*newCount);
    assert(newPtr);
    /* init new pointers */
    memset((void *)(newPtr+idl->idTableCount),
           0,
           sizeof(GWEN_IDTABLE64 *)*(newCount-idl->idTableCount));
    idl->pIdTablePointers=newPtr;
    idl->pIdTablePointers[idl->idTableCount]=idt;
    idl->lastTableIdx=idl->idTableCount;  /* this is the idx of the new table, and it is the last one */
    idl->idTableCount=newCount;
  }
  else {
    idl->pIdTablePointers[idx]=idt;
    idl->lastTableIdx=idx;
  }
}



int GWEN_IdList64_AddId(GWEN_IDLIST64 *idl, uint64_t id)
{
#ifdef GWEN_IDLIST64_COMPACT
  GWEN_IDTABLE64 *idt=NULL;
  GWEN_IDTABLE64 **tablePtr;
  int idx;

  assert(idl);

  if (idl->pIdTablePointers==NULL) {
    /* create an initial pointer table which can take up to GWEN_IDLIST64_STEP pointers */
    idl->pIdTablePointers=(GWEN_IDTABLE64 **) malloc(sizeof(GWEN_IDTABLE64 *)*GWEN_IDLIST64_STEP);
    assert(idl->pIdTablePointers);
    memset(idl->pIdTablePointers, 0, sizeof(GWEN_IDTABLE64 *)*GWEN_IDLIST64_STEP);
    idl->idTableCount=GWEN_IDLIST64_STEP;
  }

  for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
    idt=*tablePtr;
    if (idt && !GWEN_IdTable64_IsFull(idt))
      break;
  } /* while */

  if (idx>=idl->idTableCount) {
    idt=GWEN_IdTable64_new();
    GWEN_IdList64_AddTable(idl, idt);
  }
  GWEN_IdTable64_AddId(idt, id);
  idl->entryCount++;
  return 0;
#else
  GWEN_IDTABLE64 *idt=NULL;
  int idx;

  assert(idl);

  if (idl->pIdTablePointers==NULL) {
    /* create an initial pointer table which can take up to GWEN_IDLIST64_STEP pointers */
    idl->pIdTablePointers=(GWEN_IDTABLE64 **) malloc(sizeof(GWEN_IDTABLE64 *)*GWEN_IDLIST64_STEP);
    assert(idl->pIdTablePointers);
    memset(idl->pIdTablePointers, 0, sizeof(GWEN_IDTABLE64 *)*GWEN_IDLIST64_STEP);
    idl->idTableCount=GWEN_IDLIST64_STEP;
  }
  idx=idl->lastTableIdx;
  idt=idl->pIdTablePointers[idx];
  if (idt==NULL || GWEN_IdTable64_IsFull(idt)) {
    idt=GWEN_IdTable64_new();
    GWEN_IdList64_AddTable(idl, idt);
  }
  GWEN_IdTable64_AddId(idt, id);
  idl->entryCount++;
  return 0;
#endif
}



int GWEN_IdList64_DelId(GWEN_IDLIST64 *idl, uint64_t id)
{
  if (idl->pIdTablePointers) {
    GWEN_IDTABLE64 *idt=NULL;
    GWEN_IDTABLE64 **tablePtr;
    uint32_t idx;

    for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
      idt=*tablePtr;
      if (idt && !GWEN_IdTable64_DelId(idt, id)) {
        /* found a table which had this id */
        GWEN_IdList64_Clean(idl);
        idl->entryCount--;
        return 0;
      }
    }
  }

  return -1;
}



int GWEN_IdList64_HasId(const GWEN_IDLIST64 *idl, uint64_t id)
{
  if (idl->pIdTablePointers) {
    GWEN_IDTABLE64 *idt=NULL;
    GWEN_IDTABLE64 **tablePtr;
    uint32_t idx;

    for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
      idt=*tablePtr;
      if (idt && GWEN_IdTable64_HasId(idt, id))
        return 1;
    }
  }

  return 0;
}



void GWEN_IdList64_Clean(GWEN_IDLIST64 *idl)
{
  GWEN_IDTABLE64 *idt=NULL;
  GWEN_IDTABLE64 **tablePtr;
  uint32_t idx;

  for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
    idt=*tablePtr;
    if (idt && GWEN_IdTable64_IsEmpty(idt)) {
      GWEN_IdTable64_free(idt);
      *tablePtr=NULL;
    }
  }
}



void GWEN_IdList64_Clear(GWEN_IDLIST64 *idl)
{
  if (idl->pIdTablePointers) {
    GWEN_IDTABLE64 *idt=NULL;
    GWEN_IDTABLE64 **tablePtr;
    uint32_t idx;

    for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
      idt=*tablePtr;
      if (idt) {
        GWEN_IdTable64_free(idt);
        *tablePtr=NULL;
      }
    }
    free(idl->pIdTablePointers);
    idl->pIdTablePointers=NULL;
  }
  idl->entryCount=0;
  idl->nextIdx=0;
}



static int __compAscending(const void *pa, const void *pb)
{
  uint64_t a=*((const uint64_t *)pa);
  uint64_t b=*((const uint64_t *)pb);

  if (a<b)
    return -1;
  else if (a>b)
    return 1;
  else
    return 0;
}



static int __compDescending(const void *pa, const void *pb)
{
  uint64_t a=*((const uint64_t *)pa);
  uint64_t b=*((const uint64_t *)pb);

  if (a<b)
    return 1;
  else if (a>b)
    return -1;
  else
    return 0;
}



static int GWEN_IdList64__Sort(GWEN_IDLIST64 *idl, int ascending)
{
  assert(idl);
  assert(idl->refCount);
  if (idl->pIdTablePointers && idl->entryCount) {
    GWEN_IDLIST64_ITERATOR *it;
    unsigned int cnt;
    uint64_t *ptr;
    unsigned int i;

    assert(idl);

    /* count ids */
    cnt=idl->entryCount;

    /* move ids to a temporary list */
    ptr=(uint64_t *)malloc(sizeof(uint64_t)*cnt);
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
    GWEN_IdList64_Clear(idl);

    if (ascending)
      qsort(ptr, cnt, sizeof(uint64_t), __compAscending);
    else
      qsort(ptr, cnt, sizeof(uint64_t), __compDescending);

    /* move back sorted list of ids from temporary list */
    for (i=0; i<cnt; i++) {
      GWEN_IdList64_AddId(idl, ptr[i]);
    }
    free(ptr);
  }
  return 0;
}



int GWEN_IdList64_Sort(GWEN_IDLIST64 *idl)
{
  return GWEN_IdList64__Sort(idl, 1);
}



int GWEN_IdList64_ReverseSort(GWEN_IDLIST64 *idl)
{
  return GWEN_IdList64__Sort(idl, 0);
}



GWEN_IDLIST64 *GWEN_IdList64_dup(const GWEN_IDLIST64 *idl)
{
  GWEN_IDLIST64 *nidl;
  uint32_t idx;

  nidl=GWEN_IdList64_new();

  nidl->idTableCount=idl->idTableCount;
  nidl->entryCount=idl->entryCount;
  if (idl->pIdTablePointers) {
    for (idx=0; idx<idl->idTableCount; idx++) {
      GWEN_IDTABLE64 *idt;

      idt=idl->pIdTablePointers[idx];
      if (idt && !GWEN_IdTable64_IsEmpty(idt)) {
        GWEN_IDTABLE64 *nidt;

        nidt=GWEN_IdTable64_new();
        memmove(nidt->entries, idt->entries, GWEN_IDTABLE64_MAXENTRIES*sizeof(uint64_t));
        nidt->freeEntries=idt->freeEntries;
        GWEN_IdList64_AddTable(nidl, nidt);
      }
    }
  }

  return nidl;
}



uint64_t GWEN_IdList64_GetEntryCount(const GWEN_IDLIST64 *idl)
{
  assert(idl);
  assert(idl->refCount);

  return idl->entryCount;
}



uint64_t GWEN_IdList64__GetFirstId(const GWEN_IDLIST64 *idl, uint64_t *pos)
{
  GWEN_IDTABLE64 *idt=NULL;
  GWEN_IDTABLE64 **tablePtr;
  uint32_t idx;
  int idIndex=0;

  *pos=0;
  for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
    idt=*tablePtr;
    if (idt && !GWEN_IdTable64_IsEmpty(idt)) {
      int i;
      uint64_t id;

      for (i=0; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
        if (idt->entries[i]!=0) {
          id=idt->entries[i];
          *pos=idIndex+i+1;
          return id;
        }
      }
    }
    idIndex+=GWEN_IDTABLE64_MAXENTRIES;
  }

  return 0;
}



uint64_t GWEN_IdList64__GetNextId(const GWEN_IDLIST64 *idl, uint64_t *pos)
{
  if (*pos) {
    GWEN_IDTABLE64 *idt;
    uint64_t tableNum=*pos / GWEN_IDTABLE64_MAXENTRIES;
    uint64_t tableIdx=*pos % GWEN_IDTABLE64_MAXENTRIES;
    GWEN_IDTABLE64 **tablePtr;
    int idIndex=0;
    uint32_t idx;

    if (tableNum>idl->idTableCount) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Table number out of range");
      *pos=0;
      return 0;
    }

    idIndex=(tableNum*GWEN_IDTABLE64_MAXENTRIES);
    for (idx=tableNum, tablePtr=idl->pIdTablePointers+tableNum; idx<idl->idTableCount; idx++, tablePtr++) {
      idt=*tablePtr;
      if (idt && !GWEN_IdTable64_IsEmpty(idt)) {
        int i;
        uint64_t id;

        if (idx==tableNum) {
          for (i=tableIdx; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
            if (idt->entries[i]!=0) {
              id=idt->entries[i];
              *pos=idIndex+i+1;
              return id;
            }
          }
        }
        else {
          for (i=0; i<GWEN_IDTABLE64_MAXENTRIES; i++) {
            if (idt->entries[i]!=0) {
              id=idt->entries[i];
              *pos=idIndex+i+1;
              return id;
            }
          }
        }
      }
      idIndex+=GWEN_IDTABLE64_MAXENTRIES;
    }
    *pos=0;
  }

  return 0;
}



#ifndef NO_DEPRECATED_SYMBOLS
uint64_t GWEN_IdList64_GetFirstId(GWEN_IDLIST64 *idl)
{
  return GWEN_IdList64__GetFirstId(idl, &(idl->nextIdx));
}



uint64_t GWEN_IdList64_GetNextId(GWEN_IDLIST64 *idl)
{
  return GWEN_IdList64__GetNextId(idl, &(idl->nextIdx));
}



uint64_t GWEN_IdList64_GetFirstId2(const GWEN_IDLIST64 *idl, uint64_t *pos)
{
  return GWEN_IdList64__GetFirstId(idl, pos);
}



uint64_t GWEN_IdList64_GetNextId2(const GWEN_IDLIST64 *idl, uint64_t *pos)
{
  return GWEN_IdList64__GetNextId(idl, pos);
}
#endif  // ifndef NO_DEPRECATED_SYMBOLS






GWEN_IDLIST64_ITERATOR *GWEN_IdList64_Iterator_new(GWEN_IDLIST64 *idl)
{
  GWEN_IDLIST64_ITERATOR *it;

  assert(idl);
  GWEN_NEW_OBJECT(GWEN_IDLIST64_ITERATOR, it);

  GWEN_IdList64_Attach(idl);
  it->list=idl;

  return it;
}



void GWEN_IdList64_Iterator_free(GWEN_IDLIST64_ITERATOR *it)
{
  if (it) {
    GWEN_IdList64_free(it->list);
    GWEN_FREE_OBJECT(it);
  }
}



uint64_t GWEN_IdList64_Iterator_GetFirstId(GWEN_IDLIST64_ITERATOR *it)
{
  return GWEN_IdList64__GetFirstId(it->list, &(it->nextIndex));
}



uint64_t GWEN_IdList64_Iterator_GetNextId(GWEN_IDLIST64_ITERATOR *it)
{
  return GWEN_IdList64__GetNextId(it->list, &(it->nextIndex));
}



int GWEN_IdList64_AppendId(GWEN_IDLIST64 *idl, uint64_t id)
{
  GWEN_IDTABLE64 *idt=NULL;

  assert(idl);

  if (idl->pIdTablePointers==NULL) {
    /* create an initial pointer table which can take up to GWEN_IDLIST64_STEP pointers */
    idl->pIdTablePointers=(GWEN_IDTABLE64 **) malloc(sizeof(GWEN_IDTABLE64 *)*GWEN_IDLIST64_STEP);
    assert(idl->pIdTablePointers);
    memset(idl->pIdTablePointers, 0, sizeof(GWEN_IDTABLE64 *)*GWEN_IDLIST64_STEP);
    idl->idTableCount=GWEN_IDLIST64_STEP;
  }

  assert(idl->lastTableIdx<idl->idTableCount);
  idt=idl->pIdTablePointers[idl->lastTableIdx];
  if (idt==NULL || GWEN_IdTable64_IsFull(idt)) {
    idt=GWEN_IdTable64_new();
    GWEN_IdList64_AddTable(idl, idt);
  }

  GWEN_IdTable64_AppendId(idt, id);
  idl->entryCount++;
  return 0;
}



uint64_t GWEN_IdList64_GetIdAt(const GWEN_IDLIST64 *idl, uint64_t idx)
{
  GWEN_IDTABLE64 *idt;
  uint64_t tableNum=idx / GWEN_IDTABLE64_MAXENTRIES;
  uint64_t tableIdx=idx % GWEN_IDTABLE64_MAXENTRIES;

  assert(idl);
  if (tableNum>idl->idTableCount) {
    DBG_INFO(GWEN_LOGDOMAIN, "Table index out of range");
    return 0;
  }

  idt=idl->pIdTablePointers[tableNum];
  if (idt==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "Table index points to an empty table");
    return 0;
  }

  return idt->entries[tableIdx];
}











