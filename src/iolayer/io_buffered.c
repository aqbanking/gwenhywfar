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


#include "io_buffered_p.h"
#include <gwenhywfar/iolayer_be.h>
#include <gwenhywfar/iorequest_be.h>
#include <gwenhywfar/iomanager.h>

#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <assert.h>



GWEN_INHERIT(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED)



GWEN_IO_LAYER *GWEN_Io_LayerBuffered_new(GWEN_IO_LAYER *baseLayer) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_BUFFERED *xio;

  io=GWEN_Io_Layer_new(GWEN_IO_LAYER_BUFFERED_TYPE, baseLayer);
  assert(io);
  GWEN_NEW_OBJECT(GWEN_IO_LAYER_BUFFERED, xio);
  assert(xio);
  GWEN_INHERIT_SETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED, io, xio, GWEN_Io_LayerBuffered_freeData);

  GWEN_Io_Layer_SetWorkOnRequestsFn(io, GWEN_Io_LayerBuffered_WorkOnRequests);
  GWEN_Io_Layer_SetAddRequestFn(io, GWEN_Io_LayerBuffered_AddRequest);
  GWEN_Io_Layer_SetDelRequestFn(io, GWEN_Io_LayerBuffered_DelRequest);
  GWEN_Io_Layer_SetHasWaitingRequestsFn(io, GWEN_Io_LayerBuffered_HasWaitingRequests);

  /* this io layer is always initially connected */
  GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusConnected);

  return io;
}



GWENHYWFAR_CB
void GWEN_Io_LayerBuffered_freeData(void *bp, void *p) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_BUFFERED *xio;

  io=(GWEN_IO_LAYER*) bp;
  assert(io);
  xio=(GWEN_IO_LAYER_BUFFERED*) p;
  assert(xio);

  GWEN_Io_LayerBuffered_AbortInRequests(io, GWEN_ERROR_ABORTED);
  GWEN_Io_LayerBuffered_AbortOutRequests(io);

  GWEN_RingBuffer_free(xio->readBuffer);
  GWEN_RingBuffer_free(xio->writeBuffer);

  GWEN_FREE_OBJECT(xio);
}



void GWEN_Io_LayerBuffered_AbortInRequests(GWEN_IO_LAYER *io, int errorCode) {
  GWEN_IO_LAYER_BUFFERED *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED, io);
  assert(xio);

  if (xio->readRequestIn) {
    GWEN_IO_REQUEST *r;

    r=xio->readRequestIn;
    xio->readRequestIn=NULL;
    DBG_INFO(GWEN_LOGDOMAIN, "Aborting in read request");
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, errorCode);
    GWEN_Io_Request_free(r);
  }
  if (xio->writeRequestIn) {
    GWEN_IO_REQUEST *r;

    r=xio->writeRequestIn;
    xio->writeRequestIn=NULL;
    DBG_INFO(GWEN_LOGDOMAIN, "Aborting in write request");
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, errorCode);
    GWEN_Io_Request_free(r);
  }
}



