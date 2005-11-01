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


#include "nl_hbci_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/nl_http.h>

#include <ctype.h>


GWEN_INHERIT(GWEN_NETLAYER, GWEN_NL_HBCI)


GWEN_NETLAYER *GWEN_NetLayerHbci_new(GWEN_NETLAYER *baseLayer) {
  GWEN_NETLAYER *nl;
  GWEN_NL_HBCI *nld;

  assert(baseLayer);
  nl=GWEN_NetLayer_new(GWEN_NL_HBCI_NAME);
  GWEN_NEW_OBJECT(GWEN_NL_HBCI, nld);
  GWEN_INHERIT_SETDATA(GWEN_NETLAYER, GWEN_NL_HBCI, nl, nld,
                       GWEN_NetLayerHbci_FreeData);

  nld->outBuffer=GWEN_Buffer_new(0, 512, 0, 1);
  nld->inBuffer=GWEN_Buffer_new(0, 512, 0, 1);

  GWEN_NetLayer_SetBaseLayer(nl, baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  GWEN_NetLayer_SetParentLayer(baseLayer, nl);

  GWEN_NetLayer_SetLocalAddr(nl, GWEN_NetLayer_GetLocalAddr(baseLayer));
  GWEN_NetLayer_SetPeerAddr(nl, GWEN_NetLayer_GetPeerAddr(baseLayer));

  /* this protocol is packet based */
  GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_PKG_BASED);

  GWEN_NetLayer_SetWorkFn(nl, GWEN_NetLayerHbci_Work);
  GWEN_NetLayer_SetReadFn(nl, GWEN_NetLayerHbci_Read);
  GWEN_NetLayer_SetWriteFn(nl, GWEN_NetLayerHbci_Write);
  GWEN_NetLayer_SetConnectFn(nl, GWEN_NetLayerHbci_Connect);
  GWEN_NetLayer_SetDisconnectFn(nl, GWEN_NetLayerHbci_Disconnect);
  GWEN_NetLayer_SetListenFn(nl, GWEN_NetLayerHbci_Listen);
  GWEN_NetLayer_SetAddSocketsFn(nl, GWEN_NetLayerHbci_AddSockets);
  GWEN_NetLayer_SetBeginOutPacketFn(nl, GWEN_NetLayerHbci_BeginOutPacket);
  GWEN_NetLayer_SetEndOutPacketFn(nl, GWEN_NetLayerHbci_EndOutPacket);
  GWEN_NetLayer_SetBeginInPacketFn(nl, GWEN_NetLayerHbci_BeginInPacket);
  GWEN_NetLayer_SetCheckInPacketFn(nl, GWEN_NetLayerHbci_CheckInPacket);

  return nl;
}



void GWEN_NetLayerHbci_FreeData(void *bp, void *p) {
}



