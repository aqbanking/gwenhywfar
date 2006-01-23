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


#include "nl_http_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/net2.h>

#include <ctype.h>


GWEN_INHERIT(GWEN_NETLAYER, GWEN_NL_HTTP)


GWEN_NETLAYER *GWEN_NetLayerHttp_new(GWEN_NETLAYER *baseLayer) {
  GWEN_NETLAYER *nl;
  GWEN_NL_HTTP *nld;

  assert(baseLayer);
  nl=GWEN_NetLayer_new(GWEN_NL_HTTP_NAME);
  GWEN_NEW_OBJECT(GWEN_NL_HTTP, nld);
  GWEN_INHERIT_SETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl, nld,
                       GWEN_NetLayerHttp_FreeData);

  GWEN_NetLayer_SetBaseLayer(nl, baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  GWEN_NetLayer_SetParentLayer(baseLayer, nl);

  GWEN_NetLayer_SetLocalAddr(nl, GWEN_NetLayer_GetLocalAddr(baseLayer));
  GWEN_NetLayer_SetPeerAddr(nl, GWEN_NetLayer_GetPeerAddr(baseLayer));

  /* this protocol is packet based */
  GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_PKG_BASED);

  GWEN_NetLayer_SetWorkFn(nl, GWEN_NetLayerHttp_Work);
  GWEN_NetLayer_SetReadFn(nl, GWEN_NetLayerHttp_Read);
  GWEN_NetLayer_SetWriteFn(nl, GWEN_NetLayerHttp_Write);
  GWEN_NetLayer_SetConnectFn(nl, GWEN_NetLayerHttp_Connect);
  GWEN_NetLayer_SetDisconnectFn(nl, GWEN_NetLayerHttp_Disconnect);
  GWEN_NetLayer_SetListenFn(nl, GWEN_NetLayerHttp_Listen);
  GWEN_NetLayer_SetAddSocketsFn(nl, GWEN_NetLayerHttp_AddSockets);
  GWEN_NetLayer_SetBaseStatusChangeFn(nl, GWEN_NetLayerHttp_BaseStatusChange);
  GWEN_NetLayer_SetBeginOutPacketFn(nl, GWEN_NetLayerHttp_BeginOutPacket);
  GWEN_NetLayer_SetEndOutPacketFn(nl, GWEN_NetLayerHttp_EndOutPacket);
  GWEN_NetLayer_SetBeginInPacketFn(nl, GWEN_NetLayerHttp_BeginInPacket);
  GWEN_NetLayer_SetCheckInPacketFn(nl, GWEN_NetLayerHttp_CheckInPacket);

  nld->pversion=GWEN_NetLayerHttpVersion_1_1;

  /* outgoing */
  nld->dbOutHeader=GWEN_DB_Group_new("outHeader");
  nld->outBuffer=GWEN_Buffer_new(0, 512, 0, 1);

  /* incoming */
  nld->dbInHeader=GWEN_DB_Group_new("inHeader");
  nld->inBuffer=GWEN_Buffer_new(0, 512, 0, 1);

  return nl;
}



void GWEN_NetLayerHttp_FreeData(void *bp, void *p) {
  GWEN_NL_HTTP *nld;

  nld=(GWEN_NL_HTTP*)p;

  GWEN_DB_Group_free(nld->dbInHeader);
  GWEN_Buffer_free(nld->inBuffer);
  free(nld->inCommand);
  GWEN_Url_free(nld->inUrl);
  free(nld->inStatusText);

  GWEN_Buffer_free(nld->outBuffer);
  free(nld->outCommand);
  GWEN_Url_free(nld->outUrl);
  free(nld->outStatusText);
  GWEN_DB_Group_free(nld->dbOutHeader);

  GWEN_FREE_OBJECT(nld);
}



int GWEN_NetLayerHttp_Connect(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;
  int rv;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);
  rv=GWEN_NetLayer_Connect(baseLayer);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Result of BaseLayer Connect: %d", rv);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  GWEN_NetLayer_SubFlags(nl, GWEN_NETLAYER_FLAGS_PASSIVE);
  return rv;
}



int GWEN_NetLayerHttp_Disconnect(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;
  int rv;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);
  rv=GWEN_NetLayer_Disconnect(baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  return rv;
}



int GWEN_NetLayerHttp_Listen(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;
  int rv;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);
  rv=GWEN_NetLayer_Listen(baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_PASSIVE);
  return rv;
}



int GWEN_NetLayerHttp_Read(GWEN_NETLAYER *nl, char *buffer, int *bsize){
  GWEN_NL_HTTP *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  if (nld->inMode==GWEN_NetLayerHttpInMode_Idle) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not in reading mode");
    return GWEN_ERROR_INVALID;
  }

  if (nld->inMode==GWEN_NetLayerHttpInMode_ReadDone) {
    DBG_WARN(GWEN_LOGDOMAIN, "Body fully read.");
    *bsize=0; /* flag EOF */ /* TODO: return error */
    return 0;
  }

  if (nld->inMode!=GWEN_NetLayerHttpInMode_ReadBody) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Still in command/header read mode");
    return 1;
  }

  if (nld->inChunkSize!=-1) {
    int lsize;

    lsize=nld->inChunkSize-nld->inChunkRead;
    assert(lsize>=0);
    if (*bsize<lsize)
      lsize=*bsize;

    rv=GWEN_NetLayer_Read(baseLayer, buffer, &lsize);
    if (rv==0) {
      *bsize=lsize;
      nld->inChunkRead+=*bsize;
      if (nld->inChunkRead==nld->inChunkSize) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "Chunk finished");
        nld->inMode=GWEN_NetLayerHttpInMode_ReadChunkSize;
      }
    }
  }
  else if (nld->inBodySize!=-1) {
    int lsize;

    lsize=nld->inBodySize-nld->inBodyRead;
    if (*bsize<lsize)
      lsize=*bsize;
    rv=GWEN_NetLayer_Read(baseLayer, buffer, &lsize);
    if (rv==0)
      *bsize=lsize;
  }
  else
    rv=GWEN_NetLayer_Read(baseLayer, buffer, bsize);

  if (rv==0)
    nld->inBodyRead+=*bsize;

  return rv;
}



