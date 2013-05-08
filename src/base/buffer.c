/***************************************************************************
    begin       : Fri Sep 12 2003
    copyright   : (C) 2003-2010 by Martin Preuss
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

#include "buffer_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>


GWEN_BUFFER *GWEN_Buffer_new(char *buffer,
                             uint32_t size,
                             uint32_t used,
                             int take){
  GWEN_BUFFER *bf;

  GWEN_NEW_OBJECT(GWEN_BUFFER, bf);
  bf->_refCount=1;
  if (!buffer) {
    /* allocate buffer */
    bf->realPtr=(char*)GWEN_Memory_malloc(size+1);
    assert(bf->realPtr);
    bf->ptr=bf->realPtr;
    bf->realBufferSize=size+1;
    bf->bufferSize=size+1;
    bf->flags=GWEN_BUFFER_FLAGS_OWNED;
    bf->bytesUsed=used;
    bf->ptr[0]=0;
  }
  else {
    /* use existing buffer */
    bf->realPtr=buffer;
    bf->ptr=buffer;
    bf->realBufferSize=size;
    bf->bufferSize=size;
    bf->bytesUsed=used;
    if (take)
      bf->flags=GWEN_BUFFER_FLAGS_OWNED;
  }

  bf->mode=GWEN_BUFFER_MODE_DEFAULT;
  bf->hardLimit=GWEN_BUFFER_DEFAULT_HARDLIMIT;
  bf->step=GWEN_BUFFER_DYNAMIC_STEP;
  return bf;
}



void GWEN_Buffer_Attach(GWEN_BUFFER *bf){
  assert(bf->_refCount);
  bf->_refCount++;
}



void GWEN_Buffer_free(GWEN_BUFFER *bf){
  if (bf) {
    assert(bf->_refCount);
    if (bf->_refCount==1) {
      if (bf->flags & GWEN_BUFFER_FLAGS_OWNED)
        GWEN_Memory_dealloc(bf->realPtr);
      if (bf->syncIo) {
        if (bf->flags & GWEN_BUFFER_FLAGS_OWN_SYNCIO)
          GWEN_SyncIo_free(bf->syncIo);
      }
      GWEN_FREE_OBJECT(bf);
    }
    else
      bf->_refCount--;
  }
}



GWEN_BUFFER *GWEN_Buffer_dup(GWEN_BUFFER *bf) {
  GWEN_BUFFER *newbf;
  uint32_t i;

  GWEN_NEW_OBJECT(GWEN_BUFFER, newbf);
  newbf->_refCount=1;

  if (bf->realPtr && bf->realBufferSize) {
    newbf->realPtr=(char*)GWEN_Memory_malloc(bf->realBufferSize);
    newbf->ptr=newbf->realPtr+(bf->ptr-bf->realPtr);
    newbf->realBufferSize=bf->realBufferSize;
    newbf->bufferSize=bf->bufferSize;
    newbf->bytesUsed=bf->bytesUsed;
    if (newbf->bytesUsed) {
      unsigned int toCopy;

      toCopy=bf->bytesUsed+1;
      if (toCopy>(newbf->bufferSize)) {
        fprintf(stderr, "Panic: Too many bytes in buffer");
        abort();
      }
      memmove(newbf->ptr, bf->ptr, toCopy);
    }
    newbf->pos=bf->pos;
  }
  newbf->flags=bf->flags | GWEN_BUFFER_FLAGS_OWNED;
  newbf->mode=bf->mode&GWEN_BUFFER_MODE_COPYMASK;
  newbf->hardLimit=bf->hardLimit;
  newbf->step=bf->step;
  for (i=0; i<GWEN_BUFFER_MAX_BOOKMARKS; i++)
    newbf->bookmarks[i]=bf->bookmarks[i];

  return newbf;
}



int GWEN_Buffer_Relinquish(GWEN_BUFFER *bf) {
  assert(bf);
  if (!(bf->flags & GWEN_BUFFER_FLAGS_OWNED))
    return GWEN_ERROR_INVALID;
  if (bf->realPtr!=bf->ptr)
    return GWEN_ERROR_INVALID;

  bf->flags&=~GWEN_BUFFER_FLAGS_OWNED;
  return 0;
}



