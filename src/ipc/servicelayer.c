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
static unsigned int GWEN_Global_ServiceLayer_LastId=0;


/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_GlobalServiceLayer_Work(int timeout){
  unsigned int ndone;
  unsigned int socks;
  GWEN_IPCCONNLAYER *curr;
  GWEN_ERRORCODE err;

  DBG_NOTICE(0, "ServiceLayer is working");
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

      DBG_INFO(0, "Checking connection %d",
               GWEN_ConnectionLayer_GetId(curr));
      ml=GWEN_ConnectionLayer_GetMsgLayer(curr);
      assert(ml);
      tl=GWEN_MsgLayer_GetTransportLayer(ml);
      state=GWEN_MsgLayer_GetState(ml);

      DBG_INFO(0, "MsgLayerState=\"%s\" (%d), msgs=%d",
               GWEN_MsgLayer_GetStateString(state),
               state,
               GWEN_MsgLayer_OutgoingMsgs(ml));

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

      if (state==GWEN_IPCMsglayerStateWriting ||
          state==GWEN_IPCMsglayerStateConnecting ||
          (state==GWEN_IPCMsglayerStateIdle &&
           GWEN_MsgLayer_OutgoingMsgs(ml))) {
        GWEN_SOCKET *sock;

        DBG_INFO(0, "Checking for writing...");
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
      curr=curr->next;
    } /* while */

    if (!socks) {
      if (ndone) {
        /* no socket but we did something, so sleep to reduce CPU usage */
        if (timeout) {
          /* well, only sleep if the caller wanted a timeout */
          DBG_VERBOUS(0, "Sleeping");
          GWEN_Socket_Select(0, 0, 0, GWEN_SERVICELAYER_CPU_TIMEOUT);
        }
	GWEN_SocketSet_free(rset);
	GWEN_SocketSet_free(wset);
	return 0;
      }
      else {
        /* nothing done, so simply return an error */
        GWEN_SocketSet_free(rset);
        GWEN_SocketSet_free(wset);
        DBG_DEBUG(0, "nothing done, no connection");
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

      if (state==GWEN_IPCMsglayerStateListening) {
        sock=GWEN_IPCTransportLayer_GetReadSocket(tl);
        if (sock) {
          if (GWEN_SocketSet_HasSocket(rset, sock)) {
            /* listening socket is readable, this means there is a
             * new connection waiting */
            GWEN_IPCCONNLAYER *newconn;

            DBG_INFO(0, "Listening connection %d is readable",
                     GWEN_ConnectionLayer_GetId(curr));
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
      }

      if (state==GWEN_IPCMsglayerStateIdle ||
          state==GWEN_IPCMsglayerStateWriting) {
        sock=GWEN_IPCTransportLayer_GetWriteSocket(tl);
        if (sock) {
          if (GWEN_SocketSet_HasSocket(wset, sock)) {
            DBG_INFO(0, "Idle/writing connection %d is writeable",
                     GWEN_ConnectionLayer_GetId(curr));
            err=GWEN_ConnectionLayer_Work(curr, 0);
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "Called from here");
            }
          } /* if socket is writeable */
        } /* if sock */
      }

      if (state==GWEN_IPCMsglayerStateIdle ||
          state==GWEN_IPCMsglayerStateReading) {
        sock=GWEN_IPCTransportLayer_GetReadSocket(tl);
        if (sock) {
          if (GWEN_SocketSet_HasSocket(rset, sock)) {
            DBG_INFO(0, "Idle/Reading connection %d is readable",
                     GWEN_ConnectionLayer_GetId(curr));
            err=GWEN_ConnectionLayer_Work(curr, 1);
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "Called from here");
            }
          } /* if socket is readable */
        } /* if sock */
      }

      if (state==GWEN_IPCMsglayerStateConnecting) {
        sock=GWEN_IPCTransportLayer_GetWriteSocket(tl);
        if (sock) {
          if (GWEN_SocketSet_HasSocket(wset, sock)) {
            DBG_INFO(0, "Connecting connection %d is writeable",
                     GWEN_ConnectionLayer_GetId(curr));
            err=GWEN_ConnectionLayer_Work(curr, 0);
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "Called from here");
            }
          } /* if socket is writeable */
        } /* if sock */
      }

      if (state==GWEN_IPCMsglayerStateWaiting) {
        /* TODO: timeout for waiting sockets */
      }

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
void GWEN_GlobalServiceLayer_RemoveClosed() {
  GWEN_IPCCONNLAYER *cl;
  GWEN_IPCMSGLAYER *ml;

  DBG_INFO(0, "Removing closed connections");
  cl=GWEN_Global_ServiceLayer->connections;
  while(cl) {
    GWEN_IPCCONNLAYER *nextcl;

    nextcl=cl->next;
    ml=GWEN_ConnectionLayer_GetMsgLayer(cl);
    assert(ml);
    if (GWEN_MsgLayer_GetState(ml)==GWEN_IPCMsglayerStateClosed) {
      /* notify connection layer */
      GWEN_ConnectionLayer_Down(cl);

      if (!(GWEN_ConnectionLayer_GetFlags(cl) &
            GWEN_IPCCONNLAYER_FLAGS_PERSISTENT)) {
        /* remove closed connection if it is not marked persistent */
        DBG_INFO(0, "Removing connection (%d)",
                 GWEN_ConnectionLayer_GetId(cl));
        GWEN_LIST_DEL(GWEN_IPCCONNLAYER,
                      cl,
                      &(GWEN_Global_ServiceLayer->connections));
        GWEN_ConnectionLayer_free(cl);
      }
      else {
        DBG_INFO(0, "Connection %d is closed but persistent",
                 GWEN_ConnectionLayer_GetId(cl));
      }
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



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_GlobalServiceLayer_RegisterLocalLayer(GWEN_SERVICELAYER *sl){
  assert(sl);
  if (GWEN_Global_ServiceLayer==0) {
    GWEN_Global_ServiceLayer=GWEN_GlobalServiceLayer_new();
  }
  GWEN_Global_ServiceLayer->localLayers++;
  sl->id=++GWEN_Global_ServiceLayer_LastId;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_GlobalServiceLayer_UnregisterLocalLayer(GWEN_SERVICELAYER *sl){
  assert(sl);
  if (GWEN_Global_ServiceLayer) {
    DBG_INFO(0, "Unregistering local service layer %d", sl->id);
    if (GWEN_Global_ServiceLayer->localLayers<2) {
      DBG_INFO(0, "Global service layer no longer in use, freeing");
      GWEN_GlobalServiceLayer_free(GWEN_Global_ServiceLayer);
      GWEN_Global_ServiceLayer=0;
    }
    else {
      GWEN_Global_ServiceLayer->localLayers--;
      DBG_INFO(0, "Global service layer still in use (%d layers)",
	       GWEN_Global_ServiceLayer->localLayers);
    }
  }
  else {
    DBG_WARN(0, "Unregister: No local service layer registered.");
  }
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_SERVICELAYER *GWEN_ServiceLayer_new(){
  GWEN_SERVICELAYER *sl;
  GWEN_ERRORCODE err;

  GWEN_NEW_OBJECT(GWEN_SERVICELAYER, sl);
  err=GWEN_GlobalServiceLayer_RegisterLocalLayer(sl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR(0, "Could not register local layer, aborting.");
    assert(GWEN_Error_IsOk(err));
  }
  return sl;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ServiceLayer_free(GWEN_SERVICELAYER *sl){
  if (sl) {
    GWEN_ERRORCODE err;

    err=GWEN_GlobalServiceLayer_UnregisterLocalLayer(sl);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR(0, "Could not unregister local layer, aborting.");
      assert(GWEN_Error_IsOk(err));
    }
    free(sl);
  }
}





/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_ServiceLayer_AddConnection(GWEN_SERVICELAYER *sl,
				GWEN_IPCCONNLAYER *conn){
  assert(sl);
  assert(conn);
  GWEN_ConnectionLayer_SetLibMark(conn, sl->id);
  return GWEN_GlobalServiceLayer_AddConnection(conn);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCCONNLAYER*
GWEN_ServiceLayer_FindConnection(GWEN_SERVICELAYER *sl,
				 unsigned int id,
				 unsigned int userMark){
  assert(sl);
  return GWEN_GlobalServiceLayer_FindConnection(id,
						sl->id,
						userMark);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_ServiceLayer_Work(GWEN_SERVICELAYER *sl, int timeout){
  GWEN_ERRORCODE err;

  assert(sl);
  err=GWEN_GlobalServiceLayer_Work(timeout);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    return err;
  }

  /* TODO: read next message, sort it in */

  /* do some cleanup */
  GWEN_ServiceLayer_RemoveClosed(sl);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ServiceLayer_RemoveClosed(GWEN_SERVICELAYER *sl){
  assert(sl);
  GWEN_GlobalServiceLayer_RemoveClosed();
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_ServiceLayer_Close(GWEN_SERVICELAYER *sl,
			     unsigned int id,
                             unsigned int userMark,
			     int force){
  assert(sl);
  GWEN_GlobalServiceLayer_Close(id, sl->id, userMark, force);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCMSG *GWEN_ServiceLayer_NextIncomingMsg(GWEN_SERVICELAYER *sl,
                                               unsigned int userMark) {
  GWEN_IPCCONNLAYER *curr;
  GWEN_IPCCONNLAYER *startedAt;

  assert(sl);
  curr=0;

  if (sl->lastCheckedMsgLayer)
    curr=GWEN_ServiceLayer_FindConnection(sl, 0, userMark);
  if (!curr)
    curr=GWEN_Global_ServiceLayer->connections;
  else {
    /* get the next connection */
    while(curr) {
      if (sl->id==GWEN_ConnectionLayer_GetLibMark(curr))
        if (userMark==0 || userMark==GWEN_ConnectionLayer_GetUserMark(curr))
          break;
      curr=GWEN_ConnectionLayer_GetNext(curr);
    } /* while */
  }

  if (!curr) {
    DBG_VERBOUS(0, "No connection found for %d/%d",
                sl->id, userMark);
    return 0;
  }

  startedAt=curr;
  while (curr) {
    GWEN_IPCMSGLAYER *ml;
    GWEN_IPCMSG *msg;

    ml=GWEN_ConnectionLayer_GetMsgLayer(curr);
    assert(ml);
    msg=GWEN_MsgLayer_GetIncomingMsg(ml);
    if (msg) {
      DBG_INFO(0, "Got a message from %d", GWEN_MsgLayer_GetId(ml));
      sl->lastCheckedMsgLayer=GWEN_MsgLayer_GetId(ml);
      return msg;
    }

    /* get next matching connection layer */
    curr=GWEN_ConnectionLayer_GetNext(curr);
    if (!curr)
      curr=GWEN_Global_ServiceLayer->connections;
    while(curr && curr!=startedAt) {
      if (sl->id==GWEN_ConnectionLayer_GetLibMark(curr))
        if (userMark==0 || userMark==GWEN_ConnectionLayer_GetUserMark(curr))
          break;
      curr=GWEN_ConnectionLayer_GetNext(curr);
      if (!curr)
        curr=GWEN_Global_ServiceLayer->connections;
    } /* while */
    if (curr==startedAt) {
      DBG_INFO(0, "All checked, no message");
      return 0;
    }
  } /* while */

  DBG_INFO(0, "No message");
  return 0;
}









