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


#include "nl_packets_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/nl_packets.h>
#include <gwenhywfar/net2.h>

#include <ctype.h>


GWEN_INHERIT(GWEN_NETLAYER, GWEN_NL_PACKETS)


GWEN_LIST_FUNCTIONS(GWEN_NL_PACKET, GWEN_NL_Packet)
GWEN_INHERIT_FUNCTIONS(GWEN_NL_PACKET)




/* -------------------------------------------------------------- FUNCTION */
GWEN_NL_PACKET *GWEN_NL_Packet_new() {
  GWEN_NL_PACKET *pk;

  GWEN_NEW_OBJECT(GWEN_NL_PACKET, pk);
  GWEN_LIST_INIT(GWEN_NL_PACKET, pk);
  GWEN_INHERIT_INIT(GWEN_NL_PACKET, pk);
  pk->usage=1;

  pk->buffer=GWEN_Buffer_new(0, GWEN_NL_PACKETS_CHUNKSIZE, 0, 1);

  return pk;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NL_Packet_free(GWEN_NL_PACKET *pk) {
  if (pk) {
    assert(pk->usage);
    if (pk->usage==1) {
      GWEN_INHERIT_FINI(GWEN_NL_PACKET, pk);
      GWEN_LIST_FINI(GWEN_NL_PACKET, pk);
      GWEN_Buffer_free(pk->buffer);
      pk->usage=0;
      GWEN_FREE_OBJECT(pk);
    }
    else
      pk->usage--;
  }
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NL_Packet_Attach(GWEN_NL_PACKET *pk) {
  assert(pk);
  assert(pk->usage);
  pk->usage++;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NL_PACKET_STATUS GWEN_NL_Packet_GetStatus(const GWEN_NL_PACKET *pk) {
  assert(pk);
  return pk->status;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_BUFFER *GWEN_NL_Packet_GetBuffer(const GWEN_NL_PACKET *pk) {
  assert(pk);
  return pk->buffer;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_BUFFER *GWEN_NL_Packet_TakeBuffer(GWEN_NL_PACKET *pk) {
  GWEN_BUFFER *buf;

  assert(pk);
  buf=pk->buffer;
  pk->buffer=0;
  return buf;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NL_Packet_SetBuffer(GWEN_NL_PACKET *pk, GWEN_BUFFER *buf) {
  assert(pk);
  GWEN_Buffer_free(pk->buffer);
  pk->buffer=buf;
}








/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER *GWEN_NetLayerPackets_new(GWEN_NETLAYER *baseLayer) {
  GWEN_NETLAYER *nl;
  GWEN_NL_PACKETS *nld;

  assert(baseLayer);
  nl=GWEN_NetLayer_new(GWEN_NL_PACKETS_NAME);
  GWEN_NEW_OBJECT(GWEN_NL_PACKETS, nld);
  GWEN_INHERIT_SETDATA(GWEN_NETLAYER, GWEN_NL_PACKETS, nl, nld,
                       GWEN_NetLayerPackets_FreeData);

  GWEN_NetLayer_SetBaseLayer(nl, baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  GWEN_NetLayer_SetParentLayer(baseLayer, nl);

  GWEN_NetLayer_SetLocalAddr(nl, GWEN_NetLayer_GetLocalAddr(baseLayer));
  GWEN_NetLayer_SetPeerAddr(nl, GWEN_NetLayer_GetPeerAddr(baseLayer));

  /* this protocol is packet based */
  GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_PKG_BASED);

  GWEN_NetLayer_SetWorkFn(nl, GWEN_NetLayerPackets_Work);
  GWEN_NetLayer_SetConnectFn(nl, GWEN_NetLayerPackets_Connect);
  GWEN_NetLayer_SetDisconnectFn(nl, GWEN_NetLayerPackets_Disconnect);
  GWEN_NetLayer_SetListenFn(nl, GWEN_NetLayerPackets_Listen);
  GWEN_NetLayer_SetAddSocketsFn(nl, GWEN_NetLayerPackets_AddSockets);

  nld->outPackets=GWEN_NL_Packet_List_new();
  nld->inPackets=GWEN_NL_Packet_List_new();

  return nl;
}



/* -------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB GWEN_NetLayerPackets_FreeData(void *bp, void *p) {
  GWEN_NL_PACKETS *nld;

  nld=(GWEN_NL_PACKETS*)p;
  GWEN_NL_Packet_List_free(nld->outPackets);
  GWEN_NL_Packet_List_free(nld->inPackets);
  GWEN_NL_Packet_free(nld->currentOutPacket);
  GWEN_NL_Packet_free(nld->currentInPacket);
  GWEN_FREE_OBJECT(nld);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerPackets_Connect(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;
  int rv=0;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);
  if (GWEN_NetLayer_GetStatus(baseLayer)!=GWEN_NetLayerStatus_Connected) {
    rv=GWEN_NetLayer_Connect(baseLayer);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "Result of BaseLayer Connect: %d", rv);
    }
  }
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  GWEN_NetLayer_SubFlags(nl, GWEN_NETLAYER_FLAGS_PASSIVE);
  return rv;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerPackets_Disconnect(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;
  int rv;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);
  rv=GWEN_NetLayer_Disconnect(baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  return rv;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerPackets_Listen(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;
  int rv;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);
  rv=GWEN_NetLayer_Listen(baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_PASSIVE);
  return rv;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerPackets_AddSockets(GWEN_NETLAYER *nl,
                                    GWEN_SOCKETSET *readSet,
                                    GWEN_SOCKETSET *writeSet,
                                    GWEN_SOCKETSET *exSet) {
  GWEN_NETLAYER *baseLayer;
  int rv;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);
  rv=GWEN_NetLayer_AddSockets(baseLayer, readSet, writeSet, exSet);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  return rv;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayerPackets__WriteWork(GWEN_NETLAYER *nl) {
  GWEN_NL_PACKETS *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;
  GWEN_NL_PACKET *pk;
  char *p;
  int bsize;
  GWEN_NETLAYER_STATUS st;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_PACKETS, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  st=GWEN_NetLayer_GetStatus(nl);
  if (st==GWEN_NetLayerStatus_Listening)
    return GWEN_NetLayerResult_Idle;

  if (st==GWEN_NetLayerStatus_Connecting)
    return GWEN_NetLayerResult_WouldBlock;

  pk=nld->currentOutPacket;
  if (!pk) {
    pk=GWEN_NL_Packet_List_First(nld->outPackets);
    if (pk) {
      /* dequeue packet */
      GWEN_NL_Packet_List_Del(pk);
      nld->currentOutPacket=pk;
    }
  }
  if (!pk)
    return GWEN_NetLayerResult_Idle;

  switch(pk->status) {

  case GWEN_NL_PacketStatus_New:
  case GWEN_NL_PacketStatus_Enqueued:
  case GWEN_NL_PacketStatus_StartWriteMsg:
    rv=GWEN_NetLayer_BeginOutPacket(baseLayer,
                                    GWEN_Buffer_GetUsedBytes(pk->buffer));
    if (rv==1)
      return GWEN_NetLayerResult_WouldBlock;
    else if (rv<0 && rv!=GWEN_ERROR_UNSUPPORTED) {
      pk->status=GWEN_NL_PacketStatus_Aborted;
      GWEN_NL_Packet_free(pk);
      nld->currentOutPacket=0;
      return GWEN_NetLayerResult_Error;
    }
    pk->status=GWEN_NL_PacketStatus_WriteMsg;
    GWEN_Buffer_Rewind(pk->buffer);
    return GWEN_NetLayerResult_Changed;

  case GWEN_NL_PacketStatus_WriteMsg:
    bsize=GWEN_Buffer_GetBytesLeft(pk->buffer);
    if (bsize==0) {
      pk->status=GWEN_NL_PacketStatus_EndWriteMsg;
      return GWEN_NetLayerResult_Changed;
    }
    p=GWEN_Buffer_GetPosPointer(pk->buffer);
    rv=GWEN_NetLayer_Write(baseLayer, p, &bsize);
    if (rv==1)
      return GWEN_NetLayerResult_WouldBlock;
    else if (rv<0) {
      pk->status=GWEN_NL_PacketStatus_Aborted;
      GWEN_NL_Packet_free(pk);
      nld->currentOutPacket=0;
      return GWEN_NetLayerResult_Error;
    }
    GWEN_Buffer_IncrementPos(pk->buffer, bsize);
    return GWEN_NetLayerResult_Changed;

  case GWEN_NL_PacketStatus_EndWriteMsg:
    rv=GWEN_NetLayer_EndOutPacket(baseLayer);
    if (rv==1)
      return GWEN_NetLayerResult_WouldBlock;
    else if (rv<0 && rv!=GWEN_ERROR_UNSUPPORTED) {
      pk->status=GWEN_NL_PacketStatus_Aborted;
      GWEN_NL_Packet_free(pk);
      nld->currentOutPacket=0;
      return GWEN_NetLayerResult_Error;
    }
    pk->status=GWEN_NL_PacketStatus_Finished;
    GWEN_NL_Packet_free(pk);
    nld->currentOutPacket=0;
    return GWEN_NetLayerResult_Changed;

  case GWEN_NL_PacketStatus_StartReadMsg:
  case GWEN_NL_PacketStatus_ReadMsg:
  case GWEN_NL_PacketStatus_Finished:
  case GWEN_NL_PacketStatus_Aborted:
  default:
    return GWEN_NetLayerResult_Idle;
  }

}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayerPackets__ReadWork(GWEN_NETLAYER *nl) {
  GWEN_NL_PACKETS *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;
  GWEN_NL_PACKET *pk;
  char *p;
  int bsize;
  GWEN_NETLAYER_STATUS st;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_PACKETS, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  st=GWEN_NetLayer_GetStatus(nl);
  if (st==GWEN_NetLayerStatus_Listening)
    return GWEN_NetLayerResult_Idle;

  if (st==GWEN_NetLayerStatus_Connecting)
    return GWEN_NetLayerResult_WouldBlock;

  pk=nld->currentInPacket;
  if (!pk) {
    nld->currentInPacket=GWEN_NL_Packet_new();
    pk=nld->currentInPacket;
  }

  switch(pk->status) {
  case GWEN_NL_PacketStatus_New:
  case GWEN_NL_PacketStatus_Enqueued:
  case GWEN_NL_PacketStatus_StartReadMsg:
    rv=GWEN_NetLayer_BeginInPacket(baseLayer);
    if (rv==1)
      return GWEN_NetLayerResult_WouldBlock;
    else if (rv<0 && rv!=GWEN_ERROR_UNSUPPORTED) {
      pk->status=GWEN_NL_PacketStatus_Aborted;
      GWEN_NL_Packet_List_Add(pk, nld->inPackets);
      nld->currentInPacket=GWEN_NL_Packet_new();
      return GWEN_NetLayerResult_Error;
    }
    pk->status=GWEN_NL_PacketStatus_ReadMsg;
    return GWEN_NetLayerResult_Changed;

  case GWEN_NL_PacketStatus_ReadMsg:
    if (nld->fixedPacketSize) {
      /* fixed messages sizes, so we now how many bytes to read */
      bsize=nld->fixedPacketSize-GWEN_Buffer_GetUsedBytes(pk->buffer);
      if (bsize>GWEN_NL_PACKETS_CHUNKSIZE)
        bsize=GWEN_NL_PACKETS_CHUNKSIZE;
      GWEN_Buffer_AllocRoom(pk->buffer, bsize);
      p=GWEN_Buffer_GetPosPointer(pk->buffer);
      rv=GWEN_NetLayer_Read(baseLayer, p, &bsize);
      if (rv==1)
        return GWEN_NetLayerResult_WouldBlock;
      else if (rv==-1) {
        pk->status=GWEN_NL_PacketStatus_Aborted;
        GWEN_NL_Packet_List_Add(pk, nld->inPackets);
        nld->currentInPacket=GWEN_NL_Packet_new();
        return GWEN_NetLayerResult_Error;
      }
      GWEN_Buffer_IncrementPos(pk->buffer, bsize);
      GWEN_Buffer_AdjustUsedBytes(pk->buffer);

      /* check whether we are finished */
      if (GWEN_Buffer_GetUsedBytes(pk->buffer)>=nld->fixedPacketSize) {
        DBG_INFO(GWEN_LOGDOMAIN, "Message finished");
        pk->status=GWEN_NL_PacketStatus_Finished;
        GWEN_NL_Packet_List_Add(pk, nld->inPackets);
        nld->currentInPacket=GWEN_NL_Packet_new();
      }
      return GWEN_NetLayerResult_Changed;
    }
    else {
      GWEN_Buffer_AllocRoom(pk->buffer, GWEN_NL_PACKETS_CHUNKSIZE);
      bsize=GWEN_NL_PACKETS_CHUNKSIZE;
      p=GWEN_Buffer_GetPosPointer(pk->buffer);
      rv=GWEN_NetLayer_Read(baseLayer, p, &bsize);
      if (rv==1)
        return GWEN_NetLayerResult_WouldBlock;
      else if (rv==-1) {
        pk->status=GWEN_NL_PacketStatus_Aborted;
        GWEN_NL_Packet_List_Add(pk, nld->inPackets);
        nld->currentInPacket=GWEN_NL_Packet_new();
        return GWEN_NetLayerResult_Error;
      }
      if (bsize==0) {
        DBG_INFO(GWEN_LOGDOMAIN, "Message finished");
        pk->status=GWEN_NL_PacketStatus_Finished;
        GWEN_NL_Packet_List_Add(pk, nld->inPackets);
        nld->currentInPacket=GWEN_NL_Packet_new();
        return GWEN_NetLayerResult_Changed;
      }
      else {
        GWEN_Buffer_IncrementPos(pk->buffer, bsize);
        GWEN_Buffer_AdjustUsedBytes(pk->buffer);
        return GWEN_NetLayerResult_Changed;
      }
    }
    /* no break here since we will never reach this point */
  case GWEN_NL_PacketStatus_StartWriteMsg:
  case GWEN_NL_PacketStatus_Finished:
  case GWEN_NL_PacketStatus_Aborted:
  default:
    return GWEN_NetLayerResult_Idle;
  }
}




/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayerPackets_Work(GWEN_NETLAYER *nl) {
  GWEN_NL_PACKETS *nld;
  GWEN_NETLAYER_RESULT bres=GWEN_NetLayerResult_Idle;
  GWEN_NETLAYER_RESULT res;
  GWEN_NETLAYER *baseLayer;
  GWEN_NETLAYER_STATUS st;

#define GWEN_NL_PACKETS_MERGE_RESULTS(r1, r2)    \
  if (r1==GWEN_NetLayerResult_Idle)           \
    r1=r2;                                    \
  else if (r1!=GWEN_NetLayerResult_Changed) { \
    if (r2==GWEN_NetLayerResult_Changed)      \
      r1=GWEN_NetLayerResult_Changed;          \
  }

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_PACKETS, nl);
  assert(nld);

  st=GWEN_NetLayer_GetStatus(nl);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Working with status \"%s\" (%d)",
              GWEN_NetLayerStatus_toString(st), st);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  if (st==GWEN_NetLayerStatus_Listening) {
    GWEN_NETLAYER *newNl;

    newNl=GWEN_NetLayer_GetIncomingLayer(baseLayer);
    if (newNl) {
      GWEN_NETLAYER *newNlPackets;

      DBG_INFO(GWEN_LOGDOMAIN, "Incoming connection");
      newNlPackets=GWEN_NetLayerPackets_new(newNl);
      GWEN_NetLayer_AddFlags(newNlPackets, GWEN_NETLAYER_FLAGS_PASSIVE);
      GWEN_NetLayer_free(newNl);
      GWEN_NetLayer_AddIncomingLayer(nl, newNlPackets);
      bres=GWEN_NetLayerResult_Changed;
    }
  }

  res=GWEN_NetLayer_Work(baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  DBG_VERBOUS(GWEN_LOGDOMAIN,
              "Result of BaseLayer work: %s",
              GWEN_NetLayerResult_toString(res));
  if (res==GWEN_NetLayerResult_Error) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "here");
    return res;
  }
  GWEN_NL_PACKETS_MERGE_RESULTS(bres, res);

  res=GWEN_NetLayerPackets__WriteWork(nl);
  DBG_VERBOUS(GWEN_LOGDOMAIN,
              "Result of WriteWork: %s",
              GWEN_NetLayerResult_toString(res));
  if (res==GWEN_NetLayerResult_Error) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return res;
  }
  GWEN_NL_PACKETS_MERGE_RESULTS(bres, res);

  res=GWEN_NetLayerPackets__ReadWork(nl);
  DBG_VERBOUS(GWEN_LOGDOMAIN,
              "Result of ReadWork: %s",
              GWEN_NetLayerResult_toString(res));
  if (res==GWEN_NetLayerResult_Error) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return res;
  }
  GWEN_NL_PACKETS_MERGE_RESULTS(bres, res);

