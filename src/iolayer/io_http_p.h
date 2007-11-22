/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_IOLAYER_HTTP_P_H
#define GWEN_IOLAYER_HTTP_P_H

#include <gwenhywfar/io_http.h>
#include <gwenhywfar/ringbuffer.h>




#define GWEN_IO_LAYER_HTTP_LINE_STEPSIZE 256



typedef enum {
  GWEN_Io_LayerHttp_Mode_Idle=0,
  GWEN_Io_LayerHttp_Mode_Command,
  GWEN_Io_LayerHttp_Mode_Status,
  GWEN_Io_LayerHttp_Mode_Header,
  /** not used when writing */
  GWEN_Io_LayerHttp_Mode_ChunkSize,
  /** not used when writing */
  GWEN_Io_LayerHttp_Mode_Chunk,
  GWEN_Io_LayerHttp_Mode_Body,
  GWEN_Io_LayerHttp_Mode_Finished
} GWEN_IO_LAYER_HTTP_MODE;



typedef struct GWEN_IO_LAYER_HTTP GWEN_IO_LAYER_HTTP;
struct GWEN_IO_LAYER_HTTP {
  GWEN_IO_REQUEST *readRequestIn;
  GWEN_IO_REQUEST *writeRequestIn;

  GWEN_IO_LAYER_HTTP_MODE readMode;
  GWEN_IO_LAYER_HTTP_MODE writeMode;

  GWEN_IO_REQUEST *readRequestOut;
  GWEN_IO_REQUEST *writeRequestOut;
  int lastReadOutResult;
  int lastWriteOutResult;

  GWEN_DB_NODE *dbCommandIn;
  GWEN_DB_NODE *dbStatusIn;
  GWEN_DB_NODE *dbHeaderIn;
  GWEN_BUFFER *readBuffer;
  int readLineFinished;
  int currentReadChunkSize;
  int currentReadBodySize;
  uint32_t lastHeaderPos;

  GWEN_DB_NODE *dbCommandOut;
  GWEN_DB_NODE *dbStatusOut;
  GWEN_DB_NODE *dbHeaderOut;
  GWEN_BUFFER *writeBuffer;
  int currentWriteBodySize;
};


static GWENHYWFAR_CB void GWEN_Io_LayerHttp_freeData(void *bp, void *p);

static void GWEN_Io_LayerHttp_AbortInRequests(GWEN_IO_LAYER *io, int errorCode);
static void GWEN_Io_LayerHttp_AbortOutRequests(GWEN_IO_LAYER *io);


static int GWEN_Io_LayerHttp_ParseCommand(GWEN_IO_LAYER *io, const char *buf);
static int GWEN_Io_LayerHttp_ParseStatus(GWEN_IO_LAYER *io, const char *buf);
static int GWEN_Io_LayerHttp_ParseHeader(GWEN_IO_LAYER *io, char *buf);

static int GWEN_Io_LayerHttp_WriteCommand(GWEN_IO_LAYER *io, GWEN_BUFFER *buf);
static int GWEN_Io_LayerHttp_WriteStatus(GWEN_IO_LAYER *io, GWEN_BUFFER *buf);
static int GWEN_Io_LayerHttp_WriteHeader(GWEN_IO_LAYER *io, GWEN_BUFFER *buf);


static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnReadRequest1(GWEN_IO_LAYER *io);
static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnReadRequest2(GWEN_IO_LAYER *io);
static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnReadRequest(GWEN_IO_LAYER *io);

static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnWriteRequest1(GWEN_IO_LAYER *io);
static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnWriteRequest2(GWEN_IO_LAYER *io);
static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnWriteRequest(GWEN_IO_LAYER *io);

static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnRequests(GWEN_IO_LAYER *io);

static int GWEN_Io_LayerHttp_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);
static int GWEN_Io_LayerHttp_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);
static int GWEN_Io_LayerHttp_HasWaitingRequests(GWEN_IO_LAYER *io);



#endif




