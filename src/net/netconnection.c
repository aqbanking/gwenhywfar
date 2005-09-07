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

#define DISABLE_DEBUGLOG


/* Define this if you are extending the "class" NetConnection */
#define GWEN_EXTEND_NETCONNECTION

#include "netconnection_p.h"
#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/gwentime.h>

#include <time.h>



GWEN_LIST_FUNCTIONS(GWEN_NETCONNECTION, GWEN_NetConnection)
GWEN_LIST2_FUNCTIONS(GWEN_NETCONNECTION, GWEN_NetConnection)
GWEN_INHERIT_FUNCTIONS(GWEN_NETCONNECTION)




/* -------------------------------------------------------------- FUNCTION */
GWEN_NETCONNECTION *GWEN_NetConnection_new(GWEN_NETTRANSPORT *tr,
                                           int take,
                                           GWEN_TYPE_UINT32 libId){
  GWEN_NETCONNECTION *conn;

  GWEN_NEW_OBJECT(GWEN_NETCONNECTION, conn);
  DBG_MEM_INC("GWEN_NETCONNECTION", 0);
  GWEN_LIST_INIT(GWEN_NETCONNECTION, conn);
  GWEN_INHERIT_INIT(GWEN_NETCONNECTION, conn);
  conn->readBuffer=GWEN_RingBuffer_new(GWEN_NETCONNECTION_BUFFERSIZE);
  conn->writeBuffer=GWEN_RingBuffer_new(GWEN_NETCONNECTION_BUFFERSIZE);
  conn->transportLayer=tr;
  conn->takeTransport=take;
  conn->lastResult=GWEN_NetTransportResultOk;
  conn->inMsgs=GWEN_NetMsg_List_new();
  conn->outMsgs=GWEN_NetMsg_List_new();
  conn->libraryMark=libId;

  conn->usage=1;
  return conn;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_free(GWEN_NETCONNECTION *conn){
  if (conn) {
    DBG_MEM_DEC("GWEN_NETCONNECTION");
    assert(conn->usage);
    if (--(conn->usage)==0) {
      GWEN_INHERIT_FINI(GWEN_NETCONNECTION, conn);
      GWEN_RingBuffer_free(conn->readBuffer);
      GWEN_RingBuffer_free(conn->writeBuffer);
      if (conn->takeTransport)
        GWEN_NetTransport_free(conn->transportLayer);
      GWEN_NetMsg_List_free(conn->inMsgs);
      GWEN_NetMsg_List_free(conn->outMsgs);

      GWEN_LIST_FINI(GWEN_NETCONNECTION, conn);
      free(conn);
    }
  }
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnection_Read(GWEN_NETCONNECTION *conn,
                            char *buffer,
                            GWEN_TYPE_UINT32 *bsize){
  assert(conn);
  if (GWEN_NetTransport_GetStatus(conn->transportLayer)==
      GWEN_NetTransportStatusDisabled) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Connection disabled");
    return -1;
  }
  return GWEN_RingBuffer_ReadBytes(conn->readBuffer,
                                   buffer,
                                   bsize);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnection_Read_Wait(GWEN_NETCONNECTION *conn,
				 char *buffer,
				 GWEN_TYPE_UINT32 *bsize,
				 int timeout){
  time_t startt;
  int distance;
  int count;
  GWEN_NETCONNECTION_WORKRESULT rv;
  int lastHadNoWaitFlags;

  startt=time(0);
  assert(conn);

  /* check whether EOF was met */
  if (GWEN_RingBuffer_GetUsedBytes(conn->readBuffer)==0 &&
      GWEN_NetTransport_GetFlags(conn->transportLayer) &
      GWEN_NETTRANSPORT_FLAGS_EOF_IN) {
    DBG_INFO(GWEN_LOGDOMAIN, "Reading beyond EOF, aborting");
    return -1;
  }

  if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE)
    distance=GWEN_NETCONNECTION_TIMEOUT_NONE;
  else if (timeout==GWEN_NETCONNECTION_TIMEOUT_FOREVER)
    distance=GWEN_NETCONNECTION_TIMEOUT_FOREVER;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance)>timeout)
        distance=timeout;
    if (!distance)
      distance=750;
  }

  lastHadNoWaitFlags=0;
  for (count=0;;) {
    /* actually try to read */
    rv=GWEN_NetConnection_Read(conn, buffer, bsize);
    if (rv==0) {
      /* read was successfull, break */
      DBG_VERBOUS(GWEN_LOGDOMAIN, "I have read %d byte(s)", *bsize);
      break;
    }

    /* check whether EOF was now met */
    if (GWEN_NetTransport_GetFlags(conn->transportLayer) &
        GWEN_NETTRANSPORT_FLAGS_EOF_IN) {
      DBG_INFO(GWEN_LOGDOMAIN, "EOF now met, aborting");
      *bsize=0;
      break;
    }

    /* check whether we should abort */
    if (lastHadNoWaitFlags)
      break;

    /* let the connection work */
    rv=GWEN_NetConnection_Work(conn);
    if (rv==GWEN_NetConnectionWorkResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "Error while working");
      return -1;
    }

    for (;;count++) {
      GWEN_TYPE_UINT32 waitFlags;
      double d;
      int res;

      if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
	DBG_ERROR(GWEN_LOGDOMAIN, "User aborted via waitcallback");
	return -1;
      }

      DBG_DEBUG(GWEN_LOGDOMAIN,
               "WRITE: %d bytes in buffer",
               GWEN_RingBuffer_GetUsedBytes(conn->writeBuffer));

      /* prepare waitflags */
      waitFlags=0;
      if (GWEN_RingBuffer_GetBytesLeft(conn->readBuffer) ||
          conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTREAD)
        waitFlags|=GWEN_NETCONNECTION_WAIT_READ;
      if (GWEN_RingBuffer_GetUsedBytes(conn->writeBuffer) ||
          conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTWRITE)
        waitFlags|=GWEN_NETCONNECTION_WAIT_WRITE;
      if (!waitFlags) {
        DBG_WARN(GWEN_LOGDOMAIN, "Nothing to wait on");
        lastHadNoWaitFlags=1;
        break;
      }

      /* wait */
      res=GWEN_NetConnection_Wait(conn, distance, waitFlags);
      if (res==-1) {
	DBG_INFO(GWEN_LOGDOMAIN, "Error while waiting");
	return -1;
      }
      if (res==0)
        /* found activity, break */
        break;

      d=difftime(time(0), startt);

      /* check timeout */
      if (timeout!=GWEN_NETCONNECTION_TIMEOUT_FOREVER) {
        if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE ||
            d>timeout) {
          DBG_INFO(GWEN_LOGDOMAIN,
                   "Could not read within %d seconds, giving up",
                   timeout);
          return 1;
        }
      }
    } /* for */

  } /* for */

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnection_Write(GWEN_NETCONNECTION *conn,
                             const char *buffer,
                             GWEN_TYPE_UINT32 *bsize){
  assert(conn);
  assert(buffer);
  assert(bsize);
  if (GWEN_NetTransport_GetStatus(conn->transportLayer)==
      GWEN_NetTransportStatusDisabled) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Connection disabled");
    return -1;
  }
  if (*bsize) {
    return GWEN_RingBuffer_WriteBytes(conn->writeBuffer,
                                      buffer,
                                      bsize);
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Nothing to write");
    return 0;
  }
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnection_Write_Wait(GWEN_NETCONNECTION *conn,
				  const char *buffer,
				  GWEN_TYPE_UINT32 *bsize,
				  int timeout){
  time_t startt;
  int distance;
  int count;
  GWEN_NETCONNECTION_WORKRESULT rv;
  int lastHadNoWaitFlags;

  startt=time(0);
  assert(conn);

  if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE)
    distance=GWEN_NETCONNECTION_TIMEOUT_NONE;
  else if (timeout==GWEN_NETCONNECTION_TIMEOUT_FOREVER)
    distance=GWEN_NETCONNECTION_TIMEOUT_FOREVER;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance)>timeout)
        distance=timeout;
    if (!distance)
      distance=750;
  }

  lastHadNoWaitFlags=0;
  for (count=0;;) {
    int res;

    /* actually try to write */
    res=GWEN_NetConnection_Write(conn, buffer, bsize);
    if (res==0) {
      /* write was successfull, break */
      DBG_VERBOUS(GWEN_LOGDOMAIN, "I have written %d bytes", *bsize);
      break;
    }

    /* check whether we should abort */
    if (lastHadNoWaitFlags)
      break;

    /* let the connection work */
    rv=GWEN_NetConnection_Work(conn);
    if (rv==GWEN_NetConnectionWorkResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "Error while working");
      return -1;
    }

    for (;;count++) {
      GWEN_TYPE_UINT32 waitFlags;
      double d;

      if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
	DBG_ERROR(GWEN_LOGDOMAIN, "User aborted via waitcallback");
	return -1;
      }

      /* prepare waitflags */
      waitFlags=0;
      if (GWEN_RingBuffer_GetBytesLeft(conn->readBuffer) ||
          conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTREAD)
        waitFlags|=GWEN_NETCONNECTION_WAIT_READ;
      if (GWEN_RingBuffer_GetUsedBytes(conn->writeBuffer) ||
          conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTWRITE)
        waitFlags|=GWEN_NETCONNECTION_WAIT_WRITE;
      if (!waitFlags) {
        DBG_WARN(GWEN_LOGDOMAIN, "Nothing to wait on");
        lastHadNoWaitFlags=1;
        break;
      }

      /* wait */
      res=GWEN_NetConnection_Wait(conn, distance, waitFlags);
      if (res==-1) {
	DBG_INFO(GWEN_LOGDOMAIN, "Error while waiting");
	return -1;
      }
      if (res==0)
	/* found activity, break */
	break;

      d=difftime(time(0), startt);

      /* check timeout */
      if (timeout!=GWEN_NETCONNECTION_TIMEOUT_FOREVER) {
        if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE ||
            d>timeout) {
	  DBG_INFO(GWEN_LOGDOMAIN, "Could not read within %d seconds, giving up",
		   timeout);
	  return 1;
	}
      }
    } /* for */

  } /* for */

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnection_Flush(GWEN_NETCONNECTION *conn,
			     int timeout){
  time_t startt;
  int distance;
  int count;
  GWEN_NETCONNECTION_WORKRESULT rv;

  startt=time(0);
  assert(conn);

  if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE)
    distance=GWEN_NETCONNECTION_TIMEOUT_NONE;
  else if (timeout==GWEN_NETCONNECTION_TIMEOUT_FOREVER)
    distance=GWEN_NETCONNECTION_TIMEOUT_FOREVER;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance)>timeout)
        distance=timeout;
    if (!distance)
      distance=750;
  }

  for (count=0;;) {
    /* let the connection work */
    rv=GWEN_NetConnection_Work(conn);
    if (rv==GWEN_NetConnectionWorkResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "Error while working");
      return -1;
    }

    /* check for remaining bytes in write buffer */
    if (GWEN_RingBuffer_GetUsedBytes(conn->writeBuffer)==0) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Write buffer empty");
      break;
    }

    for (;;count++) {
      GWEN_TYPE_UINT32 waitFlags;
      GWEN_NETTRANSPORT_STATUS st;
      double d;
      int res;

      st=GWEN_NetTransport_GetStatus(conn->transportLayer);
      if (st==GWEN_NetTransportStatusUnconnected ||
          st==GWEN_NetTransportStatusPDisconnected ||
          st==GWEN_NetTransportStatusDisabled ||
          st==GWEN_NetTransportStatusListening) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Connection is down (%s)",
		  GWEN_NetTransport_StatusName(st));
	return -1;
      }

      if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
	DBG_ERROR(GWEN_LOGDOMAIN, "User aborted via waitcallback");
	return -1;
      }

      /* prepare waitflags */
      waitFlags=0;
      if (GWEN_RingBuffer_GetBytesLeft(conn->readBuffer) ||
          conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTREAD)
        waitFlags|=GWEN_NETCONNECTION_WAIT_READ;
      if (GWEN_RingBuffer_GetUsedBytes(conn->writeBuffer) ||
          conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTWRITE)
        waitFlags|=GWEN_NETCONNECTION_WAIT_WRITE;
      if (!waitFlags) {
        DBG_WARN(GWEN_LOGDOMAIN, "Nothing to wait on, should not happen");
      }

      /* wait */
      res=GWEN_NetConnection_Wait(conn, distance, waitFlags);
      if (res==-1) {
	DBG_INFO(GWEN_LOGDOMAIN, "Error while waiting");
	return -1;
      }
      if (res==0)
	/* found activity, break */
	break;

      d=difftime(time(0), startt);

      /* check timeout */
      if (timeout!=GWEN_NETCONNECTION_TIMEOUT_FOREVER) {
	if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE ||
            d>timeout) {
          DBG_INFO(GWEN_LOGDOMAIN, "Could not write within %d seconds, giving up (%d)",
                   waitFlags,
                   timeout);
	  return 1;
	}
      }
    } /* for */

  } /* for */

  return 0;

}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnection_StartConnect(GWEN_NETCONNECTION *conn){
  assert(conn);
  if (GWEN_NetTransport_GetStatus(conn->transportLayer)==
      GWEN_NetTransportStatusDisabled) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Connection disabled");
    return -1;
  }
  conn->lastResult=GWEN_NetTransport_StartConnect(conn->transportLayer);
  if (conn->lastResult==GWEN_NetTransportResultWantRead)
    conn->ioFlags|=GWEN_NETCONNECTION_IOFLAG_WANTREAD;
  if (conn->lastResult==GWEN_NetTransportResultWantWrite)
    conn->ioFlags|=GWEN_NETCONNECTION_IOFLAG_WANTWRITE;

  if (conn->lastResult==GWEN_NetTransportResultError ||
      conn->lastResult==GWEN_NetTransportResultAborted) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return -1;
  }

  if (GWEN_NetTransport_GetStatus(conn->transportLayer)==
      GWEN_NetTransportStatusLConnected) {
    DBG_INFO(GWEN_LOGDOMAIN, "Connection immediately up");
    GWEN_NetConnection_Up(conn);
  }


  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnection_Connect_Wait(GWEN_NETCONNECTION *conn,
                                    int timeout){
  int rv;

  assert(conn);
  rv=GWEN_NetConnection_StartConnect(conn);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not start to connect");
    return rv;
  }
  rv=GWEN_NetConnection_WaitForStatus(conn,
                                      GWEN_NetTransportStatusLConnected,
                                      timeout);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not connect");
    return rv;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Connected");
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnection_StartListen(GWEN_NETCONNECTION *conn){
  assert(conn);
  if (GWEN_NetTransport_GetStatus(conn->transportLayer)==
      GWEN_NetTransportStatusDisabled) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Connection disabled");
    return -1;
  }
  conn->lastResult=GWEN_NetTransport_StartAccept(conn->transportLayer);
  if (conn->lastResult==GWEN_NetTransportResultWantRead)
    conn->ioFlags|=GWEN_NETCONNECTION_IOFLAG_WANTREAD;
  if (conn->lastResult==GWEN_NetTransportResultWantWrite)
    conn->ioFlags|=GWEN_NETCONNECTION_IOFLAG_WANTWRITE;
  if (conn->lastResult==GWEN_NetTransportResultError ||
      conn->lastResult==GWEN_NetTransportResultAborted) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return -1;
  }
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_Reset(GWEN_NETCONNECTION *conn) {
  assert(conn);

  GWEN_NetMsg_List_Clear(conn->inMsgs);
  GWEN_RingBuffer_Reset(conn->readBuffer);
  GWEN_RingBuffer_Reset(conn->writeBuffer);
  GWEN_NetMsg_List_Clear(conn->outMsgs);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnection_StartDisconnect(GWEN_NETCONNECTION *conn){
  int rv=0;

  assert(conn);
  if (GWEN_NetTransport_GetStatus(conn->transportLayer)==
      GWEN_NetTransportStatusDisabled) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Connection disabled");
    return -1;
  }
  conn->lastResult=GWEN_NetTransport_StartDisconnect(conn->transportLayer);
  if (conn->lastResult==GWEN_NetTransportResultWantRead)
    conn->ioFlags|=GWEN_NETCONNECTION_IOFLAG_WANTREAD;
  if (conn->lastResult==GWEN_NetTransportResultWantWrite)
    conn->ioFlags|=GWEN_NETCONNECTION_IOFLAG_WANTWRITE;
  if (conn->lastResult==GWEN_NetTransportResultError ||
      conn->lastResult==GWEN_NetTransportResultAborted) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    rv=-1;
  }

  if (GWEN_NetTransport_GetStatus(conn->transportLayer)==
      GWEN_NetTransportStatusPDisconnected && rv==0) {
    GWEN_NetConnection_Down(conn);
  }

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnection_Disconnect_Wait(GWEN_NETCONNECTION *conn,
                                       int timeout){
  int rv;

  assert(conn);
  rv=GWEN_NetConnection_StartDisconnect(conn);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not start to disconnect");
    return rv;
  }
  rv=GWEN_NetConnection_WaitForStatus(conn,
                                      GWEN_NetTransportStatusPDisconnected,
                                      timeout);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not disconnect");
    return rv;
  }

  GWEN_NetTransport_SetStatus(conn->transportLayer,
                              GWEN_NetTransportStatusPDisconnected);
  DBG_INFO(GWEN_LOGDOMAIN, "disconnected");
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETCONNECTION_WORKRESULT
GWEN_NetConnection_WorkIO(GWEN_NETCONNECTION *conn){
  GWEN_NETTRANSPORT_WORKRESULT rv;
  GWEN_TYPE_UINT32 psize;
  GWEN_NETTRANSPORT_STATUS startStatus;
  int doneSomething;

  assert(conn);

  doneSomething=0;
  conn->ioFlags=0;
  startStatus=GWEN_NetTransport_GetStatus(conn->transportLayer);

  /* check for disabled connection */
  if (GWEN_NetTransport_GetStatus(conn->transportLayer)==
      GWEN_NetTransportStatusDisabled) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Connection disabled");
    return GWEN_NetConnectionWorkResult_Error;
  }

  /* check for inactive connection */
  if (GWEN_NetTransport_GetStatus(conn->transportLayer)!=
      GWEN_NetTransportStatusPDisconnected &&
      GWEN_NetTransport_GetStatus(conn->transportLayer)!=
      GWEN_NetTransportStatusUnconnected){

    /* ask the next lower level to work */
    DBG_DEBUG(GWEN_LOGDOMAIN, "Letting transport layer work");
    rv=GWEN_NetTransport_Work(conn->transportLayer);
    if (rv==GWEN_NetTransportWorkResult_Error) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error in transport layer (%d)", rv);
      return GWEN_NetConnectionWorkResult_Error;
    }
    else if (rv==GWEN_NetTransportWorkResult_NoChange) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "No change in transport layer");
      /* return GWEN_NetConnectionWorkResult_NoChange; */
    }
    else if (rv==GWEN_NetTransportWorkResult_Change) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Could do something");
      conn->lastResult=GWEN_NetTransportResultOk;
      doneSomething=1;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unhandled transport result %d", rv);
    }
  }

  /* check for important status changes */
  if (GWEN_NetTransport_GetStatus(conn->transportLayer)==
      GWEN_NetTransportStatusLConnected &&
      startStatus!=GWEN_NetTransportStatusLConnected) {
    GWEN_NetConnection_Up(conn);
  }
  else if (GWEN_NetTransport_GetStatus(conn->transportLayer)!=
           GWEN_NetTransportStatusLConnected &&
           startStatus==GWEN_NetTransportStatusLConnected) {
    GWEN_NetConnection_Down(conn);
  }

  if (doneSomething) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Done something, will return");
    return GWEN_NetConnectionWorkResult_Change;
  }

  /* do some IO (only if logically connected) */
  if (GWEN_NetTransport_GetStatus(conn->transportLayer)==
      GWEN_NetTransportStatusLConnected) {
    psize=GWEN_RingBuffer_GetMaxUnsegmentedRead(conn->writeBuffer);
    if (psize) {
      const char *ptr;
      int bsize;
      GWEN_NETTRANSPORT_RESULT res;

      DBG_DEBUG(GWEN_LOGDOMAIN, "Trying to write up to %d bytes", psize);
      bsize=psize;
      ptr=GWEN_RingBuffer_GetReadPointer(conn->writeBuffer);
      res=GWEN_NetTransport_Write(conn->transportLayer,
                                  ptr, &bsize);
      DBG_DEBUG(GWEN_LOGDOMAIN,
                "Result of transport layer write: %s (%d) %d bytes written",
                GWEN_NetTransport_ResultName(res),
                res, bsize);

      if (res==GWEN_NetTransportResultOk) {
        GWEN_RingBuffer_SkipBytesRead(conn->writeBuffer,
                                      bsize);
        doneSomething=1;
      }
      conn->lastResult=res;
      if (conn->lastResult==GWEN_NetTransportResultWantRead)
        conn->ioFlags|=GWEN_NETCONNECTION_IOFLAG_WANTREAD;
      else if (conn->lastResult==GWEN_NetTransportResultWantWrite) {
        conn->ioFlags|=GWEN_NETCONNECTION_IOFLAG_WANTWRITE;
      }
    }

    if (!(GWEN_NetTransport_GetFlags(conn->transportLayer) &
          GWEN_NETTRANSPORT_FLAGS_EOF_IN)) {
      psize=GWEN_RingBuffer_GetMaxUnsegmentedWrite(conn->readBuffer);
      if (psize) {
        char *ptr;
        int bsize;
        GWEN_NETTRANSPORT_RESULT res;

        DBG_DEBUG(GWEN_LOGDOMAIN, "Trying to read up to %d bytes", psize);
        bsize=psize;
        ptr=GWEN_RingBuffer_GetWritePointer(conn->readBuffer);
        res=GWEN_NetTransport_Read(conn->transportLayer,
                                   ptr, &bsize);
        DBG_DEBUG(GWEN_LOGDOMAIN, "Result of transport layer read: %s (%d)",
                  GWEN_NetTransport_ResultName(res),
                  res);
        if (res==GWEN_NetTransportResultOk) {
          if (bsize==0) {
            DBG_INFO(GWEN_LOGDOMAIN, "Connection is down");
            GWEN_NetTransport_SetStatus(conn->transportLayer,
                                        GWEN_NetTransportStatusPDisconnected);
            GWEN_NetConnection_Down(conn);
            doneSomething=1;
          }
          else {
            DBG_DEBUG(GWEN_LOGDOMAIN, "Adding %d bytes to read buffer", bsize);
            GWEN_RingBuffer_SkipBytesWrite(conn->readBuffer,
                                           bsize);
            doneSomething=1;
          }
        }
        conn->lastResult=res;
        if (conn->lastResult==GWEN_NetTransportResultWantRead)
          conn->ioFlags|=GWEN_NETCONNECTION_IOFLAG_WANTREAD;
        else if (conn->lastResult==GWEN_NetTransportResultWantWrite) {
          conn->ioFlags|=GWEN_NETCONNECTION_IOFLAG_WANTWRITE;
        }
      }
    } /* if not EOF met */
  }

  if (doneSomething)
    return GWEN_NetConnectionWorkResult_Change;
  else
    return GWEN_NetConnectionWorkResult_NoChange;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnection_Wait(GWEN_NETCONNECTION *conn, int timeout,
			    GWEN_TYPE_UINT32 waitFlags){
  GWEN_SOCKETSET *rset, *wset;

  assert(conn);
  rset=GWEN_SocketSet_new();
  wset=GWEN_SocketSet_new();

  /* add read sockets */
  if (waitFlags & GWEN_NETCONNECTION_WAIT_READ) {
    if (GWEN_NetTransport_AddSockets(conn->transportLayer, rset, 1)) {
      DBG_INFO(GWEN_LOGDOMAIN, "Could not add read sockets");
      GWEN_SocketSet_free(rset);
      GWEN_SocketSet_free(wset);
      return -1;
    }
  }

  /* add write sockets */
  if (waitFlags & GWEN_NETCONNECTION_WAIT_WRITE) {
    if (GWEN_NetTransport_AddSockets(conn->transportLayer, wset, 0)) {
      DBG_INFO(GWEN_LOGDOMAIN, "Could not add write sockets");
      GWEN_SocketSet_free(rset);
      GWEN_SocketSet_free(wset);
      return -1;
    }
  }

  /* check whether we have sockets for a select */
  if ((GWEN_SocketSet_GetSocketCount(rset)+
      GWEN_SocketSet_GetSocketCount(wset))==0) {
    /* no socket, so sleep to reduce CPU usage */
    if (timeout) {
      /* well, actually only sleep if the caller wanted a timeout */
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Sleeping");
      GWEN_Socket_Select(0, 0, 0, GWEN_NETCONNECTION_CPU_TIMEOUT);
    }
    GWEN_SocketSet_free(rset);
    GWEN_SocketSet_free(wset);
    DBG_INFO(GWEN_LOGDOMAIN, "No socket");
    return -1; /* TODO: return 0 ? */
  }
  else {
    GWEN_ERRORCODE err;

    err=GWEN_Socket_Select(rset, wset, 0, timeout);
    if (!GWEN_Error_IsOk(err)) {
      if (GWEN_Error_GetType(err)!=
	  GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
	  GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT) {
	DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
	GWEN_SocketSet_free(rset);
	GWEN_SocketSet_free(wset);
	return -1;
      }
      else {
	DBG_VERBOUS(GWEN_LOGDOMAIN, "Timeout (%d)", waitFlags);
	GWEN_SocketSet_free(rset);
	GWEN_SocketSet_free(wset);
	return 1;
      }
    }
  }

  GWEN_SocketSet_free(rset);
  GWEN_SocketSet_free(wset);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnection_WaitForStatus(GWEN_NETCONNECTION *conn,
                                     GWEN_NETTRANSPORT_STATUS expStatus,
                                     int timeout){
  time_t startt;
  int distance;
  int count;
  GWEN_NETCONNECTION_WORKRESULT rv;

  startt=time(0);
  assert(conn);

  if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE)
    distance=GWEN_NETCONNECTION_TIMEOUT_NONE;
  else if (timeout==GWEN_NETCONNECTION_TIMEOUT_FOREVER)
    distance=GWEN_NETCONNECTION_TIMEOUT_FOREVER;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance)>timeout)
        distance=timeout;
    if (!distance)
      distance=750;
  }

  for (count=0;;) {
    /* let the connection work */
    rv=GWEN_NetConnection_Work(conn);
    if (rv==GWEN_NetConnectionWorkResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "Error while working");
      return -1;
    }

    for (;;count++) {
      GWEN_TYPE_UINT32 waitFlags;
      GWEN_NETTRANSPORT_STATUS st;
      double d;
      int res;

      st=GWEN_NetConnection_GetStatus(conn);
      if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
	DBG_ERROR(GWEN_LOGDOMAIN, "User aborted via waitcallback");
	return -1;
      }

      if (st==expStatus) {
        DBG_INFO(GWEN_LOGDOMAIN,
                 "Expected status \"%s\" (%d) reached",
                 GWEN_NetTransport_StatusName(expStatus),
                 expStatus);
        return 0;
      }
      else if (st==GWEN_NetTransportStatusDisabled ||
               st==GWEN_NetTransportStatusUnconnected) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Connection is inactive, will never change status");
        return -1;
      }

      /* prepare waitflags */
      waitFlags=0;
      if (conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTREAD ||
          conn->lastResult==GWEN_NetTransportResultOk ||
          conn->lastResult==GWEN_NetTransportResultWantRead)
        waitFlags|=GWEN_NETCONNECTION_WAIT_READ;
      /* don't change the following rule unless you really know what you
       * are doing! */
      if (conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTWRITE ||
          conn->lastResult==GWEN_NetTransportResultOk ||
          conn->lastResult==GWEN_NetTransportResultWantWrite)
        waitFlags|=GWEN_NETCONNECTION_WAIT_WRITE;
      if (!waitFlags) {
        DBG_WARN(GWEN_LOGDOMAIN, "Unexpected last result %d", conn->lastResult);
      }

      /* wait */
      res=GWEN_NetConnection_Wait(conn, distance, waitFlags);
      if (res==-1) {
	DBG_INFO(GWEN_LOGDOMAIN, "Error while waiting");
	return -1;
      }
      if (res==0)
	/* found activity, break */
	break;

      d=difftime(time(0), startt);

      /* check timeout */
      if (timeout!=GWEN_NETCONNECTION_TIMEOUT_FOREVER) {
        if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE ||
            d>timeout) {
          DBG_INFO(GWEN_LOGDOMAIN,
                   "Timeout (%d) while waiting for status %d, giving up (%d)",
                   timeout,
                   expStatus,
                   GWEN_NetConnection_GetStatus(conn));
          return 1;
        }
      }
    } /* for */

  } /* for */

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT*
GWEN_NetConnection_GetNextIncoming(GWEN_NETCONNECTION *conn){
  assert(conn);
  return GWEN_NetTransport_GetNextIncoming(conn->transportLayer);
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT*
GWEN_NetConnection_GetNextIncoming_Wait(GWEN_NETCONNECTION *conn,
                                        int timeout){
  time_t startt;
  int distance;
  int count;
  GWEN_NETCONNECTION_WORKRESULT rv;
  GWEN_NETTRANSPORT *tr;

  startt=time(0);
  assert(conn);

  if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE)
    distance=GWEN_NETCONNECTION_TIMEOUT_NONE;
  else if (timeout==GWEN_NETCONNECTION_TIMEOUT_FOREVER)
    distance=GWEN_NETCONNECTION_TIMEOUT_FOREVER;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance)>timeout)
        distance=timeout;
    if (!distance)
      distance=750;
  }

  for (count=0;;) {
    for (;;count++) {
      GWEN_TYPE_UINT32 waitFlags;
      double d;
      int res;

      if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
	DBG_ERROR(GWEN_LOGDOMAIN, "User aborted via waitcallback");
	return 0;
      }

      tr=GWEN_NetTransport_GetNextIncoming(conn->transportLayer);
      if (tr) {
        DBG_INFO(GWEN_LOGDOMAIN, "Found an incoming connection");
        return tr;
      }

      /* prepare waitflags */
      waitFlags=0;
      /* dont change this unless you know exactly what you are doing */
      if (conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTREAD ||
	  conn->lastResult==GWEN_NetTransportResultOk)
	waitFlags|=GWEN_NETCONNECTION_WAIT_READ;
      if (conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTWRITE ||
          GWEN_RingBuffer_GetUsedBytes((conn->writeBuffer)))
	waitFlags|=GWEN_NETCONNECTION_WAIT_WRITE;
      if (!waitFlags) {
	DBG_WARN(GWEN_LOGDOMAIN, "Unexpected last result %d", conn->lastResult);
      }

      /* wait */
      res=GWEN_NetConnection_Wait(conn, distance, waitFlags);
      if (res==-1) {
	DBG_INFO(GWEN_LOGDOMAIN, "Error while waiting");
	return 0;
      }
      if (res==0)
	/* found activity, break */
	break;

      d=difftime(time(0), startt);

      /* check timeout */
      if (timeout!=GWEN_NETCONNECTION_TIMEOUT_FOREVER) {
        if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE ||
            d>timeout) {
          DBG_INFO(GWEN_LOGDOMAIN, "Timeout while waiting for connection, giving up");
          return 0;
        }
      }
    } /* for */

    /* let the connection work */
    rv=GWEN_NetConnection_Work(conn);
    if (rv==GWEN_NetConnectionWorkResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "Error while working");
      return 0;
    }
  } /* for */

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT*
GWEN_NetConnection_GetTransportLayer(const GWEN_NETCONNECTION *conn){
  assert(conn);
  return conn->transportLayer;
}



