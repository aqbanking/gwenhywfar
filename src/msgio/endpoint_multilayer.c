/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*#define DISABLE_DEBUGLOG*/


#include "./endpoint_multilayer_p.h"

#include <gwenhywfar/endpoint.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>


#define GWEN_ENDPOINT_MULTILAYER_NAME             "multilayer"
#define GWEN_ENDPOINT_MULTILAYER_DEFAULT_RECONNECT_TIME  5
#define GWEN_ENDPOINT_MULTILAYER_DEFAULT_CONNECT_TIMEOUT 10


GWEN_INHERIT(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_MULTILAYER);


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(void *bp, void *p);

static void _addSockets(GWEN_MSG_ENDPOINT *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_UNUSED GWEN_SOCKETSET *xSet);
static void _checkSockets(GWEN_MSG_ENDPOINT *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet);
static int _startConnect(GWEN_MSG_ENDPOINT *ep);
static void _moveMessagesBetweenLists(GWEN_MSG_LIST *srcList, GWEN_MSG_LIST *dstList);

static void _checkForNeedToReconnect(GWEN_MSG_ENDPOINT *ep);
static void _addSocketsWhenConnecting(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT *epChild,
                                      GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet);
static void _addSocketsWhenConnected(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT *epChild,
                                     GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet);

static void _checkSocketsWhenConnecting(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT *epChild,
                                        GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet);
static void _checkSocketsWhenConnected(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT *epChild,
                                       GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



GWEN_MSG_ENDPOINT *GWEN_MultilayerEndpoint_new(const char *name, int groupId)
{
  GWEN_MSG_ENDPOINT *ep;
  GWEN_ENDPOINT_MULTILAYER *xep;

  ep=GWEN_MsgEndpoint_new(name?name:GWEN_ENDPOINT_MULTILAYER_NAME, groupId);
  GWEN_NEW_OBJECT(GWEN_ENDPOINT_MULTILAYER, xep);
  GWEN_INHERIT_SETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_MULTILAYER, ep, xep, _freeData);

  GWEN_MsgEndpoint_SetAddSocketsFn(ep, _addSockets);
  GWEN_MsgEndpoint_SetCheckSocketsFn(ep, _checkSockets);

  xep->reconnectTimeInSeconds=GWEN_ENDPOINT_MULTILAYER_DEFAULT_RECONNECT_TIME;
  xep->connectTimeoutInSeconds=GWEN_ENDPOINT_MULTILAYER_DEFAULT_CONNECT_TIMEOUT;

  return ep;
}



void _freeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_ENDPOINT_MULTILAYER *xep;

  xep=(GWEN_ENDPOINT_MULTILAYER*) p;
  GWEN_FREE_OBJECT(xep);
}



int GWEN_MultilayerEndpoint_GetReconnectTimeInSeconds(const GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    GWEN_ENDPOINT_MULTILAYER *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_MULTILAYER, ep);
    if (xep)
      return xep->reconnectTimeInSeconds;
  }
  return GWEN_ENDPOINT_MULTILAYER_DEFAULT_RECONNECT_TIME;
}



void GWEN_MultilayerEndpoint_SetReconnectTimeInSeconds(GWEN_MSG_ENDPOINT *ep, int i)
{
  if (ep) {
    GWEN_ENDPOINT_MULTILAYER *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_MULTILAYER, ep);
    if (xep)
      xep->reconnectTimeInSeconds=i;
  }
}



int GWEN_MultilayerEndpoint_GetConnectTimeoutInSeconds(const GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    GWEN_ENDPOINT_MULTILAYER *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_MULTILAYER, ep);
    if (xep)
      return xep->connectTimeoutInSeconds;
  }
  return GWEN_ENDPOINT_MULTILAYER_DEFAULT_CONNECT_TIMEOUT;
}



void GWEN_MultilayerEndpoint_SetConnectTimeoutInSeconds(GWEN_MSG_ENDPOINT *ep, int i)
{
  if (ep) {
    GWEN_ENDPOINT_MULTILAYER *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_MULTILAYER, ep);
    if (xep)
      xep->connectTimeoutInSeconds=i;
  }
}



int GWEN_MultilayerEndpoint_GetStage(const GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    GWEN_ENDPOINT_MULTILAYER *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_MULTILAYER, ep);
    if (xep)
      return xep->stage;
  }
  return 0;
}



void GWEN_MultilayerEndpoint_SetStage(GWEN_MSG_ENDPOINT *ep, int i)
{
  if (ep) {
    GWEN_ENDPOINT_MULTILAYER *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_MULTILAYER, ep);
    if (xep)
      xep->stage=i;
  }
}