#undef GWEN_NL_PACKETS_MERGE_RESULTS

  /* return cumulated result */
  DBG_VERBOUS(GWEN_LOGDOMAIN,
              "Overall result of Work: %s",
              GWEN_NetLayerResult_toString(bres));

  return bres;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerPackets_SendPacket(GWEN_NETLAYER *nl,
                                    GWEN_NL_PACKET *pk) {
  GWEN_NL_PACKETS *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_PACKETS, nl);
  assert(nld);

  GWEN_NL_Packet_Attach(pk);
  GWEN_NL_Packet_List_Add(pk, nld->outPackets);

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerPackets_HasNextPacket(const GWEN_NETLAYER *nl) {
  GWEN_NL_PACKETS *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_PACKETS, nl);
  assert(nld);

  if (GWEN_NL_Packet_List_First(nld->inPackets))
    return 1;

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NL_PACKET *GWEN_NetLayerPackets_GetNextPacket(GWEN_NETLAYER *nl) {
  GWEN_NL_PACKETS *nld;
  GWEN_NL_PACKET *pk;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_PACKETS, nl);
  assert(nld);

  pk=GWEN_NL_Packet_List_First(nld->inPackets);
  if (pk) {
    GWEN_NL_Packet_List_Del(pk);
    return pk;
  }

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NL_PACKET *GWEN_NetLayerPackets_GetNextPacket_Wait(GWEN_NETLAYER *nl,
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
    double d;
    GWEN_NL_PACKET *pk;

    if (GWEN_WaitCallback()==GWEN_WaitCallbackResult_Abort) {
      DBG_INFO(GWEN_LOGDOMAIN, "User aborted");
      return 0;
    }

    st=GWEN_NetLayer_GetStatus(nl);
    if (st!=GWEN_NetLayerStatus_Connected) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad status of netlayer: %s",
                GWEN_NetLayerStatus_toString(st));
      return 0;
    }

    pk=GWEN_NetLayerPackets_GetNextPacket(nl);
    if (pk)
      return pk;

    res=GWEN_Net_HeartBeat(distance);
    if (res==GWEN_NetLayerResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return 0;
    }

    /* check timeout */
    d=difftime(time(0), startt);
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
int GWEN_NetLayerPackets_Flush(GWEN_NETLAYER *nl, int timeout) {
  GWEN_NL_PACKETS *nld;
  time_t startt;
  int distance;
  int count;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_PACKETS, nl);
  assert(nld);

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

    if (nld->currentOutPacket==0 &&
        GWEN_NL_Packet_List_GetCount(nld->outPackets)==0)
      return 0;

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



unsigned int GWEN_NetLayerPackets_GetFixedPacketSize(const GWEN_NETLAYER *nl){
  GWEN_NL_PACKETS *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_PACKETS, nl);
  assert(nld);

  return nld->fixedPacketSize;
}



void GWEN_NetLayerPackets_SetFixedPacketSize(GWEN_NETLAYER *nl,
                                             unsigned int i) {
  GWEN_NL_PACKETS *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_PACKETS, nl);
  assert(nld);

  nld->fixedPacketSize=i;
}