int GWEN_NetLayerHttp_Write(GWEN_NETLAYER *nl, const char *buffer,int *bsize){
  GWEN_NL_HTTP *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  if (nld->outBodySize!=-1 &&
      nld->outBodyWritten+*bsize>nld->outBodySize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Too many bytes for body (%d>=%d)",
              nld->outBodyWritten+*bsize,
              nld->outBodySize);
    return GWEN_ERROR_EOF;
  }

  if (GWEN_Buffer_GetBytesLeft(nld->outBuffer)) {
    /* still some bytes in the buffer, let this layer send them first */
    return 1; /* wouldBlock */
  }
  rv=GWEN_NetLayer_Write(baseLayer, buffer, bsize);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  if (rv==0)
    nld->outBodyWritten+=*bsize;
  return rv;
}



int GWEN_NetLayerHttp_AddSockets(GWEN_NETLAYER *nl,
                                 GWEN_SOCKETSET *readSet,
                                 GWEN_SOCKETSET *writeSet,
                                 GWEN_SOCKETSET *exSet) {
  GWEN_NETLAYER *baseLayer;
  int rv;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);
  rv=GWEN_NetLayer_AddSockets(baseLayer, readSet, writeSet, exSet);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  return rv;
}



int GWEN_NetLayerHttp__WriteBuffer(GWEN_NETLAYER *nl) {
  GWEN_NL_HTTP *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;
  int bsize;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  bsize=GWEN_Buffer_GetBytesLeft(nld->outBuffer);
  if (bsize==0)
    return 0;

  if (bsize>1024)
    bsize=1024;

  rv=GWEN_NetLayer_Write(baseLayer,
			 GWEN_Buffer_GetPosPointer(nld->outBuffer),
			 &bsize);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else if (rv==1)
    return rv;

  if (bsize==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Nothing written");
    return GWEN_ERROR_BROKEN_PIPE;
  }
  GWEN_Buffer_IncrementPos(nld->outBuffer, bsize);

  return 0;
}



