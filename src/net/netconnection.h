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


#ifndef GWEN_NETCONNECTION_H
#define GWEN_NETCONNECTION_H

/** @defgroup MOD_NETCONNECTION Network Connection
 * @ingroup MOD_NET
 *
 * @brief This file contains the definition of a GWEN_NETCONNECTION.
 *
 */
/*@{*/


/** @name Wait Flags
 *
 * These flags are used with @ref GWEN_NetConnection_Wait.
 */
/*@{*/
/** wait until the transport layer is readable */
#define GWEN_NETCONNECTION_WAIT_READ  0x0001
/** wait until the transport layer is writeable */
#define GWEN_NETCONNECTION_WAIT_WRITE 0x0002
/*@}*/


/** @name Special Timeout Values
 *
 * These special timeout values can be used whenever a timeout parameter
 * is expected.
 * Positive timeout values specify a limit for the number of seconds to wait.
 */
/*@{*/
/** don't wait at all */
#define GWEN_NETCONNECTION_TIMEOUT_NONE    (0)
/** wait forever */
#define GWEN_NETCONNECTION_TIMEOUT_FOREVER (-1)
/*@}*/


#define GWEN_NETCONNECTION_CBID_IO "GWEN_NETCONNECTION_CBID_IO"


/**
 * This is the data type for a connection.
 * It should be handled as a black box for maximum compatibility.
 */
#include <gwenhywfar/gwenhywfarapi.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct GWEN_NETCONNECTION GWEN_NETCONNECTION;
#ifdef __cplusplus
}
#endif

#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/nettransport.h>
#include <gwenhywfar/netmsg.h>
#include <gwenhywfar/ringbuffer.h>

#define GWEN_NETCONNECTION_CHECK_WANTREAD  0x0001
#define GWEN_NETCONNECTION_CHECK_WANTWRITE 0x0002

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  GWEN_NetConnectionWorkResult_NoChange=0,
  GWEN_NetConnectionWorkResult_Change,
  GWEN_NetConnectionWorkResult_Error,
} GWEN_NETCONNECTION_WORKRESULT;


GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_NETCONNECTION, GWEN_NetConnection, GWENHYWFAR_API)
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_NETCONNECTION, GWENHYWFAR_API)
/* No trailing semicolon here because this is a macro call */


/** @name Prototypes For Virtual Functions
 *
 */
/*@{*/
/** see @ref GWEN_NetConnection_Work */
typedef GWEN_NETCONNECTION_WORKRESULT
  (*GWEN_NETCONNECTION_WORKFN)(GWEN_NETCONNECTION *conn);
/** see @ref GWEN_NetConnection_Up */
typedef void (*GWEN_NETCONNECTION_UPFN)(GWEN_NETCONNECTION *conn);
/** see @ref GWEN_NetConnection_Down */
typedef void (*GWEN_NETCONNECTION_DOWNFN)(GWEN_NETCONNECTION *conn);

typedef GWEN_TYPE_UINT32
  (*GWEN_NETCONNECTION_CHECKFN)(GWEN_NETCONNECTION *conn);

/*@}*/


/** @name Constructor And Destructor
 *
 */
/*@{*/
/**
 * Constructor.
 * @param tr underlying transport layer
 * @param take if !=0 then the connection will take over ownership of the
 * transport layer
 * @param libId id assigned via @ref GWEN_Net_GetLibraryId, this can
 * be used by programs and libraries to find out which connection belongs to
 * which library/program (this is interesting for connections which have been
 * added to the connection pool via @ref GWEN_Net_AddConnectionToPool)
 */
GWENHYWFAR_API
GWEN_NETCONNECTION *GWEN_NetConnection_new(GWEN_NETTRANSPORT *tr,
                                           int take,
                                           GWEN_TYPE_UINT32 libId);

GWENHYWFAR_API
void GWEN_NetConnection_free(GWEN_NETCONNECTION *conn);

GWENHYWFAR_API
void GWEN_NetConnection_Attach(GWEN_NETCONNECTION *conn);

/*@}*/


/** @name Reading And Writing
 *
 */
/*@{*/
/**
 * Reads data from the connection.
 * @return 0 if ok, !=0 on error
 * @param conn connection
 * @param buffer target buffer
 * @param bsize pointer to a variable with the size of the buffer. Upon
 * return this variable reflects the number of bytes really read
 */