int GWEN_Buffer_ReserveBytes(GWEN_BUFFER *bf, uint32_t res){
  assert(bf);
  if (!res)
    return 0;

  if (bf->bytesUsed) {
    /* we need to move data */
    if (GWEN_Buffer_AllocRoom(bf, res))
      return -1;

    memmove(bf->ptr+res, bf->ptr, bf->bytesUsed);
    bf->ptr+=res;
    bf->bufferSize-=res;
    return 0;
  }
  else {
    /* no data in buffer, so simply move ptrs */
    if (GWEN_Buffer_AllocRoom(bf, res))
      return -1;

    bf->ptr+=res;
    bf->bufferSize-=res;
    if (bf->bufferSize)
      bf->ptr[0]=0;
    return 0;
  }
}



uint32_t GWEN_Buffer_GetMode(GWEN_BUFFER *bf){
  assert(bf);
  return bf->mode;
}



void GWEN_Buffer_SetMode(GWEN_BUFFER *bf, uint32_t mode){
  assert(bf);
  bf->mode=mode;
}


void GWEN_Buffer_AddMode(GWEN_BUFFER *bf, uint32_t mode){
  assert(bf);
  bf->mode|=mode;
}


void GWEN_Buffer_SubMode(GWEN_BUFFER *bf, uint32_t mode){
  assert(bf);
  bf->mode&=~mode;
}



uint32_t GWEN_Buffer_GetHardLimit(GWEN_BUFFER *bf){
  assert(bf);
  return bf->hardLimit;
}



void GWEN_Buffer_SetHardLimit(GWEN_BUFFER *bf, uint32_t l){
  assert(bf);
  assert(l);
  bf->hardLimit=l;
}



char *GWEN_Buffer_GetStart(const GWEN_BUFFER *bf){
  assert(bf);
  return bf->ptr;
}



uint32_t GWEN_Buffer_GetSize(const GWEN_BUFFER *bf){
  assert(bf);
  if (bf->mode & GWEN_BUFFER_MODE_DYNAMIC)
    return bf->hardLimit;
  return bf->bufferSize;
}



uint32_t GWEN_Buffer_GetPos(const GWEN_BUFFER *bf){
  assert(bf);
  return bf->pos;
}



int GWEN_Buffer_SetPos(GWEN_BUFFER *bf, uint32_t i){
  assert(bf);

  if (i>=bf->bufferSize) {
    if (bf->mode & GWEN_BUFFER_MODE_USE_SYNCIO) {
      bf->pos=i;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Position %d outside buffer boundaries (%d bytes)",
                i, bf->bufferSize);
      return GWEN_ERROR_BUFFER_OVERFLOW;
    }
  }
  bf->pos=i;
  return 0;
}



uint32_t GWEN_Buffer_GetUsedBytes(const GWEN_BUFFER *bf){
  assert(bf);
  return bf->bytesUsed;
}



int GWEN_Buffer_AllocRoom(GWEN_BUFFER *bf, uint32_t size) {
  assert(bf);
  if (bf->mode & GWEN_BUFFER_MODE_READONLY) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Read-only mode");
    if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
      abort();
      return GWEN_ERROR_PERMISSIONS;
    }
  }
  /*DBG_VERBOUS(GWEN_LOGDOMAIN, "Allocating %d bytes", size);*/
  /*if (bf->pos+size>bf->bufferSize) {*/
  if (bf->bytesUsed+(size+1) > bf->bufferSize) {
    /* need to realloc */
    uint32_t nsize;
    uint32_t noffs;
    uint32_t reserved;
    void *p;

    /* check for dynamic mode */
    if (!(bf->mode & GWEN_BUFFER_MODE_DYNAMIC)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Not in dynamic mode");
      if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
        abort();
      }
      return GWEN_ERROR_BUFFER_OVERFLOW;
    }

    /* calculate reserved bytes (to set ptr later) */
    reserved=bf->ptr-bf->realPtr;
    /* this is the raw number of bytes we need */
    /*nsize=bf->pos+size-bf->bufferSize;*/
    nsize=bf->bytesUsed+(size+1)-bf->bufferSize;
    /* round it up */
    nsize=(nsize+(bf->step-1));
    nsize&=~(bf->step-1);
    /* store number of additional bytes to allocate */
    noffs=nsize;
    /* add current size to it */
    nsize+=bf->realBufferSize;
    if (nsize>bf->hardLimit) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Size is beyond hard limit (%d>%d)",
                nsize, bf->hardLimit);
      if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
        abort();
      }
      return GWEN_ERROR_BUFFER_OVERFLOW;
    }
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Reallocating from %d to %d bytes",
                bf->bufferSize, nsize);
    /* we always add a NULL character */
    if (bf->realPtr==NULL) {
      p=GWEN_Memory_malloc(nsize+1);
    }
    else {
      p=GWEN_Memory_realloc(bf->realPtr, nsize+1);
    }
    if (!p) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Realloc failed.");
      if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
	abort();
      }
      return GWEN_ERROR_MEMORY_FULL;
    }

    /* store new size and pointer */
    bf->realPtr=p;
    bf->ptr=bf->realPtr+reserved;
    bf->realBufferSize=nsize;
    bf->bufferSize+=noffs;
  }

  return 0;
}