GWEN_NETLAYER_RESULT GWEN_NetLayerHttp__WriteWork(GWEN_NETLAYER *nl) {
  GWEN_NL_HTTP *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;
  GWEN_NETLAYER_STATUS st;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  st=GWEN_NetLayer_GetStatus(nl);
  if (st==GWEN_NetLayerStatus_Listening)
    return GWEN_NetLayerResult_Idle;

  if (GWEN_Buffer_GetBytesLeft(nld->outBuffer)) {
    rv=GWEN_NetLayerHttp__WriteBuffer(nl);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return GWEN_NetLayerResult_Error;
    }
    else if (rv==1)
      return GWEN_NetLayerResult_WouldBlock;
    else
      return GWEN_NetLayerResult_Changed;
  }
  else
    return GWEN_NetLayerResult_Idle;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerHttp__ParseCommand(GWEN_NETLAYER *nl, const char *buffer) {
  GWEN_NL_HTTP *nld;
  char *tmp;
  char *p;
  char *s;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  tmp=strdup(buffer);
  s=tmp;

  /* read command */
  p=strchr(s, ' ');
  if (!p) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Bad format of HTTP request (%s)", buffer);
    free(tmp);
    return GWEN_ERROR_INVALID;
  }
  *p=0;
  p++;

  nld->inCommand=strdup(s);
  s=p;

  /* read URL */
  p=strchr(s, ' ');
  if (!p) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Bad format of HTTP request (%s)", buffer);
    free(tmp);
    return GWEN_ERROR_INVALID;
  }
  *p=0;
  p++;

  nld->inUrl=GWEN_Url_fromCommandString(s);
  if (!nld->inUrl) {
    DBG_INFO(GWEN_LOGDOMAIN, "Bad URL \"%s\"", s);
    free(tmp);
    return GWEN_ERROR_INVALID;
  }
  s=p;

  if (*s==0) {
    /* no protocol information follows, so we assume HTTP/0.9 */
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad request (not in HTTP>=1.0)");
    free(tmp);
    return GWEN_ERROR_INVALID;
  }
  else {
    p=strchr(s, '\r');
    if (p)
      *p=0;

    if (strcasecmp(s, "HTTP/1.0")==0)
      nld->pversion=GWEN_NetLayerHttpVersion_1_0;
    else if (strcasecmp(s, "HTTP/1.1")==0)
      nld->pversion=GWEN_NetLayerHttpVersion_1_1;
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad request (invalid protocol \"%s\")", s);
      free(tmp);
      return GWEN_ERROR_INVALID;
    }
  }

  free(tmp);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerHttp__ParseStatus(GWEN_NETLAYER *nl, const char *buffer) {
  GWEN_NL_HTTP *nld;
  char *tmp;
  char *p;
  char *s;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  tmp=strdup(buffer);
  s=tmp;

  /* read protocol */
  p=strchr(s, ' ');
  if (!p) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Bad format of HTTP status (%s)", buffer);
    free(tmp);
    return GWEN_ERROR_INVALID;
  }
  *p=0;
  p++;

  if (strcasecmp(s, "HTTP/1.0")==0)
    nld->pversion=GWEN_NetLayerHttpVersion_1_0;
  else if (strcasecmp(s, "HTTP/1.1")==0)
    nld->pversion=GWEN_NetLayerHttpVersion_1_1;
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad request (invalid protocol \"%s\")", s);
    free(tmp);
    return GWEN_ERROR_INVALID;
  }
  s=p;

  /* read status code */
  while(*p && isdigit(*p))
    p++;
  if (*p) {
    *p=0;
    p++;
  }
  if (1!=sscanf(s, "%d", &(nld->inStatusCode))) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad request (status code \"%s\")", s);
    free(tmp);
    return GWEN_ERROR_INVALID;
  }
  s=p;

  /* read text */
  p=strchr(s, '\r');
  if (p)
    *p=0;

  nld->inStatusText=strdup(s);

  DBG_DEBUG(GWEN_LOGDOMAIN, "Status: %d (%s)",
            nld->inStatusCode, nld->inStatusText);

  free(tmp);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetLayerHttp__ParseHeader(GWEN_NETLAYER *nl, const char *buffer) {
  GWEN_NL_HTTP *nld;
  char *tmp;
  char *p;
  char *s;
  char *varName=0;
  int line;
  GWEN_BUFFER *vbuf;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  tmp=strdup(buffer);
  s=tmp;

  vbuf=GWEN_Buffer_new(0, 256, 0, 1);
  for (line=1;;line++) {
    if (!isspace(*s)) {
      if (GWEN_Buffer_GetUsedBytes(vbuf) && varName) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "Got header: %s=%s",
                  varName,
                  GWEN_Buffer_GetStart(vbuf));

        GWEN_DB_SetCharValue(nld->dbInHeader,
                             GWEN_DB_FLAGS_DEFAULT,
                             varName,
                             GWEN_Buffer_GetStart(vbuf));
        GWEN_Buffer_Reset(vbuf);
        varName=0;
      }

      p=strchr(s, ':');
      if (!p) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad header line %d \"%s\"", line, s);
        GWEN_Buffer_free(vbuf);
        free(tmp);
        return GWEN_ERROR_INVALID;
      }
      *p=0;
      p++;

      varName=s;
      while(*p && isspace(*p))
        p++;
      if (*p==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad header line %d \"%s\"", line, s);
        GWEN_Buffer_free(vbuf);
        free(tmp);
        return GWEN_ERROR_INVALID;
      }
      s=p;
    }
    else {
      while(*s && isspace(*s))
        s++;
      if (*s==0) {
        if (GWEN_Buffer_GetUsedBytes(vbuf) && varName) {
	  DBG_DEBUG(GWEN_LOGDOMAIN, "Got header: %s=%s",
                    varName,
                    GWEN_Buffer_GetStart(vbuf));
          GWEN_DB_SetCharValue(nld->dbInHeader,
			       GWEN_DB_FLAGS_DEFAULT,
			       varName,
			       GWEN_Buffer_GetStart(vbuf));
	  GWEN_Buffer_Reset(vbuf);
	  varName=0;
	}
        GWEN_Buffer_free(vbuf);
        free(tmp);
	return 0;
      }
    }

    p=strchr(s, '\r');
    if (!p) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad header line %d \"%s\"", line, s);
      GWEN_Buffer_free(vbuf);
      free(tmp);
      return GWEN_ERROR_INVALID;
    }
    *p=0;
    p++;
    if (*p!='\n') {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad header line %d \"%s\"", line, s);
      GWEN_Buffer_free(vbuf);
      free(tmp);
      return GWEN_ERROR_INVALID;
    }
    p++;
    if (GWEN_Buffer_GetUsedBytes(vbuf))
      GWEN_Buffer_AppendString(vbuf, " ");
    GWEN_Buffer_AppendString(vbuf, s);

    s=p;
  } /* for line */

  free(tmp);
  GWEN_Buffer_free(vbuf);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayerHttp__ReadWork(GWEN_NETLAYER *nl) {
  GWEN_NL_HTTP *nld;
  GWEN_NETLAYER *baseLayer;
  char buffer[2];
  int rv;
  GWEN_NETLAYER_STATUS st;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  st=GWEN_NetLayer_GetStatus(nl);
  if (st==GWEN_NetLayerStatus_Listening)
    return GWEN_NetLayerResult_Idle;
  else if (st==GWEN_NetLayerStatus_Connecting) {
    return GWEN_NetLayerResult_Idle;
  }


  if (nld->inMode==GWEN_NetLayerHttpInMode_ReadBody ||
      nld->inMode==GWEN_NetLayerHttpInMode_Idle ||
      nld->inMode==GWEN_NetLayerHttpInMode_ReadDone)
    return GWEN_NetLayerResult_Idle;
  else if (nld->inMode==GWEN_NetLayerHttpInMode_Aborted) {
    DBG_INFO(GWEN_LOGDOMAIN, "Operation previously aborted");
    return GWEN_NetLayerResult_Error;
  }
  else if (nld->inMode==GWEN_NetLayerHttpInMode_ReadCommand ||
           nld->inMode==GWEN_NetLayerHttpInMode_ReadStatus ||
           nld->inMode==GWEN_NetLayerHttpInMode_ReadHeader ||
           nld->inMode==GWEN_NetLayerHttpInMode_ReadChunkSize ||
           nld->inMode==GWEN_NetLayerHttpInMode_ReadChunkTrailer) {
    for (;;) {
      int bsize;
      unsigned char c;

      /* read next char */
      memset(buffer, 0, sizeof(buffer));

      bsize=1;
      rv=GWEN_NetLayer_Read(baseLayer, buffer, &bsize);
      if (rv==1)
        return GWEN_NetLayerResult_WouldBlock;
      else if (rv<0) {
	if (GWEN_Net_GetIsWorkDebugMode()) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
	}
	else {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	}
        nld->inMode=GWEN_NetLayerHttpInMode_Aborted;
        return GWEN_NetLayerResult_Error;
      }
      c=(unsigned char)(buffer[0]);
      GWEN_Buffer_AppendByte(nld->inBuffer, c);

      if (nld->inMode==GWEN_NetLayerHttpInMode_ReadHeader) {
        GWEN_TYPE_UINT32 i;

        DBG_DEBUG(GWEN_LOGDOMAIN, "Reading header...");
        i=(nld->inLast4Bytes<<8)+(c & 0xff);
        nld->inLast4Bytes=i;
        if (i==0x0d0a0d0a) {
          const char *p;

          DBG_DEBUG(GWEN_LOGDOMAIN, "Header complete");
          p=GWEN_Buffer_GetStart(nld->inBuffer);
          rv=GWEN_NetLayerHttp__ParseHeader(nl, p);
          if (rv) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            abort();
            nld->inMode=GWEN_NetLayerHttpInMode_Aborted;
            return GWEN_NetLayerResult_Error;
          }
          GWEN_Buffer_Reset(nld->inBuffer);
          nld->inBodySize=-1;
          nld->inBodySize=GWEN_DB_GetIntValue(nld->dbInHeader,
                                              "Content-Length", 0,
                                              nld->inBodySize);
          nld->inBodyRead=0;
          if (nld->inBodySize==0)
            nld->inMode=GWEN_NetLayerHttpInMode_ReadDone;
          else
            nld->inMode=GWEN_NetLayerHttpInMode_ReadBody;

          /* check for chunked transfer */
          nld->inChunkSize=-1;
          nld->inChunkRead=0;
          p=GWEN_DB_GetCharValue(nld->dbInHeader,
                                 "Transfer-Encoding", 0, 0);
          if (p && (-1!=GWEN_Text_ComparePattern(p, "*chunked*", 0))) {
            nld->inMode=GWEN_NetLayerHttpInMode_ReadChunkSize;
          }

          return GWEN_NetLayerResult_Changed;
        }

        // TODO
      } /* if in header read mode */
      else {
        if (c==10) {
          if (nld->inMode==GWEN_NetLayerHttpInMode_ReadCommand) {
            const char *p;

            DBG_DEBUG(GWEN_LOGDOMAIN, "Command line complete");
            p=GWEN_Buffer_GetStart(nld->inBuffer);
            rv=GWEN_NetLayerHttp__ParseCommand(nl, p);
            if (rv) {
              DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
              nld->inMode=GWEN_NetLayerHttpInMode_Aborted;
              return GWEN_NetLayerResult_Error;
            }
            nld->inMode=GWEN_NetLayerHttpInMode_ReadHeader;
            GWEN_Buffer_Reset(nld->inBuffer);
            nld->inLast4Bytes=0;
            return GWEN_NetLayerResult_Changed;
          }
          else if (nld->inMode==GWEN_NetLayerHttpInMode_ReadStatus) {
            const char *p;

            DBG_DEBUG(GWEN_LOGDOMAIN, "status line complete");
            p=GWEN_Buffer_GetStart(nld->inBuffer);
            rv=GWEN_NetLayerHttp__ParseStatus(nl, p);
            if (rv) {
	      if (GWEN_Net_GetIsWorkDebugMode()) {
		DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
	      }
	      else {
		DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	      }
              nld->inMode=GWEN_NetLayerHttpInMode_Aborted;
              return GWEN_NetLayerResult_Error;
            }
            nld->inMode=GWEN_NetLayerHttpInMode_ReadHeader;
            GWEN_Buffer_Reset(nld->inBuffer);
            nld->inLast4Bytes=0;
            return GWEN_NetLayerResult_Changed;
          }
          else if (nld->inMode==GWEN_NetLayerHttpInMode_ReadChunkSize) {
            const char *p;
            int i;

            p=GWEN_Buffer_GetStart(nld->inBuffer);
            while(*p) {
              if (isxdigit(*p))
                break;
              p++;
            }
            if (*p) {
              DBG_DEBUG(GWEN_LOGDOMAIN, "chunk size line complete (\"%s\")",
                       p);
              if (1!=sscanf(p, "%x", &i)) {
                DBG_ERROR(GWEN_LOGDOMAIN, "Bad chunk size (\"%s\")", p);
                nld->inMode=GWEN_NetLayerHttpInMode_Aborted;
                return GWEN_NetLayerResult_Error;
              }
              if (i==0) {
                /* no more chunks */
                DBG_DEBUG(GWEN_LOGDOMAIN, "Last chunk finished");
                nld->inMode=GWEN_NetLayerHttpInMode_ReadChunkTrailer;
                return GWEN_NetLayerResult_Changed;
              }

              if (nld->inBodySize!=-1) {
                /* check for plausibility */
                if (nld->inBodyRead+i>nld->inBodySize) {
                  DBG_ERROR(GWEN_LOGDOMAIN, "Chunk size too high (\"%s\")", p);
                  nld->inMode=GWEN_NetLayerHttpInMode_Aborted;
                  return GWEN_NetLayerResult_Error;
                }
              }
              nld->inChunkSize=i;
              nld->inChunkRead=0;
              DBG_DEBUG(GWEN_LOGDOMAIN, "Reading next %d bytes of data", i);
              nld->inMode=GWEN_NetLayerHttpInMode_ReadBody;
              GWEN_Buffer_Reset(nld->inBuffer);
              return GWEN_NetLayerResult_Changed;
            }
          }
          else if (nld->inMode==GWEN_NetLayerHttpInMode_ReadChunkTrailer) {
            DBG_DEBUG(GWEN_LOGDOMAIN, "chunk trailer finished");
            nld->inMode=GWEN_NetLayerHttpInMode_ReadDone;
            GWEN_Buffer_Reset(nld->inBuffer);
            nld->inLast4Bytes=0;
            return GWEN_NetLayerResult_Changed;
          }
        } /* if line feed */
        else {
          if (GWEN_Buffer_GetUsedBytes(nld->inBuffer)>
              GWEN_NL_HTTP_MAX_CMD_OR_STATUS_SIZE) {
            DBG_ERROR(GWEN_LOGDOMAIN,
                      "Too many bytes for command/status (%d)",
                      GWEN_Buffer_GetUsedBytes(nld->inBuffer));
            nld->inMode=GWEN_NetLayerHttpInMode_Aborted;
            return GWEN_NetLayerResult_Error;
          }
        } /* if not a line feed */
      } /* if not in header read mode */
    } /* for */
  }
  abort();
}




