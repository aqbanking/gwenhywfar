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


#include "io_packets_p.h"
#include <gwenhywfar/iolayer_be.h>
#include <gwenhywfar/iorequest_be.h>
#include <gwenhywfar/iomanager.h>
#include <gwenhywfar/io_buffered.h>

#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>

#include <assert.h>
#include <ctype.h>




GWEN_INHERIT(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS)




GWEN_IO_LAYER *GWEN_Io_LayerPackets_new(GWEN_IO_LAYER *baseLayer) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_PACKETS *xio;

  io=GWEN_Io_Layer_new(GWEN_IO_LAYER_PACKETS_TYPE, baseLayer);
  assert(io);
  GWEN_NEW_OBJECT(GWEN_IO_LAYER_PACKETS, xio);
  assert(xio);
  GWEN_INHERIT_SETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io, xio, GWEN_Io_LayerPackets_freeData);

  xio->requestsIn=GWEN_Io_Request_List_new();
  xio->requestsOut=GWEN_Io_Request_List_new();
  xio->readSize=GWEN_IO_LAYER_PACKETS_DEFAULT_READSIZE;
  xio->maxReadQueue=GWEN_IO_LAYER_PACKETS_DEFAULT_READQUEUE;
  xio->maxWriteQueue=GWEN_IO_LAYER_PACKETS_DEFAULT_WRITEQUEUE;

  GWEN_Io_Layer_SetWorkOnRequestsFn(io, GWEN_Io_LayerPackets_WorkOnRequests);
  GWEN_Io_Layer_SetAddRequestFn(io, GWEN_Io_LayerPackets_AddRequest);
  GWEN_Io_Layer_SetDelRequestFn(io, GWEN_Io_LayerPackets_DelRequest);
  GWEN_Io_Layer_SetHasWaitingRequestsFn(io, GWEN_Io_LayerPackets_HasWaitingRequests);

  return io;
}



GWENHYWFAR_CB
void GWEN_Io_LayerPackets_freeData(void *bp, void *p) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_PACKETS *xio;

  io=(GWEN_IO_LAYER*)bp;
  xio=(GWEN_IO_LAYER_PACKETS*)p;

  GWEN_Io_LayerPackets_Abort(io);
  GWEN_Io_Request_List_free(xio->requestsIn);
  GWEN_Io_Request_List_free(xio->requestsOut);

  GWEN_FREE_OBJECT(xio);
}



int GWEN_Io_LayerPackets_GetReadRequest(GWEN_IO_LAYER *io,
					GWEN_IO_REQUEST **pRequest,
					uint32_t guiid, int msecs) {
  GWEN_IO_LAYER_PACKETS *xio;
  GWEN_IO_REQUEST *r;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  r=GWEN_Io_Request_List_First(xio->requestsIn);
  if (r) {
    GWEN_Io_Request_List_Del(r);
    *pRequest=r;
    return 0;
  }
  else {
    if (msecs==GWEN_TIMEOUT_NONE)
      return GWEN_ERROR_TIMEOUT;
    else {
      uint32_t oldGuiid;
      int rv;
  
      if (xio->currentReadRequest==NULL)
	GWEN_Io_LayerPackets_WorkOnReadRequests(io);
      if (xio->currentReadRequest==NULL) {
	DBG_INFO(GWEN_LOGDOMAIN, "Could not generate read request");
	return GWEN_ERROR_IO;
      }
      r=xio->currentReadRequest;
      oldGuiid=GWEN_Io_Request_GetGuiId(r);
      GWEN_Io_Request_SetGuiId(r, guiid);
      /* let io manager wait for this request to finish */
      rv=GWEN_Io_Manager_WaitForRequest(r, msecs);
      GWEN_Io_Request_SetGuiId(r, oldGuiid);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
  
      /* now return the first request returned (there should be one now) */
      r=GWEN_Io_Request_List_First(xio->requestsIn);
      if (r) {
	GWEN_Io_Request_List_Del(r);
	*pRequest=r;
	return 0;
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "Still no read request");
	return GWEN_ERROR_IO;
      }
    }
  }
}



int GWEN_Io_LayerPackets_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_PACKETS *xio;
  GWEN_IO_LAYER_STATUS st;
  uint32_t lflags;
  uint32_t rflags;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  st=GWEN_Io_Layer_GetStatus(io);
  lflags=GWEN_Io_Layer_GetFlags(io);
  rflags=GWEN_Io_Request_GetFlags(r);

  DBG_INFO(GWEN_LOGDOMAIN, "Add %s request...",
	   GWEN_Io_RequestType_toString(GWEN_Io_Request_GetType(r)));

  switch(GWEN_Io_Request_GetType(r)) {
  case GWEN_Io_Request_TypeWrite:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "IO layer is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }

    if (GWEN_Io_Request_List_GetCount(xio->requestsOut)>=xio->maxWriteQueue)
      return GWEN_ERROR_TRY_AGAIN;
    GWEN_Io_Request_Attach(r);
    GWEN_Io_Request_List_Add(r, xio->requestsOut);
    break;

  case GWEN_Io_Request_TypeConnect:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusUnconnected &&
	st!=GWEN_Io_Layer_StatusDisconnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "Layer not un-/disconnected");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }
    GWEN_Io_Request_List_Clear(xio->requestsIn);
    GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusConnected);
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
    DBG_INFO(GWEN_LOGDOMAIN, "Layer now connected");
    break;

  case GWEN_Io_Request_TypeDisconnect:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "IO layer is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }
    else
      GWEN_Io_LayerPackets_Abort(io);
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
    DBG_INFO(GWEN_LOGDOMAIN, "Layer now disconnected");
    break;

  case GWEN_Io_Request_TypeRead:
  default:
    DBG_INFO(GWEN_LOGDOMAIN, "This request type is not supported (%d)", GWEN_Io_Request_GetType(r));
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_SUPPORTED);
    return GWEN_ERROR_NOT_SUPPORTED;
  }


  return 0;
}



