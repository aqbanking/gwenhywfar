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


#include "msgio/endpoint_ipc_tcp.h"
#include "msgio/endpoint_ipc.h"
#include "msgio/msg_ipc.h"

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




#define GWEN_MSG_ENDPOINT_IPCTCP_NAME   "ipctcp"
#define GWEN_MSG_ENDPOINTICPTCP_BACKLOG 10



static int _getReadFd(GWEN_MSG_ENDPOINT *ep);
static int _getWriteFd(GWEN_MSG_ENDPOINT *ep);
static int _handleReadable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
static int _handleWritable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
int _setupListeningSocket(const char *host, int port);
int _setSocketNonBlocking(int fd);






GWEN_MSG_ENDPOINT *GWEN_MsgEndpointIpcTcp_new(const char *host, int port, int groupId)
{
  int fd;
  GWEN_MSG_ENDPOINT *ep;

  fd=_setupListeningSocket(host, port);
  if (fd<0) {
    DBG_INFO(NULL, "here");
    return NULL;
  }

  ep=GWEN_MsgEndpoint_new(GWEN_MSG_ENDPOINT_IPCTCP_NAME, groupId);
  GWEN_MsgEndpoint_SetFd(ep, fd);

  GWEN_MsgEndpoint_SetHandleReadableFn(ep, _handleReadable);
  GWEN_MsgEndpoint_SetHandleWritableFn(ep, _handleWritable);
  GWEN_MsgEndpoint_SetGetReadFdFn(ep, _getReadFd);
  GWEN_MsgEndpoint_SetGetWriteFdFn(ep, _getWriteFd);

  return ep;
}




int _getReadFd(GWEN_MSG_ENDPOINT *ep)
{
  return GWEN_MsgEndpoint_GetFd(ep);
}



int _getWriteFd(GWEN_UNUSED GWEN_MSG_ENDPOINT *ep)
{
  return -1;
}



int _handleReadable(GWEN_MSG_ENDPOINT *ep, GWEN_UNUSED GWEN_MSG_ENDPOINT_MGR *emgr)
{
  int fd;
  int newSock;
  int rv;
  struct sockaddr_in clientAddr;
  socklen_t len;
  GWEN_MSG_ENDPOINT *newEp;

  fd=GWEN_MsgEndpoint_GetFd(ep);
  memset(&clientAddr, 0, sizeof(clientAddr));
  do {
    len=sizeof(clientAddr);
    rv=accept(fd, (struct sockaddr*) &clientAddr, &len);
  } while(rv<0 && errno==EINTR);
  if (rv<0) {
    if (errno==EAGAIN || errno==EWOULDBLOCK)
      return GWEN_ERROR_TRY_AGAIN;
    DBG_ERROR(GWEN_LOGDOMAIN, "Error on accept(): %s (%d)", strerror(errno), errno);
    return GWEN_ERROR_IO;
  }
  newSock=rv;

  rv=_setSocketNonBlocking(newSock);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    close(newSock);
    return rv;
  }

  newEp=GWEN_MsgEndpointIpc_new("TCP Client", GWEN_MsgEndpoint_GetGroupId(ep));
  GWEN_MsgEndpoint_SetFd(newEp, newSock);
  GWEN_MsgEndpoint_SetFlags(newEp, GWEN_MsgEndpoint_GetFlags(ep));

  GWEN_MsgEndpointMgr_AddEndpoint(emgr, newEp);
  return 0;
}



int _handleWritable(GWEN_UNUSED GWEN_MSG_ENDPOINT *ep, GWEN_UNUSED GWEN_MSG_ENDPOINT_MGR *emgr)
{
  /* should not get called */
  return GWEN_ERROR_INVALID;
}



int _setupListeningSocket(const char *host, int port)
{
  struct sockaddr_in addr;
  int rv;
  int sk;
  int i;

  memset(&addr, 0, sizeof(addr));
  addr.sin_port=htons(port);
  addr.sin_family=AF_INET;

  if (inet_aton(host, &(addr.sin_addr))==0) {
    /* bad address */
  }

  sk=socket(AF_INET, SOCK_STREAM, 0);
  if (sk<0) {
    /* socket error */
    DBG_ERROR(GWEN_LOGDOMAIN, "socket(): %s", strerror(errno));
  }

  i=1;
  rv=setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "setsockopt(): %s", strerror(errno));
    close(sk);
    return GWEN_ERROR_IO;
  }

  rv=_setSocketNonBlocking(sk);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    close(sk);
    return rv;
  }


  rv=bind(sk, (struct sockaddr*) &addr, sizeof(addr));
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "bind(): %s", strerror(errno));
    close(sk);
    return GWEN_ERROR_IO;
  }

  rv=listen(sk, GWEN_MSG_ENDPOINTICPTCP_BACKLOG);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "listen(): %s", strerror(errno));
    close(sk);
    return GWEN_ERROR_IO;
  }

  return sk;
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



