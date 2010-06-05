/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: idlist_p.h 1102 2006-12-30 19:39:37Z martin $
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

#ifndef GWENHYWFAR_IDLIST64_P_H
#define GWENHYWFAR_IDLIST64_P_H

#include <gwenhywfar/types.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/idlist64.h>


#define GWEN_IDTABLE64_MAXENTRIES 32
#define GWEN_IDLIST64_STEP 64


typedef struct GWEN_IDTABLE64 GWEN_IDTABLE64;

struct GWEN_IDTABLE64 {
  uint64_t freeEntries;
  uint64_t entries[GWEN_IDTABLE64_MAXENTRIES];
  uint64_t current;
  uint32_t refCount;
};

static GWEN_IDTABLE64 *GWEN_IdTable64_new();
static void GWEN_IdTable64_free(GWEN_IDTABLE64 *idt);
/*static void GWEN_IdTable64_Attach(GWEN_IDTABLE64 *idt);*/

static uint64_t GWEN_IdList64__GetFirstId(const GWEN_IDLIST64 *idl, uint64_t *pos);
static uint64_t GWEN_IdList64__GetNextId(const GWEN_IDLIST64 *idl, uint64_t *pos);

static void GWEN_IdList64_AddTable(GWEN_IDLIST64 *idl, GWEN_IDTABLE64 *idt);
static void GWEN_IdList64_Clean(GWEN_IDLIST64 *idl);

struct GWEN_IDLIST64 {
  uint32_t refCount;
  uint64_t entryCount;

  GWEN_IDTABLE64 **pIdTablePointers;
  uint32_t idTableCount;
  uint32_t lastTableIdx;
  uint64_t nextIdx;
};



struct GWEN_IDLIST64_ITERATOR {
  GWEN_IDLIST64 *list;
  uint64_t nextIndex;
};



#endif /* GWENHYWFAR_IDLIST64_P_H */

