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
 * @defgroup MOD_IPCXMLSERVICE IPCXML Service
 * @ingroup MOD_IPC_HIGHLEVEL
 * @short Provides secure interprocess communication
 * @author Martin Preuss<martin@libchipcard.de>
 *
 * <p>The functions in this group provide a secure IPC mechanism.
 * IPC messages are defined in an XML file and parsed by a
 * @ref GWEN_MSGENGINE.
 * This IPC services allows signing and encrpyting messages.
 * You can use TCP sockets or unix domain sockets for communication.</p>
 *
 * <p>The usual way of using this module is this:
 * <ol type="1" >
 *  <li>
 *   setup a @ref GWEN_MSGENGINE (create it, load appropriate XML files etc.
 *   See @ref MOD_MSGENGINE for details)
 *  </li>
 *  <li>
 *   setup a @ref GWEN_SECCTX_MANAGER (create it, load and add security
 *   contexts etc. See @ref MOD_SECCTX_MANAGER for details)
 *  </li>
 *  <li>
 *   call @ref GWEN_IPCXMLService_AddServer or
 *   @ref GWEN_IPCXMLService_AddClient as often as required
 *  </li>
 *  <li>
 *   Server: call @ref GWEN_IPCXMLService_SetRemoteName as soon as you know
 *   to whom you are talking (when using @ref GWEN_IPCXMLService_AddClient
 *   the remote name is already known)
 *  </li>
 *  <li>
 *   call @ref GWEN_IPCXMLService_SetSecurityFlags to choose whether you want
 *   encryption and signing take place
 *  </li>
 *  <li>
 *   start sending or awaiting requests and responses by calling one of
 *   @ref GWEN_IPCXMLService_AddRequest, @ref GWEN_IPCXMLService_AddResponse
 *   and @ref GWEN_IPCXMLService_GetNextRequest.<br>
 *   Please remember to delete
 *   requests which have been completely handled by calling
 *   @ref GWEN_IPCXMLService_DeleteRequest.
 *  </li>
 * </ol>
 * </p>
 */
/*@{*/

/**
 * This request id will be returned by @ref GWEN_IPCXMLService_AddRequest if
 * the flag @ref GWEN_IPCXML_REQUESTFLAGS_FORGET is given. This is just to
 * return a value different from zero if there is no error.
 * With this flag set no request will really be created, but we need to
 * return something to the application ;-)
 */
#define GWEN_IPCXML_REQUEST_ANYID 0xffffffff


/** @name Macros for Inspecting Result Codes
 *
 */
/*@{*/
/** Checks whether the result code is generally ok */
#define GWEN_IPCXML_RESULT_IS_OK(rcode) (rcode<9000)
/** Checks whether the result code represents an information */
#define GWEN_IPCXML_RESULT_IS_INFO(rcode) (rcode>=1000 && rcode<2000)
/** Checks whether the result code represents a warning */
#define GWEN_IPCXML_RESULT_IS_WARNING(rcode) (rcode>=3000 && rcode<4000)
/** Checks whether the result code represents an error */
#define GWEN_IPCXML_RESULT_IS_ERROR(rcode) (rcode>=9000)
/*@}*/


/** @name Request Flags
 *
 */
/*@{*/
/**
 * if this is set then the output queue will be flushed thus effectively
 * sending the request to the peer. Otherwisethe request will only be
 * enqueued but not send. You can flush the queue by calling
 * @ref GWEN_IPCXMLService_Flush.
 */
#define GWEN_IPCXML_REQUESTFLAGS_FLUSH  0x0001
/**
 * If this bit is set then the request will be created but not remembered.
 * Use this for requests for which you don't expect responses. Please note
 * that the request id returned by @ref GWEN_IPCXMLService_AddRequest will
 * not be a usable, it will only be @ref GWEN_IPCXML_REQUEST_ANYID
 */
#define GWEN_IPCXML_REQUESTFLAGS_FORGET 0x0002
/*@}*/


/**
 * This is the type expected by all functions of this group.
 * You should not bother about the data which is behind this type, it
 * is not part of the API.
 */
typedef struct GWEN_IPCXMLSERVICE GWEN_IPCXMLSERVICE;

/**
 * This function will be called whenever a connection is established.
 * The application can use this function to keep track of connections.
 * @param xs IPC service the connection belongs to
 * @param clid connection id
 */
typedef void (*GWEN_IPCXMLSERVICE_CONNUP_FN)(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int clid);
/**
 * This function will be called whenever a connection is lost.
 * The application can use this function to keep track of connections.
 * @param xs IPC service the connection belongs to
 * @param clid connection id
 */
