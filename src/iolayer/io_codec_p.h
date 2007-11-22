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


#ifndef GWEN_IOLAYER_CODEC_P_H
#define GWEN_IOLAYER_CODEC_P_H

#include <gwenhywfar/io_codec_be.h>
#include <gwenhywfar/ringbuffer.h>


#define GWEN_IO_LAYER_CODEC_BUFSIZE 1024


typedef struct GWEN_IO_LAYER_CODEC GWEN_IO_LAYER_CODEC;
struct GWEN_IO_LAYER_CODEC {
  GWEN_IO_REQUEST *readRequestIn;
  GWEN_IO_REQUEST *writeRequestIn;

  GWEN_IO_REQUEST *readRequestOut;
  GWEN_IO_REQUEST *writeRequestOut;

  GWEN_RINGBUFFER *readBuffer;
  GWEN_RINGBUFFER *writeBuffer;

  int lastReadOutResult;
  int lastWriteOutResult;

  GWEN_IO_LAYER_CODE_ENCODE_FN encodeFn;
  GWEN_IO_LAYER_CODE_DECODE_FN decodeFn;

  uint32_t currentGuiId;
};


static GWENHYWFAR_CB void GWEN_Io_LayerCodec_freeData(void *bp, void *p);
static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerCodec_WorkOnRequests(GWEN_IO_LAYER *io);

static int GWEN_Io_LayerCodec_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);
static int GWEN_Io_LayerCodec_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);
static int GWEN_Io_LayerCodec_HasWaitingRequests(GWEN_IO_LAYER *io);

static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerCodec_WorkOnReadRequest(GWEN_IO_LAYER *io);
static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerCodec_WorkOnWriteRequest(GWEN_IO_LAYER *io);
static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerCodec_WorkOnOutRequests(GWEN_IO_LAYER *io);

static void GWEN_Io_LayerCodec_AbortInRequests(GWEN_IO_LAYER *io, int errorCode);
static void GWEN_Io_LayerCodec_AbortOutRequests(GWEN_IO_LAYER *io);


static int GWEN_Io_LayerCodec_Encode(GWEN_IO_LAYER *io, const uint8_t *pBuffer, uint32_t lBuffer);
static int GWEN_Io_LayerCodec_Decode(GWEN_IO_LAYER *io, uint8_t *pBuffer, uint32_t lBuffer);


#endif




