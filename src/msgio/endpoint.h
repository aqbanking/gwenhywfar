/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_H
#define GWEN_MSG_ENDPOINT_H


#include <gwenhywfar/inherit.h>
#include <gwenhywfar/list.h>



#define GWEN_MSG_ENDPOINT_FLAGS_DELETE           0x80000000u
#define GWEN_MSG_ENDPOINT_FLAGS_NOIO             0x40000000u
#define GWEN_MSG_ENDPOINT_FLAGS_DISCONNECTED     0x20000000u
#define GWEN_MSG_ENDPOINT_FLAGS_DELONDISCONNECT  0x10000000u


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Object which can send and receive messages (base class).
 *
 */
typedef struct GWEN_MSG_ENDPOINT GWEN_MSG_ENDPOINT;
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_MSG_ENDPOINT, GWENHYWFAR_API)
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_MSG_ENDPOINT, GWEN_MsgEndpoint, GWENHYWFAR_API)

#ifdef __cplusplus
}
#endif


#include <gwenhywfar/endpointmgr.h>
#include <gwenhywfar/msg.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup MOD_MSGENDPOINT Message Sender/Receiver
 * @ingroup MOD_MSGIO
 *
 * @brief This file contains the definition of a GWEN_MSG_ENDPOINT, an object which
 * can send and receive messages (base class).
 *
 */
/*@{*/


/** @name Virtual Function Definitions
 *
 */
