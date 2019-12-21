/***************************************************************************
    begin       : Fri Dec 06 2019
    copyright   : (C) 2019 by Martin Preuss
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

#ifndef GWENHYWFAR_SIMPLEPTRLIST_P_H
#define GWENHYWFAR_SIMPLEPTRLIST_P_H

#include <gwenhywfar/simpleptrlist.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/misc.h>


#define GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE     0x80000000L


typedef struct INTERNAL_PTRLIST INTERNAL_PTRLIST;
struct INTERNAL_PTRLIST {
  int refCounter;
  uint64_t storedEntries;
  void *entries[0];
};



struct GWEN_SIMPLEPTRLIST {
  uint64_t maxEntries;
  uint64_t usedEntries;
  uint64_t steps;
  uint32_t flags;
  uint32_t refCount;
  INTERNAL_PTRLIST *entryList;
};



#endif /* GWENHYWFAR_SIMPLEPTRLIST_P_H */