GWENHYWFAR_API
int GWEN_NetConnection_Read(GWEN_NETCONNECTION *conn,
                            char *buffer,
                            GWEN_TYPE_UINT32 *bsize);

/**
 * Tries to read data. Waits if necessary.
 * @return 0 if ok, !=0 on error
 * @param conn connection
 * @param buffer target buffer
 * @param bsize pointer to a variable with the size of the buffer. Upon
 * return this variable reflects the number of bytes really read
 * @param timeout timeout in seconds (or a special timeout value, see
 * @ref GWEN_NETCONNECTION_TIMEOUT_NONE)
 */
GWENHYWFAR_API
int GWEN_NetConnection_Read_Wait(GWEN_NETCONNECTION *conn,
                                 char *buffer,
                                 GWEN_TYPE_UINT32 *bsize,
                                 int timeout);


/**
 * Writes data to the connection.
 * @return 0 if ok, !=0 on error
 * @param conn connection
 * @param buffer source buffer
 * @param bsize pointer to a variable with the size of the buffer. Upon
 * return this variable reflects the number of bytes really written
 */
GWENHYWFAR_API
int GWEN_NetConnection_Write(GWEN_NETCONNECTION *conn,
                             const char *buffer,
                             GWEN_TYPE_UINT32 *bsize);

/**
 * Tries to write data. Waits if necessary.
 * This function does not flush the data you are about to write here !
 * If you want to make sure that all data gets written you should call the
 * function @ref GWEN_NetConnection_Flush.
 * @return 0 if ok, !=0 on error
 * @param conn connection
 * @param buffer source buffer
 * @param bsize pointer to a variable with the size of the buffer. Upon
 * return this variable reflects the number of bytes really written
 * @param timeout timeout in seconds (or a special timeout value, see
 * @ref GWEN_NETCONNECTION_TIMEOUT_NONE)
 */
GWENHYWFAR_API
int GWEN_NetConnection_Write_Wait(GWEN_NETCONNECTION *conn,
                                  const char *buffer,
                                  GWEN_TYPE_UINT32 *bsize,
                                  int timeout);

/**
 * Tries to flush all data from the internal buffer. if this function
 * successfully returns all data has been sent via the connection.
 * @return 0 if ok, !=0 on error
 * @param conn connection
 * @param timeout timeout in seconds (or a special timeout value, see
 * @ref GWEN_NETCONNECTION_TIMEOUT_NONE)
 */
GWENHYWFAR_API
int GWEN_NetConnection_Flush(GWEN_NETCONNECTION *conn,
                             int timeout);
/*@}*/


/** @name Opening, Closing And Listening
 *
 */
/*@{*/
/**
 * Starts to connect to the address stored as peer address in the underlying
 * transport layer.
 * You should call @ref GWEN_NetConnection_WorkIO or use one of the read/write
 * functions in order to complete the connect.
 * @return 0 if ok, !=0 on error
 * @param conn connection
 */
GWENHYWFAR_API
int GWEN_NetConnection_StartConnect(GWEN_NETCONNECTION *conn);

/**
 * Does the same as @ref GWEN_NetConnection_StartConnect does, but this
 * functions waits if necessary.
 * @return 0 if ok, !=0 on error
 * @param conn connection
 */
GWENHYWFAR_API
int GWEN_NetConnection_Connect_Wait(GWEN_NETCONNECTION *conn,
                                    int timeout);

/**
 * Starts to listen on the address stored as local address in the underlying
 * transport layer.
 * You should call @ref GWEN_NetConnection_WorkIO or any wait function in order
 * to accept connections which are then available via
 * @ref GWEN_NetConnection_GetNextIncoming.
 * @return 0 if ok, !=0 on error
 * @param conn connection
 */
GWENHYWFAR_API
int GWEN_NetConnection_StartListen(GWEN_NETCONNECTION *conn);

/**
 * Returns the next incoming connection. This only makes sense if the
 * underlying transport layer is in listening state.
 * @return new incoming transport layer (or 0 if there is none)
 * @param conn connection
 */
GWENHYWFAR_API
GWEN_NETTRANSPORT*
  GWEN_NetConnection_GetNextIncoming(GWEN_NETCONNECTION *conn);

