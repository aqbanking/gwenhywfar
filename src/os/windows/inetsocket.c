/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
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

#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "chameleon/inetsocket.h"
#include "chameleon/debug.h"


// forward declaration
const char *Socket_ErrorString(int c);


int socket_is_initialized=0;
ERRORTYPEREGISTRATIONFORM socket_error_descr= {
    Socket_ErrorString,
    0,
    "Socket"};



ERRORCODE Socket_ModuleInit(){
    WORD wVersionRequested;
    WSADATA wsaData;
    int err, i;

    if (!socket_is_initialized) {
	// register error type
	if (!Error_RegisterType(&socket_error_descr))
	    return Error_New(0,
			     ERROR_SEVERITY_ERR,
			     ERROR_TYPE_ERROR,
			     ERROR_COULD_NOT_REGISTER);

        // setup WINSOCK (request version 1.1)
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if (err)
	    return Error_New(0,
			     ERROR_SEVERITY_ERR,
			     socket_error_descr.typ,
			     err);
	// check if the version returned is that we requested
	if (LOBYTE(wsaData.wVersion)!=1 ||
	    HIBYTE(wsaData.wVersion)!=1) {
	    WSACleanup();
	    return Error_New(0,
			     ERROR_SEVERITY_ERR,
			     socket_error_descr.typ,
			     SOCKET_ERROR_STARTUP);
	}

	socket_is_initialized=1;
    }
    return 0;
}


ERRORCODE Socket_ModuleFini(){
    if (socket_is_initialized) {
	socket_is_initialized=0;
	WSACleanup();
	if (!Error_UnregisterType(&socket_error_descr))
	    return Error_New(0,
			     ERROR_SEVERITY_ERR,
			     ERROR_TYPE_ERROR,
			     ERROR_COULD_NOT_UNREGISTER);
    }
    return 0;
}


ERRORCODE SocketSet_Create(SOCKETSETPTR ssp){
    assert(ssp);
    return SocketSet_Clear(ssp);
}


ERRORCODE SocketSet_Destroy(SOCKETSETPTR ssp){
    assert(ssp);
    return 0;
}


ERRORCODE SocketSet_Clear(SOCKETSETPTR ssp){
    assert(ssp);
    FD_ZERO(&(ssp->set));
    ssp->highest=0;
    return 0;
}


ERRORCODE SocketSet_AddSocket(SOCKETSETPTR ssp,
			      const struct SOCKETSTRUCT *sp){
    assert(ssp);
    assert(sp);
    ssp->highest=(ssp->highest<sp->socket)?sp->socket:ssp->highest;
    FD_SET(sp->socket,&(ssp->set));
    return 0;
}


ERRORCODE SocketSet_RemoveSocket(SOCKETSETPTR ssp,
				 const struct SOCKETSTRUCT *sp){
    assert(ssp);
    assert(sp);
    ssp->highest=(ssp->highest<sp->socket)?sp->socket:ssp->highest;
    FD_CLR(sp->socket,&(ssp->set));
    return 0;
}


int SocketSet_HasSocket(SOCKETSETPTR ssp,
			const struct SOCKETSTRUCT *sp){
    assert(ssp);
    assert(sp);
    return FD_ISSET(sp->socket,&(ssp->set));
}




SOCKETPTR Socket_new(){
  SOCKETPTR sp;

  sp=(struct SOCKETSTRUCT*)malloc(sizeof(struct SOCKETSTRUCT));
  assert(sp);
  memset(sp, 0, sizeof(struct SOCKETSTRUCT));
  return sp;
}


void Socket_free(SOCKETPTR sp){
  if (sp)
    free(sp);
}


ERRORCODE Socket_Open(SOCKETPTR sp, SOCKETTYPE socketType){
    int s;

    assert(sp);
    switch(socketType) {
    case SocketTypeTCP:
	s=socket(PF_INET, SOCK_STREAM,0);
	if (s==-1) {
	  DBG_ERROR("Error %d (%s)",
		    WSAGetLastError(),
		    strerror(WSAGetLastError()));
	  return Error_New(0,
			   ERROR_SEVERITY_ERR,
			   socket_error_descr.typ,
			   WSAGetLastError());
	}
	sp->socket=s;
	break;

    case SocketTypeUDP:
	s=socket(PF_INET, SOCK_DGRAM,0);
	if (s==-1) {
	  DBG_ERROR("Error %d (%s)",
		    WSAGetLastError(),
		    strerror(WSAGetLastError()));
	  return Error_New(0,
			   ERROR_SEVERITY_ERR,
			   socket_error_descr.typ,
			   WSAGetLastError());
	}
	sp->socket=s;
	break;

    case SocketTypeUnix:
      DBG_ERROR("Unsupported socket type %d", socketType);
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       SOCKET_ERROR_UNSUPPORTED);
      break;

    default:
      DBG_ERROR("Bad socket type %d", socketType);
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       SOCKET_ERROR_BAD_SOCKETTYPE);
    } // switch

    return 0;
}


