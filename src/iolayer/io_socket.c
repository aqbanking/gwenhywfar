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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "io_socket_p.h"
#include <gwenhywfar/iolayer_be.h>
#include <gwenhywfar/iorequest_be.h>

#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/url.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/base64.h>

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


#define GWEN_PROXY_ENVVAR "GWEN_PROXY"



GWEN_INHERIT(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET)



int GWEN_Proxy_Connect(GWEN_SOCKET *sp,
		       const GWEN_INETADDRESS *addr,
		       uint32_t guiid) {
  const char *s;

  /* do nothing for protocols other than TCP */
  if (GWEN_Socket_GetSocketType(sp)!=GWEN_SocketTypeTCP)
    return GWEN_Socket_Connect(sp, addr);

  s=getenv(GWEN_PROXY_ENVVAR);
  if (s && *s) {
    GWEN_URL *url;
    GWEN_BUFFER *sendBuf;
    int tport;
    char taddr[64];
    char numbuf[16];
    const char *t;
    int rv;
    GWEN_INETADDRESS *in;
    int ok=0;
    int first=1;
    int len;
    int idx=0;
    char wrk[1024];

    url=GWEN_Url_fromString(s);
    if (url==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad string for proxy [%s]", s);
      return GWEN_ERROR_IO;
    }

    sendBuf=GWEN_Buffer_new(0, 256, 0, 1);

    /* build command line */
    GWEN_Buffer_AppendString(sendBuf, "CONNECT ");

    tport=GWEN_InetAddr_GetPort(addr);
    GWEN_InetAddr_GetAddress(addr, taddr, sizeof(taddr));

    GWEN_Buffer_AppendString(sendBuf, taddr);
    GWEN_Buffer_AppendString(sendBuf, ":");
    snprintf(numbuf, sizeof(numbuf)-1, "%d\n", tport);
    numbuf[sizeof(numbuf)-1]=0;
    GWEN_Buffer_AppendString(sendBuf, wrk);

    GWEN_Buffer_AppendString(sendBuf, " HTTP/1.1\r\n");

    /* build host line required for HTTP/1.1 */
    GWEN_Buffer_AppendString(sendBuf, "Host: ");
    GWEN_Buffer_AppendString(sendBuf, taddr);
    GWEN_Buffer_AppendString(sendBuf, ":");
    GWEN_Buffer_AppendString(sendBuf, wrk);
    GWEN_Buffer_AppendString(sendBuf, "\r\n");

    /* possibly add auth info */
    t=GWEN_Url_GetUserName(url);
    if (t && *t) {
      GWEN_BUFFER *abuf;

      /* we have auth info */
      abuf=GWEN_Buffer_new(0, 64, 0, 1);
      GWEN_Buffer_AppendString(sendBuf, "Proxy-Authorization: ");

      GWEN_Buffer_AppendString(abuf, t);
      t=GWEN_Url_GetPassword(url);
      if (t && *t) {
	GWEN_Buffer_AppendString(abuf, ":");
	GWEN_Buffer_AppendString(abuf, t);
      }
      /* base64 encode */
      rv=GWEN_Base64_Encode((const unsigned char*) GWEN_Buffer_GetStart(abuf),
			    GWEN_Buffer_GetUsedBytes(abuf),
                            sendBuf, 0);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_Buffer_free(abuf);
	GWEN_Buffer_free(sendBuf);
	GWEN_Url_free(url);
	return rv;
      }
      GWEN_Buffer_free(abuf);
    }

    /* end header */
    GWEN_Buffer_AppendString(sendBuf, "\r\n");
    /* now sendBuf contains all necessary data */

    /* prepare connect to proxy */
    in=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
    assert(in);

    rv=GWEN_InetAddr_SetPort(in, GWEN_Url_GetPort(url));
    if (rv<0) {
      GWEN_InetAddr_free(in);
      GWEN_Buffer_free(sendBuf);
      GWEN_Url_free(url);
      return rv;
    }

    rv=GWEN_InetAddr_SetAddress(in, GWEN_Url_GetServer(url));
    if (rv) {
      if (rv==GWEN_ERROR_BAD_ADDRESS) {
	rv=GWEN_InetAddr_SetName(in, GWEN_Url_GetServer(url));
	if (rv) {
	  GWEN_InetAddr_free(in);
	  GWEN_Buffer_free(sendBuf);
	  GWEN_Url_free(url);
	  return rv;
	}
      }
      else {
	GWEN_InetAddr_free(in);
	GWEN_Buffer_free(sendBuf);
	GWEN_Url_free(url);
	return rv;
      }
    }
    GWEN_Url_free(url);

    /* this is the part I don't like very much but for now it is
     * necessary. Later this should be implemented non-blocking because the
     * code leading to the calling of this function expects the whole call
     * to be non-blocking... */
    rv=GWEN_Socket_SetBlocking(sp, 1);
    if (rv) {
      GWEN_InetAddr_free(in);
      GWEN_Buffer_free(sendBuf);
      return rv;
    }
  
    tport=GWEN_InetAddr_GetPort(in);
    GWEN_InetAddr_GetAddress(in, taddr, sizeof(taddr));
    DBG_INFO(GWEN_LOGDOMAIN, "Connecting to proxy %s (port %d)", taddr, tport);
    if (1) {
      char dbuf[256];

      snprintf(dbuf, sizeof(dbuf)-1, I18N("Connecting to proxy %s (port %d)"), taddr, tport);
      dbuf[sizeof(dbuf)-1]=0;
      GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Info, dbuf);
    }
  
    /* connect to proxy */
    rv=GWEN_Socket_Connect(sp, in);
    GWEN_InetAddr_free(in);
    if (rv) {
      if (rv==GWEN_ERROR_IN_PROGRESS) {
	rv=GWEN_Socket_WaitForWrite(sp, 10000);
	if (rv) {
	  DBG_INFO(GWEN_LOGDOMAIN, "Error connecting to proxy (%d)", rv);
	  GWEN_Buffer_free(sendBuf);
	  return rv;
	}
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "Error connecting to proxy (%d)", rv);
	GWEN_Buffer_free(sendBuf);
	return rv;
      }
    }
  
    /* write connect command */
    first=1;
    if (1) {
      const char *p;

      len=GWEN_Buffer_GetUsedBytes(sendBuf);
      p=GWEN_Buffer_GetStart(sendBuf);
      while(len) {
	int rl;

	rv=GWEN_Socket_WaitForWrite(sp, first?40000:1000);
	if (rv) {
	  DBG_INFO(GWEN_LOGDOMAIN, "Error writing to proxy (%d)", rv);
	  GWEN_Buffer_free(sendBuf);
	  return rv;
	}
    
	first=0;

        rl=len;
	rv=GWEN_Socket_Write(sp, p, &rl);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "Error writing to proxy (%d)", rv);
	  GWEN_Buffer_free(sendBuf);
	  return rv;
	}
	if (rl<1) {
	  DBG_INFO(GWEN_LOGDOMAIN, "Zero bytes written to proxy, aborting");
	  GWEN_Buffer_free(sendBuf);
	  return GWEN_ERROR_IO;
	}
	p+=rl;
	len-=rl;
      }
    }
    GWEN_Buffer_free(sendBuf);

    /* read response */
    first=1;
    while (1) {
      rv=GWEN_Socket_WaitForRead(sp, first?40000:1000);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "Error reading from proxy (%d)", rv);
	return rv;
      }
  
      first=0;
      len=1;
      rv=GWEN_Socket_Read(sp, wrk+idx, &len);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "Error reading from proxy (%d)", rv);
	return rv;
      }
  
      if(wrk[idx]=='\r')
	continue;
  
      if (wrk[idx]=='\n') {
	if (!idx) {
	  /* empty line: header end, return result of previous status check */
	  if (ok) {
	    DBG_INFO(GWEN_LOGDOMAIN, "Proxy accepted CONNECT request, EOLN met");
	    GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Info, I18N("Proxy accepted CONNECT request."));
	    return 0;
	  }
	  else {
	    DBG_INFO(GWEN_LOGDOMAIN, "Emtpy line end, unknown proxy status");
	    return GWEN_ERROR_IO;
	  }
	}

	if (!ok) {
	  wrk[idx]=0;
	  if (strncmp(wrk, "HTTP/1.0 200", 12)==0 ||
	      strncmp(wrk, "HTTP/1.1 200", 12)==0)
	    ok=1;
	  else {
	    DBG_ERROR(GWEN_LOGDOMAIN, "Proxy rejected CONNECT request: %s", wrk);
	    if (1) {
	      GWEN_BUFFER *dbuf;

	      dbuf=GWEN_Buffer_new(0, 256, 0, 1);
	      GWEN_Buffer_AppendString(dbuf, I18N("Proxy rejected CONNECT request: "));
	      GWEN_Buffer_AppendString(dbuf, wrk);
	      GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Info, GWEN_Buffer_GetStart(dbuf));
	      GWEN_Buffer_free(dbuf);
	    }
	    return GWEN_ERROR_IO;
	  }
	}
	idx=0;
      }
      else if (++idx==sizeof(wrk))
	return GWEN_ERROR_IO;
    }
  }
  else
    return GWEN_Socket_Connect(sp, addr);
}



