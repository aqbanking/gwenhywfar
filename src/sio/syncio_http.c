/***************************************************************************
 begin       : Wed Apr 28 2010
 copyright   : (C) 2010 by Martin Preuss
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



#include "syncio_http_p.h"
#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>



GWEN_INHERIT(GWEN_SYNCIO, GWEN_SYNCIO_HTTP)



GWEN_SYNCIO *GWEN_SyncIo_Http_new(GWEN_SYNCIO *baseIo) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_HTTP *xio;

  sio=GWEN_SyncIo_new(GWEN_SYNCIO_HTTP_TYPE, baseIo);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_HTTP, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio, xio, GWEN_SyncIo_Http_FreeData);

  GWEN_SyncIo_SetConnectFn(sio, GWEN_SyncIo_Http_Connect);
  GWEN_SyncIo_SetDisconnectFn(sio, GWEN_SyncIo_Http_Disconnect);
  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_Http_Read);
  GWEN_SyncIo_SetWriteFn(sio, GWEN_SyncIo_Http_Write);

  xio->dbCommandIn=GWEN_DB_Group_new("command");
  xio->dbStatusIn=GWEN_DB_Group_new("status");
  xio->dbHeaderIn=GWEN_DB_Group_new("header");

  xio->dbCommandOut=GWEN_DB_Group_new("command");
  xio->dbStatusOut=GWEN_DB_Group_new("status");
  xio->dbHeaderOut=GWEN_DB_Group_new("header");


  return sio;
}



void GWENHYWFAR_CB GWEN_SyncIo_Http_FreeData(void *bp, void *p) {
  GWEN_SYNCIO_HTTP *xio;

  xio=(GWEN_SYNCIO_HTTP*) p;

  GWEN_DB_Group_free(xio->dbCommandOut);
  GWEN_DB_Group_free(xio->dbStatusOut);
  GWEN_DB_Group_free(xio->dbHeaderOut);

  GWEN_DB_Group_free(xio->dbCommandIn);
  GWEN_DB_Group_free(xio->dbStatusIn);
  GWEN_DB_Group_free(xio->dbHeaderIn);

  GWEN_FREE_OBJECT(xio);
}



int GWENHYWFAR_CB GWEN_SyncIo_Http_Connect(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  if (GWEN_SyncIo_GetStatus(sio)==GWEN_SyncIo_Status_Connected) {
    DBG_INFO(GWEN_LOGDOMAIN, "Already connected");
    return 0;
  }

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  rv=GWEN_SyncIo_Connect(baseIo);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Connected);
  GWEN_SyncIo_Http_SetReadIdle(sio);

  return 0;
}



int GWENHYWFAR_CB GWEN_SyncIo_Http_Disconnect(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  if (GWEN_SyncIo_GetStatus(sio)!=GWEN_SyncIo_Status_Connected) {
    DBG_INFO(GWEN_LOGDOMAIN, "Not connected");
    return GWEN_ERROR_NOT_CONNECTED;
  }

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  rv=GWEN_SyncIo_Disconnect(baseIo);
  GWEN_SyncIo_SetStatus(sio, GWEN_SyncIo_Status_Disconnected);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



void GWEN_SyncIo_Http_SetReadIdle(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  xio->readMode=GWEN_SyncIo_Http_Mode_Idle;
}



int GWENHYWFAR_CB GWEN_SyncIo_Http_Read(GWEN_SYNCIO *sio,
					uint8_t *buffer,
					uint32_t size) {
  GWEN_SYNCIO_HTTP *xio;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  if (GWEN_SyncIo_GetStatus(sio)!=GWEN_SyncIo_Status_Connected) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not connected");
    return GWEN_ERROR_NOT_CONNECTED;
  }

  if (xio->readMode==GWEN_SyncIo_Http_Mode_Idle) {
    const char *s;

    /* reset status and headers */
    GWEN_DB_ClearGroup(xio->dbCommandIn, NULL);
    GWEN_DB_ClearGroup(xio->dbStatusIn, NULL);
    GWEN_DB_ClearGroup(xio->dbHeaderIn, NULL);

    if (GWEN_SyncIo_GetFlags(sio) & GWEN_SYNCIO_FLAGS_PASSIVE) {
      /* read command */
      rv=GWEN_SyncIo_Http_ReadCommand(sio);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	xio->readMode=GWEN_SyncIo_Http_Mode_Error;
	return rv;
      }

      /* possibly read header */
      s=GWEN_DB_GetCharValue(xio->dbCommandIn, "protocol", 0, "HTTP/1.0");
      if (!(s && strcasecmp(s, "HTTP/0.9")==0)) {
	rv=GWEN_SyncIo_Http_ReadHeader(sio);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  xio->readMode=GWEN_SyncIo_Http_Mode_Error;
	  return rv;
	}
      }
    }
    else {
      /* read status */
      rv=GWEN_SyncIo_Http_ReadStatus(sio);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	xio->readMode=GWEN_SyncIo_Http_Mode_Error;
	return rv;
      }

      /* possibly read header */
      s=GWEN_DB_GetCharValue(xio->dbStatusIn, "protocol", 0, "HTTP/1.0");
      if (!(s && strcasecmp(s, "HTTP/0.9")==0)) {
	rv=GWEN_SyncIo_Http_ReadHeader(sio);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  xio->readMode=GWEN_SyncIo_Http_Mode_Error;
	  return rv;
	}
      }
    }

  }

  if (xio->readMode==GWEN_SyncIo_Http_Mode_ChunkSize) {
    rv=GWEN_SyncIo_Http_ReadChunkSize(sio);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      xio->readMode=GWEN_SyncIo_Http_Mode_Error;
      return rv;
    }
    if (xio->currentReadChunkSize==0) {
      /* chunksize is 0, body ended */
      GWEN_SyncIo_Http_SetReadIdle(sio);
      return 0;
    }
    else if (xio->currentReadChunkSize==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Undetermined chunksize in chunked mode? Aborting.");
      xio->readMode=GWEN_SyncIo_Http_Mode_Error;
      return GWEN_ERROR_BAD_DATA;
    }

    /* chunksize known, next will be to read that chunk */
    xio->readMode=GWEN_SyncIo_Http_Mode_Chunk;
  }

  if (xio->readMode==GWEN_SyncIo_Http_Mode_Chunk) {
    /* read chunk */
    rv=GWEN_SyncIo_Http_ReadChunk(sio, buffer, size);
    if (rv<0) {
      xio->readMode=GWEN_SyncIo_Http_Mode_Error;
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    return rv;
  }

  if (xio->readMode==GWEN_SyncIo_Http_Mode_Body) {
    /* read chunk */
    rv=GWEN_SyncIo_Http_ReadBody(sio, buffer, size);
    if (rv<0) {
      xio->readMode=GWEN_SyncIo_Http_Mode_Error;
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    return rv;
  }

  if (xio->readMode==GWEN_SyncIo_Http_Mode_Error) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Previous read error");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int GWENHYWFAR_CB GWEN_SyncIo_Http_Write(GWEN_SYNCIO *sio,
					 const uint8_t *buffer,
					 uint32_t size) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  if (GWEN_SyncIo_GetStatus(sio)!=GWEN_SyncIo_Status_Connected) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Not connected");
    return GWEN_ERROR_NOT_CONNECTED;
  }

  if (xio->writeMode==GWEN_SyncIo_Http_Mode_Idle) {
    const char *s;

    if (GWEN_SyncIo_GetFlags(sio) & GWEN_SYNCIO_FLAGS_PASSIVE)
      /* write status */
      rv=GWEN_SyncIo_Http_WriteStatus(sio);
    else
      /* write command */
      rv=GWEN_SyncIo_Http_WriteCommand(sio);
    if (rv<0) {
      xio->writeMode=GWEN_SyncIo_Http_Mode_Error;
      return rv;
    }

    /* possibly write header */
    s=GWEN_DB_GetCharValue(xio->dbCommandOut, "protocol", 0, "HTTP/1.0");
    if (!(s && strcasecmp(s, "HTTP/0.9")==0)) {
      rv=GWEN_SyncIo_Http_WriteHeader(sio);
      if (rv<0) {
	xio->writeMode=GWEN_SyncIo_Http_Mode_Error;
	return rv;
      }
    }
  }

  if (xio->writeMode==GWEN_SyncIo_Http_Mode_ChunkSize) {
    rv=GWEN_SyncIo_Http_WriteChunkSize(sio, size);
    if (rv<0) {
      xio->writeMode=GWEN_SyncIo_Http_Mode_Error;
      return rv;
    }
    if (size==0) {
      /* chunksize is 0, body ended */
      GWEN_SyncIo_Http_SetWriteIdle(sio);
      return 0;
    }

    /* chunksize known, next will be to write that chunk */
    xio->writeMode=GWEN_SyncIo_Http_Mode_Chunk;
  }

  if (xio->writeMode==GWEN_SyncIo_Http_Mode_Chunk) {
    /* we want to write binary data transparently */
    GWEN_SyncIo_AddFlags(baseIo, GWEN_SYNCIO_FLAGS_TRANSPARENT);
    rv=GWEN_SyncIo_WriteForced(baseIo, buffer, size);
    if (rv<0) {
      xio->writeMode=GWEN_SyncIo_Http_Mode_Error;
      return rv;
    }
    xio->writeMode=GWEN_SyncIo_Http_Mode_ChunkSize;

    return rv;
  }

  if (xio->writeMode==GWEN_SyncIo_Http_Mode_Body) {
    if ((xio->currentWriteBodySize!=-1) &&
	(size>xio->currentWriteBodySize)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Size is beyond total body size (%d)!", size);
      xio->writeMode=GWEN_SyncIo_Http_Mode_Error;
      return GWEN_ERROR_INVALID;
    }

    /* we want to write binary data transparently */
    GWEN_SyncIo_AddFlags(baseIo, GWEN_SYNCIO_FLAGS_TRANSPARENT);
    rv=GWEN_SyncIo_WriteForced(baseIo, buffer, size);
    if (rv<0) {
      xio->writeMode=GWEN_SyncIo_Http_Mode_Error;
      return rv;
    }
    if (xio->currentWriteBodySize!=-1) {
      xio->currentWriteBodySize-=rv;
      if (xio->currentWriteBodySize==0)
	GWEN_SyncIo_Http_SetWriteIdle(sio);
    }

    return rv;
  }

  if (xio->writeMode==GWEN_SyncIo_Http_Mode_Error) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Previous write error");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int GWEN_SyncIo_Http_ReadLine(GWEN_SYNCIO *sio, GWEN_BUFFER *tbuf) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* we want to read a text line, so we can't have a transparent mode in the base layer */
  GWEN_SyncIo_SubFlags(baseIo, GWEN_SYNCIO_FLAGS_TRANSPARENT);

  /* read a single line */
  do {
    uint8_t *p;
    uint32_t l;

    GWEN_Buffer_AllocRoom(tbuf, 1024);
    p=(uint8_t*) GWEN_Buffer_GetPosPointer(tbuf);
    l=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf);
    rv=GWEN_SyncIo_Read(baseIo, p, l);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    else if (rv>0) {
      GWEN_Buffer_IncrementPos(tbuf, rv);
      GWEN_Buffer_AdjustUsedBytes(tbuf);
      if (p[rv-1]==10) {
        p[rv-1]=0;
	break;
      }
    }
    else if (rv==0)
      break;
  } while(rv>0);

  if (GWEN_Buffer_GetUsedBytes(tbuf)<1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Nothing received");
    return GWEN_ERROR_EOF;
  }

  return 0;
}