typedef void (*GWEN_IPCXMLSERVICE_CONNDOWN_FN)(GWEN_IPCXMLSERVICE *xs,
                                               unsigned int clid);


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
 * You MUST call @ref GWEN_IPCXMLService_free in order to prevent memory
 * leaks (a simple <i>free</i> does not free all.the data associated with
 * an IPC service).
 * @param msgEngine pointer to a message engine to be used for creating
 * and parsing the IPC messages.
 * This function DOES NOT take over ownership of that pointer !
 * @param scm security context manager. This one will be used to lookup
 * security context objects which are then to perform signing, encrypting,
 * decrypting and so on.
 * This function DOES NOT take over ownership of that pointer !
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
 * @param st connection type (in this case the socket type to be used)
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
 * @param st connection type (in this case the socket type to be used)
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
                                          const char *localName,
                                          const char *remoteName,
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
 * @return id of the new request, 0 on error or @ref GWEN_IPCXML_REQUEST_ANYID
 * @param xs Pointer to the service to use
 * @param clid Id of the client/server retrieved via
 * @ref GWEN_IPCXMLService_AddClient or @ref GWEN_IPCXMLService_AddServer
 * @param requestName name of the request. This name is looked up by the
 * message engine which creates a message from an XML file.
 * @param requestVersion this is also used by the message engine to find
 * the correct version of the message to create.
 * @param db GWEN_DB_NODE containing the parameters belonging to the given
 * request
 * @param flags see @ref GWEN_IPCXML_REQUESTFLAGS_FLUSH
 */
unsigned int GWEN_IPCXMLService_AddRequest(GWEN_IPCXMLSERVICE *xs,
                                           unsigned int clid,
                                           const char *requestName,
                                           unsigned int requestVersion,
                                           GWEN_DB_NODE *db,
                                           unsigned int flags);

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
 * @param flags see @ref GWEN_IPCXML_REQUESTFLAGS_FLUSH
 */
GWEN_ERRORCODE GWEN_IPCXMLService_AddResponse(GWEN_IPCXMLSERVICE *xs,
                                              unsigned int requestId,
                                              const char *name,
                                              unsigned int version,
                                              GWEN_DB_NODE *db,
                                              unsigned int flags);
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
 * Returns the id of the connection via which the given request reached us.
 * @param xs Pointer to the service to use
 * @param rqid id retrieved via @ref GWEN_IPCXMLService_GetNextRequest
 */
unsigned int GWEN_IPCXMLService_GetRequestConnection(GWEN_IPCXMLSERVICE *xs,
                                                     unsigned int rqid);


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


/**
 * Returns the first DB group from the given request without
 * unlinking it from that request.
 * This DB group is a part of the response returned by the peer (to which
 * the request has originally been sent).
 * The object returned MUST NOT be freed !
 * @ref GWEN_DB_Group_free.
 * @param xs Pointer to the service to use
 * @param requestId id retrieved via @ref GWEN_IPCXMLService_GetNextRequest
 */
GWEN_DB_NODE *GWEN_IPCXMLService_PeekResponseData(GWEN_IPCXMLSERVICE *xs,
                                                  unsigned int requestId);


/*@}*/



/** @name Connection Security
 *
 *
 */
/*@{*/
/**
 * Set security flags on a given connection.
 * @param xs Pointer to the service to use
 * @param clid Id of the client/server retrieved via
 * @ref GWEN_IPCXMLService_AddClient or @ref GWEN_IPCXMLService_AddServer
 * @param flags flags to set as define in gwenhywfar/hbcimsg.h.
 * Flags can be one or more of the follwoing values (OR combined):
 * <ul>
 *  <li>@ref GWEN_HBCIMSG_FLAGS_SIGN (sign outgoing messages)</li>
 *  <li>@ref GWEN_HBCIMSG_FLAGS_CRYPT (encrypt outgoing messages)</li>
 * </ul>
 */
GWEN_ERRORCODE GWEN_IPCXMLService_SetSecurityFlags(GWEN_IPCXMLSERVICE *xs,
                                                   unsigned int clid,
                                                   unsigned int flags);

/**
 * Returns the name of the service (e.g. for HBCI with banks this is the
 * bank code. Applications such as servers will use a name such as
 * <i>chipcardd</i> for Libchipcards chipcard daemon.
 */
const char *GWEN_IPCXMLService_GetServiceCode(GWEN_IPCXMLSERVICE *xs,
                                              unsigned int clid);

/**
 * Returns the local name for the given connection.
 * The local name determines which context (and which keys) are to be used
 * when decrypting received messages and signing outgoing messages.
 * This local name is set upon creation of the connection via
 *  @ref GWEN_IPCXMLService_AddClient or @ref GWEN_IPCXMLService_AddServer
 * @param xs Pointer to the service to use
 * @param clid Id of the client/server retrieved via
 *  @ref GWEN_IPCXMLService_AddClient or @ref GWEN_IPCXMLService_AddServer
 */
