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


#include "./endpoint2_tcpd_p.h"

#include <gwenhywfar/debug.h>



#define GWEN_MSG_ENDPOINT2_TCPD_NAME       "tcpcserver"
#define GWEN_MSG_ENDPOINT2_TCPD_BACKLOG    10




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(void *bp, void *p);

static void _addSockets(GWEN_MSG_ENDPOINT2 *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet);
static void _checkSockets(GWEN_MSG_ENDPOINT2 *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet);

static int _startListen(GWEN_MSG_ENDPOINT2 *ep);
static GWEN_SOCKET *_createAndSetupSocket(void);
static GWEN_INETADDRESS *_createAndSetupAddress(const char *host, int port);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



GWEN_INHERIT(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_TCPD)





GWEN_MSG_ENDPOINT2 *GWEN_TcpdEndpoint2_new(const char *host, int port, const char *name, int groupId)
{
  GWEN_MSG_ENDPOINT2 *ep;
  GWEN_ENDPOINT2_TCPD *xep;

  ep=GWEN_MsgEndpoint2_new(name?name:GWEN_MSG_ENDPOINT2_TCPD_NAME, groupId);
  GWEN_MsgEndpoint2_SetState(ep, GWEN_MSG_ENDPOINT_STATE_UNCONNECTED);

  GWEN_NEW_OBJECT(GWEN_ENDPOINT2_TCPD, xep);
  GWEN_INHERIT_SETDATA(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_TCPD, ep, xep, _freeData);
  xep->host=host?strdup(host):NULL;
  xep->port=port;

  GWEN_MsgEndpoint2_SetAddSocketsFn(ep, _addSockets);
  GWEN_MsgEndpoint2_SetCheckSocketsFn(ep, _checkSockets);

  return ep;
}



void _freeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_ENDPOINT2_TCPD *xep;

  xep=(GWEN_ENDPOINT2_TCPD*) p;
  free(xep->host);
  GWEN_FREE_OBJECT(xep);
}




void GWEN_TcpdEndpoint2_SetAcceptFn(GWEN_MSG_ENDPOINT2 *ep, GWEN_ENDPOINT2_TCPD_ACCEPT_FN f, void *data)
{
  if (ep) {
    GWEN_ENDPOINT2_TCPD *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_TCPD, ep);
    if (xep) {
      xep->acceptFn=f;
      xep->acceptData=data;
    }
  }
}



int GWEN_TcpdEndpoint2_StartListening(GWEN_MSG_ENDPOINT2 *ep)
{
  if (ep) {
    if (GWEN_MsgEndpoint2_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_UNCONNECTED) {
      int rv;

      /* connect, set state */
      rv=_startListen(ep);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Error on listen (%d)", GWEN_MsgEndpoint2_GetName(ep), rv);
        return rv;
      }
      DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Now listening", GWEN_MsgEndpoint2_GetName(ep));
      GWEN_MsgEndpoint2_SetState(ep, GWEN_MSG_ENDPOINT_STATE_CONNECTED);
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Endpoint %s: Not unconnected", GWEN_MsgEndpoint2_GetName(ep));
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No endpoint");
  }
  return GWEN_ERROR_GENERIC;
}



void _addSockets(GWEN_MSG_ENDPOINT2 *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet)
{

  if (GWEN_MsgEndpoint2_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_UNCONNECTED) {
    int rv;

    DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Starting to listen", GWEN_MsgEndpoint2_GetName(ep));
    rv=_startListen(ep);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Error on listen (%d)", GWEN_MsgEndpoint2_GetName(ep), rv);
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Now listening", GWEN_MsgEndpoint2_GetName(ep));
      GWEN_MsgEndpoint2_SetState(ep, GWEN_MSG_ENDPOINT_STATE_CONNECTED);
    }
  }

  if (GWEN_MsgEndpoint2_GetState(ep)==GWEN_MSG_ENDPOINT_STATE_CONNECTED) {
    GWEN_SocketSet_AddSocket(readSet, GWEN_MsgEndpoint2_GetSocket(ep));
  }

  GWEN_MsgEndpoint2_ChildrenAddSockets(ep, readSet, writeSet, xSet);
}