GWEN_NETLAYER_RESULT GWEN_NetLayerHttp_Work(GWEN_NETLAYER *nl) {
  GWEN_NL_HTTP *nld;
  GWEN_NETLAYER_RESULT bres=GWEN_NetLayerResult_Idle;
  GWEN_NETLAYER_RESULT res;
  GWEN_NETLAYER *baseLayer;
  GWEN_NETLAYER_STATUS st;

#define GWEN_NL_HTTP_MERGE_RESULTS(r1, r2)    \
  if (r1==GWEN_NetLayerResult_Idle)           \
    r1=r2;                                    \
  else if (r1!=GWEN_NetLayerResult_Changed) { \
    if (r2==GWEN_NetLayerResult_Changed)      \
      r1=GWEN_NetLayerResult_Changed;          \
  }

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  st=GWEN_NetLayer_GetStatus(nl);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Working with status \"%s\" (%d)",
              GWEN_NetLayerStatus_toString(st), st);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  if (st==GWEN_NetLayerStatus_Listening) {
    GWEN_NETLAYER *newNl;

    newNl=GWEN_NetLayer_GetIncomingLayer(baseLayer);
    if (newNl) {
      GWEN_NETLAYER *newNlHttp;

      newNlHttp=GWEN_NetLayerHttp_new(newNl);
      GWEN_NetLayer_AddFlags(newNlHttp, GWEN_NETLAYER_FLAGS_PASSIVE);
      if (GWEN_NetLayer_GetFlags(nl) & GWEN_NL_HTTP_FLAGS_IPC)
        GWEN_NetLayer_AddFlags(newNlHttp, GWEN_NL_HTTP_FLAGS_IPC);
      GWEN_NetLayer_free(newNl);
      GWEN_NetLayer_AddIncomingLayer(nl, newNlHttp);
      bres=GWEN_NetLayerResult_Changed;
    }
  }

  res=GWEN_NetLayer_Work(baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  if (GWEN_Net_GetIsWorkDebugMode()) {
    DBG_WARN(GWEN_LOGDOMAIN,
             "Result of BaseLayer work (%s): %s",
             GWEN_NetLayer_GetTypeName(baseLayer),
             GWEN_NetLayerResult_toString(res));
  }
  if (res==GWEN_NetLayerResult_Error) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return res;
  }
  GWEN_NL_HTTP_MERGE_RESULTS(bres, res);

  res=GWEN_NetLayerHttp__WriteWork(nl);
  if (GWEN_Net_GetIsWorkDebugMode()) {
    DBG_WARN(GWEN_LOGDOMAIN,
             "Result of WriteWork (%s): %s",
             GWEN_NetLayer_GetTypeName(nl),
             GWEN_NetLayerResult_toString(res));
  }
  if (res==GWEN_NetLayerResult_Error) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return res;
  }
  GWEN_NL_HTTP_MERGE_RESULTS(bres, res);

  res=GWEN_NetLayerHttp__ReadWork(nl);
  if (GWEN_Net_GetIsWorkDebugMode()) {
    DBG_WARN(GWEN_LOGDOMAIN,
             "Result of ReadWork (%s): %s",
             GWEN_NetLayer_GetTypeName(nl),
             GWEN_NetLayerResult_toString(res));
  }
  if (res==GWEN_NetLayerResult_Error) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return res;
  }
  GWEN_NL_HTTP_MERGE_RESULTS(bres, res);

