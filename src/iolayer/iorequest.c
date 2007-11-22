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


#include "iorequest_p.h"

#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <assert.h>



GWEN_LIST_FUNCTIONS(GWEN_IO_REQUEST, GWEN_Io_Request)
GWEN_LIST2_FUNCTIONS(GWEN_IO_REQUEST, GWEN_Io_Request)



const char *GWEN_Io_RequestType_toString(GWEN_IO_REQUEST_TYPE t) {
  switch(t) {
  case GWEN_Io_Request_TypeConnect:     return "connect";
  case GWEN_Io_Request_TypeDisconnect:  return "disconnect";
  case GWEN_Io_Request_TypeRead:        return "read";
  case GWEN_Io_Request_TypeWrite:       return "write";
  default:                              return "unknown";
  }
}



const char *GWEN_Io_RequestStatus_toString(GWEN_IO_REQUEST_STATUS st) {
  switch(st) {
  case GWEN_Io_Request_StatusFree:     return "free";
  case GWEN_Io_Request_StatusEnqueued: return "enqueued";
  case GWEN_Io_Request_StatusFinished: return "finished";
  default:                             return "unknown";
  }
}




GWEN_IO_REQUEST *GWEN_Io_Request_new(GWEN_IO_REQUEST_TYPE t,
				     uint8_t *pBuffer,
				     uint32_t lBuffer,
				     GWEN_IO_REQUEST_FINISH_FN finishFn,
				     void *user_data,
				     uint32_t guiid) {
  GWEN_IO_REQUEST *r;

  GWEN_NEW_OBJECT(GWEN_IO_REQUEST, r);
  r->refCount=1;
  GWEN_LIST_INIT(GWEN_IO_REQUEST, r);

  r->type=t;
  r->bufferPtr=pBuffer;
  r->bufferSize=lBuffer;
  r->finishFn=finishFn;
  r->user_data=user_data;
  r->guiid=guiid;

  DBG_DEBUG(GWEN_LOGDOMAIN,
	    "Request %p created (%s, %d)",
	    r,
	    GWEN_Io_RequestType_toString(r->type),
	    lBuffer);

  return r;
}



void GWEN_Io_Request_Attach(GWEN_IO_REQUEST *r) {
  assert(r);
  assert(r->refCount);
  r->refCount++;
}



void GWEN_Io_Request_free(GWEN_IO_REQUEST *r) {
  if (r) {
    assert(r->refCount);
    if (r->refCount==1) {
      GWEN_LIST_FINI(GWEN_IO_REQUEST, r);
      if (r->incomingLayer)
	GWEN_Io_Layer_free(r->incomingLayer);
      if (r->flags & GWEN_IO_REQUEST_FLAGS_TAKEOVER)
        free(r->bufferPtr);
      r->refCount=0;
      GWEN_FREE_OBJECT(r);
    }
    else
      r->refCount--;
  }
}



GWEN_IO_REQUEST_TYPE GWEN_Io_Request_GetType(const GWEN_IO_REQUEST *r) {
  assert(r);
  assert(r->refCount);

  return r->type;
}



uint32_t GWEN_Io_Request_GetFlags(const GWEN_IO_REQUEST *r) {
  assert(r);
  assert(r->refCount);

  return r->flags;
}



void GWEN_Io_Request_SetFlags(GWEN_IO_REQUEST *r, uint32_t f) {
  assert(r);
  assert(r->refCount);

  r->flags=f;
}



void GWEN_Io_Request_AddFlags(GWEN_IO_REQUEST *r, uint32_t f) {
  assert(r);
  assert(r->refCount);

  r->flags|=f;
}



void GWEN_Io_Request_SubFlags(GWEN_IO_REQUEST *r, uint32_t f) {
  assert(r);
  assert(r->refCount);

  r->flags&=~f;
}



uint8_t *GWEN_Io_Request_GetBufferPtr(const GWEN_IO_REQUEST *r) {
  assert(r);
  assert(r->refCount);

  return r->bufferPtr;
}



uint32_t GWEN_Io_Request_GetBufferSize(const GWEN_IO_REQUEST *r) {
  assert(r);
  assert(r->refCount);

  return r->bufferSize;
}



uint32_t GWEN_Io_Request_GetBufferPos(const GWEN_IO_REQUEST *r) {
  assert(r);
  assert(r->refCount);

  return r->bufferPos;
}



void GWEN_Io_Request_SetBufferPos(GWEN_IO_REQUEST *r, uint32_t i) {
  assert(r);
  assert(r->refCount);

  r->bufferPos=i;
}



uint32_t GWEN_Io_Request_GetGuiId(const GWEN_IO_REQUEST *r) {
  assert(r);
  assert(r->refCount);

  return r->guiid;
}



void GWEN_Io_Request_SetGuiId(GWEN_IO_REQUEST *r, uint32_t i) {
  assert(r);
  assert(r->refCount);

  r->guiid=i;
}



GWEN_IO_LAYER *GWEN_Io_Request_GetIoLayer(const GWEN_IO_REQUEST *r) {
  assert(r);
  assert(r->refCount);

  return r->ioLayer;
}



GWEN_IO_REQUEST_STATUS GWEN_Io_Request_GetStatus(const GWEN_IO_REQUEST *r) {
  assert(r);
  assert(r->refCount);

  return r->status;
}



void GWEN_Io_Request_SetStatus(GWEN_IO_REQUEST *r, GWEN_IO_REQUEST_STATUS st) {
  assert(r);
  assert(r->refCount);

  r->status=st;
}



int GWEN_Io_Request_GetResultCode(const GWEN_IO_REQUEST *r) {
  assert(r);
  assert(r->refCount);

  return r->resultCode;
}



void GWEN_Io_Request_SetResultCode(GWEN_IO_REQUEST *r, int result) {
  assert(r);
  assert(r->refCount);

  r->resultCode=result;
}



GWEN_IO_LAYER *GWEN_Io_Request_GetIncomingLayer(const GWEN_IO_REQUEST *r) {
  assert(r);
  assert(r->refCount);

  return r->incomingLayer;
}



GWEN_IO_LAYER *GWEN_Io_Request_TakeIncomingLayer(GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER *newIo;

  assert(r);
  assert(r->refCount);

  newIo=r->incomingLayer;
  r->incomingLayer=NULL;
  return newIo;
}



void GWEN_Io_Request_SetIncomingLayer(GWEN_IO_REQUEST *r, GWEN_IO_LAYER *iol) {
  assert(r);
  assert(r->refCount);

  r->incomingLayer=iol;
}



void GWEN_Io_Request_SetIoLayer(GWEN_IO_REQUEST *r, GWEN_IO_LAYER *io) {
  assert(r);
  assert(r->refCount);

  r->ioLayer=io;
}



void GWEN_Io_Request_Finished(GWEN_IO_REQUEST *r, GWEN_IO_REQUEST_STATUS st, int result) {
  assert(r);
  assert(r->refCount);

  DBG_DEBUG(GWEN_LOGDOMAIN,
	    "Request %p finished (%s, %d/%d, result=%d)",
	    r,
	    GWEN_Io_RequestType_toString(r->type),
	    r->bufferPos,
	    r->bufferSize,
	    result);
  GWEN_Io_Request_SetStatus(r, st);
  GWEN_Io_Request_SetResultCode(r, result);
  if (r->finishFn)
    r->finishFn(r, r->user_data);
}