int GWEN_Buffer_AppendBytes(GWEN_BUFFER *bf,
                            const char *buffer,
                            uint32_t size){
  int rv;

  assert(bf);

  if (bf->mode & GWEN_BUFFER_MODE_READONLY) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Read-only mode");
    if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
      abort();
      return GWEN_ERROR_PERMISSIONS;
    }
  }

  rv=GWEN_Buffer_AllocRoom(bf, size+1);
  if (rv<0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "called from here");
    return rv;
  }
  /* if (bf->pos+size>bf->bufferSize) { */
  if (bf->bytesUsed+size>bf->bufferSize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer full (%d [%d] of %d bytes)",
              /*bf->pos, size,*/
              bf->bytesUsed, size+1,
              bf->bufferSize);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }

  memmove(bf->ptr+bf->bytesUsed, buffer, size);
  /*bf->pos+=size;*/
  if (bf->pos==bf->bytesUsed)
    bf->pos+=size;
  bf->bytesUsed+=size;
  /* append a NULL to allow using the buffer as ASCIIZ string */
  bf->ptr[bf->bytesUsed]=0;
  return 0;
}



int GWEN_Buffer_AppendByte(GWEN_BUFFER *bf, char c){
  int rv;

  assert(bf);

  if (bf->mode & GWEN_BUFFER_MODE_READONLY) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Read-only mode");
    if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
      abort();
      return GWEN_ERROR_PERMISSIONS;
    }
  }

  if (bf->bytesUsed+1+1 > bf->bufferSize) {
    rv=GWEN_Buffer_AllocRoom(bf, 1+1);
    if (rv<0) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "here");
      return rv;
    }
  }

  bf->ptr[bf->bytesUsed]=c;
  if (bf->pos == bf->bytesUsed)
    bf->pos++;
  /* append a NULL to allow using the buffer as ASCIIZ string */
  bf->ptr[++(bf->bytesUsed)]=0;
  return 0;
}



int GWEN_Buffer__FillBuffer_SyncIo(GWEN_BUFFER *bf){
  if (bf->syncIo) {
    uint32_t toread;
    int rv;

    toread=bf->pos-bf->bytesUsed+1;
    if (GWEN_Buffer_AllocRoom(bf, toread+1)) {
      DBG_INFO(GWEN_LOGDOMAIN, "Buffer too small");
      return GWEN_ERROR_GENERIC;
    }
    rv=GWEN_SyncIo_ReadForced(bf->syncIo,
			      (uint8_t*) (bf->ptr+bf->bytesUsed),
			      toread);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    else if (rv==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
      return GWEN_ERROR_EOF;
    }

    bf->bytesUsed+=rv;
  }
  else {
    DBG_DEBUG(GWEN_LOGDOMAIN,
	      "End of used area reached and no SYNCIO (%d bytes)",
	      bf->pos);
    return GWEN_ERROR_EOF;
  }
  return 0;
}