/**
 * Does the same as @ref GWEN_NetConnection_GetNextIncoming does, but this
 * functions waits if necessary.
 * @return new incoming transport layer (or 0 if there is none)
 * @param conn connection
 * @param timeout timeout in seconds (or a special timeout value, see
 * @ref GWEN_NETCONNECTION_TIMEOUT_NONE)
 */
GWENHYWFAR_API
GWEN_NETTRANSPORT*
  GWEN_NetConnection_GetNextIncoming_Wait(GWEN_NETCONNECTION *conn,
                                          int timeout);

/**
 * Starts to disconnect this connection.
 * You should call @ref GWEN_NetConnection_WorkIO or any wait function in order
 * to complete the disconnect.
 * @return 0 if ok, !=0 on error
 * @param conn connection
 */
GWENHYWFAR_API
int GWEN_NetConnection_StartDisconnect(GWEN_NETCONNECTION *conn);

/**
 * Does the same as @ref GWEN_NetConnection_StartDisconnect does, but this
 * functions waits if necessary.
 * @return 0 if ok, !=0 on error
 * @param conn connection
 * @param timeout timeout in seconds (or a special timeout value, see
 * @ref GWEN_NETCONNECTION_TIMEOUT_NONE)
 */
GWENHYWFAR_API
int GWEN_NetConnection_Disconnect_Wait(GWEN_NETCONNECTION *conn,
                                       int timeout);
/*@}*/


/** @name Waiting And Working
 *
 */
/*@{*/
/**
 * This function should be called periodically to make use of the connection.
 * It gives the underlying transport layer the opportunity to work (by
 * calling @ref GWEN_NetTransport_Work) and fills/flushes the internal
 * buffers. This function should also be called by the implementation of
 * @ref GWEN_NetConnection_Work.
 * @return 0 if ok, !=0 on error
 * @param conn connection
 */
GWENHYWFAR_API
GWEN_NETCONNECTION_WORKRESULT
  GWEN_NetConnection_WorkIO(GWEN_NETCONNECTION *conn);

/**
 * This function calls @ref GWEN_NetConnection_WorkIO until the expected
 * status is reached (or a timeout occurrs).
 * @return 0 if ok, !=0 on error
 * @param conn connection
 * @param expStatus expected status
 * @param timeout timeout in seconds (or a special timeout value, see
 * @ref GWEN_NETCONNECTION_TIMEOUT_NONE)
 */
GWENHYWFAR_API
int GWEN_NetConnection_WaitForStatus(GWEN_NETCONNECTION *conn,
                                     GWEN_NETTRANSPORT_STATUS expStatus,
                                     int timeout);

/**
 * Lets a list of connections work. This functions waits if needed until
 * at least one connection becomes active.
 * @param connlist list of connections to walk
 * @param timeout timeout in milliseconds (or a special timeout value, see
 * @ref GWEN_NETCONNECTION_TIMEOUT_NONE)
 */
GWENHYWFAR_API
GWEN_NETCONNECTION_WORKRESULT
  GWEN_NetConnection_Walk(GWEN_NETCONNECTION_LIST *connList,
                          int timeout);

/*@}*/


/** @name Virtual Functions
 *
 */
/*@{*/
/**
 * This function should be called periodically to make use of the connection.
 * It gives the overlying protocol layer the opportunity to work (e.g. to
 * parse incoming messages etc).
 * Set this function via @ref GWEN_NetConnection_SetWorkFn to point to the
 * work function of the protocol (it defaults to
 * @ref GWEN_NetConnection_WorkIO).
 * This work function should call @ref GWEN_NetConnection_WorkIO.
 * @return 0 if ok, !=0 on error
 * @param conn connection
 */
GWENHYWFAR_API
GWEN_NETCONNECTION_WORKRESULT
  GWEN_NetConnection_Work(GWEN_NETCONNECTION *conn);

/**
 * This function is called for every connection upon establishement.
 * You can use this to implement further checks (i.e. checking against
 * the peer's IP address etc) or to notify the application when a connection
 * is up.<br>
 * Please note that this function MUST be called by a server for every
 * incoming connection by hand, because when the physical connection is
 * accepted by the transport layer there is not yet a connection layer, this
 * is created by the server and so he must call this UP function.
 * However, the DOWN function is called automatically.
 * @param conn connection
 */
