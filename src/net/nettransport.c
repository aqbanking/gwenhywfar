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


#include "nettransport_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>


GWEN_LIST_FUNCTIONS(GWEN_NETTRANSPORT, GWEN_NetTransport);
GWEN_INHERIT_FUNCTIONS(GWEN_NETTRANSPORT);




GWEN_NETTRANSPORT *GWEN_NetTransport_new(){
  GWEN_NETTRANSPORT *tr;

  GWEN_NEW_OBJECT(GWEN_NETTRANSPORT, tr);
  GWEN_LIST_INIT(GWEN_NETTRANSPORT, tr);
  GWEN_INHERIT_INIT(GWEN_NETTRANSPORT, tr);
  tr->status=GWEN_NetTransportStatusUnconnected;
  tr->incomingConnections=GWEN_NetTransport_List_new();
  tr->backLog=GWEN_NETTRANSPORT_DEFAULT_BACKLOG;
  tr->usage=1;
  return tr;
}



void GWEN_NetTransport_free(GWEN_NETTRANSPORT *tr){
  if (tr) {
    assert(tr->usage);
    if (--(tr->usage)==0) {
      GWEN_INHERIT_FINI(GWEN_NETTRANSPORT, tr);

      GWEN_NetTransport_List_free(tr->incomingConnections);
      GWEN_InetAddr_free(tr->peerAddr);

      GWEN_LIST_FINI(GWEN_NETTRANSPORT, tr);
      free(tr);
    }
  }
}



void GWEN_NetTransport_Attach(GWEN_NETTRANSPORT *tr){
  assert(tr);
  tr->usage++;
}



GWEN_NETTRANSPORT_RESULT
GWEN_NetTransport_StartConnect(GWEN_NETTRANSPORT *tr){
  assert(tr);
  assert(tr->startConnectFn);
  /* reset EOF and PASSIVE flags */
  tr->flags&=~(GWEN_NETTRANSPORT_FLAGS_EOF_OUT |
	       GWEN_NETTRANSPORT_FLAGS_EOF_IN |
	       GWEN_NETTRANSPORT_FLAGS_PASSIVE);
  return tr->startConnectFn(tr);
}



GWEN_NETTRANSPORT_RESULT
GWEN_NetTransport_StartAccept(GWEN_NETTRANSPORT *tr){
  assert(tr);
  assert(tr->startAcceptFn);

  /* reset EOF flags */
  tr->flags&=~(GWEN_NETTRANSPORT_FLAGS_EOF_OUT |
	       GWEN_NETTRANSPORT_FLAGS_EOF_IN);
  /* adjust flags */
  tr->flags|=GWEN_NETTRANSPORT_FLAGS_PASSIVE;
  return tr->startAcceptFn(tr);
}



GWEN_NETTRANSPORT_RESULT
GWEN_NetTransport_StartDisconnect(GWEN_NETTRANSPORT *tr){
  assert(tr);
  assert(tr->startDisconnectFn);
  return tr->startDisconnectFn(tr);
}



GWEN_NETTRANSPORT_RESULT
GWEN_NetTransport_Read(GWEN_NETTRANSPORT *tr,
		       char *buffer,
		       int *bsize){
  GWEN_NETTRANSPORT_RESULT rv;

  assert(tr);
  assert(tr->readFn);
  if (tr->flags & GWEN_NETTRANSPORT_FLAGS_EOF_IN) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Reading beyond EOF");
    return GWEN_NetTransportResultError;
  }

  rv=tr->readFn(tr, buffer, bsize);
  if (rv==0 && *bsize==0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "EOF met (read)");
    tr->flags|=GWEN_NETTRANSPORT_FLAGS_EOF_IN;
  }

  return rv;
}