void GWEN_Io_LayerBuffered_AbortOutRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_BUFFERED *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED, io);
  assert(xio);

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



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerBuffered_WorkOnReadRequest(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_BUFFERED *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED, io);
  assert(xio);

  if (xio->readRequestOut) {
    /* check for finished outbound read request */
    if (GWEN_Io_Request_GetStatus(xio->readRequestOut)==GWEN_Io_Request_StatusFinished) {
      uint32_t bpos;

      doneSomething=1;

      /* get all available data */
      bpos=GWEN_Io_Request_GetBufferPos(xio->readRequestOut);
      if (bpos==0) {
	DBG_INFO(GWEN_LOGDOMAIN, "No bytes read");
      }
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Read %d bytes into ringbuffer", bpos);
      GWEN_RingBuffer_SkipBytesWrite(xio->readBuffer, bpos);

      /* save result code */
      xio->lastReadOutResult=GWEN_Io_Request_GetResultCode(xio->readRequestOut);

      GWEN_Io_Request_free(xio->readRequestOut);
      xio->readRequestOut=NULL;
    }
  }

  if (xio->readRequestIn) {
    uint32_t len;
    GWEN_IO_REQUEST *r;
    uint32_t rflags;

    r=xio->readRequestIn;
    if (xio->readBuffer==NULL)
      xio->readBuffer=GWEN_RingBuffer_new(GWEN_IO_LAYER_BUFFERED_BUFSIZE);

    rflags=GWEN_Io_Request_GetFlags(r);

    /* now check whether we have some data */
    len=GWEN_RingBuffer_GetMaxUnsegmentedRead(xio->readBuffer);
    if (len==0) {
      /* empty read buffer, fill it if possible */
      if (xio->lastReadOutResult) {
	xio->readRequestIn=NULL;
	DBG_INFO(GWEN_LOGDOMAIN, "Aborting in read request (reason: %d)",
		 xio->lastReadOutResult);
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, xio->lastReadOutResult);
	GWEN_Io_Request_free(r);
	doneSomething=1;
      }
      else {
	if (xio->readRequestOut==NULL) {
	  GWEN_IO_REQUEST *br;
	  int rv;

	  /* there is no outbound read request, create one */
	  len=GWEN_RingBuffer_GetMaxUnsegmentedWrite(xio->readBuffer);
	  assert(len);

	  br=GWEN_Io_Request_new(GWEN_Io_Request_TypeRead,
				 (uint8_t*)GWEN_RingBuffer_GetWritePointer(xio->readBuffer), len,
				 NULL, NULL,
				 GWEN_Io_Request_GetGuiId(xio->readRequestIn));
	  rv=GWEN_Io_Layer_AddRequest(GWEN_Io_Layer_GetBaseLayer(io), br);
	  if (rv) {
	    GWEN_Io_Request_free(br);
	    if (rv!=GWEN_ERROR_TRY_AGAIN) {
	      xio->lastReadOutResult=rv;
	      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	      xio->readRequestIn=NULL;
	      DBG_INFO(GWEN_LOGDOMAIN, "Aborting in read request (reason: %d)",
		       xio->lastReadOutResult);
	      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, rv);
	      GWEN_Io_Request_free(r);
	    }
	  }
	  else {
	    xio->readRequestOut=br;
	  }
	  doneSomething=1;
	}
      }
    } /* if read buffer empty */
    else {
      const uint8_t *src;

      /* read buffer is not empty, so we *will* do something */
      doneSomething=1;

      src=(const uint8_t*)GWEN_RingBuffer_GetReadPointer(xio->readBuffer);
      if (rflags & GWEN_IO_REQUEST_BUFFERED_FLAGS_RAW) {
	uint32_t maxBytes;
	uint32_t bpos;
	uint8_t *dst;

	/* read raw data */
	bpos=GWEN_Io_Request_GetBufferPos(r);
	dst=GWEN_Io_Request_GetBufferPtr(r)+bpos;
	maxBytes=GWEN_Io_Request_GetBufferSize(r)-bpos;
	if (maxBytes>len)
	  maxBytes=len;
	if (maxBytes) {
	  memmove(dst, src, maxBytes);
	  if (!(rflags & GWEN_IO_REQUEST_BUFFERED_FLAGS_PEEK)) {
	    /* just peek, don't update */
	    GWEN_RingBuffer_SkipBytesRead(xio->readBuffer, maxBytes);
	  }
	  bpos+=maxBytes;
	  GWEN_Io_Request_SetBufferPos(r, bpos);
	}

	/* check whether request has been fullfilled, ignore possible flag GWEN_IO_REQUEST_FLAGS_READALL
	 * when peeking */
	if (!(rflags & GWEN_IO_REQUEST_FLAGS_READALL) ||
	    (rflags & GWEN_IO_REQUEST_BUFFERED_FLAGS_PEEK) ||
	    bpos>=GWEN_Io_Request_GetBufferSize(r)) {
	  /* request completed */
	  xio->readRequestIn=NULL;
	  GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	  GWEN_Io_Request_free(r);
	}
      }
      else {
	uint32_t maxBytes;
	uint32_t bpos;
	uint32_t i;
	uint8_t *dst;

	/* read line */
	bpos=GWEN_Io_Request_GetBufferPos(r);
	dst=GWEN_Io_Request_GetBufferPtr(r)+bpos;
	maxBytes=GWEN_Io_Request_GetBufferSize(r)-bpos;
	i=0;

	if (rflags & GWEN_IO_REQUEST_BUFFERED_FLAGS_PEEK) {
	  /* just peek, don't update counters etc */
	  while(i<len && bpos<maxBytes) {
	    if (*src==10) {
	      GWEN_Io_Request_AddFlags(r, GWEN_IO_REQUEST_FLAGS_PACKETEND);
	      GWEN_Io_Request_SetBufferPos(r, bpos);
	      xio->readRequestIn=NULL;
	      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	      GWEN_Io_Request_free(r);
	      break;
	    }
	    else if (*src!=13) {
	      *(dst++)=*src;
	      bpos++;
	    }
	    i++;
	    src++;
	  }
	  if (xio->readRequestIn && bpos>=GWEN_Io_Request_GetBufferSize(xio->readRequestIn)) {
	    GWEN_Io_Request_SetBufferPos(r, bpos);
	    xio->readRequestIn=NULL;
	    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	    GWEN_Io_Request_free(r);
	  }
	}
	else {
	  while(i<len && bpos<maxBytes) {
	    if (*src==10) {
	      xio->lastReadWasPacketEnd=1;
	      xio->readLineCount++;
	      GWEN_Io_Request_AddFlags(r, GWEN_IO_REQUEST_FLAGS_PACKETEND);
	      GWEN_Io_Request_SetBufferPos(r, bpos);
	      if (xio->readLinePos==0 &&
		  (GWEN_Io_Layer_GetFlags(io) & GWEN_IO_REQUEST_BUFFERED_FLAGS_UNTILEMPTYLINE)) {
		xio->lastReadOutResult=GWEN_ERROR_EOF;
		xio->readLinePos=0;
		xio->readRequestIn=NULL;
		GWEN_Io_Request_SetBufferPos(r, bpos);
		DBG_INFO(GWEN_LOGDOMAIN,
			 "Aborting in read request (reason: %d)",
			 xio->lastReadOutResult);
		GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_EOF);
		GWEN_Io_Request_free(r);
	      }
	      else {
		xio->readLinePos=0;
		xio->readRequestIn=NULL;
		GWEN_Io_Request_SetBufferPos(r, bpos);
		GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
		GWEN_Io_Request_free(r);
	      }
              i++;
	      break;
	    }
	    else if (*src!=13) {
	      *(dst++)=*src;
	      bpos++;
	    }
	    xio->readLinePos++;
	    i++;
	    src++;
	  }
	  GWEN_RingBuffer_SkipBytesRead(xio->readBuffer, i);

	  if (xio->readRequestIn) {
	    GWEN_Io_Request_SetBufferPos(r, bpos);

	    if (bpos>=GWEN_Io_Request_GetBufferSize(xio->readRequestIn)) {
	      /* there still is a read request and its buffer is filled completely without reaching EOLN */
	      xio->lastReadWasPacketEnd=0;
	      xio->readRequestIn=NULL;
	      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	      GWEN_Io_Request_free(r);
	    }
	  }
	}
      }
    }
  }

  if (GWEN_Io_Layer_GetStatus(io)==GWEN_Io_Layer_StatusListening) {
    GWEN_IO_LAYER *newIo;

    newIo=GWEN_Io_Layer_GetNextIncomingLayer(GWEN_Io_Layer_GetBaseLayer(io));
    if (newIo) {
      GWEN_IO_LAYER *newNewIo;
      uint32_t fl;

      fl=GWEN_Io_Layer_GetFlags(io);
      newNewIo=GWEN_Io_LayerBuffered_new(newIo);
      GWEN_Io_Layer_AddFlags(newNewIo, GWEN_IO_LAYER_FLAGS_PASSIVE);
      GWEN_Io_Layer_AddFlags(newNewIo, fl & 0xffff);
      GWEN_Io_Layer_AddIncomingLayer(io, newNewIo);
      doneSomething=1;
    }
  }

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



