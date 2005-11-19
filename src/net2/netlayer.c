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


#include "netlayer_p.h"
#include "net2.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>


GWEN_LIST_FUNCTIONS(GWEN_NETLAYER, GWEN_NetLayer)
GWEN_LIST2_FUNCTIONS(GWEN_NETLAYER, GWEN_NetLayer)
GWEN_INHERIT_FUNCTIONS(GWEN_NETLAYER)




/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_STATUS GWEN_NetLayerStatus_fromString(const char *s) {
  assert(s);
  if (strcasecmp(s, "unconnected")==0)
    return GWEN_NetLayerStatus_Unconnected;
  else if (strcasecmp(s, "connecting")==0)
    return GWEN_NetLayerStatus_Connecting;
  else if (strcasecmp(s, "connected")==0)
    return GWEN_NetLayerStatus_Connected;
  else if (strcasecmp(s, "disconnecting")==0)
    return GWEN_NetLayerStatus_Disconnecting;
  else if (strcasecmp(s, "disconnected")==0)
    return GWEN_NetLayerStatus_Disconnected;
  else if (strcasecmp(s, "listening")==0)
    return GWEN_NetLayerStatus_Listening;
  else if (strcasecmp(s, "disabled")==0)
    return GWEN_NetLayerStatus_Disabled;
  else
    return GWEN_NetLayerStatus_Unknown;
}



