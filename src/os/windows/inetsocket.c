/***************************************************************************
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002 by Martin Preuss
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



#include "inetsocket_p.h"
#include "inetaddr_p.h"
#include "errorstring.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>



GWEN_LIST_FUNCTIONS(GWEN_SOCKET, GWEN_Socket)
GWEN_LIST2_FUNCTIONS(GWEN_SOCKET, GWEN_Socket)


static int gwen_socket_is_initialized=0;



int GWEN_Socket_ModuleInit(void)
{
  WORD wVersionRequested;
  WSADATA wsaData;

  if (!gwen_socket_is_initialized) {
    int rv;

    /* setup WINSOCK (request version 1.1) */
    wVersionRequested=MAKEWORD(1, 1);
    rv=WSAStartup(wVersionRequested, &wsaData);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "Error on WSAStartup");
      return rv;
    }
    /* check if the version returned is that we requested */
    if (LOBYTE(wsaData.wVersion)!=1 ||
        HIBYTE(wsaData.wVersion)!=1) {
      WSACleanup();
      return GWEN_ERROR_STARTUP;
    }

    gwen_socket_is_initialized=1;
  }
  return 0;
}



int GWEN_Socket_ModuleFini(void)
{
  if (gwen_socket_is_initialized) {

    WSACleanup();

    gwen_socket_is_initialized=0;
  }
  return 0;
}



int GWEN_SocketSet_Clear(GWEN_SOCKETSET *ssp)
{
  assert(ssp);
  FD_ZERO(&(ssp->set));
  ssp->highest=0;
  return 0;
}



GWEN_SOCKETSET *GWEN_SocketSet_new(void)
{
  GWEN_SOCKETSET *ssp;

  GWEN_NEW_OBJECT(GWEN_SOCKETSET, ssp);
  FD_ZERO(&(ssp->set));
  return ssp;
}



GWEN_SOCKET *GWEN_Socket_fromFile(int fd)
{
  DBG_ERROR(GWEN_LOGDOMAIN,
            "No file sockets available for this system");
  return 0;
}



void GWEN_SocketSet_free(GWEN_SOCKETSET *ssp)
{
  if (ssp) {
    FD_ZERO(&(ssp->set));
    GWEN_FREE_OBJECT(ssp);
  }
}



int GWEN_SocketSet_AddSocket(GWEN_SOCKETSET *ssp,
                             const GWEN_SOCKET *sp)
{
  assert(ssp);
  assert(sp);
  if (sp->socket==-1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Socket is not connected, can not add");
    return GWEN_ERROR_NOT_OPEN;
  }
  ssp->highest=(ssp->highest<sp->socket)?sp->socket:ssp->highest;
  FD_SET(sp->socket, &(ssp->set));
  ssp->count++;
  return 0;
}



int GWEN_SocketSet_RemoveSocket(GWEN_SOCKETSET *ssp,
                                const GWEN_SOCKET *sp)
{
  assert(ssp);
  assert(sp);
  ssp->highest=(ssp->highest<sp->socket)?sp->socket:ssp->highest;
  FD_CLR(sp->socket, &(ssp->set));
  ssp->count--;
  return 0;
}



int GWEN_SocketSet_HasSocket(GWEN_SOCKETSET *ssp,
                             const GWEN_SOCKET *sp)
{
  assert(ssp);
  assert(sp);
  return FD_ISSET(sp->socket, &(ssp->set));
}



int GWEN_SocketSet_GetSocketCount(GWEN_SOCKETSET *ssp)
{
  assert(ssp);
  return ssp->count;
}





GWEN_SOCKET *GWEN_Socket_new(GWEN_SOCKETTYPE socketType)
{
  GWEN_SOCKET *sp;

  GWEN_NEW_OBJECT(GWEN_SOCKET, sp);
  GWEN_LIST_INIT(GWEN_SOCKET, sp);
  sp->type=socketType;
  return sp;
}



void GWEN_Socket_free(GWEN_SOCKET *sp)
{
  if (sp) {
    GWEN_LIST_FINI(GWEN_SOCKET, sp);
    GWEN_FREE_OBJECT(sp);
  }
}



int GWEN_Socket_Open(GWEN_SOCKET *sp)
{
  int s;

  assert(sp);
  switch (sp->type) {
  case GWEN_SocketTypeTCP:
#ifdef PF_INET
    s=socket(PF_INET, SOCK_STREAM, 0);
#else
    s=socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (s==-1)
      return WSAGetLastError();
    sp->socket=s;
    break;

  case GWEN_SocketTypeUDP:
#ifdef PF_INET
    s=socket(PF_INET, SOCK_DGRAM, 0);
#else
    s=socket(AF_INET, SOCK_DGRAM, 0);
#endif
    if (s==-1)
      return WSAGetLastError();
    sp->socket=s;
    break;

  case GWEN_SocketTypeUnix:
    DBG_ERROR(GWEN_LOGDOMAIN, "No unix domain sockets available for this system");
    return GWEN_ERROR_BAD_ADDRESS_FAMILY;
    break;

  default:
    return GWEN_ERROR_BAD_SOCKETTYPE;
  } /* switch */

  return 0;
}



