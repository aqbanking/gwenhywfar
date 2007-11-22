/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_PACKETS_P_H
#define GWEN_PACKETS_P_H

#include <gwenhywfar/io_packets.h>
#include <gwenhywfar/iorequest_be.h>



typedef struct GWEN_IO_LAYER_PACKETS GWEN_IO_LAYER_PACKETS;
struct GWEN_IO_LAYER_PACKETS {
  GWEN_IO_REQUEST *currentReadRequest;
  GWEN_IO_REQUEST *currentWriteRequest;

  GWEN_IO_REQUEST_LIST *requestsIn;
  GWEN_IO_REQUEST_LIST *requestsOut;

  int readSize;

  int maxReadQueue;
  int maxWriteQueue;
};


void GWENHYWFAR_CB GWEN_Io_LayerPackets_freeData(void *bp, void *p);
void GWEN_Io_LayerPackets_Abort(GWEN_IO_LAYER *io);

GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerPackets_WorkOnReadRequests(GWEN_IO_LAYER *io);
GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerPackets_WorkOnWriteRequests(GWEN_IO_LAYER *io);


GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerPackets_WorkOnRequests(GWEN_IO_LAYER *io);
int GWEN_Io_LayerPackets_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);
int GWEN_Io_LayerPackets_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);

int GWEN_Io_LayerPackets_HasWaitingRequests(GWEN_IO_LAYER *io);


#endif


