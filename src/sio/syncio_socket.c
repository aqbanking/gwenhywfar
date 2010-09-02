/***************************************************************************
 begin       : Tue Apr 27 2010
 copyright   : (C) 2010 by Martin Preuss
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



#include "syncio_socket_p.h"
#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>

#include <assert.h>
#include <errno.h>
#include <string.h>


#define GWEN_SYNCIO_SOCKET_READ_TIMEOUT  60000
#define GWEN_SYNCIO_SOCKET_WRITE_TIMEOUT 60000


GWEN_INHERIT(GWEN_SYNCIO, GWEN_SYNCIO_SOCKET)



GWEN_SYNCIO *GWEN_SyncIo_Socket_new(GWEN_SOCKETTYPE sockType, GWEN_AddressFamily addressFamily) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_SOCKET *xio;

  sio=GWEN_SyncIo_new(GWEN_SYNCIO_SOCKET_TYPE, NULL);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_SOCKET, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_SOCKET, sio, xio, GWEN_SyncIo_Socket_FreeData);

  GWEN_SyncIo_SetConnectFn(sio, GWEN_SyncIo_Socket_Connect);
  GWEN_SyncIo_SetDisconnectFn(sio, GWEN_SyncIo_Socket_Disconnect);
  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_Socket_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_Socket_Write);

  xio->socketType=sockType;
  xio->addressFamily=addressFamily;

  return sio;
}



GWEN_SYNCIO *GWEN_SyncIo_Socket_TakeOver(GWEN_SOCKET *socket) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_SOCKET *xio;

  sio=GWEN_SyncIo_new(GWEN_SYNCIO_SOCKET_TYPE, NULL);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_SOCKET, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_SOCKET, sio, xio, GWEN_SyncIo_Socket_FreeData);

  GWEN_SyncIo_SetConnectFn(sio, GWEN_SyncIo_Socket_Connect);
  GWEN_SyncIo_SetDisconnectFn(sio, GWEN_SyncIo_Socket_Disconnect);
  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_Socket_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_Socket_Write);

  xio->socketType=GWEN_Socket_GetSocketType(socket);
  xio->socket=socket;
  GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Connected);

  return sio;
}



void GWENHYWFAR_CB GWEN_SyncIo_Socket_FreeData(void *bp, void *p) {
  GWEN_SYNCIO_SOCKET *xio;

  xio=(GWEN_SYNCIO_SOCKET*) p;
  free(xio->address);
  GWEN_Socket_free(xio->socket);
  GWEN_FREE_OBJECT(xio);
}



const char *GWEN_SyncIo_Socket_GetAddress(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_SOCKET *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_SOCKET, sio);
  assert(xio);

  return xio->address;
}



void GWEN_SyncIo_Socket_SetAddress(GWEN_SYNCIO *sio, const char *s) {
  GWEN_SYNCIO_SOCKET *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_SOCKET, sio);
  assert(xio);

  free(xio->address);
  if (s) xio->address=strdup(s);
  else xio->address=NULL;
}



int GWEN_SyncIo_Socket_GetPort(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_SOCKET *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_SOCKET, sio);
  assert(xio);

  return xio->port;
}



void GWEN_SyncIo_Socket_SetPort(GWEN_SYNCIO *sio, int i) {
  GWEN_SYNCIO_SOCKET *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_SOCKET, sio);
  assert(xio);

  xio->port=i;
}



int GWENHYWFAR_CB GWEN_SyncIo_Socket_Connect(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_SOCKET *xio;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_SOCKET, sio);
  assert(xio);

  if (GWEN_SyncIo_GetStatus(sio)==GWEN_SyncIo_Status_Connected)
    /* already connected */
    return 0;

  if (GWEN_SyncIo_GetFlags(sio) & GWEN_SYNCIO_FLAGS_PASSIVE) {
    /* passive, can't connect */
    DBG_ERROR(GWEN_LOGDOMAIN, "Passive socket, can't connect");
    return GWEN_ERROR_INVALID;
  }

  sk=GWEN_Socket_new(xio->socketType);
  rv=GWEN_Socket_Open(sk);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Gui_ProgressLog2(0,
			  GWEN_LoggerLevel_Error,
			  I18N("Error setting up socket: %d"),
			  rv);
    GWEN_Socket_free(sk);
    return rv;
  }
  addr=GWEN_InetAddr_new(xio->addressFamily);

  rv=GWEN_InetAddr_SetAddress(addr, xio->address);
  if (rv<0) {
    GWEN_Gui_ProgressLog2(0,
			  GWEN_LoggerLevel_Info,
			  I18N("Resolving hostname \"%s\" ..."),
			  xio->address);
    rv=GWEN_InetAddr_SetName(addr, xio->address);
    if (rv<0) {
      GWEN_Gui_ProgressLog2(0,
			    GWEN_LoggerLevel_Error,
			    I18N("Unknown hostname \"%s\""),
			    xio->address);
      GWEN_InetAddr_free(addr);
      GWEN_Socket_free(sk);
      return rv;
    }
    else {
      char addrBuf[256];

      rv=GWEN_InetAddr_GetAddress(addr, addrBuf, sizeof(addrBuf)-1);
      addrBuf[sizeof(addrBuf)-1]=0;
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      }
      else
	GWEN_Gui_ProgressLog2(0,
			      GWEN_LoggerLevel_Info,
			      I18N("IP address is \"%s\""),
			      addrBuf);
    }
  }
  GWEN_InetAddr_SetPort(addr, xio->port);

  /* connect */
  GWEN_Gui_ProgressLog2(0,
			GWEN_LoggerLevel_Info,
			I18N("Connecting to \"%s\""),
			xio->address);
  rv=GWEN_Socket_Connect(sk, addr);
  if (rv<0) {
    GWEN_Gui_ProgressLog2(0,
			  GWEN_LoggerLevel_Error,
			  I18N("Error connecting to \"%s\": %s"),
			  xio->address,
			  strerror(errno));
    GWEN_InetAddr_free(addr);
    GWEN_Socket_free(sk);
    return rv;
  }

  /* done */
  xio->socket=sk;
  GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Connected);
  DBG_INFO(GWEN_LOGDOMAIN, "Connected to \"%s\"", xio->address);
  GWEN_Gui_ProgressLog2(0,
			GWEN_LoggerLevel_Notice,
			I18N("Connected to \"%s\""),
			xio->address);
  return 0;
}



