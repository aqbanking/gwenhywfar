/***************************************************************************
    begin       : Wed May 08 2013
    copyright   : (C) 2013 by Martin Preuss
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

#include "multicache_p.h"

#include <gwenhywfar/debug.h>



GWEN_LIST_FUNCTIONS(GWEN_MULTICACHE_ENTRY, GWEN_MultiCache_Entry);
GWEN_IDMAP_FUNCTIONS(GWEN_MULTICACHE_ENTRY, GWEN_MultiCache_Entry);
GWEN_LIST_FUNCTIONS(GWEN_MULTICACHE_TYPE, GWEN_MultiCache_Type);




GWEN_MULTICACHE_ENTRY *GWEN_MultiCache_Entry_new(GWEN_MULTICACHE_TYPE *ct, uint32_t id, void *p, uint32_t i)
{
  GWEN_MULTICACHE_ENTRY *e;

  GWEN_NEW_OBJECT(GWEN_MULTICACHE_ENTRY, e);
  GWEN_LIST_INIT(GWEN_MULTICACHE_ENTRY, e);

  e->cacheType=ct;
  e->id=id;
  e->dataPtr=p;
  e->dataSize=i;

  return e;
}



void GWEN_MultiCache_Entry_free(GWEN_MULTICACHE_ENTRY *e)
{
  if (e) {
    GWEN_LIST_FINI(GWEN_MULTICACHE_ENTRY, e);
    GWEN_FREE_OBJECT(e);
  }
}



uint32_t GWEN_MultiCache_Entry_GetId(const GWEN_MULTICACHE_ENTRY *e)
{
  assert(e);
  return e->id;
}



uint32_t GWEN_MultiCache_Entry_GetDataSize(const GWEN_MULTICACHE_ENTRY *e)
{
  assert(e);
  return e->dataSize;
}



void *GWEN_MultiCache_Entry_GetDataPtr(const GWEN_MULTICACHE_ENTRY *e)
{
  assert(e);
  return e->dataPtr;
}



GWEN_MULTICACHE_TYPE *GWEN_MultiCache_Entry_GetCacheType(const GWEN_MULTICACHE_ENTRY *e)
{
  assert(e);
  return e->cacheType;
}



uint32_t GWEN_MultiCache_Entry_GetParam1(const GWEN_MULTICACHE_ENTRY *e)
{
  assert(e);
  return e->param1;
}



void GWEN_MultiCache_Entry_SetParam1(GWEN_MULTICACHE_ENTRY *e, uint32_t i)
{
  assert(e);
  e->param1=i;
}



uint32_t GWEN_MultiCache_Entry_GetParam2(const GWEN_MULTICACHE_ENTRY *e)
{
  assert(e);
  return e->param2;
}



void GWEN_MultiCache_Entry_SetParam2(GWEN_MULTICACHE_ENTRY *e, uint32_t i)
{
  assert(e);
  e->param2=i;
}



uint32_t GWEN_MultiCache_Entry_GetParam3(const GWEN_MULTICACHE_ENTRY *e)
{
  assert(e);
  return e->param3;
}



void GWEN_MultiCache_Entry_SetParam3(GWEN_MULTICACHE_ENTRY *e, uint32_t i)
{
  assert(e);
  e->param3=i;
}



uint32_t GWEN_MultiCache_Entry_GetParam4(const GWEN_MULTICACHE_ENTRY *e)
{
  assert(e);
  return e->param4;
}



void GWEN_MultiCache_Entry_SetParam4(GWEN_MULTICACHE_ENTRY *e, uint32_t i)
{
  assert(e);
  e->param4=i;
}



double GWEN_MultiCache_Entry_GetParam5(const GWEN_MULTICACHE_ENTRY *e)
{
  assert(e);
  return e->param5;
}



void GWEN_MultiCache_Entry_SetParam5(GWEN_MULTICACHE_ENTRY *e, double d)
{
  assert(e);
  e->param5=d;
}









GWEN_MULTICACHE_TYPE *GWEN_MultiCache_Type_new(GWEN_MULTICACHE *mc)
{
  GWEN_MULTICACHE_TYPE *ct;

  GWEN_NEW_OBJECT(GWEN_MULTICACHE_TYPE, ct);
  GWEN_LIST_INIT(GWEN_MULTICACHE_TYPE, ct);
  ct->_refCount=1;
  ct->multiCache=mc;
  ct->entryMap=GWEN_MultiCache_Entry_IdMap_new(GWEN_IdMapAlgo_Hex4);

  return ct;
}



void GWEN_MultiCache_Type_free(GWEN_MULTICACHE_TYPE *ct)
{
  if (ct) {
    assert(ct->_refCount);
    if (ct->_refCount==1) {
      GWEN_MultiCache_ReleaseEntriesForType(ct->multiCache, ct);
      GWEN_MultiCache_Entry_IdMap_free(ct->entryMap);
      GWEN_LIST_FINI(GWEN_MULTICACHE_TYPE, ct);
      ct->_refCount=0;
      GWEN_FREE_OBJECT(ct);
    }
    else
      ct->_refCount--;
  }
}



void *GWEN_MultiCache_Type_GetData(const GWEN_MULTICACHE_TYPE *ct, uint32_t id)
{
  GWEN_MULTICACHE_ENTRY *e;

  assert(ct);
  assert(ct->_refCount);

  e=(GWEN_MULTICACHE_ENTRY *)GWEN_MultiCache_Entry_IdMap_Find(ct->entryMap, id);
  if (e) {
    void *p;

    GWEN_MultiCache_UsingEntry(ct->multiCache, e);
    p=GWEN_MultiCache_Entry_GetDataPtr(e);
    GWEN_MultiCache_Type_AttachData(ct, p);
    GWEN_MultiCache_IncCacheHits(ct->multiCache);
    return p;
  }
  GWEN_MultiCache_IncCacheMisses(ct->multiCache);
  return NULL;
}



void *GWEN_MultiCache_Type_GetDataWithParams(const GWEN_MULTICACHE_TYPE *ct, uint32_t id,
                                             uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4)
{

  GWEN_MULTICACHE_ENTRY *e;

  assert(ct);
  assert(ct->_refCount);

  e=(GWEN_MULTICACHE_ENTRY *)GWEN_MultiCache_Entry_IdMap_Find(ct->entryMap, id);
  if (e) {
    if ((GWEN_MultiCache_Entry_GetParam1(e)==param1) &&
        (GWEN_MultiCache_Entry_GetParam2(e)==param2) &&
        (GWEN_MultiCache_Entry_GetParam3(e)==param3) &&
        (GWEN_MultiCache_Entry_GetParam4(e)==param4)) {
      void *p;

      GWEN_MultiCache_UsingEntry(ct->multiCache, e);
      p=GWEN_MultiCache_Entry_GetDataPtr(e);
      GWEN_MultiCache_Type_AttachData(ct, p);
      GWEN_MultiCache_IncCacheHits(ct->multiCache);
      return p;
    }
  }
  GWEN_MultiCache_IncCacheMisses(ct->multiCache);
  return NULL;
}



void *GWEN_MultiCache_Type_GetDataWithParams5(const GWEN_MULTICACHE_TYPE *ct, uint32_t id,
                                              uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4,
                                              double param5)
{

  GWEN_MULTICACHE_ENTRY *e;

  assert(ct);
  assert(ct->_refCount);

  e=(GWEN_MULTICACHE_ENTRY *)GWEN_MultiCache_Entry_IdMap_Find(ct->entryMap, id);
  if (e) {
    if ((GWEN_MultiCache_Entry_GetParam1(e)==param1) &&
        (GWEN_MultiCache_Entry_GetParam2(e)==param2) &&
        (GWEN_MultiCache_Entry_GetParam3(e)==param3) &&
        (GWEN_MultiCache_Entry_GetParam4(e)==param4) &&
        (GWEN_MultiCache_Entry_GetParam5(e)==param5)) {
      void *p;

      GWEN_MultiCache_UsingEntry(ct->multiCache, e);
      p=GWEN_MultiCache_Entry_GetDataPtr(e);
      GWEN_MultiCache_Type_AttachData(ct, p);
      GWEN_MultiCache_IncCacheHits(ct->multiCache);
      return p;
    }
  }
  GWEN_MultiCache_IncCacheMisses(ct->multiCache);
  return NULL;
}



void GWEN_MultiCache_Type_SetData(GWEN_MULTICACHE_TYPE *ct, uint32_t id, void *ptr, uint32_t size)
{
  GWEN_MULTICACHE_ENTRY *e;

  assert(ct);
  assert(ct->_refCount);

  GWEN_MultiCache_Type_PurgeData(ct, id);
  e=GWEN_MultiCache_Entry_new(ct, id, ptr, size);
  GWEN_MultiCache_AddEntry(ct->multiCache, e);
  GWEN_MultiCache_Entry_IdMap_Insert(ct->entryMap, id, (void *) e);
}



void GWEN_MultiCache_Type_SetDataWithParams(GWEN_MULTICACHE_TYPE *ct, uint32_t id, void *ptr, uint32_t size,
                                            uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4)
{
  GWEN_MULTICACHE_ENTRY *e;

  assert(ct);
  assert(ct->_refCount);

  GWEN_MultiCache_Type_PurgeData(ct, id);

  e=GWEN_MultiCache_Entry_new(ct, id, ptr, size);
  GWEN_MultiCache_AddEntry(ct->multiCache, e);
  GWEN_MultiCache_Entry_SetParam1(e, param1);
  GWEN_MultiCache_Entry_SetParam2(e, param2);
  GWEN_MultiCache_Entry_SetParam3(e, param3);
  GWEN_MultiCache_Entry_SetParam4(e, param4);
  GWEN_MultiCache_Entry_IdMap_Insert(ct->entryMap, id, (void *) e);
}



void GWEN_MultiCache_Type_SetDataWithParams5(GWEN_MULTICACHE_TYPE *ct, uint32_t id, void *ptr, uint32_t size,
                                             uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4,
                                             double param5)
{
  GWEN_MULTICACHE_ENTRY *e;

  assert(ct);
  assert(ct->_refCount);

  GWEN_MultiCache_Type_PurgeData(ct, id);

  e=GWEN_MultiCache_Entry_new(ct, id, ptr, size);
  GWEN_MultiCache_AddEntry(ct->multiCache, e);
  GWEN_MultiCache_Entry_SetParam1(e, param1);
  GWEN_MultiCache_Entry_SetParam2(e, param2);
  GWEN_MultiCache_Entry_SetParam3(e, param3);
  GWEN_MultiCache_Entry_SetParam4(e, param4);
  GWEN_MultiCache_Entry_SetParam5(e, param5);
  GWEN_MultiCache_Entry_IdMap_Insert(ct->entryMap, id, (void *) e);
}



void GWEN_MultiCache_Type_PurgeData(GWEN_MULTICACHE_TYPE *ct, uint32_t id)
{
  GWEN_MULTICACHE_ENTRY *e;

  assert(ct);
  assert(ct->_refCount);

  e=(GWEN_MULTICACHE_ENTRY *)GWEN_MultiCache_Entry_IdMap_Find(ct->entryMap, id);
  if (e)
    GWEN_MultiCache_ReleaseEntry(ct->multiCache, e);
}



void GWEN_MultiCache_Type_PurgeAll(GWEN_MULTICACHE_TYPE *ct)
{
  assert(ct);
  assert(ct->_refCount);

  GWEN_MultiCache_ReleaseEntriesForType(ct->multiCache, ct);
}



void GWEN_MultiCache_Type_SetAttachFn(GWEN_MULTICACHE_TYPE *ct, GWEN_MULTICACHE_TYPE_ATTACH_FN fn)
{
  assert(ct);
  assert(ct->_refCount);

  ct->attachFn=fn;
}



void GWEN_MultiCache_Type_SetFreeFn(GWEN_MULTICACHE_TYPE *ct, GWEN_MULTICACHE_TYPE_FREE_FN fn)
{
  assert(ct);
  assert(ct->_refCount);

  ct->freeFn=fn;
}



void GWEN_MultiCache_Type_SetAttachObjectFn(GWEN_MULTICACHE_TYPE *ct, GWEN_MULTICACHE_TYPE_ATTACH_OBJECT_FN fn)
{
  assert(ct);
  assert(ct->_refCount);

  ct->attachObjectFn=fn;
}



void GWEN_MultiCache_Type_SetFreeObjectFn(GWEN_MULTICACHE_TYPE *ct, GWEN_MULTICACHE_TYPE_FREE_OBJECT_FN fn)
{
  assert(ct);
  assert(ct->_refCount);

  ct->freeObjectFn=fn;
}



int GWEN_MultiCache_Type_AttachData(const GWEN_MULTICACHE_TYPE *ct, void *p)
{
  assert(ct);
  assert(ct->_refCount);

  /* try attachObjectFn first, because that has THIS object as first argument */
  if (ct->attachObjectFn)
    return ct->attachObjectFn(ct, p);

  if (ct->attachFn)
    return ct->attachFn(p);

  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_MultiCache_Type_FreeData(const GWEN_MULTICACHE_TYPE *ct, void *p)
{
  assert(ct);
  assert(ct->_refCount);

  /* try freeObjectFn first, because that has THIS object as first argument */
  if (ct->freeObjectFn)
    return ct->freeObjectFn(ct, p);

  if (ct->freeFn)
    return ct->freeFn(p);

  return GWEN_ERROR_NOT_IMPLEMENTED;
}