/* -------------------------------------------------------------- FUNCTION */
const char *GWEN_NetLayerStatus_toString(GWEN_NETLAYER_STATUS st) {
  switch(st) {
  case GWEN_NetLayerStatus_Unconnected:
    return "unconnected";
  case GWEN_NetLayerStatus_Connecting:
    return "connecting";
  case GWEN_NetLayerStatus_Connected:
    return "connected";
  case GWEN_NetLayerStatus_Disconnecting:
    return "disconnecting";
  case GWEN_NetLayerStatus_Disconnected:
    return "disconnected";
  case GWEN_NetLayerStatus_Listening:
    return "listening";
  case GWEN_NetLayerStatus_Disabled:
    return "disabled";
  default:
    return "unknown";
  }
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayerResult_fromString(const char *s) {
  assert(s);
  if (strcasecmp(s, "idle")==0)
    return GWEN_NetLayerResult_Idle;
  else if (strcasecmp(s, "changed")==0)
    return GWEN_NetLayerResult_Changed;
  else if (strcasecmp(s, "wouldblock")==0)
    return GWEN_NetLayerResult_WouldBlock;
  else if (strcasecmp(s, "error")==0)
    return GWEN_NetLayerResult_Error;
  else
    return GWEN_NetLayerResult_Unknown;
}



/* -------------------------------------------------------------- FUNCTION */
const char *GWEN_NetLayerResult_toString(GWEN_NETLAYER_RESULT res) {
  switch(res) {
  case GWEN_NetLayerResult_Idle:
    return "idle";
  case GWEN_NetLayerResult_Changed:
    return "changed";
  case GWEN_NetLayerResult_WouldBlock:
    return "wouldblock";
  case GWEN_NetLayerResult_Error:
    return "error";
  default:
    return "unknown";
  }
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_GetPassword(GWEN_NETLAYER *nl,
                              char *buffer, int num,
                              int rwflag) {
  assert(nl);
  if (nl->getPasswordFn)
    return nl->getPasswordFn(nl, buffer, num, rwflag);
  else
    return GWEN_ERROR_UNSUPPORTED;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayer_Work(GWEN_NETLAYER *nl) {
  assert(nl);
  assert(nl->workFn);
  return nl->workFn(nl);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_Read(GWEN_NETLAYER *nl, char *buffer, int *bsize) {
  int rv;

  assert(nl);

  /* check whether layer is prepared to read body */
  rv=GWEN_NetLayer_CheckInPacket(nl);
  if (rv<0 && rv!=GWEN_ERROR_UNSUPPORTED) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  if (rv==0) {
    /* EOF */
    *bsize=0;
    return 0;
  }

  if (nl->readFn)
    return nl->readFn(nl, buffer, bsize);
  return GWEN_ERROR_UNSUPPORTED;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_Read_Wait(GWEN_NETLAYER *nl,
                            char *buffer, int *bsize,
                            int timeout) {
  time_t startt;
  int distance;
  int count;
  int rv;

  assert(nl);
  startt=time(0);

  if (timeout==GWEN_NET2_TIMEOUT_NONE)
    distance=GWEN_NET2_TIMEOUT_NONE;
  else if (timeout==GWEN_NET2_TIMEOUT_FOREVER)
    distance=GWEN_NET2_TIMEOUT_FOREVER;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance)>timeout)
        distance=timeout;
    if (!distance)
      distance=750;
  }

  for (count=0;;count++) {
    GWEN_NETLAYER_STATUS st;
    GWEN_NETLAYER_RESULT res;
    double d;
    int lsize;

    if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
      DBG_INFO(GWEN_LOGDOMAIN, "User aborted");
      return GWEN_ERROR_USER_ABORTED;
    }

    st=GWEN_NetLayer_GetStatus(nl);
    if (st!=GWEN_NetLayerStatus_Connected) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad status of netlayer: %s",
                GWEN_NetLayerStatus_toString(st));
      return GWEN_ERROR_GENERIC;
    }

    lsize=*bsize;
    rv=GWEN_NetLayer_Read(nl, buffer, &lsize);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    else if (rv==0) {
      *bsize=lsize;
      return rv;
    }

    res=GWEN_Net_HeartBeat(distance);
    if (res==GWEN_NetLayerResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }

    /* check timeout */
    d=difftime(time(0), startt);
    if (timeout!=GWEN_NET2_TIMEOUT_FOREVER) {
      if (timeout==GWEN_NET2_TIMEOUT_NONE ||
          d>timeout) {
        DBG_INFO(GWEN_LOGDOMAIN,
                 "Timeout (%d) while waiting, giving up",
                 timeout);
        return 1;
      }
    }

    if (res==GWEN_NetLayerResult_Idle && count && d) {
      int ratio;

      ratio=count/d;
      if (ratio>100) {
        /* insert sleep cycle to avoid CPU overuse which could prevent
         * the user from aborting a program running wild */
        DBG_WARN(GWEN_LOGDOMAIN,
                 "WARNING: Inserting sleep cycle, "
                 "please check the code! (%d, count=%d, dt=%lf, distance=%d)",
                 ratio, count, d, distance);
        GWEN_Socket_Select(0, 0, 0, 750);
      }
    }

  } /* for */

}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_Write(GWEN_NETLAYER *nl,
                        const char *buffer, int *bsize) {
  assert(nl);
  if (nl->writeFn)
    return nl->writeFn(nl, buffer, bsize);
  return GWEN_ERROR_UNSUPPORTED;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_Write_Wait(GWEN_NETLAYER *nl,
                             const char *buffer, int *bsize,
                             int timeout) {
  time_t startt;
  int distance;
  int count;
  int rv;

  assert(nl);
  startt=time(0);

  if (timeout==GWEN_NET2_TIMEOUT_NONE)
    distance=GWEN_NET2_TIMEOUT_NONE;
  else if (timeout==GWEN_NET2_TIMEOUT_FOREVER)
    distance=GWEN_NET2_TIMEOUT_FOREVER;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance)>timeout)
        distance=timeout;
    if (!distance)
      distance=750;
  }

  for (count=0;;count++) {
    GWEN_NETLAYER_STATUS st;
    GWEN_NETLAYER_RESULT res;
    double d;

    if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
      DBG_INFO(GWEN_LOGDOMAIN, "User aborted");
      return GWEN_ERROR_USER_ABORTED;
    }

    st=GWEN_NetLayer_GetStatus(nl);
    if (st!=GWEN_NetLayerStatus_Connected) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad status of netlayer: %s",
                GWEN_NetLayerStatus_toString(st));
      return GWEN_ERROR_GENERIC;
    }

    rv=GWEN_NetLayer_Write(nl, buffer, bsize);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    else if (rv==0)
      return rv;

    res=GWEN_Net_HeartBeat(distance);
    if (res==GWEN_NetLayerResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }

    /* check timeout */
    d=difftime(time(0), startt);
    if (timeout!=GWEN_NET2_TIMEOUT_FOREVER) {
      if (timeout==GWEN_NET2_TIMEOUT_NONE ||
          d>timeout) {
        DBG_INFO(GWEN_LOGDOMAIN,
                 "Timeout (%d) while waiting, giving up",
                 timeout);
        return 1;
      }
    }

    if (count && d) {
      int ratio;

      ratio=count/d;
      if (ratio>100) {
        /* insert sleep cycle to avoid CPU overuse which could prevent
         * the user from aborting a program running wild */
        DBG_WARN(GWEN_LOGDOMAIN,
                 "WARNING: Inserting sleep cycle, "
                 "please check the code! (%d)", ratio);
        GWEN_Socket_Select(0, 0, 0, 750);
      }
    }

  } /* for */
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_Connect(GWEN_NETLAYER *nl) {
  assert(nl);
  if (nl->connectFn)
    return nl->connectFn(nl);
  return GWEN_ERROR_UNSUPPORTED;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_Connect_Wait(GWEN_NETLAYER *nl, int timeout) {
  time_t startt;
  int distance;
  int count;
  int rv;

  assert(nl);
  startt=time(0);

  if (timeout==GWEN_NET2_TIMEOUT_NONE)
    distance=GWEN_NET2_TIMEOUT_NONE;
  else if (timeout==GWEN_NET2_TIMEOUT_FOREVER)
    distance=GWEN_NET2_TIMEOUT_FOREVER;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance)>timeout)
        distance=timeout;
    if (!distance)
      distance=750;
  }

  rv=GWEN_NetLayer_Connect(nl);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  for (count=0;;count++) {
    GWEN_NETLAYER_STATUS st;
    GWEN_NETLAYER_RESULT res;
    double d;

    if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
      DBG_INFO(GWEN_LOGDOMAIN, "User aborted");
      return GWEN_ERROR_USER_ABORTED;
    }

    st=GWEN_NetLayer_GetStatus(nl);
    if (st==GWEN_NetLayerStatus_Connected)
      return 0;

    if (st!=GWEN_NetLayerStatus_Connecting) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad status of netlayer: %s",
                GWEN_NetLayerStatus_toString(st));
      return GWEN_ERROR_GENERIC;
    }

    res=GWEN_Net_HeartBeat(distance);
    if (res==GWEN_NetLayerResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }

    d=difftime(time(0), startt);

    /* check timeout */
    if (timeout!=GWEN_NET2_TIMEOUT_FOREVER) {
      if (timeout==GWEN_NET2_TIMEOUT_NONE ||
          d>timeout) {
        DBG_INFO(GWEN_LOGDOMAIN,
                 "Timeout (%d) while waiting, giving up",
                 timeout);
        return 1;
      }
    }

    if (count && d) {
      int ratio;

      ratio=count/d;
      if (ratio>100) {
        /* insert sleep cycle to avoid CPU overuse which could prevent
         * the user from aborting a program running wild */
        DBG_WARN(GWEN_LOGDOMAIN,
                 "WARNING: Inserting sleep cycle, "
                 "please check the code! (%d)", ratio);
        GWEN_Socket_Select(0, 0, 0, 750);
      }
    }

  } /* for */
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_Disconnect(GWEN_NETLAYER *nl) {
  assert(nl);
  if (nl->disconnectFn)
    return nl->disconnectFn(nl);
  return GWEN_ERROR_UNSUPPORTED;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_Disconnect_Wait(GWEN_NETLAYER *nl, int timeout) {
  time_t startt;
  int distance;
  int count;
  int rv;

  assert(nl);
  startt=time(0);

  if (timeout==GWEN_NET2_TIMEOUT_NONE)
    distance=GWEN_NET2_TIMEOUT_NONE;
  else if (timeout==GWEN_NET2_TIMEOUT_FOREVER)
    distance=GWEN_NET2_TIMEOUT_FOREVER;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance)>timeout)
        distance=timeout;
    if (!distance)
      distance=750;
  }

  rv=GWEN_NetLayer_Disconnect(nl);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  for (count=0;;count++) {
    GWEN_NETLAYER_STATUS st;
    GWEN_NETLAYER_RESULT res;
    double d;

    if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
      DBG_INFO(GWEN_LOGDOMAIN, "User aborted");
      return GWEN_ERROR_USER_ABORTED;
    }

    st=GWEN_NetLayer_GetStatus(nl);
    if (st==GWEN_NetLayerStatus_Disconnected)
      return 0;

    if (st!=GWEN_NetLayerStatus_Disconnecting) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad status of netlayer: %s",
                GWEN_NetLayerStatus_toString(st));
      return GWEN_ERROR_GENERIC;
    }

    res=GWEN_Net_HeartBeat(distance);
    if (res==GWEN_NetLayerResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }

    d=difftime(time(0), startt);

    /* check timeout */
    if (timeout!=GWEN_NET2_TIMEOUT_FOREVER) {
      if (timeout==GWEN_NET2_TIMEOUT_NONE ||
          d>timeout) {
        DBG_INFO(GWEN_LOGDOMAIN,
                 "Timeout (%d) while waiting, giving up",
                 timeout);
        return 1;
      }
    }

    if (count && d) {
      int ratio;

      ratio=count/d;
      if (ratio>100) {
        /* insert sleep cycle to avoid CPU overuse which could prevent
         * the user from aborting a program running wild */
        DBG_WARN(GWEN_LOGDOMAIN,
                 "WARNING: Inserting sleep cycle, "
                 "please check the code! (%d)", ratio);
        GWEN_Socket_Select(0, 0, 0, 750);
      }
    }

  } /* for */
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_Listen(GWEN_NETLAYER *nl) {
  assert(nl);
  if (nl->listenFn)
    return nl->listenFn(nl);
  return GWEN_ERROR_UNSUPPORTED;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_AddSockets(GWEN_NETLAYER *nl,
                             GWEN_SOCKETSET *readSet,
                             GWEN_SOCKETSET *writeSet,
                             GWEN_SOCKETSET *exSet) {
  assert(nl);
  if (nl->addSocketsFn)
    return nl->addSocketsFn(nl, readSet, writeSet, exSet);
  return GWEN_ERROR_UNSUPPORTED;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_BaseStatusChange(GWEN_NETLAYER *nl,
                                    GWEN_NETLAYER_STATUS newst) {
  assert(nl);
  if (nl->baseStatusChangeFn)
    nl->baseStatusChangeFn(nl, newst);
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_StatusChange(GWEN_NETLAYER *nl,
                                GWEN_NETLAYER_STATUS newst) {
  assert(nl);
  if (nl->statusChangeFn)
    nl->statusChangeFn(nl, newst);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_BeginOutPacket(GWEN_NETLAYER *nl, int totalSize) {
  assert(nl);
  nl->outBodySize=totalSize;
  if (nl->beginOutPacketFn)
    return nl->beginOutPacketFn(nl, totalSize);
  return GWEN_ERROR_UNSUPPORTED;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_EndOutPacket(GWEN_NETLAYER *nl) {
  assert(nl);
  if (nl->endOutPacketFn)
    return nl->endOutPacketFn(nl);
  return GWEN_ERROR_UNSUPPORTED;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_EndOutPacket_Wait(GWEN_NETLAYER *nl, int timeout) {
  time_t startt;
  int distance;
  int count;
  int rv;

  assert(nl);
  startt=time(0);

  if (timeout==GWEN_NET2_TIMEOUT_NONE)
    distance=GWEN_NET2_TIMEOUT_NONE;
  else if (timeout==GWEN_NET2_TIMEOUT_FOREVER)
    distance=GWEN_NET2_TIMEOUT_FOREVER;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance)>timeout)
        distance=timeout;
    if (!distance)
      distance=750;
  }

  for (count=0;;count++) {
    GWEN_NETLAYER_STATUS st;
    GWEN_NETLAYER_RESULT res;
    double d;

    if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
      DBG_INFO(GWEN_LOGDOMAIN, "User aborted");
      return GWEN_ERROR_USER_ABORTED;
    }

    st=GWEN_NetLayer_GetStatus(nl);
    if (st!=GWEN_NetLayerStatus_Connected) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad status of netlayer: %s",
                GWEN_NetLayerStatus_toString(st));
      return GWEN_ERROR_GENERIC;
    }

    rv=GWEN_NetLayer_EndOutPacket(nl);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    else if (rv==0)
      return rv;

    res=GWEN_Net_HeartBeat(distance);
    if (res==GWEN_NetLayerResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }

    /* check timeout */
    d=difftime(time(0), startt);
    if (timeout!=GWEN_NET2_TIMEOUT_FOREVER) {
      if (timeout==GWEN_NET2_TIMEOUT_NONE ||
          d>timeout) {
        DBG_INFO(GWEN_LOGDOMAIN,
                 "Timeout (%d) while waiting, giving up",
                 timeout);
        return 1;
      }
    }

    if (count && d) {
      int ratio;

      ratio=count/d;
      if (ratio>100) {
        /* insert sleep cycle to avoid CPU overuse which could prevent
         * the user from aborting a program running wild */
        DBG_WARN(GWEN_LOGDOMAIN,
                 "WARNING: Inserting sleep cycle, "
                 "please check the code! (%d)", ratio);
        GWEN_Socket_Select(0, 0, 0, 750);
      }
    }

  } /* for */
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_BeginInPacket(GWEN_NETLAYER *nl) {
  assert(nl);
  nl->inBodySize=-1;
  if (nl->beginInPacketFn)
    return nl->beginInPacketFn(nl);
  return GWEN_ERROR_UNSUPPORTED;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_CheckInPacket(GWEN_NETLAYER *nl) {
  assert(nl);
  assert(nl->usage);
  if (nl->checkInPacketFn)
    return nl->checkInPacketFn(nl);
  return GWEN_ERROR_UNSUPPORTED;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetGetPasswordFn(GWEN_NETLAYER *nl,
                                    GWEN_NETLAYER_GETPASSWD_FN f) {
  assert(nl);
  nl->getPasswordFn=f;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetWorkFn(GWEN_NETLAYER *nl, GWEN_NETLAYER_WORK_FN f) {
  assert(nl);
  nl->workFn=f;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetReadFn(GWEN_NETLAYER *nl, GWEN_NETLAYER_READ_FN f){
  assert(nl);
  nl->readFn=f;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetWriteFn(GWEN_NETLAYER *nl, GWEN_NETLAYER_WRITE_FN f) {
  assert(nl);
  nl->writeFn=f;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetConnectFn(GWEN_NETLAYER *nl,
                                GWEN_NETLAYER_CONNECT_FN f) {
  assert(nl);
  nl->connectFn=f;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetDisconnectFn(GWEN_NETLAYER *nl,
                                   GWEN_NETLAYER_DISCONNECT_FN f){
  assert(nl);
  nl->disconnectFn=f;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetListenFn(GWEN_NETLAYER *nl, GWEN_NETLAYER_LISTEN_FN f){
  assert(nl);
  nl->listenFn=f;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetAddSocketsFn(GWEN_NETLAYER *nl,
                                   GWEN_NETLAYER_ADDSOCKETS_FN f){
  assert(nl);
  nl->addSocketsFn=f;
}



/* -------------------------------------------------------------- FUNCTION */
void
GWEN_NetLayer_SetBaseStatusChangeFn(GWEN_NETLAYER *nl,
                                    GWEN_NETLAYER_BASESTATUS_CHG_FN f){
  assert(nl);
  nl->baseStatusChangeFn=f;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetStatusChangeFn(GWEN_NETLAYER *nl,
                                     GWEN_NETLAYER_STATUS_CHG_FN f) {
  assert(nl);
  nl->statusChangeFn=f;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetBeginOutPacketFn(GWEN_NETLAYER *nl,
					GWEN_NETLAYER_BEGIN_OUT_PACKET_FN f){
  assert(nl);
  nl->beginOutPacketFn=f;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetEndOutPacketFn(GWEN_NETLAYER *nl,
				      GWEN_NETLAYER_END_OUT_PACKET_FN f){
  assert(nl);
  nl->endOutPacketFn=f;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetBeginInPacketFn(GWEN_NETLAYER *nl,
                                      GWEN_NETLAYER_BEGIN_IN_PACKET_FN f){
  assert(nl);
  nl->beginInPacketFn=f;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetCheckInPacketFn(GWEN_NETLAYER *nl,
                                      GWEN_NETLAYER_CHECK_IN_PACKET_FN f){
  assert(nl);
  nl->checkInPacketFn=f;
}












/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER *GWEN_NetLayer_new(const char *typeName) {
  GWEN_NETLAYER *nl;

  assert(typeName);
  GWEN_NEW_OBJECT(GWEN_NETLAYER, nl)
  nl->usage=1;
  DBG_MEM_INC("GWEN_NETLAYER", 0);
  GWEN_INHERIT_INIT(GWEN_NETLAYER, nl)
  GWEN_LIST_INIT(GWEN_NETLAYER, nl)

  nl->incomingLayers=GWEN_NetLayer_List_new();
  nl->backLog=GWEN_NETLAYER_DEF_BACKLOG;
  nl->typeName=strdup(typeName);
  return nl;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_free(GWEN_NETLAYER *nl) {
  if (nl) {
    assert(nl->usage);
    if (nl->usage==1) {
      GWEN_LIST_FINI(GWEN_NETLAYER, nl)
      GWEN_INHERIT_FINI(GWEN_NETLAYER, nl)
      GWEN_InetAddr_free(nl->peerAddr);
      GWEN_InetAddr_free(nl->localAddr);
      GWEN_NetLayer_List_free(nl->incomingLayers);
      if (nl->baseLayer)
        nl->baseLayer->parentLayer=0;
      GWEN_NetLayer_free(nl->baseLayer);
      free(nl->typeName);
      nl->usage=0;
      DBG_MEM_DEC("GWEN_NETLAYER");
      GWEN_FREE_OBJECT(nl);
    }
    else
      nl->usage--;
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_Attach(GWEN_NETLAYER *nl) {
  assert(nl);
  assert(nl->usage);
  DBG_MEM_INC("GWEN_NETLAYER", 1);
  nl->usage++;
}



/* -------------------------------------------------------------- FUNCTION */
const char *GWEN_NetLayer_GetTypeName(const GWEN_NETLAYER *nl) {
  assert(nl);
  return nl->typeName;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER *GWEN_NetLayer_GetBaseLayer(const GWEN_NETLAYER *nl) {
  assert(nl);
  return nl->baseLayer;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetBaseLayer(GWEN_NETLAYER *nl, GWEN_NETLAYER *baseLayer){
  assert(nl);
  if (baseLayer)
    GWEN_NetLayer_Attach(baseLayer);
  GWEN_NetLayer_free(nl->baseLayer);
  nl->baseLayer=baseLayer;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER *GWEN_NetLayer_GetParentLayer(const GWEN_NETLAYER *nl) {
  assert(nl);
  return nl->parentLayer;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetParentLayer(GWEN_NETLAYER *nl, GWEN_NETLAYER *pLayer){
  assert(nl);
  nl->parentLayer=pLayer;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_AddIncomingLayer(GWEN_NETLAYER *nl, GWEN_NETLAYER *newnl){
  assert(nl);
  GWEN_NetLayer_List_Add(newnl, nl->incomingLayers);
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER *GWEN_NetLayer_PeekIncomingLayer(const GWEN_NETLAYER *nl) {
  assert(nl);
  return GWEN_NetLayer_List_First(nl->incomingLayers);
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER *GWEN_NetLayer_GetIncomingLayer(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *newnl;

  assert(nl);
  newnl=GWEN_NetLayer_List_First(nl->incomingLayers);
  if (newnl) {
    GWEN_NetLayer_List_Del(newnl);
    return newnl;
  }
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER *GWEN_NetLayer_GetIncomingLayer_Wait(GWEN_NETLAYER *nl,
                                                   int timeout) {
  time_t startt;
  int distance;
  int count;

  assert(nl);
  startt=time(0);

  if (timeout==GWEN_NET2_TIMEOUT_NONE)
    distance=GWEN_NET2_TIMEOUT_NONE;
  else if (timeout==GWEN_NET2_TIMEOUT_FOREVER)
    distance=GWEN_NET2_TIMEOUT_FOREVER;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance)>timeout)
        distance=timeout;
    if (!distance)
      distance=750;
  }

  for (count=0;;count++) {
    GWEN_NETLAYER_STATUS st;
    GWEN_NETLAYER_RESULT res;
    GWEN_NETLAYER *newNl;
    double d;

    if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
      DBG_INFO(GWEN_LOGDOMAIN, "User aborted");
      return 0;
    }

    st=GWEN_NetLayer_GetStatus(nl);
    if (st!=GWEN_NetLayerStatus_Listening) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad status of netlayer: %s",
                GWEN_NetLayerStatus_toString(st));
      return 0;
    }

    newNl=GWEN_NetLayer_GetIncomingLayer(nl);
    if (newNl)
      return newNl;

    res=GWEN_Net_HeartBeat(distance);
    if (res==GWEN_NetLayerResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return 0;
    }

    d=difftime(time(0), startt);

    /* check timeout */
    if (timeout!=GWEN_NET2_TIMEOUT_FOREVER) {
      if (timeout==GWEN_NET2_TIMEOUT_NONE ||
          d>timeout) {
        DBG_INFO(GWEN_LOGDOMAIN,
                 "Timeout (%d) while waiting, giving up",
                 timeout);
        return 0;
      }
    }

    if (count && d) {
      int ratio;

      ratio=count/d;
      if (ratio>100) {
        /* insert sleep cycle to avoid CPU overuse which could prevent
         * the user from aborting a program running wild */
        DBG_WARN(GWEN_LOGDOMAIN,
                 "WARNING: Inserting sleep cycle, "
                 "please check the code! (%d)", ratio);
        GWEN_Socket_Select(0, 0, 0, 750);
      }
    }
  } /* for */
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_GetIncomingLayerCount(const GWEN_NETLAYER *nl) {
  assert(nl);
  return GWEN_NetLayer_List_GetCount(nl->incomingLayers);
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_DelIncomingLayers(GWEN_NETLAYER *nl) {
  assert(nl);
  GWEN_NetLayer_List_Clear(nl->incomingLayers);
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_STATUS GWEN_NetLayer_GetStatus(const GWEN_NETLAYER *nl) {
  assert(nl);
  return nl->status;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetStatus(GWEN_NETLAYER *nl, GWEN_NETLAYER_STATUS st) {
  assert(nl);
  if (nl->status!=st) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Changing status of \"%s\" from \"%s\" to \"%s\"",
             nl->typeName,
             GWEN_NetLayerStatus_toString(nl->status),
             GWEN_NetLayerStatus_toString(st));
    nl->lastStatusChange=time(0);
    /* inform callback functions */
    GWEN_NetLayer_StatusChange(nl, st);
    if (nl->parentLayer)
      GWEN_NetLayer_BaseStatusChange(nl->parentLayer, st);
    nl->status=st;
  }
}



/* -------------------------------------------------------------- FUNCTION */
time_t GWEN_NetLayer_GetLastStatusChangeTime(const GWEN_NETLAYER *nl) {
  assert(nl);
  return nl->lastStatusChange;
}



/* -------------------------------------------------------------- FUNCTION */
const GWEN_INETADDRESS *GWEN_NetLayer_GetLocalAddr(const GWEN_NETLAYER *nl){
  assert(nl);
  return nl->localAddr;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetLocalAddr(GWEN_NETLAYER *nl,
                                const GWEN_INETADDRESS *addr) {
  assert(nl);
  if (nl->localAddr!=addr) {
    GWEN_InetAddr_free(nl->localAddr);
    if (addr)
      nl->localAddr=GWEN_InetAddr_dup(addr);
    else
      nl->localAddr=0;
  }
}



/* -------------------------------------------------------------- FUNCTION */
const GWEN_INETADDRESS *GWEN_NetLayer_GetPeerAddr(const GWEN_NETLAYER *nl) {
  assert(nl);
  return nl->peerAddr;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetPeerAddr(GWEN_NETLAYER *nl,
                               const GWEN_INETADDRESS *addr) {
  assert(nl);
  if (nl->peerAddr!=addr) {
    GWEN_InetAddr_free(nl->peerAddr);
    if (addr)
      nl->peerAddr=GWEN_InetAddr_dup(addr);
    else
      nl->peerAddr=0;
  }
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_NetLayer_GetFlags(const GWEN_NETLAYER *nl) {
  assert(nl);
  return nl->flags;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetFlags(GWEN_NETLAYER *nl, GWEN_TYPE_UINT32 fl) {
  assert(nl);
  nl->flags=fl;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_AddFlags(GWEN_NETLAYER *nl, GWEN_TYPE_UINT32 fl) {
  assert(nl);
  nl->flags|=fl;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SubFlags(GWEN_NETLAYER *nl, GWEN_TYPE_UINT32 fl) {
  assert(nl);
  nl->flags&=~fl;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_GetBackLog(const GWEN_NETLAYER *nl) {
  assert(nl);
  return nl->backLog;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetBackLog(GWEN_NETLAYER *nl, int i) {
  assert(nl);
  nl->backLog=i;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_GetInBodySize(const GWEN_NETLAYER *nl) {
  assert(nl);
  return nl->inBodySize;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetInBodySize(GWEN_NETLAYER *nl, int i) {
  assert(nl);
  nl->inBodySize=i;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_GetOutBodySize(const GWEN_NETLAYER *nl) {
  assert(nl);
  return nl->outBodySize;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayer_SetOutBodySize(GWEN_NETLAYER *nl, int i) {
  assert(nl);
  nl->outBodySize=i;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayer__Wait(GWEN_NETLAYER_LIST *nll,
                                         int timeout) {
  GWEN_NETLAYER *nl;
  GWEN_SOCKETSET *rset, *wset, *xset;

  rset=GWEN_SocketSet_new();
  wset=GWEN_SocketSet_new();
  xset=GWEN_SocketSet_new();

  nl=GWEN_NetLayer_List_First(nll);
  while(nl) {
    GWEN_NETLAYER_STATUS st;

    st=GWEN_NetLayer_GetStatus(nl);
    if (st!=GWEN_NetLayerStatus_Unconnected &&
        st!=GWEN_NetLayerStatus_Disconnecting &&
        st!=GWEN_NetLayerStatus_Disabled) {
      GWEN_NetLayer_AddSockets(nl, rset, wset, xset);
    }
    nl=GWEN_NetLayer_List_Next(nl);
  }

  /* check whether we have sockets for a select */
  if ((GWEN_SocketSet_GetSocketCount(rset)+
       GWEN_SocketSet_GetSocketCount(wset)+
       GWEN_SocketSet_GetSocketCount(xset))==0) {
    /* no socket, so sleep to reduce CPU usage */
    GWEN_SocketSet_free(xset);
    GWEN_SocketSet_free(wset);
    GWEN_SocketSet_free(rset);

    if (timeout) {
      /* well, actually only sleep if the caller wanted a timeout */
      DBG_DEBUG(GWEN_LOGDOMAIN, "Sleeping");
      GWEN_Socket_Select(0, 0, 0, GWEN_NETLAYER_CPU_TIMEOUT);
      DBG_ERROR(GWEN_LOGDOMAIN, "No socket");
      return GWEN_NetLayerResult_Error;
    }
    return GWEN_NetLayerResult_Idle;
  }
  else {
    GWEN_ERRORCODE err;

    err=GWEN_Socket_Select(rset, wset, xset, timeout);
    if (!GWEN_Error_IsOk(err)) {
      GWEN_SocketSet_free(xset);
      GWEN_SocketSet_free(wset);
      GWEN_SocketSet_free(rset);
      if (GWEN_Error_GetType(err)!=
          GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
          (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
           GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
        DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
        DBG_ERROR(GWEN_LOGDOMAIN, "Error on socket");
        return GWEN_NetLayerResult_Error;
      }
      else {
        DBG_DEBUG(GWEN_LOGDOMAIN, "Timeout or interrupted");
        return GWEN_NetLayerResult_Idle;
      }
    }
    /* TODO: Maybe ask all netlayers to check for their sockets */
    GWEN_SocketSet_free(xset);
    GWEN_SocketSet_free(wset);
    GWEN_SocketSet_free(rset);
    return GWEN_NetLayerResult_Changed;
  }
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayer__WorkAll(GWEN_NETLAYER_LIST *nll) {
  GWEN_NETLAYER *nl;
  GWEN_NETLAYER_RESULT bres=GWEN_NetLayerResult_Idle;
  int errors=0;

  nl=GWEN_NetLayer_List_First(nll);
  while(nl) {
    GWEN_NETLAYER_STATUS st;

    st=GWEN_NetLayer_GetStatus(nl);
    if (st!=GWEN_NetLayerStatus_Unconnected &&
        st!=GWEN_NetLayerStatus_Disconnecting &&
        st!=GWEN_NetLayerStatus_Disabled) {
      GWEN_NETLAYER_RESULT res;

      res=GWEN_NetLayer_Work(nl);
      if (res==GWEN_NetLayerResult_Error) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        errors++;
      }
      else {
        /* merge in new result */
        if (bres==GWEN_NetLayerResult_Idle)
          bres=res;
        else if (bres!=GWEN_NetLayerResult_Changed) {
          if (res==GWEN_NetLayerResult_Changed)
            bres=GWEN_NetLayerResult_Changed;
        }
      }
    }
    nl=GWEN_NetLayer_List_Next(nl);
  }

  if (errors && (errors==GWEN_NetLayer_List_GetCount(nll))) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Errors on all netlayers");
    return GWEN_NetLayerResult_Error;
  }

  return bres;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayer_Walk(GWEN_NETLAYER_LIST *nll,
                                        int timeout) {
  GWEN_NETLAYER_RESULT res;

  res=GWEN_NetLayer__WorkAll(nll);
  if (res==GWEN_NetLayerResult_Changed ||
      res==GWEN_NetLayerResult_Error)
    return res;

  res=GWEN_NetLayer__Wait(nll, timeout);
  if (res!=GWEN_NetLayerResult_Changed)
    return res;

  res=GWEN_NetLayer__WorkAll(nll);
  return res;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER *GWEN_NetLayer_FindBaseLayer(const GWEN_NETLAYER *nl,
					   const char *tname) {
  GWEN_NETLAYER *x;

  assert(nl);
  x=nl->baseLayer;
  while(x) {
    if (strcasecmp(x->typeName, tname)==0)
      break;
    x=x->baseLayer;
  } /* while */

  return x;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_RecvPacket(GWEN_NETLAYER *nl, GWEN_BUFFER *mbuf,
			     int timeout) {
  static char buffer[512];
  int bsize;
  int rv;
  time_t startt;
  int tLeft;

  startt=time(0);

  rv=GWEN_NetLayer_BeginInPacket(nl);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not start to read (%d)", rv);
    return rv;
  }

  for (;;) {
    if (timeout!=GWEN_NET2_TIMEOUT_NONE &&
	timeout!=GWEN_NET2_TIMEOUT_FOREVER) {
      tLeft=timeout-(difftime(time(0), startt));
      if (tLeft<1)
	tLeft=1;
    }
    else
      tLeft=timeout;
    rv=GWEN_NetLayer_CheckInPacket(nl);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error checking packet (%d)", rv);
      return rv;
    }
    else if (rv==1) {
      bsize=sizeof(buffer)-1;
      DBG_DEBUG(GWEN_LOGDOMAIN, "Reading %d bytes", bsize);
      rv=GWEN_NetLayer_Read_Wait(nl, buffer, &bsize, tLeft);
      if (rv<0) {
	DBG_ERROR(GWEN_LOGDOMAIN, "ERROR: Could not read (%d)", rv);
        return rv;
      }
      else if (rv==1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "ERROR: Could not read due to a timeout");
        return GWEN_ERROR_TIMEOUT;
      }
      else {
        if (bsize==0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "INFO: EOF met");
	  break;
	}
        else {
          buffer[bsize]=0;
	  if (bsize) {
	    GWEN_Buffer_AppendBytes(mbuf, buffer, bsize);
	  }
	}
      }
    }
    else
      break;
  } /* for */

  DBG_INFO(GWEN_LOGDOMAIN, "Packet received");
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_SendPacket(GWEN_NETLAYER *nl,
			     const char *dPtr, int dLen,
			     int timeout) {
  int rv;
  time_t startt;
  int tLeft;
  GWEN_NETLAYER_RESULT res;

  startt=time(0);

  res=GWEN_NetLayer_Work(nl);
  if (res==GWEN_NetLayerResult_Error) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return -1;
  }

  if (nl->status!=GWEN_NetLayerStatus_Connected) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not connected");
    return GWEN_ERROR_NOT_CONNECTED;
  }

  rv=GWEN_NetLayer_BeginOutPacket(nl, dLen);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "ERROR: Could not begin packet (%d)", rv);
    return rv;
  }
  if (dLen>0) {
    const char *p;
    int bLeft;

    p=dPtr;
    bLeft=dLen;
    while(bLeft>0) {
      int bsize;

      if (timeout!=GWEN_NET2_TIMEOUT_NONE &&
	  timeout!=GWEN_NET2_TIMEOUT_FOREVER) {
	tLeft=timeout-(difftime(time(0), startt));
	if (tLeft<1)
	  tLeft=1;
      }
      else
        tLeft=timeout;

      bsize=bLeft;
      rv=GWEN_NetLayer_Write_Wait(nl, p, &bsize, tLeft);
      if (rv) {
	DBG_ERROR(GWEN_LOGDOMAIN, "ERROR: Could not write (%d)", rv);
	return rv;
      }
      bLeft-=bsize;
      p+=bsize;
    } /* while */
  }

  if (timeout!=GWEN_NET2_TIMEOUT_NONE &&
      timeout!=GWEN_NET2_TIMEOUT_FOREVER) {
    tLeft=timeout-(difftime(time(0), startt));
    if (tLeft<1)
      tLeft=1;
  }
  else
    tLeft=timeout;

  rv=GWEN_NetLayer_EndOutPacket_Wait(nl, tLeft);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "ERROR: Could not end packet (%d)", rv);
    return rv;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Packet sent.");
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_RecvPacketBio(GWEN_NETLAYER *nl,
                                GWEN_BUFFEREDIO *bio,
				int timeout) {
  static char buffer[512];
  int bsize;
  int rv;
  time_t startt;
  int tLeft;
  GWEN_ERRORCODE err;

  startt=time(0);

  rv=GWEN_NetLayer_BeginInPacket(nl);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not start to read (%d)", rv);
    return rv;
  }

  for (;;) {
    if (timeout!=GWEN_NET2_TIMEOUT_NONE &&
	timeout!=GWEN_NET2_TIMEOUT_FOREVER) {
      tLeft=timeout-(difftime(time(0), startt));
      if (tLeft<1)
	tLeft=1;
    }
    else
      tLeft=timeout;
    rv=GWEN_NetLayer_CheckInPacket(nl);
    DBG_INFO(GWEN_LOGDOMAIN, "Check-Result: %d", rv);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error checking packet (%d)", rv);
      return rv;
    }
    else if (rv==1) {
      DBG_INFO(GWEN_LOGDOMAIN, "Reading");
      bsize=sizeof(buffer);
      rv=GWEN_NetLayer_Read_Wait(nl, buffer, &bsize, tLeft);
      if (rv<0) {
	DBG_ERROR(GWEN_LOGDOMAIN, "ERROR: Could not read (%d)", rv);
        return rv;
      }
      else if (rv==1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "ERROR: Could not read due to a timeout");
        return GWEN_ERROR_TIMEOUT;
      }
      else {
        if (bsize==0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "INFO: EOF met");
	  break;
	}
        else {
	  if (bsize) {
            const char *p;
            int wLeft;

            wLeft=bsize;
            p=buffer;
            while(wLeft) {
              unsigned int wsize;

              wsize=wLeft;
              err=GWEN_BufferedIO_WriteRaw(bio, p, &wsize);
              if (!GWEN_Error_IsOk(err)) {
                DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
                return GWEN_Error_GetSimpleCode(err);
              }
              p+=wsize;
              wLeft-=wsize;
	    } /* while */
	  }
	}
      }
    }
    else
      break;
  } /* for */

  err=GWEN_BufferedIO_Flush(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_Error_GetSimpleCode(err);
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Packet received");
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayer_SendPacketBio(GWEN_NETLAYER *nl,
				GWEN_BUFFEREDIO *bio,
				int timeout) {
  int rv;
  time_t startt;
  int tLeft;
  char buffer[512];

  startt=time(0);

  rv=GWEN_NetLayer_BeginOutPacket(nl, -1); /* size unknown */
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "ERROR: Could not begin packet (%d)", rv);
    return rv;
  }

  for (;;) {
    unsigned int rsize;
    int wsize;
    const char *p;
    GWEN_ERRORCODE err;

    /* read from BIO */
    rsize=sizeof(buffer);
    err=GWEN_BufferedIO_ReadRaw(bio, buffer, &rsize);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
      return GWEN_Error_GetSimpleCode(err);
    }
    if (rsize==0)
      break;

    /* write to netlayer */
    p=buffer;
    wsize=rsize;
    while(wsize>0) {
      int bsize;

      if (timeout!=GWEN_NET2_TIMEOUT_NONE &&
	  timeout!=GWEN_NET2_TIMEOUT_FOREVER) {
	tLeft=timeout-(difftime(time(0), startt));
	if (tLeft<1)
	  tLeft=1;
      }
      else
	tLeft=timeout;

      bsize=wsize;
      rv=GWEN_NetLayer_Write_Wait(nl, p, &bsize, tLeft);
      if (rv) {
	DBG_ERROR(GWEN_LOGDOMAIN, "ERROR: Could not write (%d)", rv);
	return rv;
      }
      wsize-=bsize;
      p+=bsize;
    } /* while */
  }

  if (timeout!=GWEN_NET2_TIMEOUT_NONE &&
      timeout!=GWEN_NET2_TIMEOUT_FOREVER) {
    tLeft=timeout-(difftime(time(0), startt));
    if (tLeft<1)
      tLeft=1;
  }
  else
    tLeft=timeout;

  rv=GWEN_NetLayer_EndOutPacket_Wait(nl, tLeft);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "ERROR: Could not end packet (%d)", rv);
    return rv;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Packet sent.");
  return 0;
}