#if 0
int GWEN_Proxy_Connect(GWEN_SOCKET *sp, const GWEN_INETADDRESS *addr) {
  int err;
  int tport;
  int len;
  int idx=0;
  int ok=0;
  int first=1;
  char *proxy;
  char *port;
  GWEN_INETADDRESS *in;
  char taddr[64];
  char wrk[1024];

  if (GWEN_Socket_GetSocketType(sp)!=GWEN_SocketTypeTCP)
    return GWEN_Socket_Connect(sp, addr);

  tport=GWEN_InetAddr_GetPort(addr);
  GWEN_InetAddr_GetAddress(addr, taddr, sizeof(taddr));
  snprintf(wrk, sizeof(wrk)-1, "CONNECT %s:%d\n", taddr, tport);
  wrk[sizeof(wrk)-1]=0;
  len=strlen(wrk);

  port=proxy=strdup(getenv("GWEN_PROXY"));
  assert(port);

  while (*port) {
    if (*port==':') {
      *port++=0;
      break;
    }
    else
      port++;
  }

  if (!*port) {
    free(proxy);
    return GWEN_ERROR_BAD_ADDRESS;
  }

  in=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  assert(in);

  err=GWEN_InetAddr_SetPort(in, atoi(port));
  if (err) {
    free(proxy);
    GWEN_InetAddr_free(in);
    return err;
  }

  err=GWEN_InetAddr_SetAddress(in, proxy);
  if (err) {
    if (err==GWEN_ERROR_BAD_ADDRESS) {
      err=GWEN_InetAddr_SetName(in, proxy);
      if (err) {
	free(proxy);
	GWEN_InetAddr_free(in);
	return err;
      }
    }
    else {
      free(proxy);
      GWEN_InetAddr_free(in);
      return err;
    }
  }

  free(proxy);

  /* this is the part I don't like very much but for now it is
   * necessary. Later this should be implemented non-blocking because the
   * code leading to the calling of this function expects the whole call
   * to be non-blocking... */
  err=GWEN_Socket_SetBlocking(sp, 1);
  if (err) {
    GWEN_InetAddr_free(in);
    return err;
  }

  tport=GWEN_InetAddr_GetPort(in);
  GWEN_InetAddr_GetAddress(in, taddr, sizeof(taddr));
  DBG_INFO(GWEN_LOGDOMAIN, "Connecting to proxy %s (port %d)", taddr, tport);

  /* connect to proxy */
  err=GWEN_Socket_Connect(sp, in);
  GWEN_InetAddr_free(in);
  if (err) {
    if (err==GWEN_ERROR_IN_PROGRESS) {
      err=GWEN_Socket_WaitForWrite(sp, 10000);
      if (err)
        return err;
    }
    else
      return err;
  }

  /* write connect command */
  err=GWEN_Socket_Write(sp, wrk, &len);
  if (err)
    return err;

  while (1) {
    err=GWEN_Socket_WaitForRead(sp, first?40000:1000);
    if (err)
      return err;

    first=0;
    len=1;
    err=GWEN_Socket_Read(sp, wrk+idx, &len);
    if (err)
      return err;

    if(wrk[idx]=='\r')
      continue;

    if(wrk[idx]=='\n') {
      if (!idx) {
	if (ok)
	  return 0;
	else
	  return GWEN_ERROR_IO;
      }

      if(!ok) {
	wrk[idx]=0;
	if (strncmp(wrk, "HTTP/1.0 200", 12)==0 ||
	    strncmp(wrk, "HTTP/1.1 200", 12)==0)
	  ok=1;
	else
	  return GWEN_ERROR_IO;
      }

      idx=0;
    }
    else if (++idx==sizeof(wrk))
      return GWEN_ERROR_IO;
  }
}
#endif



