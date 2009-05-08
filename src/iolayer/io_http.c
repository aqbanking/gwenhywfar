/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "io_http_p.h"
#include <gwenhywfar/iolayer_be.h>
#include <gwenhywfar/iorequest_be.h>
#include <gwenhywfar/iomanager.h>
#include <gwenhywfar/io_buffered.h>

#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>

#include <assert.h>
#include <ctype.h>




GWEN_INHERIT(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP)





GWEN_IO_LAYER *GWEN_Io_LayerHttp_new(GWEN_IO_LAYER *baseLayer) {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER_HTTP *xio;

  io=GWEN_Io_Layer_new(GWEN_IO_LAYER_HTTP_TYPE, baseLayer);
  assert(io);
  GWEN_NEW_OBJECT(GWEN_IO_LAYER_HTTP, xio);
  assert(xio);
  GWEN_INHERIT_SETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io, xio, GWEN_Io_LayerHttp_freeData);

  xio->dbCommandIn=GWEN_DB_Group_new("command");
  xio->dbStatusIn=GWEN_DB_Group_new("status");
  xio->dbHeaderIn=GWEN_DB_Group_new("header");
  xio->readBuffer=GWEN_Buffer_new(0, GWEN_IO_LAYER_HTTP_LINE_STEPSIZE, 0, 1);

  xio->dbCommandOut=GWEN_DB_Group_new("command");
  xio->dbStatusOut=GWEN_DB_Group_new("status");
  xio->dbHeaderOut=GWEN_DB_Group_new("header");
  xio->writeBuffer=GWEN_Buffer_new(0, GWEN_IO_LAYER_HTTP_LINE_STEPSIZE, 0, 1);

  GWEN_Io_Layer_SetWorkOnRequestsFn(io, GWEN_Io_LayerHttp_WorkOnRequests);
  GWEN_Io_Layer_SetAddRequestFn(io, GWEN_Io_LayerHttp_AddRequest);
  GWEN_Io_Layer_SetDelRequestFn(io, GWEN_Io_LayerHttp_DelRequest);
  GWEN_Io_Layer_SetHasWaitingRequestsFn(io, GWEN_Io_LayerHttp_HasWaitingRequests);

  return io;
}



GWENHYWFAR_CB
void GWEN_Io_LayerHttp_freeData(GWEN_UNUSED void *bp, void *p) {
  GWEN_IO_LAYER_HTTP *xio;

  xio=(GWEN_IO_LAYER_HTTP*) p;
  GWEN_DB_Group_free(xio->dbCommandIn);
  GWEN_DB_Group_free(xio->dbStatusIn);
  GWEN_DB_Group_free(xio->dbHeaderIn);
  GWEN_Buffer_free(xio->readBuffer);

  GWEN_DB_Group_free(xio->dbCommandOut);
  GWEN_DB_Group_free(xio->dbStatusOut);
  GWEN_DB_Group_free(xio->dbHeaderOut);
  GWEN_Buffer_free(xio->writeBuffer);

  GWEN_FREE_OBJECT(xio);
}



GWEN_DB_NODE *GWEN_Io_LayerHttp_GetDbCommandIn(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  return xio->dbCommandIn;
}



GWEN_DB_NODE *GWEN_Io_LayerHttp_GetDbStatusIn(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  return xio->dbStatusIn;
}



GWEN_DB_NODE *GWEN_Io_LayerHttp_GetDbHeaderIn(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  return xio->dbHeaderIn;
}



GWEN_DB_NODE *GWEN_Io_LayerHttp_GetDbCommandOut(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  return xio->dbCommandOut;
}



GWEN_DB_NODE *GWEN_Io_LayerHttp_GetDbStatusOut(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  return xio->dbStatusOut;
}



GWEN_DB_NODE *GWEN_Io_LayerHttp_GetDbHeaderOut(const GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  return xio->dbHeaderOut;
}



void GWEN_Io_LayerHttp_AbortInRequests(GWEN_IO_LAYER *io, int errorCode) {
  GWEN_IO_LAYER_HTTP *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  if (xio->readRequestIn) {
    GWEN_IO_REQUEST *r;

    r=xio->readRequestIn;
    xio->readRequestIn=NULL;
    DBG_INFO(GWEN_LOGDOMAIN, "Aborting in read request");
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, errorCode);
    GWEN_Io_Request_free(r);
  }
  if (xio->writeRequestIn) {
    GWEN_IO_REQUEST *r;

    r=xio->writeRequestIn;
    xio->writeRequestIn=NULL;
    DBG_INFO(GWEN_LOGDOMAIN, "Aborting in write request");
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, errorCode);
    GWEN_Io_Request_free(r);
  }
}



void GWEN_Io_LayerHttp_AbortOutRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  if (xio->readRequestOut) {
    GWEN_IO_REQUEST *r;

    r=xio->readRequestOut;
    GWEN_Io_Layer_DelRequest(GWEN_Io_Layer_GetBaseLayer(io), r);
    xio->readRequestOut=NULL;
    GWEN_Io_Request_free(r);
  }
  if (xio->writeRequestIn) {
    GWEN_IO_REQUEST *r;

    r=xio->writeRequestIn;
    GWEN_Io_Layer_DelRequest(GWEN_Io_Layer_GetBaseLayer(io), r);
    xio->writeRequestIn=NULL;
    GWEN_Io_Request_free(r);
  }
}





int GWEN_Io_LayerHttp_ParseCommand(GWEN_IO_LAYER *io, const char *buffer) {
  GWEN_IO_LAYER_HTTP *xio;
  char *tmp;
  char *p;
  char *s;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
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
    /* be less strict n IPC mode */
    if (GWEN_Io_Layer_GetFlags(io) & GWEN_IO_LAYER_HTTP_FLAGS_IPC) {
      DBG_INFO(GWEN_LOGDOMAIN, "Ignoring missing url");
      free(tmp);
      return 0;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Bad format of HTTP request (%s)", buffer);
      free(tmp);
      return GWEN_ERROR_INVALID;
    }
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



int GWEN_Io_LayerHttp_ParseStatus(GWEN_IO_LAYER *io, const char *buffer) {
  GWEN_IO_LAYER_HTTP *xio;
  char *tmp;
  char *p;
  char *s;
  int code;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

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
    free(tmp);
    return GWEN_ERROR_INVALID;
  }
  GWEN_DB_SetIntValue(xio->dbStatusIn, GWEN_DB_FLAGS_OVERWRITE_VARS, "code", code);
  s=p;

  /* read text */
  GWEN_DB_SetCharValue(xio->dbStatusIn, GWEN_DB_FLAGS_OVERWRITE_VARS, "text", s);

  free(tmp);
  return 0;
}