ERRORCODE Socket_Connect(SOCKETPTR sp, const INETADDRESS *addr){
  assert(sp);
  if (connect(sp->socket,
	      (struct sockaddr*)&(addr->address),
	      sizeof(struct sockaddr_in))) {
    if (WSAGetLastError()!=WSAEINPROGRESS &&
		WSAGetLastError()!=WSAEWOULDBLOCK) {
      DBG_ERROR("Error %d (%s)",
		WSAGetLastError(),
		strerror(WSAGetLastError()));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       WSAGetLastError());
    }
    else
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       SOCKET_ERROR_IN_PROGRESS);
  }
  return 0;
}


ERRORCODE Socket_Close(SOCKETPTR sp){
    int rv;

    assert(sp);
    if (sp->socket==-1)
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       SOCKET_ERROR_NOT_OPEN);

    rv=closesocket(sp->socket);
    sp->socket=-1;
    if (rv==SOCKET_ERROR) {
      DBG_ERROR("Error %d (%s)",
		WSAGetLastError(),
		strerror(WSAGetLastError()));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       WSAGetLastError());
    }
    return 0;
}


ERRORCODE Socket_Bind(SOCKETPTR sp, const INETADDRESS *addr){
    assert(sp);
    if (bind(sp->socket,
	     (struct sockaddr*)&(addr->address),
	     sizeof(struct sockaddr_in))) {
      DBG_ERROR("Error %d (%s)",
		WSAGetLastError(),
		strerror(WSAGetLastError()));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       WSAGetLastError());
    }
    return 0;
}


ERRORCODE Socket_Listen(SOCKETPTR sp, int backlog){
    assert(sp);
    if (listen(sp->socket, backlog)) {
      DBG_ERROR("Error %d (%s)",
		WSAGetLastError(),
		strerror(WSAGetLastError()));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       WSAGetLastError());
    }
    return 0;
}


ERRORCODE Socket_Accept(SOCKETPTR sp, INETADDRESSPTR addr, SOCKETPTR newsock){
    int addrlen;

    assert(sp);
    assert(newsock);
    addrlen=sizeof(struct sockaddr_in);
    newsock->socket=accept(sp->socket,
			   (struct sockaddr*)&(addr->address),
			   &addrlen);
    if (newsock->socket==-1) {
      DBG_ERROR("Error %d (%s)",
		WSAGetLastError(),
		strerror(WSAGetLastError()));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       WSAGetLastError());
    }
    return 0;
}


ERRORCODE Socket_GetPeerAddr(SOCKETPTR sp, INETADDRESSPTR addr){
    int addrlen;

    assert(sp);
    addrlen=sizeof(struct sockaddr_in);
    if (getpeername(sp->socket,
		    (struct sockaddr*)&(addr->address),&addrlen)) {
      DBG_ERROR("Error %d (%s)",
		WSAGetLastError(),
		strerror(WSAGetLastError()));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       WSAGetLastError());
    }
    return 0;
}


ERRORCODE Socket_Select(SOCKETSETPTR rs,
			SOCKETSETPTR ws,
			SOCKETSETPTR xs,
			int timeout){
    int h,h1,h2,h3;
    fd_set *s1,*s2,*s3;
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
	// wait for ever
	rv=select(h+1,s1,s2,s3,0);
    else {
	// return immediately
	tv.tv_sec=0;
	tv.tv_usec=timeout*1000;
	rv=select(h+1,s1,s2,s3,&tv);
    }
    if (rv<0) {
	// error
      DBG_ERROR("Error %d (%s)",
		WSAGetLastError(),
		strerror(WSAGetLastError()));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       WSAGetLastError());
    }
    if (rv==0)
      // timeout
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       SOCKET_ERROR_TIMEOUT);
    return 0;
}


ERRORCODE Socket_Read(SOCKETPTR sp,
		      char *buffer,
		      int *bsize){
    int i;

    assert(sp);
    assert(buffer);
    assert(bsize);
    i=recv(sp->socket,buffer, *bsize,0);
    if (i<0) {
      DBG_ERROR("Error %d (%s)",
		WSAGetLastError(),
		strerror(WSAGetLastError()));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       WSAGetLastError());
    }
    *bsize=i;
    return 0;
}


ERRORCODE Socket_Write(SOCKETPTR sp,
		       const char *buffer,
		       int *bsize){
    int i;

    assert(sp);
    assert(buffer);
    assert(bsize);
#ifndef MSG_NOSIGNAL
    i=send(sp->socket,buffer, *bsize,0);
#else
    i=send(sp->socket,buffer, *bsize, MSG_NOSIGNAL);
#endif
    if (i<0) {
      DBG_ERROR("Error %d (%s)",
		WSAGetLastError(),
		strerror(WSAGetLastError()));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       WSAGetLastError());
    }
    *bsize=i;
    return 0;
}