GWEN_IO_LAYER *GWEN_Io_LayerSocket_new(GWEN_SOCKET *sk) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_SOCKET *xio;

  io=GWEN_Io_Layer_new(GWEN_IO_LAYER_SOCKET_TYPE, NULL);
  assert(io);
  GWEN_NEW_OBJECT(GWEN_IO_LAYER_SOCKET, xio);
  assert(xio);
  GWEN_INHERIT_SETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io, xio, GWEN_Io_LayerSocket_freeData);

  GWEN_Io_Layer_SetWorkOnRequestsFn(io, GWEN_Io_LayerSocket_WorkOnRequests);
  GWEN_Io_Layer_SetAddRequestFn(io, GWEN_Io_LayerSocket_AddRequest);
  GWEN_Io_Layer_SetDelRequestFn(io, GWEN_Io_LayerSocket_DelRequest);
  GWEN_Io_Layer_SetHasWaitingRequestsFn(io, GWEN_Io_LayerSocket_HasWaitingRequests);
  GWEN_Io_Layer_SetAddWaitingSocketsFn(io, GWEN_Io_LayerSocket_AddWaitingSockets);
  GWEN_Io_Layer_SetListenFn(io, GWEN_Io_LayerSocket_Listen);

  xio->socket=sk;

  GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusUnconnected);


  return io;
}



