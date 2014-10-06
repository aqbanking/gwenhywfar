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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "memcache_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



GWEN_IDMAP_FUNCTIONS(GWEN_MEMCACHE_ENTRY, GWEN_MemCacheEntry)



GWEN_MEMCACHE_ENTRY *GWEN_MemCacheEntry_new(GWEN_MEMCACHE *memCache,
    uint32_t id,
    void *dataPtr,
    size_t dataLen) {
  GWEN_MEMCACHE_ENTRY *me;

  GWEN_NEW_OBJECT(GWEN_MEMCACHE_ENTRY, me);

  me->memCache=memCache;
  me->id=id;
  me->dataPtr=dataPtr;
  me->dataLen=dataLen;
  me->isValid=1;

  /* update memcache */
  me->memCache->currentCacheEntries++;
  me->memCache->currentCacheMemory+=me->dataLen;

  return me;
}



void GWEN_MemCacheEntry_free(GWEN_MEMCACHE_ENTRY *me) {
  if (me) {
    assert(me->useCounter==0);
    assert(me->memCache);

    /* update memcache */
    me->memCache->currentCacheEntries--;
    me->memCache->currentCacheMemory-=me->dataLen;

    if (me->dataPtr && me->dataLen)
      free(me->dataPtr);

    GWEN_FREE_OBJECT(me);
  }
}



int GWEN_MemCacheEntry_GetUseCounter(const GWEN_MEMCACHE_ENTRY *me) {
  assert(me);
  return me->useCounter;
}



time_t GWEN_MemCacheEntry_GetUnusedSince(GWEN_MEMCACHE_ENTRY *me) {
  assert(me);
  return me->unusedSince;
}



uint32_t GWEN_MemCacheEntry_GetId(GWEN_MEMCACHE_ENTRY *me) {
  assert(me);
  return me->id;
}



void *GWEN_MemCacheEntry_GetDataPtr(GWEN_MEMCACHE_ENTRY *me) {
  assert(me);
  return me->dataPtr;
}



size_t GWEN_MemCacheEntry_GetDataLen(GWEN_MEMCACHE_ENTRY *me) {
  assert(me);
  return me->dataLen;
}



void GWEN_MemCacheEntry_BeginUse(GWEN_MEMCACHE_ENTRY *me) {
  int rv;

  assert(me);
  rv=GWEN_MemCache_Lock(me->memCache);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    assert(0);
  }
  me->useCounter++;
  GWEN_MemCache_Unlock(me->memCache);
}



void GWEN_MemCacheEntry_EndUse(GWEN_MEMCACHE_ENTRY *me) {
  int rv;

  assert(me);
  rv=GWEN_MemCache_Lock(me->memCache);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    assert(0);
  }
  if (me->useCounter>0) {
    me->useCounter--;
    if (me->useCounter==0) {
      if (!(me->isValid)) {
        GWEN_MemCacheEntry_free(me);
      }
      else
        me->unusedSince=time(0);
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Use counter < 1, aborting");
    GWEN_MemCache_Unlock(me->memCache);
    assert(me->useCounter>0);
  }
  GWEN_MemCache_Unlock(me->memCache);
}







GWEN_MEMCACHE *GWEN_MemCache_new(size_t maxCacheMemory,
                                 uint32_t maxCacheEntries) {
  GWEN_MEMCACHE *mc;

  GWEN_NEW_OBJECT(GWEN_MEMCACHE, mc);
  mc->mutex=GWEN_Mutex_new();
  mc->idMap=GWEN_MemCacheEntry_IdMap_new(GWEN_IdMapAlgo_Hex4);
  mc->maxCacheMemory=maxCacheMemory;
  mc->maxCacheEntries=maxCacheEntries;

  return mc;
}



void GWEN_MemCache_free(GWEN_MEMCACHE *mc) {
  if (mc) {
    GWEN_MemCacheEntry_IdMap_free(mc->idMap);
    GWEN_Mutex_free(mc->mutex);
    GWEN_FREE_OBJECT(mc);
  }
}



GWEN_MEMCACHE_ENTRY *GWEN_MemCache_FindEntry(GWEN_MEMCACHE *mc,
    uint32_t id) {
  GWEN_MEMCACHE_ENTRY *me;

  assert(mc);
  GWEN_MemCache_Lock(mc);
  me=GWEN_MemCacheEntry_IdMap_Find(mc->idMap, id);
  if (me) {
    /* we can't call GWEN_MemCache_BeginUse() here because of the mutex */
    me->useCounter++;
  }
  GWEN_MemCache_Unlock(mc);

  return me;
}



void GWEN_MemCache_PurgeEntry(GWEN_MEMCACHE *mc,
                              uint32_t id) {
  GWEN_MEMCACHE_ENTRY *me;

  assert(mc);
  GWEN_MemCache_Lock(mc);
  me=GWEN_MemCacheEntry_IdMap_Find(mc->idMap, id);
  if (me) {
    me->isValid=0;
    GWEN_MemCacheEntry_IdMap_Remove(mc->idMap, id);
    if (me->useCounter==0)
      GWEN_MemCacheEntry_free(me);
  }
  GWEN_MemCache_Unlock(mc);
}



