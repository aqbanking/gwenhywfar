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


#include "transportlayersocket_p.h"
#include <gwenhyfwar/inetsocket.h>
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>



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
  // not yet finished or real error ?
  if (GWEN_Error_GetType(err)!=GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
      GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_IN_PROGRESS) {
    // real error, so return that error
    return err;
  }

  /* connect is in progress now */
  return 0;
}



GWEN_ERRORCODE
GWEN_IPCTransportLayerSocket_FinishConnect(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_ERRORCODE err;
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;

  // get socket error
  err=GWEN_Socket_GetSocketError(tlp->socket);
  if (!GWEN_Error_IsOk(err))
    return err;

  // make socket blocking again
  err=GWEN_Socket_SetBlocking(tlp->socket,1);
  if (!GWEN_Error_IsOk(err))
    return err;
  return 0;
}



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
  return 0;
}



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
  tlp2->socket=s;
  return 0;
}



GWEN_ERRORCODE GWEN_IPCTransportLayerSocket_Disconnect(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_ERRORCODE err;
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;

  if (tlp->socket) {
    err=GWEN_Socket_Close(tlp->socket);
    GWEN_Socket_free(tlp->socket);
    tlp->socket=0;
    if (!GWEN_Error_IsOk(err))
      return err;
  }
  return 0;
}



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
  return tlp->socket;
}



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
  return tlp->socket;
}



GWEN_ERRORCODE GWEN_IPCTransportLayerSocket_Read(GWEN_IPCTRANSPORTLAYER *tl,
                                                 char *buffer,
                                                 int *bsize){
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;
  assert(tlp->socket);
  return GWEN_Socket_Read(tlp->socket,buffer,bsize);
}



GWEN_ERRORCODE GWEN_IPCTransportLayerSocket_Write(GWEN_IPCTRANSPORTLAYER *tl,
                                               const char *buffer,
                                               int *bsize){
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;
  assert(tlp->socket);
  return GWEN_Socket_Write(tlp->socket,buffer,bsize);
}



GWEN_ERRORCODE GWEN_IPCTransportLayerSocket_CanRead(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;
  assert(tlp->socket);
  return GWEN_Socket_WaitForRead(tlp->socket,0);
}



GWEN_ERRORCODE GWEN_IPCTransportLayerSocket_CanWrite(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_IPCTRANSSOCKET *tlp;

  assert(tl);
  assert(tl->privateData);
  assert(tl->type==GWEN_TransportLayerTypeSocket);
  tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;
  assert(tlp->socket);

  return GWEN_Socket_WaitForWrite(tlp->socket,0);
}



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



void GWEN_IPCTransportLayerSocket_free(GWEN_IPCTRANSPORTLAYER *tl){
  GWEN_IPCTRANSSOCKET *tlp;

  if (tl) {
    assert(tl->privateData);
    assert(tl->type==GWEN_TransportLayerTypeSocket);
    tlp=(GWEN_IPCTRANSSOCKET*)tl->privateData;

    GWEN_IPCTransportLayerSocket_Data_free(tlp);
    free(tl);
  }
}



GWEN_IPCTRANSSOCKET *GWEN_IPCTransportLayerSocket_Data_new(){
  GWEN_IPCTRANSSOCKET *tlsocket;

  GWEN_NEW_OBJECT(GWEN_IPCTRANSSOCKET, tlsocket);
  return tlsocket;
}



void GWEN_IPCTransportLayerSocket_Data_free(GWEN_IPCTRANSSOCKET *tlsocket) {
  if (tlsocket) {
    GWEN_Socket_free(tlsocket->socket);
    free(tlsocket);
  }
}



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
  t->free=GWEN_IPCTransportLayerSocket_free;
  t->type=GWEN_TransportLayerTypeSocket;
  return t;
}