int GWEN_SyncIo_Http_ParseStatus(GWEN_SYNCIO *sio, char *buffer) {
  GWEN_SYNCIO_HTTP *xio;
  char *p;
  char *s;
  int code;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  s=buffer;

  /* read protocol */
  p=strchr(s, ' ');
  if (!p) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Bad format of HTTP status (%s)", buffer);
    return GWEN_ERROR_INVALID;
  }
  *p=0;
  p++;

  GWEN_DB_SetCharValue(xio->dbStatusIn, GWEN_DB_FLAGS_OVERWRITE_VARS, "protocol", s);
  s=p;

  /* read status code */
  while(*p && isdigit((int)*p))
    p++;
  if (*p) {
    *p=0;
    p++;
  }
  if (1!=sscanf(s, "%d", &code)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad request (status code \"%s\")", s);
    return GWEN_ERROR_INVALID;
  }
  GWEN_DB_SetIntValue(xio->dbStatusIn, GWEN_DB_FLAGS_OVERWRITE_VARS, "code", code);
  s=p;

  /* read text */
  GWEN_DB_SetCharValue(xio->dbStatusIn, GWEN_DB_FLAGS_OVERWRITE_VARS, "text", s);

  return 0;
}



int GWEN_SyncIo_Http_ParseCommand(GWEN_SYNCIO *sio, const char *buffer) {
  GWEN_SYNCIO_HTTP *xio;
  char *tmp;
  char *p;
  char *s;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

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

  GWEN_DB_SetCharValue(xio->dbCommandIn, GWEN_DB_FLAGS_OVERWRITE_VARS, "command", s);
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

  GWEN_DB_SetCharValue(xio->dbCommandIn, GWEN_DB_FLAGS_OVERWRITE_VARS, "url", s);
  s=p;

  if (*s==0) {
    /* no protocol information follows, so we assume HTTP/0.9 */
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad request (not in HTTP>=1.0)");
    free(tmp);
    return GWEN_ERROR_INVALID;
  }
  else {
    GWEN_DB_SetCharValue(xio->dbCommandIn, GWEN_DB_FLAGS_OVERWRITE_VARS, "protocol", s);
  }

  free(tmp);
  return 0;
}



