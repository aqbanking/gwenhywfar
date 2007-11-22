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


#include "io_file_p.h"
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




GWEN_INHERIT(GWEN_IO_LAYER, GWEN_IO_LAYER_FILE)



GWEN_IO_LAYER *GWEN_Io_LayerFile_new(int fdRead, int fdWrite) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_FILE *xio;

  io=GWEN_Io_Layer_new(GWEN_IO_LAYER_FILE_TYPE, NULL);
  assert(io);
  GWEN_NEW_OBJECT(GWEN_IO_LAYER_FILE, xio);
  assert(xio);
  GWEN_INHERIT_SETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_FILE, io, xio, GWEN_Io_LayerFile_freeData);

  GWEN_Io_Layer_SetWorkOnRequestsFn(io, GWEN_Io_LayerFile_WorkOnRequests);
  GWEN_Io_Layer_SetAddRequestFn(io, GWEN_Io_LayerFile_AddRequest);
  GWEN_Io_Layer_SetDelRequestFn(io, GWEN_Io_LayerFile_DelRequest);
  GWEN_Io_Layer_SetHasWaitingRequestsFn(io, GWEN_Io_LayerFile_HasWaitingRequests);

  xio->fdRead=fdRead;
  xio->fdWrite=fdWrite;

#ifndef OS_WIN32
  /* TODO: How can we make a file non-blocking under win32? */
  if (fdRead!=-1) {
    int rv;
    long fl;

    /* get and save previous flags */
    xio->readFlags=fcntl(fdRead, F_GETFL);

    /* set new flags (-> nonblocking!) */
    fl=xio->readFlags | O_NONBLOCK;
    rv=fcntl(fdRead, F_SETFL, fl);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN, "fcntl(%d, F_SETFL): %s", fdRead, strerror(errno));
      xio->fdRead=-1;
    }
  }

  if (fdWrite!=-1) {
    int rv;
    long fl;

    /* get and save previous flags */
    xio->writeFlags=fcntl(fdWrite, F_GETFL);

    /* set new flags (-> nonblocking!) */
    fl=xio->writeFlags | O_NONBLOCK;
    rv=fcntl(fdWrite, F_SETFL, fl);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN, "fcntl(%d, F_SETFL): %s", fdWrite, strerror(errno));
      xio->fdWrite=-1;
    }
  }
#endif

  GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusConnected);


  return io;
}



GWENHYWFAR_CB
void GWEN_Io_LayerFile_freeData(void *bp, void *p) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_FILE *xio;

  io=(GWEN_IO_LAYER*) bp;
  assert(io);
  xio=(GWEN_IO_LAYER_FILE*) p;
  assert(xio);
  if (xio->readRequest) {
    GWEN_IO_REQUEST *r;

    r=xio->readRequest;
    xio->readRequest=NULL;
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_ABORTED);
    GWEN_Io_Request_free(r);
  }
  if (xio->writeRequest) {
    GWEN_IO_REQUEST *r;

    r=xio->writeRequest;
    xio->writeRequest=NULL;
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_ABORTED);
    GWEN_Io_Request_free(r);
  }
  if (xio->fdRead!=-1) {
    if (!(GWEN_Io_Layer_GetFlags(io) & GWEN_IO_LAYER_FLAGS_DONTCLOSE))
      close(xio->fdRead);
  }
  if (xio->fdWrite!=-1) {
    if (!(GWEN_Io_Layer_GetFlags(io) & GWEN_IO_LAYER_FLAGS_DONTCLOSE))
      close(xio->fdWrite);
  }
  GWEN_FREE_OBJECT(xio);
}



int GWEN_Io_LayerFile_GetReadFileDescriptor(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_FILE *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_FILE, io);
  assert(xio);

  return xio->fdRead;
}



int GWEN_Io_LayerFile_GetWriteFileDescriptor(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_FILE *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_FILE, io);
  assert(xio);

  return xio->fdWrite;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerFile_WorkOnRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_FILE *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_FILE, io);
  assert(xio);

  /* work on read request */
  if (xio->readRequest) {
    ssize_t rv;
    ssize_t bytesLeft;
    GWEN_IO_REQUEST *r;

    r=xio->readRequest;
    bytesLeft=GWEN_Io_Request_GetBufferSize(r)-GWEN_Io_Request_GetBufferPos(r);
    rv=read(xio->fdRead,
	    GWEN_Io_Request_GetBufferPtr(r),
	    bytesLeft);
    if (rv==(ssize_t)-1) {
      if (errno!=EAGAIN && errno!=EINTR) {
	DBG_INFO(GWEN_LOGDOMAIN, "read(%d): %s", xio->fdRead, strerror(errno));
	xio->readRequest=NULL;
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_IO);
	GWEN_Io_Request_free(r);
	doneSomething=1;
      }
      /* nothing to read, nothing done, so we don't set doneSomething=1 here ! */
    }
    else if (rv==0) {
      /* end of stream reached */
      DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
      xio->readRequest=NULL;
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_EOF);
      GWEN_Io_Request_free(r);
      doneSomething=1;
    }
    else {
      uint32_t newPos;

      /* some data returned */
      newPos=GWEN_Io_Request_GetBufferPos(r)+rv;
      GWEN_Io_Request_SetBufferPos(r, newPos);

      if (rv<bytesLeft) {
	/* fewer bytes read, is that ok? */
	if (!(GWEN_Io_Request_GetFlags(r) & GWEN_IO_REQUEST_FLAGS_READALL)) {
	  /* it is, return the data we already have */
	  xio->readRequest=NULL;
	  GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	  GWEN_Io_Request_free(r);
	}
        /* otherwise we are forced to read the rest in the next session */
      }
      else {
	/* all data read */
	xio->readRequest=NULL;
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	GWEN_Io_Request_free(r);
      }
      doneSomething=1;
    }
  }

  /* work on write request */
  if (xio->writeRequest) {
    ssize_t rv;
    ssize_t bytesLeft;
    GWEN_IO_REQUEST *r;

    r=xio->writeRequest;
    bytesLeft=GWEN_Io_Request_GetBufferSize(r)-GWEN_Io_Request_GetBufferPos(r);
    rv=write(xio->fdWrite,
	    GWEN_Io_Request_GetBufferPtr(r),
	    bytesLeft);
    if (rv==(ssize_t)-1) {
      if (errno!=EAGAIN && errno!=EINTR) {
	DBG_INFO(GWEN_LOGDOMAIN, "write(%d): %s", xio->fdWrite, strerror(errno));
	xio->writeRequest=NULL;
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_IO);
	GWEN_Io_Request_free(r);
	doneSomething=1;
      }
      /* nothing to write, nothing done, so we don't set doneSomething=1 here ! */
    }
    else {
      uint32_t newPos;

      /* some data returned */
      newPos=GWEN_Io_Request_GetBufferPos(r)+rv;
      GWEN_Io_Request_SetBufferPos(r, newPos);

      if (newPos>=GWEN_Io_Request_GetBufferSize(r) ||
	  !(GWEN_Io_Request_GetFlags(r) & GWEN_IO_REQUEST_FLAGS_WRITEALL)) {
	/* request complete */
	xio->writeRequest=NULL;
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	GWEN_Io_Request_free(r);
      }
      doneSomething=1;
    }
  }

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



