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


#ifndef GWEN_NL_HBCI_P_H
#define GWEN_NL_HBCI_P_H

#define GWEN_NL_HBCI_GETTYPE_SIZE      8
#define GWEN_NL_HBCI_GETSIZE_SIZE      30
#define GWEN_NL_HBCI_READMSG_CHUNKSIZE 512

#include "nl_hbci.h"

#include <gwenhywfar/buffer.h>


typedef enum {
  GWEN_NetLayerHbciInMode_Idle=0,
  GWEN_NetLayerHbciInMode_GetType,
  GWEN_NetLayerHbciInMode_ReadSize,
  GWEN_NetLayerHbciInMode_ReadMsg,
  GWEN_NetLayerHbciInMode_HasMsg,
  GWEN_NetLayerHbciInMode_Done,
  GWEN_NetLayerHbciInMode_ReadError,
  GWEN_NetLayerHbciInMode_Aborted
} GWEN_NL_HBCI_INMODE;


typedef enum {
  GWEN_NetLayerHbciOutMode_Idle=0,
  GWEN_NetLayerHbciOutMode_WaitForEnd,
  GWEN_NetLayerHbciOutMode_WriteMsg,
  GWEN_NetLayerHbciOutMode_Done,
  GWEN_NetLayerHbciOutMode_Aborted
} GWEN_NL_HBCI_OUTMODE;


typedef struct GWEN_NL_HBCI GWEN_NL_HBCI;
struct GWEN_NL_HBCI {

  /* incoming */
  GWEN_NL_HBCI_INMODE inMode;
  GWEN_BUFFER *inBuffer;
  int inBodyRead;
  int inIsBase64;
  int inHttpStatusCode;

  /* outgoing */
  GWEN_NL_HBCI_OUTMODE outMode;
  GWEN_BUFFER *outBuffer;
  int outBodyWritten;

};

void GWEN_NetLayerHbci_FreeData(void *bp, void *p);

int GWEN_NetLayerHbci_Connect(GWEN_NETLAYER *nl);
int GWEN_NetLayerHbci_Disconnect(GWEN_NETLAYER *nl);
int GWEN_NetLayerHbci_Listen(GWEN_NETLAYER *nl);

int GWEN_NetLayerHbci_Read(GWEN_NETLAYER *nl, char *buffer, int *bsize);
int GWEN_NetLayerHbci_Write(GWEN_NETLAYER *nl,
                              const char *buffer,
                              int *bsize);

int GWEN_NetLayerHbci_AddSockets(GWEN_NETLAYER *nl,
                                 GWEN_SOCKETSET *readSet,
                                 GWEN_SOCKETSET *writeSet,
                                 GWEN_SOCKETSET *exSet);

GWEN_NETLAYER_RESULT GWEN_NetLayerHbci_Work(GWEN_NETLAYER *nl);

GWEN_NETLAYER_RESULT GWEN_NetLayerHbci__WriteWork(GWEN_NETLAYER *nl);
GWEN_NETLAYER_RESULT GWEN_NetLayerHbci__ReadWork(GWEN_NETLAYER *nl);

int GWEN_NetLayerHbci_BeginOutPacket(GWEN_NETLAYER *nl, int totalSize);
int GWEN_NetLayerHbci_EndOutPacket(GWEN_NETLAYER *nl);

int GWEN_NetLayerHbci_BeginInPacket(GWEN_NETLAYER *nl);
int GWEN_NetLayerHbci_CheckInPacket(GWEN_NETLAYER *nl);

#endif /* GWEN_NL_HBCI_P_H */


