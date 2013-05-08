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
};



GWEN_MULTICACHE_ENTRY *GWEN_MultiCache_Entry_new(GWEN_MULTICACHE_TYPE *ct, uint32_t id, void *p, uint32_t i);
void GWEN_MultiCache_Entry_free(GWEN_MULTICACHE_ENTRY *e);

uint32_t GWEN_MultiCache_Entry_GetId(const GWEN_MULTICACHE_ENTRY *e);
void GWEN_MultiCache_Entry_SetId(GWEN_MULTICACHE_ENTRY *e, uint32_t i);

uint32_t GWEN_MultiCache_Entry_GetDataSize(const GWEN_MULTICACHE_ENTRY *e);
void *GWEN_MultiCache_Entry_GetDataPtr(const GWEN_MULTICACHE_ENTRY *e);

GWEN_MULTICACHE_TYPE *GWEN_MultiCache_Entry_GetCacheType(const GWEN_MULTICACHE_ENTRY *e);




struct GWEN_MULTICACHE_TYPE {
  GWEN_LIST_ELEMENT(GWEN_MULTICACHE_TYPE);
  GWEN_MULTICACHE *multiCache;
  GWEN_MULTICACHE_ENTRY_IDMAP *entryMap;

  GWEN_MULTICACHE_TYPE_ATTACH_FN attachFn;
  GWEN_MULTICACHE_TYPE_FREE_FN freeFn;
};

int GWEN_MultiCache_Type_AttachData(const GWEN_MULTICACHE_TYPE *ct, void *p);
int GWEN_MultiCache_Type_FreeData(const GWEN_MULTICACHE_TYPE *ct, void *p);

void GWEN_MultiCache_Type_ReleaseEntry(GWEN_MULTICACHE_TYPE *ct, GWEN_MULTICACHE_ENTRY *e);



struct GWEN_MULTICACHE {
  GWEN_MULTICACHE_TYPE_LIST *typeList;
  GWEN_MULTICACHE_ENTRY_LIST *entryList;
  uint64_t maxSize;
  uint64_t currentSize;
  uint64_t maxSizeUsed;
};


int GWEN_MultiCache_AddEntry(GWEN_MULTICACHE *mc, GWEN_MULTICACHE_ENTRY *e);
void GWEN_MultiCache_ReleaseEntry(GWEN_MULTICACHE *mc, GWEN_MULTICACHE_ENTRY *e);

void GWEN_MultiCache_ReleaseEntriesForType(GWEN_MULTICACHE *mc, GWEN_MULTICACHE_TYPE *ct);
void GWEN_MultiCache_UsingEntry(GWEN_MULTICACHE *mc, GWEN_MULTICACHE_ENTRY *e);



#endif

