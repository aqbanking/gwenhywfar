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


#include "bio_file_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <gwenhywfar/debug.h>


GWEN_INHERIT(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_FILE)
/* No trailing semicolon here because this is a macro call */



GWEN_BUFFEREDIO_FILE *GWEN_BufferedIO_File_Table__new() {
  GWEN_BUFFEREDIO_FILE *bft;

  GWEN_NEW_OBJECT(GWEN_BUFFEREDIO_FILE, bft);
  bft->fd=-1;
  return bft;
}



void GWEN_BufferedIO_File_Table__free(GWEN_BUFFEREDIO_FILE *bft) {
  GWEN_FREE_OBJECT(bft);
}



GWEN_ERRORCODE GWEN_BufferedIO_File__Read(GWEN_BUFFEREDIO *dm,
                                          char *buffer,
                                          int *size,
                                          int timeout){
  int rv;
  GWEN_BUFFEREDIO_FILE *bft;

  assert(dm);
  bft=GWEN_INHERIT_GETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_FILE, dm);
  assert(bft);

  if (*size<1) {
    DBG_WARN(GWEN_LOGDOMAIN, "Nothing to read");
    *size=0;
    return 0;
  }
  rv=read(bft->fd, buffer, *size);
  if (rv==0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "EOF met");
    *size=0;
    return 0;
  }
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not read (%s)",
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
  GWEN_BUFFEREDIO_FILE *bft;

  assert(dm);
  bft=GWEN_INHERIT_GETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_FILE, dm);
  assert(bft);

  if (*size<1) {
    DBG_WARN(GWEN_LOGDOMAIN, "Nothing to write");
    *size=0;
    return 0;
  }

  rv=write(bft->fd, buffer, *size);
  if (rv<1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not write (%s)",
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
  GWEN_BUFFEREDIO_FILE *bft;

  assert(dm);
  bft=GWEN_INHERIT_GETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_FILE, dm);
  assert(bft);
  if (bft->fd<3) {
    /* one of the console IO sockets, do not close */
    return 0;
  }
  if (close(bft->fd)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not close (%s)",
	      strerror(errno));
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_CLOSE);
  }
  return 0;
}



void GWEN_BufferedIO_File_FreeData(void *bp, void *p) {
  GWEN_BUFFEREDIO_FILE *bft;

  bft=(GWEN_BUFFEREDIO_FILE*)p;
  GWEN_BufferedIO_File_Table__free(bft);
}



GWEN_BUFFEREDIO *GWEN_BufferedIO_File_new(int fd){
  GWEN_BUFFEREDIO *bt;
  GWEN_BUFFEREDIO_FILE *bft;

  bt=GWEN_BufferedIO_new();
  bft=GWEN_BufferedIO_File_Table__new();
  bft->fd=fd;

  GWEN_INHERIT_SETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_FILE,
                       bt, bft,
                       GWEN_BufferedIO_File_FreeData);
  GWEN_BufferedIO_SetReadFn(bt, GWEN_BufferedIO_File__Read);
  GWEN_BufferedIO_SetWriteFn(bt, GWEN_BufferedIO_File__Write);
  GWEN_BufferedIO_SetCloseFn(bt, GWEN_BufferedIO_File__Close);
  GWEN_BufferedIO_SetTimeout(bt, GWEN_BUFFEREDIO_FILE_TIMEOUT);

  return bt;
}





