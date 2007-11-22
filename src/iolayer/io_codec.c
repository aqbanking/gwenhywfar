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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "io_codec_p.h"
#include <gwenhywfar/iolayer_be.h>
#include <gwenhywfar/iorequest_be.h>
#include <gwenhywfar/iomanager.h>

#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <gwenhywfar/text.h>

#include <assert.h>



GWEN_INHERIT(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC)



GWEN_IO_LAYER *GWEN_Io_LayerCodec_new(const char *typeName, GWEN_IO_LAYER *baseLayer) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_CODEC *xio;

  io=GWEN_Io_Layer_new(typeName, baseLayer);
  assert(io);
  GWEN_NEW_OBJECT(GWEN_IO_LAYER_CODEC, xio);
  assert(xio);
  GWEN_INHERIT_SETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io, xio, GWEN_Io_LayerCodec_freeData);

  GWEN_Io_Layer_SetWorkOnRequestsFn(io, GWEN_Io_LayerCodec_WorkOnRequests);
  GWEN_Io_Layer_SetAddRequestFn(io, GWEN_Io_LayerCodec_AddRequest);
  GWEN_Io_Layer_SetDelRequestFn(io, GWEN_Io_LayerCodec_DelRequest);
  GWEN_Io_Layer_SetHasWaitingRequestsFn(io, GWEN_Io_LayerCodec_HasWaitingRequests);

  return io;
}



GWENHYWFAR_CB
void GWEN_Io_LayerCodec_freeData(void *bp, void *p) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_CODEC *xio;

  io=(GWEN_IO_LAYER*) bp;
  assert(io);
  xio=(GWEN_IO_LAYER_CODEC*) p;
  assert(xio);

  GWEN_Io_LayerCodec_AbortInRequests(io, GWEN_ERROR_ABORTED);
  GWEN_Io_LayerCodec_AbortOutRequests(io);

  GWEN_RingBuffer_free(xio->readBuffer);
  GWEN_RingBuffer_free(xio->writeBuffer);

  GWEN_FREE_OBJECT(xio);
}



uint32_t GWEN_Io_LayerCodec_GetCurrentGuiId(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  return xio->currentGuiId;
}



void GWEN_Io_LayerCodec_SetCurrentGuiId(GWEN_IO_LAYER *io, uint32_t guiid) {
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  xio->currentGuiId=guiid;
}



void GWEN_Io_LayerCodec_AbortInRequests(GWEN_IO_LAYER *io, int errorCode) {
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  if (xio->readRequestIn) {
    GWEN_IO_REQUEST *r;

    r=xio->readRequestIn;
    xio->readRequestIn=NULL;
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, errorCode);
    GWEN_Io_Request_free(r);
  }
  if (xio->writeRequestIn) {
    GWEN_IO_REQUEST *r;

    r=xio->writeRequestIn;
    xio->writeRequestIn=NULL;
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, errorCode);
    GWEN_Io_Request_free(r);
  }
}



void GWEN_Io_LayerCodec_AbortOutRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  if (xio->readRequestOut) {
    GWEN_IO_REQUEST *r;

    r=xio->readRequestOut;
    GWEN_Io_Layer_DelRequest(GWEN_Io_Layer_GetBaseLayer(io), r);
    xio->readRequestOut=NULL;
    GWEN_Io_Request_free(r);
  }
  if (xio->writeRequestOut) {
    GWEN_IO_REQUEST *r;

    r=xio->writeRequestOut;
    GWEN_Io_Layer_DelRequest(GWEN_Io_Layer_GetBaseLayer(io), r);
    xio->writeRequestOut=NULL;
    GWEN_Io_Request_free(r);
  }
}



