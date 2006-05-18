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

#ifndef GWENHYWFAR_IDMAP_H
#define GWENHYWFAR_IDMAP_H


#include <gwenhywfar/types.h>



typedef struct GWEN_IDMAP GWEN_IDMAP;

typedef enum {
  GWEN_IdMapResult_Ok=0,
  GWEN_IdMapResult_NoFit,
  GWEN_IdMapResult_NotFound
} GWEN_IDMAP_RESULT;


typedef enum {
  GWEN_IdMapAlgo_Unknown=0,
  GWEN_IdMapAlgo_Hex4
} GWEN_IDMAP_ALGO;



GWENHYWFAR_API
GWEN_IDMAP *GWEN_IdMap_new(GWEN_IDMAP_ALGO algo);

GWENHYWFAR_API
void GWEN_IdMap_free(GWEN_IDMAP *map);

GWENHYWFAR_API
GWEN_IDMAP_RESULT GWEN_IdMap_SetPtr(GWEN_IDMAP *map,
				    GWEN_TYPE_UINT32 id,
				    void *ptr);

GWENHYWFAR_API
void *GWEN_IdMap_GetPtr(GWEN_IDMAP *map, GWEN_TYPE_UINT32 id);


GWENHYWFAR_API
GWEN_IDMAP_RESULT GWEN_IdMap_FindFirst(GWEN_IDMAP *map,
                                       GWEN_TYPE_UINT32 *pid);

GWENHYWFAR_API
GWEN_IDMAP_RESULT GWEN_IdMap_FindNext(GWEN_IDMAP *map,
                                      GWEN_TYPE_UINT32 *pid);




#endif

