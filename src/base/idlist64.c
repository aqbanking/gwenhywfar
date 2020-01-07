/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2020 by Martin Preuss
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

#include <gwenhywfar/memory.h>
#include <gwenhywfar/debug.h>


#include <stdlib.h>
#include <assert.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static GWENHYWFAR_CB void _attachToTable(GWEN_SIMPLEPTRLIST *pl, void *p);
static GWENHYWFAR_CB void _detachFromTable(GWEN_SIMPLEPTRLIST *pl, void *p);

static int GWEN_IdList64__Sort(GWEN_IDLIST64 *idl, int ascending);
static int __compAscending(const void *pa, const void *pb);
static int __compDescending(const void *pa, const void *pb);

static GWEN_IDTABLE64 *GWEN_IdTable64_new();
static void GWEN_IdTable64_free(GWEN_IDTABLE64 *ft);
static void GWEN_IdTable64_Attach(GWEN_IDTABLE64 *ft);
static GWEN_IDTABLE64 *GWEN_IdTable64_dup(const GWEN_IDTABLE64 *ftOrig);
static GWEN_IDTABLE64 *GWEN_IdTable64_Create(uint64_t maxEntries);
static uint64_t GWEN_IdTable64_GetMaxEntries(const GWEN_IDTABLE64 *ft);
static uint64_t GWEN_IdTable64_GetFreeEntries(const GWEN_IDTABLE64 *ft);
static void GWEN_IdTable64_DecFreeEntries(GWEN_IDTABLE64 *ft);
static uint64_t GWEN_IdTable64_GetHighestEntry(const GWEN_IDTABLE64 *ft);
static void GWEN_IdTable64_CheckAndSetHighestEntry(GWEN_IDTABLE64 *ft, uint64_t i);

static uint32_t GWEN_IdTable64_GetRuntimeFlags(const GWEN_IDTABLE64 *ft);
static void GWEN_IdTable64_AddRuntimeFlags(GWEN_IDTABLE64 *ft, uint32_t i);

static uint64_t *GWEN_IdTable64_GetPtrEntries(const GWEN_IDTABLE64 *ft);
static void GWEN_IdTable64_SetPtrEntries(GWEN_IDTABLE64 *ft, uint64_t *ptr);

static GWEN_IDTABLE64 *GWEN_IdList64_GetTableAt(const GWEN_IDLIST64 *tl, uint64_t idx);
/*static int GWEN_IdList64_SetIdAt(GWEN_IDLIST64 *tl, uint64_t idx, uint64_t entry);*/
static uint64_t GWEN_IdList64__GetFirstId(const GWEN_IDLIST64 *idl, uint64_t *pos);
static uint64_t GWEN_IdList64__GetNextId(const GWEN_IDLIST64 *idl, uint64_t *pos);




/* ------------------------------------------------------------------------------------------------
 * GWEN_IdList64
 * ------------------------------------------------------------------------------------------------
 */


GWEN_IDLIST64 *GWEN_IdList64_new(int tableMaxEntries)
{
  GWEN_IDLIST64 *idl;

  idl=GWEN_SimplePtrList_new(tableMaxEntries, tableMaxEntries);
  GWEN_SimplePtrList_SetAttachObjectFn(idl, _attachToTable);
  GWEN_SimplePtrList_SetFreeObjectFn(idl, _detachFromTable);
  GWEN_SimplePtrList_AddFlags(idl, GWEN_SIMPLEPTRLIST_FLAGS_ATTACHTOOBJECTS);
  GWEN_SimplePtrList_AddFlags(idl, GWEN_SIMPLEPTRLIST_FLAGS_DETACHFROMOBJECTS);
  GWEN_SimplePtrList_SetUserIntData(idl, tableMaxEntries);

  return idl;
}



void GWEN_IdList64_Attach(GWEN_IDLIST64 *idl)
{
  GWEN_SimplePtrList_Attach(idl);
}



void GWEN_IdList64_free(GWEN_IDLIST64 *idl)
{
  GWEN_SimplePtrList_free(idl);
}



void GWEN_IdList64_Clear(GWEN_IDLIST64 *idl)
{
  GWEN_SimplePtrList_Clear(idl);
  GWEN_SimplePtrList_SetUserCounter(idl, 0);
}



