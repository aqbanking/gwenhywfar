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


typedef struct GWEN_IDTABLE64 GWEN_IDTABLE64;
GWEN_LIST_FUNCTION_DEFS(GWEN_IDTABLE64, GWEN_IdTable)
/* No trailing semicolon here because this is a macro call */
struct GWEN_IDTABLE64 {
  GWEN_LIST_ELEMENT(GWEN_IDTABLE64)
  /* No trailing semicolon here because this is a macro call */
  uint64_t freeEntries;
  uint64_t entries[GWEN_IDTABLE64_MAXENTRIES];
  uint64_t current;
};

GWEN_IDTABLE64 *GWEN_IdTable64_new();
void GWEN_IdTable64_free(GWEN_IDTABLE64 *idt);

int GWEN_IdTable64_AddId(GWEN_IDTABLE64 *idt, uint64_t id);
int GWEN_IdTable64_HasId(const GWEN_IDTABLE64 *idt, uint64_t id);
int GWEN_IdTable64_DelId(GWEN_IDTABLE64 *idt, uint64_t id);
int GWEN_IdTable64_IsEmpty(const GWEN_IDTABLE64 *idt);
int GWEN_IdTable64_IsFull(const GWEN_IDTABLE64 *idt);
uint64_t GWEN_IdTable64_GetFirstId(GWEN_IDTABLE64 *idt);
uint64_t GWEN_IdTable64_GetNextId(GWEN_IDTABLE64 *idt);
unsigned int GWEN_IdTable64_GetCount(const GWEN_IDTABLE64 *idt);

uint64_t GWEN_IdTable64_GetFirstId2(const GWEN_IDTABLE64 *idt,
				    uint64_t *tabIdx);
uint64_t GWEN_IdTable64_GetNextId2(const GWEN_IDTABLE64 *idt,
				   uint64_t *tabIdx);

void GWEN_IdList64_Clean(GWEN_IDLIST64 *idl);

struct GWEN_IDLIST64 {
  GWEN_IDTABLE64_LIST *idTables;
  uint64_t entryCount;
  GWEN_IDTABLE64 *current;
};




#endif /* GWENHYWFAR_IDLIST64_P_H */

