/***************************************************************************
    begin       : Tue Apr 27 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "fastbuffer2.h"

#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <assert.h>


GWEN_FAST_BUFFER2 *GWEN_FastBuffer2_new(uint32_t bsize, GWEN_SYNCIO *io) {
  GWEN_FAST_BUFFER2 *fb;

  assert(bsize);

  fb=(GWEN_FAST_BUFFER2*) malloc(sizeof(GWEN_FAST_BUFFER2)+bsize);
  assert(fb);
  memset(fb, 0, sizeof(GWEN_FAST_BUFFER2)+bsize);

  fb->bufferSize=bsize;

  fb->io=io;

  return fb;
}



void GWEN_FastBuffer2_free(GWEN_FAST_BUFFER2 *fb) {
  if (fb) {
    GWEN_FREE_OBJECT(fb);
  }
}



int GWEN_FastBuffer2_ReadLine(GWEN_FAST_BUFFER2 *fb, uint8_t *p, int len) {
  int bytes;
  int copied=0;

  if (fb->bufferReadPos>=fb->bufferWritePos) {
    int rv;

    rv=GWEN_SyncIo_Read(fb->io, fb->buffer, fb->bufferSize);
    if (rv<0) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  bytes=fb->bufferWritePos-fb->bufferReadPos;
  if (bytes>len)
    bytes=len;
  while(bytes) {
    uint8_t c;

    c=fb->buffer[fb->bufferReadPos++];
    fb->bytesRead++;
    if (c==10) {
      *(p++)=c;
      copied++;
      break;
    }
    else if (c!=13) {
      *(p++)=c;
      copied++;
    }
    bytes--;
  } /* while */

  return copied;
}



int GWEN_FastBuffer2_ReadLineToBuffer(GWEN_FAST_BUFFER2 *fb, GWEN_BUFFER *buf) {
  int lineComplete=0;
  int hadSome=0;

  while (!lineComplete) {
    int bytes;
    int copied=0;
    uint8_t *p;

    if (fb->bufferReadPos>=fb->bufferWritePos) {
      int rv;
  
      rv=GWEN_SyncIo_Read(fb->io, fb->buffer, fb->bufferSize);
      if (rv<0) {
	if (rv==GWEN_ERROR_EOF && hadSome) {
          /* done */
	  return 0;
	}
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
      else if (rv==0) {
	DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
	return GWEN_ERROR_EOF;
      }
      else {
	fb->bufferWritePos=rv;
	fb->bufferReadPos=0;
      }
    }
  
    bytes=fb->bufferWritePos-fb->bufferReadPos;
    p=fb->buffer+fb->bufferReadPos;
    while(bytes) {
      uint8_t c;

      hadSome=1;

      c=fb->buffer[fb->bufferReadPos++];
      fb->bytesRead++;
      bytes--;
      if (c==10) {
	lineComplete=1;
	/* don't include this character */
	break;
      }
      else if (c==13) {
	break;
      }
      else {
	copied++;
      }
    } /* while */

    if (copied)
      GWEN_Buffer_AppendBytes(buf, (const char*)p, copied);
  }

  return 0;
}



uint32_t GWEN_FastBuffer2_GetFlags(const GWEN_FAST_BUFFER2 *fb) {
  assert(fb);
  return fb->flags;
}



void GWEN_FastBuffer2_SetFlags(GWEN_FAST_BUFFER2 *fb, uint32_t fl) {
  assert(fb);
  fb->flags=fl;
}



void GWEN_FastBuffer2_AddFlags(GWEN_FAST_BUFFER2 *fb, uint32_t fl) {
  assert(fb);
  fb->flags|=fl;
}



void GWEN_FastBuffer2_SubFlags(GWEN_FAST_BUFFER2 *fb, uint32_t fl) {
  assert(fb);
  fb->flags&=~fl;
}



uint32_t GWEN_FastBuffer2_GetBytesWritten(const GWEN_FAST_BUFFER2 *fb) {
  assert(fb);
  return fb->bytesWritten;
}



uint32_t GWEN_FastBuffer2_GetBytesRead(const GWEN_FAST_BUFFER2 *fb) {
  assert(fb);
  return fb->bytesRead;
}











