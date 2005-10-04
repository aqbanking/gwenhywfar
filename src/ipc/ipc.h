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
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/list2.h>

#include <time.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name Error codes
 *
 */
  /*@{*/
#define GWEN_IPC_ERROR_CODES   0x80000000
#define GWEN_IPC_ERROR_GENERIC 0x80000001
#define GWEN_IPC_ERROR_CONNERR 0x80000002
#define GWEN_IPC_ERROR_TIMEOUT 0x80000003
/*@}*/



typedef struct GWEN_IPCMANAGER GWEN_IPCMANAGER;
typedef struct GWEN_IPCREQUEST GWEN_IPCREQUEST;

GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_IPCREQUEST, GWENHYWFAR_API)
GWEN_LIST2_FUNCTION_LIB_DEFS(GWEN_IPCREQUEST, GWEN_IPCRequest, GWENHYWFAR_API)


GWENHYWFAR_API
GWEN_IPCMANAGER *GWEN_IPCManager_new();

GWENHYWFAR_API
void GWEN_IPCManager_free(GWEN_IPCMANAGER *mgr);

GWENHYWFAR_API
void GWEN_IPCManager_Attach(GWEN_IPCMANAGER *mgr);

GWENHYWFAR_API
void GWEN_IPCManager_Dump(GWEN_IPCMANAGER *mgr, FILE *f, int indent);


GWENHYWFAR_API
const char *GWEN_IPCManager_GetApplicationName(const GWEN_IPCMANAGER *mgr);

GWENHYWFAR_API
void GWEN_IPCManager_SetApplicationName(GWEN_IPCMANAGER *mgr,
                                        const char *name);


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
 * Updates the connection status and returns it.
 * You can use this function to check whether a connection is broken.
 */
GWENHYWFAR_API
GWEN_NETTRANSPORT_STATUS
GWEN_IPCManager_CheckConnection(GWEN_IPCMANAGER *mgr, GWEN_TYPE_UINT32 nid);


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
 * Removes a client which was previously added using
 *  @ref GWEN_IPCManager_AddClient.
 * This function removes all pending messages and/or requests for/from this
 * client.
 */
GWENHYWFAR_API
int GWEN_IPCManager_RemoveClient(GWEN_IPCMANAGER *mgr,
				 GWEN_TYPE_UINT32 nid);

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
 * The IPC manager remains the owner of the DB returned.
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

GWENHYWFAR_API
GWEN_TYPE_UINT32
GWEN_IPCManager_GetClientForConnection(const GWEN_IPCMANAGER *mgr,
				       const GWEN_NETCONNECTION *conn);




/**
 * Creates a new request for the given IPC node (client/server).
 * This request is not yet enqueued. The caller becomes
 * the owner of the object returned (if any). He/she must call
 * @ref GWEN_IPCManager_DestroyRequestR on it later (or let the IPC manager
 * take over the object via @ref GWEN_IPCManager_SendRequestR).
 * This function takes over the ownership of the given GWEN_DB_NODE in any
 * case.
 */
GWENHYWFAR_API
GWEN_IPCREQUEST *GWEN_IPCManager_CreateRequestR(GWEN_IPCMANAGER *mgr,
                                                GWEN_DB_NODE *dbReq);

/**
 * Destroys a request which is owned by the caller.
 * You MUST NOT call this function for requests which you don't own.
 */
GWENHYWFAR_API
void GWEN_IPCManager_DestroyRequestR(GWEN_IPCMANAGER *mgr,
                                     GWEN_IPCREQUEST *r);

/**
 * This function increments an internal reference counter of the given
 * request. As a result you will need an additional call to
 * @ref GWEN_IPCManager_DestroyRequestR to actually free the request.
 * Use this function if you refer to this request in different parts of
 * your code.
 * Obviously for every call to this function a matching number of
 * calls to @ref GWEN_IPCManager_DestroyRequestR is necessary to really
 * free this request.
 */
GWENHYWFAR_API
void GWEN_IPCManager_AttachRequestR(GWEN_IPCMANAGER *mgr,
                                    GWEN_IPCREQUEST *r);


/**
 * Sends a request previously created by @ref GWEN_IPCManager_CreateRequestR.
 * The caller still remains the owner of the request.
 * However, even if the owner calls @ref GWEN_IPCManager_DestroyRequestR()
 * on this request to delete it the IPC manager keeps it around as long as
 * it needs to. So if you really want to remove the request from the reach
 * of the IPC manager you will have to call
 * @ref GWEN_IPCManager_TakeRequestR() before calling
 * @ref GWEN_IPCManager_DestroyRequestR().
 */
GWENHYWFAR_API
int GWEN_IPCManager_SendRequestR(GWEN_IPCMANAGER *mgr,
                                 GWEN_TYPE_UINT32 nid,
                                 GWEN_IPCREQUEST *r);

/**
 * Sends a response to request received using
 * @ref GWEN_IPCManager_GetNextInRequestR.
 */
GWENHYWFAR_API
int GWEN_IPCManager_SendResponseR(GWEN_IPCMANAGER *mgr,
                                  GWEN_IPCREQUEST *r,
                                  GWEN_DB_NODE *rsp);


/**
 * This action removes the request from the reach of the IPC manager,
 * so no further responses to this request will be received afterwards.
 * So obviously you should only use this function with incoming requests
 * (obtained via @ref GWEN_IPCManager_GetNextInRequestR) or with finished
 * outgoing requests.
 * This function makes the caller responsible for calling
 * @ref GWEN_IPCManager_DestroyRequestR to free it (thus preventing
 * memory leaks).
 */
GWENHYWFAR_API
int GWEN_IPCManager_TakeRequestR(GWEN_IPCMANAGER *mgr,
                                 GWEN_IPCREQUEST *r);

/**
 * Returns the next incoming request which comes from an IPC node (client,
 * server) with the given mark (or from any node if mark is 0).
 * The caller becomes the new owner of the request returned (if any), but
 * the IPC manager still holds a handle on the request returned.
 * You can remove the request returned from the reach of the IPC manager
 * by calling @ref GWEN_IPCManager_TakeRequestR().
 * The caller is in any case responsible for calling
 * @ref GWEN_IPCManager_DestroyRequestR when he/she is finished with that
 * object.
 */
GWENHYWFAR_API
GWEN_IPCREQUEST *GWEN_IPCManager_GetNextInRequestR(GWEN_IPCMANAGER *mgr,
                                                   GWEN_TYPE_UINT32 mark);


GWENHYWFAR_API
GWEN_DB_NODE *GWEN_IPCManager_GetInRequestDataR(GWEN_IPCMANAGER *mgr,
                                                GWEN_IPCREQUEST *r);

GWEN_TYPE_UINT32
GWEN_IPCManager_GetInRequestSenderR(GWEN_IPCMANAGER *mgr,
                                    const GWEN_IPCREQUEST *r);


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
GWEN_DB_NODE *GWEN_IPCManager_GetResponseDataR(GWEN_IPCMANAGER *mgr,
                                               GWEN_IPCREQUEST *r);

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
GWEN_DB_NODE *GWEN_IPCManager_PeekResponseDataR(GWEN_IPCMANAGER *mgr,
                                               GWEN_IPCREQUEST *r);



#ifdef __cplusplus
}
#endif

#endif /* GWEN_IPC_H */




