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


#include "./endpoint2_tcpc_p.h"

#include <gwenhywfar/debug.h>



#define GWEN_MSG_ENDPOINT2_TCPC_NAME       "tcpclient"
#define GWEN_ENDPOINT2_TCPC_RECONNECT_TIME 5



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(void *bp, void *p);

static void _addSockets(GWEN_MSG_ENDPOINT2 *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet);
static void _checkSockets(GWEN_MSG_ENDPOINT2 *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet);

static int _startConnect(GWEN_MSG_ENDPOINT2 *ep);
static GWEN_SOCKET *_createAndSetupSocket(void);
static GWEN_INETADDRESS *_createAndSetupAddress(const char *host, int port);


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



GWEN_INHERIT(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_TCPC)




GWEN_MSG_ENDPOINT2 *GWEN_TcpcEndpoint2_new(const char *host, int port,
                                           GWEN_MSG_ENDPOINT_MGR2 *mgr,
                                           const char *name, int groupId)
{
  GWEN_MSG_ENDPOINT2 *ep;
  GWEN_ENDPOINT2_TCPC *xep;

  ep=GWEN_MsgEndpoint2_new(mgr, name?name:GWEN_MSG_ENDPOINT2_TCPC_NAME, groupId);
  GWEN_MsgEndpoint2_SetState(ep, GWEN_MSG_ENDPOINT_STATE_UNCONNECTED);

  GWEN_NEW_OBJECT(GWEN_ENDPOINT2_TCPC, xep);
  GWEN_INHERIT_SETDATA(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_TCPC, ep, xep, _freeData);
  xep->host=host?strdup(host):NULL;
  xep->port=port;

  GWEN_MsgEndpoint2_SetAddSocketsFn(ep, _addSockets);
  GWEN_MsgEndpoint2_SetCheckSocketsFn(ep, _checkSockets);

  return ep;
}



int GWEN_TcpcEndpoint2_StartConnect(GWEN_MSG_ENDPOINT2 *ep)
{
  if (ep) {
    if (GWEN_MsgEndpoint2_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_UNCONNECTED) {
      int rv;

      /* connect, set state */
      rv=_startConnect(ep);
      if (rv==GWEN_ERROR_IN_PROGRESS) {
        DBG_INFO(GWEN_LOGDOMAIN, "Connect in progress");
        GWEN_MsgEndpoint2_SetState(ep, GWEN_MSG_ENDPOINT_STATE_CONNECTING);
      }
      else if (rv==0) {
        DBG_INFO(GWEN_LOGDOMAIN, "Connected.");
        GWEN_MsgEndpoint2_SetState(ep, GWEN_MSG_ENDPOINT_STATE_CONNECTED);
      }
      else {
        DBG_INFO(GWEN_LOGDOMAIN, "Error on connect(%d)", rv);
      }
      return rv;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Endpoint \"%s\" not unconnected", GWEN_MsgEndpoint2_GetName(ep));
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No endpoint");
  }
  return GWEN_ERROR_GENERIC;
}




void _freeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_ENDPOINT2_TCPC *xep;

  xep=(GWEN_ENDPOINT2_TCPC*) p;
  free(xep->host);
  GWEN_FREE_OBJECT(xep);
}



void _addSockets(GWEN_MSG_ENDPOINT2 *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_UNUSED GWEN_SOCKETSET *xSet)
{
  if (ep) {
    if (GWEN_MsgEndpoint2_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_UNCONNECTED) {
      time_t now;
  
      now=time(NULL);
      if ((now-GWEN_MsgEndpoint2_GetTimeOfLastStateChange(ep))>=GWEN_ENDPOINT2_TCPC_RECONNECT_TIME) {
        int rv;
  
        /* (re)connect, set state */
        rv=GWEN_TcpcEndpoint2_StartConnect(ep);
        if (rv<0 && rv!=GWEN_ERROR_IN_PROGRESS) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        }
      }
    }
  
    if (GWEN_MsgEndpoint2_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_CONNECTING) {
      GWEN_SocketSet_AddSocket(writeSet, GWEN_MsgEndpoint2_GetSocket(ep));
    }
  
    if (GWEN_MsgEndpoint2_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_CONNECTED) {
      GWEN_SocketSet_AddSocket(readSet, GWEN_MsgEndpoint2_GetSocket(ep));
      if (GWEN_MsgEndpoint2_HaveMessageToSend(ep))
        GWEN_SocketSet_AddSocket(writeSet, GWEN_MsgEndpoint2_GetSocket(ep));
    }
  } /* if (ep) */
}



