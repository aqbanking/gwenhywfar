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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "simpleptrlist_p.h"
#include <gwenhywfar/debug.h>


#include <stdlib.h>
#include <assert.h>
#include <string.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void _attachToObject(GWEN_SIMPLEPTRLIST *pl, void *p);
static void _detachFromObject(GWEN_SIMPLEPTRLIST *pl, void *p);
static void _attachToAllObjects(GWEN_SIMPLEPTRLIST *pl);
static void _detachFromAllObjects(GWEN_SIMPLEPTRLIST *pl);

static INTERNAL_PTRLIST *_mallocPtrList(uint64_t totalEntries);
static void _attachToPtrList(INTERNAL_PTRLIST *entries);
static void _freePtrList(INTERNAL_PTRLIST *entries);
static INTERNAL_PTRLIST *_reallocPtrList(INTERNAL_PTRLIST *oldEntries, uint64_t totalEntries);
static INTERNAL_PTRLIST *_copyPtrList(const INTERNAL_PTRLIST *oldEntries, uint64_t totalEntries);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


GWEN_INHERIT_FUNCTIONS(GWEN_SIMPLEPTRLIST)




GWEN_SIMPLEPTRLIST *GWEN_SimplePtrList_new(uint64_t startEntries, uint64_t steps)
{
  GWEN_SIMPLEPTRLIST *pl;

  GWEN_NEW_OBJECT(GWEN_SIMPLEPTRLIST, pl);
  pl->refCount=1;
  GWEN_INHERIT_INIT(GWEN_SIMPLEPTRLIST, pl);

  pl->entryList=_mallocPtrList(startEntries);
  pl->maxEntries=startEntries;
  pl->steps=steps;
  pl->usedEntries=0;
  return pl;
}



GWEN_SIMPLEPTRLIST *GWEN_SimplePtrList_LazyCopy(GWEN_SIMPLEPTRLIST *oldList)
{
  GWEN_SIMPLEPTRLIST *pl;

  GWEN_NEW_OBJECT(GWEN_SIMPLEPTRLIST, pl);
  pl->refCount=1;
  GWEN_INHERIT_INIT(GWEN_SIMPLEPTRLIST, pl);

  pl->entryList=oldList->entryList;
  _attachToPtrList(pl->entryList);

  pl->maxEntries=oldList->maxEntries;
  pl->steps=oldList->steps;
  pl->usedEntries=oldList->usedEntries;
  pl->attachObjectFn=oldList->attachObjectFn;
  pl->freeObjectFn=oldList->freeObjectFn;
  pl->flags=oldList->flags | GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE;
  /* set copyOnWrite flag also on old list to keep lists separate even when changes to old lists are made */
  oldList->flags|=GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE;
  return pl;
}



void GWEN_SimplePtrList_Attach(GWEN_SIMPLEPTRLIST *pl)
{
  assert(pl);
  assert(pl->refCount);
  pl->refCount++;
}



void GWEN_SimplePtrList_free(GWEN_SIMPLEPTRLIST *pl)
{
  assert(pl);
  assert(pl->refCount);
  if (pl->refCount==1) {
    GWEN_INHERIT_FINI(GWEN_SIMPLEPTRLIST, pl);
    if (pl->flags & GWEN_SIMPLEPTRLIST_FLAGS_DETACHFROMOBJECTS)
      _detachFromAllObjects(pl);
    _freePtrList(pl->entryList);
    pl->entryList=NULL;
    pl->maxEntries=0;
    pl->refCount--;
    GWEN_FREE_OBJECT(pl);
  }
  else
    pl->refCount--;
}



void *GWEN_SimplePtrList_GetPtrAt(const GWEN_SIMPLEPTRLIST *pl, uint64_t idx)
{
  assert(pl);
  assert(pl->refCount);
  if (idx<pl->usedEntries) {
    return pl->entryList->entries[idx];
  }
  return NULL;
}



int GWEN_SimplePtrList_SetPtrAt(GWEN_SIMPLEPTRLIST *pl, uint64_t idx, void *p)
{
  assert(pl);
  assert(pl->refCount);

  if (idx<pl->usedEntries) {
    void *oldPtr;

    /* copy on write, if needed */
    if (pl->flags & GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE) {
      INTERNAL_PTRLIST *entryList;
      uint64_t num;

      num=pl->maxEntries+pl->steps;
      entryList=_copyPtrList(pl->entryList, num);
      if (entryList==NULL) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Memory full.");
        return GWEN_ERROR_MEMORY_FULL;
      }
      else {
        _freePtrList(pl->entryList);
        pl->entryList=entryList;
        pl->maxEntries=num;
        /* this is a copy, attach to objs */
        if (pl->flags & GWEN_SIMPLEPTRLIST_FLAGS_ATTACHTOOBJECTS)
          _attachToAllObjects(pl);
        /* clear copy-on-write flag */
        pl->flags&=~GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE;
      }
    }

    oldPtr=pl->entryList->entries[idx];
    pl->entryList->entries[idx]=p;
    if (p && (pl->flags & GWEN_SIMPLEPTRLIST_FLAGS_ATTACHTOOBJECTS))
      _attachToObject(pl, p);
    if (oldPtr && (pl->flags & GWEN_SIMPLEPTRLIST_FLAGS_DETACHFROMOBJECTS))
      _detachFromObject(pl, oldPtr);
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad index");
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }

  return 0;
}



