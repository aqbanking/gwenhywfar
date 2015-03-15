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


#ifndef GWENHYWFAR_MULTICACHE_P_H
#define GWENHYWFAR_MULTICACHE_P_H


#include <gwenhywfar/multicache.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/idmap.h>



typedef struct GWEN_MULTICACHE_ENTRY GWEN_MULTICACHE_ENTRY;
GWEN_LIST_FUNCTION_DEFS(GWEN_MULTICACHE_ENTRY, GWEN_MultiCache_Entry);
GWEN_IDMAP_FUNCTION_DEFS(GWEN_MULTICACHE_ENTRY, GWEN_MultiCache_Entry);

GWEN_LIST_FUNCTION_DEFS(GWEN_MULTICACHE_TYPE, GWEN_MultiCache_Type);


struct GWEN_MULTICACHE_ENTRY {
  GWEN_LIST_ELEMENT(GWEN_MULTICACHE_ENTRY);
  GWEN_MULTICACHE_TYPE *cacheType;
  uint32_t id;
  uint32_t dataSize;
  void *dataPtr;

  uint32_t param1;
  uint32_t param2;
  uint32_t param3;
  uint32_t param4;
  double param5;
};



static GWEN_MULTICACHE_ENTRY *GWEN_MultiCache_Entry_new(GWEN_MULTICACHE_TYPE *ct, uint32_t id, void *p, uint32_t i);
static void GWEN_MultiCache_Entry_free(GWEN_MULTICACHE_ENTRY *e);

static uint32_t GWEN_MultiCache_Entry_GetId(const GWEN_MULTICACHE_ENTRY *e);

static uint32_t GWEN_MultiCache_Entry_GetDataSize(const GWEN_MULTICACHE_ENTRY *e);
static void *GWEN_MultiCache_Entry_GetDataPtr(const GWEN_MULTICACHE_ENTRY *e);

static GWEN_MULTICACHE_TYPE *GWEN_MultiCache_Entry_GetCacheType(const GWEN_MULTICACHE_ENTRY *e);


static uint32_t GWEN_MultiCache_Entry_GetParam1(const GWEN_MULTICACHE_ENTRY *e);
static void GWEN_MultiCache_Entry_SetParam1(GWEN_MULTICACHE_ENTRY *e, uint32_t i);

static uint32_t GWEN_MultiCache_Entry_GetParam2(const GWEN_MULTICACHE_ENTRY *e);
static void GWEN_MultiCache_Entry_SetParam2(GWEN_MULTICACHE_ENTRY *e, uint32_t i);

static uint32_t GWEN_MultiCache_Entry_GetParam3(const GWEN_MULTICACHE_ENTRY *e);
static void GWEN_MultiCache_Entry_SetParam3(GWEN_MULTICACHE_ENTRY *e, uint32_t i);

static uint32_t GWEN_MultiCache_Entry_GetParam4(const GWEN_MULTICACHE_ENTRY *e);
static void GWEN_MultiCache_Entry_SetParam4(GWEN_MULTICACHE_ENTRY *e, uint32_t i);

static double GWEN_MultiCache_Entry_GetParam5(const GWEN_MULTICACHE_ENTRY *e);
static void GWEN_MultiCache_Entry_SetParam5(GWEN_MULTICACHE_ENTRY *e, double d);



struct GWEN_MULTICACHE_TYPE {
  GWEN_LIST_ELEMENT(GWEN_MULTICACHE_TYPE);
  GWEN_MULTICACHE *multiCache;
  GWEN_MULTICACHE_ENTRY_IDMAP *entryMap;

  GWEN_MULTICACHE_TYPE_ATTACH_FN attachFn;
  GWEN_MULTICACHE_TYPE_FREE_FN freeFn;

  GWEN_MULTICACHE_TYPE_ATTACH_OBJECT_FN attachObjectFn;
  GWEN_MULTICACHE_TYPE_FREE_OBJECT_FN freeObjectFn;

  uint32_t _refCount;
};

static int GWEN_MultiCache_Type_AttachData(const GWEN_MULTICACHE_TYPE *ct, void *p);
static int GWEN_MultiCache_Type_FreeData(const GWEN_MULTICACHE_TYPE *ct, void *p);

static void GWEN_MultiCache_Type_ReleaseEntry(GWEN_MULTICACHE_TYPE *ct, GWEN_MULTICACHE_ENTRY *e);



struct GWEN_MULTICACHE {
  GWEN_MULTICACHE_TYPE_LIST *typeList;
  GWEN_MULTICACHE_ENTRY_LIST *entryList;
  uint64_t maxSize;
  uint64_t currentSize;
  uint64_t maxSizeUsed;
  uint64_t cacheHits;
  uint64_t cacheMisses;
  uint64_t cacheDrops;

  uint32_t _refCount;
};


static int GWEN_MultiCache_AddEntry(GWEN_MULTICACHE *mc, GWEN_MULTICACHE_ENTRY *e);
static void GWEN_MultiCache_ReleaseEntry(GWEN_MULTICACHE *mc, GWEN_MULTICACHE_ENTRY *e);

static void GWEN_MultiCache_ReleaseEntriesForType(GWEN_MULTICACHE *mc, GWEN_MULTICACHE_TYPE *ct);
static void GWEN_MultiCache_UsingEntry(GWEN_MULTICACHE *mc, GWEN_MULTICACHE_ENTRY *e);

static void GWEN_MultiCache_IncCacheHits(GWEN_MULTICACHE *mc);
static void GWEN_MultiCache_IncCacheMisses(GWEN_MULTICACHE *mc);


#endif