#undef GWEN_NL_HTTP_MERGE_RESULTS

  /* return cumulated result */
  if (GWEN_Net_GetIsWorkDebugMode()) {
    DBG_WARN(GWEN_LOGDOMAIN,
             "Result of Work (%s): %s",
             GWEN_NetLayer_GetTypeName(nl),
             GWEN_NetLayerResult_toString(bres));
  }
  return bres;
}



int GWEN_NetLayerHttp_BeginOutPacket(GWEN_NETLAYER *nl, int totalSize) {
  GWEN_NL_HTTP *nld;
  GWEN_NETLAYER *baseLayer;
  GWEN_NETLAYER_STATUS st;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  st=GWEN_NetLayer_GetStatus(nl);
  if (st==GWEN_NetLayerStatus_Unconnected ||
      st==GWEN_NetLayerStatus_Disconnected) {
    DBG_INFO(GWEN_LOGDOMAIN, "Not connected");
    return GWEN_ERROR_NOT_CONNECTED;
  }

  if (st!=GWEN_NetLayerStatus_Connected) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not connected");
    return GWEN_ERROR_INVALID;
  }

  rv=GWEN_NetLayer_BeginOutPacket(baseLayer, -1);
  if (rv && rv!=GWEN_ERROR_UNSUPPORTED)
    return rv;

  GWEN_Buffer_Reset(nld->outBuffer);
  nld->outBodySize=totalSize;
  nld->outBodyWritten=0;

  /* prepare first line (either status or command) */
  if ((GWEN_NetLayer_GetFlags(nl) & GWEN_NETLAYER_FLAGS_PASSIVE) &&
      !(GWEN_NetLayer_GetFlags(nl) & GWEN_NL_HTTP_FLAGS_IPC)) {
    char numbuf[32];

    /* passive, prepare status line */
    DBG_DEBUG(GWEN_LOGDOMAIN, "Passive connection");
    GWEN_Buffer_AppendString(nld->outBuffer, "HTTP/");
    switch(nld->pversion) {
    case GWEN_NetLayerHttpVersion_1_0:
      GWEN_Buffer_AppendString(nld->outBuffer, "1.0");
      break;
    case GWEN_NetLayerHttpVersion_1_1:
      GWEN_Buffer_AppendString(nld->outBuffer, "1.1");
      break;
    }
    snprintf(numbuf, sizeof(numbuf)-1, " %d", nld->outStatusCode);
    GWEN_Buffer_AppendString(nld->outBuffer, numbuf);
    if (nld->outStatusText) {
      GWEN_Buffer_AppendString(nld->outBuffer, " ");
      GWEN_Buffer_AppendString(nld->outBuffer, nld->outStatusText);
    }
  }
  else {
    /* prepare command line */
    DBG_DEBUG(GWEN_LOGDOMAIN, "Active connection");
    assert(nld->outCommand);
    GWEN_Buffer_AppendString(nld->outBuffer, nld->outCommand);
    GWEN_Buffer_AppendString(nld->outBuffer, " ");
    if (nld->outUrl) {
      int rv;
  
      rv=GWEN_Url_toCommandString(nld->outUrl, nld->outBuffer);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
    else
      GWEN_Buffer_AppendString(nld->outBuffer, "/");

    GWEN_Buffer_AppendString(nld->outBuffer, " HTTP/");
    switch(nld->pversion) {
    case GWEN_NetLayerHttpVersion_1_0:
      GWEN_Buffer_AppendString(nld->outBuffer, "1.0");
      break;
    case GWEN_NetLayerHttpVersion_1_1:
      GWEN_Buffer_AppendString(nld->outBuffer, "1.1");
      break;
    }
  }
  GWEN_Buffer_AppendString(nld->outBuffer, "\r\n");

  /* prepare header */
  GWEN_DB_DeleteVar(nld->dbOutHeader, "Content-Length");
  if (totalSize>=0)
    GWEN_DB_SetIntValue(nld->dbOutHeader,
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "Content-Length", totalSize);

  /* write header to buffer */
  if (GWEN_DB_Variables_Count(nld->dbOutHeader)) {
    rv=GWEN_DB_WriteToBuffer(nld->dbOutHeader, nld->outBuffer,
                             GWEN_DB_FLAGS_HTTP);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  /* append an empty line (marks the end of the header) */
  GWEN_Buffer_AppendString(nld->outBuffer, "\r\n");

  /* the outbody size is now known, set it */
  if (totalSize!=-1) {
    GWEN_NetLayer_SetOutBodySize(baseLayer,
                                 GWEN_Buffer_GetUsedBytes(nld->outBuffer)+
                                 totalSize);
  }

  GWEN_Buffer_Rewind(nld->outBuffer);

  /* try to write buffer content to detect when the connection is down */
  rv=GWEN_NetLayerHttp__WriteBuffer(nl);
  if (rv<0) {
    if (rv==GWEN_ERROR_BROKEN_PIPE) {
      DBG_INFO(GWEN_LOGDOMAIN,
               "Connection is down, caller will have to reconnect");
      return GWEN_ERROR_NOT_CONNECTED;
    }
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Outgoing packet started");
  return 0;
}



int GWEN_NetLayerHttp_EndOutPacket(GWEN_NETLAYER *nl) {
  GWEN_NL_HTTP *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;
  int done=0;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  if (GWEN_Buffer_GetBytesLeft(nld->outBuffer)==0) {
    rv=GWEN_NetLayer_EndOutPacket(baseLayer);
    if (rv<0 && rv!=GWEN_ERROR_UNSUPPORTED)
      return rv;
    if (rv!=1)
      done++;
  }
  else {
    DBG_DEBUG(GWEN_LOGDOMAIN, "More data to be written");
  }

  if (done==0)
    return 1;

  return 0;
}



void GWEN_NetLayerHttp_SetOutCommand(GWEN_NETLAYER *nl,
                                     const char *command,
                                     const GWEN_URL *url) {
  GWEN_NL_HTTP *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  assert(command);
  assert(url);

  free(nld->outCommand);
  nld->outCommand=strdup(command);
  GWEN_Url_free(nld->outUrl);
  nld->outUrl=GWEN_Url_dup(url);
}



void GWEN_NetLayerHttp_SetOutStatus(GWEN_NETLAYER *nl,
                                    int code,
                                    const char *text) {
  GWEN_NL_HTTP *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  nld->outStatusCode=code;
  free(nld->outStatusText);
  if (text)
    nld->outStatusText=strdup(text);
  else
    nld->outStatusText=0;
}



void GWEN_NetLayerHttp_BaseStatusChange(GWEN_NETLAYER *nl,
                                        GWEN_NETLAYER_STATUS newst) {
  GWEN_NL_HTTP *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  DBG_DEBUG(GWEN_LOGDOMAIN, "Base has changed its status");
}



int GWEN_NetLayerHttp_BeginInPacket(GWEN_NETLAYER *nl) {
  GWEN_NL_HTTP *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  rv=GWEN_NetLayer_BeginInPacket(baseLayer);
  if (rv && rv!=GWEN_ERROR_UNSUPPORTED)
    return rv;

  if ((GWEN_NetLayer_GetFlags(nl) & GWEN_NETLAYER_FLAGS_PASSIVE) &&
      !(GWEN_NetLayer_GetFlags(nl) & GWEN_NL_HTTP_FLAGS_IPC)) {
    nld->inMode=GWEN_NetLayerHttpInMode_ReadCommand;
  }
  else {
    nld->inMode=GWEN_NetLayerHttpInMode_ReadStatus;
  }

  GWEN_Buffer_Reset(nld->inBuffer);
  GWEN_DB_ClearGroup(nld->dbInHeader, 0);
  nld->inBodySize=-1;
  nld->inBodyRead=0;
  free(nld->inCommand);
  nld->inCommand=0;
  GWEN_Url_free(nld->inUrl);
  nld->inUrl=0;
  nld->inStatusCode=0;
  free(nld->inStatusText);
  nld->inStatusText=0;
  nld->inLast4Bytes=0;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Starting to read incoming packet");
  return 0;
}



int GWEN_NetLayerHttp_CheckInPacket(GWEN_NETLAYER *nl) {
  GWEN_NL_HTTP *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "Read mode (%d)", nld->inMode);

  switch(nld->inMode) {
  case GWEN_NetLayerHttpInMode_Idle:
    DBG_ERROR(GWEN_LOGDOMAIN, "Not in read mode");
    return -1;

  case GWEN_NetLayerHttpInMode_ReadCommand:
  case GWEN_NetLayerHttpInMode_ReadStatus:
  case GWEN_NetLayerHttpInMode_ReadHeader:
    return 1;

  case GWEN_NetLayerHttpInMode_ReadBody:
    if (nld->inBodySize!=-1) {
      if (nld->inBodyRead>=nld->inBodySize) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "Read %d of %d",
                  nld->inBodyRead,
                  nld->inBodySize);
        nld->inMode=GWEN_NetLayerHttpInMode_ReadDone;
        DBG_DEBUG(GWEN_LOGDOMAIN, "Body complete.");
        return 0;
      }
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "Unknown incoming size");
    }
    return 1;

  case GWEN_NetLayerHttpInMode_ReadChunkSize:
  case GWEN_NetLayerHttpInMode_ReadChunkTrailer:
    return 1;

  case GWEN_NetLayerHttpInMode_ReadDone:
    DBG_DEBUG(GWEN_LOGDOMAIN, "Body already complete.");
    return 0;

  case GWEN_NetLayerHttpInMode_Aborted:
    DBG_ERROR(GWEN_LOGDOMAIN, "Aborted");
    return -1;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unhandled read mode (%d)", nld->inMode);
    return -1;
  }
}



