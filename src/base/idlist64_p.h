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

#ifndef GWENHYWFAR_IDLIST64_P_H
#define GWENHYWFAR_IDLIST64_P_H

#include <gwenhywfar/types.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/idlist64.h>

#define GWEN_IDLIST64_INITIAL_ENTRYCOUNT 64
#define GWEN_IDLIST64_STEPS              64
#define GWEN_IDLIST64_ENTRIES_PER_TABLE  256

#define GWEN_IDTABLE64_RUNTIME_FLAGS_DIRTY  0x01
#define GWEN_IDTABLE64_RUNTIME_FLAGS_ISCOPY 0x02




typedef struct GWEN_IDTABLE64 GWEN_IDTABLE64;

struct GWEN_IDTABLE64 {
  uint64_t maxEntries;
  uint64_t freeEntries;
  uint64_t highestEntry;
  uint64_t *ptrEntries;

  uint32_t refCount;
  uint32_t runtimeFlags;
};





struct GWEN_IDLIST64_ITERATOR {
  const GWEN_IDLIST64 *list;
  uint64_t nextIndex;
};



#endif /* GWENHYWFAR_IDLIST64_P_H */

