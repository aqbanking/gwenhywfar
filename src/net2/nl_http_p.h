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


#ifndef GWEN_NL_HTTP_P_H
#define GWEN_NL_HTTP_P_H

#include "nl_http.h"

#include <gwenhywfar/buffer.h>


#define GWEN_NL_HTTP_MAX_CMD_OR_STATUS_SIZE 256


typedef enum {
  GWEN_NetLayerHttpInMode_Idle=0,
  GWEN_NetLayerHttpInMode_ReadCommand,
  GWEN_NetLayerHttpInMode_ReadStatus,
  GWEN_NetLayerHttpInMode_ReadHeader,
  GWEN_NetLayerHttpInMode_ReadChunkSize,
  GWEN_NetLayerHttpInMode_ReadChunkTrailer,
  GWEN_NetLayerHttpInMode_ReadBody,
  GWEN_NetLayerHttpInMode_ReadDone,
  GWEN_NetLayerHttpInMode_Aborted
} GWEN_NL_HTTP_INMODE;


typedef struct GWEN_NL_HTTP GWEN_NL_HTTP;
struct GWEN_NL_HTTP {

  GWEN_NETLAYER_HTTP_VERSION pversion;

  /* incoming */
  GWEN_NL_HTTP_INMODE inMode;
  GWEN_DB_NODE *dbInHeader;
  GWEN_BUFFER *inBuffer;
  char *inCommand;
  GWEN_URL *inUrl;
  int inStatusCode;
  char *inStatusText;
  int inBodySize;
  int inBodyRead;
  int inChunkSize;
  int inChunkRead;
  GWEN_TYPE_UINT32 inLast4Bytes;

  /* outgoing */
  GWEN_BUFFER *outBuffer;
  char *outCommand;
  GWEN_URL *outUrl;
  int outStatusCode;
  char *outStatusText;
  GWEN_DB_NODE *dbOutHeader;
  int outBodySize;
  int outBodyWritten;

};

void GWEN_NetLayerHttp_FreeData(void *bp, void *p);

int GWEN_NetLayerHttp_Connect(GWEN_NETLAYER *nl);
int GWEN_NetLayerHttp_Disconnect(GWEN_NETLAYER *nl);
int GWEN_NetLayerHttp_Listen(GWEN_NETLAYER *nl);

int GWEN_NetLayerHttp_Read(GWEN_NETLAYER *nl, char *buffer, int *bsize);
int GWEN_NetLayerHttp_Write(GWEN_NETLAYER *nl,
                              const char *buffer,
                              int *bsize);

int GWEN_NetLayerHttp_AddSockets(GWEN_NETLAYER *nl,
                                 GWEN_SOCKETSET *readSet,
                                 GWEN_SOCKETSET *writeSet,
                                 GWEN_SOCKETSET *exSet);

void GWEN_NetLayerHttp_BaseStatusChange(GWEN_NETLAYER *nl,
                                        GWEN_NETLAYER_STATUS newst);

GWEN_NETLAYER_RESULT GWEN_NetLayerHttp_Work(GWEN_NETLAYER *nl);

GWEN_NETLAYER_RESULT GWEN_NetLayerHttp__WriteWork(GWEN_NETLAYER *nl);
GWEN_NETLAYER_RESULT GWEN_NetLayerHttp__ReadWork(GWEN_NETLAYER *nl);

int GWEN_NetLayerHttp_BeginOutPacket(GWEN_NETLAYER *nl, int totalSize);
int GWEN_NetLayerHttp_EndOutPacket(GWEN_NETLAYER *nl);

int GWEN_NetLayerHttp_BeginInPacket(GWEN_NETLAYER *nl);
int GWEN_NetLayerHttp_CheckInPacket(GWEN_NETLAYER *nl);

int GWEN_NetLayerHttp__ParseCommand(GWEN_NETLAYER *nl, const char *buffer);
int GWEN_NetLayerHttp__ParseStatus(GWEN_NETLAYER *nl, const char *buffer);

int GWEN_NetLayerHttp__ParseHeader(GWEN_NETLAYER *nl, const char *buffer);


#endif /* GWEN_NL_HTTP_P_H */