int GWEN_Io_LayerHttp_ParseHeader(GWEN_IO_LAYER *io, char *buf) {
  GWEN_IO_LAYER_HTTP *xio;
  char *p;
  const char *s;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

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
  xio->readMode=GWEN_Io_LayerHttp_Mode_Body;

  /* header received, now read some settings from it */
  s=GWEN_DB_GetCharValue(xio->dbHeaderIn, "Transfer-Encoding", 0, 0);
  if (s && (-1!=GWEN_Text_ComparePattern(s, "*chunked*", 0))) {
    /* chunked encoding, this means next we have to read the chunksize */
    xio->currentReadChunkSize=-1;
    xio->readMode=GWEN_Io_LayerHttp_Mode_ChunkSize;
  }

  /* get size of body */
  xio->currentReadBodySize=GWEN_DB_GetIntValue(xio->dbHeaderIn, "Content-Length", 0, -1);
  if (xio->currentReadBodySize==0) {
    /* no body */
    xio->readMode=GWEN_Io_LayerHttp_Mode_Finished;
  }
  if (xio->currentReadBodySize==-1) {
    int rcode;

    /* no length of body received, assume 0 in case of an error
     * This eliminates the bug where this module waits for
     * a timeout when receiving an error from a special server
     */
    rcode=GWEN_DB_GetIntValue(xio->dbStatusIn, "code", 0, -1);
    if (rcode<0 || rcode>=300) {
      /* no body */
      xio->readMode=GWEN_Io_LayerHttp_Mode_Finished;
    }
  }

  return 0;
}



int GWEN_Io_LayerHttp_WriteCommand(GWEN_IO_LAYER *io, GWEN_BUFFER *buf) {
  GWEN_IO_LAYER_HTTP *xio;
  const char *s;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  s=GWEN_DB_GetCharValue(xio->dbCommandOut, "command", 0, "GET");
  GWEN_Buffer_AppendString(buf, s);
  GWEN_Buffer_AppendString(buf, " ");

  s=GWEN_DB_GetCharValue(xio->dbCommandOut, "url", 0, "/");
  GWEN_Buffer_AppendString(buf, s);
  GWEN_Buffer_AppendString(buf, " ");

  s=GWEN_DB_GetCharValue(xio->dbCommandOut, "protocol", 0, "HTTP/1.0");
  GWEN_Buffer_AppendString(buf, s);
  GWEN_Buffer_AppendString(buf, "\r\n");

  return 0;
}



int GWEN_Io_LayerHttp_WriteStatus(GWEN_IO_LAYER *io, GWEN_BUFFER *buf) {
  GWEN_IO_LAYER_HTTP *xio;
  const char *s;
  int i;
  char numbuf[32];

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  s=GWEN_DB_GetCharValue(xio->dbStatusOut, "protocol", 0, "HTTP/1.0");
  GWEN_Buffer_AppendString(buf, s);
  GWEN_Buffer_AppendString(buf, " ");

  i=GWEN_DB_GetIntValue(xio->dbStatusOut, "code", 0, -1);
  if (i==-1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Missing status code");
    return GWEN_ERROR_NO_DATA;
  }
  snprintf(numbuf, sizeof(numbuf), "%d ", i);
  GWEN_Buffer_AppendString(buf, numbuf);

  s=GWEN_DB_GetCharValue(xio->dbStatusOut, "text", 0, "No text.");
  GWEN_Buffer_AppendString(buf, s);
  GWEN_Buffer_AppendString(buf, "\r\n");

  return 0;
}