void GWEN_MultiCache_Type_ReleaseEntry(GWEN_MULTICACHE_TYPE *ct, GWEN_MULTICACHE_ENTRY *e)
{
  assert(ct);
  assert(ct->_refCount);

  assert(e);

  GWEN_MultiCache_Entry_IdMap_Remove(ct->entryMap, GWEN_MultiCache_Entry_GetId(e));
}







GWEN_MULTICACHE *GWEN_MultiCache_new(uint64_t maxSize)
{
  GWEN_MULTICACHE *mc;

  GWEN_NEW_OBJECT(GWEN_MULTICACHE, mc);
  mc->_refCount=1;
  mc->maxSize=maxSize;
  mc->typeList=GWEN_MultiCache_Type_List_new();
  mc->entryList=GWEN_MultiCache_Entry_List_new();

  return mc;
}



void GWEN_MultiCache_free(GWEN_MULTICACHE *mc)
{
  if (mc) {
    assert(mc->_refCount);

    if (mc->_refCount==1) {
      GWEN_MULTICACHE_ENTRY *ce;

      ce=GWEN_MultiCache_Entry_List_First(mc->entryList);
      while (ce) {
        GWEN_MultiCache_ReleaseEntry(mc, ce);
        ce=GWEN_MultiCache_Entry_List_First(mc->entryList);
      }

      GWEN_MultiCache_Entry_List_free(mc->entryList);
      GWEN_MultiCache_Type_List_free(mc->typeList);

      DBG_NOTICE(GWEN_LOGDOMAIN,
                 "MultiCache usage: "
                 "%" PRIu64 " hits, "
                 "%" PRIu64 " misses, "
                 "%" PRIu64 " drops, "
                 "%" PRIu64 " mb max memory used from "
                 "%" PRIu64 " mb "
                 "(%d %%)",
                 (uint64_t) mc->cacheHits,
                 (uint64_t) mc->cacheMisses,
                 (uint64_t) mc->cacheDrops,
                 (uint64_t)((mc->maxSizeUsed)/(1024*1024)),
                 (uint64_t)((mc->maxSize)/(1024*1024)),
                 (int)((mc->maxSizeUsed)*100.0/mc->maxSize));

      mc->_refCount=0;
      GWEN_FREE_OBJECT(mc);
    }
    else
      mc->_refCount--;
  }
}