int GWEN_Io_LayerCodec_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_CODEC *xio;
  GWEN_IO_LAYER_STATUS st;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  st=GWEN_Io_Layer_GetStatus(io);

  switch(GWEN_Io_Request_GetType(r)) {
  case GWEN_Io_Request_TypeRead:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "IO layer is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }

    /* check whether we already have a read request */
    if (xio->readRequestIn) {
      DBG_INFO(GWEN_LOGDOMAIN, "There already is a read request");
      return GWEN_ERROR_TRY_AGAIN;
    }

    if (xio->lastReadOutResult) {
      DBG_INFO(GWEN_LOGDOMAIN, "Unable to read (%d)", xio->lastReadOutResult);
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, xio->lastReadOutResult);
      return xio->lastReadOutResult;
    }

    /* enqueue request */
    xio->readRequestIn=r;
    GWEN_Io_Request_Attach(xio->readRequestIn);
    break;

  case GWEN_Io_Request_TypeWrite:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "IO layer is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }

    /* check whether we already have a write request */
    if (xio->writeRequestIn) {
      DBG_INFO(GWEN_LOGDOMAIN, "There already is a write request");
      return GWEN_ERROR_TRY_AGAIN;
    }

    /* enqueue request */
    xio->writeRequestIn=r;
    GWEN_Io_Request_Attach(xio->writeRequestIn);
    break;

  case GWEN_Io_Request_TypeConnect:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusUnconnected &&
	st!=GWEN_Io_Layer_StatusDisconnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "IO layer is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }
    else {
      GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusConnected);
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
    }
    break;

  case GWEN_Io_Request_TypeDisconnect:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "IO layer is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }
    else {
      /* abort incoming requests, if any */
      GWEN_Io_LayerCodec_AbortInRequests(io, GWEN_ERROR_ABORTED);
      GWEN_Io_LayerCodec_AbortOutRequests(io);

      /* free ring buffers */
      GWEN_RingBuffer_free(xio->readBuffer);
      xio->readBuffer=NULL;
      GWEN_RingBuffer_free(xio->writeBuffer);
      xio->writeBuffer=NULL;

      /* closed */
      GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
    }
    break;

  default:
    DBG_INFO(GWEN_LOGDOMAIN, "This request type is not supported (%d)", GWEN_Io_Request_GetType(r));
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_SUPPORTED);
    return GWEN_ERROR_NOT_SUPPORTED;

  }

  return 0;
}



int GWEN_Io_LayerCodec_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  switch(GWEN_Io_Request_GetType(r)) {
  case GWEN_Io_Request_TypeRead:
    if (xio->readRequestIn==r) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Aborted read request");
      xio->readRequestIn=NULL;
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_ABORTED);
      GWEN_Io_Request_free(r);
    }
    else {
      /* not my request */
      DBG_INFO(GWEN_LOGDOMAIN, "Read request not registered with this io layer");
      return GWEN_ERROR_INVALID;
    }
    break;

  case GWEN_Io_Request_TypeWrite:
    if (xio->writeRequestIn==r) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Aborted write request");
      xio->writeRequestIn=NULL;
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_ABORTED);
      GWEN_Io_Request_free(r);
    }
    else {
      /* not my request */
      DBG_INFO(GWEN_LOGDOMAIN, "Write request not registered with this io layer");
      return GWEN_ERROR_INVALID;
    }
    break;

  default:
    break;
  }

  return 0;
}



int GWEN_Io_LayerCodec_HasWaitingRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  if (xio->readRequestIn || xio->writeRequestIn)
    return 1;
  return 0;
}



int GWEN_Io_LayerCodec_CheckWriteOut(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  if (xio->lastWriteOutResult)
    return xio->lastWriteOutResult;

  /*
  if (xio->writeRequestOut)
    return GWEN_ERROR_TRY_AGAIN;
    */

  if (xio->writeBuffer==NULL)
    xio->writeBuffer=GWEN_RingBuffer_new(GWEN_IO_LAYER_CODEC_BUFSIZE);

  if (GWEN_RingBuffer_GetMaxUnsegmentedWrite(xio->writeBuffer)==0)
    return GWEN_ERROR_TRY_AGAIN;

  return 0;
}



GWEN_RINGBUFFER *GWEN_Io_LayerCodec_GetReadBuffer(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  return xio->readBuffer;
}



GWEN_RINGBUFFER *GWEN_Io_LayerCodec_GetWriteBuffer(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  return xio->writeBuffer;
}