int GWEN_Io_LayerHttp_WriteHeader(GWEN_IO_LAYER *io, GWEN_BUFFER *buf) {
  GWEN_IO_LAYER_HTTP *xio;
  int i;
  GWEN_DB_NODE *dbVar;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  i=GWEN_DB_GetIntValue(xio->dbHeaderOut, "Content-Length", 0, -1);
  if (i!=-1)
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

	GWEN_Buffer_AppendString(buf, GWEN_DB_VariableName(dbVar));
	GWEN_Buffer_AppendString(buf, ":");
	s=GWEN_DB_GetCharValueFromNode(dbVal);
        if (s)
	  GWEN_Buffer_AppendString(buf, s);
	GWEN_Buffer_AppendString(buf, "\r\n");
      }
      else if (vtype==GWEN_DB_NodeType_ValueInt) {
	i=GWEN_DB_GetIntValueFromNode(dbVal);
	if (i!=-1 || strcasecmp(GWEN_DB_VariableName(dbVar), "Content-Length")==0) {
	  char numbuf[32];

	  /* dont write body size of -1 */
	  GWEN_Buffer_AppendString(buf, GWEN_DB_VariableName(dbVar));
	  GWEN_Buffer_AppendString(buf, ":");
	  snprintf(numbuf, sizeof(numbuf), "%d", i);
	  GWEN_Buffer_AppendString(buf, numbuf);
	  GWEN_Buffer_AppendString(buf, "\r\n");
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
  GWEN_Buffer_AppendString(buf, "\r\n");

  return 0;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnReadRequest1(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "GWEN_Io_LayerHttp_WorkOnReadRequest1 (%d)", xio->readMode);

  if (xio->readRequestOut) {
    /* check for finished outbound read request */
    if (GWEN_Io_Request_GetStatus(xio->readRequestOut)==GWEN_Io_Request_StatusFinished) {
      uint32_t bpos;
      uint32_t flags;

      doneSomething=1;

      /* get all available data */
      xio->lastReadOutResult=GWEN_Io_Request_GetResultCode(xio->readRequestOut);
      flags=GWEN_Io_Request_GetFlags(xio->readRequestOut);
      bpos=GWEN_Io_Request_GetBufferPos(xio->readRequestOut);

      /* adjusting buffer pointers */
      GWEN_Buffer_IncrementPos(xio->readBuffer, bpos);
      GWEN_Buffer_AdjustUsedBytes(xio->readBuffer);

      if (flags & GWEN_IO_REQUEST_FLAGS_PACKETEND) {
	xio->readLineFinished=1;
      }

      DBG_VERBOUS(GWEN_LOGDOMAIN, "Received %d bytes (%s)",
		  bpos, (xio->readLineFinished)?"line finished":"line open");

      /* request handled */
      GWEN_Io_Request_free(xio->readRequestOut);
      xio->readRequestOut=NULL;
    }
  } /* if readRequestOut */

  if (xio->readRequestIn) {
    GWEN_IO_REQUEST *rIn;

    rIn=xio->readRequestIn;
    if (xio->readLineFinished==0) {
      if (xio->readRequestOut==NULL) {
	int rv;

	if (xio->lastReadOutResult) {
	  xio->readRequestIn=NULL;
	  DBG_INFO(GWEN_LOGDOMAIN, "Aborting out read request (reason: %d)",
		   xio->lastReadOutResult);
	  GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, xio->lastReadOutResult);
	  GWEN_Io_Request_free(rIn);
	  rIn=NULL;
	  doneSomething=1;
	}
	else {
	  rv=GWEN_Buffer_AllocRoom(xio->readBuffer, GWEN_IO_LAYER_HTTP_LINE_STEPSIZE);
	  if (rv<0) {
	    xio->readRequestIn=NULL;
	    xio->lastReadOutResult=GWEN_ERROR_MEMORY_FULL;
	    DBG_INFO(GWEN_LOGDOMAIN, "Aborting out read request (reason: %d)",
		     xio->lastReadOutResult);
	    GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, GWEN_ERROR_MEMORY_FULL);
	    GWEN_Io_Request_free(rIn);
	    rIn=NULL;
	    doneSomething=1;
	  }
	  else {
	    GWEN_IO_REQUEST *rOut;

	    DBG_VERBOUS(GWEN_LOGDOMAIN, "Reading %d header bytes", GWEN_IO_LAYER_HTTP_LINE_STEPSIZE);
	    rOut=GWEN_Io_Request_new(GWEN_Io_Request_TypeRead,
				     (uint8_t*)GWEN_Buffer_GetPosPointer(xio->readBuffer),
				     GWEN_IO_LAYER_HTTP_LINE_STEPSIZE,
				     NULL, NULL,
				     GWEN_Io_Request_GetGuiId(rIn));
	    rv=GWEN_Io_Layer_AddRequest(GWEN_Io_Layer_GetBaseLayer(io), rOut);
	    if (rv) {
	      if (rv!=GWEN_ERROR_TRY_AGAIN) {
		GWEN_Io_Request_free(rOut);
		xio->readRequestIn=NULL;
		xio->lastReadOutResult=rv;
		DBG_INFO(GWEN_LOGDOMAIN,
			 "Aborting out read request (reason: %d)",
			 xio->lastReadOutResult);
		GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, rv);
		GWEN_Io_Request_free(rIn);
		rIn=NULL;
		doneSomething=1;
	      }
	    }
	    else {
	      DBG_DEBUG(GWEN_LOGDOMAIN, "Added header read request");
	      xio->readRequestOut=rOut;
	      doneSomething=1;
	    }
	  } /* if allocRoom succeeded */
	} /* if last result was ok */
      } /* if no readRequestOut */
    } /* if line not finished */
    else {
      int rv;

      /* line finished */
      doneSomething=1;
      xio->readLineFinished=0; /* reset for next line */
      switch(xio->readMode) {
      case GWEN_Io_LayerHttp_Mode_Command:
	DBG_DEBUG(GWEN_LOGDOMAIN, "Command received: [%s]", GWEN_Buffer_GetStart(xio->readBuffer));
	rv=GWEN_Io_LayerHttp_ParseCommand(io, GWEN_Buffer_GetStart(xio->readBuffer));
	if (rv) {
	  xio->lastReadOutResult=rv;
	  xio->readRequestIn=NULL;
	  DBG_INFO(GWEN_LOGDOMAIN,
		   "Aborting in read request (reason: %d)",
		   xio->lastReadOutResult);
	  GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, rv);
	  GWEN_Io_Request_free(rIn);
	  rIn=NULL;
	}
	else {
	  xio->lastHeaderPos=0;
	  xio->readMode=GWEN_Io_LayerHttp_Mode_Header;
	}
	GWEN_Buffer_Reset(xio->readBuffer);
	break;

      case GWEN_Io_LayerHttp_Mode_Status:
	DBG_DEBUG(GWEN_LOGDOMAIN, "Status received: [%s]", GWEN_Buffer_GetStart(xio->readBuffer));
	rv=GWEN_Io_LayerHttp_ParseStatus(io, GWEN_Buffer_GetStart(xio->readBuffer));
	if (rv) {
	  xio->lastReadOutResult=rv;
	  xio->readRequestIn=NULL;
	  DBG_INFO(GWEN_LOGDOMAIN,
		   "Aborting in read request (reason: %d)",
		   xio->lastReadOutResult);
	  GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, rv);
	  GWEN_Io_Request_free(rIn);
          rIn=NULL;
	}
	else {
	  xio->lastHeaderPos=0;
	  xio->readMode=GWEN_Io_LayerHttp_Mode_Header;
	}
        GWEN_Buffer_Reset(xio->readBuffer);
	break;

      case GWEN_Io_LayerHttp_Mode_Header: {
	uint32_t i;

	DBG_DEBUG(GWEN_LOGDOMAIN, "Header received: [%s]",
		  GWEN_Buffer_GetStart(xio->readBuffer)+xio->lastHeaderPos);
	i=GWEN_Buffer_GetUsedBytes(xio->readBuffer);
	/* append line feed for header to allow parsing of the different lines */
	GWEN_Buffer_AppendByte(xio->readBuffer, 10);
	if ((i-xio->lastHeaderPos)==0) {
	  /* header finished, parse it. NOTE: This function will also determine the next
	   * readMode and the size of the body (if any) */
          DBG_DEBUG(GWEN_LOGDOMAIN, "Parsing header");
	  rv=GWEN_Io_LayerHttp_ParseHeader(io, GWEN_Buffer_GetStart(xio->readBuffer));
	  if (rv<0) {
	    xio->lastReadOutResult=rv;
	    xio->readRequestIn=NULL;
	    DBG_INFO(GWEN_LOGDOMAIN,
		     "Aborting in read request (reason: %d)",
		     xio->lastReadOutResult);
	    GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, rv);
	    GWEN_Io_Request_free(rIn);
	    rIn=NULL;
	  }
	  GWEN_Buffer_Reset(xio->readBuffer);
	}
	else {
	  DBG_DEBUG(GWEN_LOGDOMAIN, "Awaiting next header");
	  xio->lastHeaderPos=i+1; /* count the added #10 */
	  if (xio->readRequestOut==NULL) {
	    if (xio->lastReadOutResult) {
	      xio->readRequestIn=NULL;
	      DBG_INFO(GWEN_LOGDOMAIN,
		       "Aborting in read request (reason: %d)",
		       xio->lastReadOutResult);
	      GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, xio->lastReadOutResult);
	      GWEN_Io_Request_free(rIn);
	      rIn=NULL;
	      doneSomething=1;
	    }
	    else {
	      /* read the next header line */
	      rv=GWEN_Buffer_AllocRoom(xio->readBuffer, GWEN_IO_LAYER_HTTP_LINE_STEPSIZE);
	      if (rv<0) {
		xio->readRequestIn=NULL;
		xio->lastReadOutResult=GWEN_ERROR_MEMORY_FULL;
		DBG_INFO(GWEN_LOGDOMAIN,
			 "Aborting in read request (reason: %d)",
			 xio->lastReadOutResult);
		GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, GWEN_ERROR_MEMORY_FULL);
		GWEN_Io_Request_free(rIn);
		rIn=NULL;
		doneSomething=1;
	      }
	      else {
		GWEN_IO_REQUEST *rOut;

		DBG_VERBOUS(GWEN_LOGDOMAIN, "Reading %d header bytes", GWEN_IO_LAYER_HTTP_LINE_STEPSIZE);
		rOut=GWEN_Io_Request_new(GWEN_Io_Request_TypeRead,
					 (uint8_t*)GWEN_Buffer_GetPosPointer(xio->readBuffer),
					 GWEN_IO_LAYER_HTTP_LINE_STEPSIZE,
					 NULL, NULL,
					 GWEN_Io_Request_GetGuiId(rIn));
		rv=GWEN_Io_Layer_AddRequest(GWEN_Io_Layer_GetBaseLayer(io), rOut);
		if (rv) {
		  if (rv!=GWEN_ERROR_TRY_AGAIN) {
		    GWEN_Io_Request_free(rOut);
		    xio->readRequestIn=NULL;
		    xio->lastReadOutResult=rv;
		    DBG_INFO(GWEN_LOGDOMAIN,
			     "Aborting in read request (reason: %d)",
			     xio->lastReadOutResult);
		    GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, rv);
		    GWEN_Io_Request_free(rIn);
		    rIn=NULL;
		    doneSomething=1;
		  }
		}
		else {
		  DBG_DEBUG(GWEN_LOGDOMAIN, "Added header read request");
		  xio->readRequestOut=rOut;
		  doneSomething=1;
		}
	      } /* if allocRoom succeeded */
	    }
	  } /* if no readRequestOut */
	}
        break;
      }

      case GWEN_Io_LayerHttp_Mode_ChunkSize: {
	int csize;

	if (GWEN_Buffer_GetUsedBytes(xio->readBuffer)==0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "Empty line, skipping");
	}
	else {
	  if (1!=sscanf(GWEN_Buffer_GetStart(xio->readBuffer), "%x", &csize)) {
	    DBG_INFO(GWEN_LOGDOMAIN, "Invalid chunksize [%s]", GWEN_Buffer_GetStart(xio->readBuffer));
	    xio->lastReadOutResult=GWEN_ERROR_BAD_DATA;
	    xio->readRequestIn=NULL;
	    DBG_INFO(GWEN_LOGDOMAIN,
		     "Aborting in read request (reason: %d)",
		     xio->lastReadOutResult);
	    GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, GWEN_ERROR_BAD_DATA);
	    GWEN_Io_Request_free(rIn);
	    rIn=NULL;
	  }
	  else {
	    xio->currentReadChunkSize=csize;
	    DBG_DEBUG(GWEN_LOGDOMAIN, "Chunksize: %d", csize);
	    if (csize==0) {
	      DBG_DEBUG(GWEN_LOGDOMAIN, "Last chunk received");
	      xio->readMode=GWEN_Io_LayerHttp_Mode_Finished;
	      GWEN_Io_Request_AddFlags(rIn, GWEN_IO_REQUEST_FLAGS_PACKETEND);
	    }
	    else {
	      DBG_DEBUG(GWEN_LOGDOMAIN, "Started reading next chunk (%d bytes)",
			csize);
	      xio->readMode=GWEN_Io_LayerHttp_Mode_Chunk;
	    }
	  }
	}
	break;
      }

      default:
	/* should never reach this point */
        assert(0);
	break;
      }
    } /* if line finished */
  } /* if readRequestIn */

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnReadRequest2(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "GWEN_Io_LayerHttp_WorkOnReadRequest2 (%d)", xio->readMode);

  if (xio->readRequestOut) {
    /* check for finished outbound read request */
    if (GWEN_Io_Request_GetStatus(xio->readRequestOut)==GWEN_Io_Request_StatusFinished) {
      GWEN_IO_REQUEST *rOut;
      uint32_t bposOut;
      uint32_t flagsOut;

      rOut=xio->readRequestOut;
      flagsOut=GWEN_Io_Request_GetFlags(rOut);
      bposOut=GWEN_Io_Request_GetBufferPos(rOut);

      doneSomething=1;

      if (xio->readRequestIn) {
	GWEN_IO_REQUEST *rIn;
	uint32_t bposIn;
        uint32_t flagsIn;

	rIn=xio->readRequestIn;
	flagsIn=GWEN_Io_Request_GetFlags(rIn);
	bposIn=GWEN_Io_Request_GetBufferPos(rIn);

	DBG_VERBOUS(GWEN_LOGDOMAIN, "Received %d (total: %d) bytes", bposIn, bposIn+bposOut);

	/* get all available data */
	xio->lastReadOutResult=GWEN_Io_Request_GetResultCode(xio->readRequestOut);

	/* delete outbound request */
	GWEN_Io_Request_free(xio->readRequestOut);
	xio->readRequestOut=NULL;

	/* advance point in incoming buffer by amount of data read */
	bposIn+=bposOut;
	GWEN_Io_Request_SetBufferPos(rIn, bposIn);
        /* decrease number of bytes to read for body */
	if (xio->currentReadBodySize!=-1) {
	  xio->currentReadBodySize-=bposOut;
	}
	/* decrease number of bytes to read for chunk (if any) */
	if (xio->readMode==GWEN_Io_LayerHttp_Mode_Chunk) {
	  xio->currentReadChunkSize-=bposOut;
	}
	/* check whether the body has been finished */
	if (xio->currentReadBodySize==0) {
	  /* body finished (in any mode) */
	  GWEN_Io_Request_AddFlags(rIn, GWEN_IO_REQUEST_FLAGS_PACKETEND);
	  xio->readMode=GWEN_Io_LayerHttp_Mode_Finished;
	}
	else {
	  if (xio->readMode==GWEN_Io_LayerHttp_Mode_Chunk && xio->currentReadChunkSize==0) {
	    /* chunk finished, next step is to read the size of the next chunk */
            DBG_DEBUG(GWEN_LOGDOMAIN, "Chunk finished");
	    xio->currentReadChunkSize=-1;
	    xio->readMode=GWEN_Io_LayerHttp_Mode_ChunkSize;
	    xio->readLineFinished=0;
	    GWEN_Buffer_Reset(xio->readBuffer);
	    doneSomething=1;
	  }
	}
      }
    }
  }

  if (xio->readRequestIn) {
    GWEN_IO_REQUEST *rIn;
    uint32_t flagsIn;

    rIn=xio->readRequestIn;
    flagsIn=GWEN_Io_Request_GetFlags(rIn);

    if (xio->readMode==GWEN_Io_LayerHttp_Mode_Finished) {
      uint32_t bposIn;

      GWEN_Io_Request_AddFlags(rIn, GWEN_IO_REQUEST_FLAGS_PACKETEND);
      bposIn=GWEN_Io_Request_GetBufferPos(rIn);
      if (bposIn<GWEN_Io_Request_GetBufferSize(rIn) &&
	  (flagsIn & GWEN_IO_REQUEST_FLAGS_READALL)) {
	/* fixed size has been given but can't be fulfilled, so return an error here */
	xio->readRequestIn=NULL;
	DBG_INFO(GWEN_LOGDOMAIN,
		 "Aborting in read request (reason: %d)",
		 GWEN_ERROR_BAD_SIZE);
	GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, GWEN_ERROR_BAD_SIZE);
	GWEN_Io_Request_free(rIn);
	rIn=NULL;
      }
      else {
	/* request finished */
	xio->readRequestIn=NULL;
	GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, 0);
	GWEN_Io_Request_free(rIn);
	rIn=NULL;
      }
      xio->readMode=GWEN_Io_LayerHttp_Mode_Idle;
      doneSomething=1;
    } /* if finished */
    else if (xio->readMode==GWEN_Io_LayerHttp_Mode_ChunkSize) {
      /* don't enqueue a new request here since we
       * just changed into readChunkSize mode */
    }
    else {
      uint32_t len;
      uint32_t bposIn;

      bposIn=GWEN_Io_Request_GetBufferPos(rIn);
      len=GWEN_Io_Request_GetBufferSize(rIn)-bposIn;
      if (len==0) {
	/* inbound read request fullfilled */
	xio->readRequestIn=NULL;
	GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, 0);
	GWEN_Io_Request_free(rIn);
	rIn=NULL;
	doneSomething=1;
      } /* if request fullfilled */
      else {
	if (xio->readRequestOut==NULL) {
	  if (xio->lastReadOutResult) {
	    xio->readRequestIn=NULL;
	    DBG_INFO(GWEN_LOGDOMAIN,
		     "Aborting in read request (reason: %d)",
		     xio->lastReadOutResult);
	    GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, xio->lastReadOutResult);
	    GWEN_Io_Request_free(rIn);
	    rIn=NULL;
	    doneSomething=1;
	  }
	  else {
	    if (xio->readMode==GWEN_Io_LayerHttp_Mode_Chunk &&
		xio->currentReadChunkSize!=-1 &&
		len>xio->currentReadChunkSize)
	      len=xio->currentReadChunkSize;
	    else if (xio->readMode==GWEN_Io_LayerHttp_Mode_Body &&
		     xio->currentReadBodySize!=-1 &&
		     len>xio->currentReadBodySize)
	      len=xio->currentReadBodySize;

	    if (len) {
	      GWEN_IO_REQUEST *rOut;
	      int rv;

	      DBG_VERBOUS(GWEN_LOGDOMAIN, "Reading %d body bytes (already have %d/%d)",
			  len, bposIn, GWEN_Io_Request_GetBufferSize(rIn));
	      rOut=GWEN_Io_Request_new(GWEN_Io_Request_TypeRead,
				       GWEN_Io_Request_GetBufferPtr(rIn)+bposIn, len,
				       NULL, NULL,
				       GWEN_Io_Request_GetGuiId(rIn));
	      GWEN_Io_Request_AddFlags(rOut, GWEN_IO_REQUEST_BUFFERED_FLAGS_RAW);
	      rv=GWEN_Io_Layer_AddRequest(GWEN_Io_Layer_GetBaseLayer(io), rOut);
	      if (rv) {
		GWEN_Io_Request_free(rOut);
		if (rv!=GWEN_ERROR_TRY_AGAIN) {
		  xio->lastReadOutResult=rv;
		  xio->readRequestIn=NULL;
		  DBG_INFO(GWEN_LOGDOMAIN,
			   "Aborting in read request (reason: %d)",
			   xio->lastReadOutResult);
		  GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, rv);
		  GWEN_Io_Request_free(rIn);
		  rIn=NULL;
		  doneSomething=1;
		}
	      }
	      else {
		xio->readRequestOut=rOut;
		doneSomething=1;
	      }
	    } /* no bytes to read */
	    else {
	      if (xio->currentReadBodySize==0)
		xio->readMode=GWEN_Io_LayerHttp_Mode_Finished;
	      if (xio->currentReadChunkSize==0) {
		xio->currentReadChunkSize=-1;
		xio->readMode=GWEN_Io_LayerHttp_Mode_ChunkSize;
	      }
	    }
	  }
	} /* if there is no outbound read request */
      } /* if inbound request not yet fullfilled */
    } /* if not finished */
  } /* if there is an inbound read request */

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnReadRequest(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "GWEN_Io_LayerHttp_WorkOnReadRequest (%d)", xio->readMode);

  switch(xio->readMode) {
  case GWEN_Io_LayerHttp_Mode_Command:
  case GWEN_Io_LayerHttp_Mode_Status:
  case GWEN_Io_LayerHttp_Mode_Header:
  case GWEN_Io_LayerHttp_Mode_ChunkSize:
    return GWEN_Io_LayerHttp_WorkOnReadRequest1(io);

  case GWEN_Io_LayerHttp_Mode_Chunk:
  case GWEN_Io_LayerHttp_Mode_Body:
  case GWEN_Io_LayerHttp_Mode_Finished:
  default:
    return GWEN_Io_LayerHttp_WorkOnReadRequest2(io);
  }
}



