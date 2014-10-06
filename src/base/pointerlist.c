/***************************************************************************
    begin       : Wed Sep 12 2012
    copyright   : (C) 2012 by Martin Preuss
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


#include "pointerlist_p.h"
#include <gwenhywfar/debug.h>


#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_POINTERLIST_TABLE *GWEN_PointerListTable_new(void) {
  GWEN_POINTERLIST_TABLE *idt;

  GWEN_NEW_OBJECT(GWEN_POINTERLIST_TABLE, idt);
  idt->refCount=1;

  idt->freeEntries=GWEN_POINTERLIST_TABLE_MAXENTRIES;
  return idt;
}



void GWEN_PointerListTable_free(GWEN_POINTERLIST_TABLE *idt) {
  if (idt) {
    assert(idt->refCount);
    if (--(idt->refCount)==0) {
      GWEN_FREE_OBJECT(idt);
    }
  }
}



static inline int GWEN_PointerListTable_AddPtr(GWEN_POINTERLIST_TABLE *idt, void *ptr) {
  unsigned int i;

  for (i=0; i<GWEN_POINTERLIST_TABLE_MAXENTRIES; i++) {
    if (idt->entries[i]==0) {
      idt->entries[i]=ptr;
      idt->freeEntries--;
      return 0;
    }
  } /* for */
  return -1;
}



static inline int GWEN_PointerListTable_AppendPtr(GWEN_POINTERLIST_TABLE *idt, void *ptr) {
  if (idt->freeEntries) {
    unsigned int i;

    i=GWEN_POINTERLIST_TABLE_MAXENTRIES-idt->freeEntries;
    idt->entries[i]=ptr;
    idt->freeEntries--;
    return 0;
  }
  else
    return -1;
}



static inline int GWEN_PointerListTable_HasPtr(const GWEN_POINTERLIST_TABLE *idt, void *ptr) {
  unsigned int i;

  for (i=0; i<GWEN_POINTERLIST_TABLE_MAXENTRIES; i++) {
    if (idt->entries[i]==ptr) {
      return 1;
    }
  } /* for */
  return 0;
}



static inline int GWEN_PointerListTable_DelPtr(GWEN_POINTERLIST_TABLE *idt, void *ptr) {
  unsigned int i;

  for (i=0; i<GWEN_POINTERLIST_TABLE_MAXENTRIES; i++) {
    if (idt->entries[i]==ptr) {
      idt->entries[i]=0;
      idt->freeEntries++;
      return 0;
    }
  } /* for */
  return -1;
}



static inline int GWEN_PointerListTable_IsEmpty(const GWEN_POINTERLIST_TABLE *idt) {
  return GWEN_POINTERLIST_TABLE_MAXENTRIES==idt->freeEntries;
}



static inline int GWEN_PointerListTable_IsFull(const GWEN_POINTERLIST_TABLE *idt) {
  return idt->freeEntries==0;
}



static inline unsigned int GWEN_PointerListTable_GetCount(const GWEN_POINTERLIST_TABLE *idt) {
  return GWEN_POINTERLIST_TABLE_MAXENTRIES-idt->freeEntries;
}



static inline void *GWEN_PointerListTable_GetFirstPtr(const GWEN_POINTERLIST_TABLE *idt, uint64_t *tabIdx) {
  unsigned int i;

  for (i=0; i<GWEN_POINTERLIST_TABLE_MAXENTRIES; i++) {
    if (idt->entries[i]!=0) {
      *tabIdx=i;
      return idt->entries[i];
    }
  } /* for */
  return NULL;
}



static inline void *GWEN_PointerListTable_GetNextPtr(const GWEN_POINTERLIST_TABLE *idt, uint64_t *tabIdx) {
  unsigned int i;

  for (i=(*tabIdx)+1; i<GWEN_POINTERLIST_TABLE_MAXENTRIES; i++) {
    if (idt->entries[i]!=0) {
      *tabIdx=i;
      return idt->entries[i];
    }
  } /* for */
  return NULL;
}






