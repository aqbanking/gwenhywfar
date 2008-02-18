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


#include "iolayer_p.h"
#include "iorequest_l.h"

#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/iomanager.h>

#include <assert.h>



GWEN_INHERIT_FUNCTIONS(GWEN_IO_LAYER)
GWEN_LIST_FUNCTIONS(GWEN_IO_LAYER, GWEN_Io_Layer)
GWEN_LIST2_FUNCTIONS(GWEN_IO_LAYER, GWEN_Io_Layer)





GWEN_IO_LAYER *GWEN_Io_Layer_new(const char *typeName, GWEN_IO_LAYER *baseLayer) {
  GWEN_IO_LAYER *io;

  assert(typeName);
  GWEN_NEW_OBJECT(GWEN_IO_LAYER, io);
  io->refCount=1;
  GWEN_INHERIT_INIT(GWEN_IO_LAYER, io);
  GWEN_LIST_INIT(GWEN_IO_LAYER, io);

  io->typeName=strdup(typeName);
  if (baseLayer) {
    io->baseLayer=baseLayer;
    GWEN_Io_Layer_Attach(io->baseLayer);
  }

  io->incomingLayers=GWEN_Io_Layer_List_new();

  return io;
}



void GWEN_Io_Layer_Attach(GWEN_IO_LAYER *io) {
  assert(io);
  assert(io->refCount);
  io->refCount++;
}



void GWEN_Io_Layer_free(GWEN_IO_LAYER *io) {
  if (io) {
    assert(io->refCount);
    if (io->refCount==1) {
      GWEN_LIST_FINI(GWEN_IO_LAYER, io);
      GWEN_INHERIT_FINI(GWEN_IO_LAYER, io);
      GWEN_Io_Layer_List_free(io->incomingLayers);
      GWEN_Io_Layer_free(io->baseLayer);
      free(io->typeName);
      io->refCount=0;
      GWEN_FREE_OBJECT(io);
    }
    else
      io->refCount--;
  }
}



GWEN_IO_LAYER *GWEN_Io_Layer_GetBaseLayer(GWEN_IO_LAYER *io) {
  assert(io);
  assert(io->refCount);

  return io->baseLayer;
}



GWEN_IO_LAYER *GWEN_Io_Layer_FindBaseLayerByType(GWEN_IO_LAYER *io, const char *tname) {
  assert(io);
  assert(io->refCount);

  assert(tname);

  while(io) {
    if (strcasecmp(io->typeName, tname)==0)
      break;
    io=io->baseLayer;
  }

  return io;
}



void GWEN_Io_Layer_AddIncomingLayer(GWEN_IO_LAYER *io, GWEN_IO_LAYER *newIo) {
  assert(io);
  assert(io->refCount);
  assert(newIo);
  assert(newIo->refCount);

  GWEN_Io_Layer_List_Add(newIo, io->incomingLayers);
}



GWEN_IO_LAYER *GWEN_Io_Layer_GetNextIncomingLayer(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER *newIo;

  assert(io);
  assert(io->refCount);

  newIo=GWEN_Io_Layer_List_First(io->incomingLayers);
  if (newIo)
    GWEN_Io_Layer_List_Del(newIo);
  return newIo;
}



void GWEN_Io_Layer_ClearIncomingLayers(GWEN_IO_LAYER *io) {
  assert(io);
  assert(io->refCount);

  GWEN_Io_Layer_List_Clear(io->incomingLayers);
}



const char *GWEN_Io_Layer_GetTypeName(const GWEN_IO_LAYER *io) {
  assert(io);
  assert(io->refCount);

  return io->typeName;
}



uint32_t GWEN_Io_Layer_GetFlags(const GWEN_IO_LAYER *io) {
  assert(io);
  assert(io->refCount);

  return io->flags;
}



void GWEN_Io_Layer_SetFlags(GWEN_IO_LAYER *io, uint32_t f) {
  assert(io);
  assert(io->refCount);

  io->flags=f;
}



void GWEN_Io_Layer_AddFlags(GWEN_IO_LAYER *io, uint32_t f) {
  assert(io);
  assert(io->refCount);

  io->flags|=f;
}



void GWEN_Io_Layer_SubFlags(GWEN_IO_LAYER *io, uint32_t f) {
  assert(io);
  assert(io->refCount);

  io->flags&=~f;
}



GWEN_IO_LAYER_STATUS GWEN_Io_Layer_GetStatus(const GWEN_IO_LAYER *io) {
  assert(io);
  assert(io->refCount);

  return io->status;
}