/*@{*/
typedef int (*GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
typedef int (*GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);
typedef void (*GWEN_MSG_ENDPOINT_RUN_FN)(GWEN_MSG_ENDPOINT *ep);
typedef GWEN_MSG_ENDPOINT* (*GWEN_MSG_ENDPOINT_CREATECHILD_FN)(GWEN_MSG_ENDPOINT *ep);
typedef int (*GWEN_MSG_ENDPOINT_GET_READFD_FN)(GWEN_MSG_ENDPOINT *ep);
typedef int (*GWEN_MSG_ENDPOINT_GET_WRITEFD_FN)(GWEN_MSG_ENDPOINT *ep);
typedef void (*GWEN_MSG_ENDPOINT_PROC_OUTMSG_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);
typedef int (*GWEN_MSG_ENDPOINT_ISMSGCOMPLETE_FN)(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);
/*@}*/



/** @name Constructor, Destructor
 *
 */
/*@{*/

/**
 * Constructor.
 *
 * @return endpoint created (NULL on error)
 * @param name name of this endpoint type (e.g. "tcp client"), for debugging purposes
 * @param groupId can be used by applications or derived endpoint manager classes to differentiate
 *  between multiple message types (not used internally)
 */
GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_MsgEndpoint_new(const char *name, int groupId);

/**
 * Destructor.
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_free(GWEN_MSG_ENDPOINT *ep);
/*@}*/


/** @name Getter, Setter
 *
 */
/*@{*/

/**
 * Groupid is a variable completely free to use by implementations.
 * The internal implementations of some virtual functions use this value for newly received
 * messages (see @ref GWEN_Msg_GetGroupId).
 *
 * @return groupid set in the constructor
 * @param ep endpoint
 */
GWENHYWFAR_API int GWEN_MsgEndpoint_GetGroupId(const GWEN_MSG_ENDPOINT *ep);

/**
 * The endpoint class stores a file descriptor on behalf of derived classes. This file descriptor is
 * not used by this class directly. When sampling file descriptors for the "select" call internally always
 * the virtual functions @ref GWEN_MsgEndpoint_GetReadFd and @ref GWEN_MsgEndpoint_GetWriteFd are used.
 * @return stored file descriptor (-1 on error or if not set)
 */
GWENHYWFAR_API int GWEN_MsgEndpoint_GetFd(const GWEN_MSG_ENDPOINT *ep);

/**
 * Set file descriptor.
 *
 * @param ep endpoint
 * @param fd file descriptor to store (use -1 for invalid descriptor)
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_SetFd(GWEN_MSG_ENDPOINT *ep, int fd);

/**
 * Return name of this endpoint class (used for debugging purposes).
 *
 * @return class name as set by the constructor
 * @param ep endpoint
 */
GWENHYWFAR_API const char *GWEN_MsgEndpoint_GetName(const GWEN_MSG_ENDPOINT *ep);


/**
 * Default maximum buffer size when creating message objects (see @ref GWEN_MSG).
 *
 * @return default maximum buffer size
 * @param ep endpoint
 */
GWENHYWFAR_API uint32_t GWEN_MsgEndpoint_GetDefaultBufferSize(const GWEN_MSG_ENDPOINT *ep);

/**
 * Set default maximum buffer size for messages.
 *
 * @param ep endpoint
 * @param v size in bytes
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_SetDefaultBufferSize(GWEN_MSG_ENDPOINT *ep, uint32_t v);

/**
 * Get flags (see @ref GWEN_MSG_ENDPOINT_FLAGS_DELETE and following).
 *
 * @return current flags
 * @param ep endpoint
 */
GWENHYWFAR_API uint32_t GWEN_MsgEndpoint_GetFlags(const GWEN_MSG_ENDPOINT *ep);

/**
 * Set new flags.
 *
 * @param ep endpoint
 * @param f new flags
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_SetFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f);

/**
 * Add flags.
 *
 * @param ep endpoint
 * @param f flags to add to current flags
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_AddFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f);

/**
 * Clear given flags.
 *
 * @param ep endpoint
 * @param f flags to clear from current flags
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_DelFlags(GWEN_MSG_ENDPOINT *ep, uint32_t f);
/*@}*/


/** @name Message Management
 *
 * Endpoints have one list for received messages and another list for outbound messages.
 * This base class doesn't have any knowledge of the format of messages exchanged, this is up
 * to derived endpoint classes.
 */
/*@{*/

/**
 * Get list of received messages. The endpoint object keeps ownership of the list returned.
 *
 * @return list of received messages
 * @param ep endpoint
 */
GWENHYWFAR_API GWEN_MSG_LIST *GWEN_MsgEndpoint_GetReceivedMessageList(const GWEN_MSG_ENDPOINT *ep);

/**
 * Get list of outbound messages. The endpoint object keeps ownership of the list returned.
 *
 * @return list of outbound messages
 * @param ep endpoint
 */
GWENHYWFAR_API GWEN_MSG_LIST *GWEN_MsgEndpoint_GetSendMessageList(const GWEN_MSG_ENDPOINT *ep);

/**
 * Add a received message. Mainly called from inside implementations of the virtual
 * function @ref GWEN_MsgEndpoint_HandleReadable.
 * Messages are directly added to the internal list without further handling.
 *
 * @param ep endpoint
 * @param m message to add (endpoint takes over ownership
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_AddReceivedMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);

/**
 * Get the first message from the list of received messages and remove it from the list
 * The caller takes over ownership of the message returned (if any), free it
 * via @ref GWEN_Msg_free.
 *
 * @return first message taken from the incoming list (NULL if none)
 * @param ep endpoint
 */
GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint_TakeFirstReceivedMessage(GWEN_MSG_ENDPOINT *ep);

/**
 * Add an outbound message.
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_AddSendMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);

/**
 * Return the first outbound message. The object returned (if any) remains in the list
 * of outbound messages and the endpoint object keeps ownership.
 *
 * @return first message in the list of outbound messages (NULL if none)
 * @param ep endpoint
 */
GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint_GetFirstSendMessage(const GWEN_MSG_ENDPOINT *ep);

/**
 * Check whether there are outbound messages in the internal list.
 *
 * @return 1 if there is at least one outbound message, 0 otherwise
 * @param ep endpoint
 */
GWENHYWFAR_API int GWEN_MsgEndpoint_HaveMessageToSend(const GWEN_MSG_ENDPOINT *ep);

/**
 * This virtual function is called with a message to be forwarded to this endpoint.
 * Example:
 * The project AqHome uses multiple message types:
 * - messages received from or sent to a network of microcontrollers
 * - MQTT messages forwarded to an MQTT server
 * - IPC messages for clients connecting to the aqhome daemon.
 * For every message type there is a dedicated endpoint class. However, messages from the network of
 * microcontrollers (e.g. temperature measurements, humidity etc) need to be forwarded from the mcu network to an
 * MQTT server.
 * For this the derived endpoint manager class (see @ref GWEN_MSG_ENDPOINT_MGR) of aqhome takes incoming messages from
 * the MCU endpoint class and calls GWEN_MsgEndpoint_ProcessOutMessage of the MQTT endpoint to transform the message into
 * something the MQTT server understands, because we can't just send an MCU message directly to the MQTT server (different
 * protocol).
 *
 * So the implementation of this class should look at the message presented here and internally make another message from this
 * which can then be send to the recipient of this endpoint (e.g. the MQTT server).
 * The way to look at such a message is to use the group id (see @ref GWEN_Msg_GetGroupId and @ref GWEN_MsgEndpoint_GetGroupId).
 * This is always set by the source endpoint class which received the message to the groupid of the receiving endpoint.
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_ProcessOutMessage(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);

/**
 * This virtual function is called by the internal implementation of @ref GWEN_MsgEndpoint_HandleReadable to check whether a
 * message is completely received.
 * @return 1 if complete, 0 otherwise, negative on error (see @ref GWEN_ERROR_GENERIC)
 * @param ep endpoint
 * @param m message to check
 */
GWENHYWFAR_API int GWEN_MsgEndpoint_IsMsgComplete(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);

/**
 * Get message currently in receiption.
 *
 * @return message (endpoint keeps ownership)
 * @param ep endpoint
 */
GWENHYWFAR_API GWEN_MSG *GWEN_MsgEndpoint_GetCurrentlyReceivedMsg(const GWEN_MSG_ENDPOINT *ep);

/**
 * Set message currently in receiption. Replaces previously set message. Endpoint takes over
 * ownership of the new message (if any).
 * This function is mainly called by implementations of @ref GWEN_MsgEndpoint_HandleReadable.
 *
 * @param ep endpoint
 * @param m message currently under receiption
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_SetCurrentlyReceivedMsg(GWEN_MSG_ENDPOINT *ep, GWEN_MSG *m);

/**
 * Not used by the base msgio functions/classes but by derived classes (especially in AqHome).
 *
 * groupids should only contain a single bit set so that they can simply be ORed together.
 * Default value is the one given as groupId in the constructor (see @ref GWEN_MsgEndpoint_new).
 *
 * @return accepted group ids
 * @param ep endpoint
 */
GWENHYWFAR_API int GWEN_MsgEndpoint_GetAcceptedGroupIds(const GWEN_MSG_ENDPOINT *ep);

/**
 * Set accepted group ids (only used by derived classes).
 *
 * @param ep endpoint
 * @param i groupids to set
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_SetAcceptedGroupIds(GWEN_MSG_ENDPOINT *ep, int i);

/**
 * Add accepted group ids (only used by derived classes).
 *
 * @param ep endpoint
 * @param i groupids to add
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_AddAcceptedGroupIds(GWEN_MSG_ENDPOINT *ep, int i);

/**
 * Remove accepted group ids (only used by derived classes).
 *
 * @param ep endpoint
 * @param i groupids to remove
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_DelAcceptedGroupIds(GWEN_MSG_ENDPOINT *ep, int i);


/*@}*/


/** @name Input/Output
 *
 * Endpoints are generally based on file descriptors/sockets (see glibc function "open").
 * The implementation of an endpoint class uses these virtual functions here to actually handle
 * IO.
 * The class @ref GWEN_MSG_ENDPOINT_MGR calls @ref GWEN_MsgEndpoint_GetReadFd to get a readable file descriptor
 * from endpoints and @ref GWEN_MsgEndpoint_GetWriteFd to get the file descriptor for writing.
 * When file descriptors are returned those are then used for a call to "select" to wait for activity.
 * If a file descriptor becomes readable @ref GWEN_MsgEndpoint_HandleReadable is called.
 * If a file descriptor becomes writable @ref GWEN_MsgEndpoint_HandleWritable is called.
 * If one of those functions fails (i.e. returns an error code) the endpoint is marked as "DISCONNECTED"
 * (i.e. the flag @ref GWEN_MSG_ENDPOINT_FLAGS_DISCONNECTED is added to the endpoints flags).
 */
/*@{*/

/**
 * Virtual function to get file descriptor to be used for the read list of "select" (see man 3 select).
 *
 * If the implementation wants to read a message it returns the file descriptor/socket. If the endpoint
 * does not to read it should return -1 here.
 *
 * The internal implementation always returns the file descriptor set by @ref GWEN_MsgEndpoint_SetFd.
 *
 * @return file descriptor/socket to read from (-1 if none)
 * @param ep endpoint
 */
GWENHYWFAR_API int GWEN_MsgEndpoint_GetReadFd(GWEN_MSG_ENDPOINT *ep);

/**
 * Virtual function to get file descriptor to be used for the write list of "select" (see man 3 select).
 *
 * If the implementation wants to write a message it returns the file descriptor/socket. If the endpoint
 * does not to write it should return -1 here.
 *
 * The internal implementation returns the file descriptor set by @ref GWEN_MsgEndpoint_SetFd when the endpoint
 * has outbound messages in its internal list.
 *
 * @return file descriptor/socket to write to (-1 if none)
 * @param ep endpoint
 */
GWENHYWFAR_API int GWEN_MsgEndpoint_GetWriteFd(GWEN_MSG_ENDPOINT *ep);

/**
 * This virtual function is called when the "select" call indicates that the file descriptor given by
 * @ref GWEN_MsgEndpoint_GetReadFd is readable.
 *
 * The internal implementation just reads data from the file descriptor and sends it to the current message in
 * receiption (@ref GWEN_MsgEndpoint_GetCurrentlyReceivedMsg) as long as @ref GWEN_MsgEndpoint_IsMsgComplete
 * indicates that the current message is not complete.
 * If the current message is complete it will be added to the internal list of received messages and a new message
 * is created and set as the current message in receiption. The groupid of the newly created message is set to the
 * groupid of the endpoint (see @ref GWEN_MsgEndpoint_GetGroupId).
 *
 * @return 0 if okay, error code otherwise (see @ref GWEN_ERROR_GENERIC and others)
 * @param ep endpoint
 * @param emgr endpoint manager to which this endpoint belongs (i.e. the caller of this function)
 */
GWENHYWFAR_API int GWEN_MsgEndpoint_HandleReadable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);

