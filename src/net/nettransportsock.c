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


#include "nettransportsock_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>


GWEN_INHERIT(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSOCKET);



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORTSOCKET *GWEN_NetTransportSocketData_new(){
  GWEN_NETTRANSPORTSOCKET *skd;

  GWEN_NEW_OBJECT(GWEN_NETTRANSPORTSOCKET, skd);
  return skd;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetTransportSocketData_free(GWEN_NETTRANSPORTSOCKET *skd){
  if (skd) {
    if (skd->ownSocket)
      GWEN_Socket_free(skd->socket);
    free(skd);
  }
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT *GWEN_NetTransportSocket_new(GWEN_SOCKET *sk,
                                               int relinquish){
  GWEN_NETTRANSPORT *tr;
  GWEN_NETTRANSPORTSOCKET *skd;

  tr=GWEN_NetTransport_new();
  skd=GWEN_NetTransportSocketData_new();
  GWEN_INHERIT_SETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSOCKET,
                       tr, skd, GWEN_NetTransportSocket_FreeData);

  skd->socket=sk;
  skd->ownSocket=relinquish;

  GWEN_NetTransport_SetStartConnectFn(tr,
                                      GWEN_NetTransportSocket_StartConnect);
  GWEN_NetTransport_SetStartAcceptFn(tr,
                                     GWEN_NetTransportSocket_StartAccept);
  GWEN_NetTransport_SetStartDisconnectFn(tr,
                                         GWEN_NetTransportSocket_StartDisconnect);
  GWEN_NetTransport_SetReadFn(tr,
                              GWEN_NetTransportSocket_Read);
  GWEN_NetTransport_SetWriteFn(tr,
                               GWEN_NetTransportSocket_Write);
  GWEN_NetTransport_SetAddSocketsFn(tr,
                                    GWEN_NetTransportSocket_AddSockets);
  GWEN_NetTransport_SetWorkFn(tr,
                              GWEN_NetTransportSocket_Work);
  return tr;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetTransportSocket_FreeData(void *bp, void *p){
  GWEN_NetTransportSocketData_free((GWEN_NETTRANSPORTSOCKET*)p);
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT_RESULT
GWEN_NetTransportSocket_StartConnect(GWEN_NETTRANSPORT *tr){
  GWEN_NETTRANSPORTSOCKET *skd;
  GWEN_ERRORCODE err;
  char addrBuffer[128];

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSOCKET, tr);

  GWEN_InetAddr_GetAddress(GWEN_NetTransport_GetPeerAddr(tr),
                           addrBuffer, sizeof(addrBuffer));

  if (GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusUnconnected &&
      GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusPDisconnected){
    DBG_ERROR(0, "Socket is not unconnected (status \"%s\")",
              GWEN_NetTransport_StatusName(GWEN_NetTransport_GetStatus(tr)));
    return GWEN_NetTransportResultError;
  }

  DBG_INFO(0, "Starting to connect to %s (port %d)",
           addrBuffer,
           GWEN_InetAddr_GetPort(GWEN_NetTransport_GetPeerAddr(tr)));

  /* arm socket code */
  err=GWEN_Socket_Open(skd->socket);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* set nonblocking */
  err=GWEN_Socket_SetBlocking(skd->socket, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* actually start to connect */
  err=GWEN_Socket_Connect(skd->socket, GWEN_NetTransport_GetPeerAddr(tr));
  /* not yet finished or real error ? */
  if (!GWEN_Error_IsOk(err)) {
    if (GWEN_Error_GetType(err)!=GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
        GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_IN_PROGRESS) {
      /* real error, so return that error */
      DBG_ERROR_ERR(0, err);
      return GWEN_NetTransportResultError;
    }

    /* adjust status (physically connecting) */
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPConnecting);
  }
  else {
    /* connection succeeded */
    DBG_INFO(0, "Connection established with %s (port %d)",
             addrBuffer,
             GWEN_InetAddr_GetPort(GWEN_NetTransport_GetPeerAddr(tr)));
    /* adjust status (logically connected) */
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusLConnected);
  }
  GWEN_NetTransport_MarkActivity(tr);
  return GWEN_NetTransportResultOk;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT_RESULT
GWEN_NetTransportSocket_StartAccept(GWEN_NETTRANSPORT *tr){
  GWEN_NETTRANSPORTSOCKET *skd;
  GWEN_ERRORCODE err;
  char addrBuffer[128];

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSOCKET, tr);
  if (GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusUnconnected) {
    DBG_ERROR(0, "Socket is not unconnected (%d)",
              GWEN_NetTransport_GetStatus(tr));
    return GWEN_NetTransportResultError;
  }

  GWEN_InetAddr_GetAddress(GWEN_NetTransport_GetLocalAddr(tr),
                           addrBuffer, sizeof(addrBuffer));

  DBG_INFO(0, "Starting to listen on %s (port %d)",
           addrBuffer,
           GWEN_InetAddr_GetPort(GWEN_NetTransport_GetLocalAddr(tr)));

  /* arm socket code */
  err=GWEN_Socket_Open(skd->socket);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* set nonblocking */
  err=GWEN_Socket_SetBlocking(skd->socket, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* reuse address */
  err=GWEN_Socket_SetReuseAddress(skd->socket, 1);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* bind socket to local address */
  err=GWEN_Socket_Bind(skd->socket, GWEN_NetTransport_GetLocalAddr(tr));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* start listening */
  err=GWEN_Socket_Listen(skd->socket, 10);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* adjust status (listening) */
  GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusListening);
  GWEN_NetTransport_MarkActivity(tr);

  return GWEN_NetTransportResultOk;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT_RESULT
GWEN_NetTransportSocket_StartDisconnect(GWEN_NETTRANSPORT *tr){
  GWEN_NETTRANSPORTSOCKET *skd;
  GWEN_ERRORCODE err;
  GWEN_NETTRANSPORT_STATUS st;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSOCKET, tr);

  /* check status */
  st=GWEN_NetTransport_GetStatus(tr);
  if (st==GWEN_NetTransportStatusUnconnected ||
      st==GWEN_NetTransportStatusPDisconnected ||
      st==GWEN_NetTransportStatusDisabled) {
    DBG_ERROR(0,
              "Socket is inactive: %s (%d)",
              GWEN_NetTransport_StatusName(GWEN_NetTransport_GetStatus(tr)),
              GWEN_NetTransport_GetStatus(tr));
    return GWEN_NetTransportResultError;
  }

  /* close socket */
  err=GWEN_Socket_Close(skd->socket);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return GWEN_NetTransportResultError;
  }

  /* adjust status (PDisconnected) */
  GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusPDisconnected);
  GWEN_NetTransport_MarkActivity(tr);

  return GWEN_NetTransportResultOk;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT_RESULT
