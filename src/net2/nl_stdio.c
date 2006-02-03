/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Feb 09 2004
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


#ifdef OS_WIN32
# include <gwenhywfar/netlayer.h>
# include <gwenhywfar/debug.h>

  GWEN_NETLAYER *GWEN_NetLayerStdio_new() {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "NetLayer stdio is not supported on your system");
    return 0;
  }

#else




#include "nl_stdio_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/inetsocket.h>

GWEN_INHERIT(GWEN_NETLAYER, GWEN_NL_STDIO)


/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER *GWEN_NetLayerStdio_new() {
  GWEN_NETLAYER *nl;
  GWEN_NL_STDIO *nld;
  GWEN_ERRORCODE err;

  nl=GWEN_NetLayer_new(GWEN_NL_STDIO_NAME);
  GWEN_NEW_OBJECT(GWEN_NL_STDIO, nld)
  DBG_MEM_INC("GWEN_NL_STDIO", 0);

  nld->socketRead=GWEN_Socket_fromFile(0);
  nld->socketWrite=GWEN_Socket_fromFile(1);

  /* set nonblocking */
  err=GWEN_Socket_SetBlocking(nld->socketRead, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_NetLayer_free(nl);
    return 0;
  }
  err=GWEN_Socket_SetBlocking(nld->socketWrite, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_NetLayer_free(nl);
    return 0;
  }

  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Connected);

  GWEN_INHERIT_SETDATA(GWEN_NETLAYER, GWEN_NL_STDIO, nl, nld,
                       GWEN_NetLayerStdio_FreeData);

  GWEN_NetLayer_SetWorkFn(nl, GWEN_NetLayerStdio_Work);
  GWEN_NetLayer_SetReadFn(nl, GWEN_NetLayerStdio_Read);
  GWEN_NetLayer_SetWriteFn(nl, GWEN_NetLayerStdio_Write);
  GWEN_NetLayer_SetDisconnectFn(nl, GWEN_NetLayerStdio_Disconnect);
  GWEN_NetLayer_SetAddSocketsFn(nl, GWEN_NetLayerStdio_AddSockets);

  return nl;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetLayerStdio_FreeData(void *bp, void *p) {
  GWEN_NL_STDIO *nld;

  nld=(GWEN_NL_STDIO*)p;
  GWEN_Socket_free(nld->socketRead);
  GWEN_Socket_free(nld->socketWrite);
  DBG_MEM_DEC("GWEN_NL_STDIO");
  GWEN_FREE_OBJECT(nld);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerStdio_Disconnect(GWEN_NETLAYER *nl){
  GWEN_NL_STDIO *nld;
  GWEN_ERRORCODE err;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_STDIO, nl);
  assert(nld);

  /* close socket */
  err=GWEN_Socket_Close(nld->socketRead);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    /*return GWEN_Error_GetSimpleCode(err); */
  }
  err=GWEN_Socket_Close(nld->socketWrite);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    /*return GWEN_Error_GetSimpleCode(err); */
  }

  /* adjust status (Disconnected) */
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
  GWEN_NetLayer_SubFlags(nl,
                         GWEN_NETLAYER_FLAGS_EOFMET |
                         GWEN_NETLAYER_FLAGS_BROKENPIPE |
                         GWEN_NETLAYER_FLAGS_WANTREAD |
                         GWEN_NETLAYER_FLAGS_WANTWRITE);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerStdio_Read(GWEN_NETLAYER *nl,
                            char *buffer,
                            int *bsize){
  GWEN_NL_STDIO *nld;
  GWEN_ERRORCODE err;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Reading %d bytes", *bsize);

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_STDIO, nl);
  assert(nld);

  /* check status */
  if (GWEN_NetLayer_GetStatus(nl)!=GWEN_NetLayerStatus_Connected) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Socket is not connected (%d)",
              GWEN_NetLayer_GetStatus(nl));
    return GWEN_ERROR_INVALID;
  }

  if (GWEN_NetLayer_GetFlags(nl) & GWEN_NETLAYER_FLAGS_EOFMET) {
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayerStatus_Disconnected);
    return GWEN_ERROR_EOF;
  }

  /* try to read */
  err=GWEN_Socket_Read(nld->socketRead, buffer, bsize);


  /* check result */
  if (!GWEN_Error_IsOk(err)) {
    if (GWEN_Error_GetType(err)!=
        GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
        (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
         GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
      DBG_DEBUG_ERR(GWEN_LOGDOMAIN, err);
      return GWEN_Error_GetSimpleCode(err);
    }
    GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_WANTREAD);
    return 1;
  }

  if (*bsize==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
    GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_EOFMET);
  }
  else {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Read %d bytes", *bsize);
    GWEN_Text_LogString(buffer, *bsize, GWEN_LOGDOMAIN,
                        GWEN_LoggerLevel_Verbous);
  }
  GWEN_NetLayer_SubFlags(nl, GWEN_NETLAYER_FLAGS_WANTREAD);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerStdio_Write(GWEN_NETLAYER *nl,
                             const char *buffer,
                             int *bsize){
  GWEN_NL_STDIO *nld;
  GWEN_ERRORCODE err;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_STDIO, nl);
  assert(nld);

  /* check status */
  if (GWEN_NetLayer_GetStatus(nl)!=GWEN_NetLayerStatus_Connected) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Socket is not connected (%d)",
              GWEN_NetLayer_GetStatus(nl));
    return GWEN_ERROR_INVALID;
  }

  /* try to write */
  err=GWEN_Socket_Write(nld->socketWrite, buffer, bsize);

  /* check result */
  if (!GWEN_Error_IsOk(err)) {
    if (GWEN_Error_GetType(err)!=
        GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE) ||
        (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
         GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED)) {
      DBG_DEBUG_ERR(GWEN_LOGDOMAIN, err);
      return GWEN_Error_GetSimpleCode(err);
    }
    GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_WANTWRITE);
    return 1;
  }
  DBG_DEBUG(GWEN_LOGDOMAIN, "Written %d bytes", *bsize);
  GWEN_Text_LogString(buffer, *bsize, 0, GWEN_LoggerLevel_Verbous);
  GWEN_NetLayer_SubFlags(nl, GWEN_NETLAYER_FLAGS_WANTWRITE);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerStdio_AddSockets(GWEN_NETLAYER *nl,
                                   GWEN_SOCKETSET *readSet,
                                   GWEN_SOCKETSET *writeSet,
                                   GWEN_SOCKETSET *exSet) {
  GWEN_NL_STDIO *nld;
  GWEN_ERRORCODE err;
  GWEN_TYPE_UINT32 flags;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_STDIO, nl);
  assert(nld);

  flags=GWEN_NetLayer_GetFlags(nl);

  if (flags & GWEN_NETLAYER_FLAGS_WANTREAD) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Adding socket to readSet");
    err=GWEN_SocketSet_AddSocket(readSet, nld->socketRead);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
      return GWEN_Error_GetSimpleCode(err);
    }
  }

  if (flags & GWEN_NETLAYER_FLAGS_WANTWRITE) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Adding socket to writeSet");
    err=GWEN_SocketSet_AddSocket(writeSet, nld->socketWrite);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
      return GWEN_Error_GetSimpleCode(err);
    }
  }

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayerStdio_Work(GWEN_NETLAYER *nl) {
  GWEN_NL_STDIO *nld;
  GWEN_NETLAYER_STATUS st;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_STDIO, nl);
  assert(nld);

  st=GWEN_NetLayer_GetStatus(nl);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Working with status \"%s\" (%d)",
              GWEN_NetLayerStatus_toString(st), st);

  switch(st) {

  case GWEN_NetLayerStatus_Connecting:
  case GWEN_NetLayerStatus_Listening: {
    DBG_ERROR(GWEN_LOGDOMAIN, "Status \"%s\" not supported",
              GWEN_NetLayerStatus_toString(st));
    return GWEN_NetLayerResult_Error;
    break;
  }

  case GWEN_NetLayerStatus_Connected:
    break;

  case GWEN_NetLayerStatus_Unconnected:
  case GWEN_NetLayerStatus_Disabled:
  case GWEN_NetLayerStatus_Disconnecting:
  case GWEN_NetLayerStatus_Disconnected:
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Inactive connection (status \"%s\")",
                GWEN_NetLayerStatus_toString(st));
    break;

  default:
    DBG_WARN(GWEN_LOGDOMAIN, "Hmm, status \"%s\" (%d) is unexpected...",
             GWEN_NetLayerStatus_toString(st), st);
    break;
  } /* switch */

  return GWEN_NetLayerResult_Idle;
}


#endif /* if not WIN32 */