int GWEN_Io_LayerBuffered_TryFlush(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_BUFFERED *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED, io);
  assert(xio);

  if (xio->writeBuffer!=NULL) {
    uint32_t len;

    len=GWEN_RingBuffer_GetMaxUnsegmentedRead(xio->writeBuffer);
    if (len!=0) {
      /* write buffer not empty, flush it if possible */
      if (xio->writeRequestOut==NULL) {
	if (xio->lastWriteOutResult)
	  return xio->lastWriteOutResult;
	else {
	  GWEN_IO_REQUEST *br;
	  int rv;

	  /* there is no outbound write request, create one */
	  len=GWEN_RingBuffer_GetMaxUnsegmentedRead(xio->writeBuffer);
	  assert(len);

	  br=GWEN_Io_Request_new(GWEN_Io_Request_TypeWrite,
				 (uint8_t*)GWEN_RingBuffer_GetReadPointer(xio->writeBuffer), len,
				 NULL, NULL,
				 GWEN_Io_Request_GetGuiId(xio->writeRequestIn));
	  DBG_DEBUG(GWEN_LOGDOMAIN, "Adding write request (%d bytes)", len);
	  rv=GWEN_Io_Layer_AddRequest(GWEN_Io_Layer_GetBaseLayer(io), br);
	  if (rv) {
	    if (rv!=GWEN_ERROR_TRY_AGAIN)
	      xio->lastWriteOutResult=rv;
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Io_Request_free(br);
	    return rv;
	  }
	  else {
	    xio->writeRequestOut=br;
	    return 0;
	  }
	}
      }
      else
	/* there already is a write request */
	return GWEN_ERROR_IN_PROGRESS;
    }
  }
  return GWEN_ERROR_NO_DATA;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerBuffered_WorkOnWriteRequest(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_BUFFERED *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED, io);
  assert(xio);

  if (xio->writeRequestOut) {
    /* check for finished outbound write request */
    if (GWEN_Io_Request_GetStatus(xio->writeRequestOut)==GWEN_Io_Request_StatusFinished) {
      uint32_t bpos;

      doneSomething=1;

      /* handle all available data */
      bpos=GWEN_Io_Request_GetBufferPos(xio->writeRequestOut);
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Written %d bytes into ringbuffer", bpos);
      GWEN_RingBuffer_SkipBytesRead(xio->writeBuffer, bpos);

      /* save result code */
      xio->lastWriteOutResult=GWEN_Io_Request_GetResultCode(xio->writeRequestOut);
      GWEN_Io_Request_free(xio->writeRequestOut);
      xio->writeRequestOut=NULL;
    }
  }

  if (xio->writeRequestIn) {
    uint32_t len;
    GWEN_IO_REQUEST *r;
    uint32_t rflags;

    r=xio->writeRequestIn;
    if (xio->writeBuffer==NULL)
      xio->writeBuffer=GWEN_RingBuffer_new(GWEN_IO_LAYER_BUFFERED_BUFSIZE);

    rflags=GWEN_Io_Request_GetFlags(r);

    len=GWEN_RingBuffer_GetMaxUnsegmentedWrite(xio->writeBuffer);
    if (len==0) {
      int rv;

      /* buffer full, try to flush */
      rv=GWEN_Io_LayerBuffered_TryFlush(io);
      if (rv==0) {
	doneSomething=1;
      }
      else {
	assert(rv!=GWEN_ERROR_NO_DATA);
	if (rv!=GWEN_ERROR_TRY_AGAIN &&
	    rv!=GWEN_ERROR_IN_PROGRESS) {
	  xio->writeRequestIn=NULL;
	  DBG_INFO(GWEN_LOGDOMAIN,
		   "Aborting in write request (reason: %d)",
		   rv);
	  GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, rv);
	  GWEN_Io_Request_free(r);
	  doneSomething=1;
	}
      }
    }
    else {
      uint8_t *dst;
      uint32_t maxBytes;
      uint32_t bpos;
      const uint8_t *src;

      /* write buffer is not full */
      dst=(uint8_t*)GWEN_RingBuffer_GetWritePointer(xio->writeBuffer);

      /* write all data (if any) */
      bpos=GWEN_Io_Request_GetBufferPos(r);
      src=GWEN_Io_Request_GetBufferPtr(r)+bpos;
      maxBytes=GWEN_Io_Request_GetBufferSize(r)-bpos;
      if (maxBytes>len)
	maxBytes=len;
      if (maxBytes) {
	memmove(dst, src, maxBytes);
	GWEN_RingBuffer_SkipBytesWrite(xio->writeBuffer, maxBytes);
	bpos+=maxBytes;
	GWEN_Io_Request_SetBufferPos(r, bpos);
	doneSomething=1;
      }

      if (bpos>=GWEN_Io_Request_GetBufferSize(r)) {
	int rv;

	/* request data written completely, now check whether we have to add CR/LF */
	if (rflags & GWEN_IO_REQUEST_FLAGS_PACKETEND) {
	  if (GWEN_Io_Layer_GetFlags(io) & GWEN_IO_LAYER_BUFFERED_FLAGS_DOSMODE)
	    rflags|=GWEN_IO_REQUEST_BUFFERED_FLAGS_WRITE13 | GWEN_IO_REQUEST_BUFFERED_FLAGS_WRITE10;
          else
	    rflags|=GWEN_IO_REQUEST_BUFFERED_FLAGS_WRITE10;
	  rflags&=~GWEN_IO_REQUEST_FLAGS_PACKETEND;
	  GWEN_Io_Request_SetFlags(r, rflags);
	  doneSomething=1;
	}

	if (rflags & GWEN_IO_REQUEST_BUFFERED_FLAGS_WRITE13) {
	  rv=GWEN_RingBuffer_WriteByte(xio->writeBuffer, 13);
	  if (rv==0) {
	    rflags&=~GWEN_IO_REQUEST_BUFFERED_FLAGS_WRITE13;
	    GWEN_Io_Request_SubFlags(r, GWEN_IO_REQUEST_BUFFERED_FLAGS_WRITE13);
	    doneSomething=1;
	  }
	}

	if (rflags & GWEN_IO_REQUEST_BUFFERED_FLAGS_WRITE10) {
	  rv=GWEN_RingBuffer_WriteByte(xio->writeBuffer, 10);
	  if (rv==0) {
	    rflags&=~GWEN_IO_REQUEST_BUFFERED_FLAGS_WRITE10;
	    GWEN_Io_Request_SubFlags(r, GWEN_IO_REQUEST_BUFFERED_FLAGS_WRITE10);
	    doneSomething=1;
	  }
	}
      }

      if (bpos>=GWEN_Io_Request_GetBufferSize(r) &&
	  !(rflags & GWEN_IO_REQUEST_BUFFERED_FLAGS_WRITE13) &&
	  !(rflags & GWEN_IO_REQUEST_BUFFERED_FLAGS_WRITE10)) {
	if (rflags & GWEN_IO_REQUEST_FLAGS_FLUSH) {
	  int rv;

	  /* flush requested, so do it */
	  rv=GWEN_Io_LayerBuffered_TryFlush(io);
	  if (rv==GWEN_ERROR_NO_DATA) {
	    /* flushed, request finished */
	    xio->writeRequestIn=NULL;
	    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	    GWEN_Io_Request_free(r);
	    doneSomething=1;
	  }
	  else if (rv==0) {
	    /* flush started */
	    doneSomething=1;
	  }
	  else if (rv!=GWEN_ERROR_TRY_AGAIN && rv!=GWEN_ERROR_IN_PROGRESS) {
	    xio->writeRequestIn=NULL;
	    DBG_INFO(GWEN_LOGDOMAIN,
		     "Aborting in write request (reason: %d)",
		     rv);
	    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, rv);
	    GWEN_Io_Request_free(r);
	    doneSomething=1;
	  }
	}
	else {
	  xio->writeRequestIn=NULL;
	  GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	  GWEN_Io_Request_free(r);
	  doneSomething=1;
	}
      }
    }
  }

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerBuffered_WorkOnRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_BUFFERED *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED, io);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "LayerBuffered: Working");

  /* work on read request */
  if (GWEN_Io_LayerBuffered_WorkOnReadRequest(io)!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;

  /* work on write request */
  if (GWEN_Io_LayerBuffered_WorkOnWriteRequest(io)!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;

  /* let base layer work */
  if (GWEN_Io_Layer_WorkOnRequests(GWEN_Io_Layer_GetBaseLayer(io))!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;

  if (GWEN_Io_Layer_GetStatus(io)==GWEN_Io_Layer_StatusListening) {
    GWEN_IO_LAYER *newIo;

    newIo=GWEN_Io_Layer_GetNextIncomingLayer(GWEN_Io_Layer_GetBaseLayer(io));
    if (newIo) {
      GWEN_IO_LAYER *newNewIo;

      newNewIo=GWEN_Io_LayerBuffered_new(newIo);
      GWEN_Io_Layer_AddIncomingLayer(io, newNewIo);
      doneSomething=1;
    }
  }

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



int GWEN_Io_LayerBuffered_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_BUFFERED *xio;
  GWEN_IO_LAYER_STATUS st;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED, io);
  assert(xio);

  st=GWEN_Io_Layer_GetStatus(io);

  switch(GWEN_Io_Request_GetType(r)) {
  case GWEN_Io_Request_TypeRead:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "File is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }

    /* return data from buffer if there is any */
    if (xio->lastReadOutResult &&
	(xio->readBuffer &&
	 GWEN_RingBuffer_GetMaxUnsegmentedRead(xio->readBuffer)==0)) {
      DBG_INFO(GWEN_LOGDOMAIN, "Unable to read (%d)", xio->lastReadOutResult);
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, xio->lastReadOutResult);
      return xio->lastReadOutResult;
    }

    /* check whether we already have a read request */
    if (xio->readRequestIn) {
      DBG_INFO(GWEN_LOGDOMAIN, "There already is a read request");
      return GWEN_ERROR_TRY_AGAIN;
    }

    /* check for begin of line */
    if (!(GWEN_Io_Request_GetFlags(r) && GWEN_IO_REQUEST_BUFFERED_FLAGS_RAW)) {
      if (xio->lastReadWasPacketEnd) {
	/* if the previous line read request ended at the end of a line then this line read request
	 * starts at the beginning of a new line. Therefore we set the PACKETBEGIN flag and clear
	 * the lastReadWasPacketEnd flag (since the line didn't end just yet). */
	GWEN_Io_Request_AddFlags(r, GWEN_IO_REQUEST_FLAGS_PACKETBEGIN);
	xio->lastReadWasPacketEnd=0;
      }
    }
    else
      /* when reading raw data the last line has definately ended, so the next line read will start
       * with a new line */
      xio->lastReadWasPacketEnd=0;

    /* enqueue request */
    xio->readRequestIn=r;
    GWEN_Io_Request_Attach(xio->readRequestIn);
    break;

  case GWEN_Io_Request_TypeWrite:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "File is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }

    /* check whether we already have a read request */
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
      xio->lastReadOutResult=0;
      GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusConnected);
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
      DBG_INFO(GWEN_LOGDOMAIN, "Layer now connected");
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
      GWEN_Io_LayerBuffered_AbortInRequests(io, GWEN_ERROR_ABORTED);
      GWEN_Io_LayerBuffered_AbortOutRequests(io);

      /* free ring buffers */
      GWEN_RingBuffer_free(xio->readBuffer);
      xio->readBuffer=NULL;
      GWEN_RingBuffer_free(xio->writeBuffer);
      xio->writeBuffer=NULL;

      xio->lastReadOutResult=0;

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



int GWEN_Io_LayerBuffered_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_BUFFERED *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED, io);
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



