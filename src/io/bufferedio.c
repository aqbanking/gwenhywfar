/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Fri Feb 07 2003
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

#define DISABLE_DEBUGLOG


#include "bufferedio_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/text.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "gwenhyfwar/debug.h"


const char *GWEN_BufferedIO_ErrorString(int c){
  const char *s;

  switch(c) {
  case GWEN_BUFFEREDIO_ERROR_READ:
    s="Error on read";
    break;
  case GWEN_BUFFEREDIO_ERROR_WRITE:
    s="Error on write";
    break;
  case GWEN_BUFFEREDIO_ERROR_CLOSE:
    s="Error on write";
    break;
  case GWEN_BUFFEREDIO_ERROR_TIMEOUT:
    s="Transaction timout";
    break;
  default:
    s=0;
  } /* switch */

  return s;
}



static int gwen_bufferedio_is_initialized=0;
static GWEN_ERRORTYPEREGISTRATIONFORM *gwen_bufferedio_errorform=0;



GWEN_ERRORCODE GWEN_BufferedIO_ModuleInit(){
  if (!gwen_bufferedio_is_initialized) {
    GWEN_ERRORCODE err;

    gwen_bufferedio_errorform=GWEN_ErrorType_new();
    GWEN_ErrorType_SetName(gwen_bufferedio_errorform,
                           GWEN_BUFFEREDIO_ERROR_TYPE);
    GWEN_ErrorType_SetMsgPtr(gwen_bufferedio_errorform,
                             GWEN_BufferedIO_ErrorString);
    err=GWEN_Error_RegisterType(gwen_bufferedio_errorform);
    if (!GWEN_Error_IsOk(err))
      return err;
    gwen_bufferedio_is_initialized=1;
  }
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_ModuleFini(){
  if (gwen_bufferedio_is_initialized) {
    GWEN_ERRORCODE err;

    err=GWEN_Error_UnregisterType(gwen_bufferedio_errorform);
    GWEN_ErrorType_free(gwen_bufferedio_errorform);
    if (!GWEN_Error_IsOk(err))
      return err;
    gwen_bufferedio_is_initialized=0;
  }
  return 0;
}



GWEN_BUFFEREDIO *GWEN_BufferedIO_new(){
  GWEN_BUFFEREDIO *bt;

  GWEN_NEW_OBJECT(GWEN_BUFFEREDIO, bt);
  bt->lineMode=GWEN_LineModeUnix;
  return bt;
}



void GWEN_BufferedIO_free(GWEN_BUFFEREDIO *bt){
  if (bt) {
    if (bt->freePtr)
      bt->freePtr(bt->privateData);
    free(bt->readerBuffer);
    free(bt->writerBuffer);
    free(bt);
  }
}



void GWEN_BufferedIO_SetReadBuffer(GWEN_BUFFEREDIO *bt,
                                   char *buffer, int len){
  assert(bt);
  free(bt->readerBuffer);
  bt->readerBuffer=0;
  if (buffer==0) {
    if (len>0) {
      bt->readerBuffer=malloc(len);
      assert(bt->readerBuffer);
    }
  }
  else
    bt->readerBuffer=buffer;

  bt->readerBufferLength=len;
  bt->readerBufferFilled=0;
  bt->readerBufferPos=0;
}



void GWEN_BufferedIO_SetWriteBuffer(GWEN_BUFFEREDIO *bt, char *buffer, int len){
  assert(bt);
  free(bt->writerBuffer);
  bt->writerBuffer=0;
  if (buffer==0) {
    if (len>0) {
      bt->writerBuffer=malloc(len);
      assert(bt->writerBuffer);
    }
  }
  else
    bt->writerBuffer=buffer;

  bt->writerBufferLength=len;
  bt->writerBufferFilled=0;
  bt->writerBufferPos=0;
}



int GWEN_BufferedIO_CheckEOF(GWEN_BUFFEREDIO *bt){
  return (GWEN_BufferedIO_PeekChar(bt)==-2);
}



int GWEN_BufferedIO_PeekChar(GWEN_BUFFEREDIO *bt){
  assert(bt);
  assert(bt->readerBuffer);

  /* do some fast checks */
  if (bt->readerError) {
    DBG_DEBUG(0, "Error flagged");
    return -1;
  }
  if (bt->readerEOF) {
    DBG_DEBUG(0, "EOF flagged");
    return -2;
  }

  if (bt->readerBufferPos>=bt->readerBufferFilled) {
    /* buffer empty, no EOF met, so fill it */
    GWEN_ERRORCODE err;
    int i;

    assert(bt->readPtr);
    i=bt->readerBufferLength;
    err=bt->readPtr(bt,
		    bt->readerBuffer,
		    &i,
		    bt->timeout);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      bt->readerError=1;
      return -1;
    }
    bt->readerBufferFilled=i;
    bt->readerBufferPos=0;
    bt->readerEOF=(i==0);
  }
  if (bt->readerEOF) {
    DBG_DEBUG(0, "EOF now met");
    return -2;
  }
  return (unsigned char)(bt->readerBuffer[bt->readerBufferPos]);
}



