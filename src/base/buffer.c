/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri Sep 12 2003
    copyright   : (C) 2003 by Martin Preuss
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

#include "buffer_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>


GWEN_BUFFER *GWEN_Buffer_new(char *buffer,
                             unsigned int size,
                             unsigned int used,
                             int take){
  GWEN_BUFFER *bf;

  GWEN_NEW_OBJECT(GWEN_BUFFER, bf);
  if (!buffer) {
    /* allocate buffer */
    if (size) {
      bf->ptr=(char*)malloc(size);
      assert(bf);
      bf->bufferSize=size;
      bf->flags=GWEN_BUFFER_FLAGS_OWNED;
      bf->bytesUsed=used;
    }
  }
  else {
    /* use existing buffer */
    bf->ptr=buffer;
    bf->bufferSize=size;
    bf->bytesUsed=used;
    if (take)
      bf->flags=GWEN_BUFFER_FLAGS_OWNED;
  }

  return bf;
}



void GWEN_Buffer_free(GWEN_BUFFER *bf){
  if (bf) {
    if (bf->flags & GWEN_BUFFER_FLAGS_OWNED)
      free(bf->ptr);
    free(bf);
  }
}



char *GWEN_Buffer_GetStart(GWEN_BUFFER *bf){
  assert(bf);
  return bf->ptr;
}



unsigned int GWEN_Buffer_GetSize(GWEN_BUFFER *bf){
  assert(bf);
  return bf->bufferSize;
}



unsigned int GWEN_Buffer_GetPos(GWEN_BUFFER *bf){
  assert(bf);
  return bf->pos;
}



int GWEN_Buffer_SetPos(GWEN_BUFFER *bf, unsigned int i){
  assert(bf);

  if (i>=bf->bufferSize) {
    DBG_ERROR(0, "Position %d outside buffer boundaries (%d bytes)",
              i, bf->bufferSize);
    return 1;
  }
  bf->pos=i;
  return 0;
}



unsigned int GWEN_Buffer_GetUsedBytes(GWEN_BUFFER *bf){
  assert(bf);
  return bf->bytesUsed;
}



int GWEN_Buffer_SetUsedBytes(GWEN_BUFFER *bf, unsigned int i){
  assert(bf);

  if (i>bf->bufferSize) {
    DBG_ERROR(0, "Bytes used>buffer size (%d>%d bytes)",
              i, bf->bufferSize);
    return 1;
  }
  bf->bytesUsed=i;
  return 0;
}



int GWEN_Buffer_AppendBytes(GWEN_BUFFER *bf,
                            const char *buffer,
                            unsigned int size){
  assert(bf);
  if (bf->pos+size>bf->bufferSize) {
    DBG_ERROR(0, "Buffer full (%d of %d bytes)", bf->pos, bf->bufferSize);
    return 1;
  }
  memmove(bf->ptr+bf->pos, buffer, size);
  bf->pos+=size;
  bf->bytesUsed+=size;
  return 0;
}



int GWEN_Buffer_AppendByte(GWEN_BUFFER *bf, char c){
  assert(bf);

  if (bf->pos+1>bf->bufferSize) {
    DBG_ERROR(0, "Buffer full (%d of %d bytes)", bf->pos, bf->bufferSize);
    return 1;
  }
  bf->ptr[bf->pos++]=c;
  bf->bytesUsed++;
  return 0;
}



int GWEN_Buffer_PeekByte(GWEN_BUFFER *bf){
  assert(bf);

  if (bf->pos>=bf->bufferSize) {
    DBG_ERROR(0, "Buffer end reached (%d bytes)", bf->pos);
    return -1;
  }

  if (bf->pos>=bf->bytesUsed) {
    DBG_ERROR(0, "End of used area reached (%d bytes)", bf->pos);
    return -1;
  }

  return (unsigned char) (bf->ptr[bf->pos]);
}



int GWEN_Buffer_ReadByte(GWEN_BUFFER *bf){
  int c;

  c=GWEN_Buffer_PeekByte(bf);
  if (c!=-1)
    bf->pos++;
  return c;
}



int GWEN_Buffer_IncrementPos(GWEN_BUFFER *bf, unsigned int i){
  assert(bf);

  if (i+bf->pos>=bf->bufferSize) {
    DBG_ERROR(0, "Position %d outside buffer boundaries (%d bytes)",
              i+bf->pos, bf->bufferSize);
    return 1;
  }
  bf->pos+=i;
  return 0;
}



int GWEN_Buffer_DecrementPos(GWEN_BUFFER *bf, unsigned int i){
  assert(bf);

  if (bf->pos-i<0) {
    DBG_ERROR(0, "Position %d outside buffer boundaries (%d bytes)",
              bf->pos-i, bf->bufferSize);
    return 1;
  }
  bf->pos-=i;
  return 0;
}



int GWEN_Buffer_AppendBuffer(GWEN_BUFFER *bf,
                             GWEN_BUFFER *sf){
  assert(bf);
  assert(sf);
  if (bf->pos+sf->bytesUsed>bf->bufferSize) {
    DBG_ERROR(0, "Buffer full (%d of %d bytes)", bf->pos, bf->bufferSize);
    return 1;
  }
  memmove(bf->ptr+bf->pos, sf->ptr, sf->bytesUsed);
  bf->pos+=sf->bytesUsed;
  bf->bytesUsed+=sf->bytesUsed;
  return 0;
}



unsigned int GWEN_Buffer_RoomLeft(GWEN_BUFFER *bf){
  assert(bf);

  return (bf->bufferSize-bf->bytesUsed);
}



unsigned int GWEN_Buffer_BytesLeft(GWEN_BUFFER *bf){
  assert(bf);

  return bf->bytesUsed-bf->pos;
}



char *GWEN_Buffer_GetPosPointer(GWEN_BUFFER *bf){
  assert(bf);
  return bf->ptr+bf->pos;
}



