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

#ifndef GWENNET_HTTP_H
#define GWENNET_HTTP_H

#include <gwenhywfar/netconnection.h>
#include <gwenhywfar/bufferedio.h>


#define GWEN_NETCONN_MODE_WAITBEFOREREAD     0x00000001
#define GWEN_NETCONN_MODE_WAITBEFOREBODYREAD 0x00000002
/**
 * Always act as if this were a passive connection as opposed to a active
 * one. This is only usefull for passive connections (as used by HTTP servers)
 * if you want to use the protocol for IPC purposes. Without this flag the
 * connection could only work in a one-way request-response mode.
 * With this flag set there are only requests (so the server must return
 * another request in order to tell the requestor about the result).
 * If this sounds complicated to you then you most probably do not need this
 * mode ;-) However, it MUST NOT be activated for normal HTTP io.
 */
#define GWEN_NETCONN_MODE_IPC                  0x00000004

#define GWEN_NETCONN_MODE_DEFAULT (\
  GWEN_NETCONN_MODE_WAITBEFOREREAD \
  )

#define GWEN_NETCONNHTTP_STATE_STOPBODYREAD    0x00000001
#define GWEN_NETCONNHTTP_STATE_STOPREAD        0x00000002
#define GWEN_NETCONNHTTP_STATE_STARTEDREAD     0x00000004
#define GWEN_NETCONNHTTP_STATE_STARTEDBODYREAD 0x00000008

typedef enum {
  GWEN_NetConnHttp_WriteBodyModeStart=0,
  GWEN_NetConnHttp_WriteBodyModeWrite,
  GWEN_NetConnHttp_WriteBodyModeStop,
  GWEN_NetConnHttp_WriteBodyModeAbort
} GWEN_NETCONNHTTP_WRITEBODY_MODE;


typedef int (*GWEN_NETCONNHTTP_WRITEBODY_FN)(GWEN_NETCONNECTION *conn,
                                             const char *buffer,
                                             GWEN_TYPE_UINT32 *bsize,
                                             GWEN_NETCONNHTTP_WRITEBODY_MODE m);

GWEN_NETCONNECTION *GWEN_NetConnectionHTTP_new(GWEN_NETTRANSPORT *tr,
                                               int take,
                                               GWEN_TYPE_UINT32 libId,
                                               int pmajor,
                                               int pminor);

void GWEN_NetConnectionHTTP_StartBodyRead(GWEN_NETCONNECTION *conn);
void GWEN_NetConnectionHTTP_StartRead(GWEN_NETCONNECTION *conn);


GWEN_TYPE_UINT32 GWEN_NetConnectionHTTP_GetMode(GWEN_NETCONNECTION *conn);
void GWEN_NetConnectionHTTP_SetMode(GWEN_NETCONNECTION *conn,
                                    GWEN_TYPE_UINT32 m);
void GWEN_NetConnectionHTTP_AddMode(GWEN_NETCONNECTION *conn,
                                    GWEN_TYPE_UINT32 m);
void GWEN_NetConnectionHTTP_SubMode(GWEN_NETCONNECTION *conn,
                                    GWEN_TYPE_UINT32 m);

GWEN_TYPE_UINT32 GWEN_NetConnectionHTTP_GetState(GWEN_NETCONNECTION *conn);


int GWEN_NetConnectionHTTP_AddRequest(GWEN_NETCONNECTION *conn,
                                      GWEN_DB_NODE *dbRequest,
                                      GWEN_BUFFER *body,
                                      GWEN_BUFFEREDIO *bio);


GWEN_NETMSG *GWEN_NetConnectionHTTP_GetCurrentInMsg(GWEN_NETCONNECTION *conn);




#endif /* GWENNET_HTTP_H */