int GWEN_NetLayerHbci_Connect(GWEN_NETLAYER *nl) {
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



int GWEN_NetLayerHbci_Disconnect(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;
  int rv;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);
  rv=GWEN_NetLayer_Disconnect(baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  return rv;
}



int GWEN_NetLayerHbci_Listen(GWEN_NETLAYER *nl) {
  GWEN_NETLAYER *baseLayer;
  int rv;

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);
  rv=GWEN_NetLayer_Listen(baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  GWEN_NetLayer_AddFlags(nl, GWEN_NETLAYER_FLAGS_PASSIVE);
  return rv;
}



int GWEN_NetLayerHbci_AddSockets(GWEN_NETLAYER *nl,
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



GWEN_NETLAYER_RESULT GWEN_NetLayerHbci__WriteWork(GWEN_NETLAYER *nl) {
  GWEN_NL_HBCI *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;
  int bsize;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HBCI, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  bsize=GWEN_Buffer_GetBytesLeft(nld->outBuffer);
  if (bsize==0)
    return GWEN_NetLayerResult_Idle;

  if (bsize>1024)
    bsize=1024;

  rv=GWEN_NetLayer_Write(baseLayer,
                         GWEN_Buffer_GetPosPointer(nld->outBuffer),
                         &bsize);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return GWEN_NetLayerResult_Error;
  }
  else if (rv==1)
    return GWEN_NetLayerResult_WouldBlock;

  if (bsize==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Nothing written");
    return GWEN_NetLayerResult_Error;
  }
  GWEN_Buffer_IncrementPos(nld->outBuffer, bsize);
  return GWEN_NetLayerResult_Changed;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_NetLayerHbci__ReadWork(GWEN_NETLAYER *nl) {
  GWEN_NL_HBCI *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HBCI, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  if (nld->inMode==GWEN_NetLayerHbciInMode_HasMsg ||
      nld->inMode==GWEN_NetLayerHbciInMode_Idle ||
      nld->inMode==GWEN_NetLayerHbciInMode_Done)
    return GWEN_NetLayerResult_Idle;
  else if (nld->inMode==GWEN_NetLayerHbciInMode_Aborted) {
    DBG_INFO(GWEN_LOGDOMAIN, "Operation previously aborted");
    return GWEN_NetLayerResult_Error;
  }
  else if (nld->inMode==GWEN_NetLayerHbciInMode_GetType) {
    int bsize;
    GWEN_NETLAYER *nlHttp;

    nlHttp=GWEN_NetLayer_FindBaseLayer(nl, GWEN_NL_HTTP_NAME);
    if (nlHttp) {
      int code;

      code=GWEN_NetLayerHttp_GetInStatusCode(nlHttp);
      if (code) {
        GWEN_BUFFER *lbuf;
        char sbuf[32];
        const char *text;

        nld->inHttpStatusCode=code;
        lbuf=GWEN_Buffer_new(0, 64, 0, 1);
        snprintf(sbuf, sizeof(sbuf)-1, "HTTP-Status: %d", code);
        sbuf[sizeof(sbuf)-1]=0;
        GWEN_Buffer_AppendString(lbuf, sbuf);
        text=GWEN_NetLayerHttp_GetInStatusText(nlHttp);
        if (text) {
          GWEN_Buffer_AppendString(lbuf, " (");
          GWEN_Buffer_AppendString(lbuf, text);
          GWEN_Buffer_AppendString(lbuf, ")");
        }
        DBG_DEBUG(GWEN_LOGDOMAIN, "Status: %d (%s)",
                  code, GWEN_NetLayerHttp_GetInStatusText(nlHttp));
        if (code<200 || code>299) {
          if (code!=100) {
            DBG_ERROR(GWEN_LOGDOMAIN, "Got an error response");
            GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                                  GWEN_Buffer_GetStart(lbuf));
          }
          else {
            GWEN_WaitCallback_Log(GWEN_LoggerLevelInfo,
                                  GWEN_Buffer_GetStart(lbuf));
          }
          GWEN_Buffer_free(lbuf);
          nld->inMode=GWEN_NetLayerHbciInMode_ReadError;
          return GWEN_NetLayerResult_Changed;
        }
        else {
          GWEN_WaitCallback_Log(GWEN_LoggerLevelInfo,
                                GWEN_Buffer_GetStart(lbuf));
          GWEN_Buffer_free(lbuf);
        }
      }
    }

    bsize=GWEN_Buffer_GetUsedBytes(nld->inBuffer);
    if (bsize<GWEN_NL_HBCI_GETTYPE_SIZE) {
      char *p;

      GWEN_Buffer_AllocRoom(nld->inBuffer, GWEN_NL_HBCI_GETTYPE_SIZE);
      p=GWEN_Buffer_GetPosPointer(nld->inBuffer);
      bsize=GWEN_NL_HBCI_GETTYPE_SIZE-bsize;
      rv=GWEN_NetLayer_Read(baseLayer, p, &bsize);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        nld->inMode=GWEN_NetLayerHbciInMode_Aborted;
        return GWEN_NetLayerResult_Error;
      }
      else if (rv==1)
        return GWEN_NetLayerResult_WouldBlock;
      else {
        if (bsize==0) { /* EOF, premature in this case */
          nld->inMode=GWEN_NetLayerHbciInMode_Aborted;
          return GWEN_NetLayerResult_Error;
        }
        GWEN_Buffer_IncrementPos(nld->inBuffer, bsize);
        GWEN_Buffer_AdjustUsedBytes(nld->inBuffer);
        return GWEN_NetLayerResult_Changed;
      }
    }
    if (strncasecmp(GWEN_Buffer_GetStart(nld->inBuffer),
                    "HNHBK:", 6)==0) {
      nld->inIsBase64=0; /* not in base64 */
      nld->inMode=GWEN_NetLayerHbciInMode_ReadSize;
      return GWEN_NetLayerResult_Changed;
    }
    else {
      if (GWEN_NetLayer_GetFlags(nl) & GWEN_NL_HBCI_FLAGS_BASE64) {
        DBG_DEBUG(GWEN_LOGDOMAIN,
                  "Assuming BASE64 encoded message, reading until EOF");
        nld->inIsBase64=1;
        nld->inMode=GWEN_NetLayerHbciInMode_ReadMsg;
        return GWEN_NetLayerResult_Changed;
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad message");
        GWEN_Buffer_Dump(nld->inBuffer, stderr, 2);
        nld->inMode=GWEN_NetLayerHbciInMode_Aborted;
        return GWEN_NetLayerResult_Error;
      }
    }
  }
  else if (nld->inMode==GWEN_NetLayerHbciInMode_ReadSize) {
    int bsize;
    char *p;

    bsize=GWEN_Buffer_GetUsedBytes(nld->inBuffer);
    if (bsize<GWEN_NL_HBCI_GETSIZE_SIZE) {
      GWEN_Buffer_AllocRoom(nld->inBuffer, GWEN_NL_HBCI_GETSIZE_SIZE);
      p=GWEN_Buffer_GetPosPointer(nld->inBuffer);
      bsize=GWEN_NL_HBCI_GETSIZE_SIZE-bsize;
      rv=GWEN_NetLayer_Read(baseLayer, p, &bsize);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        nld->inMode=GWEN_NetLayerHbciInMode_Aborted;
        return GWEN_NetLayerResult_Error;
      }
      else if (rv==1)
        return GWEN_NetLayerResult_WouldBlock;
      else {
        if (bsize==0) { /* EOF, premature in this case */
          nld->inMode=GWEN_NetLayerHbciInMode_Aborted;
          return GWEN_NetLayerResult_Error;
        }
        GWEN_Buffer_IncrementPos(nld->inBuffer, bsize);
        GWEN_Buffer_AdjustUsedBytes(nld->inBuffer);
        return GWEN_NetLayerResult_Changed;
      }
    }

    /* find first occurrence of "+", the size follows behind this */
    p=strchr(GWEN_Buffer_GetStart(nld->inBuffer), '+');
    if (p==0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad message");
      GWEN_Buffer_Dump(nld->inBuffer, stderr, 2);
      nld->inMode=GWEN_NetLayerHbciInMode_Aborted;
      return GWEN_NetLayerResult_Error;
    }

    /* read size */
    p++;
    bsize=0;
    while(*p && isdigit(*p)) {
      bsize*=10;
      bsize+=*p-'0';
      p++;
    }
    if (!*p) {
      /* size not completed within first 30 bytes */
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad message");
      GWEN_Buffer_Dump(nld->inBuffer, stderr, 2);
      nld->inMode=GWEN_NetLayerHbciInMode_Aborted;
      return GWEN_NetLayerResult_Error;
    }
    GWEN_NetLayer_SetInBodySize(nl, bsize);
    GWEN_Buffer_Rewind(nld->inBuffer);
    nld->inMode=GWEN_NetLayerHbciInMode_HasMsg;
    return GWEN_NetLayerResult_Changed;
  }
  else if (nld->inMode==GWEN_NetLayerHbciInMode_ReadMsg) {
    char *p;
    int bsize;

    GWEN_Buffer_AllocRoom(nld->inBuffer, GWEN_NL_HBCI_READMSG_CHUNKSIZE);
    p=GWEN_Buffer_GetPosPointer(nld->inBuffer);
    bsize=GWEN_NL_HBCI_READMSG_CHUNKSIZE;
    rv=GWEN_NetLayer_Read(baseLayer, p, &bsize);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      nld->inMode=GWEN_NetLayerHbciInMode_Aborted;
      return GWEN_NetLayerResult_Error;
    }
    if (rv==1)
      return GWEN_NetLayerResult_WouldBlock;
    if (bsize==0) {
      GWEN_BUFFER *tbuf;
      const unsigned char *up;

      /* EOF met, finished reading BASE64 stream */
      GWEN_Buffer_AppendByte(nld->inBuffer, 0);
      tbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(nld->inBuffer), 0, 1);
      up=(const unsigned char*)GWEN_Buffer_GetStart(nld->inBuffer);
      rv=GWEN_Base64_Decode(up, 0, tbuf);
      if (rv) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad message (%d)", rv);
        GWEN_Buffer_Dump(nld->inBuffer, stderr, 2);
        nld->inMode=GWEN_NetLayerHbciInMode_Aborted;
        GWEN_Buffer_free(tbuf);
        return GWEN_NetLayerResult_Error;
      }
      GWEN_NetLayer_SetInBodySize(nl, GWEN_Buffer_GetUsedBytes(tbuf));

      /* exchange base64 encoded inBuffer with decoded data */
      GWEN_Buffer_free(nld->inBuffer);
      nld->inBuffer=tbuf;
      GWEN_Buffer_Rewind(nld->inBuffer);
      nld->inMode=GWEN_NetLayerHbciInMode_HasMsg;
      return GWEN_NetLayerResult_Changed;
    }
    else {
      GWEN_Buffer_IncrementPos(nld->inBuffer, bsize);
      GWEN_Buffer_AdjustUsedBytes(nld->inBuffer);
      return GWEN_NetLayerResult_Changed;
    }
  }
  else if (nld->inMode==GWEN_NetLayerHbciInMode_ReadError) {
    char *p;
    int bsize;

    GWEN_Buffer_AllocRoom(nld->inBuffer, GWEN_NL_HBCI_READMSG_CHUNKSIZE);
    p=GWEN_Buffer_GetPosPointer(nld->inBuffer);
    bsize=GWEN_NL_HBCI_READMSG_CHUNKSIZE;
    rv=GWEN_NetLayer_Read(baseLayer, p, &bsize);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      nld->inMode=GWEN_NetLayerHbciInMode_Aborted;
      return GWEN_NetLayerResult_Error;
    }
    if (rv==1)
      return GWEN_NetLayerResult_WouldBlock;
    if (bsize==0) {
      if (nld->inHttpStatusCode==100) {
        /* last message was just a coninuation message, skip it and
         * start the next packet */
        DBG_DEBUG(GWEN_LOGDOMAIN, "Starting to read next message");
        rv=GWEN_NetLayer_BeginInPacket(baseLayer);
        if (rv<0 && rv!=GWEN_ERROR_UNSUPPORTED)
          return rv;
        assert(rv==0);
        GWEN_Buffer_Reset(nld->inBuffer);
        nld->inMode=GWEN_NetLayerHbciInMode_GetType;
        nld->inBodyRead=0;
        return GWEN_NetLayerResult_Changed;
      }
      else {
        nld->inMode=GWEN_NetLayerHbciInMode_Aborted;
        return GWEN_NetLayerResult_Error;
      }
    }
    else {
      GWEN_Buffer_IncrementPos(nld->inBuffer, bsize);
      GWEN_Buffer_AdjustUsedBytes(nld->inBuffer);
      return GWEN_NetLayerResult_Changed;
    }
    return GWEN_NetLayerResult_Changed;
  }
  abort();
}




