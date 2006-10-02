/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Feb 09 2004
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


#include "nl_socket_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/text.h>

#if 0 /* disabled, doesn't work on WIN32 (no poll) */
#include "inetsocket_l.h" /* for GWEN_Socket_GetSocketInt() */
#include <poll.h>
#include <string.h>
#include <errno.h>
#endif

GWEN_INHERIT(GWEN_NETLAYER, GWEN_NL_SOCKET)


/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER *GWEN_NetLayerSocket_new(GWEN_SOCKET *sk, int takeOver) {
  GWEN_NETLAYER *nl;
  GWEN_NL_SOCKET *nld;

  nl=GWEN_NetLayer_new(GWEN_NL_SOCKET_NAME);
  GWEN_NEW_OBJECT(GWEN_NL_SOCKET, nld)
  DBG_MEM_INC("GWEN_NL_SOCKET", 0);
  nld->socket=sk;
  nld->ownSocket=takeOver;

  GWEN_INHERIT_SETDATA(GWEN_NETLAYER, GWEN_NL_SOCKET, nl, nld,
                       GWEN_NetLayerSocket_FreeData);

  GWEN_NetLayer_SetWorkFn(nl, GWEN_NetLayerSocket_Work);
  GWEN_NetLayer_SetReadFn(nl, GWEN_NetLayerSocket_Read);
  GWEN_NetLayer_SetWriteFn(nl, GWEN_NetLayerSocket_Write);
  GWEN_NetLayer_SetConnectFn(nl, GWEN_NetLayerSocket_Connect);
  GWEN_NetLayer_SetDisconnectFn(nl, GWEN_NetLayerSocket_Disconnect);
  GWEN_NetLayer_SetListenFn(nl, GWEN_NetLayerSocket_Listen);
  GWEN_NetLayer_SetAddSocketsFn(nl, GWEN_NetLayerSocket_AddSockets);

  return nl;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayerSocket_FreeData(void *bp, void *p) {
  GWEN_NL_SOCKET *nld;

  nld=(GWEN_NL_SOCKET*)p;
  if (nld->ownSocket)
    GWEN_Socket_free(nld->socket);
  DBG_MEM_DEC("GWEN_NL_SOCKET");
  GWEN_FREE_OBJECT(nld);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSocket_Connect(GWEN_NETLAYER *nl){
  GWEN_NL_SOCKET *nld;
  GWEN_ERRORCODE err;
  char addrBuffer[128];

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SOCKET, nl);
  assert(nld);

  GWEN_InetAddr_GetAddress(GWEN_NetLayer_GetPeerAddr(nl),
                           addrBuffer, sizeof(addrBuffer));

  if (GWEN_NetLayer_GetStatus(nl)!=GWEN_NetLayerStatus_Unconnected &&
      GWEN_NetLayer_GetStatus(nl)!=GWEN_NetLayerStatus_Disconnected){
    DBG_INFO(GWEN_LOGDOMAIN, "Socket is not unconnected (status \"%s\")",
              GWEN_NetLayerStatus_toString(GWEN_NetLayer_GetStatus(nl)));
    return GWEN_ERROR_INVALID;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Starting to connect to %s (port %d)",
           addrBuffer,
           GWEN_InetAddr_GetPort(GWEN_NetLayer_GetPeerAddr(nl)));

  GWEN_NetLayer_SubFlags(nl,
                         GWEN_NETLAYER_FLAGS_EOFMET |
                         GWEN_NETLAYER_FLAGS_BROKENPIPE |
                         GWEN_NETLAYER_FLAGS_WANTREAD |
                         GWEN_NETLAYER_FLAGS_WANTWRITE);

  /* arm socket code */
  err=GWEN_Socket_Open(nld->socket);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_Error_GetSimpleCode(err);
  }

  /* set nonblocking */
  err=GWEN_Socket_SetBlocking(nld->socket, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_Error_GetSimpleCode(err);
  }

  /* actually start to connect */
  err=GWEN_Socket_Connect(nld->socket, GWEN_NetLayer_GetPeerAddr(nl));
  /* not yet finished or real error ? */
  if (!GWEN_Error_IsOk(err)) {
    if (GWEN_Error_GetType(err)!=GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE)
        ||
        GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_IN_PROGRESS) {
      /* real error, so return that error */
      DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
      return GWEN_Error_GetSimpleCode(err);
    }

    /* adjust status (physically connecting) */
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Connecting);
    GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_WANTWRITE);
    return 1;
  }
  else {
    /* connection succeeded */
    DBG_INFO(GWEN_LOGDOMAIN, "Connection established with %s (port %d)",
             addrBuffer,
             GWEN_InetAddr_GetPort(GWEN_NetLayer_GetPeerAddr(nl)));
    /* adjust status */
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Connected);
    GWEN_NetLayer_SubFlags(nl, GWEN_NETLAYER_FLAGS_WANTWRITE);
  }
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSocket_Listen(GWEN_NETLAYER *nl){
  GWEN_NL_SOCKET *nld;
  GWEN_ERRORCODE err;
  char addrBuffer[128];

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SOCKET, nl);
  assert(nld);

  if (GWEN_NetLayer_GetStatus(nl)!=GWEN_NetLayerStatus_Unconnected) {
    DBG_INFO(GWEN_LOGDOMAIN, "Socket is not unconnected (%d)",
              GWEN_NetLayer_GetStatus(nl));
    return GWEN_ERROR_INVALID;
  }

  GWEN_InetAddr_GetAddress(GWEN_NetLayer_GetLocalAddr(nl),
                           addrBuffer, sizeof(addrBuffer));

  DBG_INFO(GWEN_LOGDOMAIN, "Starting to listen on %s (port %d)",
           addrBuffer,
           GWEN_InetAddr_GetPort(GWEN_NetLayer_GetLocalAddr(nl)));

  GWEN_NetLayer_SubFlags(nl,
                         GWEN_NETLAYER_FLAGS_EOFMET |
                         GWEN_NETLAYER_FLAGS_BROKENPIPE |
                         GWEN_NETLAYER_FLAGS_WANTREAD |
                         GWEN_NETLAYER_FLAGS_WANTWRITE);

  /* arm socket code */
  err=GWEN_Socket_Open(nld->socket);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_Error_GetSimpleCode(err);
  }

  /* set nonblocking */
  err=GWEN_Socket_SetBlocking(nld->socket, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_Error_GetSimpleCode(err);
  }

  /* reuse address */
  err=GWEN_Socket_SetReuseAddress(nld->socket, 1);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_Error_GetSimpleCode(err);
  }

  /* bind socket to local address */
  err=GWEN_Socket_Bind(nld->socket, GWEN_NetLayer_GetLocalAddr(nl));
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_Error_GetSimpleCode(err);
  }

  /* start listening */
  err=GWEN_Socket_Listen(nld->socket, 10);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_Error_GetSimpleCode(err);
  }

  /* adjust status (listening) */
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Listening);
  GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_PASSIVE);
  GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_WANTREAD);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSocket_Disconnect(GWEN_NETLAYER *nl){
  GWEN_NL_SOCKET *nld;
  GWEN_ERRORCODE err;
#if 0
  GWEN_NETLAYER_STATUS st;
#endif

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SOCKET, nl);
  assert(nld);