GWEN_DB_NODE *GWEN_NetLayerHttp_GetOutHeader(const GWEN_NETLAYER *nl) {
  GWEN_NL_HTTP *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  return nld->dbOutHeader;
}



GWEN_DB_NODE *GWEN_NetLayerHttp_GetInHeader(const GWEN_NETLAYER *nl) {
  GWEN_NL_HTTP *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  return nld->dbInHeader;
}



int GWEN_NetLayerHttp_GetInStatusCode(const GWEN_NETLAYER *nl) {
  GWEN_NL_HTTP *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  return nld->inStatusCode;
}



const char *GWEN_NetLayerHttp_GetInStatusText(const GWEN_NETLAYER *nl) {
  GWEN_NL_HTTP *nld;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  return nld->inStatusText;
}



int GWEN_NetLayerHttp__RecvPacketBio(GWEN_NETLAYER *nl,
                                     GWEN_BUFFEREDIO *bio,
                                     int timeout) {
  GWEN_NL_HTTP *nld;
  static char buffer[512];
  int bsize;
  int rv;
  time_t startt;
  int tLeft;
  GWEN_ERRORCODE err;
  int bodyStarted=0;
  int ignoreBody=0;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  startt=time(0);

  rv=GWEN_NetLayer_BeginInPacket(nl);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not start to read (%d)", rv);
    return rv;
  }

  for (;;) {
    if (timeout!=GWEN_NET2_TIMEOUT_NONE &&
	timeout!=GWEN_NET2_TIMEOUT_FOREVER) {
      tLeft=timeout-(difftime(time(0), startt));
      if (tLeft<1)
	tLeft=1;
    }
    else
      tLeft=timeout;
    rv=GWEN_NetLayer_CheckInPacket(nl);
    DBG_DEBUG(GWEN_LOGDOMAIN, "Check-Result: %d", rv);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error checking packet (%d)", rv);
      return rv;
    }
    else if (rv==1) {
      if (!bodyStarted) {
        if (nld->inStatusCode>=100 && nld->inStatusCode<200)
          ignoreBody=1;
        else
          ignoreBody=0;
        bodyStarted=1;
      }
      DBG_DEBUG(GWEN_LOGDOMAIN, "Reading");
      bsize=sizeof(buffer);
      rv=GWEN_NetLayer_Read_Wait(nl, buffer, &bsize, tLeft);
      if (rv<0) {
	DBG_ERROR(GWEN_LOGDOMAIN, "ERROR: Could not read (%d)", rv);
        return rv;
      }
      else if (rv==1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "ERROR: Could not read due to a timeout");
        return GWEN_ERROR_TIMEOUT;
      }
      else {
        if (bsize==0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "INFO: EOF met");
	  break;
	}
        else {
          if (!ignoreBody && bsize) {
            const char *p;
            int wLeft;

            wLeft=bsize;
            p=buffer;
            while(wLeft) {
              unsigned int wsize;

              wsize=wLeft;
              err=GWEN_BufferedIO_WriteRaw(bio, p, &wsize);
              if (!GWEN_Error_IsOk(err)) {
                DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
                return GWEN_Error_GetSimpleCode(err);
              }
              p+=wsize;
              wLeft-=wsize;
	    } /* while */
	  }
	}
      }
    }
    else
      break;
  } /* for */

  err=GWEN_BufferedIO_Flush(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_Error_GetSimpleCode(err);
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Packet received");
  if (ignoreBody)
    return 0;
  return nld->inStatusCode;
}