void GWEN_Io_Layer_SetStatus(GWEN_IO_LAYER *io, GWEN_IO_LAYER_STATUS st) {
  assert(io);
  assert(io->refCount);

  io->status=st;
}



GWEN_IO_LAYER_WORKONREQUESTS_FN GWEN_Io_Layer_SetWorkOnRequestsFn(GWEN_IO_LAYER *io, GWEN_IO_LAYER_WORKONREQUESTS_FN f) {
  GWEN_IO_LAYER_WORKONREQUESTS_FN of;

  assert(io);
  assert(io->refCount);

  of=io->workOnRequestsFn;
  io->workOnRequestsFn=f;

  return of;
}



GWEN_IO_LAYER_ADDREQUEST_FN GWEN_Io_Layer_SetAddRequestFn(GWEN_IO_LAYER *io, GWEN_IO_LAYER_ADDREQUEST_FN f) {
  GWEN_IO_LAYER_ADDREQUEST_FN of;

  assert(io);
  assert(io->refCount);

  of=io->addRequestFn;
  io->addRequestFn=f;

  return of;
}



GWEN_IO_LAYER_DELREQUEST_FN GWEN_Io_Layer_SetDelRequestFn(GWEN_IO_LAYER *io, GWEN_IO_LAYER_DELREQUEST_FN f) {
  GWEN_IO_LAYER_DELREQUEST_FN of;

  assert(io);
  assert(io->refCount);

  of=io->delRequestFn;
  io->delRequestFn=f;

  return of;
}



GWEN_IO_LAYER_HASWAITINGREQUESTS_FN GWEN_Io_Layer_SetHasWaitingRequestsFn(GWEN_IO_LAYER *io,
									  GWEN_IO_LAYER_HASWAITINGREQUESTS_FN f) {
  GWEN_IO_LAYER_HASWAITINGREQUESTS_FN of;

  assert(io);
  assert(io->refCount);

  of=io->hasWaitingRequestsFn;
  io->hasWaitingRequestsFn=f;

  return of;
}



GWEN_IO_LAYER_ADDWAITINGSOCKETS_FN GWEN_Io_Layer_SetAddWaitingSocketsFn(GWEN_IO_LAYER *io,
									GWEN_IO_LAYER_ADDWAITINGSOCKETS_FN f) {
  GWEN_IO_LAYER_ADDWAITINGSOCKETS_FN of;

  assert(io);
  assert(io->refCount);

  of=io->addWaitingSocketsFn;
  io->addWaitingSocketsFn=f;

  return of;
}



GWEN_IO_LAYER_LISTEN_FN GWEN_Io_Layer_SetListenFn(GWEN_IO_LAYER *io, GWEN_IO_LAYER_LISTEN_FN f) {
  GWEN_IO_LAYER_LISTEN_FN of;

  assert(io);
  assert(io->refCount);

  of=io->listenFn;
  io->listenFn=f;

  return of;
}



int GWEN_Io_Layer_WorkOnRequests(GWEN_IO_LAYER *io) {
  assert(io);
  assert(io->refCount);

  if (io->workOnRequestsFn)
    return io->workOnRequestsFn(io);
  else
    return GWEN_Io_Layer_WorkResultError;
}



int GWEN_Io_Layer_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  assert(io);
  assert(io->refCount);

  if (io->addRequestFn) {
    /* preset some vars */
    GWEN_Io_Request_SetStatus(r, GWEN_Io_Request_StatusEnqueued);
    GWEN_Io_Request_SetResultCode(r, 0);
    GWEN_Io_Request_SetBufferPos(r, 0);
    GWEN_Io_Request_SetIoLayer(r, io);
    return io->addRequestFn(io, r);
  }
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Io_Layer_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  assert(io);
  assert(io->refCount);

  GWEN_Io_Request_SetIoLayer(r, NULL);
  if (io->delRequestFn)
    return io->delRequestFn(io, r);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Io_Layer_HasWaitingRequests(GWEN_IO_LAYER *io) {
  assert(io);
  assert(io->refCount);

  if (io->hasWaitingRequestsFn)
    return io->hasWaitingRequestsFn(io);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Io_Layer_AddWaitingSockets(GWEN_IO_LAYER *io,
				    GWEN_SOCKET_LIST2 *readSockets,
				    GWEN_SOCKET_LIST2 *writeSockets) {
  assert(io);
  assert(io->refCount);

  if (io->addWaitingSocketsFn)
    return io->addWaitingSocketsFn(io, readSockets, writeSockets);
  else {
    if (io->baseLayer)
      return GWEN_Io_Layer_AddWaitingSockets(io->baseLayer, readSockets, writeSockets);
    else
      return 0;
  }
}



int GWEN_Io_Layer_Listen(GWEN_IO_LAYER *io) {
  assert(io);
  assert(io->refCount);

  if (io->listenFn)
    return io->listenFn(io);
  else {
    GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusListening);
  }

  return 0;
}