uint64_t GWEN_MultiCache_GetMaxSizeUsed(const GWEN_MULTICACHE *mc)
{
  assert(mc);
  assert(mc->_refCount);
  return mc->maxSizeUsed;
}



int GWEN_MultiCache_AddEntry(GWEN_MULTICACHE *mc, GWEN_MULTICACHE_ENTRY *e)
{
  uint32_t esize;

  assert(mc);
  assert(mc->_refCount);
  assert(e);

  esize=GWEN_MultiCache_Entry_GetDataSize(e);
  if ((mc->currentSize+esize)>mc->maxSize) {
    int64_t nsize;

    /* make room */
    nsize=(mc->currentSize+esize)-mc->maxSize;
    while (nsize>0) {
      GWEN_MULTICACHE_ENTRY *ce;

      ce=GWEN_MultiCache_Entry_List_First(mc->entryList);
      if (ce) {
        nsize-=GWEN_MultiCache_Entry_GetDataSize(ce);
        GWEN_MultiCache_ReleaseEntry(mc, ce);
        mc->cacheDrops++;
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "No entry left to release, cache size limit too low");
        return GWEN_ERROR_MEMORY_FULL;
      }
    }
  }

  mc->currentSize+=esize;
  if (mc->currentSize>mc->maxSizeUsed)
    mc->maxSizeUsed=mc->currentSize;
  GWEN_MultiCache_Type_AttachData(GWEN_MultiCache_Entry_GetCacheType(e), GWEN_MultiCache_Entry_GetDataPtr(e));
  GWEN_MultiCache_Entry_List_Add(e, mc->entryList);
  return 0;
}



