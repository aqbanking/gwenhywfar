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


#ifndef GWENHYWFAR_BUFFEREDIO_P_H
#define GWENHYWFAR_BUFFEREDIO_P_H "$Id$"

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/error.h>
#include <gwenhywfar/inetsocket.h>
#include "bufferedio_l.h"


#define GWEN_BUFFEREDIO_CR 13
#define GWEN_BUFFEREDIO_LF 10


struct GWEN_BUFFEREDIOSTRUCT {
  GWEN_INHERIT_ELEMENT(GWEN_BUFFEREDIO)
  /* No trailing semicolon here because this is a macro call */
  GWEN_BUFFEREDIOREADFN readPtr;
  GWEN_BUFFEREDIOWRITEFN writePtr;
  GWEN_BUFFEREDIOCLOSEFN closePtr;
  GWEN_TYPE_UINT32 flags;

  GWEN_BUFFEREDIOLINEMODE lineMode;
  int timeout;
  char *readerBuffer;
  int readerBufferLength;
  int readerBufferFilled;
  int readerBufferPos;
  int readerEOF;
  int readerError;

  char *writerBuffer;
  int writerBufferLength;
  int writerBufferFilled;
  int writerBufferPos;
  int writerBufferFlushPos;

  int lines;
};


GWENHYWFAR_API GWEN_ERRORCODE GWEN_BufferedIO_ModuleInit();
GWENHYWFAR_API GWEN_ERRORCODE GWEN_BufferedIO_ModuleFini();


#endif /* GWENHYWFAR_BUFFEREDIO_P_H */





