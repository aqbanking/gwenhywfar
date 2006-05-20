/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: csv_p.h 120 2003-12-03 23:29:33Z aquamaniac $
 begin       : Thu Oct 30 2003
 copyright   : (C) 2003 by Martin Preuss
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

#ifndef GWEN_STO_STORAGE_H
#define GWEN_STO_STORAGE_H


typedef struct GWEN_STO_STORAGE GWEN_STO_STORAGE;

#include <gwenhywfar/types.h>
#include <gwenhywfar/st_log.h>


#define GWEN_STO_OPENFLAGS_RD 0x00000001
#define GWEN_STO_OPENFLAGS_WR 0x00000002


typedef enum {
  GWEN_StoCloseMode_Unknown=0,
  GWEN_StoCloseMode_Normal,
  GWEN_StoCloseMode_NoUpdate,
  GWEN_StoCloseMode_Delete
} GWEN_STO_CLOSEMODE;


typedef enum {
  GWEN_StoLockMode_Unknown=0,
  GWEN_StoLockMode_Lock,
  GWEN_StoLockMode_Unlock
} GWEN_STO_LOCKMODE;

typedef void (*GWEN_STO_STORAGE_LOG_CB)(GWEN_STO_STORAGE *st,
                                        const GWEN_STO_LOG *log);


#include <gwenhywfar/st_client.h>


GWENHYWFAR_API
GWEN_STO_STORAGE *GWEN_StoStorage_Factory(const char *modname,
                                          const char *address);


GWENHYWFAR_API
void GWEN_StoStorage_free(GWEN_STO_STORAGE *st);

GWENHYWFAR_API
const char *GWEN_StoStorage_GetTypeName(const GWEN_STO_STORAGE *st);

GWENHYWFAR_API
const char *GWEN_StoStorage_GetAddress(const GWEN_STO_STORAGE *st);

GWENHYWFAR_API
GWEN_STO_STORAGE_LOG_CB
  GWEN_StoStorage_SetLog_Cb(GWEN_STO_STORAGE *st,
                            GWEN_STO_STORAGE_LOG_CB cb);

GWENHYWFAR_API
int GWEN_StoStorage_RegisterClient(GWEN_STO_STORAGE *st,
                                   const char *userName,
                                   GWEN_STO_CLIENT **pcl);

GWENHYWFAR_API
int GWEN_StoStorage_UnregisterClient(GWEN_STO_STORAGE *st,
                                     GWEN_STO_CLIENT *cl,
                                     const char *reason);

#endif



