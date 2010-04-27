/***************************************************************************
 begin       : Tue Apr 27 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "syncio_buffered_p.h"
#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <assert.h>
#include <errno.h>
#include <string.h>



GWEN_INHERIT(GWEN_SYNCIO, GWEN_SYNCIO_BUFFERED)



GWEN_SYNCIO *GWEN_SyncIo_Buffered_new(GWEN_SYNCIO *baseIo) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_BUFFERED *xio;

  assert(baseIo);
  sio=GWEN_SyncIo_new(GWEN_SYNCIO_BUFFERED_TYPE, baseIo);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_BUFFERED, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_BUFFERED, sio, xio, GWEN_SyncIo_Buffered_FreeData);

  GWEN_SyncIo_SetConnectFn(sio, GWEN_SyncIo_Buffered_Connect);
  GWEN_SyncIo_SetDisconnectFn(sio, GWEN_SyncIo_Buffered_Disconnect);
  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_Buffered_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_Buffered_Write);

  xio->readBuffer=GWEN_RingBuffer_new(1024);

  return sio;
}



void GWENHYWFAR_CB GWEN_SyncIo_Buffered_FreeData(void *bp, void *p) {
  GWEN_SYNCIO_BUFFERED *xio;

  xio=(GWEN_SYNCIO_BUFFERED*) p;
  GWEN_RingBuffer_free(xio->readBuffer);
  GWEN_FREE_OBJECT(xio);
}



int GWENHYWFAR_CB GWEN_SyncIo_Buffered_Connect(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO *baseIo;

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  if (baseIo) {
    int rv;

    rv=GWEN_SyncIo_Connect(baseIo);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    return rv;
  }

  return 0;
}



int GWENHYWFAR_CB GWEN_SyncIo_Buffered_Disconnect(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO *baseIo;

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  if (baseIo) {
    int rv;

    rv=GWEN_SyncIo_Disconnect(baseIo);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    return rv;
  }

  return 0;
}



int GWENHYWFAR_CB GWEN_SyncIo_Buffered_Read(GWEN_SYNCIO *sio,
					    uint8_t *buffer,
					    uint32_t size) {
  GWEN_SYNCIO_BUFFERED *xio;
  uint32_t flags;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_BUFFERED, sio);
  assert(xio);

  if (xio->readBuffer==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No buffer");
    return GWEN_ERROR_INTERNAL;
  }

  flags=GWEN_SyncIo_GetFlags(sio);
  if (flags & GWEN_SYNCIO_FLAGS_TRANSPARENT) {
    uint32_t bytesInBuffer;

    bytesInBuffer=GWEN_RingBuffer_GetUsedBytes(xio->readBuffer);
    if (bytesInBuffer) {
      int rv;
      uint32_t i;

      /* still bytes in buffer, return them first */
      if (size>bytesInBuffer)
	i=bytesInBuffer;
      else
	i=size;
      rv=GWEN_RingBuffer_ReadBytes(xio->readBuffer, (char*) buffer, &i);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
      /* bytes read */
      return i;
    }
    else {
      GWEN_SYNCIO *baseIo;

      baseIo=GWEN_SyncIo_GetBaseIo(sio);
      if (baseIo) {
	int rv;

	rv=GWEN_SyncIo_Read(baseIo, buffer, size);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  return rv;
	}
	return rv;
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "No base layer");
        return GWEN_ERROR_INTERNAL;
      }
    }
  }
  else {
    uint32_t bytesRead=0;

    while(bytesRead==0) {
      uint32_t bytesInBuffer;
      const uint8_t *psrc;
      uint32_t bytesSkipped=0;

      bytesInBuffer=GWEN_RingBuffer_GetMaxUnsegmentedRead(xio->readBuffer);
      if (bytesInBuffer==0) {
	uint32_t bytesFree;
	GWEN_SYNCIO *baseIo;
	int rv;

	/* fill buffer */
	bytesFree=GWEN_RingBuffer_GetMaxUnsegmentedWrite(xio->readBuffer);
	if (bytesFree==0) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "No unsegmente read and write. TSNH!");
	  return GWEN_ERROR_INTERNAL;
	}

	baseIo=GWEN_SyncIo_GetBaseIo(sio);
        assert(baseIo);

	do {
	  rv=GWEN_SyncIo_Read(baseIo,
			      (uint8_t*) GWEN_RingBuffer_GetWritePointer(xio->readBuffer),
			      bytesFree);
	} while (rv==GWEN_ERROR_INTERRUPTED);

	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  return rv;
	}
	else if (rv==0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
	  return bytesRead;
	}
	GWEN_RingBuffer_SkipBytesWrite(xio->readBuffer, rv);
	bytesInBuffer=GWEN_RingBuffer_GetMaxUnsegmentedRead(xio->readBuffer);
	if (bytesInBuffer==0) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Still no bytes available?? TSNH!");
	  return GWEN_ERROR_INTERNAL;
	}
      }

      /* read data from ring buffer */
      psrc=(const uint8_t*)GWEN_RingBuffer_GetReadPointer(xio->readBuffer);
      while(bytesSkipped<bytesInBuffer && bytesRead<(size-1)) {
	uint8_t c;

	c=*psrc;
	if (c!=13) {
	  *(buffer++)=c;
	  bytesRead++;
	}
	psrc++;
	bytesSkipped++;
	if (c==10)
	  break;
      }
      GWEN_RingBuffer_SkipBytesRead(xio->readBuffer, bytesSkipped);
    }
    *buffer=0;

    return bytesRead;
  }
}



int GWENHYWFAR_CB GWEN_SyncIo_Buffered_Write(GWEN_SYNCIO *sio,
					     const uint8_t *buffer,
					     uint32_t size) {
  GWEN_SYNCIO_BUFFERED *xio;
  GWEN_SYNCIO *baseIo;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_BUFFERED, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  if (baseIo) {
    uint32_t flags;

    flags=GWEN_SyncIo_GetFlags(sio);
    if (flags & GWEN_SYNCIO_FLAGS_TRANSPARENT) {
      int rv;

      /* transparent mode, write directly to base io */
      do {
	rv=GWEN_SyncIo_Write(baseIo, buffer, size);
      } while (rv==GWEN_ERROR_INTERRUPTED);

      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
      return rv;
    }
    else {
      int rv;

      if (size) {
	rv=GWEN_SyncIo_WriteForced(baseIo, buffer, size);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  return rv;
	}
      }

      if (flags & GWEN_SYNCIO_FLAGS_BUFFERED_DOS) {
	do {
	  rv=GWEN_SyncIo_Write(baseIo, (const uint8_t*) "\r\n", 2);
	} while (rv==GWEN_ERROR_INTERRUPTED);
      }
      else {
	do {
	  rv=GWEN_SyncIo_Write(baseIo, (const uint8_t*) "\n", 1);
	} while (rv==GWEN_ERROR_INTERRUPTED);
      }

      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }

      return size;
    }
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No base layer");
    return GWEN_ERROR_INTERNAL;
  }
}