int GWEN_Io_Layer__WaitForRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r, int msecs,
				  int deleteRequest) {
  int rv;
  GWEN_IO_REQUEST_STATUS st;
  GWEN_IO_REQUEST_TYPE t;

  t=GWEN_Io_Request_GetType(r);

  /* add request */
  rv=GWEN_Io_Layer_AddRequest(io, r);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    if (deleteRequest)
      GWEN_Io_Request_free(r);
    return rv;
  }

  /* wait for request to finish */
  rv=GWEN_Io_Manager_WaitForRequest(r, msecs);
  st=GWEN_Io_Request_GetStatus(r);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d, status=%d)", rv, st);
    if (st==GWEN_Io_Request_StatusEnqueued)
      GWEN_Io_Layer_DelRequest(io, r);
    if (deleteRequest)
      GWEN_Io_Request_free(r);
    return rv;
  }

  /* if not finished, report an error */
  if (st!=GWEN_Io_Request_StatusFinished) {
    DBG_INFO(GWEN_LOGDOMAIN, "Bad status of io request (%d)", GWEN_Io_Request_GetStatus(r));
    if (st==GWEN_Io_Request_StatusEnqueued)
      GWEN_Io_Layer_DelRequest(io, r);
    if (deleteRequest)
      GWEN_Io_Request_free(r);
    return rv;
  }

  /* check result code */
  rv=GWEN_Io_Request_GetResultCode(r);
  if (rv) {
    if (rv==GWEN_ERROR_EOF && t==GWEN_Io_Request_TypeRead) {
      rv=GWEN_Io_Request_GetBufferPos(r);
      if (rv==0) {
	/* only report EOF error if nothing could be read here */
        DBG_INFO(GWEN_LOGDOMAIN, "EOF, nothing read");
	if (deleteRequest)
	  GWEN_Io_Request_free(r);
	return GWEN_ERROR_EOF;
      }
      /* otherwise use the standard handling below */
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "Result of IO request is an error (%d)", rv);
      if (deleteRequest)
	GWEN_Io_Request_free(r);
      return rv;
    }
  }

  /* return bufferpos, it is 0 for most non-read/write requests and holds the number of transfered bytes otherwise */
  rv=GWEN_Io_Request_GetBufferPos(r);

  if (deleteRequest)
    GWEN_Io_Request_free(r);
  return rv;
}



int GWEN_Io_Layer_Connect(GWEN_IO_LAYER *io,
			  uint32_t flags,
			  uint32_t guiid, int msecs) {
  GWEN_IO_REQUEST *r;

  /* create request */
  r=GWEN_Io_Request_new(GWEN_Io_Request_TypeConnect, NULL, 0, NULL, NULL, guiid);
  assert(r);
  GWEN_Io_Request_SetFlags(r, flags);

  return GWEN_Io_Layer__WaitForRequest(io, r, msecs, 1);
}



int GWEN_Io_Layer_ConnectRecursively(GWEN_IO_LAYER *io,
				     GWEN_IO_LAYER *stopAtLayer,
				     uint32_t flags,
				     uint32_t guiid, int msecs) {
  assert(io);
  assert(io->refCount);

  if (io==stopAtLayer)
    /* caller wanted to stop here */
    return 0;

  /* first connect all base layers */
  if (io->baseLayer) {
    int rv;

    rv=GWEN_Io_Layer_ConnectRecursively(io->baseLayer, stopAtLayer, flags, guiid, msecs);
    if (rv)
      return rv;
  }

  /* then connect this layer, if it not already is */
  if (io->status!=GWEN_Io_Layer_StatusConnected)
    return GWEN_Io_Layer_Connect(io, flags, guiid, msecs);
  return 0;
}



int GWEN_Io_Layer_ListenRecursively(GWEN_IO_LAYER *io, GWEN_IO_LAYER *stopAtLayer) {
  assert(io);
  assert(io->refCount);

  if (io==stopAtLayer)
    /* caller wanted to stop here */
    return 0;

  /* first connect all base layers */
  if (io->baseLayer) {
    int rv;

    rv=GWEN_Io_Layer_ListenRecursively(io->baseLayer, stopAtLayer);
    if (rv)
      return rv;
  }

  /* then connect this layer, if it not already is */
  if (io->status!=GWEN_Io_Layer_StatusListening)
    return GWEN_Io_Layer_Listen(io);
  return 0;
}



