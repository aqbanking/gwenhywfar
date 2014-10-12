/***************************************************************************
    begin       : Mon Jul 14 2008
    copyright   : (C) 2008 by Martin Preuss
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

#ifndef GWENHYWFAR_MEMCACHE_P_H
#define GWENHYWFAR_MEMCACHE_P_H


#include <gwenhywfar/memcache.h>
#include <gwenhywfar/mutex.h>
#include <gwenhywfar/idmap.h>



GWEN_IDMAP_FUNCTION_DEFS(GWEN_MEMCACHE_ENTRY, GWEN_MemCacheEntry)


struct GWEN_MEMCACHE_ENTRY {
  GWEN_MEMCACHE *memCache;
  uint32_t id;
  int useCounter;
  time_t unusedSince;
  void *dataPtr;
  size_t dataLen;
  int isValid;
};



GWEN_MEMCACHE_ENTRY *GWEN_MemCacheEntry_new(GWEN_MEMCACHE *memCache,
    uint32_t id,
    void *dataPtr,
    size_t dataLen);

int GWEN_MemCacheEntry_GetUseCounter(const GWEN_MEMCACHE_ENTRY *me);
time_t GWEN_MemCacheEntry_GetUnusedSince(GWEN_MEMCACHE_ENTRY *me);


void GWEN_MemCacheEntry_SetIsValid(GWEN_MEMCACHE_ENTRY *me, int b);



struct GWEN_MEMCACHE {
  GWEN_MEMCACHE_ENTRY_IDMAP *idMap;

  size_t maxCacheMemory;
  uint32_t maxCacheEntries;

  size_t currentCacheMemory;
  uint32_t currentCacheEntries;

  GWEN_MUTEX *mutex;
};


int GWEN_MemCache_Lock(GWEN_MEMCACHE *mc);
int GWEN_MemCache_Unlock(GWEN_MEMCACHE *mc);

int GWEN_MemCache__MakeRoom(GWEN_MEMCACHE *mc,
                            size_t neededSize);

#endif