GWEN_NETLAYER_RESULT GWEN_NetLayerHbci_Work(GWEN_NETLAYER *nl) {
  GWEN_NL_HBCI *nld;
  GWEN_NETLAYER_RESULT bres=GWEN_NetLayerResult_Idle;
  GWEN_NETLAYER_RESULT res;
  GWEN_NETLAYER *baseLayer;
  GWEN_NETLAYER_STATUS st;

#define GWEN_NL_HBCI_MERGE_RESULTS(r1, r2)    \
  if (r1==GWEN_NetLayerResult_Idle)           \
    r1=r2;                                    \
  else if (r1!=GWEN_NetLayerResult_Changed) { \
    if (r2==GWEN_NetLayerResult_Changed)      \
      r1=GWEN_NetLayerResult_Changed;          \
  }

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HBCI, nl);
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
      GWEN_NETLAYER *newNlHbci;

      newNlHbci=GWEN_NetLayerHbci_new(newNl);
      GWEN_NetLayer_AddFlags(newNlHbci, GWEN_NETLAYER_FLAGS_PASSIVE);
      GWEN_NetLayer_free(newNl);
      GWEN_NetLayer_AddIncomingLayer(nl, newNlHbci);
      bres=GWEN_NetLayerResult_Changed;
    }
  }

  res=GWEN_NetLayer_Work(baseLayer);
  GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  DBG_VERBOUS(GWEN_LOGDOMAIN,
              "Result of BaseLayer work: %s",
              GWEN_NetLayerResult_toString(res));
  if (res==GWEN_NetLayerResult_Error) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "here");
    return res;
  }
  GWEN_NL_HBCI_MERGE_RESULTS(bres, res);

  res=GWEN_NetLayerHbci__WriteWork(nl);
  DBG_VERBOUS(GWEN_LOGDOMAIN,
              "Result of WriteWork: %s",
              GWEN_NetLayerResult_toString(res));
  if (res==GWEN_NetLayerResult_Error) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return res;
  }
  GWEN_NL_HBCI_MERGE_RESULTS(bres, res);

  res=GWEN_NetLayerHbci__ReadWork(nl);
  DBG_VERBOUS(GWEN_LOGDOMAIN,
              "Result of ReadWork: %s",
              GWEN_NetLayerResult_toString(res));
  if (res==GWEN_NetLayerResult_Error) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return res;
  }
  GWEN_NL_HBCI_MERGE_RESULTS(bres, res);