void GWEN_MultiCache_ReleaseEntry(GWEN_MULTICACHE *mc, GWEN_MULTICACHE_ENTRY *e)
{
  uint32_t esize;

  assert(mc);
  assert(mc->_refCount);
  assert(e);
  assert(e->cacheType);

  /* remove from list first */
  GWEN_MultiCache_Entry_List_Del(e);

  /* release from type's idmap */
  GWEN_MultiCache_Type_ReleaseEntry(GWEN_MultiCache_Entry_GetCacheType(e), e);

  /* release */
  esize=GWEN_MultiCache_Entry_GetDataSize(e);
  GWEN_MultiCache_Type_FreeData(e->cacheType, GWEN_MultiCache_Entry_GetDataPtr(e));
  GWEN_MultiCache_Entry_free(e);
  mc->currentSize-=esize;
}



void GWEN_MultiCache_ReleaseEntriesForType(GWEN_MULTICACHE *mc, GWEN_MULTICACHE_TYPE *ct)
{
  GWEN_MULTICACHE_ENTRY *ce;

  assert(mc);
  assert(mc->_refCount);
  assert(ct);


  ce=GWEN_MultiCache_Entry_List_First(mc->entryList);
  while (ce) {
    GWEN_MULTICACHE_ENTRY *ceNext;

    ceNext=GWEN_MultiCache_Entry_List_Next(ce);
    if (GWEN_MultiCache_Entry_GetCacheType(ce)==ct)
      GWEN_MultiCache_ReleaseEntry(mc, ce);
    ce=ceNext;
  }
}



void GWEN_MultiCache_UsingEntry(GWEN_MULTICACHE *mc, GWEN_MULTICACHE_ENTRY *e)
{
  assert(mc);
  assert(mc->_refCount);

  /* move cache entry to the end of the list */
  GWEN_MultiCache_Entry_List_Del(e);
  GWEN_MultiCache_Entry_List_Add(e, mc->entryList);
}



void GWEN_MultiCache_IncCacheHits(GWEN_MULTICACHE *mc)
{
  assert(mc);
  assert(mc->_refCount);
  mc->cacheHits++;
}



void GWEN_MultiCache_IncCacheMisses(GWEN_MULTICACHE *mc)
{
  assert(mc);
  assert(mc->_refCount);
  mc->cacheMisses++;
}