void GWEN_IdList64_IncIdCounter(GWEN_SIMPLEPTRLIST *pl)
{
  GWEN_SimplePtrList_IncUserCounter(pl);
}



int GWEN_IdList64_DecIdCounter(GWEN_SIMPLEPTRLIST *pl)
{
  return GWEN_SimplePtrList_DecUserCounter(pl);
}



uint64_t GWEN_IdList64_GetEntryCount(const GWEN_SIMPLEPTRLIST *pl)
{
  return GWEN_SimplePtrList_GetUserCounter(pl);
}




int GWEN_IdList64_GetTableMaxEntries(const GWEN_IDLIST64 *idl)
{
  return GWEN_SimplePtrList_GetUserIntData(idl);
}



GWEN_IDLIST64 *GWEN_IdList64_LazyCopy(GWEN_IDLIST64 *oldList)
{
  return GWEN_SimplePtrList_LazyCopy(oldList);
}



GWEN_IDTABLE64 *GWEN_IdList64_GetTableAt(const GWEN_IDLIST64 *idl, uint64_t idx)
{
  return GWEN_SimplePtrList_GetPtrAt(idl, idx);
}



int GWEN_IdList64_SetTableAt(GWEN_IDLIST64 *idl, uint64_t idx, GWEN_IDTABLE64 *t)
{
  return GWEN_SimplePtrList_SetPtrAt(idl, idx, t);
}



int64_t GWEN_IdList64_AddTable(GWEN_IDLIST64 *idl, GWEN_IDTABLE64 *t)
{
  return GWEN_SimplePtrList_AddPtr(idl, t);
}



uint64_t GWEN_IdList64_GetUsedTables(const GWEN_IDLIST64 *idl)
{
  return GWEN_SimplePtrList_GetUsedEntries(idl);
}



int64_t GWEN_IdList64_GetLastTablePos(const GWEN_IDLIST64 *idl)
{
  uint64_t idx;

  idx=GWEN_SimplePtrList_GetUsedEntries(idl);
  if (idx)
    return idx-1;
  return GWEN_ERROR_NO_DATA;
}



int64_t GWEN_IdList64_GetIdAt(const GWEN_IDLIST64 *idl, uint64_t idx)
{
  int entriesPerTable;

  entriesPerTable=GWEN_SimplePtrList_GetUserIntData(idl);
  if (entriesPerTable) {
    uint64_t tablePos;
    GWEN_IDTABLE64 *t;

    tablePos=idx/entriesPerTable;
    t=GWEN_IdList64_GetTableAt(idl, tablePos);
    if (t) {
      uint64_t *entries;

      entries=GWEN_IdTable64_GetPtrEntries(t);
      if (entries) {
	uint64_t entryPos;

	entryPos=idx%entriesPerTable;
	return entries[entryPos];
      }
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "No table at table pos %lu", (unsigned long) tablePos);
    }
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No entriesPerTable");
  }

  return GWEN_ERROR_BUFFER_OVERFLOW;
}


#if 0
int GWEN_IdList64_SetIdAt(GWEN_IDLIST64 *idl, uint64_t idx, uint64_t entry)
{
  int rv;
  int entriesPerTable;

  rv=GWEN_SimplePtrList_EnsureWritability(idl);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return (int64_t) rv;
  }

  entriesPerTable=GWEN_SimplePtrList_GetUserIntData(idl);
  if (entriesPerTable) {
    uint64_t tablePos;
    GWEN_IDTABLE64 *t;

    tablePos=idx/entriesPerTable;
    t=GWEN_IdList64_GetTableAt(idl, tablePos);
    if (t) {
      uint64_t *entries;

      /* copy table if necessary (copy-on-write) */
      if (!(GWEN_IdTable64_GetRuntimeFlags(t) & GWEN_IDTABLE64_RUNTIME_FLAGS_ISCOPY)) {
	GWEN_IDTABLE64 *pTableCopy;

	pTableCopy=GWEN_IdTable64_dup(t);
	GWEN_IdList64_SetTableAt(idl, tablePos, pTableCopy);
	t=pTableCopy;
	GWEN_IdTable64_AddRuntimeFlags(t, GWEN_IDTABLE64_RUNTIME_FLAGS_ISCOPY);
      }

      entries=GWEN_IdTable64_GetPtrEntries(t);
      if (entries) {
	uint64_t entryPos;

	entryPos=idx%entriesPerTable;
	entries[entryPos]=entry;
	return 0;
      }
    } /* if (t) */
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "No table at position %lu", (unsigned long int) tablePos);
      return GWEN_ERROR_INTERNAL;
    }
  } /* if (entriesPerTable) */
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No entriesPerTable, internal error");
    return GWEN_ERROR_INTERNAL;
  }

  return GWEN_ERROR_BUFFER_OVERFLOW;

}
#endif