int64_t GWEN_SimplePtrList_AddPtr(GWEN_SIMPLEPTRLIST *pl, void *p)
{
  assert(pl);
  assert(pl->refCount);

  if (pl->usedEntries >= pl->maxEntries || (pl->flags & GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE)) {
    uint64_t num;

    num=pl->maxEntries+pl->steps;
    if (num>pl->maxEntries) {
      INTERNAL_PTRLIST *entryList;

      if (pl->flags & GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE) {
	/* make new entries pointer a copy of the old one */
	entryList=_copyPtrList(pl->entryList, num);
	if (entryList==NULL) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Memory full.");
	  return GWEN_ERROR_MEMORY_FULL;
	}
	else {
          _freePtrList(pl->entryList);
	  pl->entryList=entryList;
          pl->maxEntries=num;
          /* this is a copy, attach to objs */
          if (pl->flags & GWEN_SIMPLEPTRLIST_FLAGS_ATTACHTOOBJECTS)
            _attachToAllObjects(pl);
          /* clear copy-on-write flag */
	  pl->flags&=~GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE;
	}
      }
      else {
	/* resize current list */
	entryList=_reallocPtrList(pl->entryList, num);
	if (entryList==NULL) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Memory full.");
	  return GWEN_ERROR_MEMORY_FULL;
	}
	else {
	  pl->entryList=entryList;
	  pl->maxEntries=num;
	}
      }
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Table full (step size==0).");
      return GWEN_ERROR_MEMORY_FULL;
    }
  }

  /* add entry */
  pl->entryList->entries[pl->usedEntries]=p;
  if (p && (pl->flags & GWEN_SIMPLEPTRLIST_FLAGS_ATTACHTOOBJECTS))
    _attachToObject(pl, p);
  pl->usedEntries++;
  return pl->usedEntries-1;
}



uint64_t GWEN_SimplePtrList_GetSteps(const GWEN_SIMPLEPTRLIST *pl)
{
  assert(pl);
  assert(pl->refCount);
  return pl->steps;
}



void GWEN_SimplePtrList_SetSteps(GWEN_SIMPLEPTRLIST *pl, uint64_t steps)
{
  assert(pl);
  assert(pl->refCount);
  pl->steps=steps;
}



uint64_t GWEN_SimplePtrList_GetMaxEntries(const GWEN_SIMPLEPTRLIST *pl)
{
  assert(pl);
  assert(pl->refCount);
  return pl->maxEntries;
}



uint64_t GWEN_SimplePtrList_GetUsedEntries(const GWEN_SIMPLEPTRLIST *pl)
{
  assert(pl);
  assert(pl->refCount);
  return pl->usedEntries;
}



void *GWEN_SimplePtrList_GetEntries(const GWEN_SIMPLEPTRLIST *pl)
{
  assert(pl);
  assert(pl->refCount);
  return pl->entryList->entries;
}



uint32_t GWEN_SimplePtrList_GetFlags(const GWEN_SIMPLEPTRLIST *pl)
{
  assert(pl);
  assert(pl->refCount);
  return pl->flags;
}



void GWEN_SimplePtrList_SetFlags(GWEN_SIMPLEPTRLIST *pl, uint32_t f)
{
  assert(pl);
  assert(pl->refCount);
  pl->flags=f;
}



void GWEN_SimplePtrList_AddFlags(GWEN_SIMPLEPTRLIST *pl, uint32_t f)
{
  assert(pl);
  assert(pl->refCount);
  pl->flags|=f;
}



void GWEN_SimplePtrList_SubFlags(GWEN_SIMPLEPTRLIST *pl, uint32_t f)
{
  assert(pl);
  assert(pl->refCount);
  pl->flags&=~f;
}



GWEN_SIMPLEPTRLIST_ATTACHOBJECT_FN GWEN_SimplePtrList_SetAttachObjectFn(GWEN_SIMPLEPTRLIST *pl, GWEN_SIMPLEPTRLIST_ATTACHOBJECT_FN fn)
{
  GWEN_SIMPLEPTRLIST_ATTACHOBJECT_FN oldFn;

  assert(pl);
  assert(pl->refCount);

  oldFn=pl->attachObjectFn;
  pl->attachObjectFn=fn;
  return oldFn;
}