int GWEN_BufferedIO_ReadChar(GWEN_BUFFEREDIO *bt){
  int i;

  i=GWEN_BufferedIO_PeekChar(bt);
  if (i>=0)
    bt->readerBufferPos++;
  return i;
}



GWEN_ERRORCODE GWEN_BufferedIO_Flush(GWEN_BUFFEREDIO *bt){
  GWEN_ERRORCODE err;
  int i;
  int written;

  assert(bt);
  if (bt->writerBufferFilled==0) {
    DBG_DEBUG(0, "WriteBuffer empty, nothing to flush.");
    return 0;
  }
  assert(bt->writerBuffer);
  assert(bt->writePtr);
  written=bt->writerBufferFlushPos;
  DBG_DEBUG(0, "Flushing %d bytes", bt->writerBufferFilled);
  while(written<bt->writerBufferFilled) {
    i=bt->writerBufferFilled-written;
    err=bt->writePtr(bt,
		     &(bt->writerBuffer[written]),
		     &i,
		     bt->timeout);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return err;
    }
    written+=i;
  } /* while */

  bt->writerBufferPos=0;
  bt->writerBufferFilled=0;
  bt->writerBufferFlushPos=0;

  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_ShortFlush(GWEN_BUFFEREDIO *bt){
  GWEN_ERRORCODE err;
  int i;

  assert(bt);
  if (bt->writerBufferFilled==0) {
    DBG_DEBUG(0, "WriteBuffer empty, nothing to flush.");
    return 0;
  }
  assert(bt->writerBuffer);
  assert(bt->writePtr);
  i=bt->writerBufferFilled-bt->writerBufferFlushPos;
  DBG_DEBUG(0, "Flushing %d bytes", i);
  err=bt->writePtr(bt,
		   &(bt->writerBuffer[bt->writerBufferFlushPos]),
		   &i,
		   bt->timeout);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return err;
  }
  if (i<bt->writerBufferFilled-bt->writerBufferFlushPos) {
    /* partial flush */
    bt->writerBufferFlushPos+=i;
    return GWEN_Error_new(0,
			  GWEN_ERROR_SEVERITY_WARN,
			  GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
			  GWEN_BUFFEREDIO_ERROR_PARTIAL);
  }
  else {
    /* all bytes written, flush finished */
    bt->writerBufferFlushPos=0;
    bt->writerBufferPos=0;
    bt->writerBufferFilled=0;
    return 0;
  }
}



int GWEN_BufferedIO_ReadBufferEmpty(GWEN_BUFFEREDIO *bt) {
  assert(bt);
  return ((bt->readerBuffer==0) ||
          !bt->readerBufferFilled ||
          bt->readerBufferPos>=bt->readerBufferFilled);
}



int GWEN_BufferedIO_WriteBufferEmpty(GWEN_BUFFEREDIO *bt) {
  assert(bt);
  return ((bt->writerBuffer==0)  ||
          !bt->writerBufferFilled ||
          bt->writerBufferPos>=bt->writerBufferFilled);
}