GWEN_ENDPOINT_MULTILAYER_STARTCONNECT_FN GWEN_MultilayerEndpoint_SetStartConnectFn(GWEN_MSG_ENDPOINT *ep,
                                                                                   GWEN_ENDPOINT_MULTILAYER_STARTCONNECT_FN fn)
{
  if (ep) {
    GWEN_ENDPOINT_MULTILAYER *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_MULTILAYER, ep);
    if (xep) {
      GWEN_ENDPOINT_MULTILAYER_STARTCONNECT_FN oldFn;

      oldFn=xep->startConnectFn;
      xep->startConnectFn=fn;
      return oldFn;
    }
  }

  return NULL;
}



GWEN_ENDPOINT_MULTILAYER_CHECKSOCKETS_FN GWEN_MultilayerEndpoint_SetCheckSocketsFn(GWEN_MSG_ENDPOINT *ep,
                                                                                   GWEN_ENDPOINT_MULTILAYER_CHECKSOCKETS_FN fn)
{
  if (ep) {
    GWEN_ENDPOINT_MULTILAYER *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_MULTILAYER, ep);
    if (xep) {
      GWEN_ENDPOINT_MULTILAYER_CHECKSOCKETS_FN oldFn;

      oldFn=xep->checkSocketsFn;
      xep->checkSocketsFn=fn;
      return oldFn;
    }
  }

  return NULL;
}



int GWEN_MultilayerEndpoint_StartConnect(GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    if (GWEN_MsgEndpoint_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_UNCONNECTED) {
      int rv;

      /* connect, set state */
      rv=_startConnect(ep);
      if (rv<0 && rv!=GWEN_ERROR_IN_PROGRESS) {
        DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Error connecting (%d)", GWEN_MsgEndpoint_GetName(ep), rv);
        GWEN_MsgEndpoint_SetState(ep, GWEN_MSG_ENDPOINT_STATE_CONNECTING);
      }
      else {
        DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Connecting.", GWEN_MsgEndpoint_GetName(ep));
        GWEN_MsgEndpoint_SetState(ep, GWEN_MSG_ENDPOINT_STATE_CONNECTING);
      }
      return rv;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Endpoint %s: Not unconnected", GWEN_MsgEndpoint_GetName(ep));
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No endpoint");
  }
  return GWEN_ERROR_GENERIC;
}



void _addSockets(GWEN_MSG_ENDPOINT *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet)
{
  if (ep) {
    GWEN_MSG_ENDPOINT *epChild;
  
    epChild=GWEN_MsgEndpoint_Tree2_GetFirstChild(ep);
    if (epChild) {
      if (GWEN_MsgEndpoint_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_UNCONNECTED)
        _checkForNeedToReconnect(ep);
      else {
        if (GWEN_MsgEndpoint_GetState(epChild)==GWEN_MSG_ENDPOINT_STATE_UNCONNECTED) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Error on tcp layer, disconnecting");
          GWEN_MsgEndpoint_Disconnect(epChild);
          GWEN_MsgEndpoint_Disconnect(ep);
        }
        else {
          if (GWEN_MsgEndpoint_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_CONNECTING)
            _addSocketsWhenConnecting(ep, epChild, readSet, writeSet, xSet);
          if (GWEN_MsgEndpoint_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_CONNECTED)
            _addSocketsWhenConnected(ep, epChild, readSet, writeSet, xSet);
        }
      }
    } /* if (epChild) */
  } /* if (ep) */
}



void _checkSockets(GWEN_MSG_ENDPOINT *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet)
{
  DBG_DEBUG(GWEN_LOGDOMAIN, "Checking sockets in state %d", GWEN_MsgEndpoint_GetState(ep));
  if (ep) {
    GWEN_MSG_ENDPOINT *epChild;
  
    epChild=GWEN_MsgEndpoint_Tree2_GetFirstChild(ep);
    if (epChild) {
      if (GWEN_MsgEndpoint_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_UNCONNECTED) {
        /* nothing to do here */
      } /* if GWEN_MSG_ENDPOINT_STATE_UNCONNECTED */
      else {
        if (GWEN_MsgEndpoint_GetState(epChild)==GWEN_MSG_ENDPOINT_STATE_UNCONNECTED) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Error on tcp layer, disconnecting");
          GWEN_MsgEndpoint_Disconnect(epChild);
          GWEN_MsgEndpoint_Disconnect(ep);
        }
        else {
          if (GWEN_MsgEndpoint_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_CONNECTING)
            _checkSocketsWhenConnecting(ep, epChild, readSet, writeSet, xSet);
          else if (GWEN_MsgEndpoint_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_CONNECTED)
            _checkSocketsWhenConnected(ep, epChild, readSet, writeSet, xSet);
        }
      }
    }
  }
}



