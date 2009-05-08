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


#include "io_memory_p.h"
#include <gwenhywfar/iolayer_be.h>
#include <gwenhywfar/iorequest_be.h>

#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>




GWEN_INHERIT(GWEN_IO_LAYER, GWEN_IO_LAYER_MEMORY)



GWEN_IO_LAYER *GWEN_Io_LayerMemory_new(GWEN_BUFFER *buffer) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_MEMORY *xio;

  io=GWEN_Io_Layer_new(GWEN_IO_LAYER_MEMORY_TYPE, NULL);
  assert(io);
  GWEN_NEW_OBJECT(GWEN_IO_LAYER_MEMORY, xio);
  assert(xio);
  GWEN_INHERIT_SETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_MEMORY, io, xio, GWEN_Io_LayerMemory_freeData);

  GWEN_Io_Layer_SetWorkOnRequestsFn(io, GWEN_Io_LayerMemory_WorkOnRequests);
  GWEN_Io_Layer_SetAddRequestFn(io, GWEN_Io_LayerMemory_AddRequest);
  GWEN_Io_Layer_SetDelRequestFn(io, GWEN_Io_LayerMemory_DelRequest);
  GWEN_Io_Layer_SetHasWaitingRequestsFn(io, GWEN_Io_LayerMemory_HasWaitingRequests);

  xio->buffer=buffer;

  GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusConnected);

  return io;
}



GWEN_IO_LAYER *GWEN_Io_LayerMemory_fromString(const uint8_t *p, int size) {
  GWEN_BUFFER *buf;
  GWEN_IO_LAYER *io;

  if (size==-1)
    size=strlen((const char*)p);
  buf=GWEN_Buffer_new(0, size, 0, 1);
  GWEN_Buffer_AppendBytes(buf, (const char*)p, size);
  GWEN_Buffer_Rewind(buf);
  io=GWEN_Io_LayerMemory_new(buf);
  assert(io);
  GWEN_Io_Layer_AddFlags(io, GWEN_IO_LAYER_FLAGS_TAKEOVER);

  return io;
}



GWENHYWFAR_CB
void GWEN_Io_LayerMemory_freeData(void *bp, void *p) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_MEMORY *xio;

  io=(GWEN_IO_LAYER*) bp;
  assert(io);
  xio=(GWEN_IO_LAYER_MEMORY*) p;
  assert(xio);

  if (GWEN_Io_Layer_GetFlags(io) & GWEN_IO_LAYER_FLAGS_TAKEOVER) {
    GWEN_Buffer_free(xio->buffer);
    xio->buffer=(GWEN_BUFFER*)-1;
  }

  GWEN_FREE_OBJECT(xio);
}



GWEN_BUFFER *GWEN_Io_LayerMemory_GetBuffer(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_MEMORY *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_MEMORY, io);
  assert(xio);

  return xio->buffer;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerMemory_WorkOnRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_MEMORY *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_MEMORY, io);
  assert(xio);

  return GWEN_Io_Layer_WorkResultBlocking;
}



int GWEN_Io_LayerMemory_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_MEMORY *xio;
  GWEN_IO_LAYER_STATUS st;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_MEMORY, io);
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
    else {
      uint32_t bytes;
      uint32_t bytesLeft;
      uint32_t bpos;
      const uint8_t *src;
      uint8_t *dst;

      bpos=GWEN_Io_Request_GetBufferPos(r);
      dst=GWEN_Io_Request_GetBufferPtr(r);
      dst+=bpos;
      src=(const uint8_t*)GWEN_Buffer_GetPosPointer(xio->buffer);
      bytes=GWEN_Io_Request_GetBufferSize(r)-bpos;
      bytesLeft=GWEN_Buffer_GetBytesLeft(xio->buffer);
      if (bytesLeft==0) {
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_EOF);
	return GWEN_ERROR_EOF;
      }
      if (bytes>bytesLeft)
	bytes=bytesLeft;

      if (bytes) {
	memmove(dst, src, bytes);
	GWEN_Buffer_IncrementPos(xio->buffer, bytes);
        bpos+=bytes;
        GWEN_Io_Request_SetBufferPos(r, bpos);
      }

      if ((bpos>=GWEN_Io_Request_GetBufferSize(r)) ||
	  !(GWEN_Io_Request_GetFlags(r) & GWEN_IO_REQUEST_FLAGS_READALL)) {
	/* return bytes we have so far */
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	return 0;
      }
    }
    break;

  case GWEN_Io_Request_TypeWrite:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "IO layer is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }
    else {
      int rv=0;
      uint32_t bytes;

      bytes=GWEN_Io_Request_GetBufferSize(r);
      if (bytes)
	rv=GWEN_Buffer_AppendBytes(xio->buffer, (const char*)GWEN_Io_Request_GetBufferPtr(r), bytes);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, rv);
	return rv;
      }
      GWEN_Io_Request_SetBufferPos(r, bytes);
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
      return 0;
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



int GWEN_Io_LayerMemory_DelRequest(GWEN_UNUSED GWEN_IO_LAYER *io, GWEN_UNUSED GWEN_IO_REQUEST *r) {
  return GWEN_ERROR_INVALID;
}



int GWEN_Io_LayerMemory_HasWaitingRequests(GWEN_UNUSED GWEN_IO_LAYER *io) {
  return 0;
}