GWEN_ERRORCODE GWEN_BufferedIO_WriteChar(GWEN_BUFFEREDIO *bt, char c){
  assert(bt);
  assert(bt->writerBuffer);

  /* flush buffer if needed (only needed if last flush attempt failed) */
  if (bt->writerBufferFilled>=bt->writerBufferLength) {
    GWEN_ERRORCODE err;

    err=GWEN_BufferedIO_Flush(bt);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return err;
    }
  }

  /* write char to buffer */
  bt->writerBuffer[bt->writerBufferPos++]=c;
  if (bt->writerBufferPos>bt->writerBufferFilled)
    bt->writerBufferFilled=bt->writerBufferPos;

  /* flush buffer if needed */
  if (bt->writerBufferFilled>=bt->writerBufferLength) {
    GWEN_ERRORCODE err;

    err=GWEN_BufferedIO_Flush(bt);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return err;
    }
  }
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_Close(GWEN_BUFFEREDIO *bt){
  GWEN_ERRORCODE err, err2;

  assert(bt);
  assert(bt->closePtr);
  err=GWEN_BufferedIO_Flush(bt);
  err2=bt->closePtr(bt);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return err;
  }
  if (!GWEN_Error_IsOk(err2)) {
    DBG_ERROR_ERR(0, err2);
    return err2;
  }
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_Abandon(GWEN_BUFFEREDIO *bt){
  GWEN_ERRORCODE err;

  assert(bt);
  assert(bt->closePtr);
  err=bt->closePtr(bt);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return err;
  }
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_ReadLine(GWEN_BUFFEREDIO *bt,
                                        char *buffer,
                                        unsigned int s){
  int c;
  int pos;

  assert(s);
  pos=0;
  /* now read */
  while(s>1) {
    if (GWEN_BufferedIO_CheckEOF(bt)) {
      buffer[pos]=0;
      break;
    }
    c=GWEN_BufferedIO_ReadChar(bt);
    if (c<0) {
      DBG_ERROR(0, "Error while reading");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                            GWEN_BUFFEREDIO_ERROR_READ);
    }

    if (c==GWEN_BUFFEREDIO_LF) {
      /* LF ends every line */
      buffer[pos]=0;
      break;
    }

    if (c!=GWEN_BUFFEREDIO_CR || bt->lineMode==GWEN_LineModeUnix) {
      buffer[pos]=(unsigned char)c;
      pos++;
      s--;
    }
  } /* while */

  /* add terminating null */
  if (s)
    buffer[pos]=0;

  /* reading done */
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_Write(GWEN_BUFFEREDIO *bt,
                                     const char *buffer){
  GWEN_ERRORCODE err;

  assert(bt);
  assert(buffer);
  while(*buffer) {
    err=GWEN_BufferedIO_WriteChar(bt, *buffer);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return err;
    }
    buffer++;
  } /* while */
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_WriteLine(GWEN_BUFFEREDIO *bt,
                                         const char *buffer){
  GWEN_ERRORCODE err;

  assert(bt);
  assert(buffer);
  err=GWEN_BufferedIO_Write(bt, buffer);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return err;
  }
  if (bt->lineMode==GWEN_LineModeDOS) {
    err=GWEN_BufferedIO_WriteChar(bt, GWEN_BUFFEREDIO_CR);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return err;
    }
  }
  err=GWEN_BufferedIO_WriteChar(bt, GWEN_BUFFEREDIO_LF);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return err;
  }

  err=GWEN_BufferedIO_Flush(bt);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return err;
  }

  return 0;
}



void GWEN_BufferedIO_SetLineMode(GWEN_BUFFEREDIO *dm,
                                 GWEN_BUFFEREDIOLINEMODE lm){
  assert(dm);
  dm->lineMode=lm;
}



GWEN_BUFFEREDIOLINEMODE GWEN_BufferedIO_GetLineMode(GWEN_BUFFEREDIO *dm){
  assert(dm);
  return dm->lineMode;
}



void GWEN_BufferedIO_SetTimeout(GWEN_BUFFEREDIO *dm, int timeout){
  assert(dm);
  dm->timeout=timeout;
}



int GWEN_BufferedIO_GetTimeout(GWEN_BUFFEREDIO *dm){
  assert(dm);
  return dm->timeout;
}



GWEN_ERRORCODE GWEN_BufferedIO_WriteRaw(GWEN_BUFFEREDIO *bt,
                                        const char *buffer,
                                        unsigned int *bsize){

  GWEN_ERRORCODE err;
  int i;

  assert(bt);
  assert(bsize);
  assert(*bsize);

  if (bt->writerBufferFilled) {
    /* some data in the buffer, this must be flushed first */
    err=GWEN_BufferedIO_ShortFlush(bt);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return err;
    }
    if (GWEN_Error_GetType(err)==
        GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE) &&
        GWEN_Error_GetCode(err)==GWEN_BUFFEREDIO_ERROR_PARTIAL) {
      /* still some bytes in the buffer, can not write right now */
      *bsize=0;
      return err;
    }
  }

  /* try to write as many bytes as possible */
  i=*bsize;
  err=bt->writePtr(bt,
                   buffer,
                   &i,
                   bt->timeout);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return err;
  }
  *bsize=i;
  return err;
}