GWEN_POINTERLIST *GWEN_PointerList_new(void) {
  GWEN_POINTERLIST *idl;

  GWEN_NEW_OBJECT(GWEN_POINTERLIST, idl);
  idl->refCount=1;
  idl->tableStep=GWEN_POINTERLIST_DEFAULT_STEP;
  return idl;
}



void GWEN_PointerList_Attach(GWEN_POINTERLIST *idl) {
  assert(idl);
  assert(idl->refCount);
  idl->refCount++;
}



void GWEN_PointerList_free(GWEN_POINTERLIST *idl) {
  if (idl) {
    assert(idl->refCount);
    if (idl->refCount==1) {
      GWEN_PointerList_Clear(idl);
      idl->refCount=0;
      GWEN_FREE_OBJECT(idl);
    }
    else
      idl->refCount--;
  }
}



void GWEN_PointerList_AddTable(GWEN_POINTERLIST *idl, GWEN_POINTERLIST_TABLE *idt) {
  GWEN_POINTERLIST_TABLE **tablePtr;
  int idx;

  assert(idl);

  tablePtr=idl->pIdTablePointers;
  for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
    if (*tablePtr==NULL)
      break;
  } /* while */

  if (idx>=idl->idTableCount) {
    uint32_t newCount;
    GWEN_POINTERLIST_TABLE **newPtr;

    /* resize */
    newCount=idl->idTableCount+idl->tableStep;
    newPtr=(GWEN_POINTERLIST_TABLE **)realloc(idl->pIdTablePointers, sizeof(GWEN_POINTERLIST_TABLE*)*newCount);
    assert(newPtr);
    /* init new pointers */
    memset((void*)(newPtr+idl->idTableCount),
           0,
           sizeof(GWEN_POINTERLIST_TABLE*)*(newCount-idl->idTableCount));
    idl->pIdTablePointers=newPtr;
    idl->pIdTablePointers[idl->idTableCount]=idt;
    idl->lastTableIdx=idl->idTableCount;
    idl->idTableCount=newCount;
  }
  else {
    idl->pIdTablePointers[idx]=idt;
    idl->lastTableIdx=idx;
  }
}



int GWEN_PointerList_AddPtr(GWEN_POINTERLIST *idl, void *ptr) {
  GWEN_POINTERLIST_TABLE *idt=NULL;
  GWEN_POINTERLIST_TABLE **tablePtr;
  int idx;

  assert(idl);

  if (idl->pIdTablePointers==NULL) {
    /* create an initial pointer table which can take up to tableStep pointers */
    idl->pIdTablePointers=(GWEN_POINTERLIST_TABLE **) malloc(sizeof(GWEN_POINTERLIST_TABLE*)*(idl->tableStep));
    assert(idl->pIdTablePointers);
    memset(idl->pIdTablePointers, 0, sizeof(GWEN_POINTERLIST*)*(idl->tableStep));
    idl->idTableCount=idl->tableStep;
  }

  for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
    idt=*tablePtr;
    if (idt && !GWEN_PointerListTable_IsFull(idt))
      break;
  } /* while */

  if (idx>=idl->idTableCount) {
    idt=GWEN_PointerListTable_new();
    GWEN_PointerList_AddTable(idl, idt);
  }

  GWEN_PointerListTable_AddPtr(idt, ptr);
  idl->entryCount++;
  return 0;
}



int GWEN_PointerList_DelPtr(GWEN_POINTERLIST *idl, void *ptr) {
  if (idl->pIdTablePointers) {
    GWEN_POINTERLIST_TABLE *idt=NULL;
    GWEN_POINTERLIST_TABLE **tablePtr;
    int idx;

    for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
      idt=*tablePtr;
      if (idt && !GWEN_PointerListTable_DelPtr(idt, ptr)) {
        /* found a table which had this id */
        GWEN_PointerList_Clean(idl);
        idl->entryCount--;
        return 0;
      }
    }
  }

  return -1;
}