GWENHYWFAR_API
void GWEN_NetConnection_Up(GWEN_NETCONNECTION *conn);

/**
 * This function is called for every connection when it is detected
 * to be down.
 * @param conn connection
 */
GWENHYWFAR_API
void GWEN_NetConnection_Down(GWEN_NETCONNECTION *conn);

/*@}*/


/** @name Setters For Virtual Functions
 *
 */
/*@{*/
GWENHYWFAR_API
void GWEN_NetConnection_SetWorkFn(GWEN_NETCONNECTION *conn,
                                  GWEN_NETCONNECTION_WORKFN fn);

GWENHYWFAR_API
void GWEN_NetConnection_SetUpFn(GWEN_NETCONNECTION *conn,
                                GWEN_NETCONNECTION_UPFN fn);

GWENHYWFAR_API
void GWEN_NetConnection_SetDownFn(GWEN_NETCONNECTION *conn,
                                  GWEN_NETCONNECTION_DOWNFN fn);
GWENHYWFAR_API
void GWEN_NetConnection_SetCheckFn(GWEN_NETCONNECTION *conn,
                                   GWEN_NETCONNECTION_CHECKFN fn);
/*@}*/


#if (defined(GWEN_EXTEND_NETCONNECTION) || defined(DOXYGEN))
/** @name Inheritance API
 *
 * <p>
 * Functions in this group are meant to be called by inheriting "classes".
 * These functions MUST NOT be called by programs.
 * </p>
 * <p>
 * You must define GWEN_EXTEND_NETCONNECTION prior to including this
 * header file in order to have these functions defined.
 * </p>
 *
 */
/*@{*/

/**
 * Returns a pointer to the connection's read buffer.
 * The connection still remains owner of that buffer, therefore the caller
 * MUST NOT free the buffer returned. Programs should rather call the
 * various read functions instead of directly accessing the ring buffer.
 */
GWENHYWFAR_API
GWEN_RINGBUFFER *GWEN_NetConnection_GetReadBuffer(GWEN_NETCONNECTION *conn);

/**
 * Returns a pointer to the connection's write buffer.
 * The connection still remains owner of that buffer, therefore the caller
 * MUST NOT free the buffer returned. Programs should rather call the
 * various write functions instead of directly accessing the ring buffer.
 */
GWENHYWFAR_API
GWEN_RINGBUFFER *GWEN_NetConnection_GetWriteBuffer(GWEN_NETCONNECTION *conn);

/**
 * Adds a message to the incoming list. Inheriting classes should call this
 * when finishing parsing an incoming messages in order to make them available
 * for programs. This function takes over ownership of the given message.
 */
GWENHYWFAR_API
void GWEN_NetConnection_AddInMsg(GWEN_NETCONNECTION *conn,
                                 GWEN_NETMSG *msg);

/**
 * Returns the next message from the output queue (if any).
 * The caller is responsible for freeing the message returned since this
 * function relinquishes ownership of that message.
 */
GWENHYWFAR_API
GWEN_NETMSG *GWEN_NetConnection_GetOutMsg(GWEN_NETCONNECTION *conn);

/**
 * Returns the next message from the output queue (if any) but keeps it in
 * the queue. You should not modify that msg neither must you free
 * this message.
 */
GWENHYWFAR_API
GWEN_NETMSG *GWEN_NetConnection_PeekOutMsg(const GWEN_NETCONNECTION *conn);

/*@}*/
#endif /* GWEN_EXTENDING_NETCONNECTION */


/** @name Message Related Functions
 *
 */
/*@{*/
/**
 * Returns the next message from the input queue (if any).
 * The caller is responsible for freeing the message returned since this
 * function relinquishes ownership of that message.
 */
GWENHYWFAR_API
GWEN_NETMSG *GWEN_NetConnection_GetInMsg(GWEN_NETCONNECTION *conn);

/**
 * Returns the next message from the input queue, waits if necessary.
 * The caller is responsible for freeing the message returned since this
 * function relinquishes ownership of that message.
 */
GWENHYWFAR_API
GWEN_NETMSG *GWEN_NetConnection_GetInMsg_Wait(GWEN_NETCONNECTION *conn,
                                             int timeout);

/**
 * Returns the next message from the input queue (if any) but keeps it in
 * the queue. You should not modify that msg neither must you free
 * this message.
 */
