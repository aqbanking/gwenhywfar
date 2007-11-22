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


#ifndef GWEN_IOLAYER_SOCKET_P_H
#define GWEN_IOLAYER_SOCKET_P_H

#include <gwenhywfar/io_socket.h>


typedef struct GWEN_IO_LAYER_SOCKET GWEN_IO_LAYER_SOCKET;
struct GWEN_IO_LAYER_SOCKET {
  GWEN_SOCKET *socket;
  GWEN_IO_REQUEST *readRequest;
  GWEN_IO_REQUEST *writeRequest;
  GWEN_IO_REQUEST *connectRequest;

  GWEN_INETADDRESS *localAddr;
  GWEN_INETADDRESS *peerAddr;

  int listening;
};

static GWENHYWFAR_CB void GWEN_Io_LayerSocket_freeData(void *bp, void *p);
static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerSocket_WorkOnRequests(GWEN_IO_LAYER *io);
static int GWEN_Io_LayerSocket_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);
static int GWEN_Io_LayerSocket_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);
static int GWEN_Io_LayerSocket_HasWaitingRequests(GWEN_IO_LAYER *io);

static void GWEN_Io_LayerSocket_AbortRequests(GWEN_IO_LAYER *io, int errorCode);

static int GWEN_Io_LayerSocket_AddWaitingSockets(GWEN_IO_LAYER *io,
						 GWEN_SOCKET_LIST2 *readSockets,
						 GWEN_SOCKET_LIST2 *writeSockets);

static int GWEN_Io_LayerSocket_Listen(GWEN_IO_LAYER *io);

static GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerSocket_CheckForIncoming(GWEN_IO_LAYER *io);



#endif