int GWEN_Io_LayerFile_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_FILE *xio;
  GWEN_IO_LAYER_STATUS st;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_FILE, io);
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

    /* check whether we already have a read request */
    if (xio->readRequest) {
      DBG_INFO(GWEN_LOGDOMAIN, "There already is a read request");
      return GWEN_ERROR_TRY_AGAIN;
    }

    /* check whether the file is open */
    if (xio->fdRead==-1) {
      DBG_INFO(GWEN_LOGDOMAIN, "File is not open for reading");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }

    /* enqueue request */
    xio->readRequest=r;
    GWEN_Io_Request_Attach(xio->readRequest);
    break;

  case GWEN_Io_Request_TypeWrite:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "File is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }

    /* check whether we already have a read request */
    if (xio->writeRequest) {
      DBG_INFO(GWEN_LOGDOMAIN, "There already is a write request");
      return GWEN_ERROR_TRY_AGAIN;
    }

    /* check whether the file is open */
    if (xio->fdWrite==-1) {
      DBG_INFO(GWEN_LOGDOMAIN, "File is not open for writing");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }

    /* enqueue request */
    xio->writeRequest=r;
    GWEN_Io_Request_Attach(xio->writeRequest);
    break;

  case GWEN_Io_Request_TypeDisconnect:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "File is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }
    else {
      int rv=0, rv1=0, rv2=0, rv3=0, rv4=0;

      /* restore old flags */
#ifndef OS_WIN32
      if (xio->fdRead!=-1)
	rv1=fcntl(xio->fdRead, F_SETFL, xio->readFlags);
      if (xio->fdWrite!=-1)
	rv2=fcntl(xio->fdWrite, F_SETFL, xio->writeFlags);
#endif

      /* close files if not forbidden */
      if (!(GWEN_Io_Layer_GetFlags(io) & GWEN_IO_LAYER_FLAGS_DONTCLOSE)) {
	if (xio->fdRead!=-1)
	  rv3=close(xio->fdRead);
	if (xio->fdWrite!=-1 && xio->fdRead!=xio->fdWrite)
	  rv4=close(xio->fdWrite);

	/* overwrite descriptors */
	xio->fdRead=-1;
	xio->fdWrite=-1;
      }

      /* sample all error codes */
      if (rv1) rv=rv1;
      if (rv2) rv=rv2;
      if (rv3) rv=rv3;
      if (rv4) rv=rv4;
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "Error closing file: %s", strerror(errno));
	GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, rv);
	return rv;
      }

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



int GWEN_Io_LayerFile_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_FILE *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_FILE, io);
  assert(xio);

  switch(GWEN_Io_Request_GetType(r)) {
  case GWEN_Io_Request_TypeRead:
    if (xio->readRequest==r) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Aborted read request");
      GWEN_Io_Request_Finished(xio->readRequest, GWEN_Io_Request_StatusFinished, GWEN_ERROR_ABORTED);
      GWEN_Io_Request_free(xio->readRequest);
      xio->readRequest=NULL;
    }
    else {
      /* not my request */
      DBG_INFO(GWEN_LOGDOMAIN, "Read request not registered with this io layer");
      return GWEN_ERROR_INVALID;
    }
    break;

  case GWEN_Io_Request_TypeWrite:
    if (xio->writeRequest==r) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Aborted write request");
      GWEN_Io_Request_Finished(xio->writeRequest, GWEN_Io_Request_StatusFinished, GWEN_ERROR_ABORTED);
      GWEN_Io_Request_free(xio->writeRequest);
      xio->writeRequest=NULL;
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



int GWEN_Io_LayerFile_HasWaitingRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_FILE *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_FILE, io);
  assert(xio);

  if (xio->readRequest || xio->writeRequest)
    return 1;
  else
    return 0;
}














