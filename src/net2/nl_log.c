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


#include "nl_log_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/text.h>

#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



static int gwen_netlayerlog__count=0;


GWEN_INHERIT(GWEN_NETLAYER, GWEN_NL_LOG)


GWEN_NETLAYER *GWEN_NetLayerLog_new(GWEN_NETLAYER *baseLayer,
                                    const char *fileNameBase) {
  GWEN_NETLAYER *nl;
  GWEN_NL_LOG *nld;
  GWEN_TYPE_UINT32 fl;

  assert(baseLayer);
  nl=GWEN_NetLayer_new(GWEN_NL_LOG_NAME);
  GWEN_NEW_OBJECT(GWEN_NL_LOG, nld);
  GWEN_INHERIT_SETDATA(GWEN_NETLAYER, GWEN_NL_LOG, nl, nld,
                       GWEN_NetLayerLog_FreeData);

  nld->nameBase=strdup(fileNameBase);
  nld->inFd=-1;
  nld->outFd=-1;

  GWEN_NetLayer_SetBaseLayer(nl, baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  fl=GWEN_NetLayer_GetFlags(baseLayer) & ~GWEN_NETLAYER_FLAGS_SPECMASK;
  GWEN_NetLayer_SetFlags(nl, fl);
  GWEN_NetLayer_SetParentLayer(baseLayer, nl);

  GWEN_NetLayer_SetLocalAddr(nl, GWEN_NetLayer_GetLocalAddr(baseLayer));
  GWEN_NetLayer_SetPeerAddr(nl, GWEN_NetLayer_GetPeerAddr(baseLayer));

  GWEN_NetLayer_SetWorkFn(nl, GWEN_NetLayerLog_Work);
  GWEN_NetLayer_SetReadFn(nl, GWEN_NetLayerLog_Read);
  GWEN_NetLayer_SetWriteFn(nl, GWEN_NetLayerLog_Write);
  GWEN_NetLayer_SetConnectFn(nl, GWEN_NetLayerLog_Connect);
  GWEN_NetLayer_SetDisconnectFn(nl, GWEN_NetLayerLog_Disconnect);
  GWEN_NetLayer_SetListenFn(nl, GWEN_NetLayerLog_Listen);
  GWEN_NetLayer_SetAddSocketsFn(nl, GWEN_NetLayerLog_AddSockets);
  GWEN_NetLayer_SetBaseStatusChangeFn(nl, GWEN_NetLayerLog_BaseStatusChange);
  GWEN_NetLayer_SetBeginOutPacketFn(nl, GWEN_NetLayerLog_BeginOutPacket);
  GWEN_NetLayer_SetEndOutPacketFn(nl, GWEN_NetLayerLog_EndOutPacket);
  GWEN_NetLayer_SetBeginInPacketFn(nl, GWEN_NetLayerLog_BeginInPacket);
  GWEN_NetLayer_SetCheckInPacketFn(nl, GWEN_NetLayerLog_CheckInPacket);

  return nl;
}



void GWEN_NetLayerLog_FreeData(void *bp, void *p) {
  GWEN_NL_LOG *nld;

  nld=(GWEN_NL_LOG*)p;
  free(nld->nameBase);
  GWEN_FREE_OBJECT(nld);
}



int GWEN_NetLayerLog_Connect(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;
  int rv=0;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);
  if (GWEN_NetLayer_GetStatus(baseLayer)!=GWEN_NetLayerStatus_Connected) {
    rv=GWEN_NetLayer_Connect(baseLayer);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "Result of BaseLayer Connect: %d", rv);
    }
  }
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  GWEN_NetLayer_SubFlags(nl, GWEN_NETLAYER_FLAGS_PASSIVE);
  return rv;
}



int GWEN_NetLayerLog_Disconnect(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  return GWEN_NetLayer_Disconnect(baseLayer);
}



int GWEN_NetLayerLog_Listen(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;
  int rv;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  rv=GWEN_NetLayer_Listen(baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_PASSIVE);
  return rv;
}



int GWEN_NetLayerLog_Read(GWEN_NETLAYER *nl, char *buffer, int *bsize){
  GWEN_NL_LOG *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_LOG, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  rv=GWEN_NetLayer_Read(baseLayer, buffer, bsize);
  if (rv==0 && nld->inFd!=-1 && *bsize!=0) {
    int res;

    res=write(nld->inFd, buffer, *bsize);
    if (res!=*bsize) {
      DBG_ERROR(GWEN_LOGDOMAIN, "write(): %s", strerror(errno));
      close(nld->inFd);
      nld->inFd=-1;
    }
  }

  return rv;
}