void _checkSockets(GWEN_MSG_ENDPOINT2 *ep, GWEN_SOCKETSET *readSet, GWEN_SOCKETSET *writeSet, GWEN_SOCKETSET *xSet)
{
  if (ep) {
    GWEN_ENDPOINT2_TCPD *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_TCPD, ep);
    if (xep) {
      GWEN_SOCKET *sk;

      sk=GWEN_MsgEndpoint2_GetSocket(ep);
      if (sk && GWEN_SocketSet_HasSocket(readSet, sk)) {
        GWEN_INETADDRESS *incomingAddress=NULL;
        GWEN_SOCKET *incomingSocket=NULL;
        int rv;

        /* accept incoming connection */
        rv=GWEN_Socket_Accept(sk, &incomingAddress, &incomingSocket);
        if (rv<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Error on accept (%d)", GWEN_MsgEndpoint2_GetName(ep), rv);
        }
        else {
          char addrBuffer[16];

          GWEN_InetAddr_GetAddress(incomingAddress, addrBuffer, sizeof(addrBuffer));
          DBG_INFO(GWEN_LOGDOMAIN,
                   "Endpoint %s: Incoming connection from %s:%d",
                   GWEN_MsgEndpoint2_GetName(ep),
                   addrBuffer,
                   GWEN_InetAddr_GetPort(incomingAddress));
          if (xep->acceptFn) {
            GWEN_MSG_ENDPOINT2 *incomingEp;

            incomingEp=xep->acceptFn(ep, incomingSocket, incomingAddress, xep->acceptData);
            if (incomingEp) {
              DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Incoming connection accepted, adding", GWEN_MsgEndpoint2_GetName(ep));
              GWEN_MsgEndpoint2_SetState(incomingEp, GWEN_MSG_ENDPOINT_STATE_CONNECTED);
              GWEN_MsgEndpoint2_Tree2_AddChild(ep, incomingEp);
            }
            else {
              DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: Error accepting incoming connection", GWEN_MsgEndpoint2_GetName(ep));
              GWEN_Socket_free(incomingSocket);
            }
          }
          else {
            DBG_INFO(GWEN_LOGDOMAIN, "Endpoint %s: function \"acceptFn\" not set", GWEN_MsgEndpoint2_GetName(ep));
            GWEN_Socket_free(incomingSocket);
          }
        }
        GWEN_InetAddr_free(incomingAddress);
      }
    }
  }

  GWEN_MsgEndpoint2_ChildrenCheckSockets(ep, readSet, writeSet, xSet);
}



int _startListen(GWEN_MSG_ENDPOINT2 *ep)
{
  GWEN_ENDPOINT2_TCPD *xep;

  DBG_INFO(GWEN_LOGDOMAIN, "Starting to listen");
  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT2, GWEN_ENDPOINT2_TCPD, ep);
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

    rv=GWEN_Socket_Bind(sk, addr);
    if (rv<0) {
      if (rv!=GWEN_ERROR_IN_PROGRESS) {
        DBG_INFO(GWEN_LOGDOMAIN, "Error on bind(\"%s\", %d): %d", xep->host, xep->port, rv);
        GWEN_Socket_free(sk);
        GWEN_InetAddr_free(addr);
        return rv;
      }
    }
    GWEN_InetAddr_free(addr);

    rv=GWEN_Socket_Listen(sk, GWEN_MSG_ENDPOINT2_TCPD_BACKLOG);
    if (rv<0) {
      if (rv!=GWEN_ERROR_IN_PROGRESS) {
        DBG_INFO(GWEN_LOGDOMAIN, "Error on listen(\"%s\", %d): %d", xep->host, xep->port, rv);
        GWEN_Socket_free(sk);
        return rv;
      }
    }

    GWEN_MsgEndpoint2_SetSocket(ep, sk);
    return rv;
  }
  return GWEN_ERROR_GENERIC;
}



GWEN_SOCKET *_createAndSetupSocket(void)
{
  GWEN_SOCKET *sk;
  int rv;

  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);

  rv=GWEN_Socket_Open(sk);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error opening socket: %d", rv);
    GWEN_Socket_free(sk);
    return NULL;
  }

  rv=GWEN_Socket_SetReuseAddress(sk, 1);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error setting up address reuse on socket: %d", rv);
    GWEN_Socket_free(sk);
    return NULL;
  }

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