#if 0
  /* check status */
  st=GWEN_NetLayer_GetStatus(nl);
  if (st==GWEN_NetLayerStatus_Unconnected ||
      st==GWEN_NetLayerStatus_Disconnected ||
      st==GWEN_NetLayerStatus_Disabled) {
    DBG_INFO(GWEN_LOGDOMAIN,
              "Socket is inactive: %s (%d)",
              GWEN_NetLayerStatus_toString(st), st);
    return GWEN_ERROR_INVALID;
  }
#endif

  /* close socket */
  err=GWEN_Socket_Close(nld->socket);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    /*return GWEN_Error_GetSimpleCode(err); */
  }

  /* adjust status (Disconnected) */
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
  GWEN_NetLayer_SubFlags(nl,
                         GWEN_NETLAYER_FLAGS_EOFMET |
                         GWEN_NETLAYER_FLAGS_BROKENPIPE |
                         GWEN_NETLAYER_FLAGS_WANTREAD |
                         GWEN_NETLAYER_FLAGS_WANTWRITE);

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSocket_Read(GWEN_NETLAYER *nl,
                             char *buffer,
                             int *bsize){
  GWEN_NL_SOCKET *nld;
  GWEN_ERRORCODE err;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Reading %d bytes", *bsize);

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SOCKET, nl);
  assert(nld);

  /* check status */
  if (GWEN_NetLayer_GetStatus(nl)!=GWEN_NetLayerStatus_Connected) {
    DBG_INFO(GWEN_LOGDOMAIN, "Socket is not connected (%d)",
              GWEN_NetLayer_GetStatus(nl));
    return GWEN_ERROR_INVALID;
  }

  if (GWEN_NetLayer_GetFlags(nl) & GWEN_NETLAYER_FLAGS_EOFMET) {
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
    return GWEN_ERROR_EOF;
  }

  /* try to read */
  err=GWEN_Socket_Read(nld->socket, buffer, bsize);


  /* check result */
  if (!GWEN_Error_IsOk(err)) {
    if (GWEN_Error_GetType(err)!=
        GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
        (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
         GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
      DBG_DEBUG_ERR(GWEN_LOGDOMAIN, err);
      return GWEN_Error_GetSimpleCode(err);
    }
    GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_WANTREAD);
    return 1;
  }

  if (*bsize==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
    GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_EOFMET);
  }
  else {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Read %d bytes", *bsize);
    GWEN_Text_LogString(buffer, *bsize, GWEN_LOGDOMAIN,
                        GWEN_LoggerLevel_Verbous);
  }
  GWEN_NetLayer_SubFlags(nl, GWEN_NETLAYER_FLAGS_WANTREAD);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSocket_Write(GWEN_NETLAYER *nl,
                              const char *buffer,
                              int *bsize){
  GWEN_NL_SOCKET *nld;
  GWEN_ERRORCODE err;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SOCKET, nl);
  assert(nld);

  /* check status */
  if (GWEN_NetLayer_GetStatus(nl)!=GWEN_NetLayerStatus_Connected) {
    DBG_INFO(GWEN_LOGDOMAIN, "Socket is not connected (%d)",
              GWEN_NetLayer_GetStatus(nl));
    return GWEN_ERROR_INVALID;
  }

  /* try to write */
  err=GWEN_Socket_Write(nld->socket, buffer, bsize);

  /* check result */
  if (!GWEN_Error_IsOk(err)) {
    if (GWEN_Error_GetType(err)!=
        GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
        (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
         GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
      DBG_DEBUG_ERR(GWEN_LOGDOMAIN, err);
      return GWEN_Error_GetSimpleCode(err);
    }
    GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_WANTWRITE);
    return 1;
  }
  DBG_DEBUG(GWEN_LOGDOMAIN, "Written %d bytes", *bsize);
  GWEN_Text_LogString(buffer, *bsize, 0, GWEN_LoggerLevel_Verbous);
  GWEN_NetLayer_SubFlags(nl, GWEN_NETLAYER_FLAGS_WANTWRITE);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSocket_AddSockets(GWEN_NETLAYER *nl,
                                   GWEN_SOCKETSET *readSet,
                                   GWEN_SOCKETSET *writeSet,
                                   GWEN_SOCKETSET *exSet) {
  GWEN_NL_SOCKET *nld;
  GWEN_ERRORCODE err;
  GWEN_TYPE_UINT32 flags;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SOCKET, nl);
  assert(nld);

  flags=GWEN_NetLayer_GetFlags(nl);

  if (flags & GWEN_NETLAYER_FLAGS_WANTREAD) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Adding socket to readSet");
    err=GWEN_SocketSet_AddSocket(readSet, nld->socket);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
      return GWEN_Error_GetSimpleCode(err);
    }
  }

  if (flags & GWEN_NETLAYER_FLAGS_WANTWRITE) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Adding socket to writeSet");
    err=GWEN_SocketSet_AddSocket(writeSet, nld->socket);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
      return GWEN_Error_GetSimpleCode(err);
    }
  }

  return 0;
}