int GWEN_Socket_Connect(GWEN_SOCKET *sp,
                        const GWEN_INETADDRESS *addr)
{
  assert(sp);
  if (connect(sp->socket,
              addr->address,
              addr->size)) {
    if (WSAGetLastError()!=WSAEINPROGRESS &&
        WSAGetLastError()!=WSAEWOULDBLOCK) {
      DBG_INFO(GWEN_LOGDOMAIN, "Error %d (%s)",
               WSAGetLastError(),
               GWEN_ErrorString_Windows(WSAGetLastError()));
      return WSAGetLastError();
    }
    else
      return GWEN_ERROR_IN_PROGRESS;
  }
  return 0;
}



int GWEN_Socket_Close(GWEN_SOCKET *sp)
{
  int rv;

  assert(sp);
  if (sp->socket==-1)
    return GWEN_ERROR_NOT_OPEN;

  rv=closesocket(sp->socket);
  sp->socket=-1;
  if (rv==-1)
    return WSAGetLastError();
  return 0;
}



int GWEN_Socket_Bind(GWEN_SOCKET *sp,
                     const GWEN_INETADDRESS *addr)
{
  assert(sp);
  assert(addr);
  if (bind(sp->socket,
           addr->address,
           addr->size))
    return WSAGetLastError();
  return 0;
}



int GWEN_Socket_Listen(GWEN_SOCKET *sp, int backlog)
{
  assert(sp);
  if (listen(sp->socket, backlog))
    return WSAGetLastError();
  return 0;
}



int GWEN_Socket_Accept(GWEN_SOCKET *sp,
                       GWEN_INETADDRESS **newaddr,
                       GWEN_SOCKET **newsock)
{
  int addrlen;
  GWEN_INETADDRESS *localAddr;
  GWEN_SOCKET *localSocket;
  GWEN_AddressFamily af;

  assert(sp);
  assert(newsock);
  assert(newaddr);

  switch (sp->type) {
  case GWEN_SocketTypeTCP:
  case GWEN_SocketTypeUDP:
    af=GWEN_AddressFamilyIP;
    break;
  case GWEN_SocketTypeUnix:
    af=GWEN_AddressFamilyUnix;
    break;
  default:
    return GWEN_ERROR_BAD_SOCKETTYPE;
  } /* switch */

  localAddr=GWEN_InetAddr_new(af);
  addrlen=localAddr->size;
  localSocket=GWEN_Socket_new(sp->type);
  localSocket->socket=accept(sp->socket,
                             localAddr->address,
                             &addrlen);
  if (localSocket->socket==-1) {
    GWEN_InetAddr_free(localAddr);
    GWEN_Socket_free(localSocket);
    if (WSAGetLastError()==WSAEWOULDBLOCK)
      return GWEN_ERROR_TIMEOUT;
    else
      return WSAGetLastError();
  }
  localSocket->type=sp->type;
  localAddr->size=addrlen;
  *newaddr=localAddr;
  *newsock=localSocket;
  return 0;
}



int GWEN_Socket_GetPeerAddr(GWEN_SOCKET *sp,
                            GWEN_INETADDRESS **newaddr)
{
  int addrlen;
  GWEN_INETADDRESS *localAddr;
  GWEN_AddressFamily af;

  assert(sp);
  assert(newaddr);

  switch (sp->type) {
  case GWEN_SocketTypeTCP:
  case GWEN_SocketTypeUDP:
    af=GWEN_AddressFamilyIP;
    break;
  case GWEN_SocketTypeUnix:
    af=GWEN_AddressFamilyUnix;
    break;
  default:
    return GWEN_ERROR_BAD_SOCKETTYPE;
  } /* switch */

  localAddr=GWEN_InetAddr_new(af);
  addrlen=localAddr->size;

  if (getpeername(sp->socket,
                  localAddr->address, &addrlen)) {
    GWEN_InetAddr_free(localAddr);
    return WSAGetLastError();
  }
  localAddr->size=addrlen;
  *newaddr=localAddr;
  return 0;
}