int GWEN_Io_LayerCodec_EnsureReadOk(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_CODEC *xio;
  uint32_t len;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  if (xio->readBuffer==NULL)
    xio->readBuffer=GWEN_RingBuffer_new(GWEN_IO_LAYER_CODEC_BUFSIZE);

  len=GWEN_RingBuffer_GetMaxUnsegmentedRead(xio->readBuffer);
  if (len==0) {
    /* read buffer empty, fill it if possible */
    if (xio->readRequestOut)
      /* there already is a read request */
      return GWEN_ERROR_IN_PROGRESS;
    else {
      if (xio->lastReadOutResult)
	return xio->lastReadOutResult;
      else {
	GWEN_IO_REQUEST *br;
	int rv;

	len=GWEN_RingBuffer_GetMaxUnsegmentedWrite(xio->readBuffer);
	assert(len);

	/* there is no outbound write request, create one */
	br=GWEN_Io_Request_new(GWEN_Io_Request_TypeRead,
			       (uint8_t*)GWEN_RingBuffer_GetWritePointer(xio->readBuffer), len,
			       NULL, NULL,
			       xio->currentGuiId);
	DBG_DEBUG(GWEN_LOGDOMAIN, "Adding read request (%d bytes)", len);
	rv=GWEN_Io_Layer_AddRequest(GWEN_Io_Layer_GetBaseLayer(io), br);
	if (rv) {
	  if (rv!=GWEN_ERROR_TRY_AGAIN)
	    xio->lastReadOutResult=rv;
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  GWEN_Io_Request_free(br);
	  return rv;
	}
	else {
	  xio->readRequestOut=br;
	  return GWEN_ERROR_TRY_AGAIN;
	}
      }
    }
  }
  return 0;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerCodec_WorkOnOutRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_CODEC *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  if (xio->readRequestOut) {
    /* check for finished outbound read request */
    if (GWEN_Io_Request_GetStatus(xio->readRequestOut)==GWEN_Io_Request_StatusFinished) {
      uint32_t bpos;

      doneSomething=1;

      /* get all available data */
      bpos=GWEN_Io_Request_GetBufferPos(xio->readRequestOut);
      DBG_INFO(GWEN_LOGDOMAIN, "Read %d bytes into ringbuffer (now %d)",
	       bpos, GWEN_RingBuffer_GetUsedBytes(xio->readBuffer)+bpos);
      GWEN_RingBuffer_SkipBytesWrite(xio->readBuffer, bpos);

      /* save result code */
      xio->lastReadOutResult=GWEN_Io_Request_GetResultCode(xio->readRequestOut);
      GWEN_Io_Request_free(xio->readRequestOut);
      xio->readRequestOut=NULL;
    }
  }

  if (xio->writeRequestOut) {
    /* check for finished outbound write request */
    if (GWEN_Io_Request_GetStatus(xio->writeRequestOut)==GWEN_Io_Request_StatusFinished) {
      uint32_t bpos;

      doneSomething=1;

      /* handle all available data */
      bpos=GWEN_Io_Request_GetBufferPos(xio->writeRequestOut);
      DBG_INFO(GWEN_LOGDOMAIN, "Written %d bytes from ringbuffer", bpos);
      GWEN_RingBuffer_SkipBytesRead(xio->writeBuffer, bpos);

      /* save result code */
      xio->lastWriteOutResult=GWEN_Io_Request_GetResultCode(xio->writeRequestOut);
      GWEN_Io_Request_free(xio->writeRequestOut);
      xio->writeRequestOut=NULL;
    }
  }

  if (xio->writeRequestOut==NULL && xio->lastWriteOutResult==0 && xio->writeBuffer) {
    uint32_t len;

    len=GWEN_RingBuffer_GetMaxUnsegmentedRead(xio->writeBuffer);
    if (len!=0) {
      GWEN_IO_REQUEST *br;
      int rv;

      /* write buffer not empty, flush it if possible */
      DBG_DEBUG(GWEN_LOGDOMAIN, "Write buffer is not empty, flushing");
      br=GWEN_Io_Request_new(GWEN_Io_Request_TypeWrite,
			     (uint8_t*)GWEN_RingBuffer_GetReadPointer(xio->writeBuffer), len,
			     NULL, NULL,
			     xio->currentGuiId);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Adding write request (%d bytes)", len);
      rv=GWEN_Io_Layer_AddRequest(GWEN_Io_Layer_GetBaseLayer(io), br);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	if (rv!=GWEN_ERROR_TRY_AGAIN) {
	  xio->lastWriteOutResult=rv;
	  doneSomething=1;
	}
	GWEN_Io_Request_free(br);
      }
      else {
	xio->writeRequestOut=br;
	doneSomething=1;
      }
    }
  }

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