GWENHYWFAR_CB
void GWEN_Io_LayerSocket_freeData(void *bp, void *p) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_SOCKET *xio;
  uint32_t lflags;

  io=(GWEN_IO_LAYER*) bp;
  assert(io);
  xio=(GWEN_IO_LAYER_SOCKET*) p;
  assert(xio);

  GWEN_Io_LayerSocket_AbortRequests(io, GWEN_ERROR_ABORTED);
  lflags=GWEN_Io_Layer_GetFlags(io);

  /* close socket (if it not already is) */
  if (!(lflags & GWEN_IO_LAYER_FLAGS_DONTCLOSE))
    GWEN_Socket_Close(xio->socket);

  /* free ressource (if requested) */
  if (lflags & GWEN_IO_LAYER_FLAGS_TAKEOVER) {
    GWEN_Socket_free(xio->socket);
    xio->socket=(GWEN_SOCKET*) -1;
  }

  /* done */
  GWEN_FREE_OBJECT(xio);
}



GWEN_INETADDRESS *GWEN_Io_LayerSocket_GetLocalAddr(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_SOCKET *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io);
  assert(xio);

  return xio->localAddr;
}



void GWEN_Io_LayerSocket_SetLocalAddr(GWEN_IO_LAYER *io, const GWEN_INETADDRESS *addr) {
  GWEN_IO_LAYER_SOCKET *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io);
  assert(xio);

  GWEN_InetAddr_free(xio->localAddr);
  if (addr) xio->localAddr=GWEN_InetAddr_dup(addr);
  else xio->localAddr=NULL;
}



GWEN_INETADDRESS *GWEN_Io_LayerSocket_GetPeerAddr(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_SOCKET *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io);
  assert(xio);

  return xio->peerAddr;
}



void GWEN_Io_LayerSocket_SetPeerAddr(GWEN_IO_LAYER *io, const GWEN_INETADDRESS *addr) {
  GWEN_IO_LAYER_SOCKET *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io);
  assert(xio);

  GWEN_InetAddr_free(xio->peerAddr);
  if (addr) xio->peerAddr=GWEN_InetAddr_dup(addr);
  else xio->peerAddr=NULL;
}



