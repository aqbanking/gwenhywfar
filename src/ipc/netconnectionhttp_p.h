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

#ifndef GWENNET_HTTP_P_H
#define GWENNET_HTTP_P_H

#include <gwenhywfar/netconnectionhttp.h>
#include <gwenhywfar/netmsg.h>

#define GWEN_NETCONNHTTP_MSGBUFSIZE 1024
#define GWEN_NETCONNHTTP_MAXBODYSIZE (64*1024)

/* default is HTTP/1.1 */
#define GWEN_NETCONN_PMAJOR_DEFAULT 1
#define GWEN_NETCONN_PMINOR_DEFAULT  1

#define GWEN_NETCONNHTTP_STATE_STOPBODYREAD       0x00010000
#define GWEN_NETCONNHTTP_STATE_STOPREAD           0x00020000


#define GWEN_NETCONNHTTP_NO_ESCAPE(x) \
  ( \
  (x>='A' && x<='Z') || \
  (x>='a' && x<='z') || \
  (x>='0' && x<='9') || \
  x=='-' || \
  x=='_' || \
  x=='.' || \
  x=='*' \
  )


typedef enum {
  GWEN_NetConnHttpMsgModeCmd=0,
  GWEN_NetConnHttpMsgModeStatus,
  GWEN_NetConnHttpMsgModeHeader,
  GWEN_NetConnHttpMsgModeBody,

  GWEN_NetConnHttpMsgModeBuffer,
  GWEN_NetConnHttpMsgModeBio
} GWEN_NETCONNHTTP_MSG_MODE;


typedef struct GWEN_NETCONNECTIONHTTP GWEN_NETCONNECTIONHTTP;
struct GWEN_NETCONNECTIONHTTP {
  int pmajor;
  int pminor;
  GWEN_TYPE_UINT32 state;
  GWEN_TYPE_UINT32 mode;
  GWEN_NETCONNHTTP_MSG_MODE inMode;
  GWEN_NETMSG *currentInMsg;
  GWEN_TYPE_UINT32 inLast4Bytes;
  GWEN_TYPE_UINT32 headerPos;
  GWEN_TYPE_UINT32 bodyBytesRead;

  GWEN_NETCONNHTTP_MSG_MODE outMode;
  GWEN_NETMSG *currentOutMsg;
  GWEN_TYPE_UINT32 bodyBytesWritten;

  GWEN_NETCONNHTTP_WRITEBODY_FN writeBodyFn;
};


void GWEN_NetConnectionHTTP_FreeData(void *bp, void *p);
GWEN_NETCONNECTION_WORKRESULT
  GWEN_NetConnectionHTTP_Work(GWEN_NETCONNECTION *conn);

int GWEN_NetConnectionHTTP_ParseCommand(const char *buffer,
                                        GWEN_DB_NODE *db);
int GWEN_NetConnectionHTTP_ParseStatus(const char *buffer,
                                       GWEN_DB_NODE *db);
int GWEN_NetConnectionHTTP_ParseHeader(const char *buffer,
                                       GWEN_DB_NODE *db);
int GWEN_NetConnectionHTTP_WriteBody(GWEN_NETCONNECTION *conn,
                                     const char *buffer,
                                     GWEN_TYPE_UINT32 *bsize,
                                     GWEN_NETCONNHTTP_WRITEBODY_MODE m);

int GWEN_NetConnectionHTTP_WriteCommand(GWEN_NETCONNECTION *conn,
                                        GWEN_DB_NODE *db,
                                        GWEN_BUFFER *buf);
int GWEN_NetConnectionHTTP_WriteStatus(GWEN_NETCONNECTION *conn,
                                       GWEN_DB_NODE *db,
                                       GWEN_BUFFER *buf);
int GWEN_NetConnectionHTTP_WriteHeader(GWEN_NETCONNECTION *conn,
                                       GWEN_DB_NODE *db,
                                       GWEN_BUFFER *buf);



#endif /* GWENNET_HTTP_P_H */