/* -------------------------------------------------------------- FUNCTION */
const GWEN_INETADDRESS*
GWEN_NetConnection_GetPeerAddr(const GWEN_NETCONNECTION *conn){
  assert(conn);
  return GWEN_NetTransport_GetPeerAddr(conn->transportLayer);
}


/* -------------------------------------------------------------- FUNCTION */
GWEN_NETCONNECTION_WORKRESULT
GWEN_NetConnection_Work(GWEN_NETCONNECTION *conn){
  GWEN_NETCONNECTION_WORKRESULT res;

  assert(conn);
  if (conn->workFn)
    res=conn->workFn(conn);
  else {
    DBG_WARN(GWEN_LOGDOMAIN, "Work function not set");
    res=GWEN_NetConnectionWorkResult_Error;
  }

  if (res==GWEN_NetConnectionWorkResult_Change) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Change on connection.");
  }
  else if (res==GWEN_NetConnectionWorkResult_Error) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error on connection.");
  }
  return res;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_Up(GWEN_NETCONNECTION *conn){
  assert(conn);
  if (!(conn->notified & GWEN_NETCONNECTION_NOTIFIED_UP)) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Connection %p (%p) is up",
             (void*)conn,
             (void*)GWEN_NetConnection_GetTransportLayer(conn));
    if (conn->upFn) {
      conn->upFn(conn);
      /* set UP notified flag, remove DOWN notify flag */
      conn->notified|=GWEN_NETCONNECTION_NOTIFIED_UP;
      conn->notified&=~GWEN_NETCONNECTION_NOTIFIED_DOWN;
    }
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_Down(GWEN_NETCONNECTION *conn){
  assert(conn);
  assert(conn);

  if ((conn->notified & GWEN_NETCONNECTION_NOTIFIED_UP) &&
      !(conn->notified & GWEN_NETCONNECTION_NOTIFIED_DOWN)) {
    /* only inform about DOWN connection if the application already knows
     * about that connection */
    DBG_INFO(GWEN_LOGDOMAIN,
             "Connection %p (%p) is down",
             (void*)conn,
             (void*)GWEN_NetConnection_GetTransportLayer(conn));
    if (conn->downFn) {
      conn->downFn(conn);
      /* set DOWN notified flag, remove UP notify flag */
      conn->notified|=GWEN_NETCONNECTION_NOTIFIED_DOWN;
      conn->notified&=~GWEN_NETCONNECTION_NOTIFIED_UP;
    }
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_SetWorkFn(GWEN_NETCONNECTION *conn,
				  GWEN_NETCONNECTION_WORKFN fn){
  assert(conn);
  conn->workFn=fn;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_SetUpFn(GWEN_NETCONNECTION *conn,
                                GWEN_NETCONNECTION_UPFN fn) {
  assert(conn);
  conn->upFn=fn;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_SetDownFn(GWEN_NETCONNECTION *conn,
                                  GWEN_NETCONNECTION_DOWNFN fn){
  assert(conn);
  conn->downFn=fn;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_AddInMsg(GWEN_NETCONNECTION *conn,
				 GWEN_NETMSG *msg){
  assert(conn);
  GWEN_NetMsg_List_Add(msg, conn->inMsgs);
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETMSG *GWEN_NetConnection_GetInMsg(GWEN_NETCONNECTION *conn){
  GWEN_NETMSG *msg;

  assert(conn);
  msg=GWEN_NetMsg_List_First(conn->inMsgs);
  if (msg) {
    GWEN_NetMsg_List_Del(msg);
    return msg;
  }
  else
    return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETMSG *GWEN_NetConnection_GetInMsg_Wait(GWEN_NETCONNECTION *conn,
                                             int timeout){
  time_t startt;
  int distance;
  int count;
  GWEN_NETCONNECTION_WORKRESULT rv;
  GWEN_NETMSG *msg;

  startt=time(0);
  assert(conn);

  if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE)
    distance=GWEN_NETCONNECTION_TIMEOUT_NONE;
  else if (timeout==GWEN_NETCONNECTION_TIMEOUT_FOREVER)
    distance=GWEN_NETCONNECTION_TIMEOUT_FOREVER;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance)>timeout)
        distance=timeout;
    if (!distance)
      distance=750;
  }

  rv=GWEN_NetConnection_Work(conn);
  if (rv==GWEN_NetConnectionWorkResult_Error) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error while working");
    return 0;
  }

  for (count=0;;) {
    for (;;count++) {
      GWEN_TYPE_UINT32 waitFlags;
      double d;
      int res;

      if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
	DBG_ERROR(GWEN_LOGDOMAIN, "User aborted via waitcallback");
	return 0;
      }

      msg=GWEN_NetConnection_GetInMsg(conn);
      if (msg) {
        DBG_INFO(GWEN_LOGDOMAIN, "Found an incoming message");
        return msg;
      }

#if 0
      if (conn->notified & GWEN_NETCONNECTION_NOTIFIED_DOWN) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Connection went down");
        return 0;
      }
#endif

#if 1
      /* prepare waitflags */
      waitFlags=0;
      if (GWEN_RingBuffer_GetBytesLeft(conn->readBuffer) ||
          conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTREAD)
        waitFlags|=GWEN_NETCONNECTION_WAIT_READ;
      if (GWEN_RingBuffer_GetUsedBytes(conn->writeBuffer) ||
          conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTWRITE ||
          GWEN_NetMsg_List_GetCount(conn->outMsgs))
        waitFlags|=GWEN_NETCONNECTION_WAIT_WRITE;
      if (!waitFlags) {
        DBG_WARN(GWEN_LOGDOMAIN, "Nothing to wait on, should not happen");
      }

      /* wait */
      res=GWEN_NetConnection_Wait(conn, distance, waitFlags);
      if (res==-1) {
	DBG_INFO(GWEN_LOGDOMAIN, "Error while waiting");
	return 0;
      }
      if (res==0)
	/* found activity, break */
	break;
#endif

      d=difftime(time(0), startt);

      /* check timeout */
      if (timeout!=GWEN_NETCONNECTION_TIMEOUT_FOREVER) {
	if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE ||
            d>timeout) {
          DBG_INFO(GWEN_LOGDOMAIN, "Timeout while waiting for connection, giving up");
          return 0;
        }
      }
    } /* for */

    /* let the connection work */
    rv=GWEN_NetConnection_Work(conn);
    if (rv==GWEN_NetConnectionWorkResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "Error while working");
      return 0;
    }
  } /* for */

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETMSG *GWEN_NetConnection_PeekInMsg(const GWEN_NETCONNECTION *conn){
  assert(conn);
  return GWEN_NetMsg_List_First(conn->inMsgs);
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_AddOutMsg(GWEN_NETCONNECTION *conn,
				  GWEN_NETMSG *msg){
  assert(conn);
  GWEN_NetMsg_List_Add(msg, conn->outMsgs);
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETMSG *GWEN_NetConnection_GetOutMsg(GWEN_NETCONNECTION *conn){
  GWEN_NETMSG *msg;

  assert(conn);
  msg=GWEN_NetMsg_List_First(conn->outMsgs);
  if (msg) {
    GWEN_NetMsg_List_Del(msg);
    return msg;
  }
  else
    return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETMSG *GWEN_NetConnection_PeekOutMsg(const GWEN_NETCONNECTION *conn){
  assert(conn);
  return GWEN_NetMsg_List_First(conn->outMsgs);
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_RINGBUFFER *GWEN_NetConnection_GetReadBuffer(GWEN_NETCONNECTION *conn){
  assert(conn);
  return conn->readBuffer;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_RINGBUFFER *GWEN_NetConnection_GetWriteBuffer(GWEN_NETCONNECTION *conn){
  assert(conn);
  return conn->writeBuffer;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETCONNECTION_WORKRESULT
GWEN_NetConnection__Walk(GWEN_NETCONNECTION_LIST *connList,
                         int timeout) {
  GWEN_NETCONNECTION *curr;
  GWEN_SOCKETSET *rset, *wset;
  GWEN_NETCONNECTION_WORKRESULT rv;
  unsigned int errors;
  int changes;

  changes=0;
  errors=0;
  rset=GWEN_SocketSet_new();
  wset=GWEN_SocketSet_new();

  /* -------------------------------------------- let all connections work */
  curr=GWEN_NetConnection_List_First(connList);
  while(curr) {
    GWEN_NETTRANSPORT_STATUS st;

    st=GWEN_NetConnection_GetStatus(curr);
    if (st!=GWEN_NetTransportStatusDisabled &&
        st!=GWEN_NetTransportStatusUnconnected) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Working on connection...");
      rv=GWEN_NetConnection_Work(curr);
      if (rv==GWEN_NetConnectionWorkResult_Error) {
        DBG_INFO(GWEN_LOGDOMAIN, "Error working (result was %d)", rv);
        errors++;
      }
      else if (rv==GWEN_NetConnectionWorkResult_Change) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "Change in connection");
        changes++;
      }
    }
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Skipping inactive connection");
    }

    curr=GWEN_NetConnection_List_Next(curr);
  } /* while */

  if (changes) {
    GWEN_SocketSet_free(rset);
    GWEN_SocketSet_free(wset);
    DBG_DEBUG(GWEN_LOGDOMAIN, "Change in any connection");
    return GWEN_NetConnectionWorkResult_Change;
  }
  if (errors==GWEN_NetConnection_List_GetCount(connList)) {
    GWEN_SocketSet_free(rset);
    GWEN_SocketSet_free(wset);
    DBG_ERROR( GWEN_LOGDOMAIN, "Not a single connection succeeded");
    return GWEN_NetConnectionWorkResult_Error;
  }

  /* ------------------------------------------------------ sample sockets */
  curr=GWEN_NetConnection_List_First(connList);
  while(curr) {
    GWEN_NETTRANSPORT_STATUS st;
    GWEN_TYPE_UINT32 connCheckValue;

    st=GWEN_NetTransport_GetStatus(curr->transportLayer);
    if (st!=GWEN_NetTransportStatusUnconnected &&
        st!=GWEN_NetTransportStatusPDisconnected &&
        st!=GWEN_NetTransportStatusDisabled) {
      connCheckValue=GWEN_NetConnection_Check(curr);

      if (GWEN_RingBuffer_GetBytesLeft(curr->readBuffer) ||
          (curr->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTREAD) ||
          (connCheckValue & GWEN_NETCONNECTION_CHECK_WANTREAD)) {
        DBG_VERBOUS(GWEN_LOGDOMAIN, "Adding Read socket");

        /* add read sockets */
        if (GWEN_NetTransport_AddSockets(curr->transportLayer, rset, 1)) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Could not add read sockets");
          GWEN_NetConnection_Dump(curr);
          errors++;
        }
      }
      else {
        DBG_VERBOUS(GWEN_LOGDOMAIN, "Not adding read socket:");
      }

      if (GWEN_RingBuffer_GetUsedBytes(curr->writeBuffer) ||
          (curr->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTWRITE) ||
          GWEN_NetMsg_List_GetCount(curr->outMsgs) ||
          (connCheckValue & GWEN_NETCONNECTION_CHECK_WANTWRITE)) {
        DBG_VERBOUS(GWEN_LOGDOMAIN, "Adding write socket");

        /* add write sockets */
        if (GWEN_NetTransport_AddSockets(curr->transportLayer, wset, 0)) {
          DBG_INFO(GWEN_LOGDOMAIN, "Could not add write sockets");
          errors++;
        }
      }
      else {
	DBG_VERBOUS(GWEN_LOGDOMAIN,
                    "Not adding write socket (last result was %d)",
                    curr->lastResult);
      }
    }
    else {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Inactive connection:");
    }
    curr=GWEN_NetConnection_List_Next(curr);
  } /* while */


  /* -------------------------------------------------------------- select */
  /* check whether we have sockets for a select */
  if ((GWEN_SocketSet_GetSocketCount(rset)+
       GWEN_SocketSet_GetSocketCount(wset))==0) {
    /* no socket, so sleep to reduce CPU usage */
    if (timeout) {
      /* well, actually only sleep if the caller wanted a timeout */
      DBG_DEBUG(GWEN_LOGDOMAIN, "Sleeping");
      GWEN_Socket_Select(0, 0, 0, GWEN_NETCONNECTION_CPU_TIMEOUT);
      GWEN_SocketSet_free(rset);
      GWEN_SocketSet_free(wset);
      DBG_ERROR(GWEN_LOGDOMAIN, "No socket");
      return GWEN_NetConnectionWorkResult_Error;
    }
  }
  else {
    GWEN_ERRORCODE err;

    err=GWEN_Socket_Select(rset, wset, 0, timeout);
    if (!GWEN_Error_IsOk(err)) {
      if (GWEN_Error_GetType(err)!=
	  GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
          (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
           GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
	DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
	GWEN_SocketSet_free(rset);
	GWEN_SocketSet_free(wset);
        DBG_ERROR(GWEN_LOGDOMAIN, "Error on socket");
	return GWEN_NetConnectionWorkResult_Error;
      }
      else {
	DBG_DEBUG(GWEN_LOGDOMAIN, "Timeout or interrupted");
	GWEN_SocketSet_free(rset);
        GWEN_SocketSet_free(wset);
        if (changes)
          return GWEN_NetConnectionWorkResult_Change;
        else
          return GWEN_NetConnectionWorkResult_NoChange; /* FIXME: Change? */
      }
    }
  }
  GWEN_SocketSet_free(rset);
  GWEN_SocketSet_free(wset);

  /* -------------------------------------- let all connections work again */
  curr=GWEN_NetConnection_List_First(connList);
  while(curr) {
    GWEN_NETTRANSPORT_STATUS st;

    st=GWEN_NetConnection_GetStatus(curr);
    if (st!=GWEN_NetTransportStatusDisabled &&
        st!=GWEN_NetTransportStatusUnconnected) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Working on connection...");
      rv=GWEN_NetConnection_Work(curr);
      if (rv==GWEN_NetConnectionWorkResult_Error) {
        DBG_INFO(GWEN_LOGDOMAIN, "Error working (result was %d)", rv);
        errors++;
      }
      else if (rv==GWEN_NetConnectionWorkResult_Change) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "Change in connection");
        changes++;
      }
    }
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Skipping inactive connection");
    }

    curr=GWEN_NetConnection_List_Next(curr);
  } /* while */

  if (errors) {
    DBG_INFO(GWEN_LOGDOMAIN, "Got %d errors", errors);
    return -1;
  }
  if (changes)
    return GWEN_NetConnectionWorkResult_Change;

  return GWEN_NetConnectionWorkResult_NoChange;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETCONNECTION_WORKRESULT