GWEN_ERRORCODE GWEN_BufferedIO_ReadRaw(GWEN_BUFFEREDIO *bt,
                                       char *buffer,
                                       unsigned int *bsize){
  assert(bt);

  /* do some fast checks */
  if (bt->readerError) {
    DBG_DEBUG(0, "Error flagged");
    return -1;
  }
  if (bt->readerEOF) {
    DBG_DEBUG(0, "EOF flagged");
    return -2;
  }

  if (bt->readerBufferPos<bt->readerBufferFilled) {
    /* buffer not empty, so read from the buffer first */
    int i;

    i=bt->readerBufferFilled-bt->readerBufferPos;
    if (i>*bsize)
      i=*bsize;
    DBG_DEBUG(0, "Reading rest from buffer (%d at %d of %d)",
             i,bt->readerBufferPos, bt->readerBufferFilled);

    if (i) {
      /* copy as much bytes as needed, advance pointer */
      memmove(buffer, bt->readerBuffer+bt->readerBufferPos, i);
      bt->readerBufferPos+=i;
    }
    *bsize=i;
    DBG_DEBUG(0, "Read %d bytes from buffer", i);
    return 0;
  }
  else {
    /* buffer empty, so read directly from source */
    GWEN_ERRORCODE err;
    int i;

    DBG_DEBUG(0, "Reading directly from source");
    assert(bt->readPtr);
    i=*bsize;
    err=bt->readPtr(bt,
                    buffer,
                    &i,
                    bt->timeout);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      bt->readerError=1;
      return -1;
    }
    bt->readerEOF=(i==0);
    *bsize=i;
    DBG_DEBUG(0, "Read %d bytes from source", i);
  }
  if (bt->readerEOF) {
    DBG_DEBUG(0, "EOF now met");
    return -2;
  }
  return 0;
}