int GWEN_NetLayerHttp_Request(GWEN_NETLAYER *nl,
                              const char *command,
                              const GWEN_URL *url,
                              GWEN_DB_NODE *dbHeader,
                              const char *pBody,
                              int lBody,
                              GWEN_BUFFEREDIO *bio) {
  GWEN_NL_HTTP *nld;
  GWEN_DB_NODE *dbT;
  int rv;
  int doClose=0;
  const char *s;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HTTP, nl);
  assert(nld);

  /* prepare request */
  dbT=GWEN_NetLayerHttp_GetOutHeader(nl);
  assert(dbT);
  GWEN_DB_ClearGroup(dbT, 0);
  if (dbHeader)
    GWEN_DB_AddGroupChildren(dbT, dbHeader);
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "host",
                       GWEN_Url_GetServer(url));

  GWEN_NetLayerHttp_SetOutCommand(nl, command, url);

  /* send request */
  rv=GWEN_NetLayer_SendPacket(nl, pBody, lBody, 30);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* receive response to BIO */
  rv=GWEN_NetLayerHttp__RecvPacketBio(nl, bio, 30);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (rv==0) {
    /* intermediate message, body ignored, read real packet */
    DBG_INFO(GWEN_LOGDOMAIN,
             "Ignoring intermediate message (%d: %s)",
             nld->inStatusCode, nld->inStatusText);
    rv=GWEN_NetLayerHttp__RecvPacketBio(nl, bio, 30);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    if (rv==0) {
      /* one intermedate message is enough */
      DBG_ERROR(GWEN_LOGDOMAIN, "Received too many intermediate messages");
      return GWEN_ERROR_BAD_DATA;
    }
  }

  if (nld->pversion==GWEN_NetLayerHttpVersion_1_0) {
    doClose=1;
    DBG_DEBUG(GWEN_LOGDOMAIN, "HTTP/1.0: Default is to close connection");
  }
  else {
    doClose=0;
    DBG_DEBUG(GWEN_LOGDOMAIN,
	     "HTTP/1.1: Default is to keep connection alive");
  }

  s=GWEN_DB_GetCharValue(nld->dbInHeader, "Connection", 0, 0);
  if (s) {
    if (strcasecmp(s, "close")!=0) {
      DBG_DEBUG(GWEN_LOGDOMAIN,
	       "Header indicates not to close the connection");
      doClose=0;
    }
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN,
	       "Header indicates to close connection");
      doClose=1;
    }
  }

  if (doClose) {
    int disrv;

    DBG_INFO(GWEN_LOGDOMAIN, "Closing connection (indicated by header)");
    disrv=GWEN_NetLayer_Disconnect_Wait(nl, 2);
    if (disrv) {
      /* just an info, this isn't treated as error, since we want to close
       * the connection anyway */
      DBG_INFO(GWEN_LOGDOMAIN, "Could not disconnect (%d)", disrv);
    }
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Request completed");
  return rv;
}