/**
 * This virtual function is called when the "select" call indicates that the file descriptor given by
 * @ref GWEN_MsgEndpoint_GetReadFd is writable.
 *
 * The internal implementation just writes data from the currently first outbound message until all data is send.
 * After complete sending the message is removed from the outbound list and free'd.
 *
 * @return 0 if okay, error code otherwise (see @ref GWEN_ERROR_GENERIC and others)
 * @param ep endpoint
 * @param emgr endpoint manager to which this endpoint belongs (i.e. the caller of this function)
 */
GWENHYWFAR_API int GWEN_MsgEndpoint_HandleWritable(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_MGR *emgr);

/**
 * Virtual function called to handle incoming messages or to initiate connections etc.
 *
 * @param ep endpoint
 */
GWENHYWFAR_API void GWEN_MsgEndpoint_Run(GWEN_MSG_ENDPOINT *ep);

/**
 * This virtual function is called from implementations which can spawn other endpoints.
 * An example is the TCP server endpoint class. It has a file descriptor/socket which is only used to
 * accept connections. For an accepted incoming connection a new endpoint needs to be created.
 * This virtual function can be used to create specific endpoints (see @ref GWEN_TcpdEndpoint_new).
 */
GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_MsgEndpoint_CreateChild(GWEN_MSG_ENDPOINT *ep);

