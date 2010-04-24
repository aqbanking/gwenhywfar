/***************************************************************************
    begin       : Fri May 07 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_IPC_H
#define GWEN_IPC_H


/** @file src/ipc/ipc.h
 *
 * This module is now deprecated. It was only used by Libchipcard which now no
 * longer uses it. This module is kinda bloated anyway...
 *
 */


#include <gwenhywfar/types.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/iolayer.h>
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



typedef void (*GWEN_IPCMANAGER_CLIENTDOWN_FN)(GWEN_IPCMANAGER *mgr,
					      uint32_t id,
					      GWEN_IO_LAYER *io,
					      void *user_data);


GWENHYWFAR_API DEPRECATED
GWEN_IPCMANAGER *GWEN_IpcManager_new();

GWENHYWFAR_API DEPRECATED
void GWEN_IpcManager_free(GWEN_IPCMANAGER *mgr);

GWENHYWFAR_API DEPRECATED
void GWEN_IpcManager_Attach(GWEN_IPCMANAGER *mgr);

GWENHYWFAR_API DEPRECATED
void GWEN_IpcManager_Dump(GWEN_IPCMANAGER *mgr, FILE *f, int indent);


GWENHYWFAR_API DEPRECATED
const char *GWEN_IpcManager_GetApplicationName(const GWEN_IPCMANAGER *mgr);

GWENHYWFAR_API DEPRECATED
void GWEN_IpcManager_SetApplicationName(GWEN_IPCMANAGER *mgr,
                                        const char *name);


/**
 * Starts disconnecting the given client/server.
 */
GWENHYWFAR_API DEPRECATED
int GWEN_IpcManager_Disconnect(GWEN_IPCMANAGER *mgr, uint32_t nid);

/**
 * Adds a server to the IPC manager. The IPC manager will immediately
 * enter listen mode on the given transport layer.
 * @return node id of the node created (0 on error)
 * @param mgr pointer to the IPC manager object
 * @param ioBase io layer over which the IPC is to be transfered. This module
 *  stacks some more layers on top of this: LayerPackets-LayerHttp->LayerBuffered.
 *  The given base layer MUST be neither connected nor in listening state)
 * @param mark a value assigned to the new connection. This value can be used
 *   for functions @ref GWEN_IpcManager_GetNextInRequest and
 *   @ref GWEN_IpcManager_SendMultiRequest, it is not otherwise used by
 *   the IPC manager
 */
GWENHYWFAR_API DEPRECATED
uint32_t GWEN_IpcManager_AddServer(GWEN_IPCMANAGER *mgr,
                                   GWEN_IO_LAYER *ioBase,
				   uint32_t mark);

/**
 * Adds a client to the IPC manager. The connection will not be established
 * until needed.
 * @return node id of the node created (0 on error)
 * @param mgr pointer to the IPC manager object
 * @param ioBase io layer over which the IPC is to be transfered. This module
 *  stacks some more layers on top of this: LayerPackets-LayerHttp->LayerBuffered.
 *  The given base layer MUST be neither connected nor in listening state)
 * @param mark a value assigned to the new connection. This value can be used
 *   for functions @ref GWEN_IpcManager_GetNextInRequest and
 *   @ref GWEN_IpcManager_SendMultiRequest, it is not otherwise used by
 *   the IPC manager
 */
GWENHYWFAR_API DEPRECATED
uint32_t GWEN_IpcManager_AddClient(GWEN_IPCMANAGER *mgr,
				   GWEN_IO_LAYER *ioBase,
				   uint32_t mark);
/**
 * Removes a client which was previously added using
 *  @ref GWEN_IpcManager_AddClient.
 * This function removes all pending messages and/or requests for/from this
 * client.
 */
GWENHYWFAR_API DEPRECATED
int GWEN_IpcManager_RemoveClient(GWEN_IPCMANAGER *mgr,
				 uint32_t nid);

/**
 * Sends a request via the given connection.
 * It takes over the ownership of the given GWEN_DB in any case.
 * @return request id of the request created (0 on error)
 * @param mgr pointer to the IPC manager object
 * @param nid value returned by @ref GWEN_IpcManager_AddClient or by
 *   @ref GWEN_IpcManager_GetInRequestData (in variable "ipc/nodeId");
 * @param req DB containing the request
 */