int GWEN_Io_LayerPackets_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_PACKETS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  switch(GWEN_Io_Request_GetType(r)) {
  case GWEN_Io_Request_TypeWrite:
    if (xio->currentWriteRequest==r) {
      int rv;

      rv=GWEN_Io_Layer_DelRequest(GWEN_Io_Layer_GetBaseLayer(io), r);
      xio->currentWriteRequest=NULL;
      return rv;
    }
    else {
      if (GWEN_Io_Request_List_HasElement(xio->requestsOut, r)) {
	GWEN_Io_Request_List_Del(r);
      }
      else {
        DBG_INFO(GWEN_LOGDOMAIN, "Request is not enqeued here");
	return GWEN_ERROR_NOT_FOUND;
      }
    }
    break;

  default:
    break;
  }

  return 0;
}



int GWEN_Io_LayerPackets_GetReadSize(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_PACKETS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  return xio->readSize;
}



void GWEN_Io_LayerPackets_SetReadSize(GWEN_IO_LAYER *io, int i) {
  GWEN_IO_LAYER_PACKETS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  xio->readSize=i;
}



int GWEN_Io_LayerPackets_GetMaxReadQueue(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_PACKETS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  return xio->maxReadQueue;
}



void GWEN_Io_LayerPackets_SetMaxReadQueue(GWEN_IO_LAYER *io, int i) {
  GWEN_IO_LAYER_PACKETS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  xio->maxReadQueue=i;
}



int GWEN_Io_LayerPackets_GetMaxWriteQueue(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_PACKETS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  return xio->maxWriteQueue;
}



void GWEN_Io_LayerPackets_SetMaxWriteQueue(GWEN_IO_LAYER *io, int i) {
  GWEN_IO_LAYER_PACKETS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  xio->maxWriteQueue=i;
}



