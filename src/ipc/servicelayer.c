/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Sep 16 2003
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

#include "servicelayer_p.h"
#include <gwenhyfwar/connlayer.h>
#include <gwenhyfwar/ipc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/misc.h>


static GWEN_GLOBAL_SERVICELAYER *GWEN_Global_ServiceLayer=0;



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_GlobalServiceLayer_Work(int timeout){
  unsigned int ndone;
  unsigned int socks;
  GWEN_IPCCONNLAYER *curr;
  GWEN_ERRORCODE err;

  ndone=0;
  socks=0;
  curr=GWEN_Global_ServiceLayer->connections;
  if (!curr) {
    DBG_WARN(0, "No connection layers.");
  }
  else {
    GWEN_SOCKETSET *rset, *wset;

    rset=GWEN_SocketSet_new();
    wset=GWEN_SocketSet_new();

    /* This loop samples all sockets for a "select" call. If a connection
     * layer does not provide a socket then its work function is called.*/
    while(curr) {
      GWEN_IPCMSGLAYER *ml;
      GWEN_IPCTRANSPORTLAYER *tl;
      GWEN_IPCMSGLAYER_STATE state;

      ml=GWEN_ConnectionLayer_GetMsgLayer(curr);
      assert(ml);
      tl=GWEN_MsgLayer_GetTransportLayer(ml);
      state=GWEN_MsgLayer_GetState(ml);

      if (state!=GWEN_IPCMsglayerStateClosed) {
        /* check for read sockets */
        if (state==GWEN_IPCMsglayerStateReading ||
            state==GWEN_IPCMsglayerStateListening ||
            state==GWEN_IPCMsglayerStateIdle) {
          GWEN_SOCKET *sock;

          sock=GWEN_IPCTransportLayer_GetReadSocket(tl);
          if (sock) {
            DBG_VERBOUS(0, "Adding a socket to the read set");
            err=GWEN_SocketSet_AddSocket(rset, sock);
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "Called from here");
            }
            else
              socks++;
          } /* if sock */
          else {
            DBG_VERBOUS(0, "No read socket, doing read work");
            err=GWEN_ConnectionLayer_Work(curr, 1);
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "Called from here");
            }
            else
              ndone++;
          } /* if no socket */
        } /* if readable state */
        else if (state==GWEN_IPCMsglayerStateWriting ||
                 state==GWEN_IPCMsglayerStateConnecting) {
          GWEN_SOCKET *sock;

          sock=GWEN_IPCTransportLayer_GetWriteSocket(tl);
          if (sock) {
            DBG_VERBOUS(0, "Adding a socket to the write set");
            err=GWEN_SocketSet_AddSocket(wset, sock);
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "Called from here");
            }
            else
              socks++;
          } /* if sock */
          else {
            DBG_VERBOUS(0, "No write socket, doing read work");
            err=GWEN_ConnectionLayer_Work(curr, 0);
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "Called from here");
            }
            else
              ndone++;
          } /* if no socket */
        } /* if writeable state */
      } /* if not closed */
      curr=curr->next;
    } /* while */

    if (!socks) {
      if (ndone) {
        /* no socket but something done, so sleep to reduce CPU usage */
        DBG_VERBOUS(0, "Sleeping");
        GWEN_Socket_Select(0, 0, 0, timeout);
        GWEN_SocketSet_free(rset);
        GWEN_SocketSet_free(wset);
        return 0;
      }
      else {
        /* nothing done, so simply return an error */
        GWEN_SocketSet_free(rset);
        GWEN_SocketSet_free(wset);
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                              GWEN_IPC_ERROR_NO_CONNECTIONS);
      }
    } /* if no socks */

    DBG_VERBOUS(0, "Selecting");
    err=GWEN_Socket_Select(rset, wset, 0, timeout);
    if (!GWEN_Error_IsOk(err)) {
      if (GWEN_Error_GetType(err)!=
          GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
          GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT) {
        DBG_DEBUG_ERR(0, err);
        GWEN_SocketSet_free(rset);
        GWEN_SocketSet_free(wset);
        return err;
      }
    }

    /* now check all connection layers whose sockets changed */
    curr=GWEN_Global_ServiceLayer->connections;
    while (curr) {
      GWEN_IPCMSGLAYER *ml;
      GWEN_IPCTRANSPORTLAYER *tl;
      GWEN_IPCMSGLAYER_STATE state;
      GWEN_SOCKET *sock;

      ml=GWEN_ConnectionLayer_GetMsgLayer(curr);
      assert(ml);
      tl=GWEN_MsgLayer_GetTransportLayer(ml);
      assert(tl);
      state=GWEN_MsgLayer_GetState(ml);

      switch(state) {
      case GWEN_IPCMsglayerStateListening:
        sock=GWEN_IPCTransportLayer_GetReadSocket(tl);
        if (sock) {
          if (GWEN_SocketSet_HasSocket(rset, sock)) {
            /* listening socket is readable, this means there is a
             * new connection waiting */
            GWEN_IPCCONNLAYER *newconn;

            /* accept connection */
            err=GWEN_ConnectionLayer_Accept(curr, &newconn);
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "Called from here");
            }
            else {
              /* add connection */
              err=GWEN_GlobalServiceLayer_AddConnection(newconn);
              if (!GWEN_Error_IsOk(err)) {
                DBG_INFO(0, "Called from here");
              }
            }
          } /* if socket is readable */
        } /* if sock */
        break;

      case GWEN_IPCMsglayerStateIdle:
        /* TODO: timeout for idle sockets */
      case GWEN_IPCMsglayerStateReading:
        sock=GWEN_IPCTransportLayer_GetReadSocket(tl);
        if (sock) {
          if (GWEN_SocketSet_HasSocket(rset, sock)) {
            err=GWEN_ConnectionLayer_Work(curr, 1);
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "Called from here");
            }
          } /* if socket is readable */
        } /* if sock */
        break;

      case GWEN_IPCMsglayerStateConnecting:
        sock=GWEN_IPCTransportLayer_GetWriteSocket(tl);
        if (sock) {
          if (GWEN_SocketSet_HasSocket(wset, sock)) {
            err=GWEN_ConnectionLayer_Work(curr, 0);
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "Called from here");
            }
          } /* if socket is writeable */
        } /* if sock */
        break;

      case GWEN_IPCMsglayerStateWaiting:
        /* TODO: timeout for waiting sockets */

      default:
        DBG_INFO(0, "Unhandled state %d", state);
      } /* switch */

      /* check next connection layer */
      curr=curr->next;
    } /* while curr */
    GWEN_SocketSet_free(rset);
    GWEN_SocketSet_free(wset);
  } /* if there are connection layers */

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_GlobalServiceLayer_AddConnection(GWEN_IPCCONNLAYER *conn){
  assert(conn);
  GWEN_LIST_ADD(GWEN_IPCCONNLAYER,conn,
                &(GWEN_Global_ServiceLayer->connections));
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYER*
GWEN_GlobalServiceLayer_FindConnection(unsigned int id,
                                       unsigned int libMark,
                                       unsigned int userMark){
  GWEN_IPCCONNLAYER *curr;

  curr=GWEN_Global_ServiceLayer->connections;
  while(curr) {
    if (id==0 || id==GWEN_ConnectionLayer_GetId(curr))
      if (libMark==0 || libMark==GWEN_ConnectionLayer_GetLibMark(curr))
        if (userMark==0 || userMark==GWEN_ConnectionLayer_GetUserMark(curr))
          return curr;
    curr=curr->next;
  } /* while */

  DBG_VERBOUS(0, "No connection found for %d/%d/%d",
              id, libMark, userMark);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_GlobalServiceLayer_CheckClosed() {
  GWEN_IPCCONNLAYER *cl;
  GWEN_IPCMSGLAYER *ml;
  GWEN_ERRORCODE err;

  cl=GWEN_Global_ServiceLayer->connections;
  while(cl) {
    GWEN_IPCCONNLAYER *nextcl;

    nextcl=cl->next;
    ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
    assert(ml);
    if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateClosed) {
      if (GWEN_ConnectionLayer_GetState(cl)==
          GWEN_IPCConnectionLayerStateOpen) {
        /* force closing of connection */
        DBG_NOTICE(0, "Forcing logical close of connection (%s)",
                   GWEN_ConnectionLayer_GetInfo(cl));
        err=GWEN_ConnectionLayer_Close(cl, 1);
        if (!GWEN_Error_IsOk(err)) {
          DBG_INFO(0, "Called from here");
        }
      } /* if normally open */
    }
    cl=nextcl;
  } /* while */
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_GlobalServiceLayer_RemoveClosed() {
  GWEN_IPCCONNLAYER *cl;
  GWEN_IPCMSGLAYER *ml;

  cl=GWEN_Global_ServiceLayer->connections;
  while(cl) {
    GWEN_IPCCONNLAYER *nextcl;

    nextcl=cl->next;
    ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
    assert(ml);
    if (GWEN_ConnectionLayer_GetState(cl)==
        GWEN_IPCConnectionLayerStateClosed)
      if (GWEN_ConnectionLayer_GetFlags(cl) &
          GWEN_IPCCONNLAYER_FLAGS_PERSISTENT) {
        /* remove closed connection if it is not marked persistent */
        DBG_NOTICE(0, "Removing connection (%s)",
                   GWEN_ConnectionLayer_GetInfo(cl));
        GWEN_LIST_DEL(GWEN_IPCCONNLAYER,
                      cl,
                      &(GWEN_Global_ServiceLayer->connections));
      }
    cl=nextcl;
  } /* while */
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_GLOBAL_SERVICELAYER *GWEN_GlobalServiceLayer_new(){
  GWEN_GLOBAL_SERVICELAYER *gsl;

  GWEN_NEW_OBJECT(GWEN_GLOBAL_SERVICELAYER, gsl);
  return gsl;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_GlobalServiceLayer_free(GWEN_GLOBAL_SERVICELAYER *gsl){
  if (gsl) {
    GWEN_IPCCONNLAYER *cl;

    /* free all connection layers */
    cl=gsl->connections;
    while(cl) {
      GWEN_IPCCONNLAYER *nextcl;

      nextcl=cl->next;
      GWEN_ConnectionLayer_free(cl);
      cl=nextcl;
    } /* while */
    free(gsl);
  }
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_GlobalServiceLayer_Close(unsigned int id,
                                   unsigned int libMark,
                                   unsigned int userMark,
                                   int force){
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;

  assert(GWEN_Global_ServiceLayer);
  cl=GWEN_Global_ServiceLayer->connections;
  while(cl) {
    if (id==0 || id==GWEN_ConnectionLayer_GetId(cl))
      if (libMark==0 || libMark==GWEN_ConnectionLayer_GetLibMark(cl))
        if (userMark==0 || userMark==GWEN_ConnectionLayer_GetUserMark(cl)){
          err=GWEN_ConnectionLayer_Close(cl, force);
          if (!GWEN_Error_IsOk(err)) {
            DBG_INFO(0, "Called from here");
          }
        }
    cl=cl->next;
  } /* while */
}