int GWEN_Buffer__FillBuffer(GWEN_BUFFER *bf){
  assert(bf);
  if (bf->mode & GWEN_BUFFER_MODE_USE_SYNCIO)
    return GWEN_Buffer__FillBuffer_SyncIo(bf);
  else {
    DBG_DEBUG(GWEN_LOGDOMAIN,
	      "End of used area reached (%d bytes)", bf->pos);
    return GWEN_ERROR_EOF;
  }
}



int GWEN_Buffer_PeekByte(GWEN_BUFFER *bf){
  assert(bf);

  if (bf->pos>=bf->bytesUsed) {
    int rv;

    rv=GWEN_Buffer__FillBuffer(bf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return (unsigned char) (bf->ptr[bf->pos]);
}



int GWEN_Buffer_ReadByte(GWEN_BUFFER *bf){
  assert(bf);

  if (bf->pos>=bf->bytesUsed) {
    int rv;

    rv=GWEN_Buffer__FillBuffer(bf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return (unsigned char) (bf->ptr[bf->pos++]);
}



int GWEN_Buffer_IncrementPos(GWEN_BUFFER *bf, uint32_t i){
  assert(bf);

  if (i+bf->pos>=bf->bufferSize) {
    if (!(bf->mode & GWEN_BUFFER_MODE_USE_SYNCIO)) {
      DBG_DEBUG(GWEN_LOGDOMAIN,
                "Position %d outside buffer boundaries (%d bytes)\n"
                "Incrementing anyway",
		i+bf->pos, bf->bufferSize);
    }
  }

  bf->pos+=i;
  return 0;
}



int GWEN_Buffer_AdjustUsedBytes(GWEN_BUFFER *bf){
  assert(bf);
  if (bf->pos<=bf->bufferSize) {
    if (bf->pos>bf->bytesUsed) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Adjusted buffer (uses now %d bytes)",
               bf->pos);
      bf->bytesUsed=bf->pos;
    }
    /* append a NULL to allow using the buffer as ASCIIZ string */
    bf->ptr[bf->bytesUsed]=0;
    return 0;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Pointer outside buffer size (%d bytes)",
              bf->bufferSize);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }
}



int GWEN_Buffer_DecrementPos(GWEN_BUFFER *bf, uint32_t i){
  assert(bf);

  if (bf->pos<i) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Position %d outside buffer boundaries (%d bytes)",
              bf->pos-i, bf->bufferSize);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }
  bf->pos-=i;
  return 0;
}



int GWEN_Buffer_AppendBuffer(GWEN_BUFFER *bf, GWEN_BUFFER *sf){

  assert(bf);
  assert(sf);

  if (bf->mode & GWEN_BUFFER_MODE_READONLY) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Read-only mode");
    if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
      abort();
      return GWEN_ERROR_PERMISSIONS;
    }
  }

  if (sf->bytesUsed)
    return GWEN_Buffer_AppendBytes(bf, sf->ptr, sf->bytesUsed);
  return 0;
}



uint32_t GWEN_Buffer_GetMaxUnsegmentedWrite(GWEN_BUFFER *bf){
  assert(bf);

  return (bf->bufferSize-bf->bytesUsed);
}



uint32_t GWEN_Buffer_GetBytesLeft(GWEN_BUFFER *bf){
  assert(bf);

  if (bf->pos<bf->bytesUsed)
    return bf->bytesUsed-bf->pos;
  else
    return 0;
}



char *GWEN_Buffer_GetPosPointer(const GWEN_BUFFER *bf){
  assert(bf);
  return bf->ptr+bf->pos;
}



void GWEN_Buffer_OverwriteContent(GWEN_BUFFER *bf, int c) {
  assert(bf);
  if (bf->realPtr && bf->realBufferSize) {
    memset(bf->realPtr, c, bf->realBufferSize);
  }
}



uint32_t GWEN_Buffer_GetBookmark(const GWEN_BUFFER *bf, unsigned int idx){
  assert(bf);
  assert(idx<GWEN_BUFFER_MAX_BOOKMARKS);
  return bf->bookmarks[idx];
}



void GWEN_Buffer_SetBookmark(GWEN_BUFFER *bf, unsigned int idx,
                             uint32_t v){
  assert(bf);
  assert(idx<GWEN_BUFFER_MAX_BOOKMARKS);
  bf->bookmarks[idx]=v;
}



