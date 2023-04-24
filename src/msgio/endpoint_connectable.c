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

#include "msgio/endpoint_connectable_p.h"

#include <gwenhywfar/debug.h>

#include <unistd.h>


#define GWEN_CONN_ENDPOINT_RECONNECT_WAITTIME 2



GWEN_INHERIT(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT)



static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static int _getReadFd(GWEN_MSG_ENDPOINT *ep);
static int _getWriteFd(GWEN_MSG_ENDPOINT *ep);
static int _handleReadable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
static int _handleWritable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
static void _run(GWEN_MSG_ENDPOINT *ep);
static int _connect(GWEN_MSG_ENDPOINT *ep);




void GWEN_ConnectableMsgEndpoint_Extend(GWEN_MSG_ENDPOINT *ep)
{
  GWEN_CONN_ENDPOINT *xep;

  GWEN_NEW_OBJECT(GWEN_CONN_ENDPOINT, xep);

  GWEN_INHERIT_SETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep, xep, _freeData);
  xep->reconnectWaitTime=GWEN_CONN_ENDPOINT_RECONNECT_WAITTIME;
  xep->getReadFdFn=GWEN_MsgEndpoint_SetGetReadFdFn(ep, _getReadFd);
  xep->getWriteFdFn=GWEN_MsgEndpoint_SetGetWriteFdFn(ep, _getWriteFd);
  xep->handleReadableFn=GWEN_MsgEndpoint_SetHandleReadableFn(ep, _handleReadable);
  xep->handleWritableFn=GWEN_MsgEndpoint_SetHandleWritableFn(ep, _handleWritable);
  xep->runFn=GWEN_MsgEndpoint_SetRunFn(ep, _run);

  xep->fullyConnectedState=GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTED; /* preset */
}



void _freeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=(GWEN_CONN_ENDPOINT*) p;
  GWEN_FREE_OBJECT(xep);
}



int GWEN_ConnectableMsgEndpoint_GetState(const GWEN_MSG_ENDPOINT *ep)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep) {
    return xep->state;
  }
  return GWEN_MSG_ENDPOINT_CONN_STATE_UNCONNECTED;
}



void GWEN_ConnectableMsgEndpoint_SetState(GWEN_MSG_ENDPOINT *ep, int m)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep) {
    if (xep->state!=m) {
      xep->timeOfLastStateChange=time(NULL);
      DBG_INFO(GWEN_LOGDOMAIN, "Changing status of endpoint %s to %d", GWEN_MsgEndpoint_GetName(ep), m);
    }
    xep->state=m;
  }
}



time_t GWEN_ConnectableMsgEndpoint_GetTimeOfLastStateChange(const GWEN_MSG_ENDPOINT *ep)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep) {
    return xep->timeOfLastStateChange;
  }
  return 0;
}



int GWEN_ConnectableMsgEndpoint_GetFullyConnectedState(const GWEN_MSG_ENDPOINT *ep)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep)
    return xep->fullyConnectedState;
  return GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTED;
}



void GWEN_ConnectableMsgEndpoint_SetFullyConnectedState(GWEN_MSG_ENDPOINT *ep, int m)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep)
    xep->fullyConnectedState=m;
}




int GWEN_ConnectableMsgEndpoint_GetReconnectWaitTime(const GWEN_MSG_ENDPOINT *ep)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep)
    return xep->reconnectWaitTime;
  return 0;
}



void GWEN_ConnectableMsgEndpoint_SetReconnectWaitTime(GWEN_MSG_ENDPOINT *ep, int t)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep)
    xep->reconnectWaitTime=t;
}



int GWEN_ConnectableMsgEndpoint_Connect(GWEN_MSG_ENDPOINT *ep)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep && xep->connectFn)
    return xep->connectFn(ep);

  return GWEN_ERROR_NOT_IMPLEMENTED;
}



void GWEN_ConnectableMsgEndpoint_Disconnect(GWEN_MSG_ENDPOINT *ep)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep && xep->disconnectFn)
    xep->disconnectFn(ep);
  else {
    int fd;

    fd=GWEN_MsgEndpoint_GetFd(ep);
    if (fd!=-1) {
      close(fd);
      GWEN_MsgEndpoint_SetFd(ep, -1);
    }
  }
}



int _getReadFd(GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    GWEN_CONN_ENDPOINT *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
    if (xep) {
      if (xep->state>=GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTED) {
	if (GWEN_MsgEndpoint_GetFlags(ep) & GWEN_MSG_ENDPOINT_FLAGS_DISCONNECTED) {
          int fd;

          fd=GWEN_MsgEndpoint_GetFd(ep);
          close(fd);
          GWEN_MsgEndpoint_SetFd(ep, -1);
          xep->state=GWEN_MSG_ENDPOINT_CONN_STATE_DISCONNECTED;
          GWEN_MsgEndpoint_DelFlags(ep, GWEN_MSG_ENDPOINT_FLAGS_DISCONNECTED);
        }
	else {
	  int fd;

	  fd=xep->getReadFdFn(ep);
	  DBG_DEBUG(GWEN_LOGDOMAIN, "GetReadFd: Endpoint %s, state: %d (fd=%d)", GWEN_MsgEndpoint_GetName(ep), xep->state, fd);
	  return fd;
	}
      }
    }
  }
  return -1;
}



