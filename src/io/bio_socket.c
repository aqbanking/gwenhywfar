/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Fri Feb 07 2003
 copyright   : (C) 2003 by Martin Preuss
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

#define DISABLE_DEBUGLOG


#include "bio_socket_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <gwenhywfar/debug.h>


GWEN_INHERIT(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_SOCKET);


GWEN_BUFFEREDIO_SOCKET *GWEN_BufferedIO_Socket_Table__new() {
  GWEN_BUFFEREDIO_SOCKET *bft;

  GWEN_NEW_OBJECT(GWEN_BUFFEREDIO_SOCKET, bft);

  return bft;
}



void GWEN_BufferedIO_Socket_Table__free(GWEN_BUFFEREDIO_SOCKET *bft) {
  if (bft) {
    GWEN_Socket_free(bft->sock);
    free(bft);
  }
}



GWEN_ERRORCODE GWEN_BufferedIO_Socket__Read(GWEN_BUFFEREDIO *dm,
                                            char *buffer,
                                            int *size,
                                            int timeout){
  GWEN_ERRORCODE err;
  GWEN_BUFFEREDIO_SOCKET *bft;
  int retrycount;

  DBG_DEBUG(0, "Reading %d bytes", *size);
  assert(dm);
  assert(buffer);
  assert(size);
  bft=GWEN_INHERIT_GETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_SOCKET, dm);
  assert(bft);
  assert(bft->sock);
  if (*size<1) {
    DBG_WARN(0, "Nothing to read");
    *size=0;
    return 0;
  }

  if (timeout>=0) {
    retrycount=GWEN_BUFFEREDIO_SOCKET_TRIES;
    while(retrycount) {
      err=GWEN_Socket_WaitForRead(bft->sock, timeout);
      if (!GWEN_Error_IsOk(err)) {
        if (GWEN_Error_GetType(err)==
            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE)) {
          if (GWEN_Error_GetCode(err)==GWEN_SOCKET_ERROR_TIMEOUT)
            return
              GWEN_Error_new(0,
                             GWEN_ERROR_SEVERITY_ERR,
                             GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                             GWEN_BUFFEREDIO_ERROR_TIMEOUT);
          else if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
            DBG_ERROR_ERR(0, err);
            return err;
          }
	} /* if socket error */
	else {
	  DBG_ERROR_ERR(0, err);
          return err;
	}
      }
      else
	break;
      retrycount--;
    } /* while */
    if (retrycount<1) {
      DBG_ERROR(0, "Interrupted too often, giving up");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                            GWEN_BUFFEREDIO_ERROR_READ);
    }
  } /* if timeout */

  /* ok. socket seems to be ready now */
  retrycount=GWEN_BUFFEREDIO_SOCKET_TRIES;
  while(retrycount) {
    err=GWEN_Socket_Read(bft->sock, buffer, size);
    if (!GWEN_Error_IsOk(err)) {
      if (GWEN_Error_GetType(err)==
          GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE)) {
	if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
	  DBG_ERROR_ERR(0, err);
	  return err;
	}
      } /* if socket error */
      else {
	DBG_ERROR_ERR(0, err);
	return err;
      }
    }
    else
      break;
    retrycount--;
  } /* while */
  if (retrycount<1) {
    DBG_ERROR(0, "Interrupted too often, giving up");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_READ);
  }

  DBG_DEBUG(0, "Reading ok (%d bytes)", *size);
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_Socket__Write(GWEN_BUFFEREDIO *dm,
                                             const char *buffer,
                                             int *size,
                                             int timeout){
  GWEN_ERRORCODE err;
  GWEN_BUFFEREDIO_SOCKET *bft;
  int retrycount;

  assert(dm);
  assert(buffer);
  assert(size);
  bft=GWEN_INHERIT_GETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_SOCKET, dm);
  assert(bft);
  assert(bft->sock);
  if (*size<1) {
    DBG_WARN(0, "Nothing to write");
    *size=0;
    return 0;
  }

  if (timeout>=0) {
    retrycount=GWEN_BUFFEREDIO_SOCKET_TRIES;
    while(retrycount) {
      err=GWEN_Socket_WaitForWrite(bft->sock, timeout);
      if (!GWEN_Error_IsOk(err)) {
        if (GWEN_Error_GetType(err)==
            GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE)) {
	  if (GWEN_Error_GetCode(err)==GWEN_SOCKET_ERROR_TIMEOUT)
            return GWEN_Error_new(0,
                                  GWEN_ERROR_SEVERITY_ERR,
                                  GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                                  GWEN_BUFFEREDIO_ERROR_TIMEOUT);
	  else if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
	    DBG_ERROR_ERR(0, err);
	    return err;
	  }
	} /* if socket error */
	else {
	  DBG_ERROR_ERR(0, err);
	  return err;
	}
      }
      else
	break;
      retrycount--;
    } /* while */
    if (retrycount<1) {
      DBG_ERROR(0, "Interrupted too often, giving up");
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                            GWEN_BUFFEREDIO_ERROR_WRITE);
    }
  } /* if timeout */

  /* ok. socket seems to be ready now */
  retrycount=GWEN_BUFFEREDIO_SOCKET_TRIES;
  while(retrycount) {
    err=GWEN_Socket_Write(bft->sock, buffer, size);
    if (!GWEN_Error_IsOk(err)) {
      if (GWEN_Error_GetType(err)==
          GWEN_Error_FindType(GWEN_SOCKET_ERROR_TYPE)) {
	if (GWEN_Error_GetCode(err)!=GWEN_SOCKET_ERROR_INTERRUPTED) {
	  DBG_ERROR_ERR(0, err);
	  return err;
	}
      } /* if socket error */
      else {
	DBG_ERROR_ERR(0, err);
	return err;
      }
    }
    else
      break;

    retrycount--;
  } /* while */
  if (retrycount<1) {
    DBG_ERROR(0, "Interrupted too often, giving up");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_WRITE);
  }

  DBG_VERBOUS(0, "Writing ok");
  return 0;
}



