/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 15 2003
    copyright   : (C) 2003 by Martin Preuss
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


#ifndef GWENHYWFAR_IPCXML_H
#define GWENHYWFAR_IPCXML_H

#include <gwenhywfar/db.h>
#include <gwenhywfar/servicelayer.h>
#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/keymanager.h>
#include <gwenhywfar/ipcxmlsecctx.h>


typedef struct GWEN_IPCXMLREQUEST GWEN_IPCXMLREQUEST;

/** @name IPC Request
 *
 * These functions are only to be used by Gwenhywfar itself, not by an
 * application.
 * They are NOT part of the API.
 */
/*@{*/
GWEN_IPCXMLREQUEST *GWEN_IPCXMLRequest_new();
void GWEN_IPCXMLRequest_free(GWEN_IPCXMLREQUEST *r);

unsigned int GWEN_IPCXMLRequest_GetId(GWEN_IPCXMLREQUEST *r);
void GWEN_IPCXMLRequest_SetId(GWEN_IPCXMLREQUEST *r,
                              unsigned int i);

unsigned int GWEN_IPCXMLRequest_GetMsgLayerId(GWEN_IPCXMLREQUEST *r);
void GWEN_IPCXMLRequest_SetMsgLayerId(GWEN_IPCXMLREQUEST *r,
                                      unsigned int i);

unsigned int GWEN_IPCXMLRequest_GetDialogId(GWEN_IPCXMLREQUEST *r);
void GWEN_IPCXMLRequest_SetDialogId(GWEN_IPCXMLREQUEST *r,
                                    unsigned int i);

unsigned int GWEN_IPCXMLRequest_GetMessageNumber(GWEN_IPCXMLREQUEST *r);
void GWEN_IPCXMLRequest_SetMessageNumber(GWEN_IPCXMLREQUEST *r,
                                         unsigned int i);

unsigned int GWEN_IPCXMLRequest_GetSegmentNumber(GWEN_IPCXMLREQUEST *r);
void GWEN_IPCXMLRequest_SetSegmentNumber(GWEN_IPCXMLREQUEST *r,
                                         unsigned int i);

GWEN_DB_NODE *GWEN_IPCXMLRequest_GetDb(GWEN_IPCXMLREQUEST *r);
void GWEN_IPCXMLRequest_SetDb(GWEN_IPCXMLREQUEST *r,
                              GWEN_DB_NODE *db);
/*@}*/



/**
 * @defgroup MOD_IPCXMLSERVICE IPC Service
 * @short Provides secure interprocess communication
 * @author Martin Preuss<martin@libchipcard.de>
 *
 * The functions in this group provide a secure IPC mechanism.
 * IPC messages are defined in an XML file and parsed by a
 * @ref GWEN_MSGENGINE.
 * This IPC services allows signing and encrpyting messages.
 * You can use TCP sockets or Unix Domain sockets for the communication.
 */
/*@{*/

/**
 * This is the type expected by all functions of this group.
 * You should not bother about the data which is behind this type, it
 * is not part of the API.
 */
typedef struct GWEN_IPCXMLSERVICE GWEN_IPCXMLSERVICE;

/**
 * List of available transport layer types.
 */
typedef enum {
  /** unknown service type */
  GWEN_IPCXMLServiceTypeUnknown=0,
  /** using TCP sockets */
  GWEN_IPCXMLServiceTypeTCP,
  /** using unix domain sockets */
  GWEN_IPCXMLServiceTypeUnix
} GWEN_IPCXMLSERVICE_TYPE;





/** @name Constructor And Destructor
 *
 */
/*@{*/
/**
 * Creates new IPC service.
 * @param scm security context manager. This one will be used to lookup
 * security context objects which are then to perform signing, encrypting,
 * decrypting and so on.
 * You MUST call @ref GWEN_IPCXMLService_free in order to prevent memory
 * leaks (a simple <i>free</i> does not free all.the data associated with
 * an IPC service).
 */
GWEN_IPCXMLSERVICE *GWEN_IPCXMLService_new(GWEN_MSGENGINE *msgEngine,
                                           GWEN_SECCTX_MANAGER *scm);

/**
 * Frees the IPC service and all data associated with it.
 * @param xs Pointer to the service to use
 */
void GWEN_IPCXMLService_free(GWEN_IPCXMLSERVICE *xs);
/*@}*/