int _getWriteFd(GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    GWEN_CONN_ENDPOINT *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
    if (xep) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "GetWriteFd: Endpoint %s, state: %d", GWEN_MsgEndpoint_GetName(ep), xep->state);
      if (xep->state>=GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTING) {
	if (xep->state<GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTED) {
	  int fd;

	  /* not yet physically connected, return socket directly */
	  fd=GWEN_MsgEndpoint_GetFd(ep);
	  DBG_DEBUG(GWEN_LOGDOMAIN, "GetWriteFd: Not physically connected (\"%s\", state: %d, fd: %d)",
		    GWEN_MsgEndpoint_GetName(ep), xep->state, fd);
	  return fd;
	}
	else {
	  int fd;

	  /* physically connected, so message exchange can appear, call virtual function */
	  fd=xep->getWriteFdFn(ep);
	  DBG_DEBUG(GWEN_LOGDOMAIN, "GetWriteFd: At least physically connected (fd=%d)", fd);
	  return fd;
	}
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "GetWriteFd: Unexpected state %d", xep->state);
      }
    }
  }
  return -1;
}



int _handleReadable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep && xep->handleReadableFn) {
    int rv;

    rv=xep->handleReadableFn(ep, emgr);
    if (rv<0) {
      if (rv!=GWEN_ERROR_TRY_AGAIN) {
	DBG_INFO(GWEN_LOGDOMAIN, "Error reading, disconnecting");
	GWEN_ConnectableMsgEndpoint_SetState(ep, GWEN_MSG_ENDPOINT_CONN_STATE_DISCONNECTED);
	GWEN_ConnectableMsgEndpoint_Disconnect(ep);
      }
      return rv;
    }

    if (xep->state<xep->fullyConnectedState) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "HandleReadable: Continue connecting...");
      rv=_connect(ep);
      if (rv<0 && rv!=GWEN_ERROR_TRY_AGAIN) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }

    return rv;
  }

  return GWEN_ERROR_GENERIC;
}



int _handleWritable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep) {
    int rv;

    DBG_DEBUG(GWEN_LOGDOMAIN, "HandleWritable: Endpoint %s, state: %d", GWEN_MsgEndpoint_GetName(ep), xep->state);
//    if (xep->state==GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTING) {
    if (xep->state<xep->fullyConnectedState) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "HandleWritable: Continue connecting...");
      rv=_connect(ep);
      if (rv<0 && rv!=GWEN_ERROR_TRY_AGAIN) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
    if (xep->state>=GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTED) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "HandleWritable: Connected, writing.");
      if (xep->handleWritableFn) {
	rv=xep->handleWritableFn(ep, emgr);
        if (rv<0 && rv!=GWEN_ERROR_TRY_AGAIN) {
          DBG_INFO(GWEN_LOGDOMAIN, "Error writing, disconnecting");
          GWEN_ConnectableMsgEndpoint_SetState(ep, GWEN_MSG_ENDPOINT_CONN_STATE_DISCONNECTED);
          GWEN_ConnectableMsgEndpoint_Disconnect(ep);
        }
        return rv;
      }
      return GWEN_ERROR_NOT_IMPLEMENTED;
    }
    return 0;
  }

  return GWEN_ERROR_GENERIC;
}



void _run(GWEN_MSG_ENDPOINT *ep)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep) {
    if (xep->state>GWEN_MSG_ENDPOINT_CONN_STATE_UNCONNECTED) {
      if (xep->runFn)
        xep->runFn(ep);
    }
    else {
      time_t now;

      now=time(NULL);
      if ((int)(now-xep->timeOfLastStateChange)>xep->reconnectWaitTime) {
        int rv;

        DBG_DEBUG(GWEN_LOGDOMAIN, "Trying to connect");
        rv=_connect(ep);
        if (rv<0 && rv!=GWEN_ERROR_TRY_AGAIN) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        }
      }
    }
  }
}



GWEN_CONN_ENDPOINT_CONNECT_FN GWEN_ConnectableMsgEndpoint_SetConnectFn(GWEN_MSG_ENDPOINT *ep,
                                                                       GWEN_CONN_ENDPOINT_CONNECT_FN fn)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep) {
    GWEN_CONN_ENDPOINT_CONNECT_FN oldFn;

    oldFn=xep->connectFn;
    xep->connectFn=fn;
    return oldFn;
  }

  return NULL;
}



GWEN_CONN_ENDPOINT_DISCONNECT_FN GWEN_ConnectableMsgEndpoint_SetDisconnectFn(GWEN_MSG_ENDPOINT *ep,
                                                                             GWEN_CONN_ENDPOINT_DISCONNECT_FN fn)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep) {
    GWEN_CONN_ENDPOINT_DISCONNECT_FN oldFn;

    oldFn=xep->disconnectFn;
    xep->disconnectFn=fn;
    return oldFn;
  }

  return NULL;
}



int _connect(GWEN_MSG_ENDPOINT *ep)
{
  GWEN_CONN_ENDPOINT *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_CONN_ENDPOINT, ep);
  if (xep) {
    int rv;

    DBG_DEBUG(GWEN_LOGDOMAIN, "Endpoint %s, state: %d", GWEN_MsgEndpoint_GetName(ep), xep->state);
    rv=GWEN_ConnectableMsgEndpoint_Connect(ep);
    if (rv<0) {
      if (rv!=GWEN_ERROR_TRY_AGAIN) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_ConnectableMsgEndpoint_SetState(ep, GWEN_MSG_ENDPOINT_CONN_STATE_DISCONNECTED);
      }
      else {
        DBG_DEBUG(GWEN_LOGDOMAIN, "Connecting...");
	//GWEN_ConnectableMsgEndpoint_SetState(ep, GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTING);
      }
    }
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Connected");
      //GWEN_ConnectableMsgEndpoint_SetState(ep, GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTED);
    }
    return rv;
  }
  return 0;
}





