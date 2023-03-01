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


#include "msgio/endpointmgr_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/list.h>
#include <gwenhywfar/error.h>
#include <gwenhywfar/debug.h>

#include <errno.h>



static int _ioLoopOnce(GWEN_MSG_ENDPOINT_MGR *emgr);
static void _runAllEndpoints(GWEN_MSG_ENDPOINT_MGR *emgr);




GWEN_MSG_ENDPOINT_MGR *GWEN_MsgEndpointMgr_new(void)
{
  GWEN_MSG_ENDPOINT_MGR *emgr;

  GWEN_NEW_OBJECT(GWEN_MSG_ENDPOINT_MGR, emgr);
  emgr->endpointList=GWEN_MsgEndpoint_List_new();
  return emgr;
}



void GWEN_MsgEndpointMgr_free(GWEN_MSG_ENDPOINT_MGR *emgr)
{
  if (emgr) {
    GWEN_MsgEndpoint_List_free(emgr->endpointList);
    GWEN_FREE_OBJECT(emgr);
  }
}



GWEN_MSG_ENDPOINT_LIST *GWEN_MsgEndpointMgr_GetEndpointList(const GWEN_MSG_ENDPOINT_MGR *emgr)
{
  return emgr->endpointList;
}




void GWEN_MsgEndpointMgr_AddEndpoint(GWEN_MSG_ENDPOINT_MGR *emgr, GWEN_MSG_ENDPOINT *ep)
{
  GWEN_MsgEndpoint_List_Add(ep, emgr->endpointList);
}



void GWEN_MsgEndpointMgr_DelEndpoint(GWEN_UNUSED GWEN_MSG_ENDPOINT_MGR *emgr, GWEN_MSG_ENDPOINT *ep)
{
  GWEN_MsgEndpoint_List_Del(ep);
}



int GWEN_MsgEndpointMgr_LoopOnce(GWEN_MSG_ENDPOINT_MGR *emgr)
{
  int rv;

  rv=_ioLoopOnce(emgr);
  _runAllEndpoints(emgr);
  return rv;
}



int _ioLoopOnce(GWEN_MSG_ENDPOINT_MGR *emgr)
{
  GWEN_MSG_ENDPOINT *ep;
  fd_set readSet;
  fd_set writeSet;
  int highestRdFd=-1;
  int highestWrFd=-1;
  struct timeval tv;
  int rv;

  FD_ZERO(&readSet);
  FD_ZERO(&writeSet);
  tv.tv_sec=2;
  tv.tv_usec=0;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Sampling sockets");
  ep=GWEN_MsgEndpoint_List_First(emgr->endpointList);
  if (ep==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No endpoints.");
    return GWEN_ERROR_GENERIC;
  }
  while(ep) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "- checking endpoint %s", GWEN_MsgEndpoint_GetName(ep));
    if (!(GWEN_MsgEndpoint_GetFlags(ep) & GWEN_MSG_ENDPOINT_FLAGS_NOIO)) {
      int fd;

      fd=GWEN_MsgEndpoint_GetReadFd(ep);
      if (fd>=0) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "  - adding socket %d for read", fd);
        FD_SET(fd, &readSet);
        highestRdFd=(fd>highestRdFd)?fd:highestRdFd;
      }

      fd=GWEN_MsgEndpoint_GetWriteFd(ep);
      if (fd>=0) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "  - adding socket %d for write", fd);
        FD_SET(fd, &writeSet);
        highestWrFd=(fd>highestWrFd)?fd:highestWrFd;
      }
    }
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "  - endpoint %s does not support IO", GWEN_MsgEndpoint_GetName(ep));
    }
    ep=GWEN_MsgEndpoint_List_Next(ep);
  }

  DBG_DEBUG(GWEN_LOGDOMAIN, "Calling select (highest read socket: %d, highest write socket: %d)", highestRdFd, highestWrFd);
  rv=select(((highestRdFd>highestWrFd)?highestRdFd:highestWrFd)+1,
            (highestRdFd<0)?NULL:&readSet,
            (highestWrFd<0)?NULL:&writeSet,
            NULL,
            &tv);
  DBG_DEBUG(GWEN_LOGDOMAIN, "Return from select (%d, %d=%s)", rv, (rv<0)?errno:0, (rv<0)?strerror(errno):"no error");
  if (rv<0) {
    if (errno!=EINTR) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error on select");
      return GWEN_ERROR_IO;
    }
  }
  else if (rv==0) {
    /* timeout */
    DBG_DEBUG(GWEN_LOGDOMAIN, "timeout");
    return GWEN_ERROR_TRY_AGAIN;
  }
  else if (rv) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Letting all endpoints handle IO");
    ep=GWEN_MsgEndpoint_List_First(emgr->endpointList);
    while(ep) {
      GWEN_MSG_ENDPOINT *epNext;
      int fd;
      int rv;

      epNext=GWEN_MsgEndpoint_List_Next(ep);
      fd=GWEN_MsgEndpoint_GetFd(ep);
      if (fd!=-1 && FD_ISSET(fd, &readSet)) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "- endpoint(%s): read", GWEN_MsgEndpoint_GetName(ep));
        rv=GWEN_MsgEndpoint_HandleReadable(ep, emgr);
        if (rv<0 && rv!=GWEN_ERROR_TRY_AGAIN) {
          DBG_DEBUG(GWEN_LOGDOMAIN, "error, removing endpoint %s", GWEN_MsgEndpoint_GetName(ep));
          fd=-1;
          GWEN_MsgEndpointMgr_DelEndpoint(emgr, ep);
        }
      }
      if (fd!=-1 && FD_ISSET(fd, &writeSet)) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "- endpoint(%s): write", GWEN_MsgEndpoint_GetName(ep));
        rv=GWEN_MsgEndpoint_HandleWritable(ep, emgr);
        if (rv<0 && rv!=GWEN_ERROR_TRY_AGAIN) {
          DBG_DEBUG(GWEN_LOGDOMAIN, "error, removing endpoint %s", GWEN_MsgEndpoint_GetName(ep));
          fd=-1;
          GWEN_MsgEndpointMgr_DelEndpoint(emgr, ep);
        }
      }
      ep=epNext;
    }
  }

  return 0;
}



void _runAllEndpoints(GWEN_MSG_ENDPOINT_MGR *emgr)
{
  GWEN_MSG_ENDPOINT *ep;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Running all endpoints");
  ep=GWEN_MsgEndpoint_List_First(emgr->endpointList);
  while(ep) {
    GWEN_MSG_ENDPOINT *next;

    next=GWEN_MsgEndpoint_List_Next(ep);
    DBG_DEBUG(GWEN_LOGDOMAIN, "- running endpoint %s", GWEN_MsgEndpoint_GetName(ep));
    GWEN_MsgEndpoint_Run(ep);
    ep=next;
  }
}