GWEN_NetConnection_Walk(GWEN_NETCONNECTION_LIST *connList,
                        int timeout) {
  GWEN_TIME *t0;
  int distance;
  int count;
  GWEN_NETCONNECTION_WORKRESULT rv;
  time_t startt;

  t0=GWEN_CurrentTime();
  assert(t0);

  startt=time(0);

  if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE)
    distance=GWEN_NETCONNECTION_TIMEOUT_NONE;
  else if (timeout==GWEN_NETCONNECTION_TIMEOUT_FOREVER)
    distance=GWEN_NETCONNECTION_TIMEOUT_FOREVER;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance)>timeout)
        distance=timeout;
    if (!distance)
      distance=50;
  }

  for (count=0;;count++) {
    if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
      DBG_ERROR(GWEN_LOGDOMAIN, "User aborted via waitcallback");
      return GWEN_NetConnectionWorkResult_Error;
    }

    rv=GWEN_NetConnection__Walk(connList, distance);
    if (rv==GWEN_NetConnectionWorkResult_Error) {
      if (count==0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here");
	GWEN_Time_free(t0);
	return rv;
      }
      else {
	/* not the first call, so this doesn't need to be an error
	 * However, this means previous calls yielded no error, so there must
	 * have been any kind of change if we now got an error.
         */
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Hmm, not a real error, will return \"change\" instead");
	GWEN_Time_free(t0);
	return GWEN_NetConnectionWorkResult_Change;
      }
    }
    else if (rv==GWEN_NetConnectionWorkResult_Change) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Walk done");
      GWEN_Time_free(t0);
      return rv;
    }
    else {
      /* check timeout */
      if (timeout!=GWEN_NETCONNECTION_TIMEOUT_FOREVER) {
        GWEN_TIME *t1;
        double d;

        if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE) {
          return GWEN_NetConnectionWorkResult_NoChange;
        }
        t1=GWEN_CurrentTime();
        assert(t1);
        d=GWEN_Time_Diff(t1, t0);
        GWEN_Time_free(t1);

        if (d>=timeout) {
          DBG_DEBUG(GWEN_LOGDOMAIN,
                    "Could not walk within %d milliseconds, giving up",
		    timeout);
          GWEN_Time_free(t0);
          return GWEN_NetConnectionWorkResult_NoChange;
        }
      }
      else {
        double d;

        d=difftime(time(0), startt);
      }
    }
  } /* for */

  DBG_WARN(GWEN_LOGDOMAIN, "We should never reach this point");
  GWEN_Time_free(t0);
  return GWEN_NetConnectionWorkResult_Error;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_NetConnection_GetFlags(const GWEN_NETCONNECTION *conn){
  assert(conn);
  return GWEN_NetTransport_GetFlags(conn->transportLayer);
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_SetFlags(GWEN_NETCONNECTION *conn,
                                 GWEN_TYPE_UINT32 fl){
  assert(conn);
  GWEN_NetTransport_SetFlags(conn->transportLayer, fl);
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_Attach(GWEN_NETCONNECTION *conn){
  assert(conn);
  DBG_MEM_INC("GWEN_NETCONNECTION", 1);
  conn->usage++;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32
GWEN_NetConnection_GetLibraryMark(const GWEN_NETCONNECTION *conn){
  assert(conn);
  return conn->libraryMark;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32
GWEN_NetConnection_GetUserMark(const GWEN_NETCONNECTION *conn){
  assert(conn);
  return conn->userMark;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_SetUserMark(GWEN_NETCONNECTION *conn,
                                    GWEN_TYPE_UINT32 m){
  assert(conn);
  conn->userMark=m;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETTRANSPORT_STATUS
GWEN_NetConnection_GetStatus(const GWEN_NETCONNECTION *conn){
  assert(conn);
  assert(conn->transportLayer);
  return GWEN_NetTransport_GetStatus(conn->transportLayer);
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_SetStatus(GWEN_NETCONNECTION *conn,
                                  GWEN_NETTRANSPORT_STATUS nst){
  assert(conn);
  assert(conn->transportLayer);
  GWEN_NetTransport_SetStatus(conn->transportLayer, nst);
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_SetDownAfterSend(GWEN_NETCONNECTION *conn, int i){
  assert(conn);
  conn->downAfterSend=i;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnection_GetDownAfterSend(GWEN_NETCONNECTION *conn){
  assert(conn);
  return conn->downAfterSend;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnection_SetCheckFn(GWEN_NETCONNECTION *conn,
                                   GWEN_NETCONNECTION_CHECKFN fn){
  assert(conn);
  conn->checkFn=fn;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_NetConnection_Check(GWEN_NETCONNECTION *conn){
  assert(conn);
  if (conn->checkFn)
    return conn->checkFn(conn);
  else
    return 0;
}



void GWEN_NetConnection_Dump(const GWEN_NETCONNECTION *conn) {
  if (conn) {
    GWEN_NETMSG *m;
    const char *s;
    GWEN_TYPE_UINT32 fl;

    fprintf(stderr, "--------------------------------\n");
    fprintf(stderr, "Net Connection\n");
    fprintf(stderr, "Pointer        : %p\n", (void*)conn);
    fprintf(stderr, "Usage          : %d\n", conn->usage);
    fprintf(stderr, "Library mark   : %d\n", conn->libraryMark);
    fprintf(stderr, "User mark      : %d\n", conn->userMark);
    fprintf(stderr, "Last result    : %d\n", conn->lastResult);
    fprintf(stderr, "Status         : ");
    switch(GWEN_NetConnection_GetStatus(conn)) {
    case GWEN_NetTransportStatusUnconnected:
      s="Unconnected";
      break;
    case GWEN_NetTransportStatusPConnecting:
      s="Connecting physically";
      break;
    case GWEN_NetTransportStatusPConnected:
      s="Physically connected";
      break;
    case GWEN_NetTransportStatusLConnecting:
      s="Connecting logically";
      break;
    case GWEN_NetTransportStatusLConnected:
      s="Logically connected";
      break;
    case GWEN_NetTransportStatusLDisconnecting:
      s="Disconnecting logically";
      break;
    case GWEN_NetTransportStatusLDisconnected:
      s="Logically disconnected";
      break;
    case GWEN_NetTransportStatusPDisconnecting:
      s="Physically disconnecting";
      break;
    case GWEN_NetTransportStatusPDisconnected:
      s="Physically disconnected";
      break;
    case GWEN_NetTransportStatusListening:
      s="Listening";
      break;
    case GWEN_NetTransportStatusDisabled:
      s="Disabled";
      break;
    default:
      s="Unknown";
    }
    fprintf(stderr, "%s\n", s);
    fprintf(stderr, "Down after send: %s\n",
            (conn->downAfterSend)?"yes":"no");
    fprintf(stderr, "IO-Flags       :");
    if (conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTREAD)
      fprintf(stderr, " WANTREAD");
    if (conn->ioFlags & GWEN_NETCONNECTION_IOFLAG_WANTWRITE)
      fprintf(stderr, " WANTWRITE");
    fprintf(stderr, "\n");

    fl=GWEN_NetTransport_GetFlags(conn->transportLayer);
    fprintf(stderr, "Flags          :");
    if (fl & GWEN_NETTRANSPORT_FLAGS_PASSIVE)
      fprintf(stderr, " PASSIVE");
    if (fl & GWEN_NETTRANSPORT_FLAGS_EOF_IN)
      fprintf(stderr, " EOF_IN");
    if (fl & GWEN_NETTRANSPORT_FLAGS_EOF_OUT)
      fprintf(stderr, " EOF_OUT");
    if (fl & GWEN_NETTRANSPORT_FLAGS_RESTARTABLE)
      fprintf(stderr, " RESTARTABLE");
    if (fl & GWEN_NETTRANSPORT_FLAGS_WAS_LCONNECTED)
      fprintf(stderr, " WAS_LCONNECTED");
    if (fl & GWEN_NETTRANSPORT_FLAGS_WENT_DOWN)
      fprintf(stderr, " WENT_DOWN");
    fprintf(stderr, "\n");

    fprintf(stderr, "Incoming messages:\n");
    m=GWEN_NetMsg_List_First(conn->inMsgs);
    if (!m)
      fprintf(stderr, "none\n");
    while(m) {
      GWEN_NetMsg_Dump(m);
      m=GWEN_NetMsg_List_Next(m);
    }

    fprintf(stderr, "Outgoing messages:\n");
    m=GWEN_NetMsg_List_First(conn->outMsgs);
    if (!m)
      fprintf(stderr, "none\n");
    while(m) {
      GWEN_NetMsg_Dump(m);
      m=GWEN_NetMsg_List_Next(m);
    }
  }
}