int GWEN_Socket_Select(GWEN_SOCKETSET *rs,
                       GWEN_SOCKETSET *ws,
                       GWEN_SOCKETSET *xs,
                       int timeout)
{
  int h, h1, h2, h3;
  fd_set *s1, *s2, *s3;
  int rv;
  struct timeval tv;

  s1=s2=s3=0;
  h1=h2=h3=0;

  if (rs) {
    h1=rs->highest;
    s1=&rs->set;
  }
  if (ws) {
    h2=ws->highest;
    s2=&ws->set;
  }
  if (xs) {
    h3=xs->highest;
    s3=&xs->set;
  }
  h=(h1>h2)?h1:h2;
  h=(h>h3)?h:h3;
  if (timeout<0)
    /* wait for ever */
    rv=select(h+1, s1, s2, s3, 0);
  else {
    /* return immediately */
    tv.tv_sec=0;
    tv.tv_usec=timeout*1000;
    rv=select(h+1, s1, s2, s3, &tv);
  }
  if (rv<0) {
    /* error */
    if (WSAGetLastError()==WSAEINTR)
      return GWEN_ERROR_INTERRUPTED;
    else
      return WSAGetLastError();
  }
  if (rv==0)
    /* timeout */
    return GWEN_ERROR_TIMEOUT;
  return 0;
}



int GWEN_Socket_Read(GWEN_SOCKET *sp,
                     char *buffer,
                     int *bsize)
{
  int i;

  assert(sp);
  assert(buffer);
  assert(bsize);
  i=recv(sp->socket, buffer, *bsize, 0);
  if (i<0) {
    if (WSAGetLastError()==WSAEWOULDBLOCK)
      return GWEN_ERROR_TIMEOUT;
    else if (WSAGetLastError()==WSAEINTR)
      return GWEN_ERROR_INTERRUPTED;
    else
      return WSAGetLastError();
  }
  *bsize=i;
  return 0;
}



int GWEN_Socket_Write(GWEN_SOCKET *sp,
                      const char *buffer,
                      int *bsize)
{
  int i;

  assert(sp);
  assert(buffer);
  assert(bsize);
#ifndef MSG_NOSIGNAL
  i=send(sp->socket, buffer, *bsize, 0);
#else
  i=send(sp->socket, buffer, *bsize, MSG_NOSIGNAL);
#endif
  if (i<0) {
    if (WSAGetLastError()==WSAEWOULDBLOCK)
      return GWEN_ERROR_TIMEOUT;
    else if (WSAGetLastError()==WSAEINTR)
      return GWEN_ERROR_INTERRUPTED;
    else
      return WSAGetLastError();
  }
  *bsize=i;
  return 0;
}



int GWEN_Socket_ReadFrom(GWEN_SOCKET *sp,
                         GWEN_INETADDRESS **newaddr,
                         char *buffer,
                         int *bsize)
{
  int addrlen;
  int i;
  GWEN_INETADDRESS *localAddr;
  GWEN_AddressFamily af;

  assert(sp);
  assert(newaddr);
  assert(buffer);
  assert(bsize);

  switch (sp->type) {
  case GWEN_SocketTypeTCP:
  case GWEN_SocketTypeUDP:
    af=GWEN_AddressFamilyIP;
    break;
  case GWEN_SocketTypeUnix:
    af=GWEN_AddressFamilyUnix;
    break;
  default:
    return GWEN_ERROR_BAD_SOCKETTYPE;
  } /* switch */

  localAddr=GWEN_InetAddr_new(af);
  addrlen=localAddr->size;

  i=recvfrom(sp->socket,
             buffer,
             *bsize,
             0,
             localAddr->address,
             &addrlen);
  if (i<0) {
    GWEN_InetAddr_free(localAddr);
    if (WSAGetLastError()==WSAEWOULDBLOCK)
      return GWEN_ERROR_TIMEOUT;
    else if (WSAGetLastError()==WSAEINTR)
      return GWEN_ERROR_INTERRUPTED;
    else
      return WSAGetLastError();
  }
  *bsize=i;
  localAddr->size=addrlen;
  *newaddr=localAddr;
  return 0;
}



int GWEN_Socket_WriteTo(GWEN_SOCKET *sp,
                        const GWEN_INETADDRESS *addr,
                        const char *buffer,
                        int *bsize)
{
  int i;

  assert(sp);
  assert(addr);
  assert(buffer);
  assert(bsize);
  i=sendto(sp->socket,
           buffer,
           *bsize,
#ifndef MSG_NOSIGNAL
           0,
#else
           MSG_NOSIGNAL,
#endif
           addr->address,
           addr->size);
  if (i<0) {
    if (WSAGetLastError()==WSAEWOULDBLOCK)
      return GWEN_ERROR_TIMEOUT;
    else if (WSAGetLastError()==WSAEINTR)
      return GWEN_ERROR_INTERRUPTED;
    else
      return WSAGetLastError();
  }
  *bsize=i;
  return 0;
}