int GWEN_Io_LayerCodec_Encode(GWEN_IO_LAYER *io, const uint8_t *pBuffer, uint32_t lBuffer) {
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  if (xio->encodeFn)
    return xio->encodeFn(io, pBuffer, lBuffer);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Io_LayerCodec_Decode(GWEN_IO_LAYER *io, uint8_t *pBuffer, uint32_t lBuffer) {
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  if (xio->decodeFn)
    return xio->decodeFn(io, pBuffer, lBuffer);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerCodec_WorkOnReadRequest(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_CODEC *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  if (xio->readRequestIn) {
    GWEN_IO_REQUEST *r;
    uint32_t maxBytes;
    uint32_t bpos;
    uint8_t *dst;
    int rv;

    r=xio->readRequestIn;
    xio->currentGuiId=GWEN_Io_Request_GetGuiId(r);

    /* read raw data */
    bpos=GWEN_Io_Request_GetBufferPos(r);
    dst=GWEN_Io_Request_GetBufferPtr(r)+bpos;
    maxBytes=GWEN_Io_Request_GetBufferSize(r)-bpos;
    rv=GWEN_Io_LayerCodec_Decode(io, dst, maxBytes);
    if (rv<0) {
      if (rv!=GWEN_ERROR_TRY_AGAIN) {
	xio->readRequestIn=NULL;
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, rv);
	GWEN_Io_Request_free(r);
	doneSomething=1;
      }
    }
    else {
      doneSomething=1;
      if (rv) {
	bpos+=rv;
	GWEN_Io_Request_SetBufferPos(r, bpos);
	xio->readRequestIn=NULL;

	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_EOF);
      }
      GWEN_Io_Request_free(r);
    }
  }

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerCodec_WorkOnWriteRequest(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_CODEC *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  if (xio->writeRequestIn) {
    GWEN_IO_REQUEST *r;
    uint32_t maxBytes;
    uint32_t bpos;
    const uint8_t *src;
    int rv;

    r=xio->writeRequestIn;
    xio->currentGuiId=GWEN_Io_Request_GetGuiId(r);

    /* read raw data */
    bpos=GWEN_Io_Request_GetBufferPos(r);
    src=GWEN_Io_Request_GetBufferPtr(r)+bpos;
    maxBytes=GWEN_Io_Request_GetBufferSize(r)-bpos;
    rv=GWEN_Io_LayerCodec_Encode(io, src, maxBytes);
    if (rv<0) {
      if (rv!=GWEN_ERROR_TRY_AGAIN) {
	xio->writeRequestIn=NULL;
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, rv);
	GWEN_Io_Request_free(r);
	doneSomething=1;
      }
    }
    else {
      doneSomething=1;
      bpos+=rv;
      GWEN_Io_Request_SetBufferPos(r, bpos);
      xio->writeRequestIn=NULL;
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
      GWEN_Io_Request_free(r);
    }
  }

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerCodec_WorkOnRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_CODEC *xio;
  GWEN_IO_LAYER *baseLayer;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  baseLayer=GWEN_Io_Layer_GetBaseLayer(io);

  if (GWEN_Io_LayerCodec_WorkOnOutRequests(io)!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;
  if (GWEN_Io_LayerCodec_WorkOnReadRequest(io)!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;
  if (GWEN_Io_LayerCodec_WorkOnWriteRequest(io)!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;
  if (GWEN_Io_LayerCodec_WorkOnOutRequests(io)!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;
  if (baseLayer && GWEN_Io_Layer_WorkOnRequests(baseLayer)!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



void GWEN_Io_LayerCodec_AbortRequests(GWEN_IO_LAYER *io, int errorCode) {
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  /* abort inbound requests */
  if (xio->readRequestIn) {
    GWEN_IO_REQUEST *r;

    r=xio->readRequestIn;
    xio->readRequestIn=NULL;
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, errorCode);
    GWEN_Io_Request_free(r);
  }
  if (xio->writeRequestIn) {
    GWEN_IO_REQUEST *r;

    r=xio->writeRequestIn;
    xio->writeRequestIn=NULL;
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, errorCode);
    GWEN_Io_Request_free(r);
  }

  /* abort outbound requests */
  if (xio->readRequestOut) {
    GWEN_IO_REQUEST *r;

    r=xio->readRequestOut;
    GWEN_Io_Layer_DelRequest(GWEN_Io_Layer_GetBaseLayer(io), r);
    xio->readRequestOut=NULL;
    GWEN_Io_Request_free(r);
  }
  if (xio->writeRequestIn) {
    GWEN_IO_REQUEST *r;

    r=xio->writeRequestIn;
    GWEN_Io_Layer_DelRequest(GWEN_Io_Layer_GetBaseLayer(io), r);
    xio->writeRequestIn=NULL;
    GWEN_Io_Request_free(r);
  }
}



void GWEN_Io_LayerCodec_Reset(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  /* abort incoming requests, if any */
  GWEN_Io_LayerCodec_AbortInRequests(io, GWEN_ERROR_ABORTED);
  GWEN_Io_LayerCodec_AbortOutRequests(io);

  /* free ring buffers */
  GWEN_RingBuffer_free(xio->readBuffer);
  xio->readBuffer=NULL;
  GWEN_RingBuffer_free(xio->writeBuffer);
  xio->writeBuffer=NULL;
  xio->lastReadOutResult=0;
  xio->lastWriteOutResult=0;
}



GWEN_IO_LAYER_CODE_ENCODE_FN GWEN_Io_LayerCodec_SetEncodeFn(GWEN_IO_LAYER *io,
							    GWEN_IO_LAYER_CODE_ENCODE_FN f) {
  GWEN_IO_LAYER_CODE_ENCODE_FN of;
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  of=xio->encodeFn;
  xio->encodeFn=f;

  return of;
}



GWEN_IO_LAYER_CODE_DECODE_FN GWEN_Io_LayerCodec_SetDecodeFn(GWEN_IO_LAYER *io,
							    GWEN_IO_LAYER_CODE_DECODE_FN f) {
  GWEN_IO_LAYER_CODE_DECODE_FN of;
  GWEN_IO_LAYER_CODEC *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_CODEC, io);
  assert(xio);

  of=xio->decodeFn;
  xio->decodeFn=f;

  return of;
}