const char *GWEN_IPCXMLService_GetLocalName(GWEN_IPCXMLSERVICE *xs,
                                            unsigned int clid);

/**
 * Returns the remote name for the connection.
 * The remote name determines whcih context (and keys) are to be used when
 * encrypting outgoing messages and verifying signatures of incoming
 * messages. It is reset whenever the physical connection is lost.
 * @param xs Pointer to the service to use
 * @param clid Id of the client/server retrieved via
 *  @ref GWEN_IPCXMLService_AddClient or @ref GWEN_IPCXMLService_AddServer
 */
const char *GWEN_IPCXMLService_GetRemoteName(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int clid);

/**
 * Sets the remote name for the given connection.
 * The remote name determines which context (and keys) are to be used when
 * encrypting outgoing messages.
 * It is reset whenever the physical connection is lost.
 * You MUST set the remote name if you want to enter a secure mode (using
 * signatures and encryption).
 * @param xs Pointer to the service to use
 * @param clid Id of the client/server retrieved via
 *  @ref GWEN_IPCXMLService_AddClient or @ref GWEN_IPCXMLService_AddServer
 * @param s new remote name
 */
void GWEN_IPCXMLService_SetRemoteName(GWEN_IPCXMLSERVICE *xs,
                                      unsigned int clid,
                                      const char *s);

/**
 * Returns a pointer to the local sign key currently used by the given
 * connection.
 * @param xs Pointer to the service to use
 * @param clid Id of the client/server retrieved via
 *  @ref GWEN_IPCXMLService_AddClient or @ref GWEN_IPCXMLService_AddServer
 */
const GWEN_CRYPTKEY *GWEN_IPCXMLService_GetSignKey(GWEN_IPCXMLSERVICE *xs,
                                                   unsigned int clid);

/**
 * Returns a pointer to the local crypt key currently used by the given
 * connection.
 * @param xs Pointer to the service to use
 * @param clid Id of the client/server retrieved via
 *  @ref GWEN_IPCXMLService_AddClient or @ref GWEN_IPCXMLService_AddServer
 */
const GWEN_CRYPTKEY *GWEN_IPCXMLService_GetCryptKey(GWEN_IPCXMLSERVICE *xs,
                                                    unsigned int clid);


/*@}*/



/** @name Work
 *
 *
 * <p>Since the IPC mechanism provided by a GWEN_IPCXMLSERVICE is plain
 * single-threaded your application needs to periodically give this library
 * an opportunity to do its work.</p>
 * <p>This group contains functions which do the input/output and functions
 * which handle incoming messages by sorting them into internal request
 * lists.</p>
 */
/*@{*/
/**
 * This function does all the physical reading and writing. It also
 * handles incoming connections. You should call this function every once in a
 * while if you want sending/receiving messages to actually take place.
 * @param xs Pointer to the service to use
 * @param timeout time to wait for sockets in milliseconds (in most cases this
 * function uses the system call <i>select</i>, in this case this parameter
 * is used as a timeout value). If <i>0</i> then this function won't wait
 * at all, if <i>-1</i> it will probably wait forever (or just until any
 * socket is readable or writable ;-).
 */
GWEN_ERRORCODE GWEN_IPCXMLService_Work(GWEN_IPCXMLSERVICE *xs,
                                       int timeout);

/**
 * This function checks for messages received by the message layers and
 * handles them. If an incoming message is a response to a previously
 * sent request then it will be sorted into that request as a response.
 * Otherwise the message will be enqueued as a new incoming request.
 * Messages which pretend to be a response but for which no request can
 * be found (either because there never was such a request or the request has
 * meanwhile been deleted) are silently dropped (well, not completely silenty,
 * they will be logged but dismissed).
 */
GWEN_ERRORCODE GWEN_IPCXMLService_HandleMsgs(GWEN_IPCXMLSERVICE *xs,
                                             unsigned int userMark,
                                             unsigned int maxmsgs);
/*@}*/ /* name */

/** @name Extending IPC Service
 *
 * Gwenhywfars IPC module provides some callback functions which can be used
 * to inform the application about connections going up or down.
 * Most server applications will need these functions too keep track of
 * the state of the connections (e.g. to release ressources when the
 * connection to a client is lost).
 */
/*@{*/
/**
 * You can setup a function that will be called whenever a connection is
 * established.
 */
void GWEN_IPCXMLService_SetConnectionUpFn(GWEN_IPCXMLSERVICE *xs,
                                          GWEN_IPCXMLSERVICE_CONNUP_FN fn);
/**
 * You can setup a function that will be called whenever connection is
 * lost.
 */
void
  GWEN_IPCXMLService_SetConnectionDownFn(GWEN_IPCXMLSERVICE *xs,
                                         GWEN_IPCXMLSERVICE_CONNDOWN_FN fn);
/*@}*/ /* name */


/*@}*/ /* defgroup */



#endif