/*_________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                           File Module
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




struct GWEN_BUFFEREDIO_FILE_STRUCT {
  int fd;
};
typedef struct GWEN_BUFFEREDIO_FILE_STRUCT GWEN_BUFFEREDIO_FILE_TABLE;




GWEN_BUFFEREDIO_FILE_TABLE *GWEN_BufferedIO_File_Table__new() {
  GWEN_BUFFEREDIO_FILE_TABLE *bft;

  bft=(GWEN_BUFFEREDIO_FILE_TABLE *)malloc(sizeof(GWEN_BUFFEREDIO_FILE_TABLE));
  assert(bft);
  memset(bft,0,sizeof(GWEN_BUFFEREDIO_FILE_TABLE));
  bft->fd=-1;
  return bft;
}



void GWEN_BufferedIO_File_Table__free(GWEN_BUFFEREDIO_FILE_TABLE *bft) {
  free(bft);
}



GWEN_ERRORCODE GWEN_BufferedIO_File__Read(GWEN_BUFFEREDIO *dm,
                                          char *buffer,
                                          int *size,
                                          int timeout){
  int rv;
  GWEN_BUFFEREDIO_FILE_TABLE *bft;

  assert(dm);
  bft=(GWEN_BUFFEREDIO_FILE_TABLE *)(dm->privateData);
  assert(bft);
  if (*size<1) {
    DBG_WARN(0, "Nothing to read");
    *size=0;
    return 0;
  }
  rv=read(bft->fd, buffer, *size);
  if (rv==0) {
    DBG_DEBUG(0, "EOF met");
    *size=0;
    return 0;
  }
  if (rv<0) {
    DBG_ERROR(0, "Could not read (%s)",
	      strerror(errno));
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_READ);
  }
  *size=rv;
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_File__Write(GWEN_BUFFEREDIO *dm,
                                           const char *buffer,
                                           int *size,
                                           int timeout){
  int rv;
  GWEN_BUFFEREDIO_FILE_TABLE *bft;

  assert(dm);
  bft=(GWEN_BUFFEREDIO_FILE_TABLE *)(dm->privateData);
  assert(bft);
  if (*size<1) {
    DBG_WARN(0, "Nothing to write");
    *size=0;
    return 0;
  }
  rv=write(bft->fd, buffer, *size);
  if (rv<1) {
    DBG_ERROR(0, "Could not write (%s)",
	      strerror(errno));
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_WRITE);
  }
  *size=rv;
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_File__Close(GWEN_BUFFEREDIO *dm){
  GWEN_BUFFEREDIO_FILE_TABLE *bft;

  assert(dm);
  bft=(GWEN_BUFFEREDIO_FILE_TABLE *)(dm->privateData);
  assert(bft);
  if (close(bft->fd)) {
    DBG_ERROR(0, "Could not close (%s)",
	      strerror(errno));
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_CLOSE);
  }
  return 0;
}



void GWEN_BufferedIO_File__free(void *p){
  if (p)
    GWEN_BufferedIO_File_Table__free((GWEN_BUFFEREDIO_FILE_TABLE *)p);
}



GWEN_BUFFEREDIO *GWEN_BufferedIO_File_new(int fd){
  GWEN_BUFFEREDIO *bt;
  GWEN_BUFFEREDIO_FILE_TABLE *bft;

  bt=GWEN_BufferedIO_new();
  bft=GWEN_BufferedIO_File_Table__new();
  bt->privateData=bft;
  bft->fd=fd;
  bt->readPtr=GWEN_BufferedIO_File__Read;
  bt->writePtr=GWEN_BufferedIO_File__Write;
  bt->closePtr=GWEN_BufferedIO_File__Close;
  bt->freePtr=GWEN_BufferedIO_File__free;
  bt->iotype=GWEN_BufferedIOTypeFile;
  bt->timeout=GWEN_BUFFEREDIO_FILE_TIMEOUT;
  return bt;
}




/*_________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                           Socket Module
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




struct GWEN_BUFFEREDIO_SOCKET_STRUCT {
  GWEN_SOCKET *sock;
};
typedef struct GWEN_BUFFEREDIO_SOCKET_STRUCT GWEN_BUFFEREDIO_SOCKET;


GWEN_BUFFEREDIO_SOCKET *GWEN_BufferedIO_Socket_Table__new() {
  GWEN_BUFFEREDIO_SOCKET *bft;

  GWEN_NEW_OBJECT(GWEN_BUFFEREDIO_SOCKET, bft);

  return bft;
}



void GWEN_BufferedIO_Socket_Table__free(GWEN_BUFFEREDIO_SOCKET *bft) {
  if (bft) {
    GWEN_Socket_free(bft->sock);
    free(bft);
  }
}



GWEN_ERRORCODE GWEN_BufferedIO_Socket__Read(GWEN_BUFFEREDIO *dm,
                                            char *buffer,
                                            int *size,
                                            int timeout){
  GWEN_ERRORCODE err;
  GWEN_BUFFEREDIO_SOCKET *bft;
  int retrycount;

  assert(dm);
  assert(buffer);
  assert(size);
  bft=(GWEN_BUFFEREDIO_SOCKET *)(dm->privateData);
  assert(bft);
  assert(bft->sock);
  if (*size<1) {
    DBG_WARN(0, "Nothing to read");
    *size=0;
    return 0;
  }

  if (timeout>=0) {
    retrycount=GWEN_BUFFEREDIO_SOCKET_TRIES;
    while(retrycount) {
      err=GWEN_Socket_WaitForRead(bft->sock, timeout);
      if (!GWEN_Error_IsOk(err)) {
        if (GWEN_Error_GetType(err)==
            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE)) {
          if (GWEN_Error_GetCode(err)==GWEN_SOCKET_ERROR_TIMEOUT)
            return
              GWEN_Error_new(0,
                             GWEN_ERROR_SEVERITY_ERR,
                             GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                             GWEN_BUFFEREDIO_ERROR_TIMEOUT);
          else if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
            DBG_ERROR_ERR(0, err);
            return err;
          }
	} /* if socket error */
	else {
	  DBG_ERROR_ERR(0, err);
          return err;
	}
      }
      else
	break;
      retrycount--;
    } /* while */
    if (retrycount<1) {
      DBG_ERROR(0, "Interrupted too often, giving up");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                            GWEN_BUFFEREDIO_ERROR_READ);
    }
  } /* if timeout */

  /* ok. socket seems to be ready now */
  retrycount=GWEN_BUFFEREDIO_SOCKET_TRIES;
  while(retrycount) {
    err=GWEN_Socket_Read(bft->sock, buffer, size);
    if (!GWEN_Error_IsOk(err)) {
      if (GWEN_Error_GetType(err)==
          GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE)) {
	if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
	  DBG_ERROR_ERR(0, err);
	  return err;
	}
      } /* if socket error */
      else {
	DBG_ERROR_ERR(0, err);
	return err;
      }
    }
    else
      break;
    retrycount--;
  } /* while */
  if (retrycount<1) {
    DBG_ERROR(0, "Interrupted too often, giving up");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_READ);
  }

  DBG_VERBOUS(0, "Reading ok (%d bytes)", *size);
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_Socket__Write(GWEN_BUFFEREDIO *dm,
                                             const char *buffer,
                                             int *size,
                                             int timeout){
  GWEN_ERRORCODE err;
  GWEN_BUFFEREDIO_SOCKET *bft;
  int retrycount;

  assert(dm);
  assert(buffer);
  assert(size);
  bft=(GWEN_BUFFEREDIO_SOCKET *)(dm->privateData);
  assert(bft);
  assert(bft->sock);
  if (*size<1) {
    DBG_WARN(0, "Nothing to write");
    *size=0;
    return 0;
  }

  if (timeout>=0) {
    retrycount=GWEN_BUFFEREDIO_SOCKET_TRIES;
    while(retrycount) {
      err=GWEN_Socket_WaitForWrite(bft->sock, timeout);
      if (!GWEN_Error_IsOk(err)) {
        if (GWEN_Error_GetType(err)==
            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE)) {
	  if (GWEN_Error_GetCode(err)==GWEN_SOCKET_ERROR_TIMEOUT)
            return GWEN_Error_new(0,
                                  GWEN_ERROR_SEVERITY_ERR,
                                  GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                                  GWEN_BUFFEREDIO_ERROR_TIMEOUT);
	  else if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
	    DBG_ERROR_ERR(0, err);
	    return err;
	  }
	} /* if socket error */
	else {
	  DBG_ERROR_ERR(0, err);
	  return err;
	}
      }
      else
	break;
      retrycount--;
    } /* while */
    if (retrycount<1) {
      DBG_ERROR(0, "Interrupted too often, giving up");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                            GWEN_BUFFEREDIO_ERROR_WRITE);
    }
  } /* if timeout */

  /* ok. socket seems to be ready now */
  retrycount=GWEN_BUFFEREDIO_SOCKET_TRIES;
  while(retrycount) {
    err=GWEN_Socket_Write(bft->sock, buffer, size);
    if (!GWEN_Error_IsOk(err)) {
      if (GWEN_Error_GetType(err)==
          GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE)) {
	if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
	  DBG_ERROR_ERR(0, err);
	  return err;
	}
      } /* if socket error */
      else {
	DBG_ERROR_ERR(0, err);
	return err;
      }
    }
    else
      break;

    retrycount--;
  } /* while */
  if (retrycount<1) {
    DBG_ERROR(0, "Interrupted too often, giving up");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_WRITE);
  }

  DBG_VERBOUS(0, "Writing ok");
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_Socket__Close(GWEN_BUFFEREDIO *dm){
  GWEN_BUFFEREDIO_SOCKET *bft;

  assert(dm);
  bft=(GWEN_BUFFEREDIO_SOCKET *)(dm->privateData);
  assert(bft);
  assert(bft->sock);
  DBG_DEBUG(0, "Closing socket");
  if (GWEN_Socket_Close(bft->sock)) {
    DBG_ERROR(0, "Could not close (%s)",
	      strerror(errno));
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_CLOSE);
  }
  return 0;
}