/* writing command/status/header */
GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnWriteRequest1(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "GWEN_Io_LayerHttp_WorkOnWriteRequest1 (%d)", xio->writeMode);

  if (xio->writeRequestOut) {
    if (GWEN_Io_Request_GetStatus(xio->writeRequestOut)==GWEN_Io_Request_StatusFinished) {
      GWEN_IO_REQUEST *rOut;
      uint32_t bposOut;
      uint32_t flagsOut;

      rOut=xio->writeRequestOut;
      flagsOut=GWEN_Io_Request_GetFlags(rOut);
      bposOut=GWEN_Io_Request_GetBufferPos(rOut);

      DBG_VERBOUS(GWEN_LOGDOMAIN, "Sent %d bytes", bposOut);

      GWEN_Buffer_IncrementPos(xio->writeBuffer, bposOut);
      xio->lastWriteOutResult=GWEN_Io_Request_GetResultCode(xio->writeRequestOut);

      /* delete outbound request */
      GWEN_Io_Request_free(xio->writeRequestOut);
      xio->writeRequestOut=NULL;

      doneSomething=1;
    }
  }

  if (xio->writeRequestIn) {
    GWEN_IO_REQUEST *rIn;

    rIn=xio->writeRequestIn;
    if (GWEN_Buffer_GetBytesLeft(xio->writeBuffer)) {
      if (xio->writeRequestOut==NULL) {
	if (xio->lastWriteOutResult) {
	  xio->writeRequestIn=NULL;
	  DBG_INFO(GWEN_LOGDOMAIN,
		   "Aborting in write request (reason: %d)",
		   xio->lastWriteOutResult);
	  GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, xio->lastWriteOutResult);
	  GWEN_Io_Request_free(rIn);
	  rIn=NULL;
	  doneSomething=1;
	}
	else {
	  GWEN_IO_REQUEST *rOut;
	  int rv;

	  DBG_VERBOUS(GWEN_LOGDOMAIN, "Sending %d header bytes", GWEN_Buffer_GetBytesLeft(xio->writeBuffer));
	  rOut=GWEN_Io_Request_new(GWEN_Io_Request_TypeWrite,
				   (uint8_t*)GWEN_Buffer_GetPosPointer(xio->writeBuffer),
				   GWEN_Buffer_GetBytesLeft(xio->writeBuffer),
				   NULL, NULL,
				   GWEN_Io_Request_GetGuiId(rIn));
	  GWEN_Io_Request_AddFlags(rOut,
				   GWEN_IO_REQUEST_BUFFERED_FLAGS_RAW |
                                   GWEN_IO_REQUEST_FLAGS_FLUSH |
				   GWEN_IO_REQUEST_FLAGS_WRITEALL);
	  rv=GWEN_Io_Layer_AddRequest(GWEN_Io_Layer_GetBaseLayer(io), rOut);
	  if (rv) {
	    GWEN_Io_Request_free(rOut);
	    if (rv!=GWEN_ERROR_TRY_AGAIN) {
	      xio->lastWriteOutResult=rv;
	      xio->writeRequestIn=NULL;
	      DBG_INFO(GWEN_LOGDOMAIN,
		       "Aborting in write request (reason: %d)",
		       xio->lastWriteOutResult);
	      GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, rv);
	      GWEN_Io_Request_free(rIn);
	      rIn=NULL;
	      doneSomething=1;
	    }
	  }
	  else {
	    xio->writeRequestOut=rOut;
	    doneSomething=1;
	  }
	} /* if lastOutResult was ok */
      } /* if there is no outbound request */
    } /* if there still are bytes in the buffer to write */
    else {
      /* header finished, prepare writing of the body */
      GWEN_Buffer_Reset(xio->writeBuffer);
      if (xio->currentWriteBodySize!=0)
	/* header finished, body follows */
	xio->writeMode=GWEN_Io_LayerHttp_Mode_Body;
      else
        /* no body */
	xio->writeMode=GWEN_Io_LayerHttp_Mode_Finished;
    }
  } /* if there is an inbound request */


  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