int GWENHYWFAR_CB GWEN_SyncIo_Socket_Disconnect(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_SOCKET *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_SOCKET, sio);
  assert(xio);

  if (xio->socket) {
    GWEN_Socket_Close(xio->socket);
    GWEN_Socket_free(xio->socket);
    xio->socket=NULL;
    GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
    DBG_INFO(GWEN_LOGDOMAIN, "Disconnected socket");
  }

  return 0;
}



int GWENHYWFAR_CB GWEN_SyncIo_Socket_Read(GWEN_SYNCIO *sio,
					  uint8_t *buffer,
					  uint32_t size) {
  GWEN_SYNCIO_SOCKET *xio;
  int rv;
  int i;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_SOCKET, sio);
  assert(xio);

  if (size==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Read size is zero");
    return GWEN_ERROR_INVALID;
  }

  if (GWEN_SyncIo_GetStatus(sio)!=GWEN_SyncIo_Status_Connected) {
    /* not connected */
    DBG_ERROR(GWEN_LOGDOMAIN, "Socket not connected (%d)",
	      GWEN_SyncIo_GetStatus(sio));
    return GWEN_ERROR_NOT_CONNECTED;
  }

  if (xio->socket==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No socket");
    return GWEN_ERROR_INTERNAL;
  }

  /* check whether there is data available */
  do {
    rv=GWEN_Socket_WaitForRead(xio->socket, 0);
  } while (rv==GWEN_ERROR_INTERRUPTED);


  /* nothing to read immediately, wait for data availability */
  if (rv==GWEN_ERROR_TIMEOUT) {
    GWEN_SOCKET_LIST2 *sl;

    sl=GWEN_Socket_List2_new();
    GWEN_Socket_List2_PushBack(sl, xio->socket);

    do {
      rv=GWEN_Gui_WaitForSockets(sl, NULL, 0, GWEN_SYNCIO_SOCKET_READ_TIMEOUT);
    } while (rv==GWEN_ERROR_INTERRUPTED);

    GWEN_Socket_List2_free(sl);
    if (rv<0) {
      if (rv==GWEN_ERROR_TIMEOUT) {
        DBG_ERROR(GWEN_LOGDOMAIN, "timeout (%d)", rv);
        return rv;
      }
      else {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
  }

  i=size;
  rv=GWEN_Socket_Read(xio->socket, (char*) buffer, &i);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

#if 0
  DBG_ERROR(0, "Received this:");
  GWEN_Text_DumpString((const char*) buffer, i, stderr, 2);
#endif

  return i;
}



int GWENHYWFAR_CB GWEN_SyncIo_Socket_Write(GWEN_SYNCIO *sio,
					   const uint8_t *buffer,
					   uint32_t size) {
  GWEN_SYNCIO_SOCKET *xio;
  int rv;
  int i;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_SOCKET, sio);
  assert(xio);

  if (GWEN_SyncIo_GetStatus(sio)!=GWEN_SyncIo_Status_Connected) {
    /* not connected */
    DBG_ERROR(GWEN_LOGDOMAIN, "Socket not connected");
    return GWEN_ERROR_NOT_CONNECTED;
  }

  if (xio->socket==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No socket");
    return GWEN_ERROR_INTERNAL;
  }

  /* check whether the socket is writeable */
  do {
    rv=GWEN_Socket_WaitForWrite(xio->socket, 0);
  } while (rv==GWEN_ERROR_INTERRUPTED);

  if (rv==GWEN_ERROR_TIMEOUT) {
    int rv;
    GWEN_SOCKET_LIST2 *sl;

    sl=GWEN_Socket_List2_new();
    GWEN_Socket_List2_PushBack(sl, xio->socket);

    do {
      rv=GWEN_Gui_WaitForSockets(NULL, sl, 0, GWEN_SYNCIO_SOCKET_WRITE_TIMEOUT);
    } while (rv==GWEN_ERROR_INTERRUPTED);

    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Socket_List2_free(sl);
      return rv;
    }
    GWEN_Socket_List2_free(sl);
  }

  i=size;
  rv=GWEN_Socket_Write(xio->socket, (const char*) buffer, &i);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

#if 0
  DBG_ERROR(0, "Written this:");
  GWEN_Text_DumpString((const char*) buffer, i, stderr, 2);
#endif

  return i;
}