/** @name Manage Clients And Servers
 *
 * The functions in this group allow you to add clients and servers to the
 * service. The following definition applies:
 * <table>
 *  <tr>
 *    <td>Client</td>
 *    <td>an active socket which will connect to a running server</td>
 *  </tr>
 *  <tr>
 *    <td>Server</td>
 *    <td>a passive socket which will be listening on a given socket
 *      for incoming client connections</td>
 *  </tr>
 * </table>
 */
/*@{*/
/**
 * Adds a server to this service. A server is a passive socket which
 * listens for incoming connections and accepts them.
 * @return new connection id (or 0 on error)
 * @param xs Pointer to the service to use
 * @param localName name of the local context. This is the name of the
 * owner of the local keys. These keys are looked up in the service's
 * context manager.
 * @param userMark a special mark assigned by the application.
 * E.g. if you want to add multiple servers which all provide different
 * services or have different security constraints you can distinguish
 * them by using the userMark. All incoming connections accepted by this
 * server will have this userMark assigned as well.
 * @param addr IP address to bind to. This can be either in 3-dot notation
 * (a.b.c.d) or a hostname (FQDN) which will then be looked up using DNS.
 * use "0.0.0.0" to bind to any local address.
 * @param port TCP port to bin to. Of course this is only needed with
 * TCP services, for unix domain sockets this is ignored
 * @param flags flags for the connection layer (reserved for future use)
 */
unsigned int GWEN_IPCXMLService_AddServer(GWEN_IPCXMLSERVICE *xs,
                                          GWEN_IPCXMLSERVICE_TYPE st,
                                          const char *localName,
                                          unsigned int userMark,
                                          const char *addr,
                                          unsigned int port,
                                          unsigned int flags);

/**
 * Adds a client to this service. A client is an active socket which
 * actively connects to a running server.
 * @return new connection id (or 0 on error)
 * @param xs Pointer to the service to use
 * @param localName name of the local context. This is the name of the
 * owner of the local keys. These keys are looked up in the service's
 * context manager.
 * @param userMark a special mark assigned by the application.
 * E.g. if you want to add multiple clients which all use different
 * services or have different security constraints you can distinguish
 * them by using the userMark.
 * @param addr IP address to bind to. This can be either in 3-dot notation
 * (a.b.c.d) or a hostname (FQDN) which will then be looked up using DNS.
 * use "0.0.0.0" to bind to any local address.
 * @param port TCP port to bin to. Of course this is only needed with
 * TCP services, for unix domain sockets this is ignored
 * @param flags flags for the connection layer (reserved for future use)
 */
unsigned int GWEN_IPCXMLService_AddClient(GWEN_IPCXMLSERVICE *xs,
                                          GWEN_IPCXMLSERVICE_TYPE st,
                                          const char *localContext,
                                          unsigned int userMark,
                                          const char *addr,
                                          unsigned int port,
                                          unsigned int flags);
/**
 * Closes all matching connections belonging to the given service layer.
 * @param xs Pointer to the service to use
 * @param id connection id (assigned upon creating it. 0 matches any)
 * @param userMark a special mark assigned by the application (0 matches any)
 * @param force if !=0 then the connection will be closed physically
 *        immediately. Otherwise the ConnectionLayer is allowed to do some
 *        closing handshaking.
 */
void GWEN_IPCXMLService_Close(GWEN_IPCXMLSERVICE *xs,
                              unsigned int id,
                              unsigned int userMark,
                              int force);

/*@}*/


/** @name Request Management
 *
 */
/*@{*/
/**
 * Creates a new request and enqueues it for sending to the peer.
 * @param xs Pointer to the service to use
 * @param clid Id of the client/server retrieved via
 * @ref GWEN_IPCXMLService_AddClient or @ref GWEN_IPCXMLService_AddServer
 * @param requestName name of the request. This name is looked up by the
 * message engine which creates a message from an XML file.
 * @param requestVersion this is also used by the message engine to find
 * the correct version of the message to create.
 * @param db GWEN_DB_NODE containing the parameters belonging to the given
 * request
 * @param flush if not zero then the output queue will be flushed thus
 * effectively sending the request to the peer. If zero then the request
 * will only be enqueued but not send. You can flush the queue by calling
 * @ref GWEN_IPCXMLService_Flush.
 */
unsigned int GWEN_IPCXMLService_AddRequest(GWEN_IPCXMLSERVICE *xs,
                                           unsigned int clid,
                                           const char *requestName,
                                           unsigned int requestVersion,
                                           GWEN_DB_NODE *db,
                                           int flush);