int GWEN_MemCache__MakeRoom(GWEN_MEMCACHE *mc,
                            size_t neededSize) {
  assert(mc);

  /* release unused entries until there is enough memory */
  while(neededSize) {
    GWEN_MEMCACHE_ENTRY *oldestEntry;
    GWEN_IDMAP_RESULT res;
    uint32_t currentId;

    /* get oldest entry */
    oldestEntry=NULL;
    res=GWEN_MemCacheEntry_IdMap_GetFirst(mc->idMap, &currentId);
    while(res==GWEN_IdMapResult_Ok) {
      GWEN_MEMCACHE_ENTRY *me;

      me=GWEN_MemCacheEntry_IdMap_Find(mc->idMap, currentId);
      if (me) {
        if (me->isValid && me->useCounter==0) {
          if (oldestEntry==NULL)
            oldestEntry=me;
          else {
            if (me->unusedSince<oldestEntry->unusedSince)
              oldestEntry=me;
          }
        }
      }
      res=GWEN_MemCacheEntry_IdMap_GetNext(mc->idMap, &currentId);
    }

    if (oldestEntry==NULL)
      /* no unused entry found */
      break;

    /* subtract size of to-be-removed entry from needed size */
    if (neededSize<oldestEntry->dataLen)
      neededSize=0;
    else
      neededSize-=oldestEntry->dataLen;

    /* remove oldest entry (it is unused, so we also delete it here) */
    GWEN_MemCacheEntry_IdMap_Remove(mc->idMap, oldestEntry->id);
    GWEN_MemCacheEntry_free(oldestEntry);
  }

  return (neededSize==0)?0:GWEN_ERROR_MEMORY_FULL;
}



GWEN_MEMCACHE_ENTRY *GWEN_MemCache_CreateEntry(GWEN_MEMCACHE *mc,
    uint32_t id,
    void *dataPtr,
    size_t dataLen) {
  GWEN_MEMCACHE_ENTRY *me;

  assert(mc);
  GWEN_MemCache_Lock(mc);

  /* invalidate possibly existing entry in any case */
  me=GWEN_MemCacheEntry_IdMap_Find(mc->idMap, id);
  if (me) {
    me->isValid=0;
    GWEN_MemCacheEntry_IdMap_Remove(mc->idMap, id);
    if (me->useCounter==0)
      GWEN_MemCacheEntry_free(me);
  }

  /* check for limits: entry count */
  if (mc->currentCacheEntries>=mc->maxCacheEntries) {
    int rv;

    /* release unused entries (at least 1 byte) */
    rv=GWEN_MemCache__MakeRoom(mc, 1);
    if (rv) {
      DBG_WARN(GWEN_LOGDOMAIN, "Too many entries in use");
      GWEN_MemCache_Unlock(mc);
      return NULL;
    }
  }

  /* check for limits: memory in use */
  if ((mc->currentCacheMemory+dataLen)>=mc->maxCacheMemory) {
    size_t diff;
    int rv;

    diff=(mc->currentCacheMemory+dataLen)-mc->maxCacheMemory;
    /* release unused entries */
    rv=GWEN_MemCache__MakeRoom(mc, diff);
    if (rv) {
      DBG_WARN(GWEN_LOGDOMAIN, "Too much memory in use");
      GWEN_MemCache_Unlock(mc);
      return NULL;
    }
  }

  /* create new entry */
  me=GWEN_MemCacheEntry_new(mc, id, dataPtr, dataLen);
  assert(me);
  me->useCounter++;
  GWEN_MemCacheEntry_IdMap_Insert(mc->idMap, id, me);

  GWEN_MemCache_Unlock(mc);

  return me;
}



void GWEN_MemCache_PurgeEntries(GWEN_MEMCACHE *mc,
                                uint32_t id, uint32_t mask) {
  GWEN_IDMAP_RESULT res;
  uint32_t currentId;

  assert(mc);
  GWEN_MemCache_Lock(mc);

  res=GWEN_MemCacheEntry_IdMap_GetFirst(mc->idMap, &currentId);
  while(res==GWEN_IdMapResult_Ok) {
    uint32_t nextId;

    nextId=currentId;
    res=GWEN_MemCacheEntry_IdMap_GetNext(mc->idMap, &nextId);
    if ((currentId & mask)==id) {
      GWEN_MEMCACHE_ENTRY *me;

      me=GWEN_MemCacheEntry_IdMap_Find(mc->idMap, currentId);
      if (me) {
        me->isValid=0;
        GWEN_MemCacheEntry_IdMap_Remove(mc->idMap, currentId);
        if (me->useCounter==0)
          GWEN_MemCacheEntry_free(me);
      }

    }
    currentId=nextId;
  }

  GWEN_MemCache_Unlock(mc);
}



void GWEN_MemCache_Purge(GWEN_MEMCACHE *mc) {
  assert(mc);
  GWEN_MemCache_PurgeEntries(mc, 0, 0);
}



int GWEN_MemCache_Lock(GWEN_MEMCACHE *mc) {
  assert(mc);
  return GWEN_Mutex_Lock(mc->mutex);
}



int GWEN_MemCache_Unlock(GWEN_MEMCACHE *mc) {
  assert(mc);
  return GWEN_Mutex_Unlock(mc->mutex);
}








