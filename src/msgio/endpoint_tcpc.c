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


#include "msgio/endpoint_tcpc_p.h"
#include "msgio/endpoint_connectable.h"

#include <gwenhywfar/debug.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>




#define GWEN_MSG_ENDPOINT_TCPC_NAME   "tcpc"




GWEN_INHERIT(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_TCPC)



static void GWENHYWFAR_CB _freeData(void *bp, void *p);

static int _connect(GWEN_MSG_ENDPOINT *ep);
static int _reallyConnect(GWEN_MSG_ENDPOINT *ep, int fd);
static int _createAndSetupSocket(void);
static int _setSocketNonBlocking(int fd);






GWEN_MSG_ENDPOINT *GWEN_TcpcEndpoint_new(const char *host, int port, const char *name, int groupId)
{
  GWEN_MSG_ENDPOINT *ep;
  GWEN_ENDPOINT_TCPC *xep;

  ep=GWEN_MsgEndpoint_new(name?name:GWEN_MSG_ENDPOINT_TCPC_NAME, groupId);

  GWEN_ConnectableMsgEndpoint_Extend(ep);

  GWEN_NEW_OBJECT(GWEN_ENDPOINT_TCPC, xep);
  GWEN_INHERIT_SETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_TCPC, ep, xep, _freeData);
  xep->host=host?strdup(host):NULL;
  xep->port=port;

  GWEN_ConnectableMsgEndpoint_SetConnectFn(ep, _connect);
  return ep;
}



void _freeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_ENDPOINT_TCPC *xep;

  xep=(GWEN_ENDPOINT_TCPC*) p;
  free(xep->host);
  GWEN_FREE_OBJECT(xep);
}



int _connect(GWEN_MSG_ENDPOINT *ep)
{
  int state;

  state=GWEN_ConnectableMsgEndpoint_GetState(ep);
  DBG_DEBUG(GWEN_LOGDOMAIN, "Calling connect (%d)", state);
  if (state<GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTED) {
    int fd;
    int rv;

    if (state<GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTING) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "New socket, connect not yet started");
      fd=_createAndSetupSocket();
      if (fd<0) {
	DBG_INFO(NULL, "here");
	return GWEN_ERROR_IO;
      }
    }
    else if (state==GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTING){
      DBG_DEBUG(GWEN_LOGDOMAIN, "Socket in connecting state");
      fd=GWEN_MsgEndpoint_GetFd(ep);
      if (fd<0) {
	DBG_INFO(NULL, "here");
	return GWEN_ERROR_IO;
      }
    }
    DBG_DEBUG(GWEN_LOGDOMAIN, "Trying to connect");
    rv=_reallyConnect(ep, fd);
    if (rv<0) {
      if (rv==GWEN_ERROR_TRY_AGAIN) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "Connect started...");
        GWEN_ConnectableMsgEndpoint_SetState(ep, GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTING);
        GWEN_MsgEndpoint_SetFd(ep, fd);
      }
      else {
        DBG_INFO(GWEN_LOGDOMAIN, "Error connecting.");
        GWEN_ConnectableMsgEndpoint_SetState(ep, GWEN_MSG_ENDPOINT_CONN_STATE_DISCONNECTED);
        close(fd);
        GWEN_MsgEndpoint_SetFd(ep, -1);
      }
    }
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Connected.");
      GWEN_MsgEndpoint_SetFd(ep, fd);
      GWEN_ConnectableMsgEndpoint_SetState(ep, GWEN_MSG_ENDPOINT_CONN_STATE_CONNECTED);
    }
    return rv;
  }
  else {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Already connected");
    return GWEN_ERROR_GENERIC;
  }
}



int _reallyConnect(GWEN_MSG_ENDPOINT *ep, int fd)
{
  GWEN_ENDPOINT_TCPC *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_TCPC, ep);
  if (xep) {
    struct sockaddr_in addr;
    int rv;

//    fd=GWEN_MsgEndpoint_GetFd(ep);
    memset(&addr, 0, sizeof(addr));
    addr.sin_port=htons(xep->port);
    addr.sin_family=AF_INET;

    if (inet_aton(xep->host, &(addr.sin_addr))==0) {
      /* bad address */
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad address \"%s\": %s", xep->host, strerror(errno));
      return GWEN_ERROR_IO;
    }

    rv=connect(fd, (struct sockaddr*) &addr, sizeof(addr));
    if (rv<0) {
      if (errno==EAGAIN || errno==EINPROGRESS) {
        /* connect in progress */
        return GWEN_ERROR_TRY_AGAIN;
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "connect(): %s", strerror(errno));
        close(fd);
        return GWEN_ERROR_IO;
      }
    }
    return 0;
  }

  return GWEN_ERROR_INTERNAL;
}



int _createAndSetupSocket(void)
{
  int rv;
  int fd;

  fd=socket(AF_INET, SOCK_STREAM, 0);
  if (fd<0) {
    /* socket error */
    DBG_ERROR(GWEN_LOGDOMAIN, "socket(): %s", strerror(errno));
    return GWEN_ERROR_IO;
  }

  rv=_setSocketNonBlocking(fd);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    close(fd);
    return rv;
  }

  return fd;
}



int _setSocketNonBlocking(int fd)
{
  int prevFlags;
  int newFlags;

  /* get current socket flags */
  prevFlags=fcntl(fd, F_GETFL);
  if (prevFlags==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fcntl(): %s", strerror(errno));
    return GWEN_ERROR_IO;
  }

  /* set nonblocking/blocking */
  newFlags=prevFlags|O_NONBLOCK;

  if (-1==fcntl(fd, F_SETFL, newFlags)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fcntl(): %s", strerror(errno));
    return GWEN_ERROR_IO;
  }

  return 0;
}