#undef GWEN_NL_HBCI_MERGE_RESULTS

  /* return cumulated result */
  return bres;
}



int GWEN_NetLayerHbci_BeginOutPacket(GWEN_NETLAYER *nl, int totalSize) {
  GWEN_NL_HBCI *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HBCI, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  GWEN_Buffer_Reset(nld->outBuffer);
  nld->outBodyWritten=0;

  if (GWEN_NetLayer_GetFlags(nl) & GWEN_NL_HBCI_FLAGS_BASE64) {
    nld->outMode=GWEN_NetLayerHbciOutMode_WaitForEnd;
  }
  else {
    rv=GWEN_NetLayer_BeginOutPacket(baseLayer, totalSize);
    if (rv && rv!=GWEN_ERROR_UNSUPPORTED)
      return rv;
    nld->outMode=GWEN_NetLayerHbciOutMode_WriteMsg;
  }

  return 0;
}



int GWEN_NetLayerHbci_EndOutPacket(GWEN_NETLAYER *nl) {
  GWEN_NL_HBCI *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;
  int handled=0;
  int done=0;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HBCI, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  if (nld->outMode==GWEN_NetLayerHbciOutMode_WaitForEnd) {
    GWEN_BUFFER *tbuf;
    const unsigned char *up;

    handled=1;
    tbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(nld->outBuffer), 0, 1);
    up=(const unsigned char*)GWEN_Buffer_GetStart(nld->outBuffer);
    rv=GWEN_Base64_Encode(up, GWEN_Buffer_GetUsedBytes(nld->outBuffer),
                          tbuf, 0);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error converting to BASE64");
      nld->outMode=GWEN_NetLayerHbciOutMode_Aborted;
      GWEN_Buffer_free(tbuf);
      return -1;
    }
    GWEN_Buffer_AppendString(tbuf, "\r\n");
    GWEN_Buffer_free(nld->outBuffer);
    nld->outBuffer=tbuf;
    GWEN_Buffer_Rewind(nld->outBuffer);
    /* the outbody size is now known, set it */
    GWEN_NetLayer_SetOutBodySize(baseLayer,
                                 GWEN_Buffer_GetUsedBytes(nld->outBuffer));
    rv=GWEN_NetLayer_BeginOutPacket(baseLayer,
                                    GWEN_Buffer_GetUsedBytes(nld->outBuffer));
    if (rv<0 && rv!=GWEN_ERROR_UNSUPPORTED) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not begin sending packet");
      nld->outMode=GWEN_NetLayerHbciOutMode_Aborted;
      return rv;
    }
    assert(rv==0);

    nld->outMode=GWEN_NetLayerHbciOutMode_WriteMsg;
    done++;
  }

  if (nld->outMode==GWEN_NetLayerHbciOutMode_WriteMsg) {
    handled=1;
    if (GWEN_Buffer_GetBytesLeft(nld->outBuffer)==0) {
      rv=GWEN_NetLayer_EndOutPacket(baseLayer);
      if (rv<0 && rv!=GWEN_ERROR_UNSUPPORTED)
        return rv;
      if (rv!=1)
        done++;
    }
  }

  if (!handled) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected call to EndOutPacket");
    return -1;
  }

  if (done==0)
    return 1;

  return 0;
}