int GWEN_NetLayerLog_Write(GWEN_NETLAYER *nl, const char *buffer,int *bsize){
  GWEN_NL_LOG *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_LOG, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  rv=GWEN_NetLayer_Write(baseLayer, buffer, bsize);
  if (rv==0 && nld->outFd!=-1 && *bsize!=0) {
    int res;

    res=write(nld->outFd, buffer, *bsize);
    if (res!=*bsize) {
      DBG_ERROR(GWEN_LOGDOMAIN, "write(): %s", strerror(errno));
      close(nld->outFd);
      nld->outFd=-1;
    }
  }

  return rv;
}



int GWEN_NetLayerLog_AddSockets(GWEN_NETLAYER *nl,
                                 GWEN_SOCKETSET *readSet,
                                 GWEN_SOCKETSET *writeSet,
                                 GWEN_SOCKETSET *exSet) {
  GWEN_NETLAYER *baseLayer;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  return GWEN_NetLayer_AddSockets(baseLayer, readSet, writeSet, exSet);
}



GWEN_NETLAYER_RESULT GWEN_NetLayerLog_Work(GWEN_NETLAYER *nl) {
  GWEN_NL_LOG *nld;
  GWEN_NETLAYER *baseLayer;
  GWEN_NETLAYER_RESULT rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_LOG, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  rv=GWEN_NetLayer_Work(baseLayer);
  GWEN_NetLayer_SetFlags(nl, GWEN_NetLayer_GetFlags(baseLayer));
  return rv;
}



int GWEN_NetLayerLog_BeginOutPacket(GWEN_NETLAYER *nl, int totalSize) {
  GWEN_NETLAYER *baseLayer;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  return GWEN_NetLayer_BeginOutPacket(baseLayer, totalSize);
}



int GWEN_NetLayerLog_EndOutPacket(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  return GWEN_NetLayer_EndOutPacket(baseLayer);
}



void GWEN_NetLayerLog_BaseStatusChange(GWEN_NETLAYER *nl,
                                       GWEN_NETLAYER_STATUS newst) {
  GWEN_NL_LOG *nld;
  GWEN_NETLAYER *baseLayer;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_LOG, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  DBG_NOTICE(GWEN_LOGDOMAIN, "Base has changed its status");
  GWEN_NetLayer_SetStatus(nl, newst);
  GWEN_NetLayer_SetFlags(nl, GWEN_NetLayer_GetFlags(baseLayer));

  if (newst==GWEN_NetLayerStatus_Connected) {
    GWEN_BUFFER *nbuf;
    char numbuf[16];
    GWEN_TYPE_UINT32 pos;

    nbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(nbuf, nld->nameBase);
    snprintf(numbuf, sizeof(numbuf)-1, "%d", ++gwen_netlayerlog__count);
    numbuf[sizeof(numbuf)-1]=0;
    GWEN_Buffer_AppendString(nbuf, "-");
    GWEN_Buffer_AppendString(nbuf, numbuf);
    pos=GWEN_Buffer_GetPos(nbuf);
    GWEN_Buffer_AppendString(nbuf, ".read");
    nld->inFd=open(GWEN_Buffer_GetStart(nbuf),
                   O_WRONLY | O_CREAT | O_EXCL,
                   S_IRUSR | S_IWUSR);
    if (nld->inFd==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "open(%s): %s",
                GWEN_Buffer_GetStart(nbuf),
                strerror(errno));
    }
    GWEN_Buffer_Crop(nbuf, 0, pos);
    GWEN_Buffer_AppendString(nbuf, ".write");
    nld->outFd=open(GWEN_Buffer_GetStart(nbuf),
                    O_WRONLY | O_CREAT | O_EXCL,
                    S_IRUSR | S_IWUSR);
    if (nld->outFd==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "open(%s): %s",
                GWEN_Buffer_GetStart(nbuf),
                strerror(errno));
    }
  }
  else if (newst==GWEN_NetLayerStatus_Disconnected ||
           newst==GWEN_NetLayerStatus_Disabled) {
    if (nld->outFd!=-1) {
      if (close(nld->outFd)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "close(outFd): %s",
                  strerror(errno));
      }
      nld->outFd=-1;
    }
    if (nld->inFd!=-1) {
      if (close(nld->inFd)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "close(inFd): %s",
                  strerror(errno));
      }
      nld->inFd=-1;
    }
  }

}



int GWEN_NetLayerLog_BeginInPacket(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  return GWEN_NetLayer_BeginInPacket(baseLayer);
}



int GWEN_NetLayerLog_CheckInPacket(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  return GWEN_NetLayer_CheckInPacket(baseLayer);
}