GWEN_NetTransportSocket_Read(GWEN_NETTRANSPORT *tr,
                             char *buffer,
                             int *bsize){
  GWEN_NETTRANSPORTSOCKET *skd;
  GWEN_ERRORCODE err;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSOCKET, tr);

  /* check status */
  if (GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusLConnected) {
    DBG_ERROR(0, "Socket is not connected (%d)",
              GWEN_NetTransport_GetStatus(tr));
    return GWEN_NetTransportResultError;
  }

  /* try to read */
  err=GWEN_Socket_Read(skd->socket, buffer, bsize);

  /* check result */
  if (!GWEN_Error_IsOk(err)) {
    if (GWEN_Error_GetType(err)!=
        GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
        (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
         GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
      DBG_DEBUG_ERR(0, err);
      return GWEN_NetTransportResultError;
    }
    return GWEN_NetTransportResultWantRead;
  }

  DBG_DEBUG(0, "Read %d bytes", *bsize);
  GWEN_NetTransport_MarkActivity(tr);
  return GWEN_NetTransportResultOk;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT_RESULT
GWEN_NetTransportSocket_Write(GWEN_NETTRANSPORT *tr,
                              const char *buffer,
                              int *bsize){
  GWEN_NETTRANSPORTSOCKET *skd;
  GWEN_ERRORCODE err;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSOCKET, tr);

  /* check status */
  if (GWEN_NetTransport_GetStatus(tr)!=GWEN_NetTransportStatusLConnected) {
    DBG_ERROR(0, "Socket is not connected (%d)",
              GWEN_NetTransport_GetStatus(tr));
    return GWEN_NetTransportResultError;
  }

  /* try to write */
  err=GWEN_Socket_Write(skd->socket, buffer, bsize);

  /* check result */
  if (!GWEN_Error_IsOk(err)) {
    if (GWEN_Error_GetType(err)!=
        GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
        (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
         GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
      DBG_DEBUG_ERR(0, err);
      return GWEN_NetTransportResultError;
    }
    return GWEN_NetTransportResultWantWrite;
  }
  DBG_DEBUG(0, "Written %d bytes", *bsize);
  GWEN_NetTransport_MarkActivity(tr);
  return GWEN_NetTransportResultOk;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSocket_AddSockets(GWEN_NETTRANSPORT *tr,
                                       GWEN_SOCKETSET *sset,
                                       int forReading){
  GWEN_NETTRANSPORTSOCKET *skd;
  GWEN_ERRORCODE err;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSOCKET, tr);

  /* add socket */
  err=GWEN_SocketSet_AddSocket(sset, skd->socket);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return -1;
  }
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetTransportSocket_Work(GWEN_NETTRANSPORT *tr) {
  GWEN_NETTRANSPORTSOCKET *skd;
  GWEN_NETTRANSPORT_STATUS st;
  GWEN_ERRORCODE err;

  assert(tr);
  skd=GWEN_INHERIT_GETDATA(GWEN_NETTRANSPORT, GWEN_NETTRANSPORTSOCKET, tr);

  st=GWEN_NetTransport_GetStatus(tr);
  DBG_DEBUG(0, "Working with status \"%s\" (%d)",
           GWEN_NetTransport_StatusName(st),
           st);
  switch(st) {
  case GWEN_NetTransportStatusPConnecting: {
    char addrBuffer[128];
    DBG_VERBOUS(0, "Still connecting");

    /* get socket error to check whether the connect succeeded */
    err=GWEN_Socket_GetSocketError(skd->socket);
    if (!GWEN_Error_IsOk(err)) {
      if (GWEN_Error_GetType(err)!=
          GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
          (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
           GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
        DBG_INFO_ERR(0, err);
        return -1;
      }
      DBG_VERBOUS(0, "Still not connected");
      return 0;
    }
    /* log address */
    GWEN_InetAddr_GetAddress(GWEN_NetTransport_GetPeerAddr(tr),
                             addrBuffer, sizeof(addrBuffer));
    DBG_INFO(0, "Connection established with %s (port %d)",
             addrBuffer,
             GWEN_InetAddr_GetPort(GWEN_NetTransport_GetPeerAddr(tr)));
    /* set to "logically connected" */
    GWEN_NetTransport_SetStatus(tr, GWEN_NetTransportStatusLConnected);
    DBG_INFO(0, "Connection established");
    GWEN_NetTransport_MarkActivity(tr);
    break;
  }

  case GWEN_NetTransportStatusListening: {
    GWEN_SOCKET *newS;
    GWEN_INETADDRESS *iaddr;
    GWEN_NETTRANSPORT *newTr;
    char addrBuffer[128];

    DBG_VERBOUS(0, "Listening");
    if (GWEN_NetTransport_GetIncomingCount(tr)+1<
        GWEN_NetTransport_GetBackLog(tr)) {
      newS=0;
      iaddr=0;
      /* accept new connection (if there is any) */
      err=GWEN_Socket_Accept(skd->socket, &iaddr, &newS);
      if (!GWEN_Error_IsOk(err)) {
        GWEN_InetAddr_free(iaddr);
        GWEN_Socket_free(newS);
        /* no new connection, due to an error ? */
        if (GWEN_Error_GetType(err)!=
            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
            (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
             GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
          /* jepp, there was an error */
          DBG_INFO_ERR(0, err);
          return -1;
        }
        /* otherwise there simply is no waiting connection */
        DBG_DEBUG(0, "No incoming connection");
        return 0;
      }

      /* we have an incoming connection */
      GWEN_InetAddr_GetAddress(iaddr, addrBuffer, sizeof(addrBuffer));
      DBG_INFO(0, "Incoming connection from %s (port %d)",
               addrBuffer, GWEN_InetAddr_GetPort(iaddr));

      /* set socket nonblocking */
      err=GWEN_Socket_SetBlocking(newS, 0);
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(0, err);
        GWEN_InetAddr_free(iaddr);
        GWEN_Socket_free(newS);
        return -1;
      }

      /* create new transport layer, let it take over the new socket */
      newTr=GWEN_NetTransportSocket_new(newS, 1);
      GWEN_NetTransport_SetPeerAddr(newTr, iaddr);
      GWEN_InetAddr_free(iaddr);
      GWEN_NetTransport_SetLocalAddr(newTr, GWEN_NetTransport_GetLocalAddr(tr));
      /* set status (already fully connected) */
      GWEN_NetTransport_SetStatus(newTr, GWEN_NetTransportStatusLConnected);
      /* set flags: This connection is a passive one */
      GWEN_NetTransport_SetFlags(newTr,
                                 GWEN_NetTransport_GetFlags(newTr) |
                                 GWEN_NETTRANSPORT_FLAGS_PASSIVE);

      /* add it to queue of incoming connections */
      GWEN_NetTransport_AddNextIncoming(tr, newTr);
    }
    else {
      DBG_INFO(0, "Too many incoming connections waiting");
    }
    GWEN_NetTransport_MarkActivity(tr);
    break;
  }

  case GWEN_NetTransportStatusLConnected:
    DBG_VERBOUS(0, "Active connection, nothing to do");
    break;

  case GWEN_NetTransportStatusUnconnected:
  case GWEN_NetTransportStatusDisabled:
  case GWEN_NetTransportStatusPDisconnecting:
  case GWEN_NetTransportStatusPDisconnected:
    DBG_VERBOUS(0, "Inactive connection (status \"%s\")",
                GWEN_NetTransport_StatusName(st));
    break;

  default:
    DBG_WARN(0, "Hmm, status \"%s\" (%d) is unexpected...",
             GWEN_NetTransport_StatusName(st),
             st);
    break;
  } /* switch */

  return 0;
}







