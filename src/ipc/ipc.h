/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri May 07 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_IPC_H
#define GWEN_IPC_H


#include <gwenhywfar/types.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/nettransport.h>
#include <gwenhywfar/netconnection.h>

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name Error codes
 *
 */
  /*@{*/
#define GWEN_IPC_ERROR_CODES   0x00000000
#define GWEN_IPC_ERROR_GENERIC 0x80000001
#define GWEN_IPC_ERROR_CONNERR 0x80000002
#define GWEN_IPC_ERROR_TIMEOUT 0x80000003
/*@}*/



GWENHYWFAR_API
typedef struct GWEN_IPCMANAGER GWEN_IPCMANAGER;


GWENHYWFAR_API
GWEN_IPCMANAGER *GWEN_IPCManager_new();

GWENHYWFAR_API
void GWEN_IPCManager_free(GWEN_IPCMANAGER *mgr);


/**
 * Sets a callback function which is called for every network connection that
 * went up.
 */
GWENHYWFAR_API
void GWEN_IPCManager_SetUpFn(GWEN_IPCMANAGER *mgr,
                             GWEN_TYPE_UINT32 id,
                             GWEN_NETCONNECTION_UPFN fn);

/**
 * Sets a callback function which is called for every network connection that
 * went down.
 */
GWENHYWFAR_API
void GWEN_IPCManager_SetDownFn(GWEN_IPCMANAGER *mgr,
                               GWEN_TYPE_UINT32 id,
                               GWEN_NETCONNECTION_DOWNFN fn);

/**
 * Starts disconnecting the given client/server.
 */
GWENHYWFAR_API
int GWEN_IPCManager_Disconnect(GWEN_IPCMANAGER *mgr, GWEN_TYPE_UINT32 nid);


/**
 * Adds a server to the IPC manager. The IPC manager will immediately
 * enter listen mode on the given transport layer.
 * @return node id of the node created (0 on error)
 * @param mgr pointer to the IPC manager object
 * @param tr transport layer to be used (this MUST not be connected or in
 *  listening state)
 * @param mark a value assigned to the new connection. This value can be used
 *   for functions @ref GWEN_IPCManager_GetNextInRequest and
 *   @ref GWEN_IPCManager_SendMultiRequest, it is not otherwise used by
 *   the IPC manager
 */
GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_IPCManager_AddServer(GWEN_IPCMANAGER *mgr,
                                           GWEN_NETTRANSPORT *tr,
                                           GWEN_TYPE_UINT32 mark);

/**
 * Adds a client to the IPC manager. The connection will not be established
 * until needed.
 * @return node id of the node created (0 on error)
 * @param mgr pointer to the IPC manager object
 * @param tr transport layer to be used (this MUST not be connected or in
 *  listening state)
 * @param userName if given this name will be used for HTTP authorization
 * @param passwd if given this password will be used for HTTP authorization
 * @param mark a value assigned to the new connection. This value can be used
 *   for functions @ref GWEN_IPCManager_GetNextInRequest and
 *   @ref GWEN_IPCManager_SendMultiRequest, it is not otherwise used by
 *   the IPC manager
 */
GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_IPCManager_AddClient(GWEN_IPCMANAGER *mgr,
                                           GWEN_NETTRANSPORT *tr,
                                           const char *userName,
                                           const char *passwd,
                                           GWEN_TYPE_UINT32 mark);

/**
 * Sends a request via the given connection.
 * It takes over the ownership of the given GWEN_DB in any case.
 * @return request id of the request created (0 on error)
 * @param mgr pointer to the IPC manager object
 * @param nid value returned by @ref GWEN_IPCManager_AddClient or by
 *   @ref GWEN_IPCManager_GetInRequestData (in variable "ipc/nodeId");
 * @param req DB containing the request
 */
GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_IPCManager_SendRequest(GWEN_IPCMANAGER *mgr,
                                             GWEN_TYPE_UINT32 nid,
                                             GWEN_DB_NODE *req);

/**
 * Sends a request to any connection of the given mark.
 * @return request id of the request created (0 on error)
 * @param mgr pointer to the IPC manager object
 * @param mark mark (as given to @ref GWEN_IPCManager_AddClient or
 *   @ref GWEN_IPCManager_AddServer), 0 matches all
 * @param req DB containing the request
 */
GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_IPCManager_SendMultiRequest(GWEN_IPCMANAGER *mgr,
                                                  GWEN_TYPE_UINT32 mark,
                                                  GWEN_DB_NODE *req);