int GWEN_Io_Layer_Disconnect(GWEN_IO_LAYER *io,
			     uint32_t flags,
			     uint32_t guiid, int msecs) {
  GWEN_IO_REQUEST *r;

  assert(io);
  assert(io->refCount);

  /* create request */
  r=GWEN_Io_Request_new(GWEN_Io_Request_TypeDisconnect, NULL, 0, NULL, NULL, guiid);
  assert(r);
  GWEN_Io_Request_SetFlags(r, flags);

  return GWEN_Io_Layer__WaitForRequest(io, r, msecs, 1);
}



int GWEN_Io_Layer__DisconnectRecursively(GWEN_IO_LAYER *io,
					 GWEN_IO_LAYER *stopAtLayer,
					 uint32_t flags,
					 uint32_t guiid, int msecs) {
  int firstError=0;

  assert(io);
  assert(io->refCount);

  if (io==stopAtLayer)
    /* caller wanted to stop here */
    return 0;

  /* disconnect this layer, if it not already is */
  if (io->status!=GWEN_Io_Layer_StatusDisconnected)
    firstError=GWEN_Io_Layer_Disconnect(io, flags, guiid, msecs);

  /* first connect all base layers */
  if (io->baseLayer) {
    int rv;

    rv=GWEN_Io_Layer__DisconnectRecursively(io->baseLayer, stopAtLayer, flags, guiid, msecs);
    if (rv && firstError==0)
      firstError=rv;
  }

  return firstError;
}



int GWEN_Io_Layer_DisconnectRecursively(GWEN_IO_LAYER *io,
					GWEN_IO_LAYER *stopAtLayer,
					uint32_t flags,
					uint32_t guiid, int msecs) {
  int rv;

  if (!(flags & GWEN_IO_REQUEST_FLAGS_FORCE)) {
    rv=GWEN_Io_Layer_FlushRecursively(io, guiid, msecs);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      /* return rv; */
    }
  }

  rv=GWEN_Io_Layer__DisconnectRecursively(io, stopAtLayer, flags, guiid, msecs);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_Io_Layer_ReadBytes(GWEN_IO_LAYER *io,
			    uint8_t *buffer,
			    uint32_t size,
			    uint32_t flags,
			    uint32_t guiid, int msecs) {
  GWEN_IO_REQUEST *r;
  int rv;

  assert(io);
  assert(io->refCount);

  /* create request */
  r=GWEN_Io_Request_new(GWEN_Io_Request_TypeRead, buffer, size, NULL, NULL, guiid);
  assert(r);
  GWEN_Io_Request_SetFlags(r, flags);

  rv=GWEN_Io_Layer__WaitForRequest(io, r, msecs, 1);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }
  return rv;
}



int GWEN_Io_Layer_ReadPacket(GWEN_IO_LAYER *io,
			     uint8_t *buffer,
			     uint32_t size,
			     uint32_t flags,
			     uint32_t guiid, int msecs) {
  GWEN_IO_REQUEST *r;
  int rv;
  int loop;
  uint32_t bytesRead=0;

  assert(io);
  assert(io->refCount);

  for (loop=0;;loop++) {
    uint32_t rflags;

    /* create request */
    r=GWEN_Io_Request_new(GWEN_Io_Request_TypeRead, buffer, size, NULL, NULL, guiid);
    assert(r);
    GWEN_Io_Request_SetFlags(r, flags);
    if (loop==0)
      GWEN_Io_Request_AddFlags(r, GWEN_IO_REQUEST_FLAGS_PACKETBEGIN);

    rv=GWEN_Io_Layer__WaitForRequest(io, r, msecs, 0);
    rflags=GWEN_Io_Request_GetFlags(r);
    GWEN_Io_Request_free(r);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    bytesRead+=rv;
    if (rflags & GWEN_IO_REQUEST_FLAGS_PACKETEND) {
      return bytesRead;
    }
    else if (size<=rv) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Buffer overrun after reading %d bytes", bytesRead);
      return GWEN_ERROR_BUFFER_OVERFLOW;
    }
    size-=rv;
    buffer+=rv;
  }
}