int64_t GWEN_IdList64_AddId(GWEN_IDLIST64 *idl, uint64_t entry)
{
  GWEN_IDTABLE64 *pTableCurrent=NULL;
  int64_t idxTableCurrent=0;
  int entriesPerTable=GWEN_IdList64_GetTableMaxEntries(idl);
  int rv;

  rv=GWEN_SimplePtrList_EnsureWritability(idl);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return (int64_t) rv;
  }

  /* get last table */
  idxTableCurrent=GWEN_IdList64_GetLastTablePos(idl);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Last table pos is %d", (int)idxTableCurrent);
  if (idxTableCurrent>=0)
    pTableCurrent=GWEN_IdList64_GetTableAt(idl, idxTableCurrent);

  /* check last table for existence and free entries, possibly create and add new table */
  if (pTableCurrent==NULL || GWEN_IdTable64_GetFreeEntries(pTableCurrent)==0) {
    /* create new table */
    if (pTableCurrent==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "No table, need to create one");
    }
    else if (GWEN_IdTable64_GetFreeEntries(pTableCurrent)==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Current table has no free entries, need to create new one");
    }

    DBG_INFO(GWEN_LOGDOMAIN, "Creating table with %d entries", entriesPerTable);
    pTableCurrent=GWEN_IdTable64_Create(entriesPerTable);
    GWEN_IdTable64_AddRuntimeFlags(pTableCurrent, GWEN_IDTABLE64_RUNTIME_FLAGS_ISCOPY); /* no need to copy later */

    /* add table to list */
    idxTableCurrent=GWEN_IdList64_AddTable(idl, pTableCurrent);
    if (idxTableCurrent<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int) idxTableCurrent);
      GWEN_IdTable64_free(pTableCurrent);
      return idxTableCurrent;
    }
    GWEN_IdTable64_free(pTableCurrent);
  } /* if (pTableCurrent || GWEN_IdTable64_GetFreeEntries(pTableCurrent)==0) */

  /* allocate free entry in current table */
  if (pTableCurrent && GWEN_IdTable64_GetFreeEntries(pTableCurrent)) {
    uint64_t *ptr;
    int64_t index=idxTableCurrent*entriesPerTable;
    int64_t entryPos;

    /* copy table if necessary (copy-on-write) */
    if (!(GWEN_IdTable64_GetRuntimeFlags(pTableCurrent) & GWEN_IDTABLE64_RUNTIME_FLAGS_ISCOPY)) {
      GWEN_IDTABLE64 *pTableCopy;

      DBG_INFO(GWEN_LOGDOMAIN, "Copying table at idx %lu", (unsigned long) idxTableCurrent);
      pTableCopy=GWEN_IdTable64_dup(pTableCurrent);
      GWEN_IdList64_SetTableAt(idl, idxTableCurrent, pTableCopy);
      GWEN_IdTable64_free(pTableCopy);
      pTableCurrent=pTableCopy;
      GWEN_IdTable64_AddRuntimeFlags(pTableCurrent, GWEN_IDTABLE64_RUNTIME_FLAGS_ISCOPY);
    }

    ptr=GWEN_IdTable64_GetPtrEntries(pTableCurrent);

    /* find entryPos of free entry in pTableCurrent */
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Current table (ptr=%p, %d entriesPerTable):", (void*)ptr, entriesPerTable);
    /*GWEN_IdTable64_Dump(pTableCurrent);*/
    if (GWEN_IdTable64_GetFreeEntries(pTableCurrent)==GWEN_IdTable64_GetMaxEntries(pTableCurrent)) {
      /** all entries are free, this is simple */
      entryPos=0;
    }
    else {
      if (GWEN_IdTable64_GetHighestEntry(pTableCurrent)+1<entriesPerTable) {
        /* fastest way: Just append to the end */
        DBG_VERBOUS(GWEN_LOGDOMAIN, "Finding free empty the fast way");
        entryPos=GWEN_IdTable64_GetHighestEntry(pTableCurrent)+1;
        if (ptr[entryPos]!=0) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Entry[highest+1] should be 0 but isnt, SNH!");
          return GWEN_ERROR_INTERNAL;
        }
      }
      else {
        /* slower way: find free entry somewhere in the table */
        DBG_ERROR(GWEN_LOGDOMAIN, "Finding free empty the slow way");
        for (entryPos=0; entryPos<entriesPerTable; entryPos++) {
          if (ptr[entryPos]==0)
            break;
        }
      }
    }

    DBG_VERBOUS(GWEN_LOGDOMAIN, "New entry will be at index %lu in table %lu (index=%lu, resulting index: %lu)",
                (unsigned long) entryPos,
                (unsigned long) idxTableCurrent,
                (unsigned long) index,
                (unsigned long) (index+entryPos));

    if (entryPos<entriesPerTable) {
      /* store new entry, get index */
      ptr[entryPos]=entry;
      index+=entryPos;
      GWEN_IdList64_IncIdCounter(idl);
      GWEN_IdTable64_DecFreeEntries(pTableCurrent);
      GWEN_IdTable64_CheckAndSetHighestEntry(pTableCurrent, entryPos);
      GWEN_IdTable64_AddRuntimeFlags(pTableCurrent, GWEN_IDTABLE64_RUNTIME_FLAGS_DIRTY);
      return index;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Free entry not found, internal counter is invalid. SNH!");
      return GWEN_ERROR_INTERNAL;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Still no table? SNH!");
    return GWEN_ERROR_INTERNAL;
  }
}



