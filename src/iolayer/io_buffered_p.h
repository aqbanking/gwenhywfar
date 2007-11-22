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


#ifndef GWEN_IOLAYER_BUFFERED_P_H
#define GWEN_IOLAYER_BUFFERED_P_H

#include <gwenhywfar/io_buffered.h>
#include <gwenhywfar/ringbuffer.h>


#define GWEN_IO_LAYER_BUFFERED_BUFSIZE 1024


#define GWEN_IO_REQUEST_BUFFERED_FLAGS_WRITE13           0x00008000
#define GWEN_IO_REQUEST_BUFFERED_FLAGS_WRITE10           0x00004000


typedef struct GWEN_IO_LAYER_BUFFERED GWEN_IO_LAYER_BUFFERED;
struct GWEN_IO_LAYER_BUFFERED {
  GWEN_IO_REQUEST *readRequestIn;
  GWEN_IO_REQUEST *writeRequestIn;

  GWEN_IO_REQUEST *readRequestOut;
  GWEN_IO_REQUEST *writeRequestOut;

  GWEN_RINGBUFFER *readBuffer;
  GWEN_RINGBUFFER *writeBuffer;

  int lastReadWasPacketEnd;
  uint32_t readLineCount;
  uint32_t readLinePos;

  int lastReadOutResult;
  int lastWriteOutResult;
};


static GWENHYWFAR_CB void GWEN_Io_LayerBuffered_freeData(void *bp, void *p);
static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerBuffered_WorkOnRequests(GWEN_IO_LAYER *io);
static int GWEN_Io_LayerBuffered_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);
static int GWEN_Io_LayerBuffered_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);
static int GWEN_Io_LayerBuffered_HasWaitingRequests(GWEN_IO_LAYER *io);

static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerBuffered_WorkOnReadRequest(GWEN_IO_LAYER *io);
static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerBuffered_WorkOnWriteRequest(GWEN_IO_LAYER *io);

static int GWEN_Io_LayerBuffered_TryFlush(GWEN_IO_LAYER *io);

static void GWEN_Io_LayerBuffered_AbortInRequests(GWEN_IO_LAYER *io, int errorCode);
static void GWEN_Io_LayerBuffered_AbortOutRequests(GWEN_IO_LAYER *io);


#endif