GWENHYWFAR_API
GWEN_NETMSG *GWEN_NetConnection_PeekInMsg(const GWEN_NETCONNECTION *conn);

/**
 * Adds a message to the output queue of this connection. It will be taken
 * from there by inheriting "classes" and be send via the transport layer.
 */
GWENHYWFAR_API
void GWEN_NetConnection_AddOutMsg(GWEN_NETCONNECTION *conn,
                                  GWEN_NETMSG *msg);

/*@}*/



/** @name Informational Functions
 *
 */
/*@{*/
/**
 * Returns the underlying transport layer.
 * This function does not relinquish ownership of the returned pointer !
 */
GWENHYWFAR_API
GWEN_NETTRANSPORT*
  GWEN_NetConnection_GetTransportLayer(const GWEN_NETCONNECTION *conn);


/**
 * This is just a conveniance function to retrieve the peer's address.
 */
GWENHYWFAR_API
const GWEN_INETADDRESS*
  GWEN_NetConnection_GetPeerAddr(const GWEN_NETCONNECTION *conn);

/**
 * This is just a conveniance function to retrieve the flags of the underlying
 * transport layer.
 */
GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_NetConnection_GetFlags(const GWEN_NETCONNECTION *conn);


/**
 * This is just a conveniance function to retrieve the status of the
 * underlying transport layer.
 */
GWENHYWFAR_API
GWEN_NETTRANSPORT_STATUS
  GWEN_NetConnection_GetStatus(const GWEN_NETCONNECTION *conn);

GWENHYWFAR_API
void GWEN_NetConnection_SetStatus(GWEN_NETCONNECTION *conn,
                                  GWEN_NETTRANSPORT_STATUS nst);

/**
 * This is just a conveniance function to set the flags of the underlying
 * transport layer.
 */
GWENHYWFAR_API
void GWEN_NetConnection_SetFlags(GWEN_NETCONNECTION *conn,
                                 GWEN_TYPE_UINT32 fl);

/**
 * Returns the id assigned via @ref GWEN_Net_GetLibraryId, this can
 * be used by programs and libraries to find out which connection belongs to
 * which library/program (this is interesting for connections which have been
 * added to the connection pool via @ref GWEN_Net_AddConnectionToPool)
 */
GWENHYWFAR_API
GWEN_TYPE_UINT32
  GWEN_NetConnection_GetLibraryMark(const GWEN_NETCONNECTION *conn);

/**
 * This is an integer value at the disposal of the application. It can be
 * used to separate connections by any criteria the program likes.
 * This variable is not used by Gwenhywfar at all.
 */
GWENHYWFAR_API
GWEN_TYPE_UINT32
  GWEN_NetConnection_GetUserMark(const GWEN_NETCONNECTION *conn);

/**
 * This is an integer value at the disposal of the application. It can be
 * used to separate connections by any criteria the program likes.
 * This variable is not used by Gwenhywfar at all.
 */
GWENHYWFAR_API
void GWEN_NetConnection_SetUserMark(GWEN_NETCONNECTION *conn,
                                    GWEN_TYPE_UINT32 m);

/**
 * Returns the time (in seconds) the connection has been idle.
 * Please note that the connection is idle if
 * <ul>
 *  <li>there was no traffic on the connection</li>
 *  <li>neither @ref GWEN_NetConnection_Work nor @ref GWEN_NetConnection_Walk
 *    are called</li>
 * </ul>
 */
GWENHYWFAR_API
double GWEN_NetConnection_GetIdleTime(const GWEN_NETCONNECTION *conn);


/**
 * Call this function with i!=0 if you want the connection to be disconnected
 * after the last message in the queue has been sent.
 */
GWENHYWFAR_API
void GWEN_NetConnection_SetDownAfterSend(GWEN_NETCONNECTION *conn, int i);

/**
 * Returns the flag set by @ref GWEN_NetConnection_SetDownAfterSend (see
 * there).
 */
GWENHYWFAR_API
int GWEN_NetConnection_GetDownAfterSend(GWEN_NETCONNECTION *conn);

GWENHYWFAR_API
void GWEN_NetConnection_Reset(GWEN_NETCONNECTION *conn);

/*@}*/

GWENHYWFAR_API
void GWEN_NetConnection_Dump(const GWEN_NETCONNECTION *conn);

/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif

#endif





