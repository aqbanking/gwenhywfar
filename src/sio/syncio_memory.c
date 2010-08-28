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

#define DISABLE_DEBUGLOG



#include "syncio_memory_p.h"
#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <assert.h>
#include <errno.h>
#include <string.h>



GWEN_INHERIT(GWEN_SYNCIO, GWEN_SYNCIO_MEMORY)



GWEN_SYNCIO *GWEN_SyncIo_Memory_new(GWEN_BUFFER *buffer, int take) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_MEMORY *xio;

  sio=GWEN_SyncIo_new(GWEN_SYNCIO_MEMORY_TYPE, NULL);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_MEMORY, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_MEMORY, sio, xio, GWEN_SyncIo_Memory_FreeData);

  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_Memory_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_Memory_Write);

  if (buffer) {
    xio->buffer=buffer;
    xio->own=take?1:0;
  }
  else {
    xio->buffer=GWEN_Buffer_new(0, 256, 0, 1);
    xio->own=1;
  }

  return sio;
}



GWEN_SYNCIO *GWEN_SyncIo_Memory_fromBuffer(const uint8_t *buffer, int size) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_MEMORY *xio;

  sio=GWEN_SyncIo_new(GWEN_SYNCIO_MEMORY_TYPE, NULL);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_MEMORY, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_MEMORY, sio, xio, GWEN_SyncIo_Memory_FreeData);

  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_Memory_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_Memory_Write);

  /* adapt size, if necessary */
  if (size==-1) {
    if (buffer)
      size=strlen((const char*) buffer)+1;
    else
      size=0;
  }

  xio->buffer=GWEN_Buffer_new(0, size, 0, 1);
  xio->own=1;
  if (buffer && size>0)
    GWEN_Buffer_AppendBytes(xio->buffer, (const char*) buffer, size);

  return sio;
}



void GWENHYWFAR_CB GWEN_SyncIo_Memory_FreeData(void *bp, void *p) {
  GWEN_SYNCIO_MEMORY *xio;

  xio=(GWEN_SYNCIO_MEMORY*) p;
  if (xio->buffer && xio->own)
    GWEN_Buffer_free(xio->buffer);
  GWEN_FREE_OBJECT(xio);
}



GWEN_BUFFER *GWEN_SyncIo_Memory_GetBuffer(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_MEMORY *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_MEMORY, sio);
  assert(xio);

  return xio->buffer;
}



GWEN_BUFFER *GWEN_SyncIo_Memory_TakeBuffer(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_MEMORY *xio;
  GWEN_BUFFER *buf;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_MEMORY, sio);
  assert(xio);

  if (xio->own==0 || xio->buffer==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Can't give away buffer, object does not own it");
    return NULL;
  }
  buf=xio->buffer;
  xio->buffer=NULL;
  xio->own=0;
  return buf;
}



int GWENHYWFAR_CB GWEN_SyncIo_Memory_Read(GWEN_SYNCIO *sio,
					  uint8_t *buffer,
					  uint32_t size) {
  GWEN_SYNCIO_MEMORY *xio;
  uint32_t bytesLeft;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_MEMORY, sio);
  assert(xio);

  if (xio->buffer==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No buffer");
    return GWEN_ERROR_INTERNAL;
  }

  bytesLeft=GWEN_Buffer_GetBytesLeft(xio->buffer);
  if (bytesLeft==0) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "EOF met");
    return 0;
  }

  if (size>bytesLeft)
    size=bytesLeft;
  memmove(buffer, GWEN_Buffer_GetPosPointer(xio->buffer), size);
  GWEN_Buffer_IncrementPos(xio->buffer, size);

  return size;
}



int GWENHYWFAR_CB GWEN_SyncIo_Memory_Write(GWEN_SYNCIO *sio,
					   const uint8_t *buffer,
					   uint32_t size) {
  GWEN_SYNCIO_MEMORY *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_MEMORY, sio);
  assert(xio);

  if (xio->buffer==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No socket");
    return GWEN_ERROR_INTERNAL;
  }

  if (size) {
    int rv;

    rv=GWEN_Buffer_AppendBytes(xio->buffer, (const char*) buffer, size);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return size;
}





