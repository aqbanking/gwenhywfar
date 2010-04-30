/***************************************************************************
 begin       : Wed Apr 28 2010
 copyright   : (C) 2010 by Martin Preuss
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


#ifndef GWENHYWFAR_SYNCIO_HTTP_P_H
#define GWENHYWFAR_SYNCIO_HTTP_P_H


#include <gwenhywfar/syncio_http.h>



typedef enum {
  GWEN_SyncIo_Http_Mode_Idle=0,
  /** not used when writing */
  GWEN_SyncIo_Http_Mode_ChunkSize,
  /** not used when writing */
  GWEN_SyncIo_Http_Mode_Chunk,
  GWEN_SyncIo_Http_Mode_Body,
  GWEN_SyncIo_Http_Mode_Error,
} GWEN_SYNCIO_HTTP_MODE;


typedef struct GWEN_SYNCIO_HTTP GWEN_SYNCIO_HTTP;
struct GWEN_SYNCIO_HTTP {
  GWEN_SYNCIO_HTTP_MODE readMode;

  GWEN_DB_NODE *dbCommandIn;
  GWEN_DB_NODE *dbStatusIn;
  GWEN_DB_NODE *dbHeaderIn;

  int currentReadChunkSize;
  int currentReadBodySize;

  GWEN_SYNCIO_HTTP_MODE writeMode;

  GWEN_DB_NODE *dbCommandOut;
  GWEN_DB_NODE *dbStatusOut;
  GWEN_DB_NODE *dbHeaderOut;

  int currentWriteBodySize;

};


static void GWENHYWFAR_CB GWEN_SyncIo_Http_FreeData(void *bp, void *p);


int GWENHYWFAR_CB GWEN_SyncIo_Http_Connect(GWEN_SYNCIO *sio);
int GWENHYWFAR_CB GWEN_SyncIo_Http_Disconnect(GWEN_SYNCIO *sio);


int GWENHYWFAR_CB GWEN_SyncIo_Http_Read(GWEN_SYNCIO *sio,
					uint8_t *buffer,
					uint32_t size);



int GWENHYWFAR_CB GWEN_SyncIo_Http_Write(GWEN_SYNCIO *sio,
					 const uint8_t *buffer,
					 uint32_t size);


static int GWEN_SyncIo_Http_ReadLine(GWEN_SYNCIO *sio, GWEN_BUFFER *tbuf);
static int GWEN_SyncIo_Http_ReadStatus(GWEN_SYNCIO *sio);
static int GWEN_SyncIo_Http_ReadCommand(GWEN_SYNCIO *sio);
static int GWEN_SyncIo_Http_ReadHeader(GWEN_SYNCIO *sio);
static int GWEN_SyncIo_Http_ReadChunkSize(GWEN_SYNCIO *sio);
static int GWEN_SyncIo_Http_ReadChunk(GWEN_SYNCIO *sio, uint8_t *buffer, uint32_t size);
static int GWEN_SyncIo_Http_ReadBody(GWEN_SYNCIO *sio, uint8_t *buffer, uint32_t size);

static int GWEN_SyncIo_Http_ParseStatus(GWEN_SYNCIO *sio, char *buffer);
static int GWEN_SyncIo_Http_ParseCommand(GWEN_SYNCIO *sio, const char *buffer);

static void GWEN_SyncIo_Http_SetReadIdle(GWEN_SYNCIO *sio);


static int GWEN_SyncIo_Http_WriteCommand(GWEN_SYNCIO *sio);
static int GWEN_SyncIo_Http_WriteStatus(GWEN_SYNCIO *sio);
static int GWEN_SyncIo_Http_WriteHeader(GWEN_SYNCIO *sio);
static int GWEN_SyncIo_Http_WriteChunkSize(GWEN_SYNCIO *sio, uint32_t size);

static void GWEN_SyncIo_Http_SetWriteIdle(GWEN_SYNCIO *sio);


#endif