int GWEN_IdList64_HasId(const GWEN_IDLIST64 *idl, uint64_t wantedId)
{
  uint32_t idx;
  int entriesPerTable=GWEN_IdList64_GetTableMaxEntries(idl);
  int numTables=GWEN_IdList64_GetUsedTables(idl);

  for (idx=0; idx<numTables; idx++) {
    GWEN_IDTABLE64 *idt;

    idt=GWEN_IdList64_GetTableAt(idl, idx);
    if (idt) {
      int i;

      for (i=0; i<entriesPerTable; i++) {
        if (idt->ptrEntries[i]==wantedId) {
          return 1;
        }
      }
    }
  }
  return 0;
}



int GWEN_IdList64_DelId(GWEN_IDLIST64 *idl, uint64_t wantedId)
{
  uint32_t idx;
  int entriesPerTable=GWEN_IdList64_GetTableMaxEntries(idl);
  int numTables=GWEN_IdList64_GetUsedTables(idl);

  for (idx=0; idx<numTables; idx++) {
    GWEN_IDTABLE64 *idt;

    idt=GWEN_IdList64_GetTableAt(idl, idx);
    if (idt) {
      int i;

      for (i=0; i<entriesPerTable; i++) {
        if (idt->ptrEntries[i]==wantedId) {
          idt->ptrEntries[i]=0;
          GWEN_IdList64_DecIdCounter(idl);
          return 1;
        }
      }
    }
  }
  return 0;
}





void _attachToTable(GWEN_UNUSED GWEN_SIMPLEPTRLIST *pl, void *p)
{
  GWEN_IDTABLE64 *ft;

  ft=(GWEN_IDTABLE64*) p;
  GWEN_IdTable64_Attach(ft);
}



void _detachFromTable(GWEN_UNUSED GWEN_SIMPLEPTRLIST *pl, void *p)
{
  GWEN_IDTABLE64 *ft;

  ft=(GWEN_IDTABLE64*) p;
  GWEN_IdTable64_free(ft);
}



