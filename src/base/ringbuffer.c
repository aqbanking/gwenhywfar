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

#define DISABLE_DEBUGLOG

#include "ringbuffer_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <stdlib.h>



GWEN_RINGBUFFER *GWEN_RingBuffer_new(unsigned int size){
  GWEN_RINGBUFFER *rb;

  assert(size);
  GWEN_NEW_OBJECT(GWEN_RINGBUFFER, rb);
  DBG_MEM_INC("GWEN_RINGBUFFER", 0);
  rb->ptr=(char*)malloc(size);
  rb->bufferSize=size;
  return rb;
}



void GWEN_RingBuffer_free(GWEN_RINGBUFFER *rb){
  if (rb) {
    DBG_MEM_DEC("GWEN_RINGBUFFER");
    free(rb->ptr);
    free(rb);
  }
}



int GWEN_RingBuffer_WriteBytes(GWEN_RINGBUFFER *rb,
                               const char *buffer,
                               GWEN_TYPE_UINT32 *size){
  GWEN_TYPE_UINT32 psize;
  GWEN_TYPE_UINT32 bytesLeft;

  if ((rb->bufferSize-rb->bytesUsed)==0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Buffer full");
    rb->fullCounter++;
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
    if (psize>bytesLeft)
      psize=bytesLeft;

    memmove(rb->ptr+rb->writePos, buffer, psize);
    rb->writePos+=psize;
    if (rb->writePos>=rb->bufferSize)
      rb->writePos=0;
    rb->bytesUsed+=psize;
    buffer+=psize;
    bytesLeft-=psize;
  } /* while */
  *size-=bytesLeft;
  if (rb->bytesUsed>rb->maxBytesUsed)
    rb->maxBytesUsed=rb->bytesUsed;
  return 0;
}



GWEN_TYPE_UINT32
GWEN_RingBuffer_GetMaxUnsegmentedRead(GWEN_RINGBUFFER *rb) {
  GWEN_TYPE_UINT32 psize;

  assert(rb);
  if (rb->bytesUsed==0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Buffer empty");
    rb->emptyCounter++;
    return 0;
  }

  if (rb->readPos>=rb->writePos)
    psize=rb->bufferSize-rb->readPos;
  else
    psize=rb->writePos-rb->readPos;

  return psize;
}



GWEN_TYPE_UINT32
GWEN_RingBuffer_GetMaxUnsegmentedWrite(GWEN_RINGBUFFER *rb) {
  GWEN_TYPE_UINT32 psize;

  assert(rb);
  if ((rb->bufferSize-rb->bytesUsed)==0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Buffer full");
    rb->fullCounter++;
    return 0;
  }

  if (rb->writePos>=rb->readPos)
    psize=rb->bufferSize-rb->writePos;
  else
    psize=rb->readPos-rb->writePos;

  return psize;
}



void GWEN_RingBuffer_SkipBytesRead(GWEN_RINGBUFFER *rb,
                                   GWEN_TYPE_UINT32 psize) {
  assert(rb);

  if (rb->bytesUsed<psize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Asked to skip more bytes than available");
    abort();
  }
  rb->readPos+=psize;
  if (rb->readPos>=rb->bufferSize)
    rb->readPos=0;
  rb->bytesUsed-=psize;
  rb->throughput+=psize;
}



void GWEN_RingBuffer_SkipBytesWrite(GWEN_RINGBUFFER *rb,
                                    GWEN_TYPE_UINT32 psize) {
  assert(rb);

  if ((rb->bufferSize-rb->bytesUsed)<psize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Asked to skip more bytes than possible");
    abort();
  }

  rb->writePos+=psize;
  if (rb->writePos>=rb->bufferSize)
    rb->writePos=0;
  rb->bytesUsed+=psize;
  if (rb->bytesUsed>rb->maxBytesUsed)
    rb->maxBytesUsed=rb->bytesUsed;
}



