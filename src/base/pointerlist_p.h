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

#ifndef GWENHYWFAR_POINTERLIST_P_H
#define GWENHYWFAR_POINTERLIST_P_H

#include <gwenhywfar/types.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pointerlist.h>


#define GWEN_POINTERLIST_TABLE_MAXENTRIES 64
#define GWEN_POINTERLIST_DEFAULT_STEP     64


typedef struct GWEN_POINTERLIST_TABLE GWEN_POINTERLIST_TABLE;

struct GWEN_POINTERLIST_TABLE {
  uint64_t freeEntries;
  void* entries[GWEN_POINTERLIST_TABLE_MAXENTRIES];
  uint32_t refCount;
};

static GWEN_POINTERLIST_TABLE *GWEN_PointerListTable_new();
static void GWEN_PointerListTable_free(GWEN_POINTERLIST_TABLE *idt);

static void GWEN_PointerList_AddTable(GWEN_POINTERLIST *idl, GWEN_POINTERLIST_TABLE *idt);
static void GWEN_PointerList_Clean(GWEN_POINTERLIST *idl);


struct GWEN_POINTERLIST {
  uint32_t refCount;
  uint64_t entryCount;

  GWEN_POINTERLIST_TABLE **pIdTablePointers;
  uint32_t idTableCount;
  uint32_t lastTableIdx;
  uint64_t nextIdx;

  uint32_t tableStep;
};



struct GWEN_POINTERLIST_ITERATOR {
  GWEN_POINTERLIST *list;
  uint64_t nextIndex;
};



#endif /* GWENHYWFAR_POINTERLIST_P_H */

