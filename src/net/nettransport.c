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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "nettransport_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>





GWEN_NETTRANSPORT *GWEN_NetTransport_new(const GWEN_INETADDRESS *addr){
  GWEN_NETTRANSPORT *tr;

  GWEN_NEW_OBJECT(GWEN_NETTRANSPORT, tr);
  tr->status=GWEN_NetTransportStatusUnconnected;
  return tr;
}



void GWEN_NetTransport_free(GWEN_NETTRANSPORT *tr){
  if (tr) {
    if (tr->privateData && tr->freeDataFn)
      tr->freeDataFn(tr);
    free(tr);
  }
}



GWEN_NETTRANSPORT_RESULT
GWEN_NetTransport_StartConnect(GWEN_NETTRANSPORT *tr){
  assert(tr);
  assert(tr->startConnectFn);
  return tr->startConnectFn(tr);
}



GWEN_NETTRANSPORT_RESULT
GWEN_NetTransport_StartAccept(GWEN_NETTRANSPORT *tr){
  assert(tr);
  assert(tr->startAcceptFn);
  return tr->startAcceptFn(tr);
}



GWEN_NETTRANSPORT_RESULT
GWEN_NetTransport_StartDisconnect(GWEN_NETTRANSPORT *tr){
  assert(tr);
  assert(tr->startDisconnectFn);
  return tr->startDisconnectFn(tr);
}



GWEN_NETTRANSPORT_RESULT
GWEN_NetTransport_Read(GWEN_NETTRANSPORT *tr,
		       char *buffer,
		       int *bsize){
  assert(tr);
  assert(tr->readFn);
  return tr->readFn(tr, buffer, bsize);
}



GWEN_NETTRANSPORT_RESULT
GWEN_NetTransport_Write(GWEN_NETTRANSPORT *tr,
			const char *buffer,
			int *bsize){
  assert(tr);
  assert(tr->writeFn);
  return tr->writeFn(tr, buffer, bsize);
}



int GWEN_NetTransport_AddSockets(GWEN_NETTRANSPORT *tr,
				 GWEN_SOCKETSET *sset,
				 int forReading){
  assert(tr);
  assert(tr->addSocketsFn);
  return tr->addSocketsFn(tr, sset, forReading);
}