void GWEN_Buffer_Dump(GWEN_BUFFER *bf, unsigned int insert) {
  uint32_t k;

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Buffer:\n");

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Pos            : %d (%04x)\n", bf->pos, bf->pos);

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Buffer Size    : %d\n", bf->bufferSize);

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Hard limit     : %d\n", bf->hardLimit);

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Bytes Used     : %d\n", bf->bytesUsed);

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Bytes Reserved : %u\n",
          (uint32_t)(bf->ptr-bf->realPtr));

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Flags          : %08x ( ", bf->flags);
  if (bf->flags & GWEN_BUFFER_FLAGS_OWNED)
    fprintf(stderr, "OWNED ");
  fprintf(stderr, ")\n");

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Mode           : %08x ( ", bf->mode);
  if (bf->mode & GWEN_BUFFER_MODE_DYNAMIC)
    fprintf(stderr, "DYNAMIC ");
  if (bf->mode & GWEN_BUFFER_MODE_READONLY)
    fprintf(stderr, "READONLY ");
  if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL)
    fprintf(stderr, "ABORT_ON_MEMFULL ");
  fprintf(stderr, ")\n");

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Bookmarks      :");
  for (k=0; k<GWEN_BUFFER_MAX_BOOKMARKS; k++)
    fprintf(stderr, " %d", bf->bookmarks[k]);
  fprintf(stderr, "\n");

  if (bf->ptr && bf->bytesUsed) {
    for (k=0; k<insert; k++)
      fprintf(stderr, " ");
    fprintf(stderr, "Data:\n");
    GWEN_Text_DumpString(bf->ptr, bf->bytesUsed, insert+1);
  }
}



void GWEN_Buffer_Reset(GWEN_BUFFER *bf){
  assert(bf);
  bf->pos=0;
  bf->bytesUsed=0;
  bf->ptr[0]=0;
}



void GWEN_Buffer_Rewind(GWEN_BUFFER *bf){
  assert(bf);
  bf->pos=0;
}



int GWEN_Buffer_ReadBytes(GWEN_BUFFER *bf, char *buffer, uint32_t *size){
  /* optimized for speed */
  uint32_t i;
  char *pdst;

  DBG_VERBOUS(GWEN_LOGDOMAIN, "About to copy up to %d bytes", *size);
  i=0;
  pdst=buffer;

  while(i<*size) {
    int j;
    int srcLeft;

    if (bf->pos>=bf->bytesUsed) {
      if (GWEN_Buffer__FillBuffer(bf)) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "Could not fill buffer, but that's ok");
        break;
      }
    }

    srcLeft=bf->bytesUsed - bf->pos;
    if (srcLeft==0)
      break;
    j=(*size)-i;
    if (j>srcLeft)
      j=srcLeft;
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Copying %d bytes", j);
    memmove(pdst, bf->ptr + bf->pos, j);
    pdst+=j;
    i+=j;
    bf->pos+=j;
  } /* while */

  *size=i;
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Copied %d bytes", *size);
  return 0;
}



uint32_t GWEN_Buffer_GetStep(GWEN_BUFFER *bf){
  assert(bf);
  return bf->step;
}



void GWEN_Buffer_SetStep(GWEN_BUFFER *bf, uint32_t step){
  assert(bf);
  bf->step=step;
}



void GWEN_Buffer_AdjustBookmarks(GWEN_BUFFER *bf,
                                 uint32_t pos,
                                 int offset) {
  uint32_t i;

  assert(bf);
  for (i=0; i<GWEN_BUFFER_MAX_BOOKMARKS; i++) {
    if (bf->bookmarks[i]>=pos)
      bf->bookmarks[i]+=offset;
  } /* for */
}