uint64_t GWEN_IdList64__GetFirstId(const GWEN_IDLIST64 *idl, uint64_t *pos)
{
  uint32_t idx;
  int idIndex=0;
  int entriesPerTable=GWEN_IdList64_GetTableMaxEntries(idl);
  int numTables=GWEN_IdList64_GetUsedTables(idl);

  *pos=0;
  for (idx=0; idx<numTables; idx++) {
    GWEN_IDTABLE64 *idt;

    idt=GWEN_IdList64_GetTableAt(idl, idx);
    if (idt) {
      int i;
      uint64_t id;

      for (i=0; i<entriesPerTable; i++) {
        if (idt->ptrEntries[i]!=0) {
          id=idt->ptrEntries[i];
          *pos=idIndex+i+1;
          return id;
        }
      }
    }
    idIndex+=entriesPerTable;
  }

  return 0;
}



uint64_t GWEN_IdList64__GetNextId(const GWEN_IDLIST64 *idl, uint64_t *pos)
{
  if (*pos) {
    int entriesPerTable=GWEN_IdList64_GetTableMaxEntries(idl);
    int numTables=GWEN_IdList64_GetUsedTables(idl);
    uint64_t tableNum;
    uint64_t tableIdx;
    int idIndex=0;
    uint32_t idx;

    tableNum=*pos / entriesPerTable;
    tableIdx=*pos % entriesPerTable;

    if (tableNum>numTables) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Table number out of range");
      *pos=0;
      return 0;
    }

    idIndex=(tableNum*entriesPerTable);

    for (idx=tableNum; idx<numTables; idx++) {
      GWEN_IDTABLE64 *idt;

      idt=GWEN_IdList64_GetTableAt(idl, idx);
      if (idt) {
        int i;
        uint64_t id;

        if (idx==tableNum) {
          for (i=tableIdx; i<entriesPerTable; i++) {
            if (idt->ptrEntries[i]!=0) {
              id=idt->ptrEntries[i];
              *pos=idIndex+i+1;
              return id;
            }
          }
        }
        else {
          for (i=0; i<entriesPerTable; i++) {
            if (idt->ptrEntries[i]!=0) {
              id=idt->ptrEntries[i];
              *pos=idIndex+i+1;
              return id;
            }
          }
        }
      }
      idIndex+=entriesPerTable;
    }
    *pos=0;
  }

  return 0;
}



