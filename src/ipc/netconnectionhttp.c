/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#define GWEN_EXTEND_NETCONNECTION

#include "netconnectionhttp_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


GWEN_INHERIT(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP);


/* -------------------------------------------------------------- FUNCTION */
GWEN_NETCONNECTION *GWEN_NetConnectionHTTP_new(GWEN_NETTRANSPORT *tr,
                                               int take,
					       GWEN_TYPE_UINT32 libId,
					       int pmajor,
					       int pminor){
  GWEN_NETCONNECTION *conn;
  GWEN_NETCONNECTIONHTTP *chttp;

  conn=GWEN_NetConnection_new(tr, take, libId);
  assert(conn);
  GWEN_NEW_OBJECT(GWEN_NETCONNECTIONHTTP, chttp);
  GWEN_INHERIT_SETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP,
                       conn, chttp,
                       GWEN_NetConnectionHTTP_FreeData);
  chttp->inMode=GWEN_NetConnHttpMsgModeCmd;
  chttp->outMode=GWEN_NetConnHttpMsgModeCmd;
  GWEN_NetConnection_SetWorkFn(conn, GWEN_NetConnectionHTTP_Work);

  chttp->state=0;
  chttp->mode=GWEN_NETCONN_MODE_DEFAULT;
  if (pmajor==0 && pminor==0) {
    pmajor=GWEN_NETCONN_PMAJOR_DEFAULT;
    pminor=GWEN_NETCONN_PMINOR_DEFAULT;
  }
  chttp->pmajor=pmajor;
  chttp->pminor=pminor;
  return conn;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnectionHTTP_FreeData(void *bp, void *p){
  GWEN_NETCONNECTIONHTTP *chttp;
  GWEN_NETCONNECTION *conn;

  conn=(GWEN_NETCONNECTION*)bp;
  assert(conn);
  chttp=(GWEN_NETCONNECTIONHTTP *)p;
  assert(chttp);
  GWEN_NetMsg_free(chttp->currentInMsg);
  GWEN_NetMsg_free(chttp->currentOutMsg);
  free(chttp);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnectionHTTP_ParseCommand(const char *buffer,
					GWEN_DB_NODE *db) {
  GWEN_BUFFER *tbuf;
  int i;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* read command */
  while(*buffer && *buffer!=' ') {
    GWEN_Buffer_AppendByte(tbuf, *(buffer++));
  } /* while */

  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "cmd", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  if (*buffer!=' ') {
    DBG_ERROR(0, "Bad format of HTTP request (missing space after cmd)");
    GWEN_Buffer_free(tbuf);
    return -1;
  }
  buffer++;

  /* read URL */
  while(*buffer && *buffer!=' ' && *buffer!='?') {
    GWEN_Buffer_AppendByte(tbuf, *(buffer++));
  } /* while */

  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "url", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  if (*buffer=='?') {
    GWEN_BUFFER *nbuf;
    GWEN_BUFFER *vbuf;

    /* some variables follow */
    buffer++;
    if (*buffer) {
      GWEN_DB_NODE *dbVars;

      dbVars=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "vars");
      nbuf=GWEN_Buffer_new(0, 64, 0, 1);
      vbuf=GWEN_Buffer_new(0, 64, 0, 1);

      while(*buffer && *buffer!=' ') {
	GWEN_BUFFER *t1, *t2;

	/* read name */
	GWEN_Buffer_Reset(nbuf);
	GWEN_Buffer_Reset(vbuf);
	while(*buffer && *buffer!='=' && *buffer!='&' && *buffer!=' ')
	  GWEN_Buffer_AppendByte(nbuf, *(buffer++));

	/* read value */
	if (*buffer=='=') {
	  buffer++;
	  while(*buffer && *buffer!=' ' && *buffer!='&')
	    GWEN_Buffer_AppendByte(nbuf, *(buffer++));
	}

	/* store variable/value pair in DB */
	t1=GWEN_Buffer_new(0, 64, 0, 1);
	t2=GWEN_Buffer_new(0, 64, 0, 1);
	if (GWEN_NetConnectionHTTP_Unescape(GWEN_Buffer_GetStart(nbuf),
					    t1) ||
	    GWEN_NetConnectionHTTP_Unescape(GWEN_Buffer_GetStart(vbuf),
					    t2)){
	  DBG_ERROR(0, "Bad HTTP request (badly escaped vars)");
	  GWEN_Buffer_free(t2);
	  GWEN_Buffer_free(t1);
	  GWEN_Buffer_free(vbuf);
	  GWEN_Buffer_free(nbuf);
	  return -1;
	}
	GWEN_DB_SetCharValue(dbVars,
			     GWEN_PATH_FLAGS_CREATE_VAR,
			     GWEN_Buffer_GetStart(t1),
			     GWEN_Buffer_GetStart(t2));
	GWEN_Buffer_free(t2);
	GWEN_Buffer_free(t1);
	if (*buffer=='&')
	  buffer++;

      } /* while buffer */
      GWEN_Buffer_free(vbuf);
      GWEN_Buffer_free(nbuf);
    } /* if variables follow after '?' */
  }

  if (*buffer==0) {
    /* no protocol information follows, so we assume HTTP/0.9 */
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "protocol", "HTTP");
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"pmajor", 0);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"pminor", 9);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"hasHeader", 0);
  }
  else {
    if (*buffer!=' ') {
      DBG_ERROR(0, "Bad format of HTTP request (missing space after url)");
      GWEN_Buffer_free(tbuf);
      return -1;
    }
    buffer++;

    /* read protocol */
    while(*buffer && *buffer!='/') {
      GWEN_Buffer_AppendByte(tbuf, *(buffer++));
    } /* while */
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "protocol", GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);

    if (*buffer!='/') {
      DBG_ERROR(0, "Bad format of HTTP request (missing slash after proto)");
      GWEN_Buffer_free(tbuf);
      return -1;
    }
    buffer++;

    /* read major version */
    while(*buffer && *buffer!='.') {
      GWEN_Buffer_AppendByte(tbuf, *(buffer++));
    } /* while */
    if (1!=sscanf(GWEN_Buffer_GetStart(tbuf), "%d", &i)) {
      DBG_ERROR(0, "Bad format of HTTP request (bad major version)");
      GWEN_Buffer_free(tbuf);
      return -1;
    }
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"pmajor", i);
    GWEN_Buffer_Reset(tbuf);
    if (i<1) {
      DBG_ERROR(0, "Bad HTTP request (pmajor given as 0)");
      GWEN_Buffer_free(tbuf);
      return -1;
    }

    if (*buffer!='.') {
      DBG_ERROR(0, "Bad format of HTTP request (missing decimal point)");
      GWEN_Buffer_free(tbuf);
      return -1;
    }
    buffer++;

    /* read minor version */
    if (1!=sscanf(buffer, "%d", &i)) {
      DBG_ERROR(0, "Bad format of HTTP request (bad major version)");
      GWEN_Buffer_free(tbuf);
      return -1;
    }
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"pminor", i);

    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"hasHeader", 1);
  }

  GWEN_Buffer_free(tbuf);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnectionHTTP_ParseStatus(const char *buffer,
				       GWEN_DB_NODE *db) {
  GWEN_BUFFER *tbuf;
  int i;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* read protocol */
  while(*buffer && *buffer!='/') {
    GWEN_Buffer_AppendByte(tbuf, *(buffer++));
  } /* while */
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "protocol", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  if (*buffer!='/') {
    DBG_ERROR(0, "Bad format of HTTP request (missing slash after proto)");
    GWEN_Buffer_free(tbuf);
    return -1;
  }
  buffer++;

  /* read major version */
  while(*buffer && *buffer!='.') {
    GWEN_Buffer_AppendByte(tbuf, *(buffer++));
  } /* while */
  if (1!=sscanf(GWEN_Buffer_GetStart(tbuf), "%d", &i)) {
    DBG_ERROR(0, "Bad format of HTTP request (bad major version)");
    GWEN_Buffer_free(tbuf);
    return -1;
  }
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "pmajor", i);
  GWEN_Buffer_Reset(tbuf);
  if (i<1) {
    DBG_ERROR(0, "Bad HTTP request (pmajor given as 0)");
    GWEN_Buffer_free(tbuf);
    return -1;
  }

  if (*buffer!='.') {
    DBG_ERROR(0, "Bad format of HTTP request (missing decimal point)");
    GWEN_Buffer_free(tbuf);
    return -1;
  }
  buffer++;

  /* read minor version */
  while(*buffer && *buffer!=' ') {
    GWEN_Buffer_AppendByte(tbuf, *(buffer++));
  } /* while */
  if (1!=sscanf(GWEN_Buffer_GetStart(tbuf), "%d", &i)) {
    DBG_ERROR(0, "Bad format of HTTP request (bad minor version)");
    GWEN_Buffer_free(tbuf);
    return -1;
  }
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "pminor", i);
  GWEN_Buffer_Reset(tbuf);

  if (*buffer!=' ') {
    DBG_ERROR(0, "Bad format of HTTP request (missing space after proto)");
    GWEN_Buffer_free(tbuf);
    return -1;
  }
  buffer++;

  /* read status code */
  while(*buffer && *buffer!=' ') {
    GWEN_Buffer_AppendByte(tbuf, *(buffer++));
  } /* while */
  if (1!=sscanf(GWEN_Buffer_GetStart(tbuf), "%d", &i)) {
    DBG_ERROR(0, "Bad format of HTTP request (bad status code)");
    GWEN_Buffer_free(tbuf);
    return -1;
  }
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "code", i);
  GWEN_Buffer_Reset(tbuf);

  /* read status text (if any) */
  if (*buffer==' ') buffer++;
  if (*buffer)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "text", buffer);

  GWEN_Buffer_free(tbuf);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnectionHTTP_ParseHeader(const char *buffer,
				       GWEN_DB_NODE *db) {
  GWEN_BUFFER *nbuf;
  GWEN_BUFFER *vbuf;

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  vbuf=GWEN_Buffer_new(0, 256, 0, 1);

  while(*buffer) {
    GWEN_Buffer_Reset(nbuf);

    /* read variable name */
    if (*buffer==13) {
      /* empty line, so the header ends here */
      buffer++;
      if (*buffer!=10) {
	DBG_ERROR(0, "Bad format of HTTP header (no LF after CR)");
	GWEN_Buffer_free(nbuf);
	GWEN_Buffer_free(vbuf);
	return -1;
      }
      buffer++;
      break;
    }
    while(*buffer && *buffer!=':') {
      GWEN_Buffer_AppendByte(nbuf, *(buffer++));
    } /* while */
    if (*buffer!=':') {
      DBG_ERROR(0, "Bad format of HTTP header (missing colon after name)");
      GWEN_Buffer_free(nbuf);
      GWEN_Buffer_free(vbuf);
      return -1;
    }
    buffer++;

    /* skip blanks */
    while(*buffer && isspace(*buffer)) buffer++;

    /* read value (filter out CR and stop at LF if not followed by a blank) */
    GWEN_Buffer_Reset(vbuf);
    for (;;) {
      while(*buffer && *buffer!=10) {
	if (*buffer!=13)
	  GWEN_Buffer_AppendByte(vbuf, *buffer);
        buffer++;
      } /* while */
      if (*buffer==0)
	break;
      buffer++;
      if (!isspace(*buffer))
	break;
      buffer++;
    } /* for */

    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 GWEN_Buffer_GetStart(nbuf),
			 GWEN_Buffer_GetStart(vbuf));
  } /* while */

  GWEN_Buffer_free(nbuf);
  GWEN_Buffer_free(vbuf);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnectionHTTP_WriteBody(GWEN_NETCONNECTION *conn,
				     const char *buffer,
				     GWEN_TYPE_UINT32 *bsize,
				     GWEN_NETCONNHTTP_WRITEBODY_MODE m){
  GWEN_NETCONNECTIONHTTP *chttp;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  if (chttp->writeBodyFn)
    return chttp->writeBodyFn(conn, buffer, bsize, m);
  else {
    if (m==GWEN_NetConnHttp_WriteBodyModeWrite && *bsize) {
      GWEN_Buffer_AppendBytes(GWEN_NetMsg_GetBuffer(chttp->currentInMsg),
			      buffer, *bsize);
    }
    return 0;
  }
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETCONNECTION_WORKRESULT
GWEN_NetConnectionHTTP_ReadWork(GWEN_NETCONNECTION *conn){
  GWEN_BUFFER *mbuf;
  GWEN_RINGBUFFER *rbuf;
  GWEN_NETCONNECTIONHTTP *chttp;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  if (!chttp->currentInMsg) {
    chttp->currentInMsg=GWEN_NetMsg_new(GWEN_NETCONNHTTP_MSGBUFSIZE);
    chttp->bodyBytesRead=0;
    chttp->headerPos=0;
    chttp->inLast4Bytes=0;
    /* set state according to mode */
    chttp->state=0;
    if (chttp->mode & GWEN_NETCONN_MODE_WAITBEFOREREAD)
      chttp->state|=GWEN_NETCONNHTTP_STATE_STOPREAD;

    if (chttp->mode & GWEN_NETCONN_MODE_WAITBEFOREBODYREAD)
      chttp->state|=GWEN_NETCONNHTTP_STATE_STOPBODYREAD;

    if (GWEN_NetConnection_GetFlags(conn) & GWEN_NETTRANSPORT_FLAGS_PASSIVE ||
	chttp->mode & GWEN_NETCONN_MODE_IPC){
      /* passive, so this is HTTP a server */
      chttp->inMode=GWEN_NetConnHttpMsgModeCmd;
    } /* if server */
    else {
      /* active, so this is a client */
      if (chttp->pmajor>0)
	/* newer protocol version, expect a status line and a header */
	chttp->inMode=GWEN_NetConnHttpMsgModeStatus;
      else
        /* old HTTP version, expect body */
	chttp->inMode=GWEN_NetConnHttpMsgModeBody;
    } /* if client */
  }

  if (chttp->state & GWEN_NETCONNHTTP_STATE_STOPREAD) {
    DBG_DEBUG(0, "StopRead state set, nothing to do");
    chttp->state|=GWEN_NETCONNHTTP_STATE_WAITINGFORREAD;
    return GWEN_NetConnectionWorkResult_NoChange;
  }

  /* just starting to read ? */
  if (!(chttp->state & GWEN_NETCONNHTTP_STATE_STARTEDREAD)) {
    /* only set state, nothing more for now */
    chttp->state|=GWEN_NETCONNHTTP_STATE_STARTEDREAD;
  }

  mbuf=GWEN_NetMsg_GetBuffer(chttp->currentInMsg);
  rbuf=GWEN_NetConnection_GetReadBuffer(conn);

  if (chttp->inMode==GWEN_NetConnHttpMsgModeBody) {
    /* read body */
    GWEN_TYPE_UINT32 size, i = 0, j, l;

    if (chttp->state & GWEN_NETCONNHTTP_STATE_STOPBODYREAD) {
      DBG_DEBUG(0, "StopBodyRead state set, nothing to do");
      chttp->state|=GWEN_NETCONNHTTP_STATE_WAITINGFORBODYREAD;
      return GWEN_NetConnectionWorkResult_NoChange;
    }
    size=GWEN_NetMsg_GetSize(chttp->currentInMsg);

    if (!(chttp->state & GWEN_NETCONNHTTP_STATE_STARTEDBODYREAD)){
      if (GWEN_NetConnectionHTTP_WriteBody
	  (conn, 0, 0, GWEN_NetConnHttp_WriteBodyModeStart)){
	DBG_ERROR(0, "Error storing body (START)");
	GWEN_NetMsg_free(chttp->currentInMsg);
	chttp->currentInMsg=0;
	return GWEN_NetConnectionWorkResult_Error;
      }
      chttp->state|=GWEN_NETCONNHTTP_STATE_STARTEDBODYREAD;
    }

    /* read rest of the message */
    while (size==0 || (i=chttp->bodyBytesRead)<size) {
      GWEN_TYPE_UINT32 lbak;

      if (size && i>=size)
	break;

      DBG_DEBUG(0, "Reading rest of the message... (%d bytes)", size);
      j=GWEN_RingBuffer_GetMaxUnsegmentedRead(rbuf);
      if (!j) {
	/* read buffer empty, check whether the connection has been closed */
	if (GWEN_NetConnection_GetStatus(conn)!=
	    GWEN_NetTransportStatusLConnected) {
	  /* not logically connected, assume EOF */
	  if (size!=0) {
	    /* exact size given, so this EOF is premature */
	    DBG_ERROR(0, "EOF met prematurely");
	    GWEN_NetConnectionHTTP_WriteBody
	      (conn, 0, 0, GWEN_NetConnHttp_WriteBodyModeAbort);
	    GWEN_NetMsg_free(chttp->currentInMsg);
	    chttp->currentInMsg=0;
	    return GWEN_NetConnectionWorkResult_Error;
	  }
	  /* message complete, break */
          DBG_INFO(0, "Message finished");
	  break;
	} /* if not connected */
	DBG_DEBUG(0, "Readbuffer empty");
	return GWEN_NetConnectionWorkResult_NoChange;
      }

      l=size-i;
      if (l>j)
	l=j;

      lbak=l;
      if (GWEN_NetConnectionHTTP_WriteBody
	  (conn,
	   GWEN_RingBuffer_GetReadPointer(rbuf),
	   &l, GWEN_NetConnHttp_WriteBodyModeWrite)) {
	DBG_ERROR(0, "Error storing body (1)");
	GWEN_NetMsg_free(chttp->currentInMsg);
	chttp->currentInMsg=0;
	return GWEN_NetConnectionWorkResult_Error;
      }
      if (l) {
	DBG_DEBUG(0, "%d bytes read (total: %d)", l, l+chttp->bodyBytesRead);
	GWEN_RingBuffer_SkipBytesRead(rbuf, l);
	chttp->bodyBytesRead+=l;
      }
      if (l<lbak) {
	DBG_DEBUG(0, "Writebuffer full");
	return GWEN_NetConnectionWorkResult_NoChange;
      }
    } /* while */

    /* tell the writer function about the end of the body */
    if (GWEN_NetConnectionHTTP_WriteBody(conn, 0, 0,
					 GWEN_NetConnHttp_WriteBodyModeStop)){
      DBG_ERROR(0, "Error storing body (STOP)");
      GWEN_NetMsg_free(chttp->currentInMsg);
      chttp->currentInMsg=0;
      return GWEN_NetConnectionWorkResult_Error;
    }

    /* append message to connection's queue */
    DBG_INFO(0, "Got a message");
    GWEN_NetConnection_AddInMsg(conn, chttp->currentInMsg);
    chttp->currentInMsg=0;
    return GWEN_NetConnectionWorkResult_Change;
  } /* if in body mode */
  else {
    /* in command or header mode */
    while(GWEN_RingBuffer_GetUsedBytes(rbuf)) {
      int c;

      c=GWEN_RingBuffer_ReadByte(rbuf);
      if (c==-1) {
	return GWEN_NetConnectionWorkResult_NoChange;
      }

      chttp->inLast4Bytes=(chttp->inLast4Bytes<<8)+(c & 0xff);

      if (chttp->inMode==GWEN_NetConnHttpMsgModeCmd) {
        /* read command line */
	DBG_DEBUG(0, "Reading command...");

	GWEN_Buffer_AppendByte(mbuf, c);
	if (c==10) {
	  GWEN_DB_NODE *dbCmd;
	  int pmajor;
          int pminor;

	  /* line feed, so we have the command */

	  /* parse command */
	  dbCmd=GWEN_DB_GetGroup(GWEN_NetMsg_GetDB(chttp->currentInMsg),
				 GWEN_DB_FLAGS_OVERWRITE_GROUPS,
				 "command");
	  assert(dbCmd);
	  if (GWEN_NetConnectionHTTP_ParseCommand(GWEN_Buffer_GetStart(mbuf),
						  dbCmd)) {
	    DBG_ERROR(0, "Error parsing HTTP command");
	    GWEN_NetMsg_free(chttp->currentInMsg);
	    chttp->currentInMsg=0;
	    return GWEN_NetConnectionWorkResult_Error;
	  }
	  /* adjust HTTP version to that used in the command */
	  pmajor=GWEN_DB_GetIntValue(dbCmd,
				     "pmajor", 0,
				     0);
	  pminor=GWEN_DB_GetIntValue(dbCmd,
				     "pminor", 0,
				     9);
	  GWEN_NetMsg_SetProtocolVersion(chttp->currentInMsg, pmajor, pminor);

	  if (GWEN_DB_GetIntValue(dbCmd, "hasHeader", 0, 0)) {
	    /* header is supposed to follow */
            chttp->inMode=GWEN_NetConnHttpMsgModeHeader;
            chttp->headerPos=GWEN_Buffer_GetPos(mbuf);
            GWEN_Buffer_SetBookmark(mbuf, 0, GWEN_Buffer_GetPos(mbuf));
	  }
	  else {
	    const char *cmd;

	    /* no header, body eventually follows */
	    cmd=GWEN_DB_GetCharValue(dbCmd, "cmd", 0, "");
	    if (strcmp(cmd, "PUT")==0 ||
		strcmp(cmd, "POST")) {
	      chttp->inMode=GWEN_NetConnHttpMsgModeBody;
	      if (chttp->mode & GWEN_NETCONN_MODE_WAITBEFOREBODYREAD)
		chttp->state|=GWEN_NETCONNHTTP_STATE_STOPBODYREAD;
	      chttp->headerPos=0;
	    }
	    else {
	      /* no body, message complete */
	      DBG_INFO(0, "Got a message");
	      GWEN_NetConnection_AddInMsg(conn, chttp->currentInMsg);
	      chttp->currentInMsg=0;
	      return GWEN_NetConnectionWorkResult_Change;
	    }
	  }
	  break;
	}
      }
      else if (chttp->inMode==GWEN_NetConnHttpMsgModeStatus) {
	/* read status line */
	DBG_DEBUG(0, "Reading status...");

	GWEN_Buffer_AppendByte(mbuf, c);
	if (c==10) {
	  GWEN_DB_NODE *dbStatus;

	  /* line feed, so we have the command */

	  /* parse command */
	  dbStatus=GWEN_DB_GetGroup(GWEN_NetMsg_GetDB(chttp->currentInMsg),
				    GWEN_DB_FLAGS_OVERWRITE_GROUPS,
				    "status");
	  assert(dbStatus);
	  if (GWEN_NetConnectionHTTP_ParseStatus(GWEN_Buffer_GetStart(mbuf),
						 dbStatus)) {
	    DBG_ERROR(0, "Error parsing HTTP status");
	    GWEN_NetMsg_free(chttp->currentInMsg);
	    chttp->currentInMsg=0;
	    return GWEN_NetConnectionWorkResult_Error;
	  }

	  /* header is supposed to follow */
	  chttp->inMode=GWEN_NetConnHttpMsgModeHeader;
	  chttp->headerPos=GWEN_Buffer_GetPos(mbuf);
          GWEN_Buffer_SetBookmark(mbuf, 0, GWEN_Buffer_GetPos(mbuf));
	  break;
	}
      }
      else if (chttp->inMode==GWEN_NetConnHttpMsgModeHeader) {
	/* read header line */
	DBG_DEBUG(0, "Reading header...");

	GWEN_Buffer_AppendByte(mbuf, c);
	if (c==10) {
	  /* line feed, so we have a line */
	  if (chttp->inLast4Bytes==0x0d0a0d0a){
	    GWEN_DB_NODE *dbHeader;
            int hasBody;
	    const char *p;

	    /* empty line, so the header ends, parse it */
	    dbHeader=GWEN_DB_GetGroup(GWEN_NetMsg_GetDB(chttp->currentInMsg),
				      GWEN_DB_FLAGS_OVERWRITE_GROUPS,
				      "header");
	    assert(dbHeader);
	    p=GWEN_Buffer_GetStart(mbuf)+chttp->headerPos;
	    if (GWEN_NetConnectionHTTP_ParseHeader(p, dbHeader)) {
	      DBG_ERROR(0, "Error parsing HTTP command");
	      GWEN_NetMsg_free(chttp->currentInMsg);
	      chttp->currentInMsg=0;
	      return GWEN_NetConnectionWorkResult_Error;
	    }
            GWEN_Buffer_SetBookmark(mbuf, 1, GWEN_Buffer_GetPos(mbuf));

	    hasBody=0;
	    if ((GWEN_NetConnection_GetFlags(conn) &
		 GWEN_NETTRANSPORT_FLAGS_PASSIVE) ||
		(chttp->mode & GWEN_NETCONN_MODE_IPC)) {
	      p=GWEN_DB_GetCharValue(GWEN_NetMsg_GetDB(chttp->currentInMsg),
				     "command/cmd", 0, "");
	      if (strcmp(p, "PUT")==0 ||
		  strcmp(p, "POST")==0)
		hasBody=1;
	    }
	    else
	      hasBody=1;

	    if (hasBody) {
	      if (GWEN_DB_VariableExists(dbHeader, "content-length")) {
                int size;

		if (1!=sscanf(GWEN_DB_GetCharValue(dbHeader,
                                                   "content-length", 0, ""),
                              "%d", &size)){
                  DBG_ERROR(0, "Error parsing HTTP header (bad size)");
		  GWEN_NetMsg_free(chttp->currentInMsg);
		  chttp->currentInMsg=0;
		  return GWEN_NetConnectionWorkResult_Error;
                }
		if (size>GWEN_NETCONNHTTP_MAXBODYSIZE) {
		  DBG_ERROR(0, "Bad HTTP request (size limit exceeded)");
		  GWEN_NetMsg_free(chttp->currentInMsg);
		  chttp->currentInMsg=0;
		  return GWEN_NetConnectionWorkResult_Error;
		}

                if (!size) {
                  /* no body, message complete */
                  DBG_INFO(0, "Got a message");
                  GWEN_NetConnection_AddInMsg(conn, chttp->currentInMsg);
                  chttp->currentInMsg=0;
                  return GWEN_NetConnectionWorkResult_Change;
                }
                /* set size */
		GWEN_NetMsg_SetSize(chttp->currentInMsg, size);
	      } /* if size given */
	      chttp->inMode=GWEN_NetConnHttpMsgModeBody;
	      if (chttp->mode & GWEN_NETCONN_MODE_WAITBEFOREBODYREAD)
		chttp->state|=GWEN_NETCONNHTTP_STATE_STOPBODYREAD;
	    }
	    else {
	      /* no body, message complete */
	      DBG_INFO(0, "Got a message");
	      GWEN_NetConnection_AddInMsg(conn, chttp->currentInMsg);
	      chttp->currentInMsg=0;
	      return GWEN_NetConnectionWorkResult_Change;
	    }
	  }
	  break;
	}
      }
    } /* while */
  } /* if not in body mode */


  return GWEN_NetConnectionWorkResult_NoChange;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETCONNECTION_WORKRESULT
GWEN_NetConnectionHTTP_WriteWork(GWEN_NETCONNECTION *conn){
  GWEN_BUFFER *mbuf;
  GWEN_NETCONNECTIONHTTP *chttp;
  GWEN_TYPE_UINT32 i = 0, j;
  GWEN_RINGBUFFER *wbuf;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  /* check whether we are currently working on a message */
  if (!chttp->currentOutMsg) {
    /* no, we aren't, get the next from the queue */
    chttp->currentOutMsg=GWEN_NetConnection_GetOutMsg(conn);
    /* still no message in work ? */
    if (!chttp->currentOutMsg) {
      DBG_DEBUG(0, "Nothing to write");
      if (GWEN_NetConnection_GetDownAfterSend(conn)) {
	DBG_DEBUG(0, "Closing connections as instructed");
	if (GWEN_NetConnection_StartDisconnect(conn)) {
	  DBG_INFO(0, "Could not start to disconnect");
	  return GWEN_NetConnectionWorkResult_Error;
	}
	return GWEN_NetConnectionWorkResult_Change;
      }
      return GWEN_NetConnectionWorkResult_NoChange;
    }
    mbuf=GWEN_NetMsg_GetBuffer(chttp->currentOutMsg);
    GWEN_Buffer_Rewind(mbuf);
    chttp->bodyBytesWritten=0;
    chttp->outMode=GWEN_NetConnHttpMsgModeBuffer;
  }
  else
    mbuf=GWEN_NetMsg_GetBuffer(chttp->currentOutMsg);


  wbuf=GWEN_NetConnection_GetWriteBuffer(conn);

  if (chttp->outMode==GWEN_NetConnHttpMsgModeBuffer) {
    /* complete header if necessary */
    while((i=GWEN_Buffer_GetBytesLeft(mbuf))) {
      DBG_DEBUG(0, "Writing message...");
      j=GWEN_RingBuffer_GetMaxUnsegmentedWrite(wbuf);
      if (!j) {
	DBG_DEBUG(0, "Writebuffer full");
	return GWEN_NetConnectionWorkResult_NoChange;
      }
      if (j>i)
	j=i;

      memmove(GWEN_RingBuffer_GetWritePointer(wbuf),
	      GWEN_Buffer_GetPosPointer(mbuf),
	      j);
      GWEN_RingBuffer_SkipBytesWrite(wbuf, j);
      GWEN_Buffer_IncrementPos(mbuf, j);
    } /* while */
    chttp->outMode=GWEN_NetConnHttpMsgModeBio;
  }

  if (chttp->outMode==GWEN_NetConnHttpMsgModeBio) {
    GWEN_BUFFEREDIO *bio;
    unsigned int size;

    bio=GWEN_NetMsg_GetBufferedIO(chttp->currentOutMsg);
    if (bio==0) {
      GWEN_NetMsg_free(chttp->currentOutMsg);
      chttp->currentOutMsg=0;

      DBG_INFO(0, "Message written");
      return GWEN_NetConnectionWorkResult_Change;
    }

    size=GWEN_NetMsg_GetSize(chttp->currentOutMsg);
    while(size==0 || size<chttp->bodyBytesWritten) {
      unsigned int jbak;
      GWEN_ERRORCODE err;

      if (GWEN_BufferedIO_CheckEOF(bio)) {
	/* EOF met */
	if (size) {
	  /* size given, so the end is premature */
	  DBG_ERROR(0, "EOF met prematurely");
	  GWEN_NetMsg_free(chttp->currentOutMsg);
	  chttp->currentOutMsg=0;
	  return GWEN_NetConnectionWorkResult_Error;
	}
        /* otherwise EOF flags the end of the body */
	break;
      } /* if EOF */

      DBG_DEBUG(0, "Writing body...");
      j=GWEN_RingBuffer_GetMaxUnsegmentedWrite(wbuf);
      if (!j) {
	DBG_DEBUG(0, "Writebuffer full");
	return GWEN_NetConnectionWorkResult_NoChange;
      }
      if (j>i)
	j=i;

      jbak=j;
      err=GWEN_BufferedIO_ReadRaw(bio,
				  GWEN_RingBuffer_GetWritePointer(wbuf),
				  &j);
      if (!GWEN_Error_IsOk(err)) {
	DBG_ERROR_ERR(0, err);
	GWEN_NetMsg_free(chttp->currentOutMsg);
	chttp->currentOutMsg=0;
	return GWEN_NetConnectionWorkResult_Error;
      }

      GWEN_RingBuffer_SkipBytesWrite(wbuf, j);
      chttp->bodyBytesWritten+=j;

      if (j<jbak) {
	DBG_DEBUG(0, "Read buffer empty");
	return GWEN_NetConnectionWorkResult_NoChange;
      }
    } /* while */
  } /* if BIO mode (-> if writing the body) */

  /* otherwise message written */
  GWEN_NetMsg_free(chttp->currentOutMsg);
  chttp->currentOutMsg=0;
  return GWEN_NetConnectionWorkResult_Change;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETCONNECTION_WORKRESULT
GWEN_NetConnectionHTTP_Work(GWEN_NETCONNECTION *conn){
  GWEN_NETCONNECTIONHTTP *chttp;
  GWEN_NETCONNECTION_WORKRESULT rv;
  GWEN_NETCONNECTION_WORKRESULT rvBak;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  DBG_DEBUG(0, "Working on HTTP connection");

  rv=GWEN_NetConnectionHTTP_WriteWork(conn);
  if (rv==GWEN_NetConnectionWorkResult_Change) {
    DBG_DEBUG(0, "Change while writing");
    return rv;
  }
  else if (rv==GWEN_NetConnectionWorkResult_Error) {
    DBG_DEBUG(0, "Error on writing");
    return rv;
  }

  rv=GWEN_NetConnection_WorkIO(conn);
  if (rv==GWEN_NetConnectionWorkResult_Change) {
    DBG_DEBUG(0, "Change on WorkIO");
  }
  else if (rv==GWEN_NetConnectionWorkResult_Error) {
    DBG_DEBUG(0, "Error on WorkIO");
  }
  rvBak=rv;

  rv=GWEN_NetConnectionHTTP_ReadWork(conn);
  if (rv!=GWEN_NetConnectionWorkResult_NoChange) {
    DBG_INFO(0, "Change while reading");
    return rv;
  }

  return rvBak;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnectionHTTP_Escape(const char *src, GWEN_BUFFER *buf) {
  while(*src) {
    unsigned char x;

    x=(unsigned char)*src;
    if (!GWEN_NETCONNHTTP_NO_ESCAPE(x)) {
      unsigned char c;

      GWEN_Buffer_AppendByte(buf, '%');
      c=(((unsigned char)(*src))>>4)&0xf;
      if (c>9)
	c+=7;
      c+='0';
      GWEN_Buffer_AppendByte(buf, c);
      c=((unsigned char)(*src))&0xf;
      if (c>9)
	c+=7;
      c+='0';
      GWEN_Buffer_AppendByte(buf, c);
    }
    else {
      GWEN_Buffer_AppendByte(buf, *src);
    }

    src++;
  } /* while */
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnectionHTTP_Unescape(const char *src, GWEN_BUFFER *buf) {
  while(*src) {
    unsigned char x;

    x=(unsigned char)*src;
    if (GWEN_NETCONNHTTP_NO_ESCAPE(x)) {
      GWEN_Buffer_AppendByte(buf, *src);
    }
    else {
      if (*src=='%') {
	unsigned char d1, d2;
	unsigned char c;

	/* skip '%' */
	src++;
	if (!(*src) || !isxdigit(*src)) {
	  DBG_ERROR(0, "Incomplete escape sequence (no digits)");
	  return -1;
	}
	/* read first digit */
	d1=(unsigned char)(toupper(*src));

	/* get second digit */
	src++;
	if (!(*src) || !isxdigit(*src)) {
	  DBG_ERROR(0, "Incomplete escape sequence (only 1 digit)");
	  return -1;
	}
	d2=(unsigned char)(toupper(*src));
	/* compute character */
	d1-='0';
	if (d1>9)
	  d1-=7;
	c=(d1<<4)&0xf0;
	d2-='0';
	if (d2>9)
	  d2-=7;
	c+=(d2&0xf);
	/* store character */
	GWEN_Buffer_AppendByte(buf, (char)c);
      }
      else {
	DBG_ERROR(0, "Found non-alphanum "
		  "characters in escaped string (\"%s\")",
		  src);
	return -1;
      }
    }
    src++;
  } /* while */

  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_NetConnectionHTTP_GetMode(GWEN_NETCONNECTION *conn){
  GWEN_NETCONNECTIONHTTP *chttp;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  return chttp->mode;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnectionHTTP_SetMode(GWEN_NETCONNECTION *conn,
				    GWEN_TYPE_UINT32 m){
  GWEN_NETCONNECTIONHTTP *chttp;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  chttp->mode=m;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnectionHTTP_AddMode(GWEN_NETCONNECTION *conn,
				    GWEN_TYPE_UINT32 m){
  GWEN_NETCONNECTIONHTTP *chttp;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  chttp->mode|=m;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnectionHTTP_SubMode(GWEN_NETCONNECTION *conn,
				    GWEN_TYPE_UINT32 m){
  GWEN_NETCONNECTIONHTTP *chttp;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  chttp->mode&=~m;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnectionHTTP_StartBodyRead(GWEN_NETCONNECTION *conn){
  GWEN_NETCONNECTIONHTTP *chttp;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  chttp->state&=~GWEN_NETCONNHTTP_STATE_STOPBODYREAD;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_NetConnectionHTTP_StartRead(GWEN_NETCONNECTION *conn){
  GWEN_NETCONNECTIONHTTP *chttp;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  chttp->state&=~GWEN_NETCONNHTTP_STATE_STOPREAD;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_NetConnectionHTTP_GetState(GWEN_NETCONNECTION *conn){
  GWEN_NETCONNECTIONHTTP *chttp;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  return chttp->state;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnectionHTTP_WriteCommand(GWEN_NETCONNECTION *conn,
					GWEN_DB_NODE *db,
					GWEN_BUFFER *buf) {
  const char *p;
  int pmajor;
  int pminor;
  GWEN_NETCONNECTIONHTTP *chttp;
  GWEN_DB_NODE *dbVarGrp;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  /* append command */
  p=GWEN_DB_GetCharValue(db, "cmd", 0, 0);
  if (!p) {
    DBG_ERROR(0, "Command missing");
    return -1;
  }
  while(*p) GWEN_Buffer_AppendByte(buf, toupper(*(p++)));

  /* append URL */
  GWEN_Buffer_AppendByte(buf, ' ');
  p=GWEN_DB_GetCharValue(db, "url", 0, 0);
  if (!p) {
    DBG_ERROR(0, "URL missing");
    return -1;
  }
  GWEN_Buffer_AppendString(buf, p);

  /* append variables if any */
  dbVarGrp=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "vars");
  if (dbVarGrp) {
    GWEN_DB_NODE *dbVar;

    dbVar=GWEN_DB_GetFirstVar(dbVarGrp);
    if (dbVar) {
      int first;

      first=1;
      GWEN_Buffer_AppendByte(buf, '?');
      while(dbVar) {
        const char *vname;
        const char *p;

	if (first)
	  first=0;
	else
	  GWEN_Buffer_AppendByte(buf, '&');

        vname=GWEN_DB_VariableName(dbVar);
	GWEN_Buffer_AppendString(buf, vname);
	p=GWEN_DB_GetCharValue(dbVarGrp, vname, 0, 0);
	if (p) {
	  GWEN_Buffer_AppendByte(buf, '=');
	  GWEN_NetConnectionHTTP_Escape(p, buf);
	}
	dbVar=GWEN_DB_GetNextVar(dbVar);
      } /* while */
    }
  }

  /* append protocol version if >=1.0 */
  pmajor=chttp->pmajor;
  pminor=chttp->pminor;
  if (pmajor>0) {
    char numbuf[16];

    GWEN_Buffer_AppendString(buf, " HTTP/");
    snprintf(numbuf, sizeof(numbuf), "%d", pmajor);
    GWEN_Buffer_AppendString(buf, numbuf);
    GWEN_Buffer_AppendByte(buf, '.');
    snprintf(numbuf, sizeof(numbuf), "%d", pminor);
    GWEN_Buffer_AppendString(buf, numbuf);
  }
  GWEN_Buffer_AppendString(buf, "\r\n");
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnectionHTTP_WriteStatus(GWEN_NETCONNECTION *conn,
                                       GWEN_DB_NODE *db,
                                       GWEN_BUFFER *buf) {
  const char *p;
  int pmajor;
  int pminor;
  GWEN_NETCONNECTIONHTTP *chttp;
  int i;
  char numbuf[16];

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  /* append protocol version if >=1.0 */
  pmajor=chttp->pmajor;
  pminor=chttp->pminor;

  GWEN_Buffer_AppendString(buf, " HTTP/");
  snprintf(numbuf, sizeof(numbuf), "%d", pmajor);
  GWEN_Buffer_AppendString(buf, numbuf);
  GWEN_Buffer_AppendByte(buf, '.');
  snprintf(numbuf, sizeof(numbuf), "%d", pminor);
  GWEN_Buffer_AppendString(buf, numbuf);
  GWEN_Buffer_AppendByte(buf, ' ');

  /* append command */
  i=GWEN_DB_GetIntValue(db, "code", 0, 0);
  snprintf(numbuf, sizeof(numbuf), "%d", i);
  GWEN_Buffer_AppendString(buf, numbuf);
  p=GWEN_DB_GetCharValue(db, "text", 0, 0);
  if (p) {
    GWEN_Buffer_AppendByte(buf, ' ');
    GWEN_Buffer_AppendString(buf, p);
  }
  GWEN_Buffer_AppendString(buf, "\r\n");
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnectionHTTP_WriteHeader(GWEN_NETCONNECTION *conn,
				       GWEN_DB_NODE *db,
				       GWEN_BUFFER *buf) {
  GWEN_DB_NODE *dbVar;
  GWEN_NETCONNECTIONHTTP *chttp;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  dbVar=GWEN_DB_GetFirstVar(db);
  while(dbVar) {
    unsigned int i;
    const char *vname;

    vname=GWEN_DB_VariableName(dbVar);
    assert(vname);
    for (i=0; ; i++) {
      const char *p;

      p=GWEN_DB_GetCharValue(db, vname, i, 0);
      if (!p)
	break;
      GWEN_Buffer_AppendString(buf, vname);
      GWEN_Buffer_AppendString(buf, ": ");
      while(*p) {
	if (*p==10) {
	  /* only translate line feed if it is not the last logical char */
	  if (p[1]!=0) {
	    /* this catches LF and CR/LF */
	    if (p[1]!=10 && p[2]!=10) {
	      GWEN_Buffer_AppendString(buf, "\r\n ");
	    }
	  }
	}
	else {
	  if (*p!=13 && *p!=10)
	    GWEN_Buffer_AppendByte(buf, *p);
	}
        p++;
      } /* while */
      GWEN_Buffer_AppendString(buf, "\r\n");
    } /* for */

    dbVar=GWEN_DB_GetNextVar(dbVar);
  } /* while */

  /* mark end of header */
  GWEN_Buffer_AppendString(buf, "\r\n");
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnectionHTTP_AddRequest(GWEN_NETCONNECTION *conn,
				      GWEN_DB_NODE *dbRequest,
				      GWEN_BUFFER *body,
				      GWEN_BUFFEREDIO *bio) {
  GWEN_NETCONNECTIONHTTP *chttp;
  GWEN_NETMSG *msg;
  GWEN_BUFFER *buf;
  GWEN_DB_NODE *db;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  if (body && bio) {
    DBG_ERROR(0, "Please give either a buffer or a bufferedio, not both");
    abort();
  }

  if (body) {
    /* body given, create BUFFEREDIO from it */
    bio=GWEN_BufferedIO_Buffer_new(body);
    GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);
  }

  /* create message */
  msg=GWEN_NetMsg_new(1024);
  buf=GWEN_NetMsg_GetBuffer(msg);

  /* store request data */
  GWEN_DB_AddGroupChildren(GWEN_NetMsg_GetDB(msg), dbRequest);

  /* set new bufferedIO */
  if (bio)
    GWEN_NetMsg_SetBufferedIO(msg, bio);

  /* write command */
  db=GWEN_DB_GetGroup(dbRequest, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "command");
  assert(db);
  if (GWEN_NetConnectionHTTP_WriteCommand(conn, db, buf)) {
    DBG_ERROR(0, "Error writing command");
    GWEN_BufferedIO_free(bio);
    return -1;
  }

  GWEN_NetMsg_SetProtocolVersion(msg, chttp->pmajor, chttp->pminor);

  if (chttp->pmajor>0) {
    /* write header */
    db=GWEN_DB_GetGroup(dbRequest, GWEN_DB_FLAGS_DEFAULT, "header");
    assert(db);
    db=GWEN_DB_Group_dup(db);

    /* modify header (TODO) */
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "server",
			 "Gwenhywfar "GWENHYWFAR_VERSION_FULL_STRING);

    if (GWEN_NetConnectionHTTP_WriteHeader(conn, db, buf)) {
      DBG_ERROR(0, "Error writing header");
      GWEN_BufferedIO_free(bio);
      GWEN_DB_Group_free(db);
      return -1;
    }
    GWEN_DB_Group_free(db);
  }

  GWEN_Buffer_Dump(buf, stderr, 2);

  GWEN_NetConnection_AddOutMsg(conn, msg);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_NetConnectionHTTP_AddResponse(GWEN_NETCONNECTION *conn,
                                       GWEN_DB_NODE *dbResponse,
                                       GWEN_BUFFER *body,
                                       GWEN_BUFFEREDIO *bio) {
  GWEN_NETCONNECTIONHTTP *chttp;
  GWEN_NETMSG *msg;
  GWEN_BUFFER *buf;
  GWEN_DB_NODE *db;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  if (body && bio) {
    DBG_ERROR(0, "Please give either a buffer or a bufferedio, not both");
    abort();
  }

  if (body) {
    /* body given, create BUFFEREDIO from it */
    bio=GWEN_BufferedIO_Buffer_new(body);
    GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);
  }

  /* create message */
  msg=GWEN_NetMsg_new(1024);
  buf=GWEN_NetMsg_GetBuffer(msg);

  /* store request data */
  GWEN_DB_AddGroupChildren(GWEN_NetMsg_GetDB(msg), dbResponse);

  /* set new bufferedIO */
  if (bio)
    GWEN_NetMsg_SetBufferedIO(msg, bio);

  /* write command */
  db=GWEN_DB_GetGroup(dbResponse, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "status");
  assert(db);
  if (GWEN_NetConnectionHTTP_WriteCommand(conn, db, buf)) {
    DBG_ERROR(0, "Error writing status");
    GWEN_BufferedIO_free(bio);
    return -1;
  }

  GWEN_NetMsg_SetProtocolVersion(msg, chttp->pmajor, chttp->pminor);

  if (chttp->pmajor>0) {
    /* write header */
    db=GWEN_DB_GetGroup(dbResponse, GWEN_DB_FLAGS_DEFAULT, "header");
    assert(db);
    db=GWEN_DB_Group_dup(db);

    /* modify header (TODO) */
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "server",
			 "Gwenhywfar "GWENHYWFAR_VERSION_FULL_STRING);

    if (GWEN_NetConnectionHTTP_WriteHeader(conn, db, buf)) {
      DBG_ERROR(0, "Error writing header");
      GWEN_BufferedIO_free(bio);
      GWEN_DB_Group_free(db);
      return -1;
    }
    GWEN_DB_Group_free(db);
  }

  GWEN_Buffer_Dump(buf, stderr, 2);

  GWEN_NetConnection_AddOutMsg(conn, msg);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETMSG*
GWEN_NetConnectionHTTP_GetCurrentInMsg(GWEN_NETCONNECTION *conn){
  GWEN_NETCONNECTIONHTTP *chttp;

  assert(conn);
  chttp=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, GWEN_NETCONNECTIONHTTP, conn);
  assert(chttp);

  return chttp->currentInMsg;
}










