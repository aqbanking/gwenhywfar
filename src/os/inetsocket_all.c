/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002 by Martin Preuss
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

#include "inetsocket_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>



GWEN_ERRORCODE GWEN_Socket__StartOpen(GWEN_SOCKET *sp,
                                      const GWEN_INETADDRESS *addr){
  GWEN_ERRORCODE err;

  err=GWEN_Socket_Open(sp);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return err;
  }
  err=GWEN_Socket_SetBlocking(sp, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return err;
  }

  err=GWEN_Socket_Connect(sp, addr);
  /* not yet finished or real error ? */
  if (!GWEN_Error_IsOk(err)) {
    if (GWEN_Error_GetType(err)!=GWEN_Error_FindType("Socket") ||
        GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_IN_PROGRESS) {
      /* real error, so return that error */
      DBG_ERROR_ERR(0, err)
      return err;
    }
  }
  return 0;
}



GWEN_ERRORCODE GWEN_Socket__CheckOpen(GWEN_SOCKET *sp,
                                      int timeout){
  GWEN_ERRORCODE err;
  GWEN_SOCKETSET *wset;

  assert(sp);
  wset=GWEN_SocketSet_new();

  err=GWEN_SocketSet_AddSocket(wset, sp);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }
  err=GWEN_Socket_Select(0, wset, 0, timeout);
  GWEN_SocketSet_free(wset);
  if (!GWEN_Error_IsOk(err)) {
    if (GWEN_Error_GetType(err)==GWEN_Error_FindType("Socket")) {
      if (GWEN_Error_GetCode(err)==GWEN_SOCKET_ERROR_TIMEOUT) {
        DBG_INFO(0, "Socket timeout");
        return err;
      }
      else if (GWEN_Error_GetCode(err)==GWEN_SOCKET_ERROR_INTERRUPTED) {
        DBG_INFO(0, "Interrupted system call");
        return err;
      }
      else {
        DBG_ERROR_ERR(0, err);
        return err;
      }
    } /* if socket error */
    else {
      DBG_ERROR_ERR(0, err);
      return err;
    }
  } /* if error */

  /* get socket error */
  err=GWEN_Socket_GetSocketError(sp);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return err;
  }

  /* make socket blocking again */
  err=GWEN_Socket_SetBlocking(sp, 1);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return err;
  }

  DBG_INFO(0, "Connected");
  return 0;
}




GWEN_ERRORCODE GWEN_Socket_Connect_Wait(GWEN_SOCKET *sp,
                                        const GWEN_INETADDRESS *addr,
                                        int timeout){
  GWEN_ERRORCODE err;
  time_t startt;
  int distance;
  int count;

  startt=time(0);
  err=GWEN_Socket__StartOpen(sp, addr);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Socket_Close(sp);
    return err;
  }

  if (timeout==0)
    distance=0;
  else if (timeout==-1)
    distance=-1;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance/1000)>timeout)
        distance=timeout/1000;
    if (!distance)
      distance=750;
  }

  for (count=0;;count++) {
    if (GWEN_WaitCallback(count)==GWEN_WaitCallbackResult_Abort) {
      DBG_ERROR(0, "User aborted via waitcallback");
      GWEN_Socket_Close(sp);
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                            GWEN_SOCKET_ERROR_ABORTED);
    }

    err=GWEN_Socket__CheckOpen(sp, distance);
    if (GWEN_Error_IsOk(err)) {
      DBG_INFO(0, "Connected");
      return 0;
    }
    if (timeout==0) {
      DBG_ERROR(0, "Could not connect immediately, aborting");
      GWEN_Socket_Close(sp);
      return err;
    }
    if (GWEN_Error_GetType(err)==GWEN_Error_FindType("Socket")) {
      if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
          GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
        DBG_ERROR_ERR(0, err);
        return err;
      }
    } /* if socket error */
    else {
      DBG_ERROR_ERR(0, err);
      return err;
    }
    if (timeout!=-1) {
      if (difftime(time(0), startt)>timeout) {
        DBG_INFO_ERR(0, err);
        break;
      }
    }
  } /* for */

  DBG_ERROR(0, "Could not connect within %d seconds, aborting", timeout);
  GWEN_Socket_Close(sp);
  return GWEN_Error_new(0,
                        GWEN_ERROR_SEVERITY_ERR,
                        GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                        GWEN_SOCKET_ERROR_TIMEOUT);
}



GWEN_ERRORCODE GWEN_Socket_Accept_Wait(GWEN_SOCKET *sp,
                                       GWEN_INETADDRESS **addr,
                                       GWEN_SOCKET **newsock,
                                       int timeout){
  GWEN_ERRORCODE err;
  time_t startt;
  int distance;
  int count;

  startt=time(0);
  if (timeout==0)
    distance=0;
  else if (timeout==-1)
    distance=-1;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance/1000)>timeout)
        distance=timeout/1000;
    if (!distance)
      distance=750;
  }

  for (count=0;;count++) {
    if (GWEN_WaitCallback(count)==GWEN_WaitCallbackResult_Abort) {
      DBG_ERROR(0, "User aborted via waitcallback");
      GWEN_Socket_Close(sp);
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                            GWEN_SOCKET_ERROR_ABORTED);
    }
    err=GWEN_Socket_WaitForWrite(sp, distance);
    if (GWEN_Error_IsOk(err)) {
      break;
    }
    if (timeout==0) {
      DBG_ERROR(0, "Could not accept immediately, aborting");
      GWEN_Socket_Close(sp);
      return err;
    }
    if (GWEN_Error_GetType(err)==GWEN_Error_FindType("Socket")) {
      if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
          GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
        DBG_ERROR_ERR(0, err);
        return err;
      }
    } /* if socket error */
    else {
      DBG_ERROR_ERR(0, err);
      return err;
    }
    if (timeout!=-1) {
      if (difftime(time(0), startt)>timeout) {
        DBG_INFO_ERR(0, err);
        DBG_ERROR(0, "Could not accept within %d seconds, aborting", timeout);
        GWEN_Socket_Close(sp);
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                              GWEN_SOCKET_ERROR_TIMEOUT);
      }
    }
  } /* for */

  err=GWEN_Socket_Accept(sp, addr, newsock);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    return err;
  }
  return 0;
}



