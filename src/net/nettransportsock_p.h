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


#ifndef GWEN_NETTRANSPORTSOCK_P_H
#define GWEN_NETTRANSPORTSOCK_P_H


#include <gwenhywfar/nettransportsock.h>


typedef struct GWEN_NETTRANSPORTSOCKET {
  GWEN_SOCKET *socket;
  int ownSocket;
} GWEN_NETTRANSPORTSOCKET;


GWEN_NETTRANSPORTSOCKET *GWEN_NetTransportSocketData_new();

void GWEN_NetTransportSocket_FreeData(void *bp, void *p);


GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransportSocket_StartConnect(GWEN_NETTRANSPORT *tr);

GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransportSocket_StartAccept(GWEN_NETTRANSPORT *tr);

GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransportSocket_StartDisconnect(GWEN_NETTRANSPORT *tr);



GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransportSocket_Read(GWEN_NETTRANSPORT *tr,
                               char *buffer,
                               int *bsize);

GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransportSocket_Write(GWEN_NETTRANSPORT *tr,
                                const char *buffer,
                                int *bsize);



int GWEN_NetTransportSocket_AddSockets(GWEN_NETTRANSPORT *tr,
                                       GWEN_SOCKETSET *sset,
                                       int forReading);

GWEN_NETTRANSPORT_WORKRESULT
  GWEN_NetTransportSocket_Work(GWEN_NETTRANSPORT *tr);



#endif /* GWEN_NETTRANSPORTSOCK_P_H */