int GWEN_Buffer_InsertRoom(GWEN_BUFFER *bf,
                           uint32_t size){
  char *p;
  int i;
  int rv;

  assert(bf);

  if (bf->mode & GWEN_BUFFER_MODE_READONLY) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Read-only mode");
    if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
      abort();
      return GWEN_ERROR_PERMISSIONS;
    }
  }

  if (bf->pos==0) {
    if (bf->bytesUsed==0) {
      /* no bytes used, simply return */
      rv=GWEN_Buffer_AllocRoom(bf, size);
      if (rv) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "called from here");
        return rv;
      }
      bf->bytesUsed+=size;
      /* append "0" behind buffer */
      bf->ptr[bf->bytesUsed]=0;
      return 0;
    }
    else {
      if ( (bf->ptr - bf->realPtr) >= (int)size ) {
        /* simply occupy the reserved space */
        bf->ptr-=size;
        bf->bytesUsed+=size;
        bf->bufferSize+=size;
        GWEN_Buffer_AdjustBookmarks(bf, bf->pos, size);
        return 0;
      }
    }
  }

  rv=GWEN_Buffer_AllocRoom(bf, size);
  if (rv<0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "called from here");
    return rv;
  }
  if (bf->pos+size>bf->bufferSize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer full (%d [%d] of %d bytes)",
	      bf->pos, size,
	      bf->bufferSize);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }
  p=bf->ptr+bf->pos;
  i=bf->bytesUsed-bf->pos;
  if (i>0)
    /* move current data at pos out of the way */
    memmove(p+size, p, i);
  bf->bytesUsed+=size;
  /* append "0" behind buffer */
  bf->ptr[bf->bytesUsed]=0;
  GWEN_Buffer_AdjustBookmarks(bf, bf->pos, size);
  return 0;
}



int GWEN_Buffer_RemoveRoom(GWEN_BUFFER *bf, uint32_t size){
  char *p;
  int i;

  assert(bf);

  if (bf->mode & GWEN_BUFFER_MODE_READONLY) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Read-only mode");
    if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
      abort();
      return GWEN_ERROR_PERMISSIONS;
    }
  }

  if (bf->pos==0) {
    if (bf->bytesUsed<size) {
      /* can't remove bytes we don't have */
      return GWEN_ERROR_INVALID;
    }
    /* simply add to reserved space */
    bf->ptr+=size;
    bf->bytesUsed-=size;
    bf->bufferSize-=size;
  }
  else {
    if (bf->bytesUsed+size<(bf->bytesUsed)) {
      /* can't remove more bytes than we have */
      return GWEN_ERROR_INVALID;
    }

    /* we need to get the rest closer */
    p=bf->ptr+bf->pos+size;
    i=bf->bytesUsed-bf->pos-size;
    memmove(bf->ptr+bf->pos, p, i);
    bf->bytesUsed+=size;
  }

  /* append "0" behind buffer */
  bf->ptr[bf->bytesUsed]=0;
  GWEN_Buffer_AdjustBookmarks(bf, bf->pos, -((int)size));

  return 0;
}



int GWEN_Buffer_ReplaceBytes(GWEN_BUFFER *bf,
			     uint32_t rsize,
			     const char *buffer,
			     uint32_t size){
  int32_t d;
  int rv;

  if (bf->mode & GWEN_BUFFER_MODE_READONLY) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Read-only mode");
    if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
      abort();
      return GWEN_ERROR_PERMISSIONS;
    }
  }

  /* either insert or remove bytes */
  d=size-rsize;
  if (d<0) {
    rv=GWEN_Buffer_RemoveRoom(bf, -d);
  }
  else if (d>0) {
    rv=GWEN_Buffer_InsertRoom(bf, d);
  }
  else
    /* nothing to adjust if sizes are the same */
    rv=0;
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Error replacing %d bytes with %d bytes (%d)",
	      rsize, size, rv);
    return rv;
  }

  /* write new bytes */
  if (size)
    memmove(bf->ptr+bf->pos, buffer, size);
  return 0;
}



int GWEN_Buffer_InsertBytes(GWEN_BUFFER *bf,
                            const char *buffer,
                            uint32_t size){
  int rv;

  assert(bf);
  assert(buffer);

  if (bf->mode & GWEN_BUFFER_MODE_READONLY) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Read-only mode");
    if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
      abort();
      return GWEN_ERROR_PERMISSIONS;
    }
  }

  rv=GWEN_Buffer_InsertRoom(bf, size);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  memmove(bf->ptr+bf->pos, buffer, size);
  return 0;
}



