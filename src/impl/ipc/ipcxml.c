/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 15 2003
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

#include "ipcxml_p.h"
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/ipcxmlmsglayer.h>
#include <gwenhyfwar/ipcxmlconnlayer.h>
#include <gwenhyfwar/ipc.h>


static unsigned int GWEN_IPCXMLRequest_LastId=0;


GWEN_IPCXMLREQUEST *GWEN_IPCXMLRequest_new(){
  GWEN_IPCXMLREQUEST *r;

  GWEN_NEW_OBJECT(GWEN_IPCXMLREQUEST, r);
  r->id=++GWEN_IPCXMLRequest_LastId;
  return r;
}



void GWEN_IPCXMLRequest_free(GWEN_IPCXMLREQUEST *r){
  if (r) {
    GWEN_DB_Group_free(r->db);
    free(r);
  }
}



unsigned int GWEN_IPCXMLRequest_GetId(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->id;
}



unsigned int GWEN_IPCXMLRequest_GetMsgLayerId(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->msgLayerId;
}



void GWEN_IPCXMLRequest_SetMsgLayerId(GWEN_IPCXMLREQUEST *r,
                                      unsigned int i){
  assert(r);
  r->msgLayerId=i;
}


unsigned int GWEN_IPCXMLRequest_GetDialogId(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->dialogId;
}



void GWEN_IPCXMLRequest_SetDialogId(GWEN_IPCXMLREQUEST *r,
                                    unsigned int i){
  assert(r);
  r->dialogId=i;
}



unsigned int GWEN_IPCXMLRequest_GetMessageNumber(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->messageNumber;
}



void GWEN_IPCXMLRequest_SetMessageNumber(GWEN_IPCXMLREQUEST *r,
                                         unsigned int i){
  assert(r);
  r->messageNumber=i;
}



unsigned int GWEN_IPCXMLRequest_GetSegmentNumber(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->segmentNumber;
}



void GWEN_IPCXMLRequest_SetSegmentNumber(GWEN_IPCXMLREQUEST *r,
                                         unsigned int i){
  assert(r);
  r->segmentNumber=i;
}



GWEN_DB_NODE *GWEN_IPCXMLRequest_GetDb(GWEN_IPCXMLREQUEST *r){
  assert(r);
  return r->db;
}



void GWEN_IPCXMLRequest_SetDb(GWEN_IPCXMLREQUEST *r,
                              GWEN_DB_NODE *db){
  assert(r);
  GWEN_DB_Group_free(r->db);
  r->db=db;
}






GWEN_IPCXMLSERVICE *GWEN_IPCXMLService_new(GWEN_MSGENGINE *msgEngine,
                                           GWEN_SECCTX_MANAGER *scm) {
  GWEN_IPCXMLSERVICE *xs;

  GWEN_NEW_OBJECT(GWEN_IPCXMLSERVICE, xs);
  xs->msgEngine=msgEngine;
  xs->securityManager=scm;

  return xs;
}




GWEN_IPCCONNLAYER *GWEN_IPCXMLService_CreateCL(GWEN_IPCXMLSERVICE *xs,
                                               GWEN_IPCXMLSERVICE_TYPE st,
                                               unsigned int userMark,
                                               const char *addr,
                                               unsigned int port,
                                               unsigned int flags){
  GWEN_IPCTRANSPORTLAYER *tl;
  GWEN_IPCMSGLAYER *ml;
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;
  unsigned int cid;

  switch(st) {
  case GWEN_IPCXMLServiceTypeTCP:
    tl=GWEN_IPCTransportLayerTCP_new();
    GWEN_IPCTransportLayer_SetPort(tl, port);
    break;
  case GWEN_IPCXMLServiceTypeUnix:
    tl=GWEN_IPCTransportLayerUnix_new();
    break;
  default:
    DBG_ERROR(0, "Unknown service type (%d)", st);
    return 0;
  } /* switch */
  GWEN_IPCTransportLayer_SetAddress(tl, addr);

  /* start listening */
  err=GWEN_IPCTransportLayer_Listen(tl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    GWEN_IPCTransportLayer_free(tl);
    return 0;
  }

  /* create higher layers */
  if (flags & GWEN_IPCCONNLAYER_FLAGS_PASSIVE) {
    ml=GWEN_IPCXMLMsgLayer_new(xs->msgEngine, tl,
                               GWEN_IPCMsglayerStateListening);
  }
  else {
    ml=GWEN_IPCXMLMsgLayer_new(xs->msgEngine, tl,
                               GWEN_IPCMsglayerStateUnconnected);
  }
  cl=GWEN_IPCXMLConnLayer_new(xs->msgEngine,
                              xs->securityManager,
                              ml,
                              !(flags & GWEN_IPCCONNLAYER_FLAGS_PASSIVE));

  cid=GWEN_ConnectionLayer_GetId(cl);
  GWEN_ConnectionLayer_SetUserMark(cl, userMark);
  GWEN_ConnectionLayer_SetFlags(cl, flags);

  return cl;
}