int GWEN_IdList64__Sort(GWEN_IDLIST64 *idl, int ascending)
{
  uint64_t entryCount;

  assert(idl);

  entryCount=GWEN_IdList64_GetEntryCount(idl);

  if (entryCount) {
    GWEN_IDLIST64_ITERATOR *it;
    uint64_t *ptr;
    unsigned int i;

    assert(idl);

    /* move ids to a temporary list */
    ptr=(uint64_t *)malloc(sizeof(uint64_t)*entryCount);
    assert(ptr);

    it=GWEN_IdList64_Iterator_new(idl);
    for (i=0; i<entryCount; i++) {
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
      qsort(ptr, entryCount, sizeof(uint64_t), __compAscending);
    else
      qsort(ptr, entryCount, sizeof(uint64_t), __compDescending);

    /* move back sorted list of ids from temporary list */
    for (i=0; i<entryCount; i++) {
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



int __compAscending(const void *pa, const void *pb)
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



int __compDescending(const void *pa, const void *pb)
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







/* ------------------------------------------------------------------------------------------------
 * GWEN_IdList64_Iterator
 * ------------------------------------------------------------------------------------------------
 */


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






/* ------------------------------------------------------------------------------------------------
 * GWEN_IdTable64
 * ------------------------------------------------------------------------------------------------
 */



GWEN_IDTABLE64 *GWEN_IdTable64_new() {
  GWEN_IDTABLE64 *ft;

  GWEN_NEW_OBJECT(GWEN_IDTABLE64, ft);
  ft->refCount=1;

  return ft;
}



void GWEN_IdTable64_Attach(GWEN_IDTABLE64 *ft) {
  assert(ft && ft->refCount);
  if (ft && ft->refCount) {
    ft->refCount++;
  }
}



void GWEN_IdTable64_free(GWEN_IDTABLE64 *ft) {
  if (ft) {
    assert(ft->refCount);
    if (ft->refCount==1) {
      ft->refCount=0;
      free(ft->ptrEntries);
      GWEN_FREE_OBJECT(ft);
    }
    else {
      ft->refCount--;
    }
  }
}



int GWEN_IdTable64_GetRefCounter(const GWEN_IDTABLE64 *ft)
{
  assert(ft);
  return ft->refCount;
}



GWEN_IDTABLE64 *GWEN_IdTable64_dup(const GWEN_IDTABLE64 *ftOrig) {
  GWEN_IDTABLE64 *ft;

  assert(ftOrig);
  assert(ftOrig->refCount);
  ft=GWEN_IdTable64_new();
  ft->maxEntries=ftOrig->maxEntries;
  ft->freeEntries=ftOrig->freeEntries;
  ft->highestEntry=ftOrig->highestEntry;
  ft->runtimeFlags=ftOrig->runtimeFlags;

  /* copy offset entries */
  if (ftOrig->maxEntries && ftOrig->ptrEntries) {
    uint64_t offsetArraySize;

    offsetArraySize=ftOrig->maxEntries*sizeof(uint64_t);
    ft->ptrEntries=(uint64_t*) malloc(offsetArraySize);
    assert(ft->ptrEntries);
    memmove(ft->ptrEntries, ftOrig->ptrEntries, offsetArraySize);
  }

  return ft;
}



GWEN_IDTABLE64 *GWEN_IdTable64_Create(uint64_t maxEntries) {
  GWEN_IDTABLE64 *ft;
  uint64_t offsetArraySize;
  uint64_t *ptr;

  ft=GWEN_IdTable64_new();
  ft->maxEntries=maxEntries;
  ft->freeEntries=maxEntries;

  offsetArraySize=ft->maxEntries*sizeof(uint64_t);

  ptr=(uint64_t*) malloc(offsetArraySize);
  assert(ptr);
  memset(ptr, 0, offsetArraySize);
  GWEN_IdTable64_SetPtrEntries(ft, ptr);

  return ft;
}



uint64_t GWEN_IdTable64_GetMaxEntries(const GWEN_IDTABLE64 *ft) {
  assert(ft);
  assert(ft->refCount);
  return ft->maxEntries;
}



uint64_t GWEN_IdTable64_GetFreeEntries(const GWEN_IDTABLE64 *ft) {
  assert(ft);
  assert(ft->refCount);
  return ft->freeEntries;
}



void GWEN_IdTable64_DecFreeEntries(GWEN_IDTABLE64 *ft) {
  assert(ft);
  assert(ft->refCount);
  if (ft->freeEntries>0)
    ft->freeEntries--;
}



uint64_t GWEN_IdTable64_GetHighestEntry(const GWEN_IDTABLE64 *ft) {
  assert(ft);
  assert(ft->refCount);
  return ft->highestEntry;
}



void GWEN_IdTable64_CheckAndSetHighestEntry(GWEN_IDTABLE64 *ft, uint64_t i) {
  assert(ft);
  assert(ft->refCount);
  if (i>ft->highestEntry)
    ft->highestEntry=i;
}



uint64_t *GWEN_IdTable64_GetPtrEntries(const GWEN_IDTABLE64 *ft) {
  assert(ft);
  assert(ft->refCount);
  return ft->ptrEntries;
}



void GWEN_IdTable64_SetPtrEntries(GWEN_IDTABLE64 *ft, uint64_t *ptr) {
  assert(ft);
  assert(ft->refCount);
  if (ft->ptrEntries && ft->ptrEntries!=ptr)
    free(ft->ptrEntries);
  ft->ptrEntries=ptr;
}



uint32_t GWEN_IdTable64_GetRuntimeFlags(const GWEN_IDTABLE64 *ft) {
  assert(ft);
  return ft->runtimeFlags;
}



void GWEN_IdTable64_AddRuntimeFlags(GWEN_IDTABLE64 *ft, uint32_t i) {
  assert(ft);
  ft->runtimeFlags|=i;
}



/* include tests */
#include "idlist64-t.c"