/* writing body/ finished*/
GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnWriteRequest2(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "GWEN_Io_LayerHttp_WorkOnWriteRequest2 (%d)", xio->writeMode);

  if (xio->writeRequestOut) {
    if (GWEN_Io_Request_GetStatus(xio->writeRequestOut)==GWEN_Io_Request_StatusFinished) {
      GWEN_IO_REQUEST *rOut;

      doneSomething=1;
      rOut=xio->writeRequestOut;
      if (xio->writeRequestIn) {
	uint32_t bposOut;
	uint32_t flagsOut;
	GWEN_IO_REQUEST *rIn;
	uint32_t bposIn;
	uint32_t flagsIn;

	rOut=xio->writeRequestOut;
	flagsOut=GWEN_Io_Request_GetFlags(rOut);
	bposOut=GWEN_Io_Request_GetBufferPos(rOut);

	rIn=xio->writeRequestIn;
	flagsIn=GWEN_Io_Request_GetFlags(rIn);
	bposIn=GWEN_Io_Request_GetBufferPos(rIn)+bposOut;
	GWEN_Io_Request_SetBufferPos(rIn, bposIn);

	DBG_VERBOUS(GWEN_LOGDOMAIN, "Sent %d body bytes (%d)",
		    bposOut, GWEN_Io_Request_GetResultCode(rOut));

        /* get last result */
	xio->lastWriteOutResult=GWEN_Io_Request_GetResultCode(rOut);

	/* delete outbound request */
	GWEN_Io_Request_free(xio->writeRequestOut);
	xio->writeRequestOut=NULL;

	if (xio->currentWriteBodySize!=-1)
	  xio->currentWriteBodySize-=bposOut;

	/* check whether the body has been finished */
	if (xio->currentWriteBodySize==0) {
	  /* body finished (in any mode) */
	  GWEN_Io_Request_AddFlags(rIn, GWEN_IO_REQUEST_FLAGS_PACKETEND);
	  xio->writeMode=GWEN_Io_LayerHttp_Mode_Finished;
	  if (bposIn<GWEN_Io_Request_GetBufferSize(rIn) &&
	      (flagsIn & GWEN_IO_REQUEST_FLAGS_WRITEALL)) {
	    /* body finished, but caller insists on writing more data, abort */
	    xio->writeRequestIn=NULL;
	    DBG_INFO(GWEN_LOGDOMAIN,
		     "Aborting in write request (reason: %d)",
		     GWEN_ERROR_BAD_SIZE);
	    GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, GWEN_ERROR_BAD_SIZE);
	    GWEN_Io_Request_free(rIn);
            rIn=NULL;
	  }
	}
	else {
	  /* body not complete (or unknown) */
	  if (bposIn>=GWEN_Io_Request_GetBufferSize(rIn) ||
	      (bposIn && !(flagsIn & GWEN_IO_REQUEST_FLAGS_WRITEALL))) {
	    /* inbound request fullfilled */
	    xio->writeRequestIn=NULL;
	    GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, 0);
	    GWEN_Io_Request_free(rIn);
	    rIn=NULL;
	  }
	}
      }
    }
  }

  if (xio->writeRequestIn) {
    GWEN_IO_REQUEST *rIn;

    rIn=xio->writeRequestIn;
    if (xio->writeRequestOut==NULL) {
      if (xio->lastWriteOutResult) {
	xio->writeRequestIn=NULL;
	DBG_INFO(GWEN_LOGDOMAIN,
		 "Aborting in write request (reason: %d)",
		 xio->lastWriteOutResult);
	GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, xio->lastWriteOutResult);
	GWEN_Io_Request_free(rIn);
	rIn=NULL;
	doneSomething=1;
      }
      else {
	int len;
        int bpos;

	/* create new write request for remaining bytes */
        bpos=GWEN_Io_Request_GetBufferPos(rIn);
	len=GWEN_Io_Request_GetBufferSize(rIn)-bpos;
	if (xio->currentWriteBodySize!=-1 && len>xio->currentWriteBodySize)
	  /* adapt number of bytes to write */
	  len=xio->currentWriteBodySize;

	if (len==0) {
	  if (xio->currentWriteBodySize==0)
	    GWEN_Io_Request_AddFlags(rIn, GWEN_IO_REQUEST_FLAGS_PACKETEND);
          xio->writeRequestIn=NULL;
	  GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, 0);
	  GWEN_Io_Request_free(rIn);
	  rIn=NULL;
	  doneSomething=1;
	}
	else {
	  GWEN_IO_REQUEST *rOut;
	  int rv;

	  /* create new write request for remaining bytes */
	  DBG_VERBOUS(GWEN_LOGDOMAIN, "Sending %d body bytes", len);
	  rOut=GWEN_Io_Request_new(GWEN_Io_Request_TypeWrite,
				   GWEN_Io_Request_GetBufferPtr(rIn)+bpos, len,
				   NULL, NULL,
				   GWEN_Io_Request_GetGuiId(rIn));
	  GWEN_Io_Request_AddFlags(rOut,
				   GWEN_IO_REQUEST_BUFFERED_FLAGS_RAW |
				   GWEN_IO_REQUEST_FLAGS_FLUSH |
				   GWEN_IO_REQUEST_FLAGS_WRITEALL);
	  rv=GWEN_Io_Layer_AddRequest(GWEN_Io_Layer_GetBaseLayer(io), rOut);
	  if (rv) {
	    GWEN_Io_Request_free(rOut);
	    if (rv!=GWEN_ERROR_TRY_AGAIN) {
	      xio->lastWriteOutResult=rv;
	      xio->writeRequestIn=NULL;
	      DBG_INFO(GWEN_LOGDOMAIN,
		       "Aborting in write request (reason: %d)",
		       xio->lastWriteOutResult);
	      GWEN_Io_Request_Finished(rIn, GWEN_Io_Request_StatusFinished, rv);
	      GWEN_Io_Request_free(rIn);
	      rIn=NULL;
	      doneSomething=1;
	    }
	  }
	  else {
	    xio->writeRequestOut=rOut;
	    doneSomething=1;
	  }
	}
      } /* if lastOutResult was ok */
    }


  } /* if there is an inbound request */


  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnWriteRequest(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  DBG_VERBOUS(GWEN_LOGDOMAIN, "GWEN_Io_LayerHttp_WorkOnWriteRequest (%d)", xio->writeMode);

  switch(xio->writeMode) {
  case GWEN_Io_LayerHttp_Mode_Command:
  case GWEN_Io_LayerHttp_Mode_Status:
  case GWEN_Io_LayerHttp_Mode_Header:
  case GWEN_Io_LayerHttp_Mode_ChunkSize:
    return GWEN_Io_LayerHttp_WorkOnWriteRequest1(io);

  case GWEN_Io_LayerHttp_Mode_Chunk:
  case GWEN_Io_LayerHttp_Mode_Body:
  case GWEN_Io_LayerHttp_Mode_Finished:
  default:
    return GWEN_Io_LayerHttp_WorkOnWriteRequest2(io);
  }
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_LayerHttp_WorkOnRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;
  int doneSomething=0;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  /* work on read request */
  if (GWEN_Io_LayerHttp_WorkOnReadRequest(io)!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;

  /* work on write request */
  if (GWEN_Io_LayerHttp_WorkOnWriteRequest(io)!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;

  /* let base layer work */
  if (GWEN_Io_Layer_WorkOnRequests(GWEN_Io_Layer_GetBaseLayer(io))!=GWEN_Io_Layer_WorkResultBlocking)
    doneSomething=1;

  if (GWEN_Io_Layer_GetStatus(io)==GWEN_Io_Layer_StatusListening) {
    GWEN_IO_LAYER *newIo;

    newIo=GWEN_Io_Layer_GetNextIncomingLayer(GWEN_Io_Layer_GetBaseLayer(io));
    if (newIo) {
      GWEN_IO_LAYER *newNewIo;
      GWEN_DB_NODE *dbSrc;
      GWEN_DB_NODE *dbDst;
      uint32_t fl;

      fl=GWEN_Io_Layer_GetFlags(io);
      newNewIo=GWEN_Io_LayerHttp_new(newIo);
      GWEN_Io_Layer_AddFlags(newNewIo, GWEN_IO_LAYER_FLAGS_PASSIVE);
      GWEN_Io_Layer_AddFlags(newNewIo, fl & 0xffff);
      /* copy command */
      dbDst=GWEN_Io_LayerHttp_GetDbCommandOut(newNewIo);
      dbSrc=xio->dbCommandOut;
      GWEN_DB_AddGroupChildren(dbDst, dbSrc);

      GWEN_Io_Layer_AddIncomingLayer(io, newNewIo);
      doneSomething=1;
    }
  }

  return (doneSomething==0)?GWEN_Io_Layer_WorkResultBlocking:GWEN_Io_Layer_WorkResultOk;
}




int GWEN_Io_LayerHttp_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_HTTP *xio;
  GWEN_IO_LAYER_STATUS st;
  uint32_t lflags;
  uint32_t rflags;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  st=GWEN_Io_Layer_GetStatus(io);
  lflags=GWEN_Io_Layer_GetFlags(io);
  rflags=GWEN_Io_Request_GetFlags(r);

  switch(GWEN_Io_Request_GetType(r)) {
  case GWEN_Io_Request_TypeRead:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "Layer not connected (%d)", st);
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }

    if (xio->lastReadOutResult) {
      DBG_INFO(GWEN_LOGDOMAIN, "Unable to read (%d)", xio->lastReadOutResult);
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, xio->lastReadOutResult);
      return xio->lastReadOutResult;
    }

    /* check whether we already have a read request */
    if (xio->readRequestIn) {
      DBG_INFO(GWEN_LOGDOMAIN, "There already is a read request");
      return GWEN_ERROR_TRY_AGAIN;
    }

    if (rflags & GWEN_IO_REQUEST_FLAGS_PACKETBEGIN) {
      if (xio->readMode!=GWEN_Io_LayerHttp_Mode_Idle) {
	DBG_INFO(GWEN_LOGDOMAIN, "Packetbegin requested while there is still a packet in work");
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_INVALID);
	return GWEN_ERROR_INVALID;
      }

      GWEN_Buffer_Reset(xio->readBuffer);
      GWEN_DB_ClearGroup(xio->dbCommandIn, NULL);
      GWEN_DB_ClearGroup(xio->dbStatusIn, NULL);
      GWEN_DB_ClearGroup(xio->dbHeaderIn, NULL);
      xio->currentReadChunkSize=-1;
      xio->currentReadBodySize=-1;
      xio->readLineFinished=0;
      if ((lflags & GWEN_IO_LAYER_HTTP_FLAGS_IPC) ||
	  (lflags & GWEN_IO_LAYER_FLAGS_PASSIVE))
	xio->readMode=GWEN_Io_LayerHttp_Mode_Command;
      else
	xio->readMode=GWEN_Io_LayerHttp_Mode_Status;
    }
    else {
      if (xio->readMode!=GWEN_Io_LayerHttp_Mode_ChunkSize &&
	  xio->readMode!=GWEN_Io_LayerHttp_Mode_Chunk &&
	  xio->readMode!=GWEN_Io_LayerHttp_Mode_Body) {
	DBG_INFO(GWEN_LOGDOMAIN, "Bad read mode and flag GWEN_IO_REQUEST_FLAGS_PACKETBEGIN not set");
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_INVALID);
	return GWEN_ERROR_INVALID;
      }
    }

    /* enqueue request */
    xio->readRequestIn=r;
    GWEN_Io_Request_Attach(xio->readRequestIn);
    break;

  case GWEN_Io_Request_TypeWrite:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "Layer is not connected");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }

    /* check whether we already have a read request */
    if (xio->writeRequestIn) {
      DBG_INFO(GWEN_LOGDOMAIN, "There already is a write request");
      return GWEN_ERROR_TRY_AGAIN;
    }

    if (rflags & GWEN_IO_REQUEST_FLAGS_PACKETBEGIN) {
      int rv;

      if (xio->writeMode!=GWEN_Io_LayerHttp_Mode_Idle && xio->writeMode!=GWEN_Io_LayerHttp_Mode_Finished) {
	DBG_INFO(GWEN_LOGDOMAIN, "Packetbegin requested while there is still a packet in work");
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_INVALID);
	return GWEN_ERROR_INVALID;
      }

      DBG_VERBOUS(GWEN_LOGDOMAIN, "Packet begin");
      GWEN_Buffer_Reset(xio->writeBuffer);
      /* Write command/status */
      if ((lflags & GWEN_IO_LAYER_HTTP_FLAGS_IPC) ||
	  !(lflags & GWEN_IO_LAYER_FLAGS_PASSIVE)) {
	rv=GWEN_Io_LayerHttp_WriteCommand(io, xio->writeBuffer);
	xio->writeMode=GWEN_Io_LayerHttp_Mode_Command;
      }
      else {
	rv=GWEN_Io_LayerHttp_WriteStatus(io, xio->writeBuffer);
	xio->writeMode=GWEN_Io_LayerHttp_Mode_Status;
      }
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, rv);
	xio->writeMode=GWEN_Io_LayerHttp_Mode_Idle;
	return rv;
      }

      if (rflags & GWEN_IO_REQUEST_FLAGS_PACKETEND)
	/* both flags packet begin and end are given, so we already know the content-length */
	GWEN_DB_SetIntValue(xio->dbHeaderOut, GWEN_DB_FLAGS_OVERWRITE_VARS, "Content-length",
			    GWEN_Io_Request_GetBufferSize(r));

      /* write header into buffer, get body size */
      rv=GWEN_Io_LayerHttp_WriteHeader(io, xio->writeBuffer);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, rv);
	xio->writeMode=GWEN_Io_LayerHttp_Mode_Idle;
	return rv;
      }
      GWEN_Buffer_Rewind(xio->writeBuffer);
      xio->writeMode=GWEN_Io_LayerHttp_Mode_Header;
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Buffer: %d/%d",
		  GWEN_Buffer_GetBytesLeft(xio->writeBuffer),
		  GWEN_Buffer_GetUsedBytes(xio->writeBuffer));
    }
    else {
      if (xio->writeMode!=GWEN_Io_LayerHttp_Mode_Body) {
	if ((GWEN_Io_Request_GetBufferSize(r)==0) &&
	    (rflags & GWEN_IO_REQUEST_FLAGS_FLUSH)) {
	  /* this is just a flush request */
	  GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
	  return 0;
	}
	else {
	  DBG_INFO(GWEN_LOGDOMAIN, "Bad write mode and flag GWEN_IO_REQUEST_FLAGS_PACKETBEGIN not set");
	  GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_INVALID);
	  return GWEN_ERROR_INVALID;
	}
      }
    }

    /* enqueue request */
    xio->writeRequestIn=r;
    GWEN_Io_Request_Attach(xio->writeRequestIn);
    break;

  case GWEN_Io_Request_TypeConnect:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusUnconnected &&
	st!=GWEN_Io_Layer_StatusDisconnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "Layer not un-/disconnected");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }
    else {
      /* reset all structures */
      xio->lastReadOutResult=0;
      GWEN_Buffer_Reset(xio->readBuffer);
      xio->readMode=GWEN_Io_LayerHttp_Mode_Idle;
      xio->currentReadChunkSize=-1;
      xio->currentReadBodySize=-1;
      xio->readLineFinished=0;

      xio->lastWriteOutResult=0;
      GWEN_Buffer_Reset(xio->writeBuffer);
      xio->writeMode=GWEN_Io_LayerHttp_Mode_Idle;

      GWEN_DB_ClearGroup(xio->dbCommandIn, NULL);
      GWEN_DB_ClearGroup(xio->dbStatusIn, NULL);
      GWEN_DB_ClearGroup(xio->dbHeaderIn, NULL);

      GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusConnected);
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
      DBG_INFO(GWEN_LOGDOMAIN, "Layer now connected");
    }
    break;

  case GWEN_Io_Request_TypeDisconnect:
    /* check status */
    if (st!=GWEN_Io_Layer_StatusConnected) {
      DBG_INFO(GWEN_LOGDOMAIN, "IO layer is not open");
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_OPEN);
      return GWEN_ERROR_NOT_OPEN;
    }
    else {
      /* abort incoming requests, if any */
      GWEN_Io_LayerHttp_AbortInRequests(io, GWEN_ERROR_ABORTED);
      GWEN_Io_LayerHttp_AbortOutRequests(io);

      /* closed */
      GWEN_Io_Layer_SetStatus(io, GWEN_Io_Layer_StatusDisconnected);
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, 0);
    }
    break;

  default:
    DBG_INFO(GWEN_LOGDOMAIN, "This request type is not supported (%d)", GWEN_Io_Request_GetType(r));
    GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_NOT_SUPPORTED);
    return GWEN_ERROR_NOT_SUPPORTED;
  }

  return 0;
}