void GWEN_Io_LayerPackets_Abort(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_PACKETS *xio;
  GWEN_IO_REQUEST *r;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  if (xio->currentReadRequest) {
    GWEN_Io_Layer_DelRequest(GWEN_Io_Layer_GetBaseLayer(io), xio->currentReadRequest);
    GWEN_Io_Request_free(xio->currentReadRequest);
    xio->currentReadRequest=NULL;
  }
  if (xio->currentWriteRequest) {
    GWEN_Io_Layer_DelRequest(GWEN_Io_Layer_GetBaseLayer(io), xio->currentWriteRequest);
    GWEN_Io_Request_free(xio->currentWriteRequest);
    xio->currentWriteRequest=NULL;
  }

  /* don't clear list of incoming requests, these requests might still be interesting to the
   * caller (e.g. if these are notifications of some kind, or responses to write requests) */

  /* these requests need to be finished */
  r=GWEN_Io_Request_List_First(xio->requestsOut);
  while( (r=GWEN_Io_Request_List_First(xio->requestsOut)) ) {
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_ABORTED);
    GWEN_Io_Request_List_Del(r);
    GWEN_Io_Request_free(r);
  }

  GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerPackets_WorkOnReadRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_PACKETS *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  if (xio->currentReadRequest) {
    if (GWEN_Io_Request_GetStatus(xio->currentReadRequest)==GWEN_Io_Request_StatusFinished) {
      doneSomething=1;

      if (GWEN_Io_Request_GetFlags(xio->currentReadRequest) & GWEN_IO_REQUEST_FLAGS_PACKETEND) {
        DBG_INFO(GWEN_LOGDOMAIN, "Got a complete incoming packet");
	GWEN_Io_Request_List_Add(xio->currentReadRequest, xio->requestsIn);
	xio->currentReadRequest=NULL;
      }
      else {
	int res;

        res=GWEN_Io_Request_GetResultCode(xio->currentReadRequest);
	if (res) {
	  if (res==GWEN_ERROR_EOF) {
            DBG_INFO(GWEN_LOGDOMAIN, "EOF met, disconnecting");
	  }
	  else {
	    DBG_WARN(GWEN_LOGDOMAIN, "Incomplete request received, aborting connection");
	  }
	  GWEN_Io_Request_free(xio->currentReadRequest);
	  xio->currentReadRequest=NULL;
	  GWEN_Io_LayerPackets_Abort(io);
	}
      }
    }
  }

  if (xio->currentReadRequest==NULL && GWEN_Io_Layer_GetStatus(io)==GWEN_Io_Layer_StatusConnected) {
    if (GWEN_Io_Request_List_GetCount(xio->requestsIn)<xio->maxReadQueue) {
      uint8_t *buf;
      GWEN_IO_REQUEST *r;
      int rv;

      buf=(uint8_t*) malloc(xio->readSize);
      assert(buf);

      r=GWEN_Io_Request_new(GWEN_Io_Request_TypeRead,
			    buf, xio->readSize,
			    NULL, NULL, 0);
      assert(r);
      /* let the request take over the buffer, start new packet */
      GWEN_Io_Request_AddFlags(r,
			       GWEN_IO_REQUEST_FLAGS_TAKEOVER |
			       GWEN_IO_REQUEST_FLAGS_PACKETBEGIN);
      DBG_INFO(GWEN_LOGDOMAIN, "Trying to send read request");
      rv=GWEN_Io_Layer_AddRequest(GWEN_Io_Layer_GetBaseLayer(io), r);
      if (rv) {
	if (rv!=GWEN_ERROR_TRY_AGAIN) {
	  DBG_WARN(GWEN_LOGDOMAIN, "Error adding request to baselayer (%d), aborting", rv);
	  GWEN_Io_LayerPackets_Abort(io);
	  doneSomething=1;
	}
	GWEN_Io_Request_free(r);
      }
      else {
	xio->currentReadRequest=r;
	doneSomething=1;
      }
    }
  }

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerPackets_WorkOnWriteRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_PACKETS *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  if (xio->currentWriteRequest) {
    if (GWEN_Io_Request_GetStatus(xio->currentWriteRequest)==GWEN_Io_Request_StatusFinished) {
      doneSomething=1;
      GWEN_Io_Request_free(xio->currentWriteRequest);
      xio->currentWriteRequest=NULL;
    }
  }

  if (xio->currentWriteRequest==NULL && GWEN_Io_Layer_GetStatus(io)==GWEN_Io_Layer_StatusConnected) {
    GWEN_IO_REQUEST *r;

    r=GWEN_Io_Request_List_First(xio->requestsOut);
    if (r) {
      int rv;

      GWEN_Io_Request_AddFlags(r,
			       GWEN_IO_REQUEST_FLAGS_PACKETBEGIN |
			       GWEN_IO_REQUEST_FLAGS_PACKETEND |
			       GWEN_IO_REQUEST_FLAGS_WRITEALL |
			       GWEN_IO_REQUEST_FLAGS_FLUSH);
      DBG_INFO(GWEN_LOGDOMAIN, "Trying to send write request");
      rv=GWEN_Io_Layer_AddRequest(GWEN_Io_Layer_GetBaseLayer(io), r);
      if (rv) {
	if (rv!=GWEN_ERROR_TRY_AGAIN) {
	  DBG_WARN(GWEN_LOGDOMAIN, "Error adding request to baselayer (%d), aborting", rv);
	  GWEN_Io_LayerPackets_Abort(io);
	  doneSomething=1;
	}
      }
      else {
	xio->currentWriteRequest=r;
	/* remove from list (it is now pointed to by xio->currentWriteRequest) */
	GWEN_Io_Request_List_Del(r);
	doneSomething=1;
      }
    }
  }

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerPackets_WorkOnRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_PACKETS *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  /* work on read request */
  if (GWEN_Io_LayerPackets_WorkOnReadRequests(io)!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;

  /* work on write request */
  if (GWEN_Io_LayerPackets_WorkOnWriteRequests(io)!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;

  /* let base layer work */
  if (GWEN_Io_Layer_WorkOnRequests(GWEN_Io_Layer_GetBaseLayer(io))!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;

  if (GWEN_Io_Layer_GetStatus(io)==GWEN_Io_Layer_StatusListening) {
    GWEN_IO_LAYER *newIo;

    newIo=GWEN_Io_Layer_GetNextIncomingLayer(GWEN_Io_Layer_GetBaseLayer(io));
    if (newIo) {
      GWEN_IO_LAYER *newNewIo;

      newNewIo=GWEN_Io_LayerPackets_new(newIo);
      GWEN_Io_Layer_AddIncomingLayer(io, newNewIo);
      doneSomething=1;
    }
  }

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



int GWEN_Io_LayerPackets_HasWaitingRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_PACKETS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  if (xio->currentReadRequest ||
      xio->currentWriteRequest ||
      GWEN_Io_Request_List_GetCount(xio->requestsOut))
    /* don't count list of already read requests in xio->requestIn! */
    return 1;
  else
    return 0;
}



int GWEN_Io_LayerPackets_HasReadRequests(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_PACKETS *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_PACKETS, io);
  assert(xio);

  if (GWEN_Io_Request_List_GetCount(xio->requestsIn))
    return 1;
  return 0;
}