int GWEN_NetLayerHbci_BeginInPacket(GWEN_NETLAYER *nl) {
  GWEN_NL_HBCI *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HBCI, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  rv=GWEN_NetLayer_BeginInPacket(baseLayer);
  if (rv && rv!=GWEN_ERROR_UNSUPPORTED)
    return rv;

  GWEN_Buffer_Reset(nld->inBuffer);
  nld->inMode=GWEN_NetLayerHbciInMode_GetType;
  nld->inBodyRead=0;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Starting to read incoming packet");
  return 0;
}



int GWEN_NetLayerHbci_CheckInPacket(GWEN_NETLAYER *nl) {
  GWEN_NL_HBCI *nld;
  int inBodySize;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HBCI, nl);
  assert(nld);

  inBodySize=GWEN_NetLayer_GetInBodySize(nl);

  switch(nld->inMode) {
  case GWEN_NetLayerHbciInMode_Idle:
    DBG_ERROR(GWEN_LOGDOMAIN, "Not in read mode");
    return -1;

  case GWEN_NetLayerHbciInMode_GetType:
  case GWEN_NetLayerHbciInMode_ReadSize:
  case GWEN_NetLayerHbciInMode_ReadMsg:
  case GWEN_NetLayerHbciInMode_ReadError:
    return 1;

  case GWEN_NetLayerHbciInMode_HasMsg:
    if (inBodySize!=-1) {
      if (nld->inBodyRead>=inBodySize) {
        nld->inMode=GWEN_NetLayerHbciInMode_Done;
        DBG_INFO(GWEN_LOGDOMAIN, "Body complete.");
        return 0;
      }
    }
    return 1;

  case GWEN_NetLayerHbciInMode_Done:
    DBG_DEBUG(GWEN_LOGDOMAIN, "Body completely read.");
    return 0;

  case GWEN_NetLayerHbciInMode_Aborted:
    DBG_ERROR(GWEN_LOGDOMAIN, "Aborted");
    return -1;

  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unhandled read mode (%d)", nld->inMode);
    return -1;
  }
}