GWEN_NETTRANSPORT_RESULT
GWEN_NetTransport_Write(GWEN_NETTRANSPORT *tr,
			const char *buffer,
			int *bsize){
  assert(tr);
  assert(tr->writeFn);
  if (tr->flags & GWEN_NETTRANSPORT_FLAGS_EOF_OUT) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Writing beyond EOF");
    return GWEN_NetTransportResultError;
  }
  if (*bsize==0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "EOF met (write)");
    tr->flags|=GWEN_NETTRANSPORT_FLAGS_EOF_OUT;
  }
  return tr->writeFn(tr, buffer, bsize);
}



int GWEN_NetTransport_AddSockets(GWEN_NETTRANSPORT *tr,
				 GWEN_SOCKETSET *sset,
				 int forReading){
  assert(tr);
  assert(tr->addSocketsFn);
  return tr->addSocketsFn(tr, sset, forReading);
}



GWEN_NETTRANSPORT_STATUS
GWEN_NetTransport_GetStatus(const GWEN_NETTRANSPORT *tr){
  assert(tr);
  return tr->status;
}



void GWEN_NetTransport_SetStatus(GWEN_NETTRANSPORT *tr,
                                 GWEN_NETTRANSPORT_STATUS st){
  assert(tr);
  tr->status=st;
}



GWEN_TYPE_UINT32
GWEN_NetTransport_GetFlags(const GWEN_NETTRANSPORT *tr){
  assert(tr);
  return tr->flags;
}



void GWEN_NetTransport_SetFlags(GWEN_NETTRANSPORT *tr,
                                GWEN_TYPE_UINT32 flags){
  assert(tr);
  tr->flags=flags;
}



GWEN_NETTRANSPORT_WORKRESULT GWEN_NetTransport_Work(GWEN_NETTRANSPORT *tr) {
  assert(tr);
  assert(tr->workFn);
  return tr->workFn(tr);
}



GWEN_NETTRANSPORT *GWEN_NetTransport_GetNextIncoming(GWEN_NETTRANSPORT *tr){
  GWEN_NETTRANSPORT *nxt;

  nxt=GWEN_NetTransport_List_First(tr->incomingConnections);
  if (nxt) {
    GWEN_NetTransport_List_Del(nxt);
  }
  return nxt;
}



void GWEN_NetTransport_AddNextIncoming(GWEN_NETTRANSPORT *tr,
                                       GWEN_NETTRANSPORT *newTr){
  GWEN_NetTransport_List_Add(newTr, tr->incomingConnections);
}




void GWEN_NetTransport_SetStartConnectFn(GWEN_NETTRANSPORT *tr,
                                         GWEN_NETTRANSPORT_STARTCONNECT fn){
  assert(tr);
  tr->startConnectFn=fn;
}



void GWEN_NetTransport_SetStartAcceptFn(GWEN_NETTRANSPORT *tr,
                                        GWEN_NETTRANSPORT_STARTACCEPT fn){
  assert(tr);
  tr->startAcceptFn=fn;
}



void
GWEN_NetTransport_SetStartDisconnectFn(GWEN_NETTRANSPORT *tr,
                                       GWEN_NETTRANSPORT_STARTDISCONNECT fn){
  assert(tr);
  tr->startDisconnectFn=fn;
}



void GWEN_NetTransport_SetReadFn(GWEN_NETTRANSPORT *tr,
                                 GWEN_NETTRANSPORT_READ fn){
  assert(tr);
  tr->readFn=fn;
}



void GWEN_NetTransport_SetWriteFn(GWEN_NETTRANSPORT *tr,
                                  GWEN_NETTRANSPORT_WRITE fn){
  assert(tr);
  tr->writeFn=fn;
}



void GWEN_NetTransport_SetAddSocketsFn(GWEN_NETTRANSPORT *tr,
                                       GWEN_NETTRANSPORT_ADDSOCKETS fn){
  assert(tr);
  tr->addSocketsFn=fn;
}



void GWEN_NetTransport_SetWorkFn(GWEN_NETTRANSPORT *tr,
                                 GWEN_NETTRANSPORT_WORK fn){
  assert(tr);
  tr->workFn=fn;
}