int GWEN_Socket_SetBlocking(GWEN_SOCKET *sp,
                            int b)
{
  unsigned long fl;

  assert(sp);
  fl=!b;
  if (ioctlsocket(sp->socket, FIONBIO, &fl)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error %d (%s)",
             WSAGetLastError(),
             GWEN_ErrorString_Windows(WSAGetLastError()));
    return WSAGetLastError();
  }
  return 0;
}



int GWEN_Socket_SetBroadcast(GWEN_SOCKET *sp,
                             int fl)
{

  assert(sp);
  if (sp->type==GWEN_SocketTypeUnix)
    return 0;
  if (setsockopt(sp->socket,
                 SOL_SOCKET,
                 SO_BROADCAST,
                 (const char *)&fl,
                 sizeof(fl)))
    return WSAGetLastError();
  return 0;
}



int GWEN_Socket_SetReuseAddress(GWEN_SOCKET *sp, int fl)
{
  assert(sp);

  /*if (sp->type==SocketTypeUnix)
    return 0;*/

  if (setsockopt(sp->socket,
                 SOL_SOCKET,
                 SO_REUSEADDR,
                 (const char *)&fl,
                 sizeof(fl)))
    return WSAGetLastError();
  return 0;
}



int GWEN_Socket_GetSocketError(GWEN_SOCKET *sp)
{
  int rv;
  int rvs;

  assert(sp);
  rvs=sizeof(rv);
  if (-1==getsockopt(sp->socket, SOL_SOCKET, SO_ERROR, (char *)&rv, &rvs)) {
    DBG_INFO(GWEN_LOGDOMAIN, "getsockopt(): %s", GWEN_ErrorString_Windows(WSAGetLastError()));
    return GWEN_ERROR_IO;
  }

  if (rv) {
    switch (rv) {
    case WSAEWOULDBLOCK:
      return GWEN_ERROR_TIMEOUT;
    case WSAEINTR:
      return GWEN_ERROR_INTERRUPTED;
    default:
      DBG_INFO(GWEN_LOGDOMAIN, "getsockopt(): %d (%s)", rv, GWEN_ErrorString_Windows(rv));
      return GWEN_ERROR_IO;
    }
  }

  return 0;
}



int GWEN_Socket_WaitForRead(GWEN_SOCKET *sp, int timeout)
{
  int err;
  GWEN_SOCKETSET *set;

  set=GWEN_SocketSet_new();

  err=GWEN_SocketSet_AddSocket(set, sp);
  if (err) {
    GWEN_SocketSet_free(set);
    return err;
  }
  err=GWEN_Socket_Select(set, 0, 0, timeout);
  GWEN_SocketSet_free(set);

  return err;
}



int GWEN_Socket_WaitForWrite(GWEN_SOCKET *sp, int timeout)
{
  int err;
  GWEN_SOCKETSET *set;

  set=GWEN_SocketSet_new();
  err=GWEN_SocketSet_AddSocket(set, sp);
  if (err) {
    GWEN_SocketSet_free(set);
    return err;
  }
  err=GWEN_Socket_Select(0, set, 0, timeout);
  GWEN_SocketSet_free(set);

  return err;
}



GWEN_SOCKETTYPE GWEN_Socket_GetSocketType(GWEN_SOCKET *sp)
{
  assert(sp);
  return sp->type;
}



int GWEN_Socket_GetSocketInt(const GWEN_SOCKET *sp)
{
  assert(sp);
  return sp->socket;
}



const char *GWEN_Socket_ErrorString(int c)
{
  const char *s;

  switch (c) {
  case 0:
    s="Success";
    break;
  case GWEN_SOCKET_ERROR_BAD_SOCKETTYPE:
    s="Bad socket type";
    break;
  case GWEN_SOCKET_ERROR_NOT_OPEN:
    s="Socket not open";
    break;
  case GWEN_SOCKET_ERROR_TIMEOUT:
    s="Socket timeout";
    break;
  case GWEN_SOCKET_ERROR_IN_PROGRESS:
    s="Operation in progress";
    break;
  case GWEN_SOCKET_ERROR_INTERRUPTED:
    s="Operation interrupted by system signal.";
    break;
  case GWEN_SOCKET_ERROR_ABORTED:
    s="Operation aborted by user.";
    break;
  case GWEN_SOCKET_ERROR_BROKEN_PIPE:
    s="Broken connection.";
    break;
  default:
    if (c>0)
      s=GWEN_ErrorString_Windows(c);
    else
      s=(const char *)0;
  } /* switch */
  return s;
}



