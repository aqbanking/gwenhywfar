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


/** @name Mode Settings
 *
 */
/*@{*/
#define GWEN_NETCONN_MODE_WAITBEFOREREAD     0x00000001
#define GWEN_NETCONN_MODE_WAITBEFOREBODYREAD 0x00000002
/**
 * <p>
 * Always act as if this were a passive connection as opposed to an active
 * one. This is only usefull for passive connections (as used by HTTP servers)
 * if you want to use the protocol for IPC purposes.
 * </p>
 * <p>
 * Without this flag the
 * connection could only work in a one-way request-response mode.
 * With this flag set there are only requests (so the server must return
 * another request in order to tell the requestor about the result).
 * </p>
 * <p>
 * If this sounds complicated to you then you most probably do not need this
 * mode ;-) However, it MUST NOT be activated for normal HTTP io.
 * </p>
 */
#define GWEN_NETCONN_MODE_IPC                  0x00000004

#define GWEN_NETCONN_MODE_DEFAULT (\
  0 \
  )
/*@}*/


/** @name Status Settings
 *
 */
/*@{*/

/** Started to read the next message */
#define GWEN_NETCONNHTTP_STATE_STARTEDREAD        0x0001
/** Started to read the body of the next message */
#define GWEN_NETCONNHTTP_STATE_STARTEDBODYREAD    0x0002
/**
 * Waiting for the program's ok to start reading the next message. The program
 * must call @ref GWEN_NetConnectionHTTP_StartRead to actually start
 * reading
 */
#define GWEN_NETCONNHTTP_STATE_WAITINGFORREAD     0x0004
/**
 * Waiting for the program's ok to start reading the body of the next message.
 * The program must call @ref GWEN_NetConnectionHTTP_StartBodyRead to actually
 * start reading the body.
 */
#define GWEN_NETCONNHTTP_STATE_WAITINGFORBODYREAD 0x0008
/*@}*/

/**
 * This type is used when calling the callback function which writes the body
 * received.
 */
typedef enum {
  /** starting to receive the message body */
  GWEN_NetConnHttp_WriteBodyModeStart=0,
  /** have something to write (part of the message body) */
  GWEN_NetConnHttp_WriteBodyModeWrite,
  /** finished receiving the body of the message */
  GWEN_NetConnHttp_WriteBodyModeStop,
  /** aborting reception of the message body (due to an error) */
  GWEN_NetConnHttp_WriteBodyModeAbort
} GWEN_NETCONNHTTP_WRITEBODY_MODE;


GWEN_NETCONNECTION *GWEN_NetConnectionHTTP_new(GWEN_NETTRANSPORT *tr,
                                               int take,
                                               GWEN_TYPE_UINT32 libId,
                                               int pmajor,
                                               int pminor);

/** @name Reading the Message Body
 *
 * When receiving a message this class does this:
 * <ul>
 *  <li>read the command or status line</li>
 *  <li>read the header (if any)</li>
 *  <li>if <b>GWEN_NETCONN_MODE_WAITBEFOREBODYREAD</b> is set:
 *      Stop reading, thus giving the program an opportunity to interfere
 *      Otherwise it goes on with the next step.</li>
 *  <li>call the callback function @ref GWEN_NETCONNHTTP_WRITEBODY_FN with
 *      mode=@ref GWEN_NetConnHttp_WriteBodyModeStart</li>
 *  <li>read the body, store it using the callback function
 *      @ref GWEN_NETCONNHTTP_WRITEBODY_FN. If this callback is not set
 *      then the message body will be stored within the GWEN_BUFFER
 *      associated with the currently received message.</li>
 *  <li>call the callback function @ref GWEN_NETCONNHTTP_WRITEBODY_FN with
 *      mode=@ref GWEN_NetConnHttp_WriteBodyModeStop</li>
 *  <li>enqueue the received message when completed</li>
 * </ul>
 * <br>
 * Whenever there is an error while reading the message body the callback
 * function @ref GWEN_NETCONNHTTP_WRITEBODY_FN will be called with
 * mode=@ref GWEN_NetConnHttp_WriteBodyModeAbort
 */
/*@{*/
typedef int (*GWEN_NETCONNHTTP_WRITEBODY_FN)(GWEN_NETCONNECTION *conn,
                                             const char *buffer,
                                             GWEN_TYPE_UINT32 *bsize,
                                             GWEN_NETCONNHTTP_WRITEBODY_MODE m);
/**
 * This function is only needed if the connection mode
 * @ref GWEN_NETCONN_MODE_WAITBEFOREBODYREAD is in effect. In this case
 * you need to call this function to actually start reading the message
 * body.
 */
void GWEN_NetConnectionHTTP_StartBodyRead(GWEN_NETCONNECTION *conn);