ERRORCODE Socket_ReadFrom(SOCKETPTR sp,
			  INETADDRESSPTR addr,
			  char *buffer,
			  int *bsize){
    int addrlen;
    int i;

    assert(sp);
    assert(addr);
    assert(buffer);
    assert(bsize);
    addrlen=sizeof(struct sockaddr_in);
    i=recvfrom(sp->socket,
	       buffer,
	       *bsize,
               0,
	       (struct sockaddr*)&(addr->address),
	       &addrlen);
    if (i<0) {
      DBG_ERROR("Error %d (%s)",
		WSAGetLastError(),
		strerror(WSAGetLastError()));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       WSAGetLastError());
    }
    *bsize=i;
    return 0;
}


ERRORCODE Socket_WriteTo(SOCKETPTR sp,
			 const INETADDRESS *addr,
			 const char *buffer,
			 int *bsize){
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
	   (struct sockaddr*)&(addr->address),
	   sizeof(struct sockaddr_in));
  if (i<0) {
    DBG_ERROR("Error %d (%s)",
	      WSAGetLastError(),
	      strerror(WSAGetLastError()));
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     socket_error_descr.typ,
		     WSAGetLastError());
  }
  *bsize=i;
  return 0;
}


ERRORCODE Socket_SetBlocking(SOCKETPTR sp,
			     int b) {
    unsigned long fl;

    assert(sp);
    fl=!b;
    if (ioctlsocket(sp->socket,FIONBIO,&fl)) {
      DBG_ERROR("Error %d (%s)",
		WSAGetLastError(),
		strerror(WSAGetLastError()));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       WSAGetLastError());
    }
    return 0;
};


ERRORCODE Socket_SetBroadcast(SOCKETPTR sp,
			      int fl) {
    assert(sp);
    if (setsockopt(sp->socket,
		   SOL_SOCKET,
		   SO_BROADCAST,
		   &fl,
		   sizeof(fl))) {
      DBG_ERROR("Error %d (%s)",
		WSAGetLastError(),
		strerror(WSAGetLastError()));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       WSAGetLastError());
    }
    return 0;
}


ERRORCODE Socket_SetReuseAddress(SOCKETPTR sp, int fl){
  assert(sp);
  if (setsockopt(sp->socket,
		 SOL_SOCKET,
		 SO_REUSEADDR,
		 &fl,
		 sizeof(fl))) {
    DBG_ERROR("Error %d (%s)",
	      WSAGetLastError(),
	      strerror(WSAGetLastError()));
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     socket_error_descr.typ,
		     WSAGetLastError());
  }
  return 0;
}


ERRORCODE Socket_GetSocketError(SOCKETPTR sp) {
    int rv;
    unsigned int rvs;

    assert(sp);
    rvs=sizeof(rv);
    if (-1==getsockopt(sp->socket,SOL_SOCKET,SO_ERROR,(char*)&rv,&rvs)){
      DBG_ERROR("Error %d (%s)",
		WSAGetLastError(),
		strerror(WSAGetLastError()));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       WSAGetLastError());
    }
    if (rv!=0) {
      DBG_ERROR("Error %d (%s)",
		rv,
		strerror(rv));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       socket_error_descr.typ,
		       rv);
    }
    return 0;
}


ERRORCODE Socket_WaitForRead(SOCKETPTR sp,
			     int timeout) {
  ERRORCODE err;
  SOCKETSET set;

  err=SocketSet_Create(&set);
  if (!Error_IsOk(err))
    return err;

  err=SocketSet_AddSocket(&set,sp);
  if (!Error_IsOk(err))
    return err;
  err=Socket_Select(&set,0,0,timeout);
  SocketSet_Destroy(&set);
  if (Error_IsOk(err))
    return 0;

  if (Error_GetType(err)==Error_FindType("Socket") &&
      Error_GetCode(err)==SOCKET_ERROR_TIMEOUT)
    return 0;

  return err;
}


ERRORCODE Socket_WaitForWrite(SOCKETPTR sp,
			      int timeout) {
  ERRORCODE err;
  SOCKETSET set;

  err=SocketSet_Create(&set);
  if (!Error_IsOk(err))
    return err;

  err=SocketSet_AddSocket(&set,sp);
  if (!Error_IsOk(err))
    return err;
  err=Socket_Select(0,&set,0,timeout);
  SocketSet_Destroy(&set);
  if (Error_IsOk(err))
    return 0;

  if (Error_GetType(err)==Error_FindType("Socket") &&
      Error_GetCode(err)==SOCKET_ERROR_TIMEOUT)
    return 0;

  return err;

}


const char *Socket_ErrorString(int c){
    const char *s;

    switch(c) {
    case 0:
	s="Success";
        break;
    case SOCKET_ERROR_BAD_SOCKETTYPE:
	s="Bad socket type";
        break;
    case SOCKET_ERROR_NOT_OPEN:
	s="Socket not open";
        break;
    case  SOCKET_ERROR_TIMEOUT:
	s="Socket timeout";
        break;
    case SOCKET_ERROR_IN_PROGRESS:
	s="Operation in progress";
        break;
    case SOCKET_ERROR_INTERRUPTED:
      s="Operation interrupted by system signal.";
      break;
    default:
      if (c>0)
	s=strerror(c);
      else
	s=(const char*)0;
    } // switch
    return s;
}