int GWEN_Io_LayerHttp_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r) {
  GWEN_IO_LAYER_HTTP *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  switch(GWEN_Io_Request_GetType(r)) {
  case GWEN_Io_Request_TypeRead:
    if (xio->readRequestIn==r) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Aborted read request");
      xio->readRequestIn=NULL;
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_ABORTED);
      GWEN_Io_Request_free(r);
    }
    else {
      /* not my request */
      DBG_INFO(GWEN_LOGDOMAIN, "Read request not registered with this io layer");
      return GWEN_ERROR_INVALID;
    }
    break;

  case GWEN_Io_Request_TypeWrite:
    if (xio->writeRequestIn==r) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Aborted write request");
      xio->writeRequestIn=NULL;
      GWEN_Io_Request_Finished(r, GWEN_Io_Request_StatusFinished, GWEN_ERROR_ABORTED);
      GWEN_Io_Request_free(r);
    }
    else {
      /* not my request */
      DBG_INFO(GWEN_LOGDOMAIN, "Write request not registered with this io layer");
      return GWEN_ERROR_INVALID;
    }
    break;

  default:
    break;
  }

  return 0;
}



int GWEN_Io_LayerHttp_HasWaitingRequests(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER_HTTP *xio;

  assert(io);
  xio=GWEN_INHERIT_GETDATA(GWEN_IO_LAYER, GWEN_IO_LAYER_HTTP, io);
  assert(xio);

  if (xio->readRequestIn || xio->writeRequestIn)
    return 1;
  else
    return 0;
}