unsigned int GWEN_IPCXMLService_AddServer(GWEN_IPCXMLSERVICE *xs,
                                          GWEN_IPCXMLSERVICE_TYPE st,
                                          unsigned int userMark,
                                          const char *addr,
                                          unsigned int port,
                                          unsigned int flags){
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;

  cl=GWEN_IPCXMLService_CreateCL(xs, st, userMark, addr, port,
                                 flags | GWEN_IPCCONNLAYER_FLAGS_PASSIVE);
  err=GWEN_ServiceLayer_AddConnection(xs->serviceLayer, cl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_ConnectionLayer_free(cl);
    return 0;
  }

  return GWEN_ConnectionLayer_GetId(cl);
}



unsigned int GWEN_IPCXMLService_AddClient(GWEN_IPCXMLSERVICE *xs,
                                          GWEN_IPCXMLSERVICE_TYPE st,
                                          unsigned int userMark,
                                          const char *addr,
                                          unsigned int port,
                                          unsigned int flags){
  GWEN_IPCCONNLAYER *cl;
  GWEN_ERRORCODE err;

  cl=GWEN_IPCXMLService_CreateCL(xs, st, userMark, addr, port,
                                 flags & ~GWEN_IPCCONNLAYER_FLAGS_PASSIVE);
  err=GWEN_ServiceLayer_AddConnection(xs->serviceLayer, cl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_ConnectionLayer_free(cl);
    return 0;
  }

  return GWEN_ConnectionLayer_GetId(cl);
}





unsigned int GWEN_IPCXMLService_AddRequest(GWEN_IPCXMLSERVICE *xs,
                                           unsigned int clid,
                                           const char *requestName,
                                           unsigned int requestVersion,
                                           GWEN_DB_NODE *db,
                                           int flush){
  GWEN_IPCCONNLAYER *cl;
  GWEN_XMLNODE *node;
  GWEN_IPCXMLREQUEST *rq;

  cl=GWEN_ServiceLayer_FindConnection(xs->serviceLayer, clid, 0);
  if (!cl) {
    DBG_ERROR(0, "Connection not found (%d)", clid);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_CONNECTION_NOT_FOUND);
  }

  node=GWEN_MsgEngine_FindNodeByProperty(xs->msgEngine,
                                         "SEG",
                                         "id",
                                         requestVersion,
                                         requestName);
  if (!node) {
    DBG_ERROR(0, "Request \"%s\" (version %d) not found",
              requestName, requestVersion);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_UNKNOWN_MSG);
  }

  rq=GWEN_IPCXMLConnLayer_AddRequest(cl, node, db, flush);
  if (!rq) {
    DBG_ERROR(0, "Could not add request \"%s\" (version %d)",
              requestName, requestVersion);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_IPC_ERROR_TYPE),
                          GWEN_IPC_ERROR_GENERIC);
  }

  GWEN_LIST_ADD(GWEN_IPCXMLREQUEST, rq, &(xs->outgoingRequests));
  return rq->id;
}
