int GWEN_NetLayerHbci_Write(GWEN_NETLAYER *nl, const char *buffer,int *bsize){
  GWEN_NL_HBCI *nld;
  GWEN_NETLAYER *baseLayer;
  int outBodySize;
  int rv;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HBCI, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  outBodySize=GWEN_NetLayer_GetOutBodySize(nl);

  if (outBodySize!=-1 &&
      nld->outBodyWritten+*bsize>outBodySize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Too many bytes for body (%d>=%d)",
              nld->outBodyWritten+*bsize,
              outBodySize);
    return GWEN_ERROR_EOF;
  }

  if (nld->outMode==GWEN_NetLayerHbciOutMode_WaitForEnd) {
    GWEN_Buffer_AppendBytes(nld->outBuffer, buffer, *bsize);
    rv=0;
  }
  else {
    rv=GWEN_NetLayer_Write(baseLayer, buffer, bsize);
    GWEN_NetLayer_SetStatus(nl, GWEN_NetLayer_GetStatus(baseLayer));
  }

  if (rv==0)
    nld->outBodyWritten+=*bsize;
  return rv;
}



int GWEN_NetLayerHbci_Read(GWEN_NETLAYER *nl, char *buffer, int *bsize){
  GWEN_NL_HBCI *nld;
  GWEN_NETLAYER *baseLayer;
  int rv;
  int inBodySize;
  int lsize;

  assert(nl);
  nld=GWEN_INHERIT_GETDATA(GWEN_NETLAYER, GWEN_NL_HBCI, nl);
  assert(nld);

  baseLayer=GWEN_NetLayer_GetBaseLayer(nl);
  assert(baseLayer);

  if (nld->inMode==GWEN_NetLayerHbciInMode_Idle) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not in reading mode");
    return GWEN_ERROR_INVALID;
  }

  if (nld->inMode==GWEN_NetLayerHbciInMode_Done) {
    DBG_INFO(GWEN_LOGDOMAIN, "Message fully read.");
    *bsize=0; /* flag EOF */ /* TODO: return error */
    return 0;
  }

  if (nld->inMode!=GWEN_NetLayerHbciInMode_HasMsg) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Still in header read mode");
    return 1;
  }

  inBodySize=GWEN_NetLayer_GetInBodySize(nl);
  lsize=GWEN_Buffer_GetBytesLeft(nld->inBuffer);
  if (lsize) {
    /* there is data in the input buffer, return that */
    if (lsize>*bsize)
      lsize=*bsize;
    memmove(buffer, GWEN_Buffer_GetPosPointer(nld->inBuffer), lsize);
    GWEN_Buffer_IncrementPos(nld->inBuffer, lsize);
    *bsize=lsize;
    rv=0;
  }
  else {
    if (inBodySize!=-1) {
      assert(inBodySize>=nld->inBodyRead);
      lsize=inBodySize-nld->inBodyRead;
      if (lsize>*bsize)
        lsize=*bsize;
    }
    else
      lsize=*bsize;
    rv=GWEN_NetLayer_Read(baseLayer, buffer, &lsize);
    if (rv==0)
      *bsize=lsize;
  }

  /* handle sizes */
  if (rv==0) {
    nld->inBodyRead+=*bsize;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Read %d bytes (%d of %d)",
              *bsize, nld->inBodyRead, inBodySize);
    if (inBodySize!=-1) {
      if (nld->inBodyRead>=inBodySize)
        nld->inMode=GWEN_NetLayerHbciInMode_Done;
    }
  }

  return rv;
}



