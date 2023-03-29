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

static int _getWriteFd(GWEN_MSG_ENDPOINT *ep);
static int _getReadFd(GWEN_MSG_ENDPOINT *ep);
static int _handleWritable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);

static int _connect(GWEN_MSG_ENDPOINT *ep, int fd);
static int _createAndSetupSocket(void);
static int _setSocketNonBlocking(int fd);






GWEN_MSG_ENDPOINT *GWEN_TcpcEndpoint_new(const char *host, int port, const char *name, int groupId)
{
  GWEN_MSG_ENDPOINT *ep;
  GWEN_ENDPOINT_TCPC *xep;

  ep=GWEN_MsgEndpoint_new(name?name:GWEN_MSG_ENDPOINT_TCPC_NAME, groupId);
  GWEN_NEW_OBJECT(GWEN_ENDPOINT_TCPC, xep);
  GWEN_INHERIT_SETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_TCPC, ep, xep, _freeData);
  xep->host=host?strdup(host):NULL;
  xep->port=port;

  xep->state=GWEN_MSG_ENDPOINT_TCPC_STATE_UNCONNECTED;

  xep->handleWritableFn=GWEN_MsgEndpoint_SetHandleWritableFn(ep, _handleWritable);
  xep->getReadFdFn=GWEN_MsgEndpoint_SetGetReadFdFn(ep, _getReadFd);
  xep->getWriteFdFn=GWEN_MsgEndpoint_SetGetWriteFdFn(ep, _getWriteFd);

  return ep;
}



void _freeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_ENDPOINT_TCPC *xep;

  xep=(GWEN_ENDPOINT_TCPC*) p;
  free(xep->host);
  GWEN_FREE_OBJECT(xep);
}



int GWEN_TcpcEndpoint_StartConnect(GWEN_MSG_ENDPOINT *ep)
{
  GWEN_ENDPOINT_TCPC *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_TCPC, ep);
  if (xep) {
    if (xep->state!=GWEN_MSG_ENDPOINT_TCPC_STATE_UNCONNECTED) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Endpoint not unconnected");
      return GWEN_ERROR_INVALID;
    }
    else {
      int fd;
      int rv;
    
      fd=_createAndSetupSocket();
      if (fd<0) {
        DBG_INFO(NULL, "here");
        return GWEN_ERROR_IO;
      }
    
      rv=_connect(ep, fd);
      if (rv==0) {
        xep->state=GWEN_MSG_ENDPOINT_TCPC_STATE_CONNECTED;
        GWEN_MsgEndpoint_SetFd(ep, fd);
        DBG_INFO(GWEN_LOGDOMAIN, "Endpoint connected");
        return 0;
      }
      else if (rv==GWEN_ERROR_TRY_AGAIN) {
        xep->state=GWEN_MSG_ENDPOINT_TCPC_STATE_CONNECTING;
        GWEN_MsgEndpoint_SetFd(ep, fd);
        DBG_INFO(GWEN_LOGDOMAIN, "Connecting endpoint...");
        return 0;
      }
      else {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        xep->state=GWEN_MSG_ENDPOINT_TCPC_STATE_UNCONNECTED;
        GWEN_MsgEndpoint_SetFd(ep, -1);
        return rv;
      }
      return 0;
    }
  }
  return GWEN_ERROR_INVALID;
}



int GWEN_TcpcEndpoint_GetState(const GWEN_MSG_ENDPOINT *ep)
{
  GWEN_ENDPOINT_TCPC *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_TCPC, ep);
  if (xep)
    return xep->state;
  return -1;
}



void GWEN_TcpcEndpoint_SetState(GWEN_MSG_ENDPOINT *ep, int m)
{
  GWEN_ENDPOINT_TCPC *xep;

  xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_TCPC, ep);
  if (xep)
    xep->state=m;
}



int _getReadFd(GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    GWEN_ENDPOINT_TCPC *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_TCPC, ep);
    if (xep) {
      if (xep->state>=GWEN_MSG_ENDPOINT_TCPC_STATE_CONNECTED) {
        if (GWEN_MsgEndpoint_GetFlags(ep) & GWEN_MSG_ENDPOINT_FLAGS_DISCONNECTED) {
          int fd;

          fd=GWEN_MsgEndpoint_GetFd(ep);
          close(fd);
          GWEN_MsgEndpoint_SetFd(ep, -1);
          xep->state=GWEN_MSG_ENDPOINT_TCPC_STATE_UNCONNECTED;
          GWEN_MsgEndpoint_DelFlags(ep, GWEN_MSG_ENDPOINT_FLAGS_DISCONNECTED);
        }
        else
          return xep->getReadFdFn(ep);
      }
    }
  }
  return -1;
}



int _getWriteFd(GWEN_MSG_ENDPOINT *ep)
{
  if (ep) {
    GWEN_ENDPOINT_TCPC *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_TCPC, ep);
    if (xep) {
      if (xep->state==GWEN_MSG_ENDPOINT_TCPC_STATE_CONNECTING)
        return GWEN_MsgEndpoint_GetFd(ep);
      else if (xep->state>=GWEN_MSG_ENDPOINT_TCPC_STATE_CONNECTED) {
        return xep->getWriteFdFn(ep);
      }
    }
  }
  return -1;
}



int _handleWritable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr)
{
  if (ep) {
    GWEN_ENDPOINT_TCPC *xep;

    xep=GWEN_INHERIT_GETDATA(GWEN_MSG_ENDPOINT, GWEN_ENDPOINT_TCPC, ep);
    if (xep) {
      if (xep->state==GWEN_MSG_ENDPOINT_TCPC_STATE_CONNECTING) {
        int rv;

        rv=_connect(ep, GWEN_MsgEndpoint_GetFd(ep));
        if (rv<0) {
          if (rv!=GWEN_ERROR_TRY_AGAIN) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            xep->state=GWEN_MSG_ENDPOINT_TCPC_STATE_UNCONNECTED;
            return rv;
          }
          return 0;
        }
        else {
          xep->state=GWEN_MSG_ENDPOINT_TCPC_STATE_CONNECTED;
          return 0;
        }
      }
      else if (xep->state>=GWEN_MSG_ENDPOINT_TCPC_STATE_CONNECTED) {
        return xep->handleWritableFn(ep, emgr);
      }
    }
  }
  return GWEN_ERROR_GENERIC;
}



int _connect(GWEN_MSG_ENDPOINT *ep, int fd)
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