#if 0
/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerSocket_CheckConnection(GWEN_NETLAYER *nl) {
  GWEN_NL_SOCKET *nld;
  struct pollfd fds;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SOCKET, nl);
  assert(nld);

  fds.fd=GWEN_Socket_GetSocketInt(nld->socket);
  fds.events=POLLHUP;
  rv=poll(&fds, 1, 0); /* return immediately */
  if (rv>0) {
    if (fds.revents & POLLHUP) {
      DBG_INFO(GWEN_LOGDOMAIN, "Connection closed by peer");
      GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
      return 0;
    }
    else if (fds.revents & POLLERR) {
      DBG_INFO(GWEN_LOGDOMAIN, "Connection error");
      GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
      return -1;
    }
  }
  else if (rv<0) {
    if (errno!=EAGAIN && errno!=EINTR) {
      DBG_INFO(GWEN_LOGDOMAIN, "poll(): %s",
               strerror(errno));
      GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
      return -1;
    }
  }

  return 0;
}
#endif


/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayerSocket_Work(GWEN_NETLAYER *nl) {
  GWEN_NL_SOCKET *nld;
  GWEN_ERRORCODE err;
  GWEN_NETLAYER_STATUS st;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_SOCKET, nl);
  assert(nld);

  st=GWEN_NetLayer_GetStatus(nl);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Working with status \"%s\" (%d)",
              GWEN_NetLayerStatus_toString(st), st);

  switch(st) {

  case GWEN_NetLayerStatus_Connecting: {
    char addrBuffer[128];
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Still connecting");

    /* get socket error to check whether the connect succeeded */
    err=GWEN_Socket_GetSocketError(nld->socket);
    if (!GWEN_Error_IsOk(err)) {
      if (GWEN_Error_GetType(err)!=
          GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
          (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
           GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
        DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
        GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
        return GWEN_NetLayerResult_Error;
      }
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Still not connected");
      GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_WANTWRITE);
      return GWEN_NetLayerResult_WouldBlock;
    }
    /* log address */
    GWEN_InetAddr_GetAddress(GWEN_NetLayer_GetPeerAddr(nl),
                             addrBuffer, sizeof(addrBuffer));
    DBG_INFO(GWEN_LOGDOMAIN, "Connection established with %s (port %d)",
             addrBuffer,
             GWEN_InetAddr_GetPort(GWEN_NetLayer_GetPeerAddr(nl)));
    /* set to "logically connected" */
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Connected);
    DBG_INFO(GWEN_LOGDOMAIN, "Connection established");
    GWEN_NetLayer_SubFlags(nl, GWEN_NETLAYER_FLAGS_WANTWRITE);
    return GWEN_NetLayerResult_Changed;
    break;
  }

  case GWEN_NetLayerStatus_Listening: {
    GWEN_SOCKET *newS;
    GWEN_INETADDRESS *iaddr;
    GWEN_NETLAYER *newLayer;
    char addrBuffer[128];

    DBG_VERBOUS(GWEN_LOGDOMAIN, "Listening");
    GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_WANTREAD);
    if (GWEN_NetLayer_GetIncomingLayerCount(nl)+1<
        GWEN_NetLayer_GetBackLog(nl)) {
      newS=0;
      iaddr=0;
      /* accept new connection (if there is any) */
      err=GWEN_Socket_Accept(nld->socket, &iaddr, &newS);
      if (!GWEN_Error_IsOk(err)) {
        GWEN_InetAddr_free(iaddr);
        GWEN_Socket_free(newS);
        /* no new connection, due to an error ? */
        if (GWEN_Error_GetType(err)!=
            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
            (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
             GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
          /* jepp, there was an error */
          DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
          GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
          return GWEN_NetLayerResult_Error;
        }
        /* otherwise there simply is no waiting connection */
        DBG_DEBUG(GWEN_LOGDOMAIN, "No incoming connection");
        return GWEN_NetLayerResult_WouldBlock;
      }

      /* we have an incoming connection */
      GWEN_InetAddr_GetAddress(iaddr, addrBuffer, sizeof(addrBuffer));
      DBG_INFO(GWEN_LOGDOMAIN, "Incoming connection from %s (port %d)",
               addrBuffer, GWEN_InetAddr_GetPort(iaddr));

      /* set socket nonblocking */
      err=GWEN_Socket_SetBlocking(newS, 0);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
        GWEN_InetAddr_free(iaddr);
        GWEN_Socket_free(newS);
        GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
        return GWEN_NetLayerResult_Error;
      }

      /* create new transport layer, let it take over the new socket */
      newLayer=GWEN_NetLayerSocket_new(newS, 1);
      GWEN_NetLayer_SetPeerAddr(newLayer, iaddr);
      GWEN_InetAddr_free(iaddr);
      GWEN_NetLayer_SetLocalAddr(newLayer, GWEN_NetLayer_GetLocalAddr(nl));
      /* set status (already fully connected) */
      GWEN_NetLayer_SetStatus(newLayer, GWEN_NetLayerStatus_Connected);
      /* set flags: This connection is a passive one */
      GWEN_NetLayer_AddFlags(newLayer, GWEN_NETLAYER_FLAGS_PASSIVE);

      /* add it to queue of incoming connections */
      GWEN_NetLayer_AddIncomingLayer(nl, newLayer);
      return GWEN_NetLayerResult_Changed;
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "Too many incoming connections waiting");
    }
    break;
  }

  case GWEN_NetLayerStatus_Connected:
    break;

  case GWEN_NetLayerStatus_Unconnected:
  case GWEN_NetLayerStatus_Disabled:
  case GWEN_NetLayerStatus_Disconnecting:
  case GWEN_NetLayerStatus_Disconnected:
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Inactive connection (status \"%s\")",
                GWEN_NetLayerStatus_toString(st));
    break;

  default:
    DBG_WARN(GWEN_LOGDOMAIN, "Hmm, status \"%s\" (%d) is unexpected...",
             GWEN_NetLayerStatus_toString(st), st);
    break;
  } /* switch */

  return GWEN_NetLayerResult_Idle;
}