/**
 * This function can be used to drain the file descriptor/socket from waiting data.
 * An example is an endpoint implementation from AqHome which uses an USB serial device. When message receiption
 * errors occur (e.g. due to collisions on the data line) the rest of data on the line needs to be discarded
 * because it might belong the the errornous previous message.
 */
GWENHYWFAR_API int GWEN_MsgEndpoint_DiscardInput(GWEN_MSG_ENDPOINT *ep);

/*@}*/


/** @name Setters for Virtual Functions
 *
 */
/*@{*/
GWENHYWFAR_API GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN GWEN_MsgEndpoint_SetHandleReadableFn(GWEN_MSG_ENDPOINT *ep,
                                                                                        GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN f);
GWENHYWFAR_API GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN GWEN_MsgEndpoint_SetHandleWritableFn(GWEN_MSG_ENDPOINT *ep,
                                                                                        GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN f);
GWENHYWFAR_API GWEN_MSG_ENDPOINT_GET_READFD_FN GWEN_MsgEndpoint_SetGetReadFdFn(GWEN_MSG_ENDPOINT *ep,
                                                                               GWEN_MSG_ENDPOINT_GET_READFD_FN f);
GWENHYWFAR_API GWEN_MSG_ENDPOINT_GET_WRITEFD_FN GWEN_MsgEndpoint_SetGetWriteFdFn(GWEN_MSG_ENDPOINT *ep,
                                                                                 GWEN_MSG_ENDPOINT_GET_WRITEFD_FN f);

GWENHYWFAR_API GWEN_MSG_ENDPOINT_RUN_FN GWEN_MsgEndpoint_SetRunFn(GWEN_MSG_ENDPOINT *ep, GWEN_MSG_ENDPOINT_RUN_FN f);

GWENHYWFAR_API GWEN_MSG_ENDPOINT_PROC_OUTMSG_FN GWEN_MsgEndpoint_SetProcessOutMsgFn(GWEN_MSG_ENDPOINT *ep,
                                                                                    GWEN_MSG_ENDPOINT_PROC_OUTMSG_FN f);

GWENHYWFAR_API GWEN_MSG_ENDPOINT_ISMSGCOMPLETE_FN GWEN_MsgEndpoint_SetIsMsgCompleteFn(GWEN_MSG_ENDPOINT *ep,
                                                                                      GWEN_MSG_ENDPOINT_ISMSGCOMPLETE_FN f);

GWENHYWFAR_API GWEN_MSG_ENDPOINT_CREATECHILD_FN GWEN_MsgEndpoint_SetCreateChildFn(GWEN_MSG_ENDPOINT *ep,
                                                                                  GWEN_MSG_ENDPOINT_CREATECHILD_FN f);

/*@}*/


/*@}*/

#ifdef __cplusplus
}
#endif


#endif