GWENHYWFAR_API DEPRECATED
int GWEN_IpcManager_SendRequest(GWEN_IPCMANAGER *mgr,
				uint32_t nid,
				GWEN_DB_NODE *req,
				uint32_t *pReqId);

/**
 * Sends a response to a request received.
 * @return 0 if ok, !=0 on error
 * @param mgr pointer to the IPC manager object
 * @param rid id of the request to which this is a response (as returned
 *   by @ref GWEN_IpcManager_GetNextInRequest)
 * @param rsp DB containing the response
 */
GWENHYWFAR_API DEPRECATED
int GWEN_IpcManager_SendResponse(GWEN_IPCMANAGER *mgr,
                                 uint32_t rid,
                                 GWEN_DB_NODE *rsp);

/**
 * Removes a request.
 * @param mgr pointer to the IPC manager object
 * @param rid id of the request to which this is a response (as returned
 *   by @ref GWEN_IpcManager_GetNextInRequest,
 *   @ref GWEN_IpcManager_SendRequest or
 *   @ref GWEN_IpcManager_SendMultiRequest)
 * @param outbound if 0 then an incoming request is to be removed, otherwise
 *   an outgoing request is to be deleted
 */
GWENHYWFAR_API DEPRECATED
int GWEN_IpcManager_RemoveRequest(GWEN_IPCMANAGER *mgr,
                                  uint32_t rid,
                                  int outbound);

/**
 * Returns the id of the next incoming request with the given mark.
 * This request will then be taken of the list of new requests, so the
 * next call to this function will return another request id (or 0 if there
 * is none left). So you will have to preserve the id returned in order to
 * use it.
 * @param mgr pointer to the IPC manager object
 * @param mark, 0 matches any (see @ref GWEN_IpcManager_AddServer)
 */
GWENHYWFAR_API DEPRECATED
uint32_t GWEN_IpcManager_GetNextInRequest(GWEN_IPCMANAGER *mgr,
					  uint32_t mark);

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
 * @param rid request id returned by @ref GWEN_IpcManager_GetNextInRequest
 */
GWENHYWFAR_API DEPRECATED
GWEN_DB_NODE *GWEN_IpcManager_GetInRequestData(GWEN_IPCMANAGER *mgr,
                                               uint32_t rid);


/**
 * Returns the next response to the given outbound request. The data returned
 * is taken off the list of responses, so any call to this function will
 * return the next response (if any).
 * The caller takes over ownership of the data returned.
 * @param mgr pointer to the IPC manager object
 * @param rid request id returned by @ref GWEN_IpcManager_SendRequest or
 *        @ref GWEN_IpcManager_SendMultiRequest
 */
GWENHYWFAR_API DEPRECATED
GWEN_DB_NODE *GWEN_IpcManager_GetResponseData(GWEN_IPCMANAGER *mgr,
                                              uint32_t rid);

/**
 * Returns the next response to the given outbound request. The data returned
 * is not taken off the list of responses, so the next call to this function
 * will return the same response (if any).
 * The caller does NOT take over ownership of the data returned.
 * @param mgr pointer to the IPC manager object
 * @param rid request id returned by @ref GWEN_IpcManager_SendRequest or
 *        @ref GWEN_IpcManager_SendMultiRequest
 */
GWENHYWFAR_API DEPRECATED
GWEN_DB_NODE *GWEN_IpcManager_PeekResponseData(GWEN_IPCMANAGER *mgr,
                                               uint32_t rid);


/**
 * Lets the IPC manager do its work.
 * You need to call @ref GWEN_Net2_HeartBeat before this function to
 * catch changes in the network connections used.
 */
GWENHYWFAR_API DEPRECATED
int GWEN_IpcManager_Work(GWEN_IPCMANAGER *mgr);


/**
 * Returns a pointer to the connection used by the given node.
 */
GWENHYWFAR_API DEPRECATED
GWEN_IO_LAYER *GWEN_IpcManager_GetIoLayer(GWEN_IPCMANAGER *mgr, uint32_t nid);

GWENHYWFAR_API DEPRECATED
uint32_t GWEN_IpcManager_GetClientForIoLayer(const GWEN_IPCMANAGER *mgr, const GWEN_IO_LAYER *io);



GWENHYWFAR_API DEPRECATED
void GWEN_IpcManager_SetClientDownFn(GWEN_IPCMANAGER *mgr,
				     GWEN_IPCMANAGER_CLIENTDOWN_FN f,
				     void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* GWEN_IPC_H */




