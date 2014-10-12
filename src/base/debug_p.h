/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sun Jan 04 2004
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


#ifndef GWEN_DEBUG_P_H
#define GWEN_DEBUG_P_H


#include <gwenhywfar/debug.h>


typedef enum {
  GWEN_MemoryDebugEntryTypeUnknown=0,
  GWEN_MemoryDebugEntryTypeCreate,
  GWEN_MemoryDebugEntryTypeAttach,
  GWEN_MemoryDebugEntryTypeFree
} GWEN_MEMORY_DEBUG_ENTRY_TYPE;



typedef struct GWEN_MEMORY_DEBUG_ENTRY GWEN_MEMORY_DEBUG_ENTRY;
struct GWEN_MEMORY_DEBUG_ENTRY {
  GWEN_MEMORY_DEBUG_ENTRY *next;
  GWEN_MEMORY_DEBUG_ENTRY_TYPE type;
  char *file;
  int line;
};
GWEN_MEMORY_DEBUG_ENTRY*
GWEN_MemoryDebugEntry_new(GWEN_MEMORY_DEBUG_ENTRY_TYPE t,
                          const char *wFile,
                          int wLine);
void GWEN_MemoryDebugEntry_free(GWEN_MEMORY_DEBUG_ENTRY *e);


struct GWEN_MEMORY_DEBUG_OBJECT {
  GWEN_MEMORY_DEBUG_OBJECT *next;
  char *name;
  long int count;
  GWEN_MEMORY_DEBUG_ENTRY *entries;
};
GWEN_MEMORY_DEBUG_OBJECT *GWEN_MemoryDebugObject_new(const char *name);
void GWEN_MemoryDebugObject_free(GWEN_MEMORY_DEBUG_OBJECT *o);


GWEN_MEMORY_DEBUG_OBJECT *GWEN_MemoryDebug__FindObject(const char *name);
void GWEN_MemoryDebug__DumpObject(GWEN_MEMORY_DEBUG_OBJECT *o,
                                  uint32_t mode);



uint32_t GWEN_Debug_PrintDec(char *buffer,
                             uint32_t size,
                             uint32_t num,
                             int leadingZero,
                             uint32_t length);


uint32_t GWEN_Debug_PrintHex(char *buffer,
                             uint32_t size,
                             uint32_t num,
                             int leadingZero,
                             int up,
                             uint32_t length);



#endif


