/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Jan 25 2004
    copyright   : (C) 2004 by Martin Preuss
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

#include "ringbuffer_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <stdlib.h>



GWEN_RINGBUFFER *GWEN_RingBuffer_new(unsigned int size){
  GWEN_RINGBUFFER *rb;

  assert(size);
  GWEN_NEW_OBJECT(GWEN_RINGBUFFER, rb);
  rb->ptr=(char*)malloc(size);
  rb->bufferSize=size;
  return rb;
}



void GWEN_RingBuffer_free(GWEN_RINGBUFFER *rb){
  if (rb) {
    free(rb->ptr);
    free(rb);
  }
}



int Gwen_RingBuffer_WriteBytes(GWEN_RINGBUFFER *rb,
                               const char *buffer,
                               GWEN_TYPE_UINT32 *size){
  GWEN_TYPE_UINT32 psize;
  GWEN_TYPE_UINT32 bytesLeft;

  if ((rb->bufferSize-rb->bytesUsed)==0) {
    DBG_INFO(0, "Buffer full");
    return -1;
  }

  bytesLeft=*size;
  while(bytesLeft) {
    if ((rb->bufferSize-rb->bytesUsed)==0)
      break;
    if (rb->writePos>=rb->readPos)
      psize=rb->bufferSize-rb->writePos;
    else
      psize=rb->readPos-rb->writePos;

    memmove(rb->ptr+rb->writePos, buffer, psize);
    rb->writePos+=psize;
    if (rb->writePos>=rb->bufferSize)
      rb->writePos=0;
    rb->bytesUsed+=psize;
    buffer+=psize;
    bytesLeft-=psize;
  } /* while */
  *size-=bytesLeft;
  return 0;
}



int Gwen_RingBuffer_ReadBytes(GWEN_RINGBUFFER *rb,
                              char *buffer,
                              GWEN_TYPE_UINT32 *size){
  GWEN_TYPE_UINT32 psize;
  GWEN_TYPE_UINT32 bytesLeft;

  if (rb->bytesUsed==0) {
    DBG_INFO(0, "Buffer empty");
    return -1;
  }

  bytesLeft=*size;
  while(bytesLeft) {
    if (rb->bytesUsed==0)
      break;
    if (rb->readPos>=rb->writePos)
      psize=rb->bufferSize-rb->readPos;
    else
      psize=rb->writePos-rb->readPos;

    memmove(buffer, rb->ptr+rb->readPos, psize);
    rb->readPos+=psize;
    if (rb->readPos>=rb->bufferSize)
      rb->readPos=0;
    rb->bytesUsed-=psize;
    buffer+=psize;
    bytesLeft-=psize;
  } /* while */
  *size-=bytesLeft;
  return 0;
}



GWEN_TYPE_UINT32 GWEN_RingBuffer_GetUsedBytes(const GWEN_RINGBUFFER *rb){
  assert(rb);
  return rb->bytesUsed;
}



GWEN_TYPE_UINT32 GWEN_RingBuffer_GetBytesLeft(const GWEN_RINGBUFFER *rb){
  assert(rb);
  return rb->bufferSize-rb->bytesUsed;
}



int GWEN_RingBuffer_WriteByte(GWEN_RINGBUFFER *rb,
                              char c) {
  assert(rb);
  if ((rb->bufferSize-rb->bytesUsed)==0) {
    DBG_INFO(0, "Buffer full");
    return -1;
  }

  rb->ptr[rb->writePos]=c;
  rb->writePos++;
  if (rb->writePos>=rb->bufferSize)
    rb->writePos=0;
  rb->bytesUsed++;

  return 0;
}



int GWEN_RingBuffer_ReadByte(GWEN_RINGBUFFER *rb) {
  int c;

  assert(rb);
  if (rb->bytesUsed==0) {
    DBG_INFO(0, "Buffer empty");
    return -1;
  }

  c=(unsigned char)rb->ptr[rb->readPos];
  rb->readPos++;
  if (rb->readPos>=rb->bufferSize)
    rb->readPos=0;
  rb->bytesUsed--;

  return c;
}











