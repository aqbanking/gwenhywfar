/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Jan 24 2004
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


#ifndef GWEN_NL_LOG_P_H
#define GWEN_NL_LOG_P_H

#include "nl_log.h"

#include <gwenhywfar/buffer.h>


typedef struct GWEN_NL_LOG GWEN_NL_LOG;
struct GWEN_NL_LOG {
  char *nameBase;
  int logWrite;
  int logRead;
  int count;
  int inFd;
  int outFd;
};

void GWENHYWFAR_CB GWEN_NetLayerLog_FreeData(void *bp, void *p);

int GWEN_NetLayerLog_Connect(GWEN_NETLAYER *nl);
int GWEN_NetLayerLog_Disconnect(GWEN_NETLAYER *nl);
int GWEN_NetLayerLog_Listen(GWEN_NETLAYER *nl);

int GWEN_NetLayerLog_Read(GWEN_NETLAYER *nl, char *buffer, int *bsize);
int GWEN_NetLayerLog_Write(GWEN_NETLAYER *nl,
                              const char *buffer,
                              int *bsize);

int GWEN_NetLayerLog_AddSockets(GWEN_NETLAYER *nl,
                                GWEN_SOCKETSET *readSet,
                                GWEN_SOCKETSET *writeSet,
                                GWEN_SOCKETSET *exSet);

void GWEN_NetLayerLog_BaseStatusChange(GWEN_NETLAYER *nl,
                                        GWEN_NETLAYER_STATUS newst);

GWEN_NETLAYER_RESULT GWEN_NetLayerLog_Work(GWEN_NETLAYER *nl);

int GWEN_NetLayerLog_BeginOutPacket(GWEN_NETLAYER *nl, int totalSize);
int GWEN_NetLayerLog_EndOutPacket(GWEN_NETLAYER *nl);

int GWEN_NetLayerLog_BeginInPacket(GWEN_NETLAYER *nl);
int GWEN_NetLayerLog_CheckInPacket(GWEN_NETLAYER *nl);


#endif /* GWEN_NL_LOG_P_H */