GWEN_SOCKET *GWEN_Io_LayerSocket_GetSocket(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_SOCKET *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io);
  assert(xio);

  return xio->socket;
}



void GWEN_Io_LayerSocket_AbortRequests(GWEN_IO_LAYER *io, int errorCode) {
  GWEN_IO_LAYER_SOCKET *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io);
  assert(xio);

  if (xio->connectRequest) {
    GWEN_IO_REQUEST *r;

    r=xio->connectRequest;
    xio->connectRequest=NULL;
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, errorCode);
    GWEN_Io_Request_free(r);
  }
  if (xio->readRequest) {
    GWEN_IO_REQUEST *r;

    r=xio->readRequest;
    xio->readRequest=NULL;
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, errorCode);
    GWEN_Io_Request_free(r);
  }
  if (xio->writeRequest) {
    GWEN_IO_REQUEST *r;

   r=xio->writeRequest;
   xio->writeRequest=NULL;
   GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, errorCode);
   GWEN_Io_Request_free(r);
  }
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerSocket_CheckForIncoming(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_SOCKET *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io);
  assert(xio);

  if (GWEN_Io_Layer_GetStatus(io)==GWEN_Io_Layer_StatusListening) {
    int rv;
    GWEN_SOCKET *newS=NULL;
    GWEN_INETADDRESS *iaddr=NULL;

    /* accept new connection (if there is any) */
    rv=GWEN_Socket_Accept(xio->socket, &iaddr, &newS);
    if (rv<0) {
      if (rv!=GWEN_ERROR_TIMEOUT && rv!=GWEN_ERROR_INTERRUPTED) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
	doneSomething=1;
      }
      /* still no incoming connection */
    }
    else {
      char addrBuffer[128];
      int port;
      GWEN_IO_LAYER *newIo;

      /* get peer address for logging */
      GWEN_InetAddr_GetAddress(iaddr, addrBuffer, sizeof(addrBuffer));
      port=GWEN_InetAddr_GetPort(iaddr);
      DBG_INFO(GWEN_LOGDOMAIN, "Incoming connection from %s (port %d)", addrBuffer, port);

      rv=GWEN_Socket_SetBlocking(newS, 0);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_Socket_Close(newS);
	GWEN_Socket_free(newS);
	GWEN_InetAddr_free(iaddr);
      }
      else {
	/* got a connection, set it up */
	newIo=GWEN_Io_LayerSocket_new(newS);
	GWEN_Io_LayerSocket_SetLocalAddr(newIo, xio->localAddr);
	GWEN_Io_LayerSocket_SetPeerAddr(newIo, iaddr);
	GWEN_InetAddr_free(iaddr);
	GWEN_Io_Layer_AddFlags(newIo,
			       GWEN_IO_LAYER_FLAGS_PASSIVE |
			       GWEN_IO_LAYER_FLAGS_TAKEOVER); /* take over new socket */
	GWEN_Io_Layer_SetStatus(newIo, GWEN_Io_Layer_StatusConnected);

	GWEN_Io_Layer_AddIncomingLayer(io, newIo);
      }
      doneSomething=1;
    }
  }

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerSocket_WorkOnRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_SOCKET *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "LayerSocket: Working");

  /* work on connect request */
  if (xio->connectRequest) {
    int rv;
    GWEN_IO_REQUEST *r;

    r=xio->connectRequest;
    rv=GWEN_Socket_GetSocketError(xio->socket);
    if (rv<0) {
      if (rv!=GWEN_ERROR_TIMEOUT && rv!=GWEN_ERROR_INTERRUPTED) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_Io_LayerSocket_AbortRequests(io, GWEN_ERROR_ABORTED);
	GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
	doneSomething=1;
      }
      /* still not connected */
    }
    else {
      char addrBuffer[128];
      int port;

      /* get address for logging */
      GWEN_InetAddr_GetAddress(xio->peerAddr, addrBuffer, sizeof(addrBuffer));
      port=GWEN_InetAddr_GetPort(xio->peerAddr);
      /* connected */
      xio->connectRequest=NULL;
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
      GWEN_Io_Request_free(r);
      GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusConnected);
      DBG_INFO(GWEN_LOGDOMAIN, "Now connected to %s (port %d)", addrBuffer, port);
      doneSomething=1;
    }
  }

  /* work on read request */
  if (xio->readRequest) {
    ssize_t rv;
    int bytesRead;
    GWEN_IO_REQUEST *r;

    r=xio->readRequest;
    bytesRead=GWEN_Io_Request_GetBufferSize(r)-GWEN_Io_Request_GetBufferPos(r);
    rv=GWEN_Socket_Read(xio->socket,
			(char*) GWEN_Io_Request_GetBufferPtr(r)+
			GWEN_Io_Request_GetBufferPos(r),
			&bytesRead);
    if (rv<0) {
      if (rv!=GWEN_ERROR_TIMEOUT && rv!=GWEN_ERROR_INTERRUPTED) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", (int)rv);
	xio->readRequest=NULL;
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_IO);
	GWEN_Io_Request_free(r);
	doneSomething=1;
      }
      /* nothing to read, nothing done, so we don't set doneSomething=1 here ! */
    }
    else {
      if (bytesRead==0) {
	/* end of stream reached */
	DBG_INFO(GWEN_LOGDOMAIN, "End of stream reached");
	xio->readRequest=NULL;
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_EOF);
	GWEN_Io_Request_free(r);
	doneSomething=1;
      }
      else {
	uint32_t newPos;

	/* some data returned */
	newPos=GWEN_Io_Request_GetBufferPos(r)+bytesRead;
	GWEN_Io_Request_SetBufferPos(r, newPos);

	if (newPos>=GWEN_Io_Request_GetBufferSize(r) ||
	    !(GWEN_Io_Request_GetFlags(r) & GWEN_IO_REQUEST_FLAGS_READALL)) {
	  xio->readRequest=NULL;
	  GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	  GWEN_Io_Request_free(r);
	  DBG_VERBOUS(GWEN_LOGDOMAIN, "Read request finished (read %d bytes)", newPos);
	}
	else {
	  DBG_VERBOUS(GWEN_LOGDOMAIN, "Read request waiting (got %d bytes)", newPos);
	}
	doneSomething=1;
      }
    }
  }

  /* work on write request */
  if (xio->writeRequest) {
    int bytesWritten;
    GWEN_IO_REQUEST *r;
    int rv;

    r=xio->writeRequest;
    bytesWritten=GWEN_Io_Request_GetBufferSize(r)-GWEN_Io_Request_GetBufferPos(r);
    rv=GWEN_Socket_Write(xio->socket,
			 (const char*)GWEN_Io_Request_GetBufferPtr(r)+
			 GWEN_Io_Request_GetBufferPos(r),
			 &bytesWritten);
    if (rv<0) {
      if (rv!=GWEN_ERROR_TIMEOUT && rv!=GWEN_ERROR_INTERRUPTED) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	xio->writeRequest=NULL;
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_IO);
	GWEN_Io_Request_free(r);
	doneSomething=1;
      }
      /* nothing to read, nothing done, so we don't set doneSomething=1 here ! */
    }
    else {
      uint32_t newPos;

      /* some data returned */
      newPos=GWEN_Io_Request_GetBufferPos(r)+bytesWritten;
      GWEN_Io_Request_SetBufferPos(r, newPos);

      if (newPos>=GWEN_Io_Request_GetBufferSize(r) ||
	  !(GWEN_Io_Request_GetFlags(r) & GWEN_IO_REQUEST_FLAGS_WRITEALL)) {
	/* request complete */
	xio->writeRequest=NULL;
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	GWEN_Io_Request_free(r);
	DBG_INFO(GWEN_LOGDOMAIN, "Write request finished (%d bytes written)", newPos);
      }
      doneSomething=1;
    }
  }

  /* possibly check for incoming layers */
  if (GWEN_Io_LayerSocket_CheckForIncoming(io)!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;


  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



int GWEN_Io_LayerSocket_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_SOCKET *xio;
  GWEN_IO_LAYER_STATUS st;
  uint32_t lflags;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io);
  assert(xio);

  st=GWEN_Io_Layer_GetStatus(io);
  lflags=GWEN_Io_Layer_GetFlags(io);

  switch(GWEN_Io_Request_GetType(r)) {
  case GWEN_Io_Request_TypeConnect:
    /* check status */
    if (st==GWEN_Io_Layer_StatusConnected &&
	(lflags & GWEN_IO_LAYER_FLAGS_PASSIVE)) {
      DBG_INFO(GWEN_LOGDOMAIN, "Socket already connected");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
      return 0;
    }
    if (st!=GWEN_Io_Layer_StatusUnconnected &&
	st!=GWEN_Io_Layer_StatusDisconnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "Socket is open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_INVALID);
      return GWEN_ERROR_NOT_OPEN;
    }

    /* check whether we already have a read request */
    if (xio->connectRequest) {
      DBG_INFO(GWEN_LOGDOMAIN, "There already is a connect request");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_IN_PROGRESS);
      return GWEN_ERROR_IN_PROGRESS;
    }
    else {
      char addrBuffer[128];
      int port;
      int rv;

      /* get address for logging */
      GWEN_InetAddr_GetAddress(xio->peerAddr, addrBuffer, sizeof(addrBuffer));
      port=GWEN_InetAddr_GetPort(xio->peerAddr);
      DBG_INFO(GWEN_LOGDOMAIN, "Starting to connect to %s (port %d)", addrBuffer, port);

      /* not a passive io layer */
      GWEN_Io_Layer_SubFlags(io, GWEN_IO_LAYER_FLAGS_PASSIVE);

      /* open socket */
      rv=GWEN_Socket_Open(xio->socket);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, rv);
        return rv;
      }

      /* set nonblocking */
      rv=GWEN_Socket_SetBlocking(xio->socket, 0);
      if (rv) {
        GWEN_Socket_Close(xio->socket);
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, rv);
	return rv;
      }

      /* actually start to connect */
      rv=GWEN_Proxy_Connect(xio->socket, xio->peerAddr, GWEN_Io_Request_GetGuiId(r));
      /* not yet finished or real error ? */
      if (rv) {
	if (rv!=GWEN_ERROR_IN_PROGRESS) {
	  /* real error, so return that error */
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, rv);
	}
	else {
	  /* enqueue request */
	  xio->connectRequest=r;
	  GWEN_Io_Request_Attach(xio->connectRequest);
	  GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusConnecting);
	}
      }
      else {
	/* connected */
	DBG_INFO(GWEN_LOGDOMAIN, "Immediately connected to %s (port %d)", addrBuffer, port);
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusConnected);
      }
    }
    break;

  case GWEN_Io_Request_TypeDisconnect:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "Io layer not connected");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }
    else {
      if ((xio->readRequest==NULL && xio->writeRequest==NULL) ||
	  (GWEN_Io_Request_GetFlags(r) & GWEN_IO_REQUEST_FLAGS_FORCE)) {
	/* close files if not forbidden */
	if (!(lflags & GWEN_IO_LAYER_FLAGS_DONTCLOSE))
	  GWEN_Socket_Close(xio->socket);
	GWEN_Io_LayerSocket_AbortRequests(io, GWEN_ERROR_ABORTED);

	/* closed */
	GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
      }
      else {
	/* still some pending operations, try again */
        return GWEN_ERROR_TRY_AGAIN;
      }
    }
    break;

  case GWEN_Io_Request_TypeRead:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "Socket is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }

    /* check whether we already have a read request */
    if (xio->readRequest) {
      DBG_INFO(GWEN_LOGDOMAIN, "There already is a read request");
      return GWEN_ERROR_TRY_AGAIN;
    }

    /* enqueue request */
    xio->readRequest=r;
    GWEN_Io_Request_Attach(xio->readRequest);
    break;

  case GWEN_Io_Request_TypeWrite:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "File is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }

    /* check whether we already have a write request */
    if (xio->writeRequest) {
      DBG_INFO(GWEN_LOGDOMAIN, "There already is a write request");
      return GWEN_ERROR_TRY_AGAIN;
    }

    /* enqueue request */
    xio->writeRequest=r;
    GWEN_Io_Request_Attach(xio->writeRequest);
    break;

  default:
    DBG_INFO(GWEN_LOGDOMAIN, "This request type is not supported (%d)", GWEN_Io_Request_GetType(r));
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_SUPPORTED);
    return GWEN_ERROR_NOT_SUPPORTED;
  }

  return 0;
}