int GWEN_SyncIo_Http_ReadStatus(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  GWEN_BUFFER *tbuf;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  DBG_INFO(GWEN_LOGDOMAIN, "Reading status");
  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* read a single line */
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_SyncIo_Http_ReadLine(sio, tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  if (*GWEN_Buffer_GetStart(tbuf)==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Empty line received while reading status response, assuming EOF");
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_EOF;
  }

  rv=GWEN_SyncIo_Http_ParseStatus(sio, GWEN_Buffer_GetStart(tbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  GWEN_Buffer_free(tbuf);
  return 0;
}



int GWEN_SyncIo_Http_ReadCommand(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  GWEN_BUFFER *tbuf;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  DBG_INFO(GWEN_LOGDOMAIN, "Reading command");
  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* read a single line */
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_SyncIo_Http_ReadLine(sio, tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  rv=GWEN_SyncIo_Http_ParseCommand(sio, GWEN_Buffer_GetStart(tbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  GWEN_Buffer_free(tbuf);
  return 0;
}



int GWEN_SyncIo_Http_ParseHeader(GWEN_SYNCIO *sio, char *buf) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  char *p;
  const char *s;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* resolve line continuations */
  p=buf;
  while(*p) {
    p=strchr(p, 10);
    if (p) {
      if (p[1]==32 || p[1]==9)
	/* found a continuation */
	*p=32;
      p++;
    }
  }

  /* parse every line */
  p=buf;
  while(p && *p) {
    char *pNext;
    char *pVarBegin;
    char *pVarEnd;

    /* skip blanks */
    pNext=strchr(p, 10);
    if (pNext) {
      *pNext=0;
      pNext++;
    }
    while(*p && (*p==32 || *p==9))
      p++;
    if (*p) {
      pVarBegin=p;
      while(*p && *p!=':' && *p>32 && *p<127)
	p++;
      pVarEnd=p;
      if (*p!=':') {
	DBG_INFO(GWEN_LOGDOMAIN, "No separator after variable name in received header");
	return GWEN_ERROR_BAD_DATA;
      }
      *pVarEnd=0;
      p++;

      while(*p && (*p==32 || *p==9))
	p++;
      if (*p)
	GWEN_DB_SetCharValue(xio->dbHeaderIn, GWEN_PATH_FLAGS_CREATE_VAR, pVarBegin, p);
    }
    p=pNext;
  }

  /* default next mode after reading the header is reading the body
   * (if any, but that will be checked later) */
  xio->readMode=GWEN_SyncIo_Http_Mode_Body;

  /* header received, now read some settings from it */
  s=GWEN_DB_GetCharValue(xio->dbHeaderIn, "Transfer-Encoding", 0, 0);
  if (s && (-1!=GWEN_Text_ComparePattern(s, "*chunked*", 0))) {
    /* chunked encoding, this means next we have to read the chunksize */
    xio->currentReadChunkSize=-1;
    xio->readMode=GWEN_SyncIo_Http_Mode_ChunkSize;
  }

  /* get size of body */
  xio->currentReadBodySize=GWEN_DB_GetIntValue(xio->dbHeaderIn, "Content-Length", 0, -1);
  if (xio->currentReadBodySize==0) {
    /* no body */
    GWEN_SyncIo_Http_SetReadIdle(sio);
  }
  else if (xio->currentReadBodySize==-1) {
    int rcode;

    /* no length of body received, assume 0 in case of an error
     * This eliminates the bug where this module waits for
     * a timeout when receiving an error from a special server
     */
    rcode=GWEN_DB_GetIntValue(xio->dbStatusIn, "code", 0, -1);
    if (rcode<0 || rcode>=300) {
      /* no body */
      GWEN_SyncIo_Http_SetReadIdle(sio);
    }
  }

  return 0;
}



int GWEN_SyncIo_Http_ReadHeader(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  GWEN_BUFFER *tbuf;
  int rv;
  uint32_t pos;
  int lines=0;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  DBG_INFO(GWEN_LOGDOMAIN, "Reading header");
  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* we want to read a text line, so we can't have a transparent mode in the base layer */
  GWEN_SyncIo_SubFlags(baseIo, GWEN_SYNCIO_FLAGS_TRANSPARENT);

  /* read a single line */
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  pos=0;
  do {
    uint8_t *p;

    GWEN_Buffer_AllocRoom(tbuf, 1024);
    p=(uint8_t*) GWEN_Buffer_GetPosPointer(tbuf);
    rv=GWEN_SyncIo_Read(baseIo, p, 1024);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
    else if (rv>0) {
      GWEN_Buffer_IncrementPos(tbuf, rv);
      GWEN_Buffer_AdjustUsedBytes(tbuf);
      if (p[rv-1]==10) {
	uint32_t npos;

        lines++;
	npos=GWEN_Buffer_GetPos(tbuf);
	if ((npos-pos)==1) {
	  /* empty line, header finished */
	  break;
	}
        pos=npos;
      }
    }
    else if (rv==0)
      break;
  } while(rv>0);

  if (lines<1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No header line received");
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_EOF;
  }

  rv=GWEN_SyncIo_Http_ParseHeader(sio, GWEN_Buffer_GetStart(tbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  GWEN_Buffer_free(tbuf);
  return 0;
}



int GWEN_SyncIo_Http_ReadChunkSize(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  GWEN_BUFFER *tbuf;
  int rv;
  int csize;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  DBG_INFO(GWEN_LOGDOMAIN, "Reading chunksize");
  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* read a single line */
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_SyncIo_Http_ReadLine(sio, tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  if (*GWEN_Buffer_GetStart(tbuf)==0) {
    GWEN_Buffer_Reset(tbuf);
    rv=GWEN_SyncIo_Http_ReadLine(sio, tbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
  }

  if (1!=sscanf(GWEN_Buffer_GetStart(tbuf), "%x", &csize)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad data received (invalid chunksize specifier: [%s])",
              GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_BAD_DATA;
  }

  xio->currentReadChunkSize=csize;

  GWEN_Buffer_free(tbuf);
  return 0;
}



int GWEN_SyncIo_Http_ReadChunk(GWEN_SYNCIO *sio, uint8_t *buffer, uint32_t size) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  DBG_DEBUG(GWEN_LOGDOMAIN, "Reading chunk (%d bytes)", (int) size);
  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* we want to read binary data transparently */
  GWEN_SyncIo_AddFlags(baseIo, GWEN_SYNCIO_FLAGS_TRANSPARENT);

  if (size>xio->currentReadChunkSize)
    size=xio->currentReadChunkSize;

  rv=GWEN_SyncIo_Read(baseIo, buffer, size);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  xio->currentReadChunkSize-=rv;
  if (xio->currentReadBodySize>0)
    xio->currentReadBodySize-=rv;

  if (xio->currentReadChunkSize==0)
    /* chunk finished, change read mode */
    xio->readMode=GWEN_SyncIo_Http_Mode_ChunkSize;

  return rv;
}



int GWEN_SyncIo_Http_ReadBody(GWEN_SYNCIO *sio, uint8_t *buffer, uint32_t size) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(size);

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  DBG_INFO(GWEN_LOGDOMAIN, "Reading body");
  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* we want to read binary data transparently */
  GWEN_SyncIo_AddFlags(baseIo, GWEN_SYNCIO_FLAGS_TRANSPARENT);

  if ((xio->currentReadBodySize>=0) &&
      (size>xio->currentReadBodySize)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Adjusting read body size from %d to %d",
	     size, xio->currentReadBodySize);
    size=xio->currentReadBodySize;
  }

  rv=GWEN_SyncIo_Read(baseIo, buffer, size);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (xio->currentReadBodySize>=0)
    xio->currentReadBodySize-=rv;

  if (xio->currentReadBodySize==0)
    /* body finished, change read mode */
    GWEN_SyncIo_Http_SetReadIdle(sio);

  return rv;
}



int GWEN_SyncIo_Http_WriteCommand(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  int rv;
  const char *s;
  GWEN_BUFFER *tbuf;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* we will construct the line including CR/LF ourselves */
  GWEN_SyncIo_AddFlags(baseIo, GWEN_SYNCIO_FLAGS_TRANSPARENT);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  s=GWEN_DB_GetCharValue(xio->dbCommandOut, "command", 0, "GET");
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " ");

  s=GWEN_DB_GetCharValue(xio->dbCommandOut, "url", 0, "/");
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " ");

  s=GWEN_DB_GetCharValue(xio->dbCommandOut, "protocol", 0, "HTTP/1.0");
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\r\n");

  /* write */
  rv=GWEN_SyncIo_WriteForced(baseIo,
			     (const uint8_t*) GWEN_Buffer_GetStart(tbuf),
			     GWEN_Buffer_GetUsedBytes(tbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  GWEN_Buffer_free(tbuf);
  return 0;
}



int GWEN_SyncIo_Http_WriteStatus(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  int rv;
  const char *s;
  GWEN_BUFFER *tbuf;
  char numbuf[32];
  int i;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* we will construct the line including CR/LF ourselves */
  GWEN_SyncIo_AddFlags(baseIo, GWEN_SYNCIO_FLAGS_TRANSPARENT);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  s=GWEN_DB_GetCharValue(xio->dbStatusOut, "protocol", 0, "HTTP/1.0");
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " ");

  i=GWEN_DB_GetIntValue(xio->dbStatusOut, "code", 0, -1);
  if (i==-1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Missing status code");
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_NO_DATA;
  }
  snprintf(numbuf, sizeof(numbuf), "%d ", i);
  GWEN_Buffer_AppendString(tbuf, numbuf);

  s=GWEN_DB_GetCharValue(xio->dbStatusOut, "text", 0, "No text.");
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\r\n");

  /* write */
  rv=GWEN_SyncIo_WriteForced(baseIo,
			     (const uint8_t*) GWEN_Buffer_GetStart(tbuf),
			     GWEN_Buffer_GetUsedBytes(tbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  GWEN_Buffer_free(tbuf);
  return 0;
}



int GWEN_SyncIo_Http_WriteHeader(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  int i;
  GWEN_DB_NODE *dbVar;
  GWEN_BUFFER *tbuf;
  int rv;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* we will construct the line including CR/LF ourselves */
  GWEN_SyncIo_AddFlags(baseIo, GWEN_SYNCIO_FLAGS_TRANSPARENT);

  /* default next mode after writing the header is writing the body
   * (if any, but that will be checked later) */
  xio->writeMode=GWEN_SyncIo_Http_Mode_Body;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  i=GWEN_DB_GetIntValue(xio->dbHeaderOut, "Content-Length", 0, -1);
  xio->currentWriteBodySize=i;

  dbVar=GWEN_DB_GetFirstVar(xio->dbHeaderOut);
  while (dbVar) {
    GWEN_DB_NODE *dbVal;

    /* only handle first value */
    dbVal=GWEN_DB_GetFirstValue(dbVar);
    if (dbVal) {
      GWEN_DB_NODE_TYPE vtype;

      vtype=GWEN_DB_GetValueType(dbVal);
      if (vtype==GWEN_DB_NodeType_ValueChar) {
	const char *s;

	GWEN_Buffer_AppendString(tbuf, GWEN_DB_VariableName(dbVar));
	GWEN_Buffer_AppendString(tbuf, ":");
	s=GWEN_DB_GetCharValueFromNode(dbVal);
        if (s)
	  GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\r\n");

	if (strcasecmp(GWEN_DB_VariableName(dbVar), "Transfer-Encoding")==0) {
	  if (s && (-1!=GWEN_Text_ComparePattern(s, "*chunked*", 0))) {
	    /* chunked encoding, this means next we have to write the chunksize */
	    xio->writeMode=GWEN_SyncIo_Http_Mode_ChunkSize;
	  }
	}
      }
      else if (vtype==GWEN_DB_NodeType_ValueInt) {
	i=GWEN_DB_GetIntValueFromNode(dbVal);
	if (i!=-1 || strcasecmp(GWEN_DB_VariableName(dbVar), "Content-Length")==0) {
	  char numbuf[32];

	  /* don't write body size of -1 */
	  GWEN_Buffer_AppendString(tbuf, GWEN_DB_VariableName(dbVar));
	  GWEN_Buffer_AppendString(tbuf, ":");
	  snprintf(numbuf, sizeof(numbuf), "%d", i);
	  GWEN_Buffer_AppendString(tbuf, numbuf);
	  GWEN_Buffer_AppendString(tbuf, "\r\n");
	}
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "Variable type %d of var [%s] not supported",
		 vtype, GWEN_DB_VariableName(dbVar));
	return GWEN_ERROR_BAD_DATA;
      }
    }
    dbVar=GWEN_DB_GetNextVar(dbVar);
  }

  /* finalize header */
  GWEN_Buffer_AppendString(tbuf, "\r\n");

  /* write */
  rv=GWEN_SyncIo_WriteForced(baseIo,
			     (const uint8_t*) GWEN_Buffer_GetStart(tbuf),
			     GWEN_Buffer_GetUsedBytes(tbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  GWEN_Buffer_free(tbuf);

  if (xio->currentWriteBodySize==0)
    GWEN_SyncIo_Http_SetWriteIdle(sio);

  return 0;
}



int GWEN_SyncIo_Http_WriteChunkSize(GWEN_SYNCIO *sio, uint32_t size) {
  GWEN_SYNCIO_HTTP *xio;
  GWEN_SYNCIO *baseIo;
  int rv;
  char numbuf[32];

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  baseIo=GWEN_SyncIo_GetBaseIo(sio);
  assert(baseIo);

  /* we will construct the line including CR/LF ourselves */
  GWEN_SyncIo_AddFlags(baseIo, GWEN_SYNCIO_FLAGS_TRANSPARENT);

  snprintf(numbuf, sizeof(numbuf)-1, "%x\r\n", size);
  numbuf[sizeof(numbuf)-1]=0;

  rv=GWEN_SyncIo_WriteForced(baseIo, (const uint8_t*) numbuf, strlen(numbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



void GWEN_SyncIo_Http_SetWriteIdle(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  xio->writeMode=GWEN_SyncIo_Http_Mode_Idle;
  GWEN_DB_ClearGroup(xio->dbStatusOut, NULL);
}




GWEN_DB_NODE *GWEN_SyncIo_Http_GetDbCommandIn(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  return xio->dbCommandIn;
}



GWEN_DB_NODE *GWEN_SyncIo_Http_GetDbStatusIn(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  return xio->dbStatusIn;
}



GWEN_DB_NODE *GWEN_SyncIo_Http_GetDbHeaderIn(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  return xio->dbHeaderIn;
}



GWEN_DB_NODE *GWEN_SyncIo_Http_GetDbCommandOut(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  return xio->dbCommandOut;
}



GWEN_DB_NODE *GWEN_SyncIo_Http_GetDbStatusOut(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  return xio->dbStatusOut;
}



GWEN_DB_NODE *GWEN_SyncIo_Http_GetDbHeaderOut(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_HTTP *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  return xio->dbHeaderOut;
}




int GWEN_SyncIo_Http_RecvBody(GWEN_SYNCIO *sio, GWEN_BUFFER *buf) {
  GWEN_SYNCIO_HTTP *xio;
  int rv;
  int code=0;
  int firstRead=1;
  int bodySize=-1;
  int bytesRead=0;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_HTTP, sio);
  assert(xio);

  /* recv packet (this reads the HTTP body) */
  for (;;) {
    uint8_t *p;
    uint32_t l;

    rv=GWEN_Buffer_AllocRoom(buf, 1024);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    p=(uint8_t*) GWEN_Buffer_GetPosPointer(buf);
    l=GWEN_Buffer_GetMaxUnsegmentedWrite(buf);
    do {
      rv=GWEN_SyncIo_Read(sio, p, l-1);
    } while(rv==GWEN_ERROR_INTERRUPTED);

    if (rv==0)
      break;
    else if (rv<0) {
      if (rv==GWEN_ERROR_EOF) {
	if (bodySize!=-1 && bytesRead<bodySize) {
	    DBG_ERROR(GWEN_LOGDOMAIN,
		      "EOF met prematurely (%d < %d)",
		      bytesRead, bodySize);
	    return GWEN_ERROR_EOF;
	}
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	/*return rv;*/
        break;
      }
    }
    else {
      GWEN_Buffer_IncrementPos(buf, rv);
      GWEN_Buffer_AdjustUsedBytes(buf);
      if (firstRead) {
	GWEN_DB_NODE *db;

	db=GWEN_SyncIo_Http_GetDbHeaderIn(sio);
	bodySize=GWEN_DB_GetIntValue(db, "Content-length", 0, -1);
      }
      bytesRead+=rv;
    }

    if (bodySize!=-1 && bytesRead>=bodySize) {
      break;
    }
    firstRead=0;
  }

  if (rv<0) {
    if (GWEN_Buffer_GetUsedBytes(buf)) {
      /* data received, check for common error codes */
      if (rv==GWEN_ERROR_EOF || rv==GWEN_ERROR_IO || rv==GWEN_ERROR_SSL) {
	DBG_INFO(GWEN_LOGDOMAIN,
		 "We received an error, but we still got data, "
		 "so we ignore the error here");
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "No message received (%d)", rv);
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Error,
			     I18N("No message received"));
	return rv;
      }
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "No message received (%d)", rv);
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
			   I18N("No message received"));
      return rv;
    }
  }

  if (GWEN_SyncIo_GetFlags(sio) & GWEN_SYNCIO_FLAGS_PASSIVE)
    code=0;
  else {
    code=GWEN_DB_GetIntValue(xio->dbStatusIn, "code", 0, 0);
    if (code) {
      const char *s;

      s=GWEN_DB_GetCharValue(xio->dbStatusIn, "text", 0, NULL);
      DBG_DEBUG(GWEN_LOGDOMAIN, "HTTP-Status: %d (%s)",
		code, s?s:"- no text -");
      GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Info,
			    I18N("HTTP-Status: %d (%s)"),
			    code, s?s:I18N("- no details -)"));
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "No HTTP status code received");
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
			   I18N("No HTTP status code received"));
      code=GWEN_ERROR_BAD_DATA;
    }
  }

  return code;
}




