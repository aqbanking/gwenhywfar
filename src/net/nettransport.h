/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Jan 24 2004
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

#ifndef GWEN_NETTRANSPORT_H
#define GWEN_NETTRANSPORT_H

#include <gwenhywfar/inetaddr.h>
#include <gwenhywfar/inetsocket.h>


typedef struct GWEN_NETTRANSPORT GWEN_NETTRANSPORT;


typedef enum {
  GWEN_NetTransportResultOk=0,
  GWEN_NetTransportResultError,
  GWEN_NetTransportResultWantRead,
  GWEN_NetTransportResultWantWrite,
  GWEN_NetTransportResultAborted
} GWEN_NETTRANSPORT_RESULT;


typedef enum {
  GWEN_NetTransportStatusUnconnected=0,
  GWEN_NetTransportStatusPConnecting,
  GWEN_NetTransportStatusPConnected,
  GWEN_NetTransportStatusLConnecting,
  GWEN_NetTransportStatusLConnected,
  GWEN_NetTransportStatusLDisconnecting,
  GWEN_NetTransportStatusLDisconnected,
  GWEN_NetTransportStatusPDisconnecting,
  GWEN_NetTransportStatusPDisconnected,
  GWEN_NetTransportStatusDisabled
} GWEN_NETTRANSPORT_STATUS;


typedef GWEN_NETTRANSPORT_RESULT
(*GWEN_NETTRANSPORT_STARTCONNECT)(GWEN_NETTRANSPORT *tr);


typedef GWEN_NETTRANSPORT_RESULT
  (*GWEN_NETTRANSPORT_STARTACCEPT)(GWEN_NETTRANSPORT *tr);

typedef GWEN_NETTRANSPORT_RESULT
  (*GWEN_NETTRANSPORT_STARTDISCONNECT)(GWEN_NETTRANSPORT *tr);

typedef GWEN_NETTRANSPORT_RESULT
  (*GWEN_NETTRANSPORT_READ)(GWEN_NETTRANSPORT *tr,
                            char *buffer,
                            int *bsize);

typedef GWEN_NETTRANSPORT_RESULT
  (*GWEN_NETTRANSPORT_WRITE)(GWEN_NETTRANSPORT *tr,
                             const char *buffer,
                             int *bsize);

typedef int
  (*GWEN_NETTRANSPORT_ADDSOCKETS)(GWEN_NETTRANSPORT *tr,
                                  GWEN_SOCKETSET *sset,
                                  int forReading);

typedef int
  (*GWEN_NETTRANSPORT_CHECKSOCKETS)(GWEN_NETTRANSPORT *tr,
                                    GWEN_SOCKETSET *sset,
                                    int forReading);

typedef void
  (*GWEN_NETTRANSPORT_FREEDATA)(GWEN_NETTRANSPORT *tr);





GWEN_NETTRANSPORT *GWEN_NetTransport_new(const GWEN_INETADDRESS *addr);
void GWEN_NetTransport_free(GWEN_NETTRANSPORT *tr);


GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransport_StartConnect(GWEN_NETTRANSPORT *tr);

GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransport_StartAccept(GWEN_NETTRANSPORT *tr);

GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransport_StartDisconnect(GWEN_NETTRANSPORT *tr);



GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransport_Read(GWEN_NETTRANSPORT *tr,
                         char *buffer,
                         int *bsize);

GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransport_Write(GWEN_NETTRANSPORT *tr,
                          const char *buffer,
                          int *bsize);



int GWEN_NetTransport_AddSockets(GWEN_NETTRANSPORT *tr,
                                 GWEN_SOCKETSET *sset,
                                 int forReading);

int GWEN_NetTransport_CheckSockets(GWEN_NETTRANSPORT *tr,
                                   GWEN_SOCKETSET *sset,
                                   int forReading);






#endif /* GWEN_NETTRANSPORT_H */