const GWEN_INETADDRESS*
GWEN_NetTransport_GetLocalAddr(const GWEN_NETTRANSPORT *tr){
  assert(tr);
  return tr->localAddr;
}



void GWEN_NetTransport_SetLocalAddr(GWEN_NETTRANSPORT *tr,
                                    const GWEN_INETADDRESS *addr){
  assert(tr);
  GWEN_InetAddr_free(tr->localAddr);
  tr->localAddr=GWEN_InetAddr_dup(addr);
}



const GWEN_INETADDRESS*
GWEN_NetTransport_GetPeerAddr(const GWEN_NETTRANSPORT *tr){
  assert(tr);
  return tr->peerAddr;
}



void GWEN_NetTransport_SetPeerAddr(GWEN_NETTRANSPORT *tr,
                                   const GWEN_INETADDRESS *addr){
  assert(tr);
  GWEN_InetAddr_free(tr->peerAddr);
  tr->peerAddr=GWEN_InetAddr_dup(addr);
}



const char *GWEN_NetTransport_StatusName(GWEN_NETTRANSPORT_STATUS st) {
  const char *s;

  switch(st) {
  case GWEN_NetTransportStatusUnconnected:
    s="unconnected"; break;
  case GWEN_NetTransportStatusPConnecting:
    s="connecting (physically)"; break;
  case GWEN_NetTransportStatusPConnected:
    s="connected (physically)"; break;
  case GWEN_NetTransportStatusLConnecting:
    s="connecting (logically)"; break;
  case GWEN_NetTransportStatusLConnected:
    s="connected (logically)"; break;
  case GWEN_NetTransportStatusLDisconnecting:
    s="disconnecting (logically)"; break;
  case GWEN_NetTransportStatusLDisconnected:
    s="disconnected (logically)"; break;
  case GWEN_NetTransportStatusPDisconnecting:
    s="disconnecting (physically)"; break;
  case GWEN_NetTransportStatusPDisconnected:
    s="disconnected (physically)"; break;
  case GWEN_NetTransportStatusListening:
    s="listening"; break;
  case GWEN_NetTransportStatusDisabled:
    s="disabled"; break;
  default:
    s="unknown status"; break;
  } /* switch */

  return s;
}



const char *GWEN_NetTransport_ResultName(GWEN_NETTRANSPORT_RESULT res) {
  const char *s;

  switch(res) {
  case GWEN_NetTransportResultOk:
    s="Ok"; break;
  case GWEN_NetTransportResultError:
    s="Error"; break;
  case GWEN_NetTransportResultWantRead:
    s="WantRead"; break;
  case GWEN_NetTransportResultWantWrite:
    s="WantWrite"; break;
  case GWEN_NetTransportResultAborted:
    s="Aborted"; break;
  default:
    s="<unknown>"; break;
  } /* switch */

  return s;
}



void GWEN_NetTransport_MarkActivity(GWEN_NETTRANSPORT *tr){
  assert(tr);

  tr->lastActivity=time(0);
}



double GWEN_NetTransport_GetIdleTime(const GWEN_NETTRANSPORT *tr){
  assert(tr);

  if (tr->lastActivity==0)
    return 0;
  return difftime(time(0), tr->lastActivity);
}



GWEN_TYPE_UINT32 GWEN_NetTransport_GetBackLog(const GWEN_NETTRANSPORT *tr){
  assert(tr);
  return tr->backLog;
}



void GWEN_NetTransport_SetBackLog(GWEN_NETTRANSPORT *tr, GWEN_TYPE_UINT32 i){
  assert(tr);
  tr->backLog=i;
}



GWEN_TYPE_UINT32
GWEN_NetTransport_GetIncomingCount(const GWEN_NETTRANSPORT *tr){
  assert(tr);
  return GWEN_NetTransport_List_GetCount(tr->incomingConnections);
}