GWEN_ERRORCODE GWEN_Socket_Read_Wait(GWEN_SOCKET *sp,
                                     char *buffer,
                                     int *bsize,
                                     int timeout,
                                     int force){
  GWEN_ERRORCODE err;
  time_t startt;
  int distance;
  int bytesPos;
  int count;

  startt=time(0);
  if (timeout==0)
    distance=0;
  else if (timeout==-1)
    distance=-1;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance/1000)>timeout)
        distance=timeout/1000;
    if (!distance)
      distance=750;
  }

  bytesPos=0;
  count=0;
  while(bytesPos<*bsize) {
    int lsize;

    for (;;count++) {
      if (GWEN_WaitCallback(count)==GWEN_WaitCallbackResult_Abort) {
        DBG_ERROR(0, "User aborted via waitcallback");
        GWEN_Socket_Close(sp);
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                              GWEN_SOCKET_ERROR_ABORTED);
      }
      err=GWEN_Socket_WaitForRead(sp, distance);
      if (GWEN_Error_IsOk(err)) {
        break;
      }
      if (timeout==0) {
        DBG_ERROR(0, "Could not accept immediately, aborting");
        GWEN_Socket_Close(sp);
        return err;
      }
      if (GWEN_Error_GetType(err)==GWEN_Error_FindType("Socket")) {
        if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
            GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
          DBG_ERROR_ERR(0, err);
          return err;
        }
      } /* if socket error */
      else {
        DBG_ERROR_ERR(0, err);
        return err;
      }
      if (timeout!=-1) {
        if (difftime(time(0), startt)>timeout) {
          DBG_INFO_ERR(0, err);
          DBG_ERROR(0, "Could not accept within %d seconds, aborting", timeout);
          GWEN_Socket_Close(sp);
          return GWEN_Error_new(0,
                                GWEN_ERROR_SEVERITY_ERR,
                                GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                                GWEN_SOCKET_ERROR_TIMEOUT);
        }
      }
    } /* for */

    lsize=*bsize-bytesPos;
    err=GWEN_Socket_Read(sp, buffer+bytesPos, &lsize);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(0, err);
      return err;
    }
    bytesPos+=lsize;
    if (bytesPos==*bsize || !force)
      break;
  } /* while */
  *bsize=bytesPos;
  return 0;
}



GWEN_ERRORCODE GWEN_Socket_Write_Wait(GWEN_SOCKET *sp,
                                      const char *buffer,
                                      int *bsize,
                                      int timeout,
                                      int force){
  GWEN_ERRORCODE err;
  time_t startt;
  int distance;
  int bytesPos;
  int count;

  startt=time(0);
  if (timeout==0)
    distance=0;
  else if (timeout==-1)
    distance=-1;
  else {
    distance=GWEN_WaitCallback_GetDistance(0);
    if (distance)
      if ((distance/1000)>timeout)
        distance=timeout/1000;
    if (!distance)
      distance=750;
  }

  bytesPos=0;
  count=0;
  while(bytesPos<*bsize) {
    int lsize;

    for (;;count++) {
      if (GWEN_WaitCallback(count)==GWEN_WaitCallbackResult_Abort) {
        DBG_ERROR(0, "User aborted via waitcallback");
        GWEN_Socket_Close(sp);
        return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                              GWEN_SOCKET_ERROR_ABORTED);
      }
      err=GWEN_Socket_WaitForWrite(sp, distance);
      if (GWEN_Error_IsOk(err)) {
        break;
      }
      if (timeout==0) {
        DBG_ERROR(0, "Could not accept immediately, aborting");
        GWEN_Socket_Close(sp);
        return err;
      }
      if (GWEN_Error_GetType(err)==GWEN_Error_FindType("Socket")) {
        if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_TIMEOUT &&
            GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
          DBG_ERROR_ERR(0, err);
          return err;
        }
      } /* if socket error */
      else {
        DBG_ERROR_ERR(0, err);
        return err;
      }
      if (timeout!=-1) {
        if (difftime(time(0), startt)>timeout) {
          DBG_INFO_ERR(0, err);
          DBG_ERROR(0, "Could not accept within %d seconds, aborting", timeout);
          GWEN_Socket_Close(sp);
          return GWEN_Error_new(0,
                                GWEN_ERROR_SEVERITY_ERR,
                                GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE),
                                GWEN_SOCKET_ERROR_TIMEOUT);
        }
      }
    } /* for */

    lsize=*bsize-bytesPos;
    err=GWEN_Socket_Write(sp, buffer+bytesPos, &lsize);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(0, err);
      return err;
    }
    bytesPos+=lsize;
    if (bytesPos==*bsize || !force)
      break;
  } /* while */
  *bsize=bytesPos;
  return 0;
}










