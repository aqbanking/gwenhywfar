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


#ifndef GWENHYFWAR_BUFFEREDIO_P_H
#define GWENHYFWAR_BUFFEREDIO_P_H "$Id$"

#include <gwenhyfwar/gwenhyfwarapi.h>
#include <gwenhyfwar/bufferedio.h>
#include <gwenhyfwar/error.h>
#include <gwenhyfwar/inetsocket.h>


#define GWEN_BUFFEREDIO_CR 13
#define GWEN_BUFFEREDIO_LF 10

#define GWEN_BUFFEREDIO_FILE_TIMEOUT   20000
#define GWEN_BUFFEREDIO_SOCKET_TIMEOUT 20000
#define GWEN_BUFFEREDIO_SOCKET_TRIES   3


GWENHYFWAR_API
  typedef GWEN_ERRORCODE (*GWEN_BUFFEREDIOREADPTR)(GWEN_BUFFEREDIO *dm,
                                                   char *buffer,
                                                   int *size,
                                                   int timeout);
GWENHYFWAR_API
  typedef GWEN_ERRORCODE (*GWEN_BUFFEREDIOWRITEPTR)(GWEN_BUFFEREDIO *dm,
                                                    const char *buffer,
                                                    int *size,
                                                    int timeout);
GWENHYFWAR_API
  typedef GWEN_ERRORCODE (*GWEN_BUFFEREDIOCLOSEPTR)(GWEN_BUFFEREDIO *dm);
GWENHYFWAR_API typedef void (*GWEN_BUFFEREDIOFREEPTR)(void *p);



struct GWEN_BUFFEREDIOSTRUCT {
  GWEN_BUFFEREDIOREADPTR readPtr;
  GWEN_BUFFEREDIOWRITEPTR writePtr;
  GWEN_BUFFEREDIOCLOSEPTR closePtr;
  GWEN_BUFFEREDIOFREEPTR freePtr;
  void *privateData;

  GWEN_BUFFEREDIOTYPE iotype;
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
};


GWENHYFWAR_API GWEN_ERRORCODE GWEN_BufferedIO_ModuleInit();
GWENHYFWAR_API GWEN_ERRORCODE GWEN_BufferedIO_ModuleFini();


#endif /* GWENHYFWAR_BUFFEREDIO_P_H */