/**
 * This function is only needed if the connection mode
 * @ref GWEN_NETCONN_MODE_WAITBEFOREREAD is in effect. In this case
 * you need to call this function to actually start reading the next message.
 */
void GWEN_NetConnectionHTTP_StartRead(GWEN_NETCONNECTION *conn);

/**
 * This returns a pointer to the message currently under receiption.
 * You can inspect the GWEN_DB_NODE inside that message object which contains
 * the command line and message header (if any). This can be used to determine
 * how or where to store the message body and e.g. to setup the bufferedIO
 * for storing the message body.
 * This functions does not relinquish ownership of the message ! So you
 * MUST NOT free it (and you should not manipulate data inside this object
 * besides the GWEN_DB_NODE and the GWEN_BUFFEREDIO settings).
 */
GWEN_NETMSG *GWEN_NetConnectionHTTP_GetCurrentInMsg(GWEN_NETCONNECTION *conn);
/*@}*/


/** @name Informational Functions
 *
 */
/*@{*/
GWEN_TYPE_UINT32 GWEN_NetConnectionHTTP_GetMode(GWEN_NETCONNECTION *conn);
void GWEN_NetConnectionHTTP_SetMode(GWEN_NETCONNECTION *conn,
                                    GWEN_TYPE_UINT32 m);
void GWEN_NetConnectionHTTP_AddMode(GWEN_NETCONNECTION *conn,
                                    GWEN_TYPE_UINT32 m);
void GWEN_NetConnectionHTTP_SubMode(GWEN_NETCONNECTION *conn,
                                    GWEN_TYPE_UINT32 m);

/**
 * You can use this function to check whether the connection needs the
 * program's attention.
 */
GWEN_TYPE_UINT32 GWEN_NetConnectionHTTP_GetState(GWEN_NETCONNECTION *conn);
/*@}*/


/**
 * <p>
 * Functions in this group add a request or response which is sent as soon as
 * possible.
 * </p>
 * The request itself consists of three parts:
 * <ul>
 *  <li>a command (or a status in case of a response)</li>
 *  <li>an optional header</li>
 *  <li>a message body</li>
 * </ul>
 * <p>
 * The first two items are stored inside a GWEN_DB. This GWEN_DB has the
 * following structure (example):
 * @code
 *   request {
 *     command {                 # this group only exists in requests
 *       cmd="get"               # or "put" or "head" or any other valid HTTP
 *       url="/"                 # command uniform ressource locator on the
 *                               # server
 *
 *       vars {                  # this group is optional and may contain
 *         user="me"             # whatever variable you like. All these
 *         pass="xy"             # variables are URL-encoded and appended to
 *       }                       # the URL before sending
 *     }                         # HTTP-escaping is done transparently
 *
 *     status {                  # this group only exists in responses
 *       int code="200"          # status code
 *       text="Ok"               # status message (human readable)
 *     }
 *
 *     header {                  # this group is optional and may contain
 *       connection="keep-alive" # any valid HTTP header.
 *       accept="image/gif"      # any data MUST be completely surrounded
 *     }                         # quotation marks (no kommas outside!)
 *   }
 * @endcode
 * </p>
 * The body (if any) can be given by either of two ways:
 * <ul>
 *  <li>by a GWEN_BUFFER (use argument <b>body</b>)</li>
 *  <li>by a GWEN_BUFFEREDIO (use argument <b>bio</b>)</li>
 * </ul>
 * <p>
 * You can <b>not</b> use both.<br>
 * If neither way is selected no body is sent.<br>
 * </p>
 * <p>
 * <b>Note:</b>This function takes over the ownership of the GWEN_BUFFER (if
 * any) and the GWEN_BUFFER (if given) regardless of the result.
 * </p>
 * @param conn HTTP connection to use
 * @param dbRequest command and optional header as described above
 * @param body message body given by a GWEN_BUFFER (0 if not)
 * @param bio message body given by a GWEN_BUFFEREDIO (0 if not)
 */
/*@{*/
int GWEN_NetConnectionHTTP_AddRequest(GWEN_NETCONNECTION *conn,
                                      GWEN_DB_NODE *dbRequest,
                                      GWEN_BUFFER *body,
                                      GWEN_BUFFEREDIO *bio);


int GWEN_NetConnectionHTTP_AddResponse(GWEN_NETCONNECTION *conn,
                                       GWEN_DB_NODE *dbResponse,
                                       GWEN_BUFFER *body,
                                       GWEN_BUFFEREDIO *bio);
/*@}*/


/** @name Tool Functions
 *
 */
/*@{*/
void GWEN_NetConnectionHTTP_Escape(const char *src, GWEN_BUFFER *buf);
int GWEN_NetConnectionHTTP_Unescape(const char *src, GWEN_BUFFER *buf);
/*@}*/


#endif /* GWENNET_HTTP_H */