void GWEN_BufferedIO_Socket__free(void *p){
  if (p)
    GWEN_BufferedIO_Socket_Table__free((GWEN_BUFFEREDIO_SOCKET *)p);
}



GWEN_BUFFEREDIO *GWEN_BufferedIO_Socket_new(GWEN_SOCKET *sock){
  GWEN_BUFFEREDIO *bt;
  GWEN_BUFFEREDIO_SOCKET *bft;

  bt=GWEN_BufferedIO_new();
  bft=GWEN_BufferedIO_Socket_Table__new();
  bt->privateData=bft;
  bft->sock=sock;
  bt->readPtr=GWEN_BufferedIO_Socket__Read;
  bt->writePtr=GWEN_BufferedIO_Socket__Write;
  bt->closePtr=GWEN_BufferedIO_Socket__Close;
  bt->freePtr=GWEN_BufferedIO_Socket__free;
  bt->iotype=GWEN_BufferedIOTypeSocket;
  bt->timeout=GWEN_BUFFEREDIO_SOCKET_TIMEOUT;
  return bt;
}







/*_________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                           Buffer Module
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




struct GWEN_BUFFEREDIO_BUFFER_STRUCT {
  GWEN_BUFFER *buffer;
  int closed;
};
typedef struct GWEN_BUFFEREDIO_BUFFER_STRUCT GWEN_BUFFEREDIO_BUFFER;


GWEN_BUFFEREDIO_BUFFER *GWEN_BufferedIO_Buffer_Table__new() {
  GWEN_BUFFEREDIO_BUFFER *bft;

  GWEN_NEW_OBJECT(GWEN_BUFFEREDIO_BUFFER, bft);
  bft->closed=0;
  return bft;
}



void GWEN_BufferedIO_Buffer_Table__free(GWEN_BUFFEREDIO_BUFFER *bft) {
  if (bft) {
    GWEN_Buffer_free(bft->buffer);
    free(bft);
  }
}



GWEN_ERRORCODE GWEN_BufferedIO_Buffer__Read(GWEN_BUFFEREDIO *dm,
                                            char *buffer,
                                            int *size,
                                            int timeout){
  GWEN_BUFFEREDIO_BUFFER *bft;
  unsigned int readSize;

  assert(dm);
  assert(buffer);
  assert(size);
  bft=(GWEN_BUFFEREDIO_BUFFER *)(dm->privateData);
  assert(bft);
  assert(bft->buffer);

  if (bft->closed) {
    DBG_INFO(0, "Channel closed");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_READ);
  }

  if (*size<1) {
    DBG_WARN(0, "Nothing to read");
    *size=0;
    return 0;
  }

  readSize=*size;
  if (GWEN_Buffer_ReadBytes(bft->buffer, buffer, &readSize)) {
    DBG_ERROR(0, "Error reading from buffer");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_READ);
  }
  *size=readSize;
  DBG_VERBOUS(0, "Reading ok (%d bytes)", *size);
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_Buffer__Write(GWEN_BUFFEREDIO *dm,
                                             const char *buffer,
                                             int *size,
                                             int timeout){
  GWEN_BUFFEREDIO_BUFFER *bft;

  assert(dm);
  assert(buffer);
  assert(size);
  bft=(GWEN_BUFFEREDIO_BUFFER *)(dm->privateData);
  assert(bft);
  assert(bft->buffer);

  if (bft->closed) {
    DBG_INFO(0, "Channel closed");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_WRITE);
  }

  if (*size<1) {
    DBG_WARN(0, "Nothing to write");
    *size=0;
    return 0;
  }

  if (GWEN_Buffer_AppendBytes(bft->buffer, buffer, *size)) {
    DBG_INFO(0, "called from here");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_WRITE);
  }

  DBG_VERBOUS(0, "Writing ok");
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_Buffer__Close(GWEN_BUFFEREDIO *dm){
  GWEN_BUFFEREDIO_BUFFER *bft;

  assert(dm);
  bft=(GWEN_BUFFEREDIO_BUFFER *)(dm->privateData);
  assert(bft);
  assert(bft->buffer);
  DBG_DEBUG(0, "Closing socket");

  if (bft->closed) {
    DBG_DEBUG(0, "Channel already closed");
  }
  else
    bft->closed=1;
  return 0;
}



void GWEN_BufferedIO_Buffer__free(void *p){
  if (p)
    GWEN_BufferedIO_Buffer_Table__free((GWEN_BUFFEREDIO_BUFFER *)p);
}




GWEN_BUFFEREDIO *GWEN_BufferedIO_Buffer_new(GWEN_BUFFER *buffer){
  GWEN_BUFFEREDIO *bt;
  GWEN_BUFFEREDIO_BUFFER *bft;

  assert(buffer);
  bt=GWEN_BufferedIO_new();
  bft=GWEN_BufferedIO_Buffer_Table__new();
  bt->privateData=bft;
  bft->buffer=buffer;
  bt->readPtr=GWEN_BufferedIO_Buffer__Read;
  bt->writePtr=GWEN_BufferedIO_Buffer__Write;
  bt->closePtr=GWEN_BufferedIO_Buffer__Close;
  bt->freePtr=GWEN_BufferedIO_Buffer__free;
  bt->iotype=GWEN_BufferedIOTypeBuffer;
  bt->timeout=0;
  return bt;
}