void _checkForNeedToReconnect(GWEN_MSG_ENDPOINT *ep)
{
  time_t now;

  now=time(NULL);
  if ((now-GWEN_MsgEndpoint_GetTimeOfLastStateChange(ep))>=GWEN_MultilayerEndpoint_GetReconnectTimeInSeconds(ep)) {
    int rv;

    /* (re)connect, set state */
    DBG_INFO(GWEN_LOGDOMAIN, "Starting to (re-)connect");
    rv=GWEN_MultilayerEndpoint_StartConnect(ep);
    if (rv<0 && rv!=GWEN_ERROR_IN_PROGRESS) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    }
  }
}



void _addSocketsWhenConnecting(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT *epChild,
                               GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet)
{
  time_t now;
  
  now=time(NULL);
  if ((now-GWEN_MsgEndpoint_GetTimeOfLastStateChange(ep))>=GWEN_MultilayerEndpoint_GetConnectTimeoutInSeconds(ep) ||
      GWEN_MsgEndpoint_GetState(epChild)==GWEN_MSG_ENDPOINT_STATE_UNCONNECTED) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Timeout on connect");
    GWEN_MsgEndpoint_Disconnect(epChild);
    GWEN_MsgEndpoint_Disconnect(ep);
  }
  else
    GWEN_MsgEndpoint_AddSockets(epChild, readSet, writeSet, xSet);
}



void _addSocketsWhenConnected(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT *epChild,
                              GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet)
{
  if (GWEN_MsgEndpoint_GetState(epChild)==GWEN_MSG_ENDPOINT_STATE_UNCONNECTED) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error on tcp layer, disconnecting");
    GWEN_MsgEndpoint_Disconnect(epChild);
    GWEN_MsgEndpoint_Disconnect(ep);
  }
  else {
    /* move to-send messages to child */
    _moveMessagesBetweenLists(GWEN_MsgEndpoint_GetSendMessageList(ep), GWEN_MsgEndpoint_GetSendMessageList(epChild));
    GWEN_MsgEndpoint_AddSockets(epChild, readSet, writeSet, xSet);
  }
}



void _checkSocketsWhenConnected(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT *epChild,
                                GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet)
{
  _moveMessagesBetweenLists(GWEN_MsgEndpoint_GetSendMessageList(ep), GWEN_MsgEndpoint_GetSendMessageList(epChild));
  _moveMessagesBetweenLists(GWEN_MsgEndpoint_GetReceivedMessageList(epChild), GWEN_MsgEndpoint_GetReceivedMessageList(ep));
  GWEN_MsgEndpoint_CheckSockets(epChild, readSet, writeSet, xSet);
  _moveMessagesBetweenLists(GWEN_MsgEndpoint_GetReceivedMessageList(epChild), GWEN_MsgEndpoint_GetReceivedMessageList(ep));
}



void _moveMessagesBetweenLists(GWEN_MSG_LIST *srcList, GWEN_MSG_LIST *dstList)
{
  GWEN_MSG *msg;

  while( (msg=GWEN_Msg_List_First(srcList)) ) {
    GWEN_Msg_List_Del(msg);
    GWEN_Msg_List_Add(msg, dstList);
  }
}





int _startConnect(GWEN_MSG_ENDPOINT *ep)
{
  GWEN_MSG_ENDPOINT *epChild;

  epChild=GWEN_MsgEndpoint_Tree2_GetFirstChild(ep);
  if (epChild) {
    GWEN_ENDPOINT_MULTILAYER *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_MULTILAYER, ep);
    if (xep) {
      if (xep->startConnectFn)
        return xep->startConnectFn(ep, epChild);
    }
  }
  return GWEN_ERROR_GENERIC;
}



void _checkSocketsWhenConnecting(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT *epChild,
                                 GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet)
{
  GWEN_ENDPOINT_MULTILAYER *xep;

  GWEN_MsgEndpoint_CheckSockets(epChild, readSet, writeSet, xSet); /* let base layer work */

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_MULTILAYER, ep);
  if (xep) {
    if (xep->checkSocketsFn)
      xep->checkSocketsFn(ep, epChild, readSet, writeSet, xSet);
  }
}