GWEN_SIMPLEPTRLIST_FREEOBJECT_FN GWEN_SimplePtrList_SetFreeObjectFn(GWEN_SIMPLEPTRLIST *pl, GWEN_SIMPLEPTRLIST_FREEOBJECT_FN fn)
{
  GWEN_SIMPLEPTRLIST_FREEOBJECT_FN oldFn;

  assert(pl);
  assert(pl->refCount);

  oldFn=pl->freeObjectFn;
  pl->freeObjectFn=fn;
  return oldFn;
}



void _attachToObject(GWEN_SIMPLEPTRLIST *pl, void *p)
{
  if (pl->attachObjectFn)
    pl->attachObjectFn(pl, p);
}



void _detachFromObject(GWEN_SIMPLEPTRLIST *pl, void *p)
{
  if (pl->freeObjectFn)
    pl->freeObjectFn(pl, p);
}



void _attachToAllObjects(GWEN_SIMPLEPTRLIST *pl)
{
  if (pl->attachObjectFn) {
    uint64_t i;
    void **ptr;

    ptr=pl->entryList->entries;
    for (i=0; i<pl->usedEntries; i++) {
      if (*ptr!=NULL)
        pl->attachObjectFn(pl, *ptr);
      ptr++;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No attachObjectFn set");
  }
}



void _detachFromAllObjects(GWEN_SIMPLEPTRLIST *pl)
{
  if (pl->freeObjectFn) {
    uint64_t i;
    void **ptr;

    ptr=pl->entryList->entries;
    for (i=0; i<pl->usedEntries; i++) {
      if (*ptr!=NULL)
        pl->freeObjectFn(pl, *ptr);
      ptr++;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No attachObjectFn set");
  }
}






INTERNAL_PTRLIST *_mallocPtrList(uint64_t totalEntries)
{
  INTERNAL_PTRLIST *entries;

  entries=(INTERNAL_PTRLIST*) malloc(sizeof(INTERNAL_PTRLIST) + (totalEntries*sizeof(void*)));
  if (entries==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Memory full.");
    return NULL;
  }
  memset((void*)entries, 0, (totalEntries*sizeof(void*)));
  entries->refCounter=1;
  entries->storedEntries=totalEntries;
  return entries;
}



void _attachToPtrList(INTERNAL_PTRLIST *entries)
{
  assert(entries && entries->refCounter>0);
  if (entries && entries->refCounter>0) {
    entries->refCounter++;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Null pointer or already freed");
  }
}



void _freePtrList(INTERNAL_PTRLIST *entries)
{
  if (entries && entries->refCounter>0) {
    if (entries->refCounter==1) {
      entries->refCounter=0;
      free(entries);
    }
    else {
      entries->refCounter--;
    }
  }
}



INTERNAL_PTRLIST *_reallocPtrList(INTERNAL_PTRLIST *entries, uint64_t totalEntries)
{
  assert(entries && entries->refCounter>0);
  if (entries && entries->refCounter>0) {
    size_t newSize;
    uint64_t diffEntries;

    if (totalEntries<entries->storedEntries) {
      DBG_INFO(GWEN_LOGDOMAIN, "Will not decrease size (for now)");
      return entries;
    }

    diffEntries=totalEntries-(entries->storedEntries);
    newSize=sizeof(INTERNAL_PTRLIST)+totalEntries*sizeof(void*);

    entries=(INTERNAL_PTRLIST*) realloc(entries, newSize);
    if (entries==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Memory full.");
      return NULL;
    }

    /* preset new entries */
    if (diffEntries)
      memset((void*) &(entries->entries[entries->storedEntries]), 0, diffEntries*sizeof(void*));
    entries->storedEntries=totalEntries;
    return entries;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Null pointer or already freed");
    return NULL;
  }
}



INTERNAL_PTRLIST *_copyPtrList(const INTERNAL_PTRLIST *oldEntries, uint64_t totalEntries)
{
  assert(oldEntries && oldEntries->refCounter>0);
  if (oldEntries && oldEntries->refCounter>0) {
    INTERNAL_PTRLIST *entries;
    size_t oldSize;
    size_t newSize;
    uint64_t diffEntries;

    if (totalEntries<oldEntries->storedEntries)
      totalEntries=oldEntries->storedEntries;

    diffEntries=totalEntries-(oldEntries->storedEntries);
    oldSize=sizeof(INTERNAL_PTRLIST)+((oldEntries->storedEntries)*sizeof(void*));
    newSize=sizeof(INTERNAL_PTRLIST)+totalEntries*sizeof(void*);

    entries=(INTERNAL_PTRLIST*) malloc(newSize);
    if (entries==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Memory full.");
      return NULL;
    }

    /* copy old struct */
    memmove(entries, oldEntries, oldSize);

    /* preset new entries */
    if (diffEntries)
      memset((void*) &(entries->entries[entries->storedEntries]), 0, diffEntries*sizeof(void*));

    /* setup rest of the fields */
    entries->refCounter=1;
    entries->storedEntries=totalEntries;
    return entries;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Null pointer or already freed");
    return NULL;
  }
}


/* include tests */
#include "simpleptrlist-t.c"