int GWEN_Io_LayerSocket_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_SOCKET *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io);
  assert(xio);

  switch(GWEN_Io_Request_GetType(r)) {
  case GWEN_Io_Request_TypeRead:
    if (xio->readRequest==r) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Aborted read request");
      xio->readRequest=NULL;
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_ABORTED);
      GWEN_Io_Request_free(r);
    }
    else {
      /* not my request */
      DBG_INFO(GWEN_LOGDOMAIN, "Read request not registered with this io layer");
      return GWEN_ERROR_INVALID;
    }
    break;

  case GWEN_Io_Request_TypeWrite:
    if (xio->writeRequest==r) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Aborted write request");
      xio->writeRequest=NULL;
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_ABORTED);
      GWEN_Io_Request_free(r);
    }
    else {
      /* not my request */
      DBG_INFO(GWEN_LOGDOMAIN, "Write request not registered with this io layer");
      return GWEN_ERROR_INVALID;
    }
    break;

  case GWEN_Io_Request_TypeConnect:
    if (xio->connectRequest==r) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Aborted connect request");
      if (!(GWEN_Io_Layer_GetFlags(io) & GWEN_IO_LAYER_FLAGS_DONTCLOSE))
	GWEN_Socket_Close(xio->socket);
      GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
      GWEN_Io_LayerSocket_AbortRequests(io, GWEN_ERROR_ABORTED);
    }
    else {
      /* not my request */
      DBG_INFO(GWEN_LOGDOMAIN, "Read request not registered with this io layer");
      return GWEN_ERROR_INVALID;
    }
    break;

  default:
    break;
  }

  return 0;
}