int GWEN_Io_LayerBuffered_HasWaitingRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_BUFFERED *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED, io);
  assert(xio);

  if (xio->readRequestIn || xio->writeRequestIn)
    return 1;
  else
    return 0;
}



int GWEN_Io_LayerBuffered_ReadLineToBuffer(GWEN_IO_LAYER *io, GWEN_BUFFER *fbuf, uint32_t guiid, int msecs) {
  for (;;) {
    GWEN_IO_REQUEST *r;
    GWEN_IO_REQUEST_STATUS st;
    uint8_t buffer[257];
    int rv;

    r=GWEN_Io_Request_new(GWEN_Io_Request_TypeRead,
			  buffer, sizeof(buffer)-1, NULL, NULL, guiid);
    rv=GWEN_Io_Layer_AddRequest(io, r);
    if (rv<0) {
      GWEN_Io_Request_free(r);
      if (rv==GWEN_ERROR_EOF) {
	return rv;
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }

    /* wait for request to finish */
    rv=GWEN_Io_Manager_WaitForRequest(r, msecs);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Io_Request_free(r);
      return rv;
    }

    /* if not finished, report an error */
    st=GWEN_Io_Request_GetStatus(r);
    if (st!=GWEN_Io_Request_StatusFinished) {
      DBG_INFO(GWEN_LOGDOMAIN, "Bad request status (%d)", st);
      GWEN_Io_Request_free(r);
      return GWEN_ERROR_INTERNAL;
    }
  
    /* check result code */
    rv=GWEN_Io_Request_GetResultCode(r);
    if (rv && rv!=GWEN_ERROR_EOF) {
      DBG_INFO(GWEN_LOGDOMAIN, "Result of request is an error (%d)", rv);
      GWEN_Io_Request_free(r);
      return rv;
    }

    if (GWEN_Io_Request_GetBufferPos(r))
      GWEN_Buffer_AppendBytes(fbuf, (const char*)buffer, GWEN_Io_Request_GetBufferPos(r));

    if (GWEN_Io_Request_GetFlags(r) & GWEN_IO_REQUEST_FLAGS_PACKETEND) {
      GWEN_Io_Request_free(r);
      break;
    }

    GWEN_Io_Request_free(r);
  }

  return 0;
}



int GWEN_Io_LayerBuffered_WriteLine(GWEN_IO_LAYER *io, const char *buffer, int len, int flush,
				    uint32_t guiid, int msecs) {
  int rv;
  uint32_t rflags;

  if (len==-1)
    len=strlen(buffer);

  rflags=GWEN_IO_REQUEST_FLAGS_PACKETEND|GWEN_IO_REQUEST_FLAGS_WRITEALL;
  if (flush)
    rflags|=GWEN_IO_REQUEST_FLAGS_FLUSH;
  rv=GWEN_Io_Layer_WriteBytes(io, (const uint8_t*)buffer, len, rflags, guiid, msecs);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }

  return rv;
}



uint32_t GWEN_Io_LayerBuffered_GetReadLineCount(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_BUFFERED *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED, io);
  assert(xio);

  return xio->readLineCount;
}



uint32_t GWEN_Io_LayerBuffered_GetReadLinePos(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_BUFFERED *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED, io);
  assert(xio);

  return xio->readLinePos;
}



void GWEN_Io_LayerBuffered_ResetLinePosAndCounter(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_BUFFERED *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_BUFFERED, io);
  assert(xio);

  xio->readLineCount=0;
  xio->readLinePos=0;
}