int GWEN_Io_Layer_ReadPacketToBuffer(GWEN_IO_LAYER *io,
                                     GWEN_BUFFER *buf,
				     uint32_t flags,
				     uint32_t guiid, int msecs) {
  GWEN_IO_REQUEST *r;
  int rv;
  int loop;
  uint32_t bytesRead=0;

  assert(io);
  assert(io->refCount);

  for (loop=0;;loop++) {
    uint32_t rflags;
    uint8_t *p;
    uint32_t bRead;

    GWEN_Buffer_AllocRoom(buf, 512);
    p=(uint8_t*)GWEN_Buffer_GetPosPointer(buf);
    /* create request */
    r=GWEN_Io_Request_new(GWEN_Io_Request_TypeRead, p, 512, NULL, NULL, guiid);
    assert(r);
    GWEN_Io_Request_SetFlags(r, flags);
    if (loop==0)
      GWEN_Io_Request_AddFlags(r, GWEN_IO_REQUEST_FLAGS_PACKETBEGIN);

    rv=GWEN_Io_Layer__WaitForRequest(io, r, msecs, 0);
    rflags=GWEN_Io_Request_GetFlags(r);
    bRead=GWEN_Io_Request_GetBufferPos(r);
    bytesRead+=bRead;
    GWEN_Buffer_IncrementPos(buf, bRead);
    GWEN_Buffer_AdjustUsedBytes(buf);
    GWEN_Io_Request_free(r);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    if (rflags & GWEN_IO_REQUEST_FLAGS_PACKETEND) {
      return bytesRead;
    }
  }
}



int GWEN_Io_Layer_WriteBytes(GWEN_IO_LAYER *io,
			     const uint8_t *buffer,
			     uint32_t size,
			     uint32_t flags,
			     uint32_t guiid, int msecs) {
  GWEN_IO_REQUEST *r;
  int rv;

  assert(io);
  assert(io->refCount);

  /* create request */
  r=GWEN_Io_Request_new(GWEN_Io_Request_TypeWrite, (uint8_t*) buffer, size, NULL, NULL, guiid);
  assert(r);
  GWEN_Io_Request_SetFlags(r, flags);

  rv=GWEN_Io_Layer__WaitForRequest(io, r, msecs, 1);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }

  return rv;
}



int GWEN_Io_Layer_WriteString(GWEN_IO_LAYER *io,
			      const char *buffer,
			      uint32_t flags,
			      uint32_t guiid, int msecs) {
  GWEN_IO_REQUEST *r;
  uint32_t size=0;
  int rv;

  assert(io);
  assert(io->refCount);

  /* create request */
  if (buffer)
    size=strlen(buffer);
  r=GWEN_Io_Request_new(GWEN_Io_Request_TypeWrite, (uint8_t*) buffer, size, NULL, NULL, guiid);
  assert(r);
  GWEN_Io_Request_SetFlags(r, flags);

  rv=GWEN_Io_Layer__WaitForRequest(io, r, msecs, 1);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }

  return rv;
}



int GWEN_Io_Layer_WriteChar(GWEN_IO_LAYER *io,
			    char c,
			    uint32_t flags,
			    uint32_t guiid, int msecs) {
  GWEN_IO_REQUEST *r;
  int rv;

  assert(io);
  assert(io->refCount);

  /* create request */
  r=GWEN_Io_Request_new(GWEN_Io_Request_TypeWrite, (uint8_t*) &c, 1, NULL, NULL, guiid);
  assert(r);
  GWEN_Io_Request_SetFlags(r, flags);

  rv=GWEN_Io_Layer__WaitForRequest(io, r, msecs, 1);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }

  return rv;
}



int GWEN_Io_Layer_ReadToBufferUntilEof(GWEN_IO_LAYER *io,
				       GWEN_BUFFER *buf,
				       uint32_t guiid, int msecs) {
  int total=0;

  assert(io);
  assert(io->refCount);

  for(;;) {
    int rv;
    uint8_t buffer[256];

    rv=GWEN_Io_Layer_ReadBytes(io, buffer, sizeof(buffer), 0, guiid, msecs);
    if (rv<0) {
      if (rv==GWEN_ERROR_EOF)
	return total;
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    else if (rv==0) {
      return total;
    }
    else {
      GWEN_Buffer_AppendBytes(buf, (const char*)buffer, rv);
      total+=rv;
    }
  }
}



int GWEN_Io_Layer_FlushRecursively(GWEN_IO_LAYER *io, uint32_t guiid, int msecs) {
  GWEN_IO_LAYER *currIo;

  currIo=io;
  while(currIo) {
    int rv;

    rv=GWEN_Io_Layer_WriteString(currIo, "",
				 GWEN_IO_REQUEST_FLAGS_FLUSH,
				 guiid,
				 msecs);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    currIo=currIo->baseLayer;
  }

  return 0;
}








