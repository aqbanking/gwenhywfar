/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: idlist_p.h 1048 2006-05-17 17:15:35Z martin $
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

#ifndef GWENHYWFAR_IDMAP_P_H
#define GWENHYWFAR_IDMAP_P_H

#include "idmap.h"


typedef GWEN_IDMAP_RESULT (*GWEN_IDMAP_SETPAIR_FN)(GWEN_IDMAP *map,
    uint32_t id,
    void *ptr);
typedef void* (*GWEN_IDMAP_GETPAIR_FN)(GWEN_IDMAP *map,
                                       uint32_t id);

typedef GWEN_IDMAP_RESULT (*GWEN_IDMAP_FINDFIRST_FN)(const GWEN_IDMAP *map,
    uint32_t *pid);
typedef GWEN_IDMAP_RESULT (*GWEN_IDMAP_FINDNEXT_FN)(const GWEN_IDMAP *map,
    uint32_t *pid);

typedef void (*GWEN_IDMAP_DUMP_FN)(GWEN_IDMAP *map, FILE *f, int indent);


typedef void (*GWEN_IDMAP_FREEDATA_FN)(GWEN_IDMAP *map);


struct GWEN_IDMAP {
  GWEN_IDMAP_SETPAIR_FN setPairFn;
  GWEN_IDMAP_GETPAIR_FN getPairFn;
  GWEN_IDMAP_FINDFIRST_FN findFirstFn;
  GWEN_IDMAP_FINDNEXT_FN findNextFn;
  GWEN_IDMAP_FREEDATA_FN freeDataFn;
  GWEN_IDMAP_DUMP_FN dumpFn;

  GWEN_IDMAP_ALGO algo;
  void *algoData;
  uint32_t count;
};




typedef struct GWEN_IDMAP_HEX4_TABLE GWEN_IDMAP_HEX4_TABLE;
struct GWEN_IDMAP_HEX4_TABLE {
  GWEN_IDMAP_HEX4_TABLE *parent;
  int isPtrTable;
  void* ptrs[16];
};
static GWEN_IDMAP_HEX4_TABLE *GWEN_IdMapHex4Map_new(GWEN_IDMAP_HEX4_TABLE *p,
    int isPtrTable);
static void GWEN_IdMapHex4Map_free(GWEN_IDMAP_HEX4_TABLE *t);


typedef struct GWEN_IDMAP_HEX4 GWEN_IDMAP_HEX4;
struct GWEN_IDMAP_HEX4 {
  GWEN_IDMAP_HEX4_TABLE *table;
  uint32_t lastId;
};

static void GWEN_IdMapHex4_Extend(GWEN_IDMAP *map);
static void GWEN_IdMapHex4_free(GWEN_IDMAP *map);
static GWEN_IDMAP_RESULT GWEN_IdMapHex4_Insert(GWEN_IDMAP *map,
    uint32_t id,
    void *ptr);
static void *GWEN_IdMapHex4_Find(GWEN_IDMAP *map, uint32_t id);

static GWEN_IDMAP_RESULT GWEN_IdMapHex4_FindFirst(const GWEN_IDMAP *map,
    uint32_t *pid);
static GWEN_IDMAP_RESULT GWEN_IdMapHex4_FindNext(const GWEN_IDMAP *map,
    uint32_t *pid);



static
GWEN_IDMAP_HEX4_TABLE *GWEN_IdMapHex4__GetTable(GWEN_IDMAP_HEX4_TABLE *t,
    uint32_t id);

static
GWEN_IDMAP_HEX4_TABLE *GWEN_IdMapHex4__GetFirstTable(GWEN_IDMAP_HEX4_TABLE *t,
    uint32_t *pid);

static
GWEN_IDMAP_HEX4_TABLE *GWEN_IdMapHex4__GetNextTable(GWEN_IDMAP_HEX4_TABLE *t,
    uint32_t *pid,
    int incr);

static void GWEN_IdMapHex4__Dump(GWEN_IDMAP_HEX4_TABLE *table,
                                 FILE *f, int indent);

static void GWEN_IdMapHex4_Dump(GWEN_IDMAP *map, FILE *f, int indent);


#endif