/**
 * Sends a response to a request received.
 * @return 0 if ok, !=0 on error
 * @param mgr pointer to the IPC manager object
 * @param rid id of the request to which this is a response (as returned
 *   by @ref GWEN_IPCManager_GetNextInRequest)
 * @param rsp DB containing the response
 */
GWENHYWFAR_API
int GWEN_IPCManager_SendResponse(GWEN_IPCMANAGER *mgr,
                                 GWEN_TYPE_UINT32 rid,
                                 GWEN_DB_NODE *rsp);

/**
 * Removes a request.
 * @param mgr pointer to the IPC manager object
 * @param rid id of the request to which this is a response (as returned
 *   by @ref GWEN_IPCManager_GetNextInRequest,
 *   @ref GWEN_IPCManager_SendRequest or
 *   @ref GWEN_IPCManager_SendMultiRequest)
 * @param outbound if 0 then an incoming request is to be removed, otherwise
 *   an outgoing request is to be deleted
 */
GWENHYWFAR_API
int GWEN_IPCManager_RemoveRequest(GWEN_IPCMANAGER *mgr,
                                  GWEN_TYPE_UINT32 rid,
                                  int outbound);

/**
 * Returns the id of the next incoming request with the given mark.
 * This request will then be taken of the list of new requests, so the
 * next call to this function will return another request id (or 0 if there
 * is none left). So you will have to preserve the id returned in order to
 * use it.
 * @param mgr pointer to the IPC manager object
 * @param mark, 0 matches any (see @ref GWEN_IPCManager_AddServer)
 */
GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_IPCManager_GetNextInRequest(GWEN_IPCMANAGER *mgr,
                                                  GWEN_TYPE_UINT32 mark);

/**
 * <p>
 * Returns the GWEN_DB of the incoming request with the given id.
 * The caller takes ownership of the returned DB (if any), so it is up to him
 * to free the data returned.
 * </p>
 * <p>
 * The DB returned contains a special group called <b>IPC</b>, which in turn
 * contains some interesting variables:
 * <ul>
 *   <li><b>nodeId</b>: Id of the node via which this request has been
 *       received</li>
 *   <li><b>msgId</b>: id of the message</li>
 * </ul>
 * </p>
 * @param mgr pointer to the IPC manager object
 * @param rid request id returned by @ref GWEN_IPCManager_GetNextInRequest
 */
GWENHYWFAR_API
GWEN_DB_NODE *GWEN_IPCManager_GetInRequestData(GWEN_IPCMANAGER *mgr,
                                               GWEN_TYPE_UINT32 rid);


/**
 * Returns the next response to the given outbound request. The data returned
 * is taken off the list of responses, so any call to this function will
 * return the next response (if any).
 * The caller takes over ownership of the data returned.
 * @param mgr pointer to the IPC manager object
 * @param rid request id returned by @ref GWEN_IPCManager_SendRequest or
 *        @ref GWEN_IPCManager_SendMultiRequest
 */
GWENHYWFAR_API
GWEN_DB_NODE *GWEN_IPCManager_GetResponseData(GWEN_IPCMANAGER *mgr,
                                              GWEN_TYPE_UINT32 rid);

/**
 * Returns the next response to the given outbound request. The data returned
 * is not taken off the list of responses, so the next call to this function
 * will return the same response (if any).
 * The caller does NOT take over ownership of the data returned.
 * @param mgr pointer to the IPC manager object
 * @param rid request id returned by @ref GWEN_IPCManager_SendRequest or
 *        @ref GWEN_IPCManager_SendMultiRequest
 */
GWENHYWFAR_API
GWEN_DB_NODE *GWEN_IPCManager_PeekResponseData(GWEN_IPCMANAGER *mgr,
                                               GWEN_TYPE_UINT32 rid);


/**
 * Lets the IPC manager do its work.
 * You need to call @ref GWEN_Net_HeartBeat before this function to
 * catch changes in the network connections used.
 */
GWENHYWFAR_API
int GWEN_IPCManager_Work(GWEN_IPCMANAGER *mgr, int maxmsg);


/**
 * Returns a pointer to the connection used by the given node.
 */
GWENHYWFAR_API
GWEN_NETCONNECTION *GWEN_IPCManager_GetConnection(GWEN_IPCMANAGER *mgr,
                                                  GWEN_TYPE_UINT32 nid);

#ifdef __cplusplus
}
#endif

#endif /* GWEN_IPC_H */