int GWEN_RingBuffer_ReadBytes(GWEN_RINGBUFFER *rb,
                              char *buffer,
                              GWEN_TYPE_UINT32 *size){
  GWEN_TYPE_UINT32 psize;
  GWEN_TYPE_UINT32 bytesLeft;

  if (rb->bytesUsed==0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Buffer empty");
    rb->emptyCounter++;
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
    if (psize>bytesLeft)
      psize=bytesLeft;

    memmove(buffer, rb->ptr+rb->readPos, psize);
    rb->readPos+=psize;
    if (rb->readPos>=rb->bufferSize)
      rb->readPos=0;
    rb->bytesUsed-=psize;
    buffer+=psize;
    bytesLeft-=psize;
  } /* while */
  *size-=bytesLeft;
  rb->throughput+=*size;
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



GWEN_TYPE_UINT32 GWEN_RingBuffer_GetBufferSize(const GWEN_RINGBUFFER *rb){
  assert(rb);
  return rb->bufferSize;
}



int GWEN_RingBuffer_WriteByte(GWEN_RINGBUFFER *rb,
                              char c) {
  assert(rb);
  if ((rb->bufferSize-rb->bytesUsed)==0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Buffer full");
    rb->fullCounter++;
    return -1;
  }

  rb->ptr[rb->writePos]=c;
  rb->writePos++;
  if (rb->writePos>=rb->bufferSize)
    rb->writePos=0;
  rb->bytesUsed++;
  if (rb->bytesUsed>rb->maxBytesUsed)
    rb->maxBytesUsed=rb->bytesUsed;
  return 0;
}



int GWEN_RingBuffer_ReadByte(GWEN_RINGBUFFER *rb) {
  int c;

  assert(rb);
  if (rb->bytesUsed==0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Buffer empty");
    rb->emptyCounter++;
    return -1;
  }

  c=(unsigned char)rb->ptr[rb->readPos];
  rb->readPos++;
  if (rb->readPos>=rb->bufferSize)
    rb->readPos=0;
  rb->bytesUsed--;
  rb->throughput++;
  return c;
}



GWEN_TYPE_UINT32 GWEN_RingBuffer_GetMaxUsedBytes(const GWEN_RINGBUFFER *rb){
  assert(rb);
  return rb->maxBytesUsed;
}



void GWEN_RingBuffer_ResetMaxUsedBytes(GWEN_RINGBUFFER *rb) {
  assert(rb);
  rb->maxBytesUsed=0;
}



GWEN_TYPE_UINT32 GWEN_RingBuffer_GetThroughput(GWEN_RINGBUFFER *rb) {
  assert(rb);
  return rb->throughput;
}



void GWEN_RingBuffer_ResetThroughput(GWEN_RINGBUFFER *rb) {
  assert(rb);
  rb->throughput=0;
}



GWEN_TYPE_UINT32 GWEN_RingBuffer_GetFullCounter(const GWEN_RINGBUFFER *rb){
  assert(rb);
  return rb->fullCounter;
}



void GWEN_RingBuffer_ResetFullCounter(GWEN_RINGBUFFER *rb) {
  assert(rb);
  rb->fullCounter=0;
}



GWEN_TYPE_UINT32 GWEN_RingBuffer_GetEmptyCounter(const GWEN_RINGBUFFER *rb){
  assert(rb);
  return rb->emptyCounter;
}



void GWEN_RingBuffer_ResetEmptyCounter(GWEN_RINGBUFFER *rb) {
  assert(rb);
  rb->emptyCounter=0;
}



const char *GWEN_RingBuffer_GetReadPointer(const GWEN_RINGBUFFER *rb) {
  assert(rb);
  return rb->ptr+rb->readPos;
}



char *GWEN_RingBuffer_GetWritePointer(const GWEN_RINGBUFFER *rb) {
  assert(rb);
  return rb->ptr+rb->writePos;
}



void GWEN_RingBuffer_Reset(GWEN_RINGBUFFER *rb){
  assert(rb);

  rb->readPos=0;
  rb->writePos=0;
  rb->bytesUsed=0;
  rb->maxBytesUsed=0;
  rb->emptyCounter=0;
  rb->fullCounter=0;
  rb->throughput=0;
}