int GWEN_Io_LayerSocket_HasWaitingRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_SOCKET *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io);
  assert(xio);

  if (xio->readRequest || xio->writeRequest || xio->connectRequest)
    return 1;
  else
    return 0;
}



int GWEN_Io_LayerSocket_AddWaitingSockets(GWEN_IO_LAYER *io,
					  GWEN_SOCKET_LIST2 *readSockets,
					  GWEN_SOCKET_LIST2 *writeSockets) {
  GWEN_IO_LAYER_SOCKET *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io);
  assert(xio);

  if (xio->readRequest || GWEN_Io_Layer_GetStatus(io)==GWEN_Io_Layer_StatusListening)
    GWEN_Socket_List2_PushBack(readSockets, xio->socket);
  if (xio->writeRequest || xio->connectRequest)
    GWEN_Socket_List2_PushBack(writeSockets, xio->socket);

  return 0;
}



int GWEN_Io_LayerSocket_Listen(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_SOCKET *xio;
  char addrBuffer[128];
  int port;
  int rv;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_SOCKET, io);
  assert(xio);

  /* get address for logging */
  GWEN_InetAddr_GetAddress(xio->localAddr, addrBuffer, sizeof(addrBuffer));
  port=GWEN_InetAddr_GetPort(xio->localAddr);

  /* open socket */
  rv=GWEN_Socket_Open(xio->socket);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  
  /* set nonblocking */
  rv=GWEN_Socket_SetBlocking(xio->socket, 0);
  if (rv) {
    GWEN_Socket_Close(xio->socket);
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* allow reuse of addresses */
  rv=GWEN_Socket_SetReuseAddress(xio->socket, 1);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }

  /* bind socket to local address */
  rv=GWEN_Socket_Bind(xio->socket, xio->localAddr);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* start listening */
  DBG_NOTICE(GWEN_LOGDOMAIN, "Starting to listen on %s (port %d)", addrBuffer, port);
  rv=GWEN_Socket_Listen(xio->socket, 10);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusListening);

  return 0;
}