int GWEN_Buffer_InsertByte(GWEN_BUFFER *bf, char c){
  int rv;

  assert(bf);

  if (bf->mode & GWEN_BUFFER_MODE_READONLY) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Read-only mode");
    if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
      abort();
      return GWEN_ERROR_PERMISSIONS;
    }
  }

  rv=GWEN_Buffer_InsertRoom(bf, 1);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  bf->ptr[bf->pos]=c;
  return 0;
}



int GWEN_Buffer_InsertBuffer(GWEN_BUFFER *bf,
                             GWEN_BUFFER *sf){
  assert(bf);
  assert(sf);

  return GWEN_Buffer_InsertBytes(bf, sf->ptr, sf->bytesUsed);
}



int GWEN_Buffer_Crop(GWEN_BUFFER *bf,
                     uint32_t pos,
                     uint32_t l) {

  if (bf->mode & GWEN_BUFFER_MODE_READONLY) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Read-only mode");
    if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
      abort();
      return GWEN_ERROR_PERMISSIONS;
    }
  }

  if (pos>=bf->bufferSize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Position outside buffer");
    return -1;
  }
  bf->ptr+=pos;
  bf->bufferSize-=pos;
  bf->pos-=pos;
  if (bf->bytesUsed-pos<l) {
    DBG_INFO(GWEN_LOGDOMAIN, "Invalid length");
    return -1;
  }
  bf->bytesUsed=l;
  GWEN_Buffer_AdjustBookmarks(bf, pos, -pos);
  /* adjust position after possible truncation */
  if (bf->pos>bf->bytesUsed)
      bf->pos=bf->bytesUsed;

  bf->ptr[bf->bytesUsed]=0;

  return 0;
}



int GWEN_Buffer_AppendString(GWEN_BUFFER *bf,
                             const char *buffer){
  assert(bf);
  assert(buffer);
  return GWEN_Buffer_AppendBytes(bf, buffer, strlen(buffer));
}



int GWEN_Buffer_InsertString(GWEN_BUFFER *bf,
                             const char *buffer){
  assert(bf);
  assert(buffer);
  return GWEN_Buffer_InsertBytes(bf, buffer, strlen(buffer));
}



void GWEN_Buffer_SetSourceSyncIo(GWEN_BUFFER *bf,
				 GWEN_SYNCIO *sio,
				 int take) {
  assert(bf);
  if (bf->syncIo) {
    if (bf->flags & GWEN_BUFFER_FLAGS_OWN_SYNCIO) {
      GWEN_SyncIo_free(bf->syncIo);
    }
  }
  if (take)
    bf->flags|=GWEN_BUFFER_FLAGS_OWN_SYNCIO;
  else
    bf->flags&=~GWEN_BUFFER_FLAGS_OWN_SYNCIO;
  bf->syncIo=sio;
}



int GWEN_Buffer_FillWithBytes(GWEN_BUFFER *bf,
                              unsigned char c,
                              uint32_t size){
  int rv;

  assert(bf);

  if (bf->mode & GWEN_BUFFER_MODE_READONLY) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Read-only mode");
    if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
      abort();
      return GWEN_ERROR_PERMISSIONS;
    }
  }

  rv=GWEN_Buffer_AllocRoom(bf, size+1);
  if (rv<0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "called from here");
    return rv;
  }
  /* if (bf->pos+size>bf->bufferSize) { */
  if (bf->bytesUsed+size>bf->bufferSize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer full (%d [%d] of %d bytes)",
              bf->bytesUsed, size+1,
              bf->bufferSize);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }
  memset(bf->ptr+bf->bytesUsed, c, size);
  if (bf->pos==bf->bytesUsed)
    bf->pos+=size;
  bf->bytesUsed+=size;
  /* append a NULL to allow using the buffer as ASCIIZ string */
  bf->ptr[bf->bytesUsed]=0;
  return 0;
}



int GWEN_Buffer_FillLeftWithBytes(GWEN_BUFFER *bf,
                                  unsigned char c,
                                  uint32_t size){
  int rv;

  assert(bf);

  if (bf->mode & GWEN_BUFFER_MODE_READONLY) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Read-only mode");
    if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
      abort();
      return GWEN_ERROR_PERMISSIONS;
    }
  }

  rv=GWEN_Buffer_InsertRoom(bf, size);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return -1;
  }
  memset(bf->ptr+bf->pos, c, size);
  return 0;
}