int GWEN_PointerList_HasPtr(const GWEN_POINTERLIST *idl, void *ptr) {
  if (idl->pIdTablePointers) {
    GWEN_POINTERLIST_TABLE *idt=NULL;
    GWEN_POINTERLIST_TABLE **tablePtr;
    int idx;

    for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
      idt=*tablePtr;
      if (idt && GWEN_PointerListTable_HasPtr(idt, ptr))
        return 1;
    }
  }

  return 0;
}



void GWEN_PointerList_Clean(GWEN_POINTERLIST *idl) {
  GWEN_POINTERLIST_TABLE *idt=NULL;
  GWEN_POINTERLIST_TABLE **tablePtr;
  int idx;

  for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
    idt=*tablePtr;
    if (idt && GWEN_PointerListTable_IsEmpty(idt)) {
      GWEN_PointerListTable_free(idt);
      *tablePtr=NULL;
    }
  }
}



void GWEN_PointerList_Clear(GWEN_POINTERLIST *idl) {
  if (idl->pIdTablePointers) {
    GWEN_POINTERLIST_TABLE *idt=NULL;
    GWEN_POINTERLIST_TABLE **tablePtr;
    int idx;

    for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
      idt=*tablePtr;
      if (idt) {
        GWEN_PointerListTable_free(idt);
        *tablePtr=NULL;
      }
    }
    free(idl->pIdTablePointers);
    idl->pIdTablePointers=NULL;
  }
  idl->entryCount=0;
  idl->nextIdx=0;
}



GWEN_POINTERLIST *GWEN_PointerList_dup(const GWEN_POINTERLIST *idl) {
  GWEN_POINTERLIST *nidl;
  int idx;

  nidl=GWEN_PointerList_new();
  nidl->tableStep=idl->tableStep;

  nidl->idTableCount=idl->idTableCount;
  nidl->entryCount=idl->entryCount;
  if (idl->pIdTablePointers) {
    for (idx=0; idx<idl->idTableCount; idx++) {
      GWEN_POINTERLIST_TABLE *idt;

      idt=idl->pIdTablePointers[idx];
      if (idt && !GWEN_PointerListTable_IsEmpty(idt)) {
        GWEN_POINTERLIST_TABLE *nidt;

        nidt=GWEN_PointerListTable_new();
        memmove(nidt->entries, idt->entries, GWEN_POINTERLIST_TABLE_MAXENTRIES*sizeof(void*));
        nidt->freeEntries=idt->freeEntries;
        GWEN_PointerList_AddTable(nidl, nidt);
      }
    }
  }

  return nidl;
}



uint64_t GWEN_PointerList_GetEntryCount(const GWEN_POINTERLIST *idl) {
  assert(idl);
  assert(idl->refCount);

  return idl->entryCount;
}



void *GWEN_PointerList_GetFirstPtr(const GWEN_POINTERLIST *idl, uint64_t *pos) {
  GWEN_POINTERLIST_TABLE *idt=NULL;
  GWEN_POINTERLIST_TABLE **tablePtr;
  int idx;
  int idIndex=0;

  *pos=0;
  for (idx=0, tablePtr=idl->pIdTablePointers; idx<idl->idTableCount; idx++, tablePtr++) {
    idt=*tablePtr;
    if (idt && !GWEN_PointerListTable_IsEmpty(idt)) {
      int i;
      void *ptr;

      for (i=0; i<GWEN_POINTERLIST_TABLE_MAXENTRIES; i++) {
        if (idt->entries[i]!=0) {
          ptr=idt->entries[i];
          *pos=idIndex+i+1;
          return ptr;
        }
      }
    }
    idIndex+=GWEN_POINTERLIST_TABLE_MAXENTRIES;
  }

  return NULL;
}