/**
 * Creates a response to an incoming request and enqueues it for sending to
 * the peer.
 * @param xs Pointer to the service to use
 * @param requestId id of the incoming request to which a response is
 * to be created (retrieved via @ref GWEN_IPCXMLService_GetNextRequest).
 * @ref GWEN_IPCXMLService_AddClient or @ref GWEN_IPCXMLService_AddServer
 * @param name name of the message. This name is looked up by the
 * message engine which creates a message from an XML file.
 * @param version this is also used by the message engine to find
 * the correct version of the message to create.
 * @param db GWEN_DB_NODE containing the parameters belonging to the given
 * request
 * @param flush if not zero then the output queue will be flushed thus
 * effectively sending the request to the peer. If zero then the request
 * will only be enqueued but not send. You can flush the queue by calling
 * @ref GWEN_IPCXMLService_Flush.
 */
GWEN_ERRORCODE GWEN_IPCXMLService_AddResponse(GWEN_IPCXMLSERVICE *xs,
                                              unsigned int requestId,
                                              const char *name,
                                              unsigned int version,
                                              GWEN_DB_NODE *db,
                                              int flush);
/**
 * Sends all pending requests/responses for the given connection to the peer.
 * @param xs Pointer to the service to use
 * @param clid Id of the client/server retrieved via
 * @ref GWEN_IPCXMLService_AddClient or @ref GWEN_IPCXMLService_AddServer
 */
GWEN_ERRORCODE GWEN_IPCXMLService_Flush(GWEN_IPCXMLSERVICE *xs,
                                        unsigned int clid);

/**
 * Returns the id of the next incoming request.
 * @return request id (0 if none)
 * @param xs Pointer to the service to use
 */
unsigned int GWEN_IPCXMLService_GetNextRequest(GWEN_IPCXMLSERVICE *xs);

/**
 * Removes a given request.
 * @param xs Pointer to the service to use
 * @param requestId id retrieved via @ref GWEN_IPCXMLService_GetNextRequest
 */
GWEN_ERRORCODE GWEN_IPCXMLService_DeleteRequest(GWEN_IPCXMLSERVICE *xs,
                                                unsigned int requestId);

/**
 * Returns the first DB group from the given incoming request and unlinks it
 * from that request.
 * This DB group is a part of the request data sent to us.
 * The caller is reponsible for freeing the returned group by calling
 * @ref GWEN_DB_Group_free.
 * @param xs Pointer to the service to use
 * @param requestId id retrieved via @ref GWEN_IPCXMLService_GetNextRequest
 */
GWEN_DB_NODE *GWEN_IPCXMLService_GetRequestData(GWEN_IPCXMLSERVICE *xs,
                                                unsigned int requestId);

/**
 * Returns the first DB group from the given request and unlinks it from that
 * request.
 * This DB group is a part of the response returned by the peer (to which
 * the request has originally been sent).
 * The caller is reponsible for freeing the returned group by calling
 * @ref GWEN_DB_Group_free.
 * @param xs Pointer to the service to use
 * @param requestId id retrieved via @ref GWEN_IPCXMLService_GetNextRequest
 */
GWEN_DB_NODE *GWEN_IPCXMLService_GetResponseData(GWEN_IPCXMLSERVICE *xs,
                                                 unsigned int requestId);


/*@}*/



/** @name Connection Security
 *
 *
 */
/*@{*/
GWEN_ERRORCODE GWEN_IPCXMLService_SetSecurityFlags(GWEN_IPCXMLSERVICE *xs,
                                                   unsigned int clid,
                                                   unsigned int flags);

const char *GWEN_IPCXMLService_GetLocalName(GWEN_IPCXMLSERVICE *xs,
                                            unsigned int clid);
const char *GWEN_IPCXMLService_GetRemoteName(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int clid);
void GWEN_IPCXMLService_SetRemoteName(GWEN_IPCXMLSERVICE *xs,
                                      unsigned int clid,
                                      const char *s);
/*@}*/



/** @name Work
 *
 *
 */
/*@{*/
GWEN_ERRORCODE GWEN_IPCXMLService_Work(GWEN_IPCXMLSERVICE *xs,
                                       int timeout);

GWEN_ERRORCODE GWEN_IPCXMLService_HandleMsgs(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int userMark,
                                             int maxmsgs);
/*@}*/

/*@}*/ /* defgroup */



#endif