void _checkSockets(GWEN_MSG_ENDPOINT2 *ep,
                   GWEN_UNUSED GWEN_SOCKETSET *readSet,
                   GWEN_SOCKETSET *writeSet,
                   GWEN_UNUSED GWEN_SOCKETSET *xSet)
{
  if (ep) {
    GWEN_SOCKET *sk;
  
    sk=GWEN_MsgEndpoint2_GetSocket(ep);
    if (sk) {
      if (GWEN_MsgEndpoint2_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_CONNECTING) {
        if (GWEN_SocketSet_HasSocket(writeSet, sk)) {
          int rv;
  
          rv=GWEN_Socket_GetSocketError(sk);
          if (rv==GWEN_ERROR_IN_PROGRESS) {
            DBG_INFO(GWEN_LOGDOMAIN, "Connect still in progress");
          }
          else if (rv==0) {
            DBG_INFO(GWEN_LOGDOMAIN, "Connected.");
            GWEN_MsgEndpoint2_SetState(ep, GWEN_MSG_ENDPOINT_STATE_CONNECTED);
          }
          else {
            DBG_INFO(GWEN_LOGDOMAIN, "Error on connect(%d)", rv);
          }
        }
      }

#if 0
      /* this belongs in higher layers */
  
      if (GWEN_MsgEndpoint2_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_CONNECTED) {
        if (GWEN_SocketSet_HasSocket(writeSet, sk)) {
        }
  
        if (GWEN_SocketSet_HasSocket(readSet, sk)) {
        }
      }
#endif

    } /* if (sk) */
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "Endpoint \"%s\": No socket", GWEN_MsgEndpoint2_GetName(ep));
    }
  } /* if (ep) */
}



int _startConnect(GWEN_MSG_ENDPOINT2 *ep)
{
  GWEN_ENDPOINT2_TCPC *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_TCPC, ep);
  if (xep) {
    GWEN_INETADDRESS *addr;
    GWEN_SOCKET *sk;
    int rv;

    addr=_createAndSetupAddress(xep->host, xep->port);
    if (addr==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }

    sk=_createAndSetupSocket();
    if (sk==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_InetAddr_free(addr);
      return GWEN_ERROR_GENERIC;
    }

    rv=GWEN_Socket_Connect(sk, addr);
    if (rv<0) {
      if (rv!=GWEN_ERROR_IN_PROGRESS) {
        DBG_INFO(GWEN_LOGDOMAIN, "Error on connect(\"%s\", %d): %d", xep->host, xep->port, rv);
        GWEN_Socket_free(sk);
        GWEN_InetAddr_free(addr);
        return rv;
      }
    }
    GWEN_MsgEndpoint2_SetSocket(ep, sk);
    GWEN_InetAddr_free(addr);
    return rv;
  }
  return GWEN_ERROR_GENERIC;
}



GWEN_SOCKET *_createAndSetupSocket(void)
{
  GWEN_SOCKET *sk;
  int rv;

  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  rv=GWEN_Socket_SetBlocking(sk, 0);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error setting socket nonblocking: %d", rv);
    GWEN_Socket_free(sk);
    return NULL;
  }
  return sk;
}



GWEN_INETADDRESS *_createAndSetupAddress(const char *host, int port)
{
  GWEN_INETADDRESS *addr;
  int rv;

  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  rv=GWEN_InetAddr_SetAddress(addr, host);
  if (rv<0)
    rv=GWEN_InetAddr_SetName(addr, host);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error setting host \"%s\": %d", host, rv);
    GWEN_InetAddr_free(addr);
    return NULL;
  }
  rv=GWEN_InetAddr_SetPort(addr, port);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error setting port \"%d\": %d", port, rv);
    GWEN_InetAddr_free(addr);
    return NULL;
  }

  return addr;
}