void *GWEN_PointerList_GetNextPtr(const GWEN_POINTERLIST *idl, uint64_t *pos) {
  if (*pos) {
    GWEN_POINTERLIST_TABLE *idt;
    uint64_t tableNum=*pos / GWEN_POINTERLIST_TABLE_MAXENTRIES;
    uint64_t tableIdx=*pos % GWEN_POINTERLIST_TABLE_MAXENTRIES;
    GWEN_POINTERLIST_TABLE **tablePtr;
    int idIndex=0;
    int idx;

    if (tableNum>idl->idTableCount) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Table number out of range");
      *pos=0;
      return 0;
    }

    idIndex=(tableNum*GWEN_POINTERLIST_TABLE_MAXENTRIES);
    for (idx=tableNum, tablePtr=idl->pIdTablePointers+tableNum; idx<idl->idTableCount; idx++, tablePtr++) {
      idt=*tablePtr;
      if (idt && !GWEN_PointerListTable_IsEmpty(idt)) {
        int i;
        void *ptr;

        if (idx==tableNum) {
          for (i=tableIdx; i<GWEN_POINTERLIST_TABLE_MAXENTRIES; i++) {
            if (idt->entries[i]!=0) {
              ptr=idt->entries[i];
              *pos=idIndex+i+1;
              return ptr;
            }
          }
        }
        else {
          for (i=0; i<GWEN_POINTERLIST_TABLE_MAXENTRIES; i++) {
            if (idt->entries[i]!=0) {
              ptr=idt->entries[i];
              *pos=idIndex+i+1;
              return ptr;
            }
          }
        }
      }
      idIndex+=GWEN_POINTERLIST_TABLE_MAXENTRIES;
    }
    *pos=0;
  }

  return NULL;
}




GWEN_POINTERLIST_ITERATOR *GWEN_PointerList_Iterator_new(GWEN_POINTERLIST *idl) {
  GWEN_POINTERLIST_ITERATOR *it;

  assert(idl);
  GWEN_NEW_OBJECT(GWEN_POINTERLIST_ITERATOR, it);

  GWEN_PointerList_Attach(idl);
  it->list=idl;

  return it;
}



void GWEN_PointerList_Iterator_free(GWEN_POINTERLIST_ITERATOR *it) {
  if (it) {
    GWEN_PointerList_free(it->list);
    GWEN_FREE_OBJECT(it);
  }
}



void *GWEN_PointerList_Iterator_GetFirstId(GWEN_POINTERLIST_ITERATOR *it) {
  return GWEN_PointerList_GetFirstPtr(it->list, &(it->nextIndex));
}



void *GWEN_PointerList_Iterator_GetNextId(GWEN_POINTERLIST_ITERATOR *it) {
  return GWEN_PointerList_GetNextPtr(it->list, &(it->nextIndex));
}



int GWEN_PointerList_AppendPtr(GWEN_POINTERLIST *idl, void *ptr) {
  GWEN_POINTERLIST_TABLE *idt=NULL;

  assert(idl);

  if (idl->pIdTablePointers==NULL) {
    /* create an initial pointer table which can take up to tableStep pointers */
    idl->pIdTablePointers=(GWEN_POINTERLIST_TABLE **) malloc(sizeof(GWEN_POINTERLIST_TABLE*)*(idl->tableStep));
    assert(idl->pIdTablePointers);
    memset(idl->pIdTablePointers, 0, sizeof(GWEN_POINTERLIST_TABLE*)*(idl->tableStep));
    idl->idTableCount=idl->tableStep;
  }

  idt=idl->pIdTablePointers[idl->lastTableIdx];
  if (idt==NULL || GWEN_PointerListTable_IsFull(idt)) {
    idt=GWEN_PointerListTable_new();
    GWEN_PointerList_AddTable(idl, idt);
  }

  GWEN_PointerListTable_AppendPtr(idt, ptr);
  idl->entryCount++;
  return 0;
}



void *GWEN_PointerList_GetPtrAt(const GWEN_POINTERLIST *idl, uint64_t idx) {
  GWEN_POINTERLIST_TABLE *idt;
  uint64_t tableNum=idx / GWEN_POINTERLIST_TABLE_MAXENTRIES;
  uint64_t tableIdx=idx % GWEN_POINTERLIST_TABLE_MAXENTRIES;

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











