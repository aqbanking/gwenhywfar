/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Jan 24 2004
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

#ifndef GWEN_NETTRANSPORT_H
#define GWEN_NETTRANSPORT_H

#include <gwenhywfar/gwenhywfarapi.h>
#ifdef __cplusplus
extern "C" {
#endif
GWENHYWFAR_API
typedef struct GWEN_NETTRANSPORT GWEN_NETTRANSPORT;
#ifdef __cplusplus
}
#endif

#include <gwenhywfar/types.h>
#include <gwenhywfar/inetaddr.h>
#include <gwenhywfar/inetsocket.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>

#ifdef __cplusplus
extern "C" {
#endif

GWENHYWFAR_API
typedef enum {
  GWEN_NetTransportWorkResult_NoChange=0,
  GWEN_NetTransportWorkResult_Change,
  GWEN_NetTransportWorkResult_Error
} GWEN_NETTRANSPORT_WORKRESULT;


GWEN_LIST_FUNCTION_DEFS(GWEN_NETTRANSPORT, GWEN_NetTransport);
GWEN_INHERIT_FUNCTION_DEFS(GWEN_NETTRANSPORT);


/** @defgroup MOD_NETTRANSPORT Network Transport Layer
 * @ingroup MOD_NET
 *
 * @brief This file contains the definition of a GWEN_NETTRANSPORT.
 *
 * All functions in this group are expected to be non-blocking. If any of the
 * functions is unable to momentarily perform the action it is supposed to
 * perform it should return either @ref GWEN_NetTransportResultWantRead or
 * @ref GWEN_NetTransportResultWantWrite.
 */
/*@{*/



/** @name Transport Layer Flags
 *
 */
/*@{*/
#define GWEN_NETTRANSPORT_FLAGS_PASSIVE 0x0001
#define GWEN_NETTRANSPORT_FLAGS_EOF_IN  0x0002
#define GWEN_NETTRANSPORT_FLAGS_EOF_OUT 0x0004
/*@}*/


/**
 * These are the result codes to be returned by functions of this group.
 */
GWENHYWFAR_API
typedef enum {
  /** Function succeeded */
  GWEN_NetTransportResultOk=0,
  /** Function failed */
  GWEN_NetTransportResultError,
  /**
   * The transport layer needs to read in order to perform the called
   * function. The caller may then use this information for a call to select.
   */
  GWEN_NetTransportResultWantRead,
  /**
   * The transport layer needs to write in order to perform the called
   * function. The caller may then use this information for a call to select.
   */
  GWEN_NetTransportResultWantWrite,
  /**
   * The called function has been aborted by the user (via the WaitCallBack
   * mechanism)
   */
  GWEN_NetTransportResultAborted
} GWEN_NETTRANSPORT_RESULT;


/**
 * This is the status of the transport layer.
 */
GWENHYWFAR_API
typedef enum {
  /** neither connected nor listening, this is the initial state */
  GWEN_NetTransportStatusUnconnected=0,
  /** physically connecting */
  GWEN_NetTransportStatusPConnecting,
  /** physically connected */
  GWEN_NetTransportStatusPConnected,
  /** logically connecting */
  GWEN_NetTransportStatusLConnecting,
  /** logically connected */
  GWEN_NetTransportStatusLConnected,
  /** logically disconnecting */
  GWEN_NetTransportStatusLDisconnecting,
  /** logically disconnected */
  GWEN_NetTransportStatusLDisconnected,
  /** physically disconnecting */
  GWEN_NetTransportStatusPDisconnecting,
  /** physically disconnected */
  GWEN_NetTransportStatusPDisconnected,
  /** listening */
  GWEN_NetTransportStatusListening,
  /** disabled */
  GWEN_NetTransportStatusDisabled
} GWEN_NETTRANSPORT_STATUS;


/** @name Prototypes For Virtual Functions
 *
 */
/*@{*/
/**
 * This function starts to actively connect to another host.
 * Please note that this function MUST set the status accordingly (using
 * @ref GWEN_NetTransport_SetStatus).
 */
GWENHYWFAR_API
typedef GWEN_NETTRANSPORT_RESULT
(*GWEN_NETTRANSPORT_STARTCONNECT)(GWEN_NETTRANSPORT *tr);


/**
 * This function starts to passively await connections from other hosts.
 * Please note that this function MUST set the status accordingly (using
 * @ref GWEN_NetTransport_SetStatus).
 */
GWENHYWFAR_API
typedef GWEN_NETTRANSPORT_RESULT
  (*GWEN_NETTRANSPORT_STARTACCEPT)(GWEN_NETTRANSPORT *tr);

/**
 * This function starts to disconnect from another host.
 * Please note that this function MUST set the status accordingly (using
 * @ref GWEN_NetTransport_SetStatus).
 */
GWENHYWFAR_API
typedef GWEN_NETTRANSPORT_RESULT
  (*GWEN_NETTRANSPORT_STARTDISCONNECT)(GWEN_NETTRANSPORT *tr);

/**
 * This function tries to read data.
 * @param buffer pointer to a buffer for the data
 * @param bsize pointer to a variable which holds the maxmimum number of
 * bytes to read. Upon return this variable contains the number of bytes
 * actually read.
 */
GWENHYWFAR_API
typedef GWEN_NETTRANSPORT_RESULT
  (*GWEN_NETTRANSPORT_READ)(GWEN_NETTRANSPORT *tr,
                            char *buffer,
                            int *bsize);

/**
 * This function tries to write data.
 * @param buffer pointer to a buffer with the data
 * @param bsize pointer to a variable which holds the maxmimum number of
 * bytes to write. Upon return this variable contains the number of bytes
 * actually written.
 */
GWENHYWFAR_API
typedef GWEN_NETTRANSPORT_RESULT
  (*GWEN_NETTRANSPORT_WRITE)(GWEN_NETTRANSPORT *tr,
                             const char *buffer,
                             int *bsize);

/**
 * This function is called when a connection manager tries to gather sockets
 * to be used with @ref GWEN_Socket_Select.
 * Transport which actually do not use GWEN_Sockets should return
 * @ref GWEN_NetTransportResultOk without doing anything.
 * @param sset pointer to the socket set to which sockets should be added
 * @param forReading if !=0 then readable sockets are requested
 */
GWENHYWFAR_API
typedef int
  (*GWEN_NETTRANSPORT_ADDSOCKETS)(GWEN_NETTRANSPORT *tr,
                                  GWEN_SOCKETSET *sset,
                                  int forReading);

/**
 * Allows the transport layer to do some work.
 */
GWENHYWFAR_API
typedef GWEN_NETTRANSPORT_WORKRESULT
  (*GWEN_NETTRANSPORT_WORK)(GWEN_NETTRANSPORT *tr);

/*@}*/





/** @name Constructor And Destructor
 *
 */
/*@{*/
GWENHYWFAR_API
GWEN_NETTRANSPORT *GWEN_NetTransport_new();
void GWEN_NetTransport_free(GWEN_NETTRANSPORT *tr);
/*@}*/


/** @name Connect And Disconnect
 *
 */
/*@{*/
/**
 * This function starts to actively connect to another host.
 * Please note that this function MUST set the status accordingly (using
 * @ref GWEN_NetTransport_SetStatus).
 */
GWENHYWFAR_API
GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransport_StartConnect(GWEN_NETTRANSPORT *tr);

/**
 * This function starts to passively await connections from other hosts.
 * Please note that this function MUST set the status accordingly (using
 * @ref GWEN_NetTransport_SetStatus).
 */
GWENHYWFAR_API
GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransport_StartAccept(GWEN_NETTRANSPORT *tr);

/**
 * This function starts to disconnect from another host.
 * Please note that this function MUST set the status accordingly (using
 * @ref GWEN_NetTransport_SetStatus).
 */
GWENHYWFAR_API
GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransport_StartDisconnect(GWEN_NETTRANSPORT *tr);

/**
 * Returns the next incoming connection if the transport layer is in
 * listening state (or 0 if there is none).
 */
GWENHYWFAR_API
GWEN_NETTRANSPORT *GWEN_NetTransport_GetNextIncoming(GWEN_NETTRANSPORT *tr);
/*@}*/



/** @name Reading And Writing
 *
 */
/*@{*/
/**
 * This function tries to read data.
 * @param buffer pointer to a buffer for the data
 * @param bsize pointer to a variable which holds the maxmimum number of
 * bytes to read. Upon return this variable contains the number of bytes
 * actually read.
 */
GWENHYWFAR_API
GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransport_Read(GWEN_NETTRANSPORT *tr,
                         char *buffer,
                         int *bsize);

/**
 * This function tries to write data.
 * @param buffer pointer to a buffer with the data
 * @param bsize pointer to a variable which holds the maxmimum number of
 * bytes to write. Upon return this variable contains the number of bytes
 * actually written.
 */
GWENHYWFAR_API
GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransport_Write(GWEN_NETTRANSPORT *tr,
                          const char *buffer,
                          int *bsize);
/*@}*/



/** @name Working
 *
 */
/*@{*/
/**
 * This function is called when a connection manager tries to gather sockets
 * to be used with @ref GWEN_Socket_Select.
 * Transport which actually do not use GWEN_Sockets should return
 * @ref GWEN_NetTransportResultOk without doing anything.
 * @param sset pointer to the socket set to which sockets should be added
 * @param forReading if !=0 then readable sockets are requested
 */
GWENHYWFAR_API
int GWEN_NetTransport_AddSockets(GWEN_NETTRANSPORT *tr,
                                 GWEN_SOCKETSET *sset,
                                 int forReading);

/**
 * Allows the transport layer to do some work.
 */
GWENHYWFAR_API
GWEN_NETTRANSPORT_WORKRESULT GWEN_NetTransport_Work(GWEN_NETTRANSPORT *tr);
/*@}*/


/** @name Informational Functions
 *
 */
/*@{*/
GWENHYWFAR_API
GWEN_NETTRANSPORT_STATUS
  GWEN_NetTransport_GetStatus(const GWEN_NETTRANSPORT *tr);

GWENHYWFAR_API
void GWEN_NetTransport_SetStatus(GWEN_NETTRANSPORT *tr,
                                 GWEN_NETTRANSPORT_STATUS st);

GWENHYWFAR_API
GWEN_TYPE_UINT32
  GWEN_NetTransport_GetFlags(const GWEN_NETTRANSPORT *tr);

GWENHYWFAR_API
void GWEN_NetTransport_SetFlags(GWEN_NETTRANSPORT *tr,
                                GWEN_TYPE_UINT32 flags);

GWENHYWFAR_API
const GWEN_INETADDRESS*
  GWEN_NetTransport_GetLocalAddr(const GWEN_NETTRANSPORT *tr);

GWENHYWFAR_API
void GWEN_NetTransport_SetLocalAddr(GWEN_NETTRANSPORT *tr,
                                    const GWEN_INETADDRESS *addr);

GWENHYWFAR_API
const GWEN_INETADDRESS*
  GWEN_NetTransport_GetPeerAddr(const GWEN_NETTRANSPORT *tr);

GWENHYWFAR_API
void GWEN_NetTransport_SetPeerAddr(GWEN_NETTRANSPORT *tr,
                                   const GWEN_INETADDRESS *addr);

GWENHYWFAR_API
const char *GWEN_NetTransport_StatusName(GWEN_NETTRANSPORT_STATUS st);

GWENHYWFAR_API
const char *GWEN_NetTransport_ResultName(GWEN_NETTRANSPORT_RESULT res);

GWENHYWFAR_API
void GWEN_NetTransport_MarkActivity(GWEN_NETTRANSPORT *tr);

GWENHYWFAR_API
double GWEN_NetTransport_GetIdleTime(const GWEN_NETTRANSPORT *tr);

GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_NetTransport_GetBackLog(const GWEN_NETTRANSPORT *tr);

GWENHYWFAR_API
void GWEN_NetTransport_SetBackLog(GWEN_NETTRANSPORT *tr, GWEN_TYPE_UINT32 i);

GWENHYWFAR_API
GWEN_TYPE_UINT32
  GWEN_NetTransport_GetIncomingCount(const GWEN_NETTRANSPORT *tr);

/*@}*/


/** @name Functions For Inheritors
 *
 */
/*@{*/
GWENHYWFAR_API
void GWEN_NetTransport_AddNextIncoming(GWEN_NETTRANSPORT *tr,
                                       GWEN_NETTRANSPORT *newTr);
/*@}*/



/** @name Getters And Setters For Inheritors
 *
 */
/*@{*/
GWENHYWFAR_API
void
  GWEN_NetTransport_SetStartConnectFn(GWEN_NETTRANSPORT *tr,
                                      GWEN_NETTRANSPORT_STARTCONNECT fn);

GWENHYWFAR_API
void
  GWEN_NetTransport_SetStartAcceptFn(GWEN_NETTRANSPORT *tr,
                                     GWEN_NETTRANSPORT_STARTACCEPT fn);

GWENHYWFAR_API
void
  GWEN_NetTransport_SetStartDisconnectFn(GWEN_NETTRANSPORT *tr,
                                         GWEN_NETTRANSPORT_STARTDISCONNECT fn);


GWENHYWFAR_API
void GWEN_NetTransport_SetReadFn(GWEN_NETTRANSPORT *tr,
                                 GWEN_NETTRANSPORT_READ fn);

GWENHYWFAR_API
void GWEN_NetTransport_SetWriteFn(GWEN_NETTRANSPORT *tr,
                                  GWEN_NETTRANSPORT_WRITE fn);

GWENHYWFAR_API
void GWEN_NetTransport_SetAddSocketsFn(GWEN_NETTRANSPORT *tr,
                                       GWEN_NETTRANSPORT_ADDSOCKETS fn);

GWENHYWFAR_API
void GWEN_NetTransport_SetWorkFn(GWEN_NETTRANSPORT *tr,
                                 GWEN_NETTRANSPORT_WORK fn);

/*@}*/

/*@}*/ /* defgroup */

#ifdef __cplusplus
}
#endif

#endif /* GWEN_NETTRANSPORT_H */

