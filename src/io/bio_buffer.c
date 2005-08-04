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


#include "bio_buffer_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <gwenhywfar/debug.h>


GWEN_INHERIT(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_BUFFER)


GWEN_BUFFEREDIO_BUFFER *GWEN_BufferedIO_Buffer_Table__new() {
  GWEN_BUFFEREDIO_BUFFER *bft;

  GWEN_NEW_OBJECT(GWEN_BUFFEREDIO_BUFFER, bft);
  bft->closed=0;
  return bft;
}



void GWEN_BufferedIO_Buffer_Table__free(GWEN_BUFFEREDIO_BUFFER *bft) {
  if (bft) {
    if (bft->own)
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
  bft=GWEN_INHERIT_GETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_BUFFER, dm);
  assert(bft);
  assert(bft->buffer);

  if (bft->closed) {
    DBG_INFO(GWEN_LOGDOMAIN, "Channel closed");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_READ);
  }

  if (*size<1) {
    DBG_WARN(GWEN_LOGDOMAIN, "Nothing to read");
    *size=0;
    return 0;
  }

  readSize=*size;
  if (GWEN_Buffer_ReadBytes(bft->buffer, buffer, &readSize)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error reading from buffer");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_READ);
  }
  *size=readSize;
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Reading ok (%d bytes)", *size);
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
  bft=GWEN_INHERIT_GETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_BUFFER, dm);
  assert(bft);
  assert(bft->buffer);

  if (bft->closed) {
    DBG_INFO(GWEN_LOGDOMAIN, "Channel closed");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_WRITE);
  }

  if (*size<1) {
    DBG_WARN(GWEN_LOGDOMAIN, "Nothing to write");
    *size=0;
    return 0;
  }

  if (GWEN_Buffer_AppendBytes(bft->buffer, buffer, *size)) {
    DBG_INFO(GWEN_LOGDOMAIN, "called from here");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_WRITE);
  }

  DBG_VERBOUS(GWEN_LOGDOMAIN, "Writing ok");
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_Buffer__Close(GWEN_BUFFEREDIO *dm){
  GWEN_BUFFEREDIO_BUFFER *bft;

  assert(dm);
  bft=GWEN_INHERIT_GETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_BUFFER, dm);
  assert(bft);
  assert(bft->buffer);
  DBG_DEBUG(GWEN_LOGDOMAIN, "Closing socket");

  if (bft->closed) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Channel already closed");
  }
  else
    bft->closed=1;
  return 0;
}



void GWEN_BufferedIO_Buffer_FreeData(void *bp, void *p) {
  GWEN_BUFFEREDIO_BUFFER *bft;

  bft=(GWEN_BUFFEREDIO_BUFFER*)p;
  GWEN_BufferedIO_Buffer_Table__free(bft);
}



GWEN_BUFFEREDIO *GWEN_BufferedIO_Buffer2_new(GWEN_BUFFER *buffer, int take){
  GWEN_BUFFEREDIO *bt;
  GWEN_BUFFEREDIO_BUFFER *bft;

  assert(buffer);
  bt=GWEN_BufferedIO_new();
  bft=GWEN_BufferedIO_Buffer_Table__new();
  bft->buffer=buffer;
  bft->own=take;

  GWEN_INHERIT_SETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_BUFFER,
                       bt, bft,
                       GWEN_BufferedIO_Buffer_FreeData);
  GWEN_BufferedIO_SetReadFn(bt, GWEN_BufferedIO_Buffer__Read);
  GWEN_BufferedIO_SetWriteFn(bt, GWEN_BufferedIO_Buffer__Write);
  GWEN_BufferedIO_SetCloseFn(bt, GWEN_BufferedIO_Buffer__Close);
  GWEN_BufferedIO_SetTimeout(bt, 0);

  return bt;
}



GWEN_BUFFEREDIO *GWEN_BufferedIO_Buffer_new(GWEN_BUFFER *buffer){
  return GWEN_BufferedIO_Buffer2_new(buffer, 1);
}



GWEN_BUFFEREDIO *GWEN_BufferedIO_Buffer_fromString(const char *s,
                                                   int len){
  GWEN_BUFFER *dbuf;

  if (len==0)
    len=strlen(s);
  dbuf=GWEN_Buffer_new(0, len, 0, 1);
  GWEN_Buffer_AppendBytes(dbuf, s, len);
  GWEN_Buffer_Rewind(dbuf);
  return GWEN_BufferedIO_Buffer2_new(dbuf, 1);
}