GWEN_ERRORCODE GWEN_BufferedIO_Socket__Close(GWEN_BUFFEREDIO *dm){
  GWEN_BUFFEREDIO_SOCKET *bft;

  assert(dm);
  bft=GWEN_INHERIT_GETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_SOCKET, dm);
  assert(bft);
  assert(bft->sock);
  DBG_DEBUG(0, "Closing socket");
  if (GWEN_Socket_Close(bft->sock)) {
    DBG_ERROR(0, "Could not close (%s)",
	      strerror(errno));
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_BUFFEREDIO_ERROR_TYPE),
                          GWEN_BUFFEREDIO_ERROR_CLOSE);
  }
  return 0;
}



void GWEN_BufferedIO_Socket_FreeData(void *bp, void *p) {
  GWEN_BUFFEREDIO_SOCKET *bft;

  bft=(GWEN_BUFFEREDIO_SOCKET*)p;
  GWEN_BufferedIO_Socket_Table__free(bft);
}



GWEN_BUFFEREDIO *GWEN_BufferedIO_Socket_new(GWEN_SOCKET *sock){
  GWEN_BUFFEREDIO *bt;
  GWEN_BUFFEREDIO_SOCKET *bft;

  bt=GWEN_BufferedIO_new();
  bft=GWEN_BufferedIO_Socket_Table__new();
  bft->sock=sock;

  GWEN_INHERIT_SETDATA(GWEN_BUFFEREDIO, GWEN_BUFFEREDIO_SOCKET,
                       bt, bft,
                       GWEN_BufferedIO_Socket_FreeData);
  GWEN_BufferedIO_SetReadFn(bt, GWEN_BufferedIO_Socket__Read);
  GWEN_BufferedIO_SetWriteFn(bt, GWEN_BufferedIO_Socket__Write);
  GWEN_BufferedIO_SetCloseFn(bt, GWEN_BufferedIO_Socket__Close);
  GWEN_BufferedIO_SetTimeout(bt, GWEN_BUFFEREDIO_SOCKET_TIMEOUT);

  return bt;
}



