/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Sep 14 2003
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


#include "transportlayersocket_p.h"
#include <gwenhyfwar/inetsocket.h>
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
  GWEN_IPCTransportLayerSocket_StartConnect(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_ERRORCODE err;
  GWEN_INETADDRESS *addr;
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;

  if (tlp->socket==0) {
    tlp->socket=GWEN_Socket_new();
    err=GWEN_Socket_Open(tlp->socket, tlp->socketType);
    if (!GWEN_Error_IsOk(err))
      return err;
  }

  /* temporarily make socket non-blocking */
  err=GWEN_Socket_SetBlocking(tlp->socket,0);
  if (!GWEN_Error_IsOk(err))
    return err;

  /* set address and port of peer */
  assert(tl->address);
  addr=GWEN_InetAddr_new(tlp->addressFamily);
  err=GWEN_InetAddr_SetAddress(addr, tl->address);
  if (!GWEN_Error_IsOk(err))
    err=GWEN_InetAddr_SetName(addr,tl->address);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_InetAddr_free(addr);
    return err;
  }
  err=GWEN_InetAddr_SetPort(addr, tl->port);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_InetAddr_free(addr);
    return err;
  }

  /* actually start to connect */
  err=GWEN_Socket_Connect(tlp->socket,addr);
  GWEN_InetAddr_free(addr);
  /* not yet finished or real error ? */
  if (GWEN_Error_GetType(err)!=GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
      GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_IN_PROGRESS) {
    /* real error, so return that error */
    return err;
  }

  /* connect is in progress now */
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayerSocket_FinishConnect(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_ERRORCODE err;
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;

  /* get socket error */
  err=GWEN_Socket_GetSocketError(tlp->socket);
  if (!GWEN_Error_IsOk(err))
    return err;

  /* make socket blocking again */
  err=GWEN_Socket_SetBlocking(tlp->socket,1);
  if (!GWEN_Error_IsOk(err))
    return err;

  /* create buffered io */
  tlp->bio=GWEN_BufferedIO_Socket_new(tlp->socket);
  GWEN_BufferedIO_SetTimeout(tlp->bio, 0);
  GWEN_BufferedIO_SetReadBuffer(tlp->bio, 0,
                                GWEN_TRANSPORTLAYERSOCKET_BUFFERSIZE);
  GWEN_BufferedIO_SetWriteBuffer(tlp->bio, 0,
                                 GWEN_TRANSPORTLAYERSOCKET_BUFFERSIZE);

  DBG_INFO(0, "Socket is connected");
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCTransportLayerSocket_Listen(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_ERRORCODE err;
  GWEN_INETADDRESS *addr;
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;

  if (tlp->socket==0) {
    tlp->socket=GWEN_Socket_new();
    err=GWEN_Socket_Open(tlp->socket, tlp->socketType);
    if (!GWEN_Error_IsOk(err))
      return err;
  }

  /* create inet address */
  assert(tl->address);
  addr=GWEN_InetAddr_new(tlp->addressFamily);
  err=GWEN_InetAddr_SetAddress(addr, tl->address);
  if (!GWEN_Error_IsOk(err))
    err=GWEN_InetAddr_SetName(addr,tl->address);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_InetAddr_free(addr);
    return err;
  }
  err=GWEN_InetAddr_SetPort(addr, tl->port);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_InetAddr_free(addr);
    return err;
  }

  err=GWEN_Socket_SetReuseAddress(tlp->socket, 1);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_InetAddr_free(addr);
    return err;
  }

  err=GWEN_Socket_Bind(tlp->socket, addr);
  GWEN_InetAddr_free(addr);
  if (!GWEN_Error_IsOk(err))
    return err;

  err=GWEN_Socket_Listen(tlp->socket, 10);
  if (!GWEN_Error_IsOk(err))
    return err;
  tlp->listening=1;

  /* log message */
  switch(GWEN_Socket_GetSocketType(tlp->socket)) {
  case GWEN_SocketTypeTCP:
  case GWEN_SocketTypeUDP:
    DBG_INFO(0, "Socket is listening on %s:%d",
             GWEN_IPCTransportLayer_GetAddress(tl),
             GWEN_IPCTransportLayer_GetPort(tl));
    break;
  default:
    DBG_INFO(0, "Socket is listening on %s",
             GWEN_IPCTransportLayer_GetAddress(tl));
  } /* switch */

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayerSocket_Accept(GWEN_IPCTRANSPORTLAYER *tl,
                                    GWEN_IPCTRANSPORTLAYER **t){
  GWEN_SOCKET *s;
  GWEN_ERRORCODE err;
  GWEN_INETADDRESS *iaddr;
  GWEN_IPCTRANSSOCKET *tlp;
  GWEN_IPCTRANSSOCKET *tlp2;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;

  s=0;
  iaddr=0;

  err=GWEN_Socket_Accept(tlp->socket, &iaddr, &s);
  /* TODO: save peer address */
  GWEN_InetAddr_free(iaddr);
  if (!GWEN_Error_IsOk(err)) {
    GWEN_Socket_free(s);
    return err;
  }
  *t=GWEN_IPCTransportLayerSocket_new();
  tlp2=(GWEN_IPCTRANSSOCKET*)((*t)->privateData);
  /* create buffered io */
  tlp2->socket=s;
  tlp2->bio=GWEN_BufferedIO_Socket_new(s);
  GWEN_BufferedIO_SetReadBuffer(tlp2->bio, 0,
                                GWEN_TRANSPORTLAYERSOCKET_BUFFERSIZE);
  GWEN_BufferedIO_SetWriteBuffer(tlp2->bio, 0,
                                 GWEN_TRANSPORTLAYERSOCKET_BUFFERSIZE);
  GWEN_BufferedIO_SetTimeout(tlp2->bio, 0);
  DBG_INFO(0, "New connection accepted");
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayerSocket_Flush(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_ERRORCODE err;
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;

  if (tlp->bio) {
    err=GWEN_BufferedIO_ShortFlush(tlp->bio);
    if (!GWEN_Error_IsOk(err)) {
      DBG_DEBUG(0, "called from here");
    }
    else {
      DBG_INFO(0, "Socket buffer flushed");
    }
    return err;
  }
  /* no bio, so nothing to flush */
  DBG_INFO(0, "Nothing to flush");
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayerSocket_Disconnect(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_ERRORCODE err;
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;

  if (tlp->bio) {
    /* if a bio exists then the socket gets by deleted by it */
    DBG_INFO(0, "Freeing buffered IO");
    GWEN_BufferedIO_Abandon(tlp->bio);
    GWEN_BufferedIO_free(tlp->bio);
    tlp->socket=0;
    tlp->bio=0;
  }
  else {
    /* bio does not exist, so delete the socket myself */
    DBG_INFO(0, "No buffered IO, freeing socket");
    err=GWEN_Socket_Close(tlp->socket);
    GWEN_Socket_free(tlp->socket);
    tlp->socket=0;
    if (!GWEN_Error_IsOk(err))
      return err;
  }
  DBG_INFO(0, "Socket disconnected");
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_SOCKET*
GWEN_IPCTransportLayerSocket_GetReadSocket(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_ERRORCODE err;
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;

  if (tlp->socket==0) {
    tlp->socket=GWEN_Socket_new();
    err=GWEN_Socket_Open(tlp->socket, tlp->socketType);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return 0;
    }
  }
  DBG_DEBUG(0, "Returning read socket");
  return tlp->socket;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_SOCKET*
GWEN_IPCTransportLayerSocket_GetWriteSocket(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_ERRORCODE err;
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;

  if (tlp->socket==0) {
    tlp->socket=GWEN_Socket_new();
    err=GWEN_Socket_Open(tlp->socket, tlp->socketType);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return 0;
    }
  }
  DBG_DEBUG(0, "Returning write socket");
  return tlp->socket;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCTransportLayerSocket_Read(GWEN_IPCTRANSPORTLAYER *tl,
                                                 char *buffer,
                                                 unsigned int *bsize){
  GWEN_IPCTRANSSOCKET *tlp;
  GWEN_ERRORCODE err;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;
  assert(tlp->bio);
  if (*bsize==1) {
    /* use bio's buffer when reading single bytes to speed up */
    int i;

    DBG_DEBUG(0, "Reading single char");
    i=GWEN_BufferedIO_ReadChar(tlp->bio);
    if (i<0) {
      DBG_ERROR(0, "Could not read character (%d)", i);
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                            GWEN_BUFFEREDIO_ERROR_READ);
    }
    *buffer=(unsigned char)i;
    *bsize=1;
    return 0;
  }
  else {
    DBG_DEBUG(0, "Reading raw (%d bytes)", *bsize);
    err=GWEN_BufferedIO_ReadRaw(tlp->bio,
                                buffer,
                                bsize);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO(0, "called from here");
    }
    else {
      DBG_DEBUG(0, "Read %d bytes", *bsize);
    }
    return err;
  }
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCTransportLayerSocket_Write(GWEN_IPCTRANSPORTLAYER *tl,
                                                  const char *buffer,
                                                  unsigned int *bsize){
  GWEN_IPCTRANSSOCKET *tlp;
  GWEN_ERRORCODE err;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;
  assert(tlp->bio);
  err=GWEN_BufferedIO_WriteRaw(tlp->bio,
                               buffer,
                               bsize);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
  }
  else {
    DBG_INFO(0, "Written %d bytes", *bsize);
  }
  return err;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCTransportLayerSocket_CanRead(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;
  assert(tlp->bio);
  if (!GWEN_BufferedIO_ReadBufferEmpty(tlp->bio)) {
    /* reader buffer is not empty, so we can read */
    DBG_DEBUG(0, "ReadBuffer is not empty, so reading is possible");
    return 0;
  }
  assert(tlp->socket);
  /* reader buffer is empty, so check the socket directly */
  return GWEN_Socket_WaitForRead(tlp->socket,0);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_IPCTransportLayerSocket_CanWrite(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;
  assert(tlp->socket);

  return GWEN_Socket_WaitForWrite(tlp->socket,0);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayerSocket_GetPeerAddress(GWEN_IPCTRANSPORTLAYER *tl,
                                         char *buffer, unsigned int bsize) {
  GWEN_ERRORCODE err;
  GWEN_INETADDRESS *addr;
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;
  assert(tlp->socket);

  addr=0;
  /* get peer address */
  err=GWEN_Socket_GetPeerAddr(tlp->socket, &addr);
  if (!GWEN_Error_IsOk(err)) {
    GWEN_InetAddr_free(addr);
    return err;
  }

  /* copy to buffer while converting to char */
  err=GWEN_InetAddr_GetAddress(addr, buffer, bsize);
  GWEN_InetAddr_free(addr);
  if (!GWEN_Error_IsOk(err)) {
    return err;
  }

  /* done */
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_IPCTransportLayerSocket_GetPeerPort(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_ERRORCODE err;
  GWEN_INETADDRESS *addr;
  int i;
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;
  assert(tlp->socket);


  /* get peer address */
  addr=0;
  err=GWEN_Socket_GetPeerAddr(tlp->socket, &addr);
  if (!GWEN_Error_IsOk(err)) {
    GWEN_InetAddr_free(addr);
    return -1;
  }

  /* return port */
  i=GWEN_InetAddr_GetPort(addr);
  GWEN_InetAddr_free(addr);
  return i;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCTransportLayerSocket_free(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_IPCTRANSSOCKET *tlp;

  if (tl) {
    assert(tl->privateData);
    assert(tl->type==GWEN_TransportLayerTypeSocket);
    tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;

    GWEN_IPCTransportLayerSocket_Data_free(tlp);
  }
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCTRANSSOCKET *GWEN_IPCTransportLayerSocket_Data_new(){
  GWEN_IPCTRANSSOCKET *tlsocket;

  GWEN_NEW_OBJECT(GWEN_IPCTRANSSOCKET, tlsocket);
  return tlsocket;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCTransportLayerSocket_Data_free(GWEN_IPCTRANSSOCKET *tlsocket) {
  if (tlsocket) {
    if (tlsocket->bio) {
      GWEN_BufferedIO_free(tlsocket->bio);
    }
    else {
      /* only free the socket if it is not used by tlsocket->bio */
      GWEN_Socket_free(tlsocket->socket);
    }
    free(tlsocket);
  }
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCTRANSPORTLAYER *GWEN_IPCTransportLayerSocket_new(){
  GWEN_IPCTRANSPORTLAYER *t;
  GWEN_IPCTRANSSOCKET *tlsocket;

  t=GWEN_IPCTransportLayer_new();
  tlsocket=GWEN_IPCTransportLayerSocket_Data_new();
  t->privateData=tlsocket;
  t->address=strdup("");

  t->startConnect=GWEN_IPCTransportLayerSocket_StartConnect;
  t->finishConnect=GWEN_IPCTransportLayerSocket_FinishConnect;
  t->listen=GWEN_IPCTransportLayerSocket_Listen;
  t->accept=GWEN_IPCTransportLayerSocket_Accept;
  t->disconnect=GWEN_IPCTransportLayerSocket_Disconnect;
  t->read=GWEN_IPCTransportLayerSocket_Read;
  t->write=GWEN_IPCTransportLayerSocket_Write;
  t->canRead=GWEN_IPCTransportLayerSocket_CanRead;
  t->canWrite=GWEN_IPCTransportLayerSocket_CanWrite;
  t->getReadSocket=GWEN_IPCTransportLayerSocket_GetReadSocket;
  t->getWriteSocket=GWEN_IPCTransportLayerSocket_GetWriteSocket;
  t->getPeerAddress=GWEN_IPCTransportLayerSocket_GetPeerAddress;
  t->getPeerPort=GWEN_IPCTransportLayerSocket_GetPeerPort;
  t->flush=GWEN_IPCTransportLayerSocket_Flush;
  t->free=GWEN_IPCTransportLayerSocket_free;
  t->type=GWEN_TransportLayerTypeSocket;
  return t;
}



